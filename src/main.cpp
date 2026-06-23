#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoOTA.h>
#include "Config.h"
#include "MovingAverageFilter.h"
#include "MotorController.h"
#include "LEDController.h"
#include "WebController.h"

// ==========================================
// GLOBAL OBJECTS & STATE VARIABLES
// ==========================================
SystemConfig config;
SystemState systemState = STATE_POWER_ON;

// Manual test states (Tippbetrieb), active only in STATE_IDLE
bool manualLEDs[7] = {false, false, false, false, false, false, false};
bool manualSwitches[2] = {false, false};

MotorController motors;
LEDController leds;
WebController web(config, motors, manualLEDs, manualSwitches);

// State timing variables (non-blocking)
unsigned long powerOnTimer = 0;
const unsigned long POWER_ON_DURATION = 4000; // 4 seconds in Boot LED test mode

unsigned long showStartTime = 0;
unsigned long showTimeLimitMs = 0;

// ==========================================
// CONFIG STORAGE CALLBACKS
// ==========================================
void saveConfigToFlash(const SystemConfig& cfg) {
    Preferences prefs;
    prefs.begin("titan-cfg-v3", false);
    size_t written = prefs.putBytes("config", &cfg, sizeof(SystemConfig));
    prefs.end();
    if (written == sizeof(SystemConfig)) {
        Serial.println("[System] Config successfully saved to Preferences Flash.");
    } else {
        Serial.println("[System] Warning: Failed to write config bytes to Flash!");
    }
}

void loadConfigFromFlash() {
    Preferences prefs;
    prefs.begin("titan-cfg-v3", false);
    size_t readBytes = prefs.getBytes("config", &config, sizeof(SystemConfig));
    prefs.end();

    if (readBytes != sizeof(SystemConfig)) {
        Serial.println("[System] Config not found or version mismatch. Initializing default settings...");
        loadDefaultConfig(config);
        saveConfigToFlash(config);
    } else {
        Serial.println("[System] Parameters successfully loaded from Preferences Flash.");
    }
}

// Handles configuration updates from Web Dashboard
void handleConfigChange(const SystemConfig& newCfg) {
    // 1. Persist config to flash
    saveConfigToFlash(newCfg);

    // 2. Apply LED brightness changes instantly
    leds.setSegmentBrightness(newCfg.led_seg_brightness);
    
    Serial.println("[System] Settings applied.");
}

// ==========================================
// STATE MACHINE TRANSITIONS
// ==========================================
void transitionToState(SystemState newState) {
    Serial.printf("[State Machine] Transitioning: %s -> %s\n", stateToString(systemState), stateToString(newState));
    
    // Actions upon exiting previous states
    switch (systemState) {
        case STATE_SHOW_ACTIVE:
            motors.stopAll();
            break;
        default:
            break;
    }

    // Safety constraint: If transitioning OUT of IDLE mode, immediately clear manual test overrides
    if (newState != STATE_IDLE) {
        memset(manualLEDs, 0, sizeof(manualLEDs));
        memset(manualSwitches, 0, sizeof(manualSwitches));
        Serial.println("[System] Cleared manual Tippbetrieb overrides for safety.");
    }

    // Enter actions for the new state
    systemState = newState;

    switch (systemState) {
        case STATE_POWER_ON:
            // 1. Enable external power VCC_EN (HIGH)
            digitalWrite(PIN_VCC_EN, HIGH);
            
            // 2. Shut off motors for safety
            motors.stopAll();
            
            // 3. Mark the boot-up timer starting point
            powerOnTimer = millis();
            break;

        case STATE_HOMING:
            // 1. Confirm external power is active
            digitalWrite(PIN_VCC_EN, HIGH);
            // 2. Start homing timer in MotorController
            motors.startHoming();
            break;

        case STATE_IDLE:
            // 1. Shut off all motors & switches
            motors.stopAll();
            digitalWrite(PIN_VCC_EN, HIGH); // Keep VCC_EN active for LEDs
            break;

        case STATE_SHOW_ACTIVE:
            // 1. Confirm power is enabled
            digitalWrite(PIN_VCC_EN, HIGH);
            
            // 2. Prepare motor variables and clear filters
            motors.startShowMode();
            
            // 3. Setup show duration timers
            showStartTime = millis();
            if (config.show_run_time > 0) {
                showTimeLimitMs = config.show_run_time * 1000;
            } else {
                showTimeLimitMs = 0; // Infinite run time
            }
            break;

        case STATE_ESTOP:
            // 1. Crucial: Pull VCC_EN LOW immediately to cut all external power
            digitalWrite(PIN_VCC_EN, LOW);
            
            // 2. Stop PWM drivers and isolate switches
            motors.stopAll();
            
            Serial.println("[WARNING] EMERGENCY STOP TRIGGERED. External power cutoff, all outputs killed.");
            break;
    }
}

// ==========================================
// INITIAL SETUP
// ==========================================
void setup() {
    // 1. Initialize Serial Communication
    Serial.begin(115200);
    delay(500); // stable serial wait
    Serial.println("\n==========================================");
    Serial.println("  Titan Diorama System Controller Booting  ");
    Serial.println("==========================================\n");

    // 2. Configure VCC Power Enable pin
    pinMode(PIN_VCC_EN, OUTPUT);
    digitalWrite(PIN_VCC_EN, LOW); // Start powered down

    // 3. Retrieve settings from Flash memory
    loadConfigFromFlash();

    // 4. Initialize Hardware Controllers
    motors.begin();
    leds.begin(config.led_seg_brightness);

    // 5. Start State Machine at POWER_ON (performs FastLED verification counts)
    transitionToState(STATE_POWER_ON);

    // 6. Spin up Web Dashboard and bind event callbacks
    web.begin(
        [](SystemState requestedState) {
            transitionToState(requestedState);
        },
        [](const SystemConfig& updatedCfg) {
            handleConfigChange(updatedCfg);
        }
    );

    // 7. Setup ArduinoOTA for Network Uploads
    ArduinoOTA.setHostname("titandiorama");
    
    ArduinoOTA.onStart([]() {
        String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
        Serial.println("[OTA] Start updating " + type);
        
        // Critical Safety: Disable VCC_EN and stop all motors before writing flash
        motors.stopAll();
        digitalWrite(PIN_VCC_EN, LOW);
    });

    ArduinoOTA.onEnd([]() {
        Serial.println("\n[OTA] Update successfully applied. Rebooting...");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("[OTA] Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    ArduinoOTA.begin();
    Serial.println("[OTA] Network firmware update receiver initialized.");
}

// ==========================================
// SYSTEM MAIN LOOP
// ==========================================
void loop() {
    // 1. Handle background Over-The-Air firmware flashes
    ArduinoOTA.handle();

    // 2. Drive non-blocking ADC sampling (runs at 5ms interval)
    motors.updateCurrentSensing();

    // 3. Execute state-specific logic (Non-blocking state machine)
    switch (systemState) {
        case STATE_POWER_ON:
            // Remain in test mode for specified duration to verify LED segments, then auto-home
            if (millis() - powerOnTimer >= POWER_ON_DURATION) {
                Serial.println("[System] LED Test Phase complete. Initiating Homing...");
                transitionToState(STATE_HOMING);
            }
            break;

        case STATE_HOMING:
            // Drives Motor 2 downward non-blocking until stall is detected
            if (motors.runHomingCycle(config.m2_speed, config.m2_current_limit, config.m2_blanking_time)) {
                Serial.println("[System] Homing completed. System ready.");
                transitionToState(STATE_IDLE);
            }
            break;

        case STATE_IDLE:
            // Apply manual Switch outputs (Tippbetrieb) in IDLE mode
            motors.writeSwitchesManual(config, manualSwitches[0], manualSwitches[1]);
            break;

        case STATE_SHOW_ACTIVE:
            // Run show loops
            motors.updateMotor1Show(config);
            
            // Drive Motor 2 (reverses direction automatically on current-stall or time limit)
            motors.updateMotor2Show(config);
            
            motors.updateSwitchesShow(config, true);

            // Manage automatic show timeout
            if (showTimeLimitMs > 0) {
                if (millis() - showStartTime >= showTimeLimitMs) {
                    Serial.println("[System] Show run duration reached. Entering IDLE.");
                    transitionToState(STATE_IDLE);
                }
            }
            break;

        case STATE_ESTOP:
            // Maintain absolute safety state: write LOW to all outputs continuously
            motors.stopAll();
            digitalWrite(PIN_VCC_EN, LOW);
            break;
    }

    // 4. Drive non-blocking frame tick for LEDs (throttled internally to 60fps)
    // Feeds manualLEDs overrides to LED Controller (applied in IDLE state)
    leds.update(systemState, manualLEDs);

    // 5. Calculate show countdown timer
    float timeLeft = 0.0f;
    if (systemState == STATE_SHOW_ACTIVE && showTimeLimitMs > 0) {
        unsigned long elapsed = millis() - showStartTime;
        if (elapsed < showTimeLimitMs) {
            timeLeft = (float)(showTimeLimitMs - elapsed) / 1000.0f;
        }
    }

    // 6. Update Asynchronous Web Dashboard (websocket telemetry tick)
    web.update(systemState, timeLeft);
}

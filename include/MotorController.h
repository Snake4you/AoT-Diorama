#pragma once

#include <Arduino.h>
#include <esp_adc_cal.h>
#include "Config.h"
#include "MovingAverageFilter.h"

class MotorController {
private:
    // LEDC PWM settings
    const uint32_t PWM_FREQ = 20000; // 20 kHz (ultrasonic)
    const uint8_t PWM_RES = 8;       // 8-bit (0-255)

    // LEDC Channels for ESP32 Arduino Core 2.x
    const uint8_t M1_PWM1_CH = 0;
    const uint8_t M1_PWM2_CH = 1;
    const uint8_t M2_PWM1_CH = 2;
    const uint8_t M2_PWM2_CH = 3;

    // Moving average filters (32 samples window)
    MovingAverageFilter<uint32_t, 32> m1Filter;
    MovingAverageFilter<uint32_t, 32> m2Filter;
    
    // ADC calibration characteristics
    esp_adc_cal_characteristics_t adcChars;

    // Motor 1 running state variables
    bool m1DirectionFwd;
    unsigned long m1LastReversalTime;
    float m1FilteredCurrent;

    // Motor 2 running state variables
    bool m2DirectionUp;
    unsigned long m2DirectionStartTime;
    float m2FilteredCurrent;
    unsigned long m2HomingStartTime;

    unsigned long lastSampleTime;

    // Safe LEDC write wrappers
    void writeM1PWM1(uint8_t val) {
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        ledcWrite(PIN_M1_PWM1, val);
#else
        ledcWrite(M1_PWM1_CH, val);
#endif
    }

    void writeM1PWM2(uint8_t val) {
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        ledcWrite(PIN_M1_PWM2, val);
#else
        ledcWrite(M1_PWM2_CH, val);
#endif
    }

    void writeM2PWM1(uint8_t val) {
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        ledcWrite(PIN_M2_PWM1, val);
#else
        ledcWrite(M2_PWM1_CH, val);
#endif
    }

    void writeM2PWM2(uint8_t val) {
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        ledcWrite(PIN_M2_PWM2, val);
#else
        ledcWrite(M2_PWM2_CH, val);
#endif
    }

    // Safe switch driver: ensures Pin1 and Pin2 are never HIGH simultaneously
    void driveSwitch(uint8_t pin1, uint8_t pin2, bool active, bool polarity) {
        if (!active) {
            digitalWrite(pin1, LOW);
            digitalWrite(pin2, LOW);
        } else {
            if (polarity) {
                digitalWrite(pin2, LOW); // Pull low first for safety
                digitalWrite(pin1, HIGH);
            } else {
                digitalWrite(pin1, LOW); // Pull low first for safety
                digitalWrite(pin2, HIGH);
            }
        }
    }

public:
    MotorController() : 
        m1DirectionFwd(true),
        m1LastReversalTime(0),
        m1FilteredCurrent(0.0f),
        m2DirectionUp(true),
        m2DirectionStartTime(0),
        m2FilteredCurrent(0.0f),
        m2HomingStartTime(0),
        lastSampleTime(0) {}

    void begin() {
        // 1. Initialize digital switch pins as outputs
        pinMode(PIN_SW1_OUT1, OUTPUT);
        pinMode(PIN_SW1_OUT2, OUTPUT);
        pinMode(PIN_SW2_OUT1, OUTPUT);
        pinMode(PIN_SW2_OUT2, OUTPUT);

        digitalWrite(PIN_SW1_OUT1, LOW);
        digitalWrite(PIN_SW1_OUT2, LOW);
        digitalWrite(PIN_SW2_OUT1, LOW);
        digitalWrite(PIN_SW2_OUT2, LOW);

        // 2. Initialize limit switch as Input Pullup
        pinMode(PIN_M2_LIMIT_SWITCH, INPUT_PULLUP);

        // 3. Initialize LEDC PWM channels
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        ledcAttach(PIN_M1_PWM1, PWM_FREQ, PWM_RES);
        ledcAttach(PIN_M1_PWM2, PWM_FREQ, PWM_RES);
        ledcAttach(PIN_M2_PWM1, PWM_FREQ, PWM_RES);
        ledcAttach(PIN_M2_PWM2, PWM_FREQ, PWM_RES);
#else
        ledcSetup(M1_PWM1_CH, PWM_FREQ, PWM_RES);
        ledcSetup(M1_PWM2_CH, PWM_FREQ, PWM_RES);
        ledcSetup(M2_PWM1_CH, PWM_FREQ, PWM_RES);
        ledcSetup(M2_PWM2_CH, PWM_FREQ, PWM_RES);

        ledcAttachPin(PIN_M1_PWM1, M1_PWM1_CH);
        ledcAttachPin(PIN_M1_PWM2, M1_PWM2_CH);
        ledcAttachPin(PIN_M2_PWM1, M2_PWM1_CH);
        ledcAttachPin(PIN_M2_PWM2, M2_PWM2_CH);
#endif

        // Ensure motors are stopped
        stopAll();

        // 4. Initialize ESP32 ADC Calibration
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adcChars);
    }

    /**
     * @brief Performs asynchronous, non-blocking sampling of ADC currents.
     * Throttled to 5ms interval.
     */
    void updateCurrentSensing() {
        unsigned long currentMillis = millis();
        if (currentMillis - lastSampleTime >= 5) {
            lastSampleTime = currentMillis;

            // Read raw values
            uint32_t raw1 = analogRead(PIN_M1_CURRENT_ADC);
            uint32_t raw2 = analogRead(PIN_M2_CURRENT_ADC);

            // Calibrate to millivolts using ESP32 ADC Calibration
            uint32_t mv1 = esp_adc_cal_raw_to_voltage(raw1, &adcChars);
            uint32_t mv2 = esp_adc_cal_raw_to_voltage(raw2, &adcChars);

            // Feed raw voltage into moving average filter
            m1Filter.add(mv1);
            m2Filter.add(mv2);

            // Calculate filtered current in mA using conversion constant
            m1FilteredCurrent = m1Filter.get() * CURRENT_SENSE_COEFF;
            m2FilteredCurrent = m2Filter.get() * CURRENT_SENSE_COEFF;
        }
    }

    float getMotor1Current() const { return m1FilteredCurrent; }
    float getMotor2Current() const { return m2FilteredCurrent; }

    /**
     * @brief Instantly stops both motors and deactivates H-Bridge switches.
     */
    void stopAll() {
        writeM1PWM1(0);
        writeM1PWM2(0);
        writeM2PWM1(0);
        writeM2PWM2(0);

        driveSwitch(PIN_SW1_OUT1, PIN_SW1_OUT2, false, false);
        driveSwitch(PIN_SW2_OUT1, PIN_SW2_OUT2, false, false);
    }

    /**
     * @brief Manually drives Motor 1 at a specific speed and direction.
     * @param direction 1 = Forward, -1 = Reverse, 0 = Stop
     * @param speed PWM speed value (0-255)
     */
    void jogMotor1(int direction, uint8_t speed) {
        if (direction == 1) {
            writeM1PWM1(speed);
            writeM1PWM2(0);
        } else if (direction == -1) {
            writeM1PWM1(0);
            writeM1PWM2(speed);
        } else {
            writeM1PWM1(0);
            writeM1PWM2(0);
        }
    }

    /**
     * @brief Manually drives Motor 2 at a specific speed and direction.
     * Incorporates safety limit switch checks for downward travel.
     * @param direction 1 = Up, -1 = Down, 0 = Stop
     * @param speed PWM speed value (0-255)
     */
    void jogMotor2(int direction, uint8_t speed) {
        if (direction == 1) {
            writeM2PWM1(speed);
            writeM2PWM2(0);
        } else if (direction == -1) {
            if (isLimitSwitchPressed()) {
                writeM2PWM1(0);
                writeM2PWM2(0);
            } else {
                writeM2PWM1(0);
                writeM2PWM2(speed);
            }
        } else {
            writeM2PWM1(0);
            writeM2PWM2(0);
        }
    }

    /**
     * @brief Checks limit switch status.
     * @return true if limit switch is pressed (Active LOW, returns LOW)
     */
    bool isLimitSwitchPressed() {
        return digitalRead(PIN_M2_LIMIT_SWITCH) == LOW;
    }

    /**
     * @brief Asynchronously handles homing sequence for Motor 2.
     * Drives Motor 2 downwards at a safe homing speed until limit switch is triggered.
     * @param homingSpeed PWM duty cycle for homing
     * @return true when homing is complete, false while in progress
     */
    void startHoming() {
        m2HomingStartTime = millis();
        m2Filter.reset();
    }

    /**
     * @brief Asynchronously handles homing sequence for Motor 2 using current sensing.
     * Drives Motor 2 downwards at a safe homing speed until stall current is detected.
     */
    bool runHomingCycle(uint8_t homingSpeed, uint16_t currentLimit, uint16_t blankingTime) {
        unsigned long now = millis();
        unsigned long elapsed = now - m2HomingStartTime;

        if (elapsed >= blankingTime) {
            if (m2FilteredCurrent >= currentLimit) {
                stopAll();
                m2DirectionUp = true; // Set direction to UP upon homing completion
                m2DirectionStartTime = now;
                m2Filter.reset();
                Serial.printf("[Motor 2 Homing] Stall detected: %.1f mA. Homing completed.\n", m2FilteredCurrent);
                return true;
            }
        }

        // Run downward direction: PWM2 active, PWM1 low
        writeM2PWM1(0);
        writeM2PWM2(homingSpeed);
        return false;
    }

    /**
     * @brief Resets show parameters (restarts timers and clears filters) when activating Show Mode.
     */
    void startShowMode() {
        m1DirectionFwd = true;
        m1LastReversalTime = millis();
        m1Filter.reset();

        m2DirectionUp = true;
        m2DirectionStartTime = millis();
        m2Filter.reset();
    }

    /**
     * @brief Executes the non-blocking Motor 1 reciprocating routine based on current limit.
     */
    void updateMotor1Show(const SystemConfig& cfg) {
        unsigned long now = millis();

        // Drive Motor 1 based on direction
        if (m1DirectionFwd) {
            writeM1PWM1(cfg.m1_speed);
            writeM1PWM2(0);
        } else {
            writeM1PWM1(0);
            writeM1PWM2(cfg.m1_speed);
        }

        // Check for overcurrent reversing logic (only if limit is set > 0 and blanking time has passed)
        if (cfg.m1_current_limit > 0 && (now - m1LastReversalTime >= cfg.m1_blanking_time)) {
            if (m1FilteredCurrent >= cfg.m1_current_limit) {
                m1DirectionFwd = !m1DirectionFwd;
                m1LastReversalTime = now;
                m1Filter.reset(); // clear filter to avoid immediately re-triggering on residual reading
                Serial.printf("[Motor 1] Overcurrent detected: %.1f mA. Reversing to %s.\n", 
                              m1FilteredCurrent, m1DirectionFwd ? "FORWARD" : "REVERSE");
            }
        }
    }

    /**
     * @brief Executes the non-blocking Motor 2 time-ramp travel routine.
     * @return true if a critical overcurrent fault occurs, false otherwise.
     */
    /**
     * @brief Executes the non-blocking Motor 2 time-ramp travel routine.
     * Reverses on time limits or on stall current detection.
     */
    void updateMotor2Show(const SystemConfig& cfg) {
        unsigned long now = millis();
        unsigned long elapsed = now - m2DirectionStartTime;

        float totalTimeMs = (m2DirectionUp ? cfg.m2_run_time_up : cfg.m2_run_time_down) * 1000.0f;
        float rampTimeMs = cfg.m2_ramp_time * 1000.0f;

        // Safety clamp: ramp time cannot exceed half of total travel time
        if (rampTimeMs > totalTimeMs / 2.0f) {
            rampTimeMs = totalTimeMs / 2.0f;
        }

        // Change directions if time limit is reached
        if (elapsed >= totalTimeMs) {
            m2DirectionUp = !m2DirectionUp;
            m2DirectionStartTime = now;
            elapsed = 0;
            totalTimeMs = (m2DirectionUp ? cfg.m2_run_time_up : cfg.m2_run_time_down) * 1000.0f;
            m2Filter.reset();
        }

        // Calculate trapezoidal speed ramp
        float targetSpeed = cfg.m2_speed;
        float currentSpeed = 0.0f;

        if (elapsed < rampTimeMs) {
            // Acceleration phase
            currentSpeed = targetSpeed * ((float)elapsed / rampTimeMs);
        } else if (elapsed > totalTimeMs - rampTimeMs) {
            // Deceleration phase
            currentSpeed = targetSpeed * ((totalTimeMs - (float)elapsed) / rampTimeMs);
        } else {
            // Constant speed phase
            currentSpeed = targetSpeed;
        }

        uint8_t pwmVal = (uint8_t)constrain(currentSpeed, 0.0f, 255.0f);

        // Drive Motor 2 and check safety end-stops via current sensing
        if (m2DirectionUp) {
            if (elapsed >= cfg.m2_blanking_time && m2FilteredCurrent >= cfg.m2_current_limit) {
                writeM2PWM1(0);
                writeM2PWM2(0);
                m2DirectionUp = false;
                m2DirectionStartTime = now;
                m2Filter.reset();
                Serial.printf("[Motor 2] Upward end-stop stall detected: %.1f mA. Reversing DOWN.\n", m2FilteredCurrent);
            } else {
                writeM2PWM1(pwmVal);
                writeM2PWM2(0);
            }
        } else {
            if (elapsed >= cfg.m2_blanking_time && m2FilteredCurrent >= cfg.m2_current_limit) {
                writeM2PWM1(0);
                writeM2PWM2(0);
                m2DirectionUp = true;
                m2DirectionStartTime = now;
                m2Filter.reset();
                Serial.printf("[Motor 2] Downward end-stop stall detected: %.1f mA. Reversing UP.\n", m2FilteredCurrent);
            } else {
                writeM2PWM1(0);
                writeM2PWM2(pwmVal);
            }
        }
    }

    /**
     * @brief Statically enables configuration switches Out1 & Out2 safely during active show.
     */
    void updateSwitchesShow(const SystemConfig& cfg, bool showActive) {
        driveSwitch(PIN_SW1_OUT1, PIN_SW1_OUT2, cfg.sw1_active && showActive, cfg.sw1_polarity);
        driveSwitch(PIN_SW2_OUT1, PIN_SW2_OUT2, cfg.sw2_active && showActive, cfg.sw2_polarity);
    }

    /**
     * @brief Manually controls Switch 1 and Switch 2 in IDLE mode.
     */
    void writeSwitchesManual(const SystemConfig& cfg, bool sw1_manual, bool sw2_manual) {
        driveSwitch(PIN_SW1_OUT1, PIN_SW1_OUT2, sw1_manual, cfg.sw1_polarity);
        driveSwitch(PIN_SW2_OUT1, PIN_SW2_OUT2, sw2_manual, cfg.sw2_polarity);
    }
};

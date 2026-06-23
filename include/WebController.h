#pragma once

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <functional>
#include "Config.h"
#include "DashboardUI.h"
#include "MotorController.h"

typedef std::function<void(SystemState)> StateChangeCallback;
typedef std::function<void(const SystemConfig&)> ConfigChangeCallback;

class WebController {
private:
    AsyncWebServer server;
    AsyncWebSocket ws;
    SystemConfig& config;
    MotorController& motors;
    
    // References to main manual states
    bool* manualLEDs;
    bool* manualSwitches;

    StateChangeCallback stateCallback;
    ConfigChangeCallback configCallback;

    unsigned long lastTelemetryTime;
    int connectedClientsCount;

    // Safety jogging parameters
    SystemState currentSystemState;
    bool isJogging;
    unsigned long lastJogMessageTime;

    // Direct callback for WebSocket events
    void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                 void *arg, uint8_t *data, size_t len) {
        switch (type) {
            case WS_EVT_CONNECT:
                connectedClientsCount++;
                Serial.printf("[Web] Client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
                sendCurrentConfig(client);
                break;
            case WS_EVT_DISCONNECT:
                if (connectedClientsCount > 0) connectedClientsCount--;
                Serial.printf("[Web] Client #%u disconnected\n", client->id());
                break;
            case WS_EVT_DATA:
                handleWebSocketMessage(client, arg, data, len);
                break;
            case WS_EVT_PONG:
            case WS_EVT_ERROR:
                break;
        }
    }

    // Handles incoming JSON commands and configuration saves from the Web UI
    void handleWebSocketMessage(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len) {
        AwsFrameInfo *info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, data, len);
            if (error) {
                Serial.print(R"([Web] Deserialization error: )");
                Serial.println(error.c_str());
                return;
            }

            const char* cmd = doc["cmd"];
            if (cmd) {
                if (strcmp(cmd, "start") == 0) {
                    if (stateCallback) stateCallback(STATE_SHOW_ACTIVE);
                } else if (strcmp(cmd, "stop") == 0) {
                    if (stateCallback) stateCallback(STATE_IDLE);
                } else if (strcmp(cmd, "estop") == 0) {
                    if (stateCallback) stateCallback(STATE_ESTOP);
                } else if (strcmp(cmd, "reset_estop") == 0) {
                    // Reset E-stop restarts homing routine
                    if (stateCallback) stateCallback(STATE_POWER_ON);
                } else if (strcmp(cmd, "jog") == 0) {
                    // Manual Tippbetrieb (only active in IDLE state)
                    if (currentSystemState == STATE_IDLE) {
                        int motor = doc["motor"] | 1;
                        int dir = doc["dir"] | 0; // 1 = FWD/UP, -1 = REV/DOWN, 0 = STOP
                        int speed = doc["speed"] | 100;
                        
                        if (motor == 1) {
                            motors.jogMotor1(dir, speed);
                        } else if (motor == 2) {
                            motors.jogMotor2(dir, speed);
                        }

                        isJogging = true;
                        lastJogMessageTime = millis();
                    }
                } else if (strcmp(cmd, "jog_stop") == 0) {
                    int motor = doc["motor"] | 1;
                    if (motor == 1) {
                        motors.jogMotor1(0, 0);
                    } else if (motor == 2) {
                        motors.jogMotor2(0, 0);
                    }
                    isJogging = false;
                } else if (strcmp(cmd, "toggle_manual_led") == 0) {
                    // Manual LED Tippbetrieb toggling (only active in IDLE state)
                    if (currentSystemState == STATE_IDLE) {
                        int seg = doc["seg"] | 0;
                        bool state = doc["state"] | false;
                        if (seg >= 0 && seg < 7) {
                            manualLEDs[seg] = state;
                        }
                    }
                } else if (strcmp(cmd, "toggle_manual_sw") == 0) {
                    // Manual Switch toggling (only active in IDLE state)
                    if (currentSystemState == STATE_IDLE) {
                        int sw = doc["sw"] | 1;
                        bool state = doc["state"] | false;
                        if (sw == 1 || sw == 2) {
                            manualSwitches[sw - 1] = state;
                        }
                    }
                } else if (strcmp(cmd, "save_config") == 0) {
                    // WiFi Config
                    bool wifiModeChanged = (config.wifi_ap_mode != (doc["wifi_ap_mode"] | true));
                    bool wifiSSIDChanged = (strcmp(config.wifi_ssid, doc["wifi_ssid"] | "") != 0);
                    bool wifiPassChanged = (strcmp(config.wifi_pass, doc["wifi_pass"] | "") != 0);

                    strncpy(config.wifi_ssid, doc["wifi_ssid"] | "TitanDioramaAP", sizeof(config.wifi_ssid));
                    strncpy(config.wifi_pass, doc["wifi_pass"] | "12345678", sizeof(config.wifi_pass));
                    config.wifi_ap_mode = doc["wifi_ap_mode"] | true;
                    
                    // Motor 1 Config
                    config.m1_speed = doc["m1_speed"] | 0;
                    config.m1_current_limit = doc["m1_current_limit"] | 100;
                    config.m1_blanking_time = doc["m1_blanking_time"] | 300;
                    config.m1_jog_speed = doc["m1_jog_speed"] | 200;
                    
                    // Motor 2 Config
                    config.m2_speed = doc["m2_speed"] | 180;
                    config.m2_ramp_time = doc["m2_ramp_time"] | 1.0f;
                    config.m2_run_time_up = doc["m2_run_time_up"] | 10.0f;
                    config.m2_run_time_down = doc["m2_run_time_down"] | 8.0f;
                    config.m2_jog_speed = doc["m2_jog_speed"] | 200;
                    config.m2_current_limit = doc["m2_current_limit"] | 120;
                    config.m2_blanking_time = doc["m2_blanking_time"] | 300;

                    // Digital Switches Config
                    config.sw1_active = doc["sw1_active"] | false;
                    config.sw1_polarity = doc["sw1_polarity"] | true;
                    config.sw2_active = doc["sw2_active"] | false;
                    config.sw2_polarity = doc["sw2_polarity"] | true;

                    // LEDs & System Config
                    JsonArray brightnessArr = doc["led_seg_brightness"].as<JsonArray>();
                    if (brightnessArr) {
                        for (int i = 0; i < 7; i++) {
                            config.led_seg_brightness[i] = brightnessArr[i] | 128;
                        }
                    }
                    config.show_run_time = doc["show_run_time"] | 45;

                    // Execute callback to save to flash (Preferences) and update systems
                    if (configCallback) configCallback(config);

                    // Acknowledge change to client
                    client->text(R"({"t":"ack"})");

                    // Trigger WiFi reconnection if network parameters changed
                    if (wifiModeChanged || wifiSSIDChanged || wifiPassChanged) {
                        Serial.println("[Web] WiFi config changed. Reinitializing network...");
                        setupWiFi();
                    }
                }
            }
        }
    }

    // Sends system config details to a newly connected client
    void sendCurrentConfig(AsyncWebSocketClient* client) {
        JsonDocument doc;
        doc["t"] = "cfg";
        doc["wifi_ssid"] = config.wifi_ssid;
        doc["wifi_pass"] = config.wifi_pass;
        doc["wifi_ap_mode"] = config.wifi_ap_mode;
        
        doc["m1_speed"] = config.m1_speed;
        doc["m1_current_limit"] = config.m1_current_limit;
        doc["m1_blanking_time"] = config.m1_blanking_time;
        doc["m1_jog_speed"] = config.m1_jog_speed;
        
        doc["m2_speed"] = config.m2_speed;
        doc["m2_ramp_time"] = config.m2_ramp_time;
        doc["m2_run_time_up"] = config.m2_run_time_up;
        doc["m2_run_time_down"] = config.m2_run_time_down;
        doc["m2_jog_speed"] = config.m2_jog_speed;
        doc["m2_current_limit"] = config.m2_current_limit;
        doc["m2_blanking_time"] = config.m2_blanking_time;

        doc["sw1_active"] = config.sw1_active;
        doc["sw1_polarity"] = config.sw1_polarity;
        doc["sw2_active"] = config.sw2_active;
        doc["sw2_polarity"] = config.sw2_polarity;

        JsonArray brightnessArr = doc["led_seg_brightness"].to<JsonArray>();
        for (int i = 0; i < 7; i++) {
            brightnessArr.add(config.led_seg_brightness[i]);
        }
        doc["show_run_time"] = config.show_run_time;

        String jsonString;
        serializeJson(doc, jsonString);
        client->text(jsonString);
    }

    // Broadcasts telemetry packet asynchronously
    void sendTelemetry(SystemState currentState, float timeLeft) {
        if (connectedClientsCount <= 0) return;

        JsonDocument doc;
        doc["t"] = "tel";
        doc["state"] = stateToString(currentState);
        doc["m1_i"] = motors.getMotor1Current();
        doc["m2_i"] = motors.getMotor2Current();
        doc["lim"] = motors.isLimitSwitchPressed();
        doc["time"] = timeLeft;

        // Add manual states for sync
        JsonArray ledArr = doc["m_leds"].to<JsonArray>();
        for (int i = 0; i < 7; i++) {
            ledArr.add(manualLEDs[i]);
        }
        JsonArray swArr = doc["m_sw"].to<JsonArray>();
        swArr.add(manualSwitches[0]);
        swArr.add(manualSwitches[1]);

        String jsonString;
        serializeJson(doc, jsonString);
        ws.textAll(jsonString);
    }

    // Manages WiFi Initialization
    void setupWiFi() {
        WiFi.disconnect(true, true);
        delay(10); // small delay to allow stack clear

        if (config.wifi_ap_mode) {
            WiFi.mode(WIFI_AP);
            WiFi.softAP(config.wifi_ssid, config.wifi_pass);
            Serial.printf("[WiFi] Access Point initialized. SSID: %s\n", config.wifi_ssid);
            Serial.printf("[WiFi] Open browser and navigate to: http://%s/\n", WiFi.softAPIP().toString().c_str());
        } else {
            WiFi.mode(WIFI_STA);
            WiFi.begin(config.wifi_ssid, config.wifi_pass);
            Serial.printf("[WiFi] Connecting to network SSID: %s...\n", config.wifi_ssid);
        }
    }

public:
    WebController(SystemConfig& cfg, MotorController& mtr, bool* manLeds, bool* manSw) 
        : server(80), ws("/ws"), config(cfg), motors(mtr), 
          manualLEDs(manLeds), manualSwitches(manSw),
          lastTelemetryTime(0), connectedClientsCount(0),
          currentSystemState(STATE_POWER_ON), isJogging(false), lastJogMessageTime(0) {}

    void begin(StateChangeCallback stateCb, ConfigChangeCallback configCb) {
        stateCallback = stateCb;
        configCallback = configCb;

        // 1. Establish WiFi network connection
        setupWiFi();

        // 2. Set up mDNS responder
        if (MDNS.begin("titandiorama")) {
            Serial.println("[mDNS] Responder started. Reachable at http://titandiorama.local/");
            MDNS.addService("http", "tcp", 80);
        } else {
            Serial.println("[mDNS] Error setting up mDNS responder!");
        }

        // 3. Set up HTTP file handler (dashboard index HTML)
        server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
            request->send_P(200, "text/html", DASHBOARD_HTML);
        });

        // 4. Register WebSocket events
        ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                          void *arg, uint8_t *data, size_t len) {
            this->onEvent(server, client, type, arg, data, len);
        });
        server.addHandler(&ws);

        // 5. Start HTTP & Socket server
        server.begin();
    }

    /**
     * @brief Async server update ticker.
     * Manages WebSocket client connections cleanup, safety jog timeouts, and drives telemetry broadcasts.
     */
    void update(SystemState currentState, float timeLeft) {
        currentSystemState = currentState;

        // Essential: clean up zombie socket clients to avoid memory leaks
        ws.cleanupClients();

        // Safety timeout for manual jog mode (Tippbetrieb)
        if (isJogging && (millis() - lastJogMessageTime > 800)) {
            Serial.println("[Web] Safety jog timeout triggered (connection lost). Stopping motors.");
            motors.stopAll();
            isJogging = false;
        }

        // Broadcast current measurements and states at 200ms intervals (5Hz)
        unsigned long currentMillis = millis();
        if (currentMillis - lastTelemetryTime >= 200) {
            lastTelemetryTime = currentMillis;
            sendTelemetry(currentState, timeLeft);
        }

        // Output station network IP address once connection is achieved
        static bool staConnected = false;
        if (!config.wifi_ap_mode) {
            if (WiFi.status() == WL_CONNECTED && !staConnected) {
                staConnected = true;
                Serial.printf("[WiFi] Connection successful. IP Address: %s\n", WiFi.localIP().toString().c_str());
            } else if (WiFi.status() != WL_CONNECTED) {
                staConnected = false;
            }
        }
    }
};

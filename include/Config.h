#pragma once

#include <Arduino.h>

// ==========================================
// GPIO PIN DEFINITIONS
// ==========================================

// Power Enable
#define PIN_VCC_EN              26  // External Power Enable (HIGH = ON)

// FastLED Pins
#define PIN_STATUS_LED          2   // GPIO02: Status LED (Single WS2812)
#define PIN_MAIN_LED_STRIP      13  // GPIO13: Main LED Strip (226 WS2812)

// Motor 1 (H-Bridge & Current)
#define PIN_M1_PWM1             16  // PWM / Direction 1
#define PIN_M1_PWM2             25  // PWM / Direction 2
#define PIN_M1_CURRENT_ADC      34  // ADC Channel for Current Monitoring

// Motor 2 (Linear Actuator, H-Bridge & Current & Limit)
#define PIN_M2_PWM1             27  // PWM / Direction 1
#define PIN_M2_PWM2             32  // PWM / Direction 2
#define PIN_M2_CURRENT_ADC      36  // ADC Channel for Current Monitoring
#define PIN_M2_LIMIT_SWITCH     17  // Home Limit Switch (Internal Pullup, Active LOW)

// H-Bridge Switch Outputs
#define PIN_SW1_OUT1            14  // Switch 1, Pin 1
#define PIN_SW1_OUT2            15  // Switch 1, Pin 2
#define PIN_SW2_OUT1            4   // Switch 2, Pin 1
#define PIN_SW2_OUT2            12  // Switch 2, Pin 2

// ==========================================
// SYSTEM STATE ENUM
// ==========================================
enum SystemState {
    STATE_POWER_ON,       // Booting, external power enabled, test LED colors active
    STATE_HOMING,         // Motor 2 homing downwards to limit switch
    STATE_IDLE,           // Waiting for trigger, Status LED breathing
    STATE_SHOW_ACTIVE,    // Show running (Motor 1 reciprocating, Motor 2 ramp moves)
    STATE_ESTOP           // Emergency stop, all outputs disabled
};

// String conversions for telemetry
inline const char* stateToString(SystemState state) {
    switch (state) {
        case STATE_POWER_ON:     return "POWER_ON";
        case STATE_HOMING:       return "HOMING";
        case STATE_IDLE:         return "IDLE";
        case STATE_SHOW_ACTIVE:  return "SHOW_ACTIVE";
        case STATE_ESTOP:        return "ESTOP";
        default:                 return "UNKNOWN";
    }
}

// ==========================================
// CONFIGURATION PARAMETERS STRUCTURE
// ==========================================
struct SystemConfig {
    // WiFi Settings
    char wifi_ssid[32];
    char wifi_pass[64];
    bool wifi_ap_mode; // true = Access Point, false = Station (Client)

    // Motor 1 Settings
    uint8_t m1_speed;         // 0-255 PWM
    uint16_t m1_current_limit; // mA (0 = Disabled)
    uint16_t m1_blanking_time; // ms (ignore current spikes on start/reverse)
    uint8_t m1_jog_speed;      // 0-255 PWM (Jog Mode / Tippbetrieb)

    // Motor 2 Settings
    uint8_t m2_speed;          // 0-255 PWM
    float m2_ramp_time;        // seconds (acceleration/deceleration)
    float m2_run_time_up;      // seconds
    float m2_run_time_down;    // seconds
    uint8_t m2_jog_speed;      // 0-255 PWM (Jog Mode / Tippbetrieb)
    uint16_t m2_current_limit; // mA (Current Limit / Tippbetrieb)
    uint16_t m2_blanking_time; // ms (ignore current spikes on start/reverse)

    // Switches
    bool sw1_active;
    bool sw1_polarity;         // true: OUT1 HIGH/OUT2 LOW, false: OUT1 LOW/OUT2 HIGH
    bool sw2_active;
    bool sw2_polarity;

    // FastLED Settings (Segment-specific brightness 0-255)
    uint8_t led_seg_brightness[7];

    // Show Duration
    uint16_t show_run_time;    // seconds (0 = infinite / manual stop)
};

// Default Configuration Loader
inline void loadDefaultConfig(SystemConfig& cfg) {
    strncpy(cfg.wifi_ssid, "TitanDioramaAP", sizeof(cfg.wifi_ssid));
    strncpy(cfg.wifi_pass, "12345678", sizeof(cfg.wifi_pass));
    cfg.wifi_ap_mode = true;

    // Motor 1 defaults: speed 0, jog 200, current limit 100mA
    cfg.m1_speed = 0;
    cfg.m1_current_limit = 100;    // 100mA current limit
    cfg.m1_blanking_time = 300;  // 300ms spike blanking
    cfg.m1_jog_speed = 200;

    // Motor 2 defaults: speed 180, jog 200, current limit 120mA, ramp 1s, up 10s, down 8s
    cfg.m2_speed = 180;
    cfg.m2_ramp_time = 1.0f;     // 1s ramp
    cfg.m2_run_time_up = 10.0f;  // 10s up
    cfg.m2_run_time_down = 8.0f; // 8s down
    cfg.m2_jog_speed = 200;
    cfg.m2_current_limit = 120;  // 120mA overcurrent safety limit
    cfg.m2_blanking_time = 300;  // 300ms spike blanking

    cfg.sw1_active = true;
    cfg.sw1_polarity = true;
    cfg.sw2_active = true;
    cfg.sw2_polarity = true;

    // Initialize all segment brightnesses to 128 (50% default)
    for (int i = 0; i < 7; i++) {
        cfg.led_seg_brightness[i] = 128;
    }
    
    cfg.show_run_time = 45;      // 45 seconds default show run time
}

// ==========================================
// ANALOG-TO-CURRENT CONVERSION CONSTANTS
// ==========================================
// INA2180A1: Gain = 20 V/V. Shunt = 0.2 Ohms.
// Voltage Divider: 2.2k (top) / 3.2k (bottom) -> ratio = 3.2 / 5.4 = 16 / 27
// Current in A = V_shunt / R_shunt = (V_ina / Gain) / R_shunt
// V_ina = V_adc / divider_ratio = V_adc * (27 / 16)
// Current in mA = V_adc_mv * (27/16) / 20 / 0.2 = V_adc_mv * 1.6875 / 4 = V_adc_mv * 0.421875
const float CURRENT_SENSE_COEFF = 0.421875f;

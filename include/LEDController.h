#pragma once

#include <Arduino.h>
#define FASTLED_INTERNAL // Disable build banner to keep compilation quiet
#include <FastLED.h>
#include "Config.h"

// Define LED counts
#define NUM_MAIN_LEDS    226
#define NUM_STATUS_LEDS  1

// Segment Indexes
#define SEG_LOWER_DOOR   0
#define SEG_FIRE_RIGHT   1
#define SEG_LOWER_BOOKS  2
#define SEG_FIRE_LEFT    3
#define SEG_UPPER_BOOKS  4
#define SEG_UPPER_FRAME  5
#define SEG_TITAN_EYE    6

struct LEDSegment {
    const char* name;
    int start;
    int length;
    CRGB test_color;
};

// Main LED segment definitions mapping (Fire Right +4, Lower Books -4, Fire Left +4)
const LEDSegment SEGMENTS[7] = {
    {"Lower Door",   0,   48, CRGB::Red},
    {"Fire Right",   48,  16, CRGB::Orange}, // 16 LEDs (was 12)
    {"Lower Books",  64,  48, CRGB::Green},  // 48 LEDs (was 52)
    {"Fire Left",    112, 16, CRGB::Blue},   // 16 LEDs (was 12)
    {"Upper Books",  128, 48, CRGB::Purple}, // 48 LEDs (shifted)
    {"Upper Frame",  176, 48, CRGB::Yellow}, // 48 LEDs (shifted)
    {"Titan Eye",    224, 2,  CRGB::White}   // 2 LEDs (shifted)
};

class LEDController {
private:
    CRGB leds[NUM_MAIN_LEDS];
    CRGB status_led[NUM_STATUS_LEDS];
    unsigned long lastFrameTime;
    
    // Per-segment brightness control array
    uint8_t segmentBrightness[7];

    // Helper animation functions (non-blocking) with per-segment scaling
    void runPowerOnAnimation() {
        for (int s = 0; s < 7; s++) {
            const LEDSegment& seg = SEGMENTS[s];
            CRGB color = seg.test_color;
            color.nscale8_video(segmentBrightness[s]);
            for (int i = 0; i < seg.length; i++) {
                leds[seg.start + i] = color;
            }
        }
    }

    void runIdleAnimation(const bool* manualLEDs = nullptr) {
        // Keep the main strip dark or low-intensity warm white by default
        for (int s = 0; s < 7; s++) {
            const LEDSegment& seg = SEGMENTS[s];
            CRGB idle_glow = CRGB(10, 8, 4);
            idle_glow.nscale8_video(segmentBrightness[s]);
            for (int i = 0; i < seg.length; i++) {
                leds[seg.start + i] = idle_glow;
            }
        }

        // Apply manual segment overrides
        if (manualLEDs) {
            for (int s = 0; s < 7; s++) {
                if (manualLEDs[s]) {
                    const LEDSegment& seg = SEGMENTS[s];
                    CRGB test_c = seg.test_color;
                    test_c.nscale8_video(segmentBrightness[s]);
                    for (int i = 0; i < seg.length; i++) {
                        leds[seg.start + i] = test_c;
                    }
                }
            }
        }
    }

    void runHomingAnimation() {
        uint8_t val = beatsin8(20, 40, 150);
        for (int s = 0; s < 7; s++) {
            const LEDSegment& seg = SEGMENTS[s];
            CRGB color = CRGB(val, (val * 3) / 5, 0);
            color.nscale8_video(segmentBrightness[s]);
            for (int i = 0; i < seg.length; i++) {
                leds[seg.start + i] = color;
            }
        }
    }

    void runShowAnimation() {
        // Segment 1: Lower Door (Magical red pulse)
        uint8_t door_pulse = beatsin8(15, 30, 200);
        CRGB door_color = CHSV(HUE_RED, 255, door_pulse);
        door_color.nscale8_video(segmentBrightness[SEG_LOWER_DOOR]);
        fill_solid(leds + SEGMENTS[SEG_LOWER_DOOR].start, SEGMENTS[SEG_LOWER_DOOR].length, door_color);

        // Segment 2: Fire Right (Flashing fire effect)
        updateFire(SEGMENTS[SEG_FIRE_RIGHT].start, SEGMENTS[SEG_FIRE_RIGHT].length, segmentBrightness[SEG_FIRE_RIGHT]);

        // Segment 3: Lower Books (Warm candle breathe)
        updateBookshelf(SEGMENTS[SEG_LOWER_BOOKS].start, SEGMENTS[SEG_LOWER_BOOKS].length, 8, segmentBrightness[SEG_LOWER_BOOKS]);

        // Segment 4: Fire Left (Flashing fire effect)
        updateFire(SEGMENTS[SEG_FIRE_LEFT].start, SEGMENTS[SEG_FIRE_LEFT].length, segmentBrightness[SEG_FIRE_LEFT]);

        // Segment 5: Upper Books (Warm candle breathe)
        updateBookshelf(SEGMENTS[SEG_UPPER_BOOKS].start, SEGMENTS[SEG_UPPER_BOOKS].length, 12, segmentBrightness[SEG_UPPER_BOOKS]);

        // Segment 6: Upper Frame (Ambient slow rainbow wave)
        uint8_t frame_hue = (millis() / 80) % 256;
        fill_rainbow(leds + SEGMENTS[SEG_UPPER_FRAME].start, SEGMENTS[SEG_UPPER_FRAME].length, frame_hue, 4);
        for (int i = 0; i < SEGMENTS[SEG_UPPER_FRAME].length; i++) {
            leds[SEGMENTS[SEG_UPPER_FRAME].start + i].nscale8_video(segmentBrightness[SEG_UPPER_FRAME]);
        }

        // Segment 7: Titan Eye (Pulsing glowing red eye)
        uint8_t eye_pulse = beatsin8(40, 80, 255);
        CRGB eye_color = CRGB(eye_pulse, 0, 0);
        eye_color.nscale8_video(segmentBrightness[SEG_TITAN_EYE]);
        fill_solid(leds + SEGMENTS[SEG_TITAN_EYE].start, SEGMENTS[SEG_TITAN_EYE].length, eye_color);
    }

    void runEStopAnimation() {
        // Rapidly flash main strip red at full brightness for safety
        uint8_t flash = (millis() / 150) % 2 ? 150 : 0;
        fill_solid(leds, NUM_MAIN_LEDS, CRGB(flash, 0, 0));
    }

    void updateFire(int start, int length, uint8_t brightness) {
        for (int i = 0; i < length; i++) {
            uint8_t flicker = random8(100, 255);
            CRGB color = CHSV(HUE_ORANGE - 3 + random8(8), 255, flicker);
            leds[start + i] = color.nscale8_video(brightness);
        }
    }

    void updateBookshelf(int start, int length, uint8_t bpm, uint8_t brightness) {
        uint8_t val = beatsin8(bpm, 80, 180);
        CRGB color = CRGB(val, (val * 3) / 4, val / 3);
        for (int i = 0; i < length; i++) {
            leds[start + i] = color.nscale8_video(brightness);
        }
    }

    void updateStatusLED(SystemState state) {
        switch (state) {
            case STATE_POWER_ON:
                status_led[0] = CRGB::Cyan;
                break;
            case STATE_HOMING:
                status_led[0] = CHSV(HUE_YELLOW, 255, beatsin8(30, 50, 255));
                break;
            case STATE_IDLE:
                status_led[0] = CHSV(HUE_BLUE, 255, beatsin8(15, 30, 255));
                break;
            case STATE_SHOW_ACTIVE:
                status_led[0] = CHSV(HUE_GREEN, 255, beatsin8(20, 50, 255));
                break;
            case STATE_ESTOP:
                status_led[0] = (millis() / 100) % 2 ? CRGB::Red : CRGB::Black;
                break;
        }
    }

public:
    LEDController() : lastFrameTime(0) {
        for (int i = 0; i < 7; i++) {
            segmentBrightness[i] = 128;
        }
    }

    void begin(const uint8_t initialBrightness[7]) {
        setSegmentBrightness(initialBrightness);

        // Initialize FastLED with designated GPIO pins
        FastLED.addLeds<WS2812B, PIN_MAIN_LED_STRIP, GRB>(leds, NUM_MAIN_LEDS);
        FastLED.addLeds<WS2812B, PIN_STATUS_LED, GRB>(status_led, NUM_STATUS_LEDS);
        
        // FastLED global brightness remains max (255) to delegate range to per-segment scaling
        FastLED.setBrightness(255);
        
        // Turn off everything initially
        fill_solid(leds, NUM_MAIN_LEDS, CRGB::Black);
        status_led[0] = CRGB::Black;
        FastLED.show();
    }

    void setSegmentBrightness(const uint8_t brightnesses[7]) {
        for (int i = 0; i < 7; i++) {
            segmentBrightness[i] = brightnesses[i];
        }
    }

    /**
     * @brief Non-blocking frame updater, throttled to 60fps (16.6ms)
     * 
     * @param state The current state of the main state machine
     * @param manualLEDs Manual toggle overrides array in STATE_IDLE
     */
    void update(SystemState state, const bool* manualLEDs = nullptr) {
        unsigned long currentMillis = millis();
        if (currentMillis - lastFrameTime >= 16) { // ~60 FPS
            lastFrameTime = currentMillis;

            // 1. Update status LED pattern
            updateStatusLED(state);

            // 2. Update main strip animation based on state
            switch (state) {
                case STATE_POWER_ON:
                    runPowerOnAnimation();
                    break;
                case STATE_HOMING:
                    runHomingAnimation();
                    break;
                case STATE_IDLE:
                    runIdleAnimation(manualLEDs);
                    break;
                case STATE_SHOW_ACTIVE:
                    runShowAnimation();
                    break;
                case STATE_ESTOP:
                    runEStopAnimation();
                    break;
            }

            // 3. Show pixels non-blocking
            FastLED.show();
        }
    }
};

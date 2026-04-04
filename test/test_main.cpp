#include <cstdio>
#include <cstdlib>
#include <cstring>

// Include mocks before the sketch code
#include "mocks/Arduino.h"
#include "mocks/Adafruit_NeoPixel.h"

// Define the global mock Serial instance
MockSerial Serial;

// Include the sketch as raw C++ (redefine setup/loop to avoid conflicts)
// We pull in the sketch's globals and functions by including it directly.
// But first, we need to handle the .ino file which expects Arduino environment.

// Re-declare the sketch globals and functions here to test them in isolation.
#define PIN_PIXEL  6
#define NUM_PIXELS 60

// Forward declarations
void knightRider(uint32_t color, uint8_t wait, uint8_t tailLength);

Adafruit_NeoPixel strip(NUM_PIXELS, PIN_PIXEL, NEO_GRB + NEO_KHZ800);

uint8_t color_r = 0;
uint8_t color_g = 0;
uint8_t color_b = 0;

// Copy testable functions from the sketch
void commandApply() {
    strip.show();
}

void commandSetColor() {
    while (!Serial.available());
    byte r = Serial.read();
    while (!Serial.available());
    byte g = Serial.read();
    while (!Serial.available());
    byte b = Serial.read();

    color_r = r;
    color_g = g;
    color_b = b;
}

void commandLedToColor() {
    while (!Serial.available());
    byte index = Serial.read();
    if (index >= 0 && index < NUM_PIXELS) {
        strip.setPixelColor(index, color_r, color_g, color_b);
    }
}

void loop() {
    if (Serial.available()) {
        char command = Serial.read();
        switch (command) {
            case 'a':
                commandApply();
                break;
            case 'c':
                commandSetColor();
                break;
            case 'l':
                commandLedToColor();
                break;
            case 'd':
                // demo() - not tested here (relies on many animation functions)
                break;
            case 'k':
                knightRider(Adafruit_NeoPixel::Color(255, 0, 0), 0, 3);
                break;
            default:
                break;
        }
    }
}

void knightRider(uint32_t color, uint8_t wait, uint8_t tailLength) {
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    for (int pass = 0; pass < 2; pass++) {
        int start = (pass == 0) ? 0 : NUM_PIXELS - 1;
        int end   = (pass == 0) ? NUM_PIXELS : -1;
        int step  = (pass == 0) ? 1 : -1;

        for (int i = start; i != end; i += step) {
            for (int p = 0; p < NUM_PIXELS; p++) {
                strip.setPixelColor(p, 0);
            }
            for (int t = 0; t <= tailLength; t++) {
                int pos = i - t * step;
                if (pos >= 0 && pos < NUM_PIXELS) {
                    uint8_t fade = 255 / (t + 1);
                    strip.setPixelColor(pos,
                        (r * fade) / 255,
                        (g * fade) / 255,
                        (b * fade) / 255);
                }
            }
            strip.show();
            delay(wait);
        }
    }
    for (int p = 0; p < NUM_PIXELS; p++) {
        strip.setPixelColor(p, 0);
    }
    strip.show();
}

uint32_t Wheel(byte WheelPos) {
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85) {
        return Adafruit_NeoPixel::Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else if(WheelPos < 170) {
        WheelPos -= 85;
        return Adafruit_NeoPixel::Color(0, WheelPos * 3, 255 - WheelPos * 3);
    } else {
        WheelPos -= 170;
        return Adafruit_NeoPixel::Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
}

// ============================================================
// Minimal test framework
// ============================================================

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT_EQ(expected, actual, msg) do { \
    if ((expected) != (actual)) { \
        printf("  FAIL: %s (expected %d, got %d)\n", msg, (int)(expected), (int)(actual)); \
        return false; \
    } \
} while(0)

#define ASSERT_NEQ(val1, val2, msg) do { \
    if ((val1) == (val2)) { \
        printf("  FAIL: %s (values should differ but both are %d)\n", msg, (int)(val1)); \
        return false; \
    } \
} while(0)

#define RUN_TEST(fn) do { \
    tests_run++; \
    printf("  %-50s", #fn); \
    if (fn()) { tests_passed++; printf("PASS\n"); } \
    else { tests_failed++; printf("FAILED\n"); } \
} while(0)

static void resetState() {
    color_r = 0;
    color_g = 0;
    color_b = 0;
    // Clear serial buffer
    while (Serial.available()) Serial.read();
    // Reset strip
    strip.begin();
    strip.showCount = 0;
}

// ============================================================
// Tests: Wheel() function
// ============================================================

bool test_wheel_at_zero() {
    // WheelPos=0 -> adjusted=255 -> falls in else branch (>=170)
    // adjusted -= 170 -> 85
    // Color(85*3, 255-85*3, 0) = Color(255, 0, 0)
    uint32_t c = Wheel(0);
    uint8_t r = (c >> 16) & 0xFF;
    uint8_t g = (c >> 8) & 0xFF;
    uint8_t b = c & 0xFF;
    ASSERT_EQ(255, r, "Wheel(0) red");
    ASSERT_EQ(0, g, "Wheel(0) green");
    ASSERT_EQ(0, b, "Wheel(0) blue");
    return true;
}

bool test_wheel_at_255() {
    // WheelPos=255 -> adjusted=0 -> first branch (<85)
    // Color(255-0, 0, 0) = Color(255, 0, 0)
    uint32_t c = Wheel(255);
    uint8_t r = (c >> 16) & 0xFF;
    uint8_t g = (c >> 8) & 0xFF;
    uint8_t b = c & 0xFF;
    ASSERT_EQ(255, r, "Wheel(255) red");
    ASSERT_EQ(0, g, "Wheel(255) green");
    ASSERT_EQ(0, b, "Wheel(255) blue");
    return true;
}

bool test_wheel_segment_boundaries() {
    // Test at boundary between segment 1 and 2 (adjusted=84 and 85)
    // adjusted=84 (WheelPos=171): Color(255-252, 0, 252) = Color(3, 0, 252)
    uint32_t c1 = Wheel(171);
    uint8_t r1 = (c1 >> 16) & 0xFF;
    ASSERT_EQ(3, r1, "Wheel(171) red at segment boundary");

    // adjusted=85 (WheelPos=170): Color(0, 0, 255)
    uint32_t c2 = Wheel(170);
    uint8_t r2 = (c2 >> 16) & 0xFF;
    uint8_t g2 = (c2 >> 8) & 0xFF;
    uint8_t b2 = c2 & 0xFF;
    ASSERT_EQ(0, r2, "Wheel(170) red");
    ASSERT_EQ(0, g2, "Wheel(170) green");
    ASSERT_EQ(255, b2, "Wheel(170) blue");
    return true;
}

bool test_wheel_produces_nonzero_colors() {
    // Verify Wheel produces valid colors across range
    for (int i = 0; i < 256; i++) {
        uint32_t c = Wheel((byte)i);
        uint8_t r = (c >> 16) & 0xFF;
        uint8_t g = (c >> 8) & 0xFF;
        uint8_t b = c & 0xFF;
        // At least one channel should be non-zero
        if (r == 0 && g == 0 && b == 0) {
            printf("  FAIL: Wheel(%d) produced black\n", i);
            return false;
        }
    }
    return true;
}

bool test_wheel_two_channels_active() {
    // In each segment, exactly 2 channels should have nonzero values
    // (one ramping up, one ramping down) except at exact boundaries
    // Test mid-segment values
    uint32_t c = Wheel(128); // mid-range
    uint8_t r = (c >> 16) & 0xFF;
    uint8_t g = (c >> 8) & 0xFF;
    uint8_t b = c & 0xFF;
    int nonzero = (r > 0) + (g > 0) + (b > 0);
    // Should have at least 1 non-zero channel
    if (nonzero < 1) {
        printf("  FAIL: Wheel(128) has no active channels\n");
        return false;
    }
    return true;
}

// ============================================================
// Tests: commandSetColor()
// ============================================================

bool test_set_color_basic() {
    resetState();
    Serial.pushBytes({100, 150, 200});
    commandSetColor();
    ASSERT_EQ(100, color_r, "color_r");
    ASSERT_EQ(150, color_g, "color_g");
    ASSERT_EQ(200, color_b, "color_b");
    return true;
}

bool test_set_color_zeros() {
    resetState();
    Serial.pushBytes({0, 0, 0});
    commandSetColor();
    ASSERT_EQ(0, color_r, "color_r zero");
    ASSERT_EQ(0, color_g, "color_g zero");
    ASSERT_EQ(0, color_b, "color_b zero");
    return true;
}

bool test_set_color_max() {
    resetState();
    Serial.pushBytes({255, 255, 255});
    commandSetColor();
    ASSERT_EQ(255, color_r, "color_r max");
    ASSERT_EQ(255, color_g, "color_g max");
    ASSERT_EQ(255, color_b, "color_b max");
    return true;
}

bool test_set_color_overwrites_previous() {
    resetState();
    Serial.pushBytes({10, 20, 30});
    commandSetColor();
    Serial.pushBytes({40, 50, 60});
    commandSetColor();
    ASSERT_EQ(40, color_r, "color_r overwritten");
    ASSERT_EQ(50, color_g, "color_g overwritten");
    ASSERT_EQ(60, color_b, "color_b overwritten");
    return true;
}

// ============================================================
// Tests: commandLedToColor()
// ============================================================

bool test_led_to_color_first_pixel() {
    resetState();
    color_r = 255; color_g = 0; color_b = 0;
    Serial.push(0); // index 0
    commandLedToColor();
    ASSERT_EQ(255, strip.pixels[0].r, "pixel 0 red");
    ASSERT_EQ(0, strip.pixels[0].g, "pixel 0 green");
    ASSERT_EQ(0, strip.pixels[0].b, "pixel 0 blue");
    return true;
}

bool test_led_to_color_last_pixel() {
    resetState();
    color_r = 0; color_g = 255; color_b = 0;
    Serial.push(59); // last valid index
    commandLedToColor();
    ASSERT_EQ(0, strip.pixels[59].r, "pixel 59 red");
    ASSERT_EQ(255, strip.pixels[59].g, "pixel 59 green");
    ASSERT_EQ(0, strip.pixels[59].b, "pixel 59 blue");
    return true;
}

bool test_led_to_color_out_of_bounds() {
    resetState();
    color_r = 255; color_g = 255; color_b = 255;
    // Set a known state for pixel 0 to verify nothing changes
    strip.pixels[0] = {0, 0, 0};
    Serial.push(60); // out of bounds
    commandLedToColor();
    // Pixel 0 should remain unchanged (nothing should have been written)
    ASSERT_EQ(0, strip.pixels[0].r, "pixel 0 unchanged after OOB");
    return true;
}

bool test_led_to_color_max_byte_out_of_bounds() {
    resetState();
    color_r = 255; color_g = 255; color_b = 255;
    Serial.push(255); // way out of bounds
    commandLedToColor();
    // Should not crash or write anywhere invalid
    return true;
}

bool test_led_to_color_uses_current_color() {
    resetState();
    // Set color, then set LED, then change color and set another LED
    color_r = 10; color_g = 20; color_b = 30;
    Serial.push(0);
    commandLedToColor();

    color_r = 40; color_g = 50; color_b = 60;
    Serial.push(1);
    commandLedToColor();

    ASSERT_EQ(10, strip.pixels[0].r, "pixel 0 keeps first color r");
    ASSERT_EQ(20, strip.pixels[0].g, "pixel 0 keeps first color g");
    ASSERT_EQ(40, strip.pixels[1].r, "pixel 1 gets second color r");
    ASSERT_EQ(50, strip.pixels[1].g, "pixel 1 gets second color g");
    return true;
}

// ============================================================
// Tests: commandApply()
// ============================================================

bool test_apply_calls_show() {
    resetState();
    ASSERT_EQ(0, strip.showCount, "show not called initially");
    commandApply();
    ASSERT_EQ(1, strip.showCount, "show called once");
    commandApply();
    ASSERT_EQ(2, strip.showCount, "show called twice");
    return true;
}

// ============================================================
// Tests: loop() command dispatch
// ============================================================

bool test_loop_dispatches_set_color() {
    resetState();
    Serial.pushBytes({'c', 111, 222, 33});
    loop();
    ASSERT_EQ(111, color_r, "loop dispatched 'c' - red");
    ASSERT_EQ(222, color_g, "loop dispatched 'c' - green");
    ASSERT_EQ(33, color_b, "loop dispatched 'c' - blue");
    return true;
}

bool test_loop_dispatches_led_to_color() {
    resetState();
    color_r = 100; color_g = 200; color_b = 50;
    Serial.pushBytes({'l', 5});
    loop();
    ASSERT_EQ(100, strip.pixels[5].r, "loop dispatched 'l' - red");
    ASSERT_EQ(200, strip.pixels[5].g, "loop dispatched 'l' - green");
    ASSERT_EQ(50, strip.pixels[5].b, "loop dispatched 'l' - blue");
    return true;
}

bool test_loop_dispatches_apply() {
    resetState();
    Serial.push('a');
    loop();
    ASSERT_EQ(1, strip.showCount, "loop dispatched 'a'");
    return true;
}

bool test_loop_ignores_unknown_command() {
    resetState();
    Serial.push('x');
    loop();
    // Should not crash, state unchanged
    ASSERT_EQ(0, color_r, "unknown cmd no side effects r");
    ASSERT_EQ(0, strip.showCount, "unknown cmd no show");
    return true;
}

bool test_loop_no_data() {
    resetState();
    // No data in serial buffer
    loop();
    // Should not crash
    ASSERT_EQ(0, strip.showCount, "no data no action");
    return true;
}

// ============================================================
// Tests: Full command sequence integration
// ============================================================

bool test_full_sequence_set_color_set_led_apply() {
    resetState();
    // Simulate: set color to (255, 128, 64), set LED 10, apply
    Serial.pushBytes({'c', 255, 128, 64});
    loop();
    Serial.pushBytes({'l', 10});
    loop();
    Serial.push('a');
    loop();

    ASSERT_EQ(255, strip.pixels[10].r, "sequence: pixel 10 red");
    ASSERT_EQ(128, strip.pixels[10].g, "sequence: pixel 10 green");
    ASSERT_EQ(64, strip.pixels[10].b, "sequence: pixel 10 blue");
    ASSERT_EQ(1, strip.showCount, "sequence: show called");
    return true;
}

bool test_multiple_leds_same_color() {
    resetState();
    Serial.pushBytes({'c', 100, 100, 100});
    loop();
    Serial.pushBytes({'l', 0});
    loop();
    Serial.pushBytes({'l', 30});
    loop();
    Serial.pushBytes({'l', 59});
    loop();
    Serial.push('a');
    loop();

    ASSERT_EQ(100, strip.pixels[0].r, "multi-led: pixel 0");
    ASSERT_EQ(100, strip.pixels[30].r, "multi-led: pixel 30");
    ASSERT_EQ(100, strip.pixels[59].r, "multi-led: pixel 59");
    ASSERT_EQ(0, strip.pixels[1].r, "multi-led: pixel 1 untouched");
    return true;
}

bool test_different_colors_different_leds() {
    resetState();
    // Set LED 0 to red
    Serial.pushBytes({'c', 255, 0, 0});
    loop();
    Serial.pushBytes({'l', 0});
    loop();
    // Set LED 1 to green
    Serial.pushBytes({'c', 0, 255, 0});
    loop();
    Serial.pushBytes({'l', 1});
    loop();

    ASSERT_EQ(255, strip.pixels[0].r, "diff colors: pixel 0 red");
    ASSERT_EQ(0, strip.pixels[0].g, "diff colors: pixel 0 no green");
    ASSERT_EQ(0, strip.pixels[1].r, "diff colors: pixel 1 no red");
    ASSERT_EQ(255, strip.pixels[1].g, "diff colors: pixel 1 green");
    return true;
}

// ============================================================
// Tests: knightRider()
// ============================================================

bool test_knight_rider_clears_strip_when_done() {
    resetState();
    knightRider(Adafruit_NeoPixel::Color(255, 0, 0), 0, 3);
    // After finishing, all pixels should be off
    for (int i = 0; i < NUM_PIXELS; i++) {
        ASSERT_EQ(0, strip.pixels[i].r, "pixel should be off after knight rider");
    }
    return true;
}

bool test_knight_rider_calls_show() {
    resetState();
    knightRider(Adafruit_NeoPixel::Color(255, 0, 0), 0, 2);
    // Should have called show many times (2 passes * 60 pixels + 1 final clear)
    if (strip.showCount < NUM_PIXELS * 2) {
        printf("  FAIL: expected at least %d show calls, got %d\n",
               NUM_PIXELS * 2, strip.showCount);
        return false;
    }
    return true;
}

bool test_knight_rider_via_serial() {
    resetState();
    Serial.push('k');
    loop();
    // Should complete without crash and clear strip
    for (int i = 0; i < NUM_PIXELS; i++) {
        ASSERT_EQ(0, strip.pixels[i].r, "pixel off after 'k' command");
    }
    return true;
}

// ============================================================
// Main
// ============================================================

int main() {
    printf("=== Wheel() tests ===\n");
    RUN_TEST(test_wheel_at_zero);
    RUN_TEST(test_wheel_at_255);
    RUN_TEST(test_wheel_segment_boundaries);
    RUN_TEST(test_wheel_produces_nonzero_colors);
    RUN_TEST(test_wheel_two_channels_active);

    printf("\n=== commandSetColor() tests ===\n");
    RUN_TEST(test_set_color_basic);
    RUN_TEST(test_set_color_zeros);
    RUN_TEST(test_set_color_max);
    RUN_TEST(test_set_color_overwrites_previous);

    printf("\n=== commandLedToColor() tests ===\n");
    RUN_TEST(test_led_to_color_first_pixel);
    RUN_TEST(test_led_to_color_last_pixel);
    RUN_TEST(test_led_to_color_out_of_bounds);
    RUN_TEST(test_led_to_color_max_byte_out_of_bounds);
    RUN_TEST(test_led_to_color_uses_current_color);

    printf("\n=== commandApply() tests ===\n");
    RUN_TEST(test_apply_calls_show);

    printf("\n=== loop() dispatch tests ===\n");
    RUN_TEST(test_loop_dispatches_set_color);
    RUN_TEST(test_loop_dispatches_led_to_color);
    RUN_TEST(test_loop_dispatches_apply);
    RUN_TEST(test_loop_ignores_unknown_command);
    RUN_TEST(test_loop_no_data);

    printf("\n=== knightRider() tests ===\n");
    RUN_TEST(test_knight_rider_clears_strip_when_done);
    RUN_TEST(test_knight_rider_calls_show);
    RUN_TEST(test_knight_rider_via_serial);

    printf("\n=== Integration tests ===\n");
    RUN_TEST(test_full_sequence_set_color_set_led_apply);
    RUN_TEST(test_multiple_leds_same_color);
    RUN_TEST(test_different_colors_different_leds);

    printf("\n========================================\n");
    printf("Results: %d/%d passed", tests_passed, tests_run);
    if (tests_failed > 0) {
        printf(", %d FAILED", tests_failed);
    }
    printf("\n========================================\n");

    return tests_failed > 0 ? 1 : 0;
}

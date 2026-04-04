#ifndef ADAFRUIT_NEOPIXEL_H_MOCK
#define ADAFRUIT_NEOPIXEL_H_MOCK

#include <cstdint>
#include <cstring>

#define NEO_GRB  0x01
#define NEO_KHZ800 0x02

struct PixelColor {
    uint8_t r, g, b;
};

class Adafruit_NeoPixel {
public:
    int _numPixels;
    PixelColor* pixels;
    int showCount;

    Adafruit_NeoPixel() : _numPixels(0), pixels(nullptr), showCount(0) {}

    Adafruit_NeoPixel(int n, int pin, int type)
        : _numPixels(n), showCount(0) {
        pixels = new PixelColor[n]();
    }

    ~Adafruit_NeoPixel() {
        delete[] pixels;
    }

    void begin() {
        // Reset all pixels to off
        if (pixels) {
            memset(pixels, 0, _numPixels * sizeof(PixelColor));
        }
    }

    void show() {
        showCount++;
    }

    void setPixelColor(int i, uint8_t r, uint8_t g, uint8_t b) {
        if (i >= 0 && i < _numPixels) {
            pixels[i] = {r, g, b};
        }
    }

    void setPixelColor(int i, uint32_t c) {
        uint8_t r = (c >> 16) & 0xFF;
        uint8_t g = (c >> 8) & 0xFF;
        uint8_t b = c & 0xFF;
        setPixelColor(i, r, g, b);
    }

    uint16_t numPixels() const { return _numPixels; }

    static uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
        return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }
};

#endif

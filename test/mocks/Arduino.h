#ifndef ARDUINO_H_MOCK
#define ARDUINO_H_MOCK

#include <cstdint>
#include <cstddef>
#include <queue>

typedef uint8_t byte;

// Mock Serial
class MockSerial {
public:
    std::queue<uint8_t> buffer;

    void begin(unsigned long) {}

    bool available() {
        return !buffer.empty();
    }

    uint8_t read() {
        if (buffer.empty()) return 0;
        uint8_t val = buffer.front();
        buffer.pop();
        return val;
    }

    void println(const char*) {}

    void push(uint8_t val) {
        buffer.push(val);
    }

    void pushBytes(std::initializer_list<uint8_t> bytes) {
        for (auto b : bytes) buffer.push(b);
    }
};

extern MockSerial Serial;

inline void delay(unsigned long) {}

#endif

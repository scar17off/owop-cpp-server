#pragma once

#include <cstdint>

class RGB {
public:
    uint8_t r;
    uint8_t g;
    uint8_t b;
    
    RGB() : r(0), g(0), b(0) {}
    RGB(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}

    // Equality operator
    bool operator==(const RGB& other) const;

    // Inequality operator
    bool operator!=(const RGB& other) const;
};
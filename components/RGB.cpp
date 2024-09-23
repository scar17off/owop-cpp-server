#include "RGB.h"

bool RGB::operator==(const RGB& other) const {
    return (r == other.r) && (g == other.g) && (b == other.b);
}

bool RGB::operator!=(const RGB& other) const {
    return !(*this == other);
}
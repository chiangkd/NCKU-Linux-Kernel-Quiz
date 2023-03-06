#include <stdint.h>
#include <stdbool.h>

bool is_pattern(uint16_t x)
{
    if (!x)
        return 0;

    for (; x > 0; x <<= 1) {
        if (!(x & 0x8000))
            return false;
    }

    return true;
}

bool is_pattern_bitwise(uint16_t x)
{
    const uint16_t n = -x;
    return (n ^ x) < x;
}
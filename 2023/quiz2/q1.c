#include <stdint.h>
#include <stdio.h>
static inline uint64_t pow2(uint8_t e) { return ((uint64_t)1) << e; }

/* method 1 */
uint64_t next_pow2_m1(uint64_t x)
{
    uint8_t lo = 0, hi = 63;
    while (lo < hi) {
        uint8_t test = (lo + hi)/2;
        if (x < pow2(test)) { hi = test; }
        else if (pow2(test) < x) { lo = test+1; }
        else { return pow2(test); }
    }
    return pow2(lo);
}

/* method 2 */
uint64_t next_pow2_m2(uint64_t x)
{
    x -= x && 1;
    x |= x >> 1;
    x |= x >> 1;
    x |= x >> 1;
    x |= x >> 1;
    x |= x >> 1;
    x |= x >> 1;
    x |= x >> 1;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    return x + 1;
}

uint64_t next_pow2_m3(uint64_t x)
{
    x -= x && 1;
    return x ? 1 << (64 - __builtin_clzl(x)) : 1;
}


int main()
{
    for(int i = 0; i < 0b1000000000000000000000; i++) {
        if (next_pow2_m1(i) != next_pow2_m3(i)){
            printf("failed\n");
        }
        else
            ;
    }

    return 0;
}
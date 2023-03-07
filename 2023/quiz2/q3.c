#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cpucycles.h"

/* Prevent GCC to optimize out the body of count_utf8 and swar_count_utf8 */
static volatile size_t _count;

size_t count_utf8(const char *buf, size_t len)
{
    const int8_t *p = (const int8_t *) buf;
    size_t counter = 0;
    for (size_t i = 0; i < len; i++) {
        /* -65 is 0b10111111, anything larger in two-complement's should start
         * new code point.
         */
        if (p[i] > -65)
            counter++;
    }
    return counter;
}

size_t swar_count_utf8(const char *buf, size_t len)
{
    const uint64_t *qword = (const uint64_t *) buf;
    const uint64_t *end = qword + (len >> 3);

    size_t count = 0;
    for (; qword != end; qword++) {
        const uint64_t t0 = *qword;
        const uint64_t t1 = ~t0;
        const uint64_t t2 = t1 & 0x04040404040404040llu;
        const uint64_t t3 = t2 + t2;
        const uint64_t t4 = t0 & t3;
        count += __builtin_popcountll(t4);
    }

    count = (1 << 3) * (len / 8)  - count;
    count += (len & 7) ? count_utf8((const char *) end, len & 7) : 0;

    return count;
}
#define BUFSIZE 50000
#define NTESTS 10


size_t performance(uint8_t *buf, uint16_t *count)
{
    size_t ans1, ans2;
    int64_t tick1, tick2;
    /* start test */
    tick1 = cpucycles();
    ans1 = count_utf8(buf, BUFSIZE);
    tick2 = cpucycles();

    int64_t no_swar_ticks = tick2 - tick1;

    tick1 = cpucycles();
    ans2 = swar_count_utf8(buf, BUFSIZE);
    tick2 = cpucycles();

    int64_t swar_ticks = tick2 - tick1;
}

int main(void)
{
    int seed;
    seed = (int)time(NULL);
    srand(seed);
    uint8_t buf[BUFSIZE];
    
    uint16_t *count_no_swar, *count_with_swar;
    
    for(int i = 0; i < NTESTS; i++) {
        for (size_t i = 0; i < BUFSIZE; i++)
            buf[i] = rand();
    }

    return 0;
}
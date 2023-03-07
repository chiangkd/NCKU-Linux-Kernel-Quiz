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


#define SBUFSIZE 4
#define NTESTS 10

typedef size_t (*fptr_countf8)(const char *, size_t);

uint32_t performance(fptr_countf8 operation, const char *buf, size_t len)
{
    size_t ans1, ans2;
    uint32_t tick1, tick2;
    /* start test */
    tick1 = cpucycles();
    ans1 = operation(buf, len);
    tick2 = cpucycles();

    return tick2 - tick1;
}

int main(void)
{
    int seed;
    seed = (int)time(NULL);
    srand(seed);
    int bufsize[SBUFSIZE] = {10000, 20000, 50000, 100000};
    
    uint32_t count_no_swar=0, count_with_swar=0;
    for (int i = 0; i < SBUFSIZE; i++) {
        uint8_t buf[bufsize[i]];   // utf-8 sequence
        for(int j = 0; j < NTESTS; j++) {
            for (int k = 0; k < bufsize[i]; k++)
                buf[k] = rand();
        count_no_swar += performance(count_utf8, buf, bufsize[i]);
        count_with_swar += performance(swar_count_utf8, buf, bufsize[i]);
    }
    printf("%d\t %d\t %d\n",bufsize[i] ,count_no_swar/NTESTS, count_with_swar/NTESTS);
    }
    

    return 0;
}
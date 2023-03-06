#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Implementation of LFSR (linear feedback shift register)
 * on uint64_t using irreducible polynomial x^64 + x^61 + x^34 + x^9 + 1
 * (On 32 bit we could use x^32 + x^22 + x^2 + x^1 + 1)
 */
static void lfsr(uint64_t *up)
{
    uint64_t new_bit =
        ((*up) ^ ((*up) >> 3) ^ ((*up) >> 30) ^ ((*up) >> 55)) & 1u;
    /* don't have to map '+1' in polynomial */
    *up = ((*up) >> 1) | (new_bit << 63);
    /* shift *up by 1 to RIGHT and insert new_bit at "empty" position */
}

static unsigned int N_BUCKETS;
static unsigned char N_BITS;

static const char log_table_256[256] = {
#define _(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
    -1,   0,    1,    1,    2,    2,    2,    2,    3,    3,    3,
    3,    3,    3,    3,    3,    _(4), _(5), _(5), _(6), _(6), _(6),
    _(6), _(7), _(7), _(7), _(7), _(7), _(7), _(7), _(7),
#undef _
};

#undef _

/* ASSUME x >= 1
 * returns smallest integer b such that 2^b = 1 << b is >= x
 */
uint64_t log2_64(uint64_t v)
{
    unsigned r;
    uint64_t t, tt, ttt;

    ttt = v >> 32;
    if (ttt) {
        tt = ttt >> 16;
        if (tt) {
            t = tt >> 8;
            if (t) {
                r = 56 + log_table_256[t];
            } else {
                r = 48 + log_table_256[tt];
            }
        } else {
            t = ttt >> 8;
            if (t) {
                r = 40 + log_table_256[t];
            } else {
                r = 32 + log_table_256[ttt];
            }
        }
    } else {
        tt = v >> 16;
        if (tt) {
            t = tt >> 8;
            if (t) {
                r = 24 + log_table_256[t];
            } else {
                r = 16 + log_table_256[tt];
            }
        } else {
            t = v >> 8;
            if (t) {
                r = 8 + log_table_256[t];
            } else {
                r = 0 + log_table_256[v];
            }
        }
    }
    return r;
}

void set_N_BUCKETS(unsigned int n)
{
    N_BUCKETS = n;
}
void set_N_BITS()
{
    N_BITS = log2_64(N_BUCKETS);
}

/* n == number of totally available buckets, so buckets = \{0, ...,, n-1\}
 * ASSUME n < (1 << 32)
 */
unsigned int bucket_number(uint64_t x)
{
    uint64_t mask111 = (1 << (N_BITS + 1)) - 1;
    uint64_t mask011 = (1 << (N_BITS)) - 1; /* one 1 less */

    unsigned char leq = ((x & mask111) < N_BUCKETS);
    /* leq (less or equal) is 0 or 1. */

    return (leq * (x & mask111)) +
           ((1 - leq) * ((x >> (N_BITS + 1)) & mask011));
    /* 'x >> (N_BITS + 1)' : take different set of bits -> better uniformity.
     * '... & mask011' guarantees that the result is less or equal N_BUCKETS.
     */
}

void fill_buckets(unsigned int *buckets, unsigned int iterations)
{
    uint64_t x = 0x98765421b16b00b5;
    unsigned char lfsr_iter = (N_BITS << 1);

    for (uint64_t i = 0; i < iterations; i++) {
        unsigned int tmp_bucket = bucket_number(x);
        *(buckets + tmp_bucket) = *(buckets + tmp_bucket) + 1;

        // 'turn handle' on LFSR lfsr_iteration-times!
        unsigned char ell = 0;
        while (ell < lfsr_iter) {
            lfsr(&x);
            ell++;
        }
    }
}

void evaluate_buckets(unsigned int *buckets)
{
    int i = 0;
    while (i < N_BUCKETS) {
        printf("%d:%d , ", i, *(buckets + i));
        i++;
        if (i % 10 == 0)
            printf("\n");
    }
}

int main()
{
    int num_of_buckets = 120;          /* an example of some non-power of 2 */
    int num_of_iterations = (1 << 20); /* roughly 1 million */

    set_N_BUCKETS(num_of_buckets);
    set_N_BITS();

    unsigned int *buckets = malloc(N_BUCKETS * sizeof(unsigned int));

    int i = 0;
    while (i < N_BUCKETS) {
        *(buckets + i) = 0;
        i++;
    }
    fill_buckets(buckets, num_of_iterations);
    evaluate_buckets(buckets);
    free(buckets);
    return 0;
}
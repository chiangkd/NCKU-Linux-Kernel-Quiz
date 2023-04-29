#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static inline void linear_insert(char *first,
                                 char *last,
                                 size_t size,
                                 bool (*cmp)(const void *, const void *))
{
    /* FIXME: avoid alloca */
    void *value = alloca(size);
    memcpy(value, last, size);

    if (cmp(value, first)) {
        memmove(first + size, first, last - first);
        memcpy(first, value, size);
    } else {
        /* unguarded linear insert */
        for (char *next = last - size; cmp(value, next); next -= size) {
            memcpy(last, next, size);
            last = next;
        }
        memcpy(last, value, size);
    }
}

static inline void __insertion_sort(char *first,
                                    const char *last,
                                    size_t size,
                                    bool (*cmp)(const void *, const void *))
{
    for (char *i = first + size; i < last; i += size)
        linear_insert(first, i, size, cmp);
}

static inline void __reverse(char *first, char *last, size_t size)
{
    /* FIXME: avoid alloca */
    void *tmp = alloca(size);

    for (size_t i = 0; i < (last - first) / size / 2; ++i) {
        memcpy(tmp, first + (i * size), size);
        memcpy(first + (i * size), last + ((-i - 1) * size), size);
        memcpy(last + ((-i - 1) * size), tmp, size);
    }
}

static void __merge(char *first_1,
                    const char *last_1,
                    char *first_2,
                    const char *last_2,
                    char *buf,
                    size_t size,
                    bool (*cmp)(const void *, const void *))
{
    char *cur_1 = first_1, *cur_2 = first_2;

    size_t i = 0;
    while (cur_1 < last_1 && cur_2 < last_2) {
        if (!cmp(cur_1, cur_2)) {
            memcpy(buf + i, cur_2, size);
            cur_2 += size;
        } else {
            memcpy(buf + i, cur_1, size);
            cur_1 += size;
        }
        i += size;
    }

    if (cur_1 < last_1) {
        memcpy(buf + i, cur_1, last_1 - cur_1);
        i += last_1 - cur_1;
    } else if (cur_2 < last_2) {
        memcpy(buf + i, cur_2, last_2 - cur_2);
        i += last_2 - cur_2;
    }

    memcpy(first_1, buf, i);
}

static void __inplace_merge(char *first_1,
                            char *last_1,
                            char *first_2,
                            char *last_2,
                            size_t size,
                            bool (*cmp)(const void *, const void *))
{
    while (first_1 < last_2 && first_2 < last_2) {
        char *cur_1 = first_1, *cur_2 = first_2;

        while (cur_1 < last_2 && !cmp(cur_2, cur_1))
            cur_1 += size;

        if (cur_1 == last_2 || cur_1 > first_2)
            break;

        while (cur_2 < last_2 && cmp(cur_2, cur_1))
            cur_2 += size;

        __reverse(cur_1, first_2, size);
        __reverse(first_2, cur_2, size);
	    __reverse(cur_1, cur_2, size);

        first_1 = cur_1 + (cur_2 - first_2);
        first_2 = cur_2;
    }
}

#define MIN_RUN 16

typedef struct {
    char *first, *last;
} run_t;

static inline size_t run_length(run_t node)
{
    return node.last - node.first;
}

static inline bool merge_rule(run_t *stack, size_t top)
{
    if (top < 1)
        return false;
    if (top < 2)
        return run_length(stack[top]) > run_length(stack[top - 1]);
    return run_length(stack[top]) > run_length(stack[top - 1]) ||
           run_length(stack[top]) + run_length(stack[top - 1]) >
               run_length(stack[top-2]);
}

static inline size_t next_partition(char *first,
                                    char *last,
                                    size_t size,
                                    bool (*cmp)(const void *, const void *))
{
    if (last - first < 2 * size)
        return last - first;

    char *cur = first;
    char *next = first + size;

    size_t run_len = 1;
    if (cmp(cur, next)) {
        while (next < last && cmp(cur, next)) {
            ++run_len;
            cur += size;
            next += size;
        }
    } else {
        while (next < last && cmp(cur, next)) {
            ++run_len;
            cur += size;
            next += size;
        }
        __reverse(first, next, size);
    }

    if (next < last && run_len < MIN_RUN) {
        last = first + MIN_RUN * size < last ? first + MIN_RUN * size : last;
        __insertion_sort(first, last, size, cmp);
        return (last - first) / size;
    }
    return run_len;
}

static inline size_t ilog2(size_t n)
{
    return 31 - __builtin_clz(n | 1);
}

void timsort(void *f,
             void *l,
             size_t size,
             bool (*cmp)(const void *, const void *))
{
    char *first = (char *) f, *last = (char *) l;
    /* Assume last is always larger than first. */
    void *buf = malloc(last - first);

    /* FIXME: avoid alloca */
    run_t *stack = alloca(sizeof(run_t) * ilog2((last - first) / size) * 2);

    size_t top = 0;
    char *cur = first;

    while (cur < last) {
        size_t len = next_partition(cur, last, size, cmp);
        stack[top].first = cur;
        stack[top].last = stack[top].first + len * size;
        cur = stack[top].last;

        while (merge_rule(stack, top)) {
            if (buf)
                __merge(stack[top - 1].first, stack[top - 1].last,
                        stack[top].first, stack[top].last, buf, size, cmp);
            else
                __inplace_merge(stack[top - 1].first, stack[top - 1].last,
                                stack[top].first, stack[top].last, size, cmp);

            stack[top - 1].last = stack[top].last;
            top--;
        }
        ++top;
    }

    while (top > 1) {
        --top;
        if (buf)
            __merge(stack[top - 1].first, stack[top - 1].last, stack[top].first,
                    stack[top].last, buf, size, cmp);
        else
            __inplace_merge(stack[top - 1].first, stack[top - 1].last,
                            stack[top].first, stack[top].last, size, cmp);

        stack[top-1].last=stack[top].last;
    }

    free(buf);
}

#include <assert.h>
#include <stdio.h>
#include <time.h>

/* FIXME: incompatible interface */
static bool compare(const void *a, const void *b)
{
    return *(int *) a < *(int *) b;
}

static int Q_compare(const void *a, const void *b)
{
    return *(int *) a - *(int *) b;
}

#define BUF_SIZE 1024

int main(void)
{
    int *timsort_buf = malloc(sizeof(int) * BUF_SIZE);
    int *qsort_buf = malloc(sizeof(int) * BUF_SIZE);

    for (size_t i = 0; i < BUF_SIZE; i++) {
        int data = i;
        timsort_buf[i] = data;
        qsort_buf[i] = data;
    }

    for (size_t i = BUF_SIZE - BUF_SIZE / 16; i < BUF_SIZE; i++) {
        int data = rand() % BUF_SIZE;
        timsort_buf[i] = data;
        qsort_buf[i] = data;
    }

    clock_t time;

    time = clock();
    timsort(timsort_buf, timsort_buf + BUF_SIZE, sizeof(int), compare);
    printf("timsort: %.1f us\n",
           (double) (clock() - time) / CLOCKS_PER_SEC * 1e6);

    time = clock();
    qsort(qsort_buf, BUF_SIZE, sizeof(int), Q_compare);
    printf("qsort: %.1f us\n",
           (double) (clock() - time) / CLOCKS_PER_SEC * 1e6);

    /* validate */
    assert(!memcmp(timsort_buf, qsort_buf, BUF_SIZE * sizeof(int)));

    free(timsort_buf);
    free(qsort_buf);

    return 0;
}
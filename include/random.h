#ifndef _RANDOM_H_
#define _RANDOM_H_

#include <stdint.h>
#include <stdlib.h>

extern __thread uint64_t *seeds;

#define my_random xorshf96

typedef uint64_t ticks;

#if defined(__i386__)
static inline ticks getticks(void)
{
    ticks ret;
    __asm__ __volatile__("rdtsc" : "=A"(ret));
    return ret;
}
#elif defined(__x86_64__)
static inline ticks getticks(void)
{
    unsigned hi, lo;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((unsigned long long) lo) | (((unsigned long long) hi) << 32);
}
#else
#error "Unsupported platform"
#endif

static inline uint64_t *seed_rand()
{
    uint64_t *seeds;
    if (posix_memalign((void **) &seeds, 64, 64) != 0) /* something wrong */
        return NULL;

    seeds[0] = getticks() % 123456789;
    seeds[1] = getticks() % 362436069;
    seeds[2] = getticks() % 521288629;
    return seeds;
}

/* Marsaglia's xorshf generator */
static inline uint64_t xorshf96(uint64_t *x, uint64_t *y, uint64_t *z)
{
    /* period 2^96-1 */
    uint64_t t;
    (*x) ^= (*x) << 16;
    (*x) ^= (*x) >> 5;
    (*x) ^= (*x) << 1;

    t = *x;
    (*x) = *y;
    (*y) = *z;
    (*z) = t ^ (*x) ^ (*y);

    return *z;
}

static inline long rand_range(long r)
{
    long v = xorshf96(seeds, seeds + 1, seeds + 2) % r;
    v++;
    return v;
}

/* Re-entrant version of rand_range(r) */
static inline long rand_range_re(unsigned int *seed, long r)
{
    long v = xorshf96(seeds, seeds + 1, seeds + 2) % r;
    v++;
    return v;
}

#endif

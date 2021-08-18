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
#elif defined(__aarch64__)
static inline ticks getticks(void)
{
    uint64_t val;
    /* According to ARM DDI 0487F.c, from Armv8.0 to Armv8.5 inclusive, the
     * system counter is at least 56 bits wide; from Armv8.6, the counter
     * must be 64 bits wide.  So the system counter could be less than 64
     * bits wide and it is attributed with the flag 'cap_user_time_short'
     * is true.
     */
    __asm__ __volatile__("mrs %0, cntvct_el0" : "=r"(val));
    return val;
}
#else
#error "Unsupported platform"
#endif

static inline uint64_t *seed_rand()
{
    uint64_t *_seeds;
    if (posix_memalign((void **) &_seeds, 64, 64) != 0) /* something wrong */
        return NULL;

    _seeds[0] = getticks() % 123456789;
    _seeds[1] = getticks() % 362436069;
    _seeds[2] = getticks() % 521288629;
    return _seeds;
}

/* Marsaglia's xorshf generator */
static inline uint64_t xorshf96(uint64_t *x, uint64_t *y, uint64_t *z)
{
    /* period 2^96-1 */
    *x ^= (*x) << 16;
    *x ^= (*x) >> 5;
    *x ^= (*x) << 1;

    uint64_t t = *x;
    *x = *y;
    *y = *z;
    *z = t ^ (*x) ^ (*y);

    return *z;
}

#endif

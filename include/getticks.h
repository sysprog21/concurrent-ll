#ifndef _H_GETTICKS_
#define _H_GETTICKS_

#include <stdint.h>
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
    __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
    return ((unsigned long long) lo) | (((unsigned long long) hi) << 32);
}
#endif

#endif

#ifndef _UTILS_H_INCLUDED_
#define _UTILS_H_INCLUDED_
// some utility functions

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sched.h>
#include <inttypes.h>
#include <sys/time.h>
#include <unistd.h>

#if defined(__SSE__)
#include <xmmintrin.h>
#else
#define _mm_pause() asm volatile("nop")
#endif
#if defined(__SSE2__)
#include <emmintrin.h>
#endif

#include <pthread.h>
#include "getticks.h"
#include "random.h"
#include "atomic_ops_if.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DO_ALIGN

#if defined(DO_ALIGN)
#define ALIGNED(N) __attribute__((aligned(N)))
#else
#define ALIGNED(N)
#endif

#define PAUSE _mm_pause()
static inline
void pause_rep(uint32_t num_reps)
{
    uint32_t i;
    for (i = 0; i < num_reps; i++) {
        PAUSE;
    }
}

static inline
void nop_rep(uint32_t num_reps)
{
    uint32_t i;
    for (i = 0; i < num_reps; i++) {
        __asm__ volatile("NOP");
    }
}

/* machine dependent parameters */

#if defined(DEFAULT)
#define NUMBER_OF_SOCKETS 1
#define CORES_PER_SOCKET CORE_NUM
#define CACHE_LINE_SIZE 64
#define NOP_DURATION 1
static uint8_t __attribute__((unused))
the_cores[] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
               16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
               32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47
};
static uint8_t __attribute__((unused)) the_sockets[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

#endif

/* Round up to next higher power of 2 (return x if it's already a power
 * of 2) for 32-bit numbers
 */
static inline
uint32_t pow2roundup(uint32_t x)
{
    if (x == 0)
        return 1;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

#ifdef __cplusplus
}
#endif


#endif

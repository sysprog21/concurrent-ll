#ifndef _UTILS_H_
#define _UTILS_H_

#include <inttypes.h>

#include "random.h"

#ifndef ALIGNED
#define ALIGNED(N) __attribute__((aligned(N)))
#endif

/* Round up to next higher power of 2 (return x if it's already a power
 * of 2) for 32-bit numbers
 */
static inline uint32_t next_power_of_two(uint32_t x)
{
    x += !x;
    return (uint32_t) 1 << (31 - __builtin_clz(x + x - 1));
}

#endif

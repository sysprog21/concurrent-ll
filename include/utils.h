#ifndef _UTILS_H_INCLUDED_
#define _UTILS_H_INCLUDED_

#include <inttypes.h>

#include "atomic_ops_if.h"
#include "random.h"

#define DO_ALIGN
#if defined(DO_ALIGN)
#define ALIGNED(N) __attribute__((aligned(N)))
#else
#define ALIGNED(N)
#endif

/* Round up to next higher power of 2 (return x if it's already a power
 * of 2) for 32-bit numbers
 */
static inline uint32_t pow2roundup(uint32_t x)
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

#endif

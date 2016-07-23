/*
 * Cross-platform atomic operations
 */
#ifndef _ATOMIC_OPS_IF_H_INCLUDED_
#define _ATOMIC_OPS_IF_H_INCLUDED_

#include <inttypes.h>

/* atomic operations interface */

// Compare-and-swap
#define CAS_PTR(a, b, c) __sync_val_compare_and_swap(a, b, c)
#define CAS_U8(a, b, c) __sync_val_compare_and_swap(a, b, c)
#define CAS_U16(a, b, c) __sync_val_compare_and_swap(a, b, c)
#define CAS_U32(a, b, c) __sync_val_compare_and_swap(a, b, c)
#define CAS_U64(a, b, c) __sync_val_compare_and_swap(a, b, c)

// Fetch-and-increment
#define FAI_U8(a) __sync_fetch_and_add(a, 1)
#define FAI_U16(a) __sync_fetch_and_add(a, 1)
#define FAI_U32(a) __sync_fetch_and_add(a, 1)
#define FAI_U64(a) __sync_fetch_and_add(a, 1)

// Fetch-and-decrement
#define FAD_U8(a) __sync_fetch_and_sub(a, 1)
#define FAD_U16(a) __sync_fetch_and_sub(a, 1)
#define FAD_U32(a) __sync_fetch_and_sub(a, 1)
#define FAD_U64(a) __sync_fetch_and_sub(a, 1)

// Increment-and-fetch
#define IAF_U8(a) __sync_add_and_fetch(a, 1)
#define IAF_U16(a) __sync_add_and_fetch(a, 1)
#define IAF_U32(a) __sync_add_and_fetch(a, 1)
#define IAF_U64(a) __sync_add_and_fetch(a, 1)

// Decrement-and-fetch
#define DAF_U8(a) __sync_sub_and_fetch(a, 1)
#define DAF_U16(a) __sync_sub_and_fetch(a, 1)
#define DAF_U32(a) __sync_sub_and_fetch(a, 1)
#define DAF_U64(a) __sync_sub_and_fetch(a, 1)

// Memory barrier
#define MEM_BARRIER  // nop on the opteron for these benchmarks

static inline void AO_nop_full(void)
{
    MEM_BARRIER;
}

#define CAS_U64_bool(addr, old, new) (old == CAS_U64(addr, old, new))

#define ATOMIC_CAS_MB(a, e, v) \
    CAS_U64_bool((volatile AO_t *)(a), (AO_t)(e), (AO_t)(v))
#define ATOMIC_FETCH_AND_INC_FULL(a) FAI_U64((volatile AO_t *)(a))

#endif

/*
 * Cross-platform atomic operations
 */
#ifndef _ATOMICS_IF_H_
#define _ATOMICS_IF_H_

#include <inttypes.h>

/* atomic operations interface */

// Compare-and-swap
#define CAS_PTR(a, b, c)                                                \
    __extension__({                                                     \
        typeof(*a) _old = b, _new = c;                                  \
        __atomic_compare_exchange(a, &_old, &_new, 0, __ATOMIC_SEQ_CST, \
                                  __ATOMIC_SEQ_CST);                    \
        _old;                                                           \
    })
#define CAS_U8(a, b, c) CAS_PTR(a, b, c)
#define CAS_U16(a, b, c) CAS_PTR((uint16_t *) a, b, c)
#define CAS_U32(a, b, c) CAS_PTR((uint32_t *) a, b, c)
#define CAS_U64(a, b, c) CAS_PTR((uint64_t *) a, b, c)

/* Fetch-and-increment */
#define FAI_U8(a) __atomic_fetch_add(a, 1, __ATOMIC_RELAXED)
#define FAI_U16(a) FAI_U8(a)
#define FAI_U32(a) FAI_U8(a)
#define FAI_U64(a) FAI_U8(a)

/* Fetch-and-decrement */
#define FAD_U8(a) __atomic_fetch_sub(a, 1, __ATOMIC_RELAXED)
#define FAD_U16(a) FAD_U8(a)
#define FAD_U32(a) FAD_U8(a)
#define FAD_U64(a) FAD_U8(a)

/* Increment-and-fetch */
#define IAF_U8(a) __atomic_add_fetch(a, 1, __ATOMIC_RELAXED)
#define IAF_U16(a) IAF_U8(a)
#define IAF_U32(a) IAF_U8(a)
#define IAF_U64(a) IAF_U8(a)

/* Decrement-and-fetch */
#define DAF_U8(a) __atomic_sub_fetch(a, 1, __ATOMIC_RELAXED)
#define DAF_U16(a) DAF_U8(a)
#define DAF_U32(a) DAF_U8(a)
#define DAF_U64(a) DAF_U8(a)

#endif

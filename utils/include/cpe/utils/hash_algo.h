#ifndef CPE_UTILS_HASH_ALGO_H
#define CPE_UTILS_HASH_ALGO_H
#include "cpe/pal/pal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t cpe_hash_str(const void * str, size_t len);
uint32_t cpe_hash_uint32(uint32_t h);
uint64_t cpe_hash_uint64(uint64_t h);
uint32_t cpe_hash_buffer(uint32_t seed, const void *src, size_t len);
uint32_t cpe_stable_hash_buffer(uint32_t seed, const void *src, size_t len);

#define cpe_hash_variable(seed, val) \
    (cpe_hash_buffer((seed), &(val), sizeof((val))))
#define cork_stable_hash_variable(seed, val) \
    (cork_stable_hash_buffer((seed), &(val), sizeof((val))))
#define cork_big_hash_variable(seed, val) \
    (cork_big_hash_buffer((seed), &(val), sizeof((val))))

#define CPE_HASH_UINT32(__h) do {               \
        __h ^= __h >> 16;                       \
        __h *= 0x85ebca6b;                      \
        __h ^= __h >> 13;                       \
        __h *= 0xc2b2ae35;                      \
        __h ^= __h >> 16;                       \
    } while(0)

#define CPE_HASH_UINT64(__k) do {               \
        __k ^= __k >> 33;                       \
        __k *= (uint64_t)(0xff51afd7ed558ccd);  \
        __k ^= __k >> 33;                       \
        __k *= (uint64_t)(0xc4ceb9fe1a85ec53);  \
        __k ^= __k >> 33;                       \
    } while(0)

#ifdef __cplusplus
}
#endif

#endif

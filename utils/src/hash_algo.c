#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/hash_algo.h"

#define CPE_ROTL32(a,b) (((a) << ((b) & 0x1f)) | ((a) >> (32 - ((b) & 0x1f))))
#define CPE_ROTL64(a,b) (((a) << ((b) & 0x3f)) | ((a) >> (64 - ((b) & 0x3f))))

#if _MSC_VER
typedef uint32_t cpe_aliased_uint32_t;
#else
typedef uint32_t __attribute__((__may_alias__))  cpe_aliased_uint32_t;
#endif

uint32_t cpe_hash_str(const void * str, size_t len) {
    size_t i;
	uint32_t h = (uint32_t)len;
	size_t step = (len >> 5) + 1;
	for (i = len; i >= step; i -= step) {
	    h = h ^ ((h<<5)+(h>>2)+(uint32_t)((const char *)str)[i-1]);
    }

	return h;
}

uint32_t cpe_hash_uint32(uint32_t h) {
    CPE_HASH_UINT32(h);
    return h;
}

uint64_t cpe_hash_uint64(uint64_t k) {
    CPE_HASH_UINT64(k);
    return k;
}

uint32_t cpe_stable_hash_buffer(uint32_t seed, const void *src, size_t len) {
    /* This is exactly the same as cpe_murmur_hash_x86_32, but with a byte swap
     * to make sure that we always process the uint32s little-endian. */
    const unsigned int nblocks = (unsigned int)(len / 4u);
    const cpe_aliased_uint32_t  *blocks = (const cpe_aliased_uint32_t *) src;
    const cpe_aliased_uint32_t  *end = blocks + nblocks;
    const cpe_aliased_uint32_t  *curr;
    const uint8_t  *tail = (const uint8_t *) end;

    uint32_t  h1 = seed;
    uint32_t  c1 = 0xcc9e2d51;
    uint32_t  c2 = 0x1b873593;
    uint32_t  k1 = 0;

    /* body */
    for (curr = blocks; curr != end; curr++) {
        uint32_t  k1;
        CPE_COPY_HTOL32(&k1, curr);

        k1 *= c1;
        k1 = CPE_ROTL32(k1,15);
        k1 *= c2;

        h1 ^= k1;
        h1 = CPE_ROTL32(h1,13);
        h1 = h1*5+0xe6546b64;
    }

    /* tail */
    switch (len & 3) {
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0];
                k1 *= c1; k1 = CPE_ROTL32(k1,15); k1 *= c2; h1 ^= k1;
    };

    /* finalization */
    h1 ^= len;
    CPE_HASH_UINT32(h1);
    return h1;
}

#define CPE_MURMUR_HASH_X86_32(seed, src, len, dest)                    \
    do {                                                                \
        const unsigned int nblocks = (unsigned int)(len / 4u);                          \
        const cpe_aliased_uint32_t  *blocks = (const cpe_aliased_uint32_t *) src; \
        const cpe_aliased_uint32_t  *end = blocks + nblocks;                \
        const cpe_aliased_uint32_t  *curr;                                  \
        const uint8_t  *tail = (const uint8_t *) end;                   \
                                                                        \
        uint32_t  h1 = seed;                                            \
        uint32_t  c1 = 0xcc9e2d51;                                      \
        uint32_t  c2 = 0x1b873593;                                      \
        uint32_t  k1 = 0;                                               \
                                                                        \
        /* body */                                                      \
        for (curr = blocks; curr != end; curr++) {                      \
            uint32_t  k1 = *curr;                                       \
                                                                        \
            k1 *= c1;                                                   \
            k1 = CPE_ROTL32(k1,15);                                     \
            k1 *= c2;                                                   \
                                                                        \
            h1 ^= k1;                                                   \
            h1 = CPE_ROTL32(h1,13);                                     \
            h1 = h1*5+0xe6546b64;                                       \
        }                                                               \
                                                                        \
        /* tail */                                                      \
        switch (len & 3) {                                              \
        case 3: k1 ^= tail[2] << 16;                                    \
        case 2: k1 ^= tail[1] << 8;                                     \
        case 1: k1 ^= tail[0];                                          \
            k1 *= c1; k1 = CPE_ROTL32(k1,15); k1 *= c2; h1 ^= k1;       \
        };                                                              \
                                                                        \
        /* finalization */                                              \
        h1 ^= len;                                                      \
        CPE_HASH_UINT32(h1);                                            \
        *(dest) = h1;                                                   \
    } while (0)

uint32_t cpe_hash_buffer(uint32_t seed, const void *src, size_t len) {
    uint32_t hash = 0;
    CPE_MURMUR_HASH_X86_32(seed, src, len, &hash);
    return hash;
}



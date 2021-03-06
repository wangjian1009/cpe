#ifndef CPE_PAL_PLATFORM_H
#define CPE_PAL_PLATFORM_H

#if defined __APPLE__
#    include <stdint.h>
#    include <ConditionalMacros.h>
#else
#endif

#ifndef CPE_BIG_ENDIAN
    #ifdef _BIG_ENDIAN_
        #if _BIG_ENDIAN_
            #define CPE_BIG_ENDIAN 1
        #endif
    #endif
    #ifndef CPE_BIG_ENDIAN
        #if defined(__hppa__) || \
            defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
            (defined(__MIPS__) && defined(__MIPSEB__)) || \
            defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
            defined(__sparc__) || defined(__powerpc__) || \
            defined(__mc68000__) || defined(__s390x__) || defined(__s390__)
            #define CPE_BIG_ENDIAN 1
        #endif
    #endif
    #ifndef CPE_BIG_ENDIAN
        #define CPE_BIG_ENDIAN  0
    #endif
#endif

#if CPE_BIG_ENDIAN
#define CPE_LITTLE_ENDIAN 0
#else
#define CPE_LITTLE_ENDIAN 1
#endif

#ifndef __WORDSIZE
#  if __WORDSIZE
#  else
#    if __SIZEOF_INT__ == 8
#      define __WORDSIZE (64)
#    else
#      define __WORDSIZE (32)
#    endif
#  endif
#endif

#ifdef _MSC_VER
#  define CPE_DEFAULT_ALIGN (1)
#else
#  define CPE_DEFAULT_ALIGN (__WORDSIZE / 8)
#endif

#define CPE_PAL_CALC_ALIGN_8(__value) if ((__value) % 8) { (__value) = ((((__value) >> 3) + 1) << 3); }
#define CPE_PAL_CALC_ALIGN_4(__value) if ((__value) % 4) { (__value) = ((((__value) >> 2) + 1) << 2); }
#define CPE_PAL_CALC_ALIGN_2(__value) if ((__value) % 2) { (__value) = ((((__value) >> 1) + 1) << 1); }

#define CPE_PAL_CALC_ALIGN(__value, __align)    \
    switch(__align) {                           \
    case 2:                                     \
        CPE_PAL_CALC_ALIGN_2(__value);          \
        break;                                  \
    case 4:                                     \
        CPE_PAL_CALC_ALIGN_4(__value);          \
        break;                                  \
    case 8:                                     \
        CPE_PAL_CALC_ALIGN_8(__value);          \
        break;                                  \
    }


#if (__WORDSIZE == 64)
#define CPE_PAL_ALIGN_DFT CPE_PAL_CALC_ALIGN_8
#else
#define CPE_PAL_ALIGN_DFT CPE_PAL_CALC_ALIGN_4
#endif

#define CPE_COPY_ENDIAN64(outp, inp) do {           \
        const char * in = (const char *)(inp);    \
        char *out = (char *)(outp);               \
        out[0] = in[7];                         \
        out[1] = in[6];                         \
        out[2] = in[5];                         \
        out[3] = in[4];                         \
        out[4] = in[3];                         \
        out[5] = in[2];                         \
        out[6] = in[1];                         \
        out[7] = in[0];                         \
    } while(0)

#define CPE_COPY_ENDIAN32(outp, inp) do {       \
        const char * in = (const char *)(inp);  \
        char *out = (char *)(outp);             \
                                                \
        out[0] = in[3];                         \
        out[1] = in[2];                         \
        out[2] = in[1];                         \
        out[3] = in[0];                         \
    } while(0)

#define CPE_COPY_ENDIAN16(outp, inp) do {       \
        const char * in = (const char *)(inp);  \
        char *out = (char *)(outp);             \
                                                \
        out[0] = in[1];                         \
        out[1] = in[0];                         \
    } while(0)

#define CPE_SWAP_ENDIAN64(p) do {                 \
        char * inout = (char *)(p);               \
        char b;                                   \
        b = inout[0]; inout[0] = inout[7]; inout[7] = b;  \
        b = inout[1]; inout[1] = inout[6]; inout[6] = b;  \
        b = inout[2]; inout[2] = inout[5]; inout[5] = b;  \
        b = inout[3]; inout[3] = inout[4]; inout[4] = b;  \
    } while(0)

#define CPE_SWAP_ENDIAN32(p) do {                 \
        char * inout = (char *)(p);               \
        char b;                                   \
        b = inout[0]; inout[0] = inout[3]; inout[3] = b;  \
        b = inout[1]; inout[1] = inout[2]; inout[2] = b;  \
    } while(0)

#define CPE_SWAP_ENDIAN16(p) do {       \
        char * inout = (char *)(p);               \
        char b;                                   \
        b = inout[0]; inout[0] = inout[1]; inout[1] = b;  \
    } while(0)

#if CPE_LITTLE_ENDIAN
/*网络字节序处理 */
#define CPE_COPY_HTON64(outp, inp) CPE_COPY_ENDIAN64(outp, inp)
#define CPE_COPY_NTOH64(outp, inp) CPE_COPY_ENDIAN64(outp, inp)
#define CPE_COPY_HTON32(outp, inp) CPE_COPY_ENDIAN32(outp, inp)
#define CPE_COPY_NTOH32(outp, inp) CPE_COPY_ENDIAN32(outp, inp)
#define CPE_COPY_HTON16(outp, inp) CPE_COPY_ENDIAN16(outp, inp)
#define CPE_COPY_NTOH16(outp, inp) CPE_COPY_ENDIAN16(outp, inp)
#define CPE_SWAP_HTON64(p) CPE_SWAP_ENDIAN64(p)
#define CPE_SWAP_NTOH64(p) CPE_SWAP_ENDIAN64(p)
#define CPE_SWAP_HTON32(p) CPE_SWAP_ENDIAN32(p)
#define CPE_SWAP_NTOH32(p) CPE_SWAP_ENDIAN32(p)
#define CPE_SWAP_HTON16(p) CPE_SWAP_ENDIAN16(p)
#define CPE_SWAP_NTOH16(p) CPE_SWAP_ENDIAN16(p)
/*LITTLE_ENDIAN字节序处理 */
#define CPE_COPY_HTOL64(outp, inp) do { memcpy(outp, inp, 8); } while(0)
#define CPE_COPY_LTOH64(outp, inp) do { memcpy(outp, inp, 8); } while(0)
#define CPE_COPY_HTOL32(outp, inp) do { memcpy(outp, inp, 4); } while(0)
#define CPE_COPY_LTOH32(outp, inp) do { memcpy(outp, inp, 4); } while(0)
#define CPE_COPY_HTOL16(outp, inp) do { memcpy(outp, inp, 2); } while(0)
#define CPE_COPY_LTOH16(outp, inp) do { memcpy(outp, inp, 2); } while(0)
#define CPE_SWAP_HTOL64(p) do { } while(0)
#define CPE_SWAP_LTOH64(p) do { } while(0)
#define CPE_SWAP_HTOL32(p) do { } while(0)
#define CPE_SWAP_LTOH32(p) do { } while(0)
#define CPE_SWAP_HTOL16(p) do { } while(0)
#define CPE_SWAP_LTOH16(p) do { } while(0)
#else
/*网络字节序处理 */
#define CPE_COPY_HTON64(outp, inp) do { memcpy(outp, inp, 8); } while(0)
#define CPE_COPY_NTOH64(outp, inp) do { memcpy(outp, inp, 8); } while(0)
#define CPE_COPY_HTON32(outp, inp) do { memcpy(outp, inp, 4); } while(0)
#define CPE_COPY_NTOH32(outp, inp) do { memcpy(outp, inp, 4); } while(0)
#define CPE_COPY_HTON16(outp, inp) do { memcpy(outp, inp, 2); } while(0)
#define CPE_COPY_NTOH16(outp, inp) do { memcpy(outp, inp, 2); } while(0)
#define CPE_SWAP_HTON64(p) do { } while(0)
#define CPE_SWAP_NTOH64(p) do { } while(0)
#define CPE_SWAP_HTON32(p) do { } while(0)
#define CPE_SWAP_NTOH32(p) do { } while(0)
#define CPE_SWAP_HTON16(p) do { } while(0)
#define CPE_SWAP_NTOH16(p) do { } while(0)
/*LITTLE_ENDIAN字节序处理 */
#define CPE_COPY_HTOL64(outp, inp) CPE_COPY_ENDIAN64(outp, inp)
#define CPE_COPY_LTOH64(outp, inp) CPE_COPY_ENDIAN64(outp, inp)
#define CPE_COPY_HTOL32(outp, inp) CPE_COPY_ENDIAN32(outp, inp)
#define CPE_COPY_LTOH32(outp, inp) CPE_COPY_ENDIAN32(outp, inp)
#define CPE_COPY_HTOL16(outp, inp) CPE_COPY_ENDIAN16(outp, inp)
#define CPE_COPY_LTOH16(outp, inp) CPE_COPY_ENDIAN16(outp, inp)
#define CPE_SWAP_HTOL64(p) CPE_SWAP_ENDIAN64(p)
#define CPE_SWAP_LTOH64(p) CPE_SWAP_ENDIAN64(p)
#define CPE_SWAP_HTOL32(p) CPE_SWAP_ENDIAN32(p)
#define CPE_SWAP_LTOH32(p) CPE_SWAP_ENDIAN32(p)
#define CPE_SWAP_HTOL16(p) CPE_SWAP_ENDIAN16(p)
#define CPE_SWAP_LTOH16(p) CPE_SWAP_ENDIAN16(p)
#endif

# if defined(__INTEL_COMPILER) || defined(_MSC_VER)
#  define CPE_ALIGN(x) __declspec(align(x))
# else
#  define CPE_ALIGN(x) __attribute__ ((aligned(x)))
# endif

#ifdef __GNUC__
#define INLINE static inline
#else
#define INLINE static
#endif

#ifdef _MSC_VER

#define CPE_START_PACKED __pragma(pack(push, 1))
#define CPE_END_PACKED __pragma(pack(pop))
#define CPE_PACKED

#else

#define CPE_START_PACKED
#define CPE_END_PACKED
#define CPE_PACKED __attribute__((packed, aligned(1)))

#endif

#endif


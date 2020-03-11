#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_time.h"
#include "zip_pro_crypt.h"

#define CRC32(c, b) ((*(pcrc_32_tab+(((int)(c) ^ (b)) & 0xff))) ^ ((c) >> 8))

int cpe_zip_crypto_decrypt_byte(unsigned long* pkeys, const z_crc_t * pcrc_32_tab) {
    unsigned temp;  /* POTENTIAL BUG:  temp*(temp^1) may overflow in an
                     * unpredictable manner on 16-bit systems; not a problem
                     * with any known compiler so far, though */

    temp = ((unsigned)(*(pkeys+2)) & 0xffff) | 2;
    return (int)(((temp * (temp ^ 1)) >> 8) & 0xff);
}

int cpe_zip_crypto_update_keys(unsigned long* pkeys, const z_crc_t * pcrc_32_tab,int c) {
    (*(pkeys+0)) = CRC32((*(pkeys+0)), c);
    (*(pkeys+1)) += (*(pkeys+0)) & 0xff;
    (*(pkeys+1)) = (*(pkeys+1)) * 134775813L + 1;
    {
      register int keyshift = (int)((*(pkeys+1)) >> 24);
      (*(pkeys+2)) = CRC32((*(pkeys+2)), keyshift);
    }
    return c;
}

void cpe_zip_crypto_init_keys(const char* passwd,unsigned long* pkeys, const z_crc_t * pcrc_32_tab) {
    *(pkeys+0) = 305419896L;
    *(pkeys+1) = 591751049L;
    *(pkeys+2) = 878082192L;
    while (*passwd != '\0') {
        cpe_zip_crypto_update_keys(pkeys,pcrc_32_tab,(int)*passwd);
        passwd++;
    }
}

   /* "last resort" source for second part of crypt seed pattern */
#  ifndef ZCR_SEED2
#    define ZCR_SEED2 3141592654UL     /* use PI as default pattern */
#  endif

int cpe_zip_crypto_crypthead(
    const char* passwd,      /* password string */
    unsigned char* buf,      /* where to write header */
    int bufSize,
    unsigned long* pkeys,
    const z_crc_t * pcrc_32_tab,
    unsigned long crcForCrypting)
{
    int n;                       /* index in random header */
    int t;                       /* temporary */
    int c;                       /* random byte */
    unsigned char header[RAND_HEAD_LEN-2]; /* random header */
    static unsigned calls = 0;   /* ensure different random header each time */

    if (bufSize<RAND_HEAD_LEN)
      return 0;

    /* First generate RAND_HEAD_LEN-2 random bytes. We encrypt the
     * output of rand() to get less predictability, since rand() is
     * often poorly implemented.
     */
    if (++calls == 1)
    {
        srand((unsigned)(time(NULL) ^ ZCR_SEED2));
    }
    cpe_zip_crypto_init_keys(passwd, pkeys, pcrc_32_tab);
    for (n = 0; n < RAND_HEAD_LEN-2; n++)
    {
        c = (rand() >> 7) & 0xff;
        header[n] = (unsigned char)zencode(pkeys, pcrc_32_tab, c, t);
    }
    /* Encrypt random header (last two bytes is high word of crc) */
    cpe_zip_crypto_init_keys(passwd, pkeys, pcrc_32_tab);
    for (n = 0; n < RAND_HEAD_LEN-2; n++)
    {
        buf[n] = (unsigned char)zencode(pkeys, pcrc_32_tab, header[n], t);
    }
    buf[n++] = (unsigned char)zencode(pkeys, pcrc_32_tab, (int)(crcForCrypting >> 16) & 0xff, t);
    buf[n++] = (unsigned char)zencode(pkeys, pcrc_32_tab, (int)(crcForCrypting >> 24) & 0xff, t);
    return n;
}

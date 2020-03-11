#ifndef CPE_ZIP_PRO_CRYPTO_H_INCLEDED
#define CPE_ZIP_PRO_CRYPTO_H_INCLEDED
#include "zlib.h"

#define RAND_HEAD_LEN  12

/***********************************************************************
 * Return the next byte in the pseudo-random sequence
 */
int cpe_zip_crypto_decrypt_byte(unsigned long* pkeys, const z_crc_t * pcrc_32_tab);

/***********************************************************************
 * Update the encryption keys with the next byte of plain text
 */
int cpe_zip_crypto_update_keys(unsigned long* pkeys, const z_crc_t * pcrc_32_tab, int c);


/***********************************************************************
 * Initialize the encryption keys and the random header according to
 * the given password.
 */
void cpe_zip_crypto_init_keys(const char* passwd, unsigned long* pkeys, const z_crc_t * pcrc_32_tab);

#define zdecode(pkeys,pcrc_32_tab,c) \
    (cpe_zip_crypto_update_keys(pkeys,pcrc_32_tab,c ^= cpe_zip_crypto_decrypt_byte(pkeys,pcrc_32_tab)))

#define zencode(pkeys,pcrc_32_tab,c,t) \
    (t=cpe_zip_crypto_decrypt_byte(pkeys,pcrc_32_tab), cpe_zip_crypto_update_keys(pkeys,pcrc_32_tab,c), t^(c))

int cpe_zip_crypto_crypthead(
    const char* passwd,      /* password string */
    unsigned char* buf,      /* where to write header */
    int bufSize,
    unsigned long* pkeys,
    const z_crc_t * pcrc_32_tab,
    unsigned long crcForCrypting);

#endif

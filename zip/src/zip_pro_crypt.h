/* crypt.h -- base code for crypt/uncrypt ZIPfile


   Version 1.01e, February 12th, 2005

   Copyright (C) 1998-2005 Gilles Vollant

   This code is a modified version of crypting code in Infozip distribution

   The encryption/decryption parts of this source code (as opposed to the
   non-echoing password parts) were originally written in Europe.  The
   whole source package can be freely distributed, including from the USA.
   (Prior to January 2000, re-export from the US was a violation of US law.)

   This encryption code is a direct transcription of the algorithm from
   Roger Schlafly, described by Phil Katz in the file appnote.txt.  This
   file (appnote.txt) is distributed with the PKZIP program (even in the
   version without encryption capabilities).

   If you don't need crypting in your application, just define symbols
   NOCRYPT and NOUNCRYPT.

   This code support the "Traditional PKWARE Encryption".

   The new AES encryption added on Zip format by Winzip (see the page
   http://www.winzip.com/aes_info.htm ) and PKWare PKZip 5.x Strong
   Encryption is not supported.
*/

#define RAND_HEAD_LEN  12

/***********************************************************************
 * Return the next byte in the pseudo-random sequence
 */
int cpe_zip_crypto_decrypt_byte(unsigned long* pkeys, const unsigned long* pcrc_32_tab);

/***********************************************************************
 * Update the encryption keys with the next byte of plain text
 */
int cpe_zip_crypto_update_keys(unsigned long* pkeys,const unsigned long* pcrc_32_tab,int c);


/***********************************************************************
 * Initialize the encryption keys and the random header according to
 * the given password.
 */
void cpe_zip_crypto_init_keys(const char* passwd,unsigned long* pkeys,const unsigned long* pcrc_32_tab);

#define zdecode(pkeys,pcrc_32_tab,c) \
    (cpe_zip_crypto_update_keys(pkeys,pcrc_32_tab,c ^= cpe_zip_crypto_decrypt_byte(pkeys,pcrc_32_tab)))

#define zencode(pkeys,pcrc_32_tab,c,t) \
    (t=cpe_zip_crypto_decrypt_byte(pkeys,pcrc_32_tab), cpe_zip_crypto_update_keys(pkeys,pcrc_32_tab,c), t^(c))

int cpe_zip_crypto_crypthead(
    const char* passwd,      /* password string */
    unsigned char* buf,      /* where to write header */
    int bufSize,
    unsigned long* pkeys,
    const unsigned long* pcrc_32_tab,
    unsigned long crcForCrypting);

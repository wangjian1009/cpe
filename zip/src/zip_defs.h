#ifndef CPE_ZIP_DEFS_H_INCLEDED
#define CPE_ZIP_DEFS_H_INCLEDED
#include "zlib.h"

#ifdef HAVE_BZIP2
#include "bzlib.h"
#endif

#ifndef DEF_MEM_LEVEL
#if MAX_MEM_LEVEL >= 8
#define DEF_MEM_LEVEL 8
#else
#define DEF_MEM_LEVEL MAX_MEM_LEVEL
#endif
#endif

#define Z_BZIP2ED 12

typedef struct cpe_zip_tm_s {
    uInt tm_sec;            /* seconds after the minute - [0,59] */
    uInt tm_min;            /* minutes after the hour - [0,59] */
    uInt tm_hour;           /* hours since midnight - [0,23] */
    uInt tm_mday;           /* day of the month - [1,31] */
    uInt tm_mon;            /* months since January - [0,11] */
    uInt tm_year;           /* years - [1980..2044] */
} cpe_zip_tm;

int cpe_zip_fname_cmp(const char* fileName1, const char* fileName2, int iCaseSensitivity);
/*
   Compare two filename (fileName1,fileName2).
   If iCaseSenisivity = 1, comparision is case sensitivity (like strcmp)
   If iCaseSenisivity = 2, comparision is not case sensitivity (like strcmpi
                                or strcasecmp)
   If iCaseSenisivity = 0, case sensitivity is defaut of your operating system
    (like 1 on Unix, 2 on Windows)
*/

#endif

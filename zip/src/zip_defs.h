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

#endif

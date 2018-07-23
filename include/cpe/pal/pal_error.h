#ifndef CPE_PAL_ERROR_H
#define CPE_PAL_ERROR_H

#if defined CPE_OS_LINUX
#include <error.h>
#else
extern void error (int __status, int __errnum, const char *__format, ...)
        __attribute__ ((__format__ (__printf__, 3, 4)));

extern void error_at_line (int __status, int __errnum, const char *__fname, unsigned int __lineno, const char *__format, ...)
    __attribute__ ((__format__ (__printf__, 5, 6)));
#endif

#endif

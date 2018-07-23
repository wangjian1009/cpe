#include "cpe/pal/pal_error.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_errno.h"

#if defined CPE_OS_LINUX
#else
void error (int __status, int __errnum, const char *__format, ...) {
    va_list args;
    
    va_start(args, __format);
    vfprintf(stderr, __format, args);
    va_end(args);

    fprintf(stderr, ": %s", strerror(__errnum));

    if (__status) {
        exit(__status);
    }
}

void error_at_line(int __status, int __errnum, const char *__fname, unsigned int __lineno, const char *__format, ...) {
    va_list args;

    fprintf(stderr, "%s:%d: ", __fname, __lineno);
    
    va_start(args, __format);
    vfprintf(stderr, __format, args);
    va_end(args);

    fprintf(stderr, ": %s", strerror(__errnum));

    if (__status) {
        exit(__status);
    }
}

#endif


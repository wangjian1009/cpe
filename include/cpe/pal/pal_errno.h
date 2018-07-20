#ifndef CPE_PAL_ERRNO_H
#define CPE_PAL_ERRNO_H

#include <errno.h>

# if ! defined EBADFD
#define EBADFD EBADF
#endif

#endif

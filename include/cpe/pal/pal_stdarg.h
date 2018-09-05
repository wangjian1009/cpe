#ifndef CPE_PAL_STDARG_H
#define CPE_PAL_STDARG_H
#include <stdarg.h>

#if _MSC_VER < 1915
# define va_copy(t, s) (t) = (s)
#endif

#endif

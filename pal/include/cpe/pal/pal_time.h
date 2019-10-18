#ifndef CPE_PAL_TIME_H
#define CPE_PAL_TIME_H

#if _MSC_VER || __MINGW32__
#include "msvc_time.h"
#else
#include <sys/time.h>
#include <time.h>
#endif

#endif

#ifndef CPE_PAL_MSVC_TIME_H
#define CPE_PAL_MSVC_TIME_H
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct timeval;

#if _MSC_VER    
struct timezone {
    __int32  tz_minuteswest; /* minutes W of Greenwich */
    int  tz_dsttime;     /* type of dst correction */
};
#endif

int gettimeofday(struct timeval *tv/*in*/, struct timezone *tz/*in*/);
struct tm * localtime_r(const time_t *clock, struct tm *result);
struct tm * gmtime_r(const time_t *clock, struct tm *result);

void get_localtime(struct timeval *tv, struct tm  *tm_p);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

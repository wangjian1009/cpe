#if _MSC_VER || __MINGW32__
#include <winsock2.h>
#include <windows.h>
#include "cpe/pal/msvc_time.h"

const __int64 DELTA_EPOCH_IN_MICROSECS= 11644473600000000;

#define SECS_TO_FT_MULT 10000000

int gettimeofday(struct timeval *tv/*in*/, struct timezone *tz/*in*/)
{
  FILETIME ft;
  __int64 tmpres = 0;
  TIME_ZONE_INFORMATION tz_winapi;
  int rez=0;

  ZeroMemory(&ft,sizeof(ft));
  ZeroMemory(&tz_winapi,sizeof(tz_winapi));

  GetSystemTimeAsFileTime(&ft);

  tmpres = ft.dwHighDateTime;
  tmpres <<= 32;
  tmpres |= ft.dwLowDateTime;

  /*converting file time to unix epoch*/
  tmpres /= 10;  /*convert into microseconds*/
  tmpres -= DELTA_EPOCH_IN_MICROSECS; 
  tv->tv_sec = (__int32)(tmpres*0.000001);
  tv->tv_usec =(tmpres%1000000);


  //_tzset(),don't work properly, so we use GetTimeZoneInformation
  rez=GetTimeZoneInformation(&tz_winapi);
  if (tz)
  {
	  tz->tz_dsttime=(rez==2)? 1 : 0;
	  tz->tz_minuteswest = tz_winapi.Bias + ((rez==2)?tz_winapi.DaylightBias:0);
  }
  return 0;
}

struct tm * localtime_r(const time_t *clock, struct tm *result) {
    *result = *localtime(clock);
    return result;
}

struct tm * gmtime_r(const time_t *clock, struct tm *result) {
    *result = *gmtime(clock);
    return result;
}

// Find 1st Jan 1970 as a FILETIME 
static void get_base_time(LARGE_INTEGER *base_time)
{
	SYSTEMTIME st;
	FILETIME ft;

	memset(&st, 0, sizeof(st));
	st.wYear = 1970;
	st.wMonth = 1;
	st.wDay = 1;
	SystemTimeToFileTime(&st, &ft);

	base_time->LowPart = ft.dwLowDateTime;
	base_time->HighPart = ft.dwHighDateTime;
	base_time->QuadPart /= SECS_TO_FT_MULT;
}

void get_localtime(struct timeval *tv, struct tm  *tm_p)
{
	LARGE_INTEGER li;
	FILETIME ft;
	SYSTEMTIME st;
	static LARGE_INTEGER base_time;
	static char get_base_time_flag = 0;

	if (get_base_time_flag == 0)
	{
		get_base_time(&base_time);
	}
	li.QuadPart = tv->tv_sec;
	li.QuadPart += base_time.QuadPart;
	li.QuadPart *= SECS_TO_FT_MULT;

	ft.dwLowDateTime = li.LowPart;
	ft.dwHighDateTime = li.HighPart;
	FileTimeToSystemTime(&ft, &st);

	tm_p->tm_year = st.wYear - 1900;
	tm_p->tm_mon = st.wMonth - 1;
	tm_p->tm_mday = st.wDay;
	tm_p->tm_wday = st.wDayOfWeek;

	tm_p->tm_hour = st.wHour + 8;
	tm_p->tm_min = st.wMinute;
	tm_p->tm_sec = st.wSecond;
}

#endif

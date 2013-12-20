#include "bdktime.h"
#include <Windows.h>
#include <time.h>

#define __STDC_FORMAT_MACROS
#include "inttypes.h"
#undef __STDC_FORMAT_MACROS

namespace BDK {

#ifdef WIN32
static int gettimeofday(struct timeval* tv, void* tz)
{
    SYSTEMTIME localtm;
    GetLocalTime(&localtm);

    struct tm time;
    time.tm_year   = localtm.wYear - 1900;
    time.tm_mon    = localtm.wMonth - 1;
    time.tm_mday   = localtm.wDay;
    time.tm_hour   = localtm.wHour;
    time.tm_min    = localtm.wMinute;
    time.tm_sec    = localtm.wSecond;
    time.tm_isdst  = -1;

    time_t clock = mktime(&time);
    tv->tv_sec   = clock;
    tv->tv_usec  = localtm.wMilliseconds * 1000;
    return 0;
}
#endif

std::string timestamp::toString() const
{
    char buf[32] = {0};
    int64_t seconds = m_microSecondsSinceEpoch / kMicroSecondsPerSecond;
    int64_t microseconds = m_microSecondsSinceEpoch % kMicroSecondsPerSecond;
    _snprintf_s(buf, sizeof(buf)-1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
    return buf;
}

std::string timestamp::toFormattedString() const
{
    char buf[32] = {0};

    time_t seconds    = static_cast<time_t>(m_microSecondsSinceEpoch / kMicroSecondsPerSecond);
    int microseconds  = static_cast<int>(m_microSecondsSinceEpoch % kMicroSecondsPerSecond);
  //struct tm tm_time = *gmtime(&seconds);  //utc
    struct tm tm_time = *localtime(&seconds); //local time

    _snprintf_s(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d.%06d",
        tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
        tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
        microseconds);
    return buf;
}

timestamp timestamp::now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t seconds = tv.tv_sec;
    return timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

}
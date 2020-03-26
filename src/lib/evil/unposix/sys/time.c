#include<sys/time.h>

int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
    // This number is the number of 100 nanosecond intervals since 00:00:00
    // 01/01/1601 (UTC) (Windows way) until 00:00:00 01/01/1970 (POSIX way)
    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

    SYSTEMTIME system_time;
    FILETIME file_time;
    uint64_t time;

    GetSystemTime(&system_time);
    SystemTimeToFileTime( &system_time, &file_time );
    time =  ((uint64_t)file_time.dwLowDateTime )      ;
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
}

struct tm *localtime_r(const time_t * time, struct tm * result)
{
    localtime_s(result, time);
    return result;
}

#ifndef SYS_TIME_H
#define SYS_TIME_H

#define WIN32_LEAN_AND_MEAN
// Windows Kit for Windows 10 already defines `struct timeval` and `time_t`
#include <winsock2.h>
#include <time.h>

typedef unsigned short u_short;

typedef long suseconds_t;

typedef struct timeval timeval;

inline int gettimeofday(struct timeval * tp, struct timezone * tzp)
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

inline struct tm *localtime_r(const time_t * time, struct tm * result)
{
    localtime_s(result, time);
    return result;
}

#endif

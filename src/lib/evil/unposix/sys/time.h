#ifndef UNPOSIX_SYS_TIME_H
#define UNPOSIX_SYS_TIME_H

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <minwinbase.h>
#include <sysinfoapi.h>
#include <timezoneapi.h>

#include <stdint.h>
#include <time.h>

typedef unsigned short u_short;

struct timezone {
  int tz_minuteswest;     /* minutes west of Greenwich */
  int tz_dsttime;         /* type of DST correction */
};

int gettimeofday(struct timeval * tp, struct timezone * tzp);

struct tm *localtime_r(const time_t * time, struct tm * result);

#endif

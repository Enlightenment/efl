#ifndef SYS_TIME_H
#define SYS_TIME_H

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
// Windows Kit for Windows 10 already defines `struct timeval` and `time_t`
#include <winsock2.h>
#include <time.h>

typedef unsigned short u_short;

struct timezone {
  int tz_minuteswest;     /* minutes west of Greenwich */
  int tz_dsttime;         /* type of DST correction */
};

typedef long suseconds_t;

typedef struct timeval timeval;

int gettimeofday(struct timeval * tp, struct timezone * tzp);

struct tm *localtime_r(const time_t * time, struct tm * result);

#endif

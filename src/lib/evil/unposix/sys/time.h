#ifndef TIME_H
#define TIME_H

#include "unimplemented.h"

// Windows Kit for Windows 10 already defines `struct timeval` and `time_t`
#include <winsock2.h>

typedef unsigned short u_short;

typedef long suseconds_t;

typedef struct timeval timeval;

UNIMPLEMENTED inline int gettimeofday(struct timeval* a, void* b) {
    #warning gettimeofday is not implemented.
    return 0;
}

UNIMPLEMENTED inline struct tm *localtime_r(const time_t * time, struct tm * result)
{
    #warning localtime_r is not implemented.
    return 0;
}

#endif

#ifndef TIME_H
#define TIME_H

//typedef long time_t;

typedef long suseconds_t;

typedef struct timeval timeval;

int gettimeofday(timeval* a, void* b);

#endif

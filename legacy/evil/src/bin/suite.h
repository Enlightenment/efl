#ifndef __EVIL_SUITE_H__
#define __EVIL_SUITE_H__


typedef struct test test;
typedef struct list list;
typedef struct suite suite;

LARGE_INTEGER freq;
LARGE_INTEGER start;
LARGE_INTEGER end;

void suite_time_start();
void suite_time_stop();
double suite_time_get();


#endif /* __EVIL_SUITE_H__ */

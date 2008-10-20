#ifndef __EVIL_SUITE_H__
#define __EVIL_SUITE_H__


typedef struct test test;
typedef struct list list;
typedef struct suite suite;

void suite_time_start(suite *s);
void suite_time_stop(suite *s);
double suite_time_get(suite *s);


#endif /* __EVIL_SUITE_H__ */

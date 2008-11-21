#ifndef __EVIL_PRIVATE_H__
#define __EVIL_PRIVATE_H__


#ifdef __cplusplus
extern "C" {
#endif

long _evil_systemtime_to_time(SYSTEMTIME st);

void _evil_error_display(const char *fct, LONG res);

void _evil_last_error_display(const char *fct);

#ifdef __cplusplus
}
#endif


#endif /* __EVIL_PRIVATE_H__ */

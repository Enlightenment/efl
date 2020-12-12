#ifndef TIMEOUT_H
#define TIMEOUT_H 1

#ifdef __cplusplus
extern "C" {
#endif

void timeout_init(int seconds);
void timeout_func_set(void (*func) (void));

#ifdef __cplusplus
}
#endif

#endif

#ifndef E_UTIL_H
#define E_UTIL_H 1

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define START_LIST_DEL(type, base, cmp) \
type *_p, *_pp; _pp = NULL; _p = (base); while(_p) { if (cmp) { \
if (_pp) _pp->next = _p->next; else (base) = _p->next;
#define END_LIST_DEL \
return; } _pp = _p; _p = _p->next; }

double              e_get_time(void);

#endif

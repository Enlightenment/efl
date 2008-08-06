#ifndef EINA_PERFORMANCE_H_
#define EINA_PERFORMANCE_H_

#include <stdio.h>

#include "eina_types.h"

typedef struct _Eina_Counter Eina_Counter;

EAPI int eina_counter_init(void);
EAPI int eina_counter_shutdown(void);

EAPI Eina_Counter *eina_counter_add(const char *name);
EAPI void eina_counter_delete(Eina_Counter *counter);
EAPI void eina_counter_dump(Eina_Counter *counter, FILE *out);

EAPI void eina_counter_start(Eina_Counter *counter);
EAPI void eina_counter_stop(Eina_Counter *counter, int specimen);

#endif

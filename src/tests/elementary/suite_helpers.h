#ifndef SUITE_HELPERS_H
#define SUITE_HELPERS_H

#include <Evas.h>

int suite_setup(Eina_Bool legacy);
void _elm2_suite_init(void);
void _elm_suite_shutdown(void);
void *real_timer_add(double in, Ecore_Task_Cb cb, void *data);
void fail_on_errors_teardown(void);
void fail_on_errors_setup(void);

#endif

#ifndef SUITE_HELPERS_H
#define SUITE_HELPERS_H

#include <Evas.h>

int suite_setup(Eina_Bool legacy);
void _elm2_suite_init(void);
void _elm_suite_shutdown(void);
void *real_timer_add(double in, Ecore_Task_Cb cb, void *data);
void fail_on_errors_teardown(void);
void fail_on_errors_setup(void);

void get_me_to_those_events(Eo *obj);
void click_object(Eo *obj);
void click_part(Eo *obj, const char *part);
void click_object_at(Eo *obj, int x, int y);
void event_callback_that_is_called_exactly_one_time_and_sets_a_single_int_data_pointer_when_called(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED);
#endif

#ifndef SUITE_HELPERS_H
#define SUITE_HELPERS_H

#include <Evas.h>

#define DRAG_OBJECT_NUM_MOVES 4

int suite_setup(Eina_Bool legacy);
void _elm2_suite_init(void);
void _elm_suite_shutdown(void);
void *real_timer_add(double in, Ecore_Task_Cb cb, void *data);
void fail_on_errors_teardown(void);
void fail_on_errors_setup(void);

void get_me_to_those_events(Eo *obj);
void click_object(Eo *obj);
void click_object_flags(Eo *obj, int flags);
void click_part(Eo *obj, const char *part);
void click_part_flags(Eo *obj, const char *part, int flags);
void click_object_at(Eo *obj, int x, int y);
void click_object_at_flags(Eo *obj, int x, int y, int flags);
void drag_object(Eo *obj, int x, int y, int dx, int dy, Eina_Bool iterate);
void wheel_object(Eo *obj, Eina_Bool horiz, Eina_Bool down);
void wheel_part(Eo *obj, const char *part, Eina_Bool horiz, Eina_Bool down);
void wheel_object_at(Eo *obj, int x, int y, Eina_Bool horiz, Eina_Bool down);
void event_callback_single_call_int_data(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED);
void event_callback_that_quits_the_main_loop_when_called();
void event_callback_that_increments_an_int_when_called(void *data, Evas_Object *obj, void *event_info);
#endif

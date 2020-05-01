#ifndef SUITE_HELPERS_H
#define SUITE_HELPERS_H

#include <Evas.h>

#define DRAG_OBJECT_NUM_MOVES 4
#define DRAG_OBJECT_AROUND_NUM_MOVES 60

int suite_setup(Eina_Bool legacy);
void _elm2_suite_init(void);
void _elm_suite_shutdown(void);
void *real_timer_add(double in, Ecore_Task_Cb cb, void *data);
void wait_timer(double in);
void fail_on_errors_teardown(void);
void fail_on_errors_setup(void);
Eina_Bool is_forked(void);
Eina_Bool is_buffer(void);
void suite_setup_cb_set(void (*cb)(Eo*));

void get_me_to_those_events(Eo *obj);
void click_object(Eo *obj);
void click_object_flags(Eo *obj, int flags);
void click_part(Eo *obj, const char *part);
void click_part_flags(Eo *obj, const char *part, int flags);
void click_object_at(Eo *obj, int x, int y);
void click_object_at_flags(Eo *obj, int x, int y, int flags);
void press_object(Eo *obj);
void press_object_flags(Eo *obj, int flags);
void press_part(Eo *obj, const char *part);
void press_part_flags(Eo *obj, const char *part, int flags);
void press_object_at(Eo *obj, int x, int y);
void press_object_at_flags(Eo *obj, int x, int y, int flags);
void multi_click_object(Eo *obj, int ids);
void multi_press_object(Eo *obj, int ids);
void multi_click_object_at(Eo *obj, int x, int y, int ids);
void multi_press_object_at(Eo *obj, int x, int y, int ids);
int multi_drag_object_around(Eo *obj, int touch_point, int cx, int cy, int radius, int degrees);
void write_key_sequence(Eo *obj, const char *seqence);
void drag_object(Eo *obj, int x, int y, int dx, int dy, Eina_Bool iterate);
int drag_object_around(Eo *obj, int cx, int cy, int radius, int degrees);
int pinch_object(Eo *obj, int x, int y, int x2, int y2, int dx, int dy, int dx2, int dy2);
void wheel_object(Eo *obj, Eina_Bool horiz, Eina_Bool down);
void wheel_part(Eo *obj, const char *part, Eina_Bool horiz, Eina_Bool down);
void wheel_object_at(Eo *obj, int x, int y, Eina_Bool horiz, Eina_Bool down);
void event_callback_single_call_int_data(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED);
void event_callback_that_quits_the_main_loop_when_called();
void event_callback_that_increments_an_int_when_called(void *data, Evas_Object *obj, void *event_info);
void smart_callback_that_stores_event_info_to_data(void *data, Evas_Object *obj EINA_UNUSED, void *event_info);

void force_render(Eo *win);

#define assert_object_size_eq(obj, width, height) \
do \
{ \
   Eina_Size2D _sz = efl_gfx_entity_size_get((obj)); \
   ck_assert_int_eq(_sz.w, (width)); \
   ck_assert_int_eq(_sz.h, (height)); \
} while (0)

#define assert_object_width_eq(obj, width) \
do \
{ \
   Eina_Size2D _sz = efl_gfx_entity_size_get((obj)); \
   ck_assert_int_eq(_sz.w, (width)); \
} while (0)

#define assert_object_height_eq(obj, height) \
do \
{ \
   Eina_Size2D _sz = efl_gfx_entity_size_get((obj)); \
   ck_assert_int_eq(_sz.h, (height)); \
} while (0)

#define assert_object_pos_eq(obj, _x, _y) \
do \
{ \
   Eina_Position2D _pos = efl_gfx_entity_position_get((obj)); \
   ck_assert_int_eq(_pos.x, (_x)); \
   ck_assert_int_eq(_pos.y, (_y)); \
} while (0)
#endif

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Evas.h>
#include <Ecore_Evas.h>

#include "evas_suite.h"
#include "evas_tests_helpers.h"

static void
_mouse_down_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   int *val = data;
   ck_assert_int_eq(*val, 0);
   *val = 1;
}

EFL_START_TEST(evas_test_events_frozen_mouse_up)
{
   Evas *evas;
   Evas_Object *rect;
   static int callback_called = 0;
   evas = EVAS_TEST_INIT_EVAS();
   rect = evas_object_rectangle_add(evas);
   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, &callback_called);
   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_UP, _mouse_down_cb, &callback_called);
   evas_object_resize(rect, 500, 500);
   evas_object_show(rect);

   evas_event_feed_mouse_in(evas, 0, NULL);
   evas_event_feed_mouse_move(evas, 100, 100, 0, NULL);
   evas_event_feed_mouse_down(evas, 1, 0, 0, NULL);
   ck_assert_int_eq(callback_called, 1);
   callback_called = 0;
   evas_object_freeze_events_set(rect, 1);
   evas_event_feed_mouse_up(evas, 1, 0, 0, NULL);
   ck_assert_int_eq(callback_called, 0);
}
EFL_END_TEST

void evas_test_events(TCase *tc)
{
   tcase_add_test(tc, evas_test_events_frozen_mouse_up);
}

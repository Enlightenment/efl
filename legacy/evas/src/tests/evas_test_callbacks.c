
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include "evas_suite.h"
#include "Evas.h"
#include "evas_tests_helpers.h"

#define START_CALLBACK_TEST() \
   Evas *evas; \
   Evas_Object *rect; \
   evas = EVAS_TEST_INIT_EVAS(); \
   rect = evas_object_rectangle_add(evas); \
do \
{ \
} \
while (0)

#define END_CALLBACK_TEST() \
do \
{ \
   evas_object_del(rect); \
   evas_free(evas); \
   evas_shutdown(); \
} \
while (0)

static int counter = 1;

static void
_obj_event_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   (void) e;
   (void) obj;
   (void) event_info;

   fail_if(((int) data) != counter);

   counter++;
}

START_TEST(evas_object_event_callbacks_priority)
{
   START_CALLBACK_TEST();
   counter = 1;
   evas_object_event_callback_priority_add(rect, EVAS_CALLBACK_MOVE, -10,
         _obj_event_cb, (void *) 1);
   evas_object_event_callback_priority_add(rect, EVAS_CALLBACK_MOVE, 0,
         _obj_event_cb, (void *) 2);
   evas_object_event_callback_priority_add(rect, EVAS_CALLBACK_MOVE, 10,
         _obj_event_cb, (void *) 3);
   evas_object_move(rect, 2, 2);

   /* Make sure we got through all the callbacks */
   fail_if(counter != 4);

   /* Delete _obj_event_cb 3 times */
   evas_object_event_callback_del(rect, EVAS_CALLBACK_MOVE, _obj_event_cb);
   evas_object_event_callback_del(rect, EVAS_CALLBACK_MOVE, _obj_event_cb);
   evas_object_event_callback_del(rect, EVAS_CALLBACK_MOVE, _obj_event_cb);
   counter = 1;
   evas_object_event_callback_priority_add(rect, EVAS_CALLBACK_MOVE, 0,
         _obj_event_cb, (void *) 2);
   evas_object_event_callback_priority_add(rect, EVAS_CALLBACK_MOVE, -10,
         _obj_event_cb, (void *) 1);
   evas_object_event_callback_priority_add(rect, EVAS_CALLBACK_MOVE, 10,
         _obj_event_cb, (void *) 3);
   evas_object_move(rect, 3, 3);

   /* Make sure we got through all the callbacks */
   fail_if(counter != 4);

   END_CALLBACK_TEST();
}
END_TEST

static void
_event_cb(void *data, Evas *e, void *event_info)
{
   (void) e;
   (void) event_info;

   fail_if(((int) data) != counter);

   counter++;
}

START_TEST(evas_event_callbacks_priority)
{
   START_CALLBACK_TEST();
   evas_object_focus_set(rect, EINA_FALSE);
   counter = 1;
   evas_event_callback_priority_add(evas, EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN,
         -10, _event_cb, (void *) 1);
   evas_event_callback_priority_add(evas, EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN,
         0, _event_cb, (void *) 2);
   evas_event_callback_priority_add(evas, EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN,
         10, _event_cb, (void *) 3);
   evas_object_focus_set(rect, EINA_TRUE);

   /* Make sure we got through all the callbacks */
   fail_if(counter != 4);

   /* Delete _event_cb 3 times */
   evas_event_callback_del(evas, EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN,
         _event_cb);
   evas_event_callback_del(evas, EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN,
         _event_cb);
   evas_event_callback_del(evas, EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN,
         _event_cb);
   evas_object_focus_set(rect, EINA_FALSE);
   counter = 1;
   evas_event_callback_priority_add(evas, EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN,
         0, _event_cb, (void *) 2);
   evas_event_callback_priority_add(evas, EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN,
         -10, _event_cb, (void *) 1);
   evas_event_callback_priority_add(evas, EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN,
         10, _event_cb, (void *) 3);
   evas_object_focus_set(rect, EINA_TRUE);

   /* Make sure we got through all the callbacks */
   fail_if(counter != 4);

   END_CALLBACK_TEST();
}
END_TEST

void evas_test_callbacks(TCase *tc)
{
   tcase_add_test(tc, evas_object_event_callbacks_priority);
   tcase_add_test(tc, evas_event_callbacks_priority);
}

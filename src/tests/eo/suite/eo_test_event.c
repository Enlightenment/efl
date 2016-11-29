#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Eo.h>

#include "eo_suite.h"
#include "eo_test_class_simple.h"

//Class definition with one event

EWAPI const Efl_Class *efl_test_event_class_get(void);

EWAPI extern const Efl_Event_Description _EFL_TEST_EVENT_EVENT_TESTER;

#define EFL_TEST_EVENT_EVENT_TESTER (&(_EFL_TEST_EVENT_EVENT_TESTER))

typedef struct {
   Eina_Bool event1;
   Eina_Bool event2;
   Eina_Bool event3;
} Test_Data;

typedef struct {
   int not_empty;
} Efl_Test_Event_Data;


static void
_cb3(void *data, const Efl_Event *event)
{
   Test_Data *d = data;

   d->event3 = EINA_TRUE;
}

static void
_cb2(void *data, const Efl_Event *event)
{
   Test_Data *d = data;

   d->event2 = EINA_TRUE;
}

static void
_cb1(void *data, const Efl_Event *event)
{
   Test_Data *d = data;

   d->event1 = EINA_TRUE;

   efl_event_callback_add(event->object, EFL_TEST_EVENT_EVENT_TESTER, _cb3, data);
}

START_TEST(eo_event)
{
   Test_Data data;
   efl_object_init();
   Eo *obj;

   memset(&data, 0, sizeof(Test_Data));

   obj = efl_add(efl_test_event_class_get(), NULL);
   efl_event_callback_priority_add(obj, EFL_TEST_EVENT_EVENT_TESTER, EFL_CALLBACK_PRIORITY_BEFORE, _cb2, &data);
   efl_event_callback_priority_add(obj, EFL_TEST_EVENT_EVENT_TESTER, EFL_CALLBACK_PRIORITY_BEFORE, _cb1, &data);
   efl_event_callback_call(obj, EFL_TEST_EVENT_EVENT_TESTER, NULL);

   ck_assert_int_ne(data.event1, 0);
   ck_assert_int_ne(data.event2, 0);
   ck_assert_int_ne(data.event3, 0);

   efl_object_shutdown();
}
END_TEST

void eo_test_event(TCase *tc)
{
   tcase_add_test(tc, eo_event);
}

//class implementation

EWAPI const Efl_Event_Description _EFL_TEST_EVENT_EVENT_TESTER =
   EFL_EVENT_DESCRIPTION("tester");

static const Efl_Class_Description _efl_test_event_class_desc = {
   EO_VERSION,
   "Efl_Test_Event",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Efl_Test_Event_Data),
   NULL,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(efl_test_event_class_get, &_efl_test_event_class_desc, EFL_OBJECT_CLASS, NULL);

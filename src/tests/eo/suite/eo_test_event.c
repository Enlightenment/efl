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
EWAPI extern const Efl_Event_Description _EFL_TEST_EVENT_EVENT_TESTER_SUBSCRIBE;
EWAPI extern const Efl_Event_Description _EFL_TEST_EVENT_EVENT_TESTER_CLAMP_TEST;

#define EFL_TEST_EVENT_EVENT_TESTER (&(_EFL_TEST_EVENT_EVENT_TESTER))
#define EFL_TEST_EVENT_EVENT_TESTER_SUBSCRIBE (&(_EFL_TEST_EVENT_EVENT_TESTER_SUBSCRIBE))
#define EFL_TEST_EVENT_EVENT_TESTER_CLAMP_TEST (&(_EFL_TEST_EVENT_EVENT_TESTER_CLAMP_TEST))

typedef struct {
   Eina_Bool event1;
   Eina_Bool event2;
   Eina_Bool event3;
} Test_Data;

typedef struct {
   int not_empty;
} Efl_Test_Event_Data;

static void
_cb3(void *data, const Efl_Event *event EINA_UNUSED)
{
   Test_Data *d = data;

   d->event3 = EINA_TRUE;
}

static void
_cb2(void *data, const Efl_Event *event EINA_UNUSED)
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

EFL_START_TEST(eo_event)
{
   Test_Data data;
   Eo *obj;

   obj = efl_add_ref(efl_test_event_class_get(), NULL);
   efl_event_callback_priority_add(obj, EFL_TEST_EVENT_EVENT_TESTER, EFL_CALLBACK_PRIORITY_BEFORE, _cb2, &data);
   efl_event_callback_priority_add(obj, EFL_TEST_EVENT_EVENT_TESTER, EFL_CALLBACK_PRIORITY_BEFORE, _cb1, &data);

   memset(&data, 0, sizeof(Test_Data));
   efl_event_callback_call(obj, EFL_TEST_EVENT_EVENT_TESTER, NULL);
   ck_assert_int_ne(data.event1, 0);
   ck_assert_int_ne(data.event2, 0);
   ck_assert_int_eq(data.event3, 0);

   memset(&data, 0, sizeof(Test_Data));
   efl_event_callback_call(obj, EFL_TEST_EVENT_EVENT_TESTER, NULL);
   ck_assert_int_ne(data.event1, 0);
   ck_assert_int_ne(data.event2, 0);
   ck_assert_int_ne(data.event3, 0);

}
EFL_END_TEST

static void
_cb_rec_3(void *data EINA_UNUSED, const Efl_Event *event)
{
   Test_Data *d = event->info;
   ck_assert_int_eq(d->event3, 0);
   d->event3 = EINA_TRUE;
}

static void
_cb_rec_2(void *data EINA_UNUSED, const Efl_Event *event)
{
   Test_Data *d = event->info;
   ck_assert_int_eq(d->event2, 0);
   d->event2 = EINA_TRUE;
}

static void
_cb_rec_1(void *data, const Efl_Event *event)
{
   Test_Data *d = event->info;

   if (event->info)
     {
        ck_assert_int_eq(d->event1, 0);
        d->event1 = EINA_TRUE;
     }
   else
     {
        efl_event_callback_add(event->object , EFL_TEST_EVENT_EVENT_TESTER, _cb_rec_2, NULL);
        efl_event_callback_add(event->object , EFL_TEST_EVENT_EVENT_TESTER, _cb_rec_3, NULL);
        efl_event_callback_call(event->object, EFL_TEST_EVENT_EVENT_TESTER, data);
     }
}

EFL_START_TEST(eo_event_call_in_call)
{
   Test_Data data;
   Eo *obj;

   obj = efl_add_ref(efl_test_event_class_get(), NULL);
   efl_event_callback_priority_add(obj, EFL_TEST_EVENT_EVENT_TESTER, EFL_CALLBACK_PRIORITY_BEFORE, _cb_rec_1, &data);

   memset(&data, 0, sizeof(Test_Data));
   efl_event_callback_call(obj, EFL_TEST_EVENT_EVENT_TESTER, NULL);
   ck_assert_int_ne(data.event1, 0);
   ck_assert_int_ne(data.event2, 0);
   ck_assert_int_ne(data.event3, 0);

}
EFL_END_TEST

static Eina_Bool emitted = 0;

static void
_generation_clamp_step3(void *data EINA_UNUSED, const Efl_Event *e EINA_UNUSED)
{
   emitted = 1;
}

static void
_generation_clamp_subscribe(void *data EINA_UNUSED, const Efl_Event *e)
{
   //generation is 2
   efl_event_callback_add(e->object, EFL_TEST_EVENT_EVENT_TESTER_CLAMP_TEST, _generation_clamp_step3, NULL);
}

static void
_generation_clamp_step1(void *data EINA_UNUSED, const Efl_Event *e)
{
   //generation is 1
   efl_event_callback_call(e->object, EFL_TEST_EVENT_EVENT_TESTER_SUBSCRIBE, NULL);

   efl_event_callback_call(e->object, EFL_TEST_EVENT_EVENT_TESTER_CLAMP_TEST, NULL);
   efl_event_callback_call(e->object, EFL_TEST_EVENT_EVENT_TESTER_CLAMP_TEST, NULL);
}


EFL_START_TEST(eo_event_generation_bug)
{

   /*
    * The idea is:
    *
    * #1 a event gets emitted (generation is 1)
    * #2 a event gets emitted as a result of #1 (generation is 2)
    * in a callback from #2 a new subscription for E is added (S) (generation of it is 2)
    * in a callback of #1 event E is emitted (generation is 2)
    * S now MUST get executed (Here is the bug generation of S is 2 and of emission is 2, event gets skipped)
    * subscription adds a callback to a event
    */

   Eo *obj;

   obj = efl_add_ref(efl_test_event_class_get(), NULL);
   emitted = 0;
   efl_event_callback_priority_add(obj, EFL_TEST_EVENT_EVENT_TESTER, EFL_CALLBACK_PRIORITY_BEFORE, _generation_clamp_step1, NULL);
   efl_event_callback_priority_add(obj, EFL_TEST_EVENT_EVENT_TESTER_SUBSCRIBE, EFL_CALLBACK_PRIORITY_BEFORE, _generation_clamp_subscribe, NULL);
   efl_event_callback_call(obj, EFL_TEST_EVENT_EVENT_TESTER, NULL);

   ck_assert_int_ne(emitted, 0);

}
EFL_END_TEST

#include "eo_event_emitter.h"

static void
_test(void *data, const Efl_Event *info)
{
   Eina_Bool *flag = data;

   *flag = EINA_TRUE;
}


static void
_nop(void *data, const Efl_Event *info)
{

}

static void
_delete_in_ev(void *data, const Efl_Event *info)
{
   eo_event_emitter_unregister(data, _nop, EFL_EVENT_DEL, NULL);
}

EFL_START_TEST(eo_event_basic)
{
   Eo_Event_Emitter emitter;
   Eina_Bool flag = EINA_FALSE;
   Efl_Event ev = { NULL, EFL_EVENT_DEL, NULL};

   eo_event_emitter_init(&emitter);
   eo_event_emitter_register(&emitter, _test, EFL_EVENT_DEL, 0, &flag);
   eo_event_emitter_emit(&emitter, &ev, EINA_TRUE);

   ck_assert_int_eq(flag, EINA_TRUE);
   flag = EINA_FALSE;

   eo_event_emitter_unregister(&emitter, _test, EFL_EVENT_DEL, &flag);
   eo_event_emitter_emit(&emitter, &ev, EINA_TRUE);

   ck_assert_int_eq(flag, EINA_FALSE);
}
EFL_END_TEST

EFL_START_TEST(eo_event_basic_del)
{
   Eo_Event_Emitter emitter;
   Efl_Event ev = { NULL, EFL_EVENT_DEL, NULL};

   eo_event_emitter_init(&emitter);
   eo_event_emitter_register(&emitter, _nop, EFL_EVENT_DEL, 0, NULL);
   eo_event_emitter_register(&emitter, _delete_in_ev, EFL_EVENT_DEL, 0, &emitter);
   eo_event_emitter_register(&emitter, _nop, EFL_EVENT_DEL, 0, NULL);
   eo_event_emitter_emit(&emitter, &ev, EINA_TRUE);
}
EFL_END_TEST

static int counter;

#define TEST_FUNC(name) \
static void \
name(void *data, const Efl_Event *info) \
{ \
   int *flag = data; \
   *flag = counter; \
   counter++; \
}
TEST_FUNC(_ncb1)
TEST_FUNC(_ncb2)
TEST_FUNC(_ncb3)

EFL_START_TEST(eo_event_basic_order)
{
   Eo_Event_Emitter emitter;
   Efl_Event ev = { NULL, EFL_EVENT_DEL, NULL};
   int flag[] = { 0,0,0,0,0,0,0,0,0 };

   eo_event_emitter_init(&emitter);
   eo_event_emitter_register(&emitter, _ncb1, EFL_EVENT_DEL, EFL_CALLBACK_PRIORITY_BEFORE, &flag[2]);
   eo_event_emitter_register(&emitter, _ncb2, EFL_EVENT_DEL, EFL_CALLBACK_PRIORITY_BEFORE, &flag[1]);
   eo_event_emitter_register(&emitter, _ncb3, EFL_EVENT_DEL, EFL_CALLBACK_PRIORITY_BEFORE, &flag[0]);
   eo_event_emitter_register(&emitter, _ncb1, EFL_EVENT_DEL, 0, &flag[5]);
   eo_event_emitter_register(&emitter, _ncb2, EFL_EVENT_DEL, 0, &flag[4]);
   eo_event_emitter_register(&emitter, _ncb3, EFL_EVENT_DEL, 0, &flag[3]);
   eo_event_emitter_register(&emitter, _ncb1, EFL_EVENT_DEL, EFL_CALLBACK_PRIORITY_AFTER, &flag[8]);
   eo_event_emitter_register(&emitter, _ncb2, EFL_EVENT_DEL, EFL_CALLBACK_PRIORITY_AFTER, &flag[7]);
   eo_event_emitter_register(&emitter, _ncb3, EFL_EVENT_DEL, EFL_CALLBACK_PRIORITY_AFTER, &flag[6]);

   counter = 0;

   eo_event_emitter_emit(&emitter, &ev, EINA_TRUE);

   for (int i = 0; i < 9; ++i)
     {
        ck_assert_int_eq(flag[i], i);
     }
}
EFL_END_TEST

void eo_test_event(TCase *tc)
{
   tcase_add_test(tc, eo_event);
   tcase_add_test(tc, eo_event_call_in_call);
   tcase_add_test(tc, eo_event_generation_bug);
   tcase_add_test(tc, eo_event_basic);
   tcase_add_test(tc, eo_event_basic_order);
}



//class implementation

EWAPI const Efl_Event_Description _EFL_TEST_EVENT_EVENT_TESTER =
   EFL_EVENT_DESCRIPTION("tester");

EWAPI const Efl_Event_Description _EFL_TEST_EVENT_EVENT_TESTER_SUBSCRIBE =
   EFL_EVENT_DESCRIPTION("tester");

EWAPI const Efl_Event_Description _EFL_TEST_EVENT_EVENT_TESTER_CLAMP_TEST =
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

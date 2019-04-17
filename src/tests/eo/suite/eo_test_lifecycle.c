#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Eo.h>

#include "eo_suite.h"
#include "eo_test_class_simple.h"

EFL_START_TEST(eo_test_base_del)
{
   Eo *par = efl_add_ref(SIMPLE_CLASS, NULL);
   Eo *obj = efl_add(SIMPLE_CLASS, par);

   efl_del(obj);

   ck_assert_ptr_eq(efl_class_name_get(obj), NULL);

}
EFL_END_TEST

EFL_START_TEST(eo_test_base_unref)
{
   Eo *obj = efl_add_ref(SIMPLE_CLASS, NULL);

   efl_unref(obj);

   ck_assert_ptr_eq(efl_class_name_get(obj), NULL);

}
EFL_END_TEST

typedef struct {
   unsigned int time, del_time, invalidate_time, noref_time, destruct_time;
} Helper;

static void
_invalidate(void *data, const Efl_Event *ev)
{
   Helper *help = data;

   ck_assert_ptr_ne(efl_parent_get(ev->object), NULL);

   help->invalidate_time = help->time;
   help->time ++;
}

static void
_noref(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Helper *help = data;

   help->noref_time = help->time;
   help->time ++;
}

static void
_del(void *data, const Efl_Event *ev)
{
   Helper *help = data;

   ck_assert_ptr_eq(efl_parent_get(ev->object), NULL);

   help->del_time = help->time;
   help->time ++;
}

static void
_destruct(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Helper *help = data;

   help->destruct_time = help->time;
   help->time ++;
}

EFL_START_TEST(eo_test_shutdown_eventting)
{
   Eo *par = efl_add_ref(SIMPLE_CLASS, NULL);
   Eo *obj = efl_add(SIMPLE_CLASS, par);
   Helper data;

   data.time = 1;
   data.del_time = 0;
   data.invalidate_time = 0;
   data.noref_time = 0;
   data.destruct_time = 0;

   efl_event_callback_add(obj, EFL_EVENT_DEL, _del, &data);
   efl_event_callback_add(obj, EFL_EVENT_INVALIDATE, _invalidate, &data);
   efl_event_callback_add(obj, EFL_EVENT_NOREF, _noref, &data);
   efl_event_callback_add(obj, EFL_EVENT_DESTRUCT, _destruct, &data);

   efl_del(obj);

   //documented assertions:
   ck_assert_int_ne(data.del_time, 0);
   ck_assert_int_ne(data.invalidate_time, 0);
   ck_assert_int_ne(data.noref_time, 0);
   ck_assert_int_ne(data.destruct_time, 0);

   //everything happened before destruct
   ck_assert_int_lt(data.del_time, data.destruct_time);
   ck_assert_int_lt(data.invalidate_time, data.destruct_time);
   ck_assert_int_lt(data.noref_time, data.destruct_time);

   //invalidate has to happen before the destructor
   ck_assert_int_lt(data.invalidate_time, data.destruct_time);

   ck_assert_ptr_eq(efl_class_name_get(obj), NULL);

}
EFL_END_TEST

static void
_noref2(void *data EINA_UNUSED, const Efl_Event *ev)
{
   //correctly delete this
   efl_del(ev->object);
   //error unref
   efl_unref(ev->object);
}

EFL_START_TEST(eo_test_del_in_noref)
{
   Eo *par = efl_add_ref(SIMPLE_CLASS, NULL);
   Eo *obj = efl_add(SIMPLE_CLASS, par);

   efl_event_callback_add(obj, EFL_EVENT_NOREF, _noref2, NULL);

   efl_ref(obj);
   DISABLE_ABORT_ON_CRITICAL_START;
   efl_unref(obj); //this fires noref
   DISABLE_ABORT_ON_CRITICAL_END;

   ck_assert_ptr_eq(efl_class_name_get(obj), NULL);

}
EFL_END_TEST

static void
_noref3(void *data EINA_UNUSED, const Efl_Event *ev)
{
   //this is a wrong fixme
   efl_unref(ev->object);
}

EFL_START_TEST(eo_test_unref_noref)
{
   Eo *obj = efl_add_ref(SIMPLE_CLASS, NULL);

   efl_event_callback_add(obj, EFL_EVENT_NOREF, _noref3, NULL);

   DISABLE_ABORT_ON_CRITICAL_START;
   efl_unref(obj);
   DISABLE_ABORT_ON_CRITICAL_END;

   ck_assert_ptr_eq(efl_class_name_get(obj), NULL);

}
EFL_END_TEST

typedef struct {
   int shared, unique, invalidate;
} OwnershipEventsCounter;

static void
_ownership_shared_event(void *data, const Efl_Event *ev EINA_UNUSED)
{
   OwnershipEventsCounter *counter = data;
   ++(counter->shared);
}

static void
_ownership_unique_event(void *data, const Efl_Event *ev EINA_UNUSED)
{
   OwnershipEventsCounter *counter = data;
   ++(counter->unique);
}

static void
_invalidate_ownership_event(void *data, const Efl_Event *ev EINA_UNUSED)
{
   OwnershipEventsCounter *counter = data;
   ++(counter->invalidate);
}


EFL_START_TEST(eo_test_ownership_events)
{
   OwnershipEventsCounter counter = {0,};
   Eo *obj = efl_add_ref(SIMPLE_CLASS, NULL);

   efl_event_callback_add(obj, EFL_EVENT_OWNERSHIP_SHARED, _ownership_shared_event, &counter);
   efl_event_callback_add(obj, EFL_EVENT_OWNERSHIP_UNIQUE, _ownership_unique_event, &counter);
   efl_event_callback_add(obj, EFL_EVENT_INVALIDATE, _invalidate_ownership_event, &counter);

   efl_ref(obj);
   ck_assert_int_eq(counter.shared, 1);
   ck_assert_int_eq(counter.unique, 0);

   efl_unref(obj);
   ck_assert_int_eq(counter.shared, 1);
   ck_assert_int_eq(counter.unique, 1);

   efl_ref(obj);
   ck_assert_int_eq(counter.shared, 2);
   ck_assert_int_eq(counter.unique, 1);
   efl_ref(obj);
   ck_assert_int_eq(counter.shared, 2);
   ck_assert_int_eq(counter.unique, 1);
   efl_ref(obj);
   ck_assert_int_eq(counter.shared, 2);
   ck_assert_int_eq(counter.unique, 1);

   efl_unref(obj);
   ck_assert_int_eq(counter.shared, 2);
   ck_assert_int_eq(counter.unique, 1);
   efl_unref(obj);
   ck_assert_int_eq(counter.shared, 2);
   ck_assert_int_eq(counter.unique, 1);
   efl_unref(obj);
   ck_assert_int_eq(counter.shared, 2);
   ck_assert_int_eq(counter.unique, 2);
   ck_assert_int_eq(counter.invalidate, 0);

   efl_unref(obj);
   ck_assert_int_eq(counter.shared, 2);
   ck_assert_int_eq(counter.unique, 2);
   ck_assert_int_eq(counter.invalidate, 1);
}
EFL_END_TEST

EFL_START_TEST(eo_test_ownership_events_with_parent)
{
   OwnershipEventsCounter counter = {0,};
   Eo *par = efl_add_ref(SIMPLE_CLASS, NULL);
   Eo *obj = efl_add(SIMPLE_CLASS, par);

   efl_event_callback_add(obj, EFL_EVENT_OWNERSHIP_SHARED, _ownership_shared_event, &counter);
   efl_event_callback_add(obj, EFL_EVENT_OWNERSHIP_UNIQUE, _ownership_unique_event, &counter);
   efl_event_callback_add(obj, EFL_EVENT_INVALIDATE, _invalidate_ownership_event, &counter);

   efl_ref(obj);
   ck_assert_int_eq(counter.shared, 1);
   ck_assert_int_eq(counter.unique, 0);

   efl_unref(obj);
   ck_assert_int_eq(counter.shared, 1);
   ck_assert_int_eq(counter.unique, 1);

   efl_ref(obj);
   ck_assert_int_eq(counter.shared, 2);
   ck_assert_int_eq(counter.unique, 1);
   efl_ref(obj);
   ck_assert_int_eq(counter.shared, 2);
   ck_assert_int_eq(counter.unique, 1);

   efl_unref(obj);
   ck_assert_int_eq(counter.shared, 2);
   ck_assert_int_eq(counter.unique, 1);
   efl_unref(obj);
   ck_assert_int_eq(counter.shared, 2);
   ck_assert_int_eq(counter.unique, 2);
   ck_assert_int_eq(counter.invalidate, 0);

   efl_del(obj);
   ck_assert_int_eq(counter.shared, 2);
   ck_assert_int_eq(counter.unique, 2);
   ck_assert_int_eq(counter.invalidate, 1);

   efl_unref(par);
   ck_assert_int_eq(counter.shared, 2);
   ck_assert_int_eq(counter.unique, 2);
}
EFL_END_TEST

EFL_START_TEST(eo_test_ownership_events_with_parent_invalidate)
{
   OwnershipEventsCounter counter = {0,};
   Eo *par = efl_add_ref(SIMPLE_CLASS, NULL);
   Eo *obj = efl_add(SIMPLE_CLASS, par);

   efl_event_callback_add(obj, EFL_EVENT_OWNERSHIP_SHARED, _ownership_shared_event, &counter);
   efl_event_callback_add(obj, EFL_EVENT_OWNERSHIP_UNIQUE, _ownership_unique_event, &counter);
   efl_event_callback_add(obj, EFL_EVENT_INVALIDATE, _invalidate_ownership_event, &counter);

   /* Kill parent */
   efl_unref(par);
   ck_assert_int_eq(counter.shared, 0);
   ck_assert_int_eq(counter.unique, 0);
   ck_assert_int_eq(counter.invalidate, 1);
}
EFL_END_TEST

EFL_START_TEST(eo_test_ownership_events_with_parent_invalidate2)
{
   OwnershipEventsCounter counter = {0,};
   Eo *par = efl_add_ref(SIMPLE_CLASS, NULL);
   Eo *obj = efl_add(SIMPLE_CLASS, par);

   efl_event_callback_add(obj, EFL_EVENT_OWNERSHIP_SHARED, _ownership_shared_event, &counter);
   efl_event_callback_add(obj, EFL_EVENT_OWNERSHIP_UNIQUE, _ownership_unique_event, &counter);
   efl_event_callback_add(obj, EFL_EVENT_INVALIDATE, _invalidate_ownership_event, &counter);

   efl_ref(obj);
   ck_assert_int_eq(counter.shared, 1);
   ck_assert_int_eq(counter.unique, 0);
   ck_assert_int_eq(counter.invalidate, 0);

   /* Kill parent */
   efl_unref(par);
   ck_assert_int_eq(counter.shared, 1);
   ck_assert_int_eq(counter.unique, 0);
   ck_assert_int_eq(counter.invalidate, 1);
}
EFL_END_TEST

EFL_START_TEST(eo_test_ownership_events_with_parent_invalidate3)
{
   OwnershipEventsCounter counter = {0,};
   Eo *par = efl_add_ref(SIMPLE_CLASS, NULL);
   Eo *obj = efl_add(SIMPLE_CLASS, par);

   efl_event_callback_add(obj, EFL_EVENT_OWNERSHIP_SHARED, _ownership_shared_event, &counter);
   efl_event_callback_add(obj, EFL_EVENT_OWNERSHIP_UNIQUE, _ownership_unique_event, &counter);
   efl_event_callback_add(obj, EFL_EVENT_INVALIDATE, _invalidate_ownership_event, &counter);

   efl_ref(obj);
   ck_assert_int_eq(counter.shared, 1);
   ck_assert_int_eq(counter.unique, 0);

   efl_unref(obj);
   ck_assert_int_eq(counter.shared, 1);
   ck_assert_int_eq(counter.unique, 1);

   efl_ref(obj);
   ck_assert_int_eq(counter.shared, 2);
   ck_assert_int_eq(counter.unique, 1);
   efl_ref(obj);
   ck_assert_int_eq(counter.shared, 2);
   ck_assert_int_eq(counter.unique, 1);
   ck_assert_int_eq(counter.invalidate, 0);

   /* Kill parent */
   efl_unref(par);
   ck_assert_int_eq(counter.shared, 2);
   ck_assert_int_eq(counter.unique, 1);
   ck_assert_int_eq(counter.invalidate, 1);
}
EFL_END_TEST

EFL_START_TEST(eo_test_ownership_events_self_invalidate)
{
   OwnershipEventsCounter counter = {0,};
   Eo *par = efl_add_ref(SIMPLE_CLASS, NULL);
   Eo *obj = efl_add(SIMPLE_CLASS, par);

   efl_event_callback_add(obj, EFL_EVENT_OWNERSHIP_SHARED, _ownership_shared_event, &counter);
   efl_event_callback_add(obj, EFL_EVENT_OWNERSHIP_UNIQUE, _ownership_unique_event, &counter);
   efl_event_callback_add(obj, EFL_EVENT_INVALIDATE, _invalidate_ownership_event, &counter);

   ck_assert_int_eq(counter.shared, 0);
   ck_assert_int_eq(counter.unique, 0);
   ck_assert_int_eq(counter.invalidate, 0);

   efl_ref(obj);
   ck_assert_int_eq(counter.shared, 1);
   ck_assert_int_eq(counter.unique, 0);
   ck_assert_int_eq(counter.invalidate, 0);

   efl_del(obj);
   ck_assert_int_eq(counter.shared, 1);
   ck_assert_int_eq(counter.unique, 0);
   ck_assert_int_eq(counter.invalidate, 1);

   /* Kill parent */
   efl_unref(par);
   ck_assert_int_eq(counter.shared, 1);
   ck_assert_int_eq(counter.unique, 0);
   ck_assert_int_eq(counter.invalidate, 1);
}
EFL_END_TEST

typedef struct {
  Eo *par;
  Eina_Bool called;
} Invalidating_Test_Helper;

static void
_invalidate2(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Invalidating_Test_Helper *iev = data;

   iev->called = EINA_TRUE;
   ck_assert_int_eq(efl_invalidating_get(iev->par), EINA_TRUE);
   ck_assert_int_eq(efl_invalidated_get(iev->par), EINA_FALSE);
}

EFL_START_TEST(eo_test_invalidating_get)
{
   Eo *par = efl_add_ref(SIMPLE_CLASS, NULL);
   Eo *obj = efl_add(SIMPLE_CLASS, par);
   Invalidating_Test_Helper data = {par, EINA_FALSE};

   efl_event_callback_add(obj, EFL_EVENT_INVALIDATE, _invalidate2, &data);

   efl_unref(par);
   ck_assert_int_eq(data.called, EINA_TRUE);
}
EFL_END_TEST

EFL_START_TEST(eo_test_alive_get)
{
   Eina_Bool res;
   Eo *par = efl_add_ref(SIMPLE_CLASS, NULL);
   Eo *obj = efl_add(SIMPLE_CLASS, par,
      res = efl_alive_get(efl_added)
   );
   ck_assert_int_eq(res, 0);
   ck_assert_int_eq(efl_alive_get(obj), 1);
   efl_ref(obj);
   ck_assert_int_eq(efl_alive_get(obj), 1);
   efl_del(obj);
   ck_assert_int_eq(efl_alive_get(obj), 0);
   efl_unref(obj);
   ck_assert_ptr_eq(efl_class_name_get(obj), NULL);

}
EFL_END_TEST

void eo_test_lifecycle(TCase *tc)
{
   tcase_add_test(tc, eo_test_base_del);
   tcase_add_test(tc, eo_test_base_unref);
   tcase_add_test(tc, eo_test_shutdown_eventting);
   tcase_add_test(tc, eo_test_del_in_noref);
   tcase_add_test(tc, eo_test_unref_noref);
   tcase_add_test(tc, eo_test_ownership_events);
   tcase_add_test(tc, eo_test_ownership_events_with_parent);
   tcase_add_test(tc, eo_test_ownership_events_with_parent_invalidate);
   tcase_add_test(tc, eo_test_ownership_events_with_parent_invalidate2);
   tcase_add_test(tc, eo_test_ownership_events_with_parent_invalidate3);
   tcase_add_test(tc, eo_test_ownership_events_self_invalidate);
   tcase_add_test(tc, eo_test_invalidating_get);
   tcase_add_test(tc, eo_test_alive_get);
}

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Evas.h>

#include "evas_suite.h"
#include "evas_tests_helpers.h"

EFL_START_TEST(evas_object_various)
{
   Evas *evas = EVAS_TEST_INIT_EVAS();

   Evas_Object *obj = evas_object_rectangle_add(evas);

   evas_object_ref(obj);
   evas_free(evas);
   evas_object_unref(obj);

   evas = EVAS_TEST_INIT_EVAS();
   obj = evas_object_rectangle_add(evas);
   efl_ref(obj);
   evas_free(evas);
   efl_unref(obj);


   /* Twice because EVAS_TEST_INIT_EVAS inits it twice. */
}
EFL_END_TEST

EFL_START_TEST(evas_object_freeze_events)
{
   Evas *evas = EVAS_TEST_INIT_EVAS();
   Evas_Object *obj = evas_object_rectangle_add(evas);

   evas_object_freeze_events_set(obj, EINA_TRUE);
   ck_assert_int_eq(evas_object_freeze_events_get(obj), EINA_TRUE);
   evas_object_freeze_events_set(obj, EINA_FALSE);
   ck_assert_int_eq(evas_object_freeze_events_get(obj), EINA_FALSE);

   evas_object_freeze_events_set(obj, EINA_TRUE);
   evas_object_freeze_events_set(obj, EINA_TRUE);
   ck_assert_int_eq(evas_object_freeze_events_get(obj), EINA_TRUE);
   evas_object_freeze_events_set(obj, EINA_FALSE);
   ck_assert_int_eq(evas_object_freeze_events_get(obj), EINA_FALSE);
   evas_object_freeze_events_set(obj, EINA_FALSE);
   evas_object_freeze_events_set(obj, EINA_TRUE);
   ck_assert_int_eq(evas_object_freeze_events_get(obj), EINA_TRUE);
}
EFL_END_TEST

void evas_test_object(TCase *tc)
{
   tcase_add_test(tc, evas_object_various);
   tcase_add_test(tc, evas_object_freeze_events);
}

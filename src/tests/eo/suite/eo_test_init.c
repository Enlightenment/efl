#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eo.h>

#include "eo_suite.h"
#include "eo_test_class_simple.h"

EFL_START_TEST(eo_test_simple)
{
   fail_if(!efl_object_init()); /* one init by test suite */
   fail_if(!efl_object_shutdown());
}
EFL_END_TEST

EFL_START_TEST(eo_test_init_shutdown)
{
   Eo *obj;

   ck_assert_str_eq("Efl.Object", efl_class_name_get(EFL_OBJECT_CLASS));

   /* XXX-1: Essential for the next test to assign the wrong op. */
   obj = efl_add_ref(SIMPLE_CLASS, NULL);
   simple_a_set(obj, 1);
   ck_assert_int_eq(1, simple_a_get(obj));

   /* XXX-1: Essential for the next test to cache the op. */
   ck_assert_int_eq(0xBEEF, simple2_class_beef_get(SIMPLE2_CLASS));
   efl_unref(obj);
   fail_if(efl_object_shutdown());

   fail_if(!efl_object_init());
   ck_assert_str_eq("Efl.Object", efl_class_name_get(EFL_OBJECT_CLASS));

   /* XXX-1: Verify that the op was not cached. */
   ck_assert_int_eq(0xBEEF, simple2_class_beef_get(SIMPLE2_CLASS));
}
EFL_END_TEST

void eo_test_init(TCase *tc)
{
   tcase_add_test(tc, eo_test_simple);
   tcase_add_test(tc, eo_test_init_shutdown);
}

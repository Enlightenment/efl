#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Eo.h>

#include "eo_suite.h"
#include "eo_test_class_simple.h"


EFL_START_TEST(eo_test_reflection_invalid)
{
   Eina_Value numb_val = eina_value_int_init(1337);
   Eo *simple = efl_new(SIMPLE_CLASS);

   simple_a_set(simple, 22);
   efl_property_reflection_set(simple, "simple_a_asdf", numb_val);
   fail_if(efl_property_reflection_get(simple, "simple_a_invalid").type != EINA_VALUE_EMPTY.type);
}
EFL_END_TEST

EFL_START_TEST(eo_test_reflection_inherited)
{
   const int numb = 42;
   int number_ref;
   Eina_Value numb_val = eina_value_int_init(numb);
   Eo *simple = efl_new(SIMPLE3_CLASS);

   simple_a_set(simple, 22);
   efl_property_reflection_set(simple, "simple_a", numb_val);
   ck_assert_int_eq(simple_a_get(simple), numb);

   simple_a_set(simple, 22);
   Eina_Value res = efl_property_reflection_get(simple, "simple_a");
   eina_value_int_convert(&res, &number_ref);
   ck_assert_int_eq(number_ref, 22);

}
EFL_END_TEST

EFL_START_TEST(eo_test_reflection_simple)
{
   const int numb = 42;
   int number_ref;
   Eina_Value numb_val = eina_value_int_init(numb);
   Eina_Value useless_val = eina_value_int_init(7);
   Eo *simple = efl_new(SIMPLE_CLASS);

   simple_a_set(simple, 22);
   efl_property_reflection_set(simple, "simple_a", numb_val);
   ck_assert_int_eq(simple_a_get(simple), numb);

   ck_assert_int_eq(efl_property_reflection_set(simple, "should_fail", useless_val),
                    EINA_ERROR_NOT_IMPLEMENTED);

   simple_a_set(simple, 22);
   Eina_Value res = efl_property_reflection_get(simple, "simple_a");
   eina_value_int_convert(&res, &number_ref);
   ck_assert_int_eq(number_ref, 22);
}
EFL_END_TEST

void eo_test_reflection(TCase *tc)
{
   tcase_add_test(tc, eo_test_reflection_simple);
   tcase_add_test(tc, eo_test_reflection_inherited);
   tcase_add_test(tc, eo_test_reflection_invalid);
}

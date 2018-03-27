#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include <check.h>
#include "eina_suite.h"

EFL_START_TEST(eina_test_vpath_valid)
{
   char test[PATH_MAX];

   ck_assert_str_eq(eina_vpath_resolve("/"), "/");
   ck_assert_str_eq(eina_vpath_resolve("./"), "./");
   ck_assert_str_eq(eina_vpath_resolve("..bla"), "..bla");
   ck_assert_str_eq(eina_vpath_resolve(".bla"), ".bla");

   snprintf(test, sizeof(test), "%s/", eina_environment_home_get());
   ck_assert_str_eq(eina_vpath_resolve("~/"), test);

   snprintf(test, sizeof(test), "%s/bla", eina_environment_home_get());
   ck_assert_str_eq(eina_vpath_resolve("(:home:)/bla"), test);

}
EFL_END_TEST

EFL_START_TEST(eina_test_vpath_invalid)
{
   ck_assert_ptr_eq(eina_vpath_resolve("(:asdfasdfafasdf"), NULL);
   ck_assert_ptr_eq(eina_vpath_resolve("(:missing_slash:)"), NULL);
   ck_assert_ptr_eq(eina_vpath_resolve("(:"), NULL);
   ck_assert_ptr_eq(eina_vpath_resolve("(:home:)"), NULL);
   ck_assert_ptr_eq(eina_vpath_resolve("(:wrong_meta_key:)/"), NULL);

}
EFL_END_TEST

void eina_test_vpath(TCase *tc)
{
   tcase_add_test(tc, eina_test_vpath_invalid);
   tcase_add_test(tc, eina_test_vpath_valid);
}

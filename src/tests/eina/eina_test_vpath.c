#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include <check.h>
#include "eina_suite.h"
#include <pwd.h>

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
   ck_assert_str_eq(eina_vpath_resolve("/test/for/the/last/case"), "/test/for/the/last/case");

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


EFL_START_TEST(eina_test_vpath_snprintf)
{
   char *string = "blablabla";
   int x = 1337;
   char buf[PATH_MAX];
   char cmp[PATH_MAX];

   eina_vpath_resolve_snprintf(buf, sizeof(buf), "(:home:)/%s/%d/", string, x);
   snprintf(cmp, sizeof(cmp), "%s/%s/%d/", eina_environment_home_get(), string, x);
   ck_assert_str_eq(buf, cmp);
}
EFL_END_TEST

void eina_test_vpath(TCase *tc)
{
   tcase_add_test(tc, eina_test_vpath_invalid);
   tcase_add_test(tc, eina_test_vpath_valid);
   tcase_add_test(tc, eina_test_vpath_snprintf);
}

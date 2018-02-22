#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include <check.h>


START_TEST(eina_test_vpath_valid)
{
   int ret;
   char test[PATH_MAX];

   ret = eina_init();
   ck_assert_int_ne(ret, 0);

   ck_assert_str_eq(eina_vpath_resolve("/"), "/");
   ck_assert_str_eq(eina_vpath_resolve("./"), "./");
   ck_assert_str_eq(eina_vpath_resolve("..bla"), "..bla");
   ck_assert_str_eq(eina_vpath_resolve(".bla"), ".bla");

   snprintf(test, sizeof(test), "%s/", eina_environment_home_get());
   ck_assert_str_eq(eina_vpath_resolve("~/"), test);

   snprintf(test, sizeof(test), "%s/bla", eina_environment_home_get());
   ck_assert_str_eq(eina_vpath_resolve("(:home:)/bla"), test);

   ret = eina_shutdown();
}
END_TEST

START_TEST(eina_test_vpath_invalid)
{
   int ret;

   ret = eina_init();
   ck_assert_int_ne(ret, 0);

   ck_assert_ptr_eq(eina_vpath_resolve("(:asdfasdfafasdf"), NULL);
   ck_assert_ptr_eq(eina_vpath_resolve("(:missing_slash:)"), NULL);
   ck_assert_ptr_eq(eina_vpath_resolve("(:"), NULL);
   ck_assert_ptr_eq(eina_vpath_resolve("(:home:)"), NULL);
   ck_assert_ptr_eq(eina_vpath_resolve("(:wrong_meta_key:)/"), NULL);

   ret = eina_shutdown();
}
END_TEST

void eina_test_vpath(TCase *tc)
{
   tcase_add_test(tc, eina_test_vpath_invalid);
   tcase_add_test(tc, eina_test_vpath_valid);
}

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#define EFL_NOLEGACY_API_SUPPORT
#include <Efl_Core.h>
#include "efl_app_suite.h"
#include "../efl_check.h"

EFL_START_TEST(efl_core_env_test_set_get)
{
   Efl_Core_Env *env = efl_add_ref(EFL_CORE_ENV_CLASS, NULL);

   efl_core_env_set(env, "FOO", "bar");
   efl_core_env_set(env, "going", "home");
   efl_core_env_set(env, "Merry", "christmas");

   ck_assert_str_eq(efl_core_env_get(env, "FOO"), "bar");
   ck_assert_str_eq(efl_core_env_get(env, "going"), "home");
   ck_assert_str_eq(efl_core_env_get(env, "Merry"), "christmas");

   efl_core_env_unset(env, "Merry");

   ck_assert_str_eq(efl_core_env_get(env, "FOO"), "bar");
   ck_assert_str_eq(efl_core_env_get(env, "going"), "home");
   ck_assert_ptr_eq(efl_core_env_get(env, "Merry"), NULL);

   efl_unref(env);
}
EFL_END_TEST

EFL_START_TEST(efl_core_env_test_invalid_keys)
{
   Efl_Core_Env *env = efl_add_ref(EFL_CORE_ENV_CLASS, NULL);

#define CHECK(val) \
   efl_core_env_set(env, val, "TEST"); \
   ck_assert_ptr_eq(efl_core_env_get(env, val), NULL);

   CHECK("0foo");
   CHECK("foo bar");
   CHECK("foo!bar");

#undef CHECK


#define CHECK(val) \
   efl_core_env_set(env, val, "TEST"); \
   ck_assert_str_eq(efl_core_env_get(env, val), "TEST");

   CHECK("foo0");
   CHECK("foo_bar");

#undef CHECK

}
EFL_END_TEST

EFL_START_TEST(efl_core_env_test_clear)
{
   Efl_Core_Env *env = efl_add_ref(EFL_CORE_ENV_CLASS, NULL);

   efl_core_env_set(env, "FOO", "bar");
   efl_core_env_set(env, "going", "home");
   efl_core_env_set(env, "Merry", "christmas");

   efl_core_env_clear(env);

   ck_assert_ptr_eq(efl_core_env_get(env, "FOO"), NULL);
   ck_assert_ptr_eq(efl_core_env_get(env, "going"), NULL);
   ck_assert_ptr_eq(efl_core_env_get(env, "Merry"), NULL);

   efl_unref(env);
}
EFL_END_TEST

EFL_START_TEST(efl_core_env_test_fork)
{
   Efl_Core_Env *env_fork, *env = efl_add_ref(EFL_CORE_ENV_CLASS, NULL);

   efl_core_env_set(env, "FOO", "bar");
   efl_core_env_set(env, "going", "home");
   efl_core_env_set(env, "Merry", "christmas");

   env_fork = efl_duplicate(env);

   ck_assert_str_eq(efl_core_env_get(env_fork, "FOO"), "bar");
   ck_assert_str_eq(efl_core_env_get(env_fork, "going"), "home");
   ck_assert_str_eq(efl_core_env_get(env_fork, "Merry"), "christmas");

   efl_unref(env);
}
EFL_END_TEST

EFL_START_TEST(efl_core_env_test_process)
{
   Efl_Core_Env *env_fork, *env = efl_env_self(EFL_CORE_PROC_ENV_CLASS);

   ck_assert(env);

   ck_assert_str_eq(efl_core_env_get(env, "PATH"), getenv("PATH"));
   env_fork = efl_duplicate(env);
   ck_assert_str_eq(efl_core_env_get(env_fork, "PATH"), getenv("PATH"));

   efl_unref(env);
}
EFL_END_TEST

EFL_START_TEST(efl_core_env_test_undepend_fork)
{
   Efl_Core_Env *env_fork, *env = efl_env_self(EFL_CORE_PROC_ENV_CLASS);

   ck_assert(env);

   ck_assert_str_eq(efl_core_env_get(env, "PATH"), getenv("PATH"));
   env_fork = efl_duplicate(env);
   efl_core_env_set(env_fork, "PATH", "abc");
   ck_assert_str_eq(efl_core_env_get(env, "PATH"), getenv("PATH"));

   efl_unref(env);
   efl_unref(env_fork);
}
EFL_END_TEST

void efl_test_efl_env(TCase *tc)
{
   tcase_add_test(tc, efl_core_env_test_set_get);
   tcase_add_test(tc, efl_core_env_test_invalid_keys);
   tcase_add_test(tc, efl_core_env_test_clear);
   tcase_add_test(tc, efl_core_env_test_fork);
   tcase_add_test(tc, efl_core_env_test_process);
   tcase_add_test(tc, efl_core_env_test_undepend_fork);
}

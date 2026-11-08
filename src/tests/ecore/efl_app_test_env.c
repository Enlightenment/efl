#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#if defined(__GLIBC__) && defined(__GLIBC_PREREQ)
# if __GLIBC_PREREQ(2, 33)
#  include <malloc.h>
#  define HAVE_MALLINFO2
# endif
#endif
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
   Efl_Core_Env *env_fork, *env = efl_env_self();

   ck_assert(env);

   ck_assert_str_eq(efl_core_env_get(env, "PATH"), getenv("PATH"));
   env_fork = efl_duplicate(env);
   ck_assert_str_eq(efl_core_env_get(env_fork, "PATH"), getenv("PATH"));

   efl_unref(env);
}
EFL_END_TEST

EFL_START_TEST(efl_core_env_test_undepend_fork)
{
   Efl_Core_Env *env_fork, *env = efl_env_self();

   ck_assert(env);

   ck_assert_str_eq(efl_core_env_get(env, "PATH"), getenv("PATH"));
   env_fork = efl_duplicate(env);
   efl_core_env_set(env_fork, "PATH", "abc");
   ck_assert_str_eq(efl_core_env_get(env, "PATH"), getenv("PATH"));

   efl_unref(env);
   efl_unref(env_fork);
}
EFL_END_TEST

/* 200 environments of 16 variables of a kilobyte each, and a duplicate of each,
 * made and deleted. */
static void
_env_churn(void)
{
   char name[32], value[1024];
   int n, i;

   memset(value, 'v', sizeof(value) - 1);
   value[sizeof(value) - 1] = '\0';
   for (n = 0; n < 200; n++)
     {
        Efl_Core_Env *env = efl_add_ref(EFL_CORE_ENV_CLASS, NULL), *copy;

        for (i = 0; i < 16; i++)
          {
             snprintf(name, sizeof(name), "VAR_%d_%d", n, i);
             value[0] = 'a' + i;
             efl_core_env_set(env, name, value);
          }
        copy = efl_duplicate(env);
        snprintf(name, sizeof(name), "VAR_%d_3", n);
        ck_assert_int_eq(efl_core_env_get(copy, name)[0], 'd');
        efl_unref(env);
        efl_unref(copy);
     }
}

EFL_START_TEST(efl_core_env_test_destroyed_with_values)
{
#ifdef HAVE_MALLINFO2
   size_t before, after;

   /* A deleted environment gives its variables back. What a leak keeps shows in
    * the heap: 200 environments with 16 values of a kilobyte, twice over, are
    * megabytes, and the heap must not grow by a fraction of that. */
   _env_churn();
   before = mallinfo2().uordblks;
   _env_churn();
   after = mallinfo2().uordblks;
   ck_assert_msg(after < before + 256 * 1024, "the heap grew by %zu bytes", after - before);
#else
   _env_churn();
#endif
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
   tcase_add_test(tc, efl_core_env_test_destroyed_with_values);
}

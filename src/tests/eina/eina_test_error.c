/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric Bail
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
# include <Evil.h>
#endif

#include <Eina.h>

#include "eina_suite.h"

#define TEST_TEXT "The big test\n"

struct log_ctx {
   const char *msg;
   const char *fnc;
   Eina_Bool did;
};

/* tests should not output on success, just uncomment this for debugging */
//#define SHOW_LOG 1

static void
_eina_test_safety_print_cb(const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line, const char *fmt, void *data, va_list args EINA_UNUSED)
{
   struct log_ctx *ctx = data;
   va_list cp_args;
   const char *str;

   va_copy(cp_args, args);
   str = va_arg(cp_args, const char *);
   va_end(cp_args);

   ck_assert_int_eq(level, EINA_LOG_LEVEL_ERR);
   ck_assert_str_eq(fmt, "%s");
   ck_assert_str_eq(ctx->msg, str);
   ck_assert_str_eq(ctx->fnc, fnc);
   ctx->did = EINA_TRUE;

#ifdef SHOW_LOG
   eina_log_print_cb_stderr(d, level, file, fnc, line, fmt, NULL, args);
#else
   (void)d;
   (void)file;
   (void)line;
#endif
}

EFL_START_TEST(eina_error_errno)
{
   int test;

   setenv("EINA_ERROR_LEVEL", "1", 0);

   test = eina_error_msg_register(TEST_TEXT);
   fail_if(!eina_error_msg_get(test));
   fail_if(strcmp(eina_error_msg_get(test), TEST_TEXT) != 0);

   eina_error_set(test);
   fail_if(eina_error_get() != test);

   eina_error_set(EBADF);
   ck_assert_int_eq(eina_error_get(), EBADF);
   ck_assert_str_eq(eina_error_msg_get(EBADF), strerror(EBADF));

}
EFL_END_TEST

EFL_START_TEST(eina_error_test_find)
{
   int test, r;
   const char *str;

   /* Make sure the value isn't already there. */
   r = eina_error_find(TEST_TEXT TEST_TEXT);
   ck_assert_int_eq(r, 0);

   test = eina_error_msg_register(TEST_TEXT TEST_TEXT);
   ck_assert_int_ne(test, 0);

   str = eina_error_msg_get(test);
   fail_unless(str != NULL);
   ck_assert_str_eq(str, TEST_TEXT TEST_TEXT);

   eina_error_set(test);
   fail_if(eina_error_get() != test);

   r = eina_error_find(TEST_TEXT TEST_TEXT);
   ck_assert_int_eq(r, test);

}
EFL_END_TEST

EFL_START_TEST(eina_error_test_modify)
{
   int test, r;
   const char *str, smsg[] = "Do not copy this string";

   test = eina_error_msg_register("Some Test Error");
   ck_assert_int_ne(test, 0);

   str = eina_error_msg_get(test);
   fail_unless(str != NULL);
   ck_assert_str_eq(str, "Some Test Error");

   eina_error_set(test);
   fail_if(eina_error_get() != test);

   fail_unless(eina_error_msg_modify(test, "ABCDE"));

   r = eina_error_find("ABCDE");
   ck_assert_int_eq(r, test);

   test = eina_error_msg_static_register(smsg);
   ck_assert_int_ne(test, 0);

   str = eina_error_msg_get(test);
   fail_unless(str != NULL);
   fail_unless(str == smsg);

   fail_unless(eina_error_msg_modify(test, "Change that!"));
   r = eina_error_find("Change that!");
   ck_assert_int_eq(r, test);

}
EFL_END_TEST

EFL_START_TEST(eina_error_test_lots)
{
   char buf[64];
   int codes[512];
   unsigned int i;

   for (i = 0; i < sizeof(codes)/sizeof(codes[0]); i++)
     {
        snprintf(buf, sizeof(buf), "myerr-%u", i);
        codes[i] = eina_error_msg_register(buf);
        ck_assert_int_ne(codes[i], 0);
     }

   for (i = 0; i < sizeof(codes)/sizeof(codes[0]); i++)
     {
        int found;

        snprintf(buf, sizeof(buf), "myerr-%u", i);

        found = eina_error_find(buf);
        ck_assert_int_eq(codes[i], found);
     }

}
EFL_END_TEST

#ifdef EINA_SAFETY_CHECKS
EFL_START_TEST(eina_error_test_failures)
{
   struct log_ctx ctx;
   Eina_Error local_error;

   eina_log_print_cb_set(_eina_test_safety_print_cb, &ctx);

#define TEST_MAGIC_SAFETY(fn, _msg)              \
  ctx.msg = _msg;                                \
  ctx.fnc = fn;                                  \
  ctx.did = EINA_FALSE

   TEST_MAGIC_SAFETY("eina_error_msg_register",
                     "safety check failed: msg == NULL");
   ck_assert_int_eq(eina_error_msg_register(NULL), 0);
   fail_unless(ctx.did);

   TEST_MAGIC_SAFETY("eina_error_msg_static_register",
                     "safety check failed: msg == NULL");
   ck_assert_int_eq(eina_error_msg_static_register(NULL), 0);
   fail_unless(ctx.did);

   TEST_MAGIC_SAFETY("eina_error_msg_modify",
                     "safety check failed: EINA_ERROR_REGISTERED_CHECK(error) is false");
   ck_assert_int_eq(eina_error_msg_modify(0, "X"), EINA_FALSE);

   TEST_MAGIC_SAFETY("eina_error_msg_modify",
                     "safety check failed: EINA_ERROR_REGISTERED_CHECK(error) is false");
   ck_assert_int_eq(eina_error_msg_modify(4096, "X"), EINA_FALSE);

   TEST_MAGIC_SAFETY("eina_error_msg_modify",
                     "safety check failed: msg == NULL");
   ck_assert_int_eq(eina_error_msg_modify(ENOMEM, NULL),
                    EINA_FALSE);
   fail_unless(ctx.did);

   local_error = eina_error_msg_static_register("Local error for test");
   ck_assert_int_ne(local_error, 0);

   TEST_MAGIC_SAFETY("eina_error_msg_modify",
                     "safety check failed: msg == NULL");
   ck_assert_int_eq(eina_error_msg_modify(local_error, NULL),
                    EINA_FALSE);
   fail_unless(ctx.did);

   ck_assert_str_eq(eina_error_msg_get(ENOMEM),
                    "Cannot allocate memory");

   TEST_MAGIC_SAFETY("eina_error_find",
                     "safety check failed: msg == NULL");
   ck_assert_int_eq(eina_error_find(NULL), 0);
   fail_unless(ctx.did);

   ck_assert_int_eq(eina_error_find("Non-existent Error..."), 0);

   fail_if(eina_error_msg_get(0));
   fail_if(eina_error_msg_get(4096));

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

}
EFL_END_TEST
#endif

void
eina_test_error(TCase *tc)
{
   tcase_add_test(tc, eina_error_errno);
   tcase_add_test(tc, eina_error_test_find);
   tcase_add_test(tc, eina_error_test_modify);
   tcase_add_test(tc, eina_error_test_lots);
#ifdef EINA_SAFETY_CHECKS
   tcase_add_test(tc, eina_error_test_failures);
#endif
}

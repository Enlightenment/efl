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

#define EINA_MAGIC_DEBUG

#include <Eina.h>
#include "eina_safety_checks.h"

#include "eina_suite.h"

#define EINA_MAGIC_TEST 0x7781fee7
#define EINA_MAGIC_TEST2 0x42241664
#define EINA_MAGIC_STRING "Eina Magic Test"

typedef struct _Eina_Magic_Struct Eina_Magic_Struct;
struct _Eina_Magic_Struct
{
   EINA_MAGIC
};

struct log_ctx {
   const char *msg;
   const char *fnc;
   Eina_Bool did;
   int expected_level;
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

   ck_assert_int_eq(level, ctx->expected_level);
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

static void
_eina_test_magic_print_cb(const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line, const char *fmt, void *data, va_list args EINA_UNUSED)
{
   struct log_ctx *ctx = data;

   ck_assert_int_eq(level, ctx->expected_level);
   ck_assert_str_eq(ctx->msg, fmt);
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

#ifdef EINA_SAFETY_CHECKS
#define TEST_MAGIC_SAFETY(fn, _msg)              \
  ctx.msg = _msg;                                \
  ctx.fnc = fn;                                  \
  ctx.did = EINA_FALSE;                          \
  ctx.expected_level = EINA_LOG_LEVEL_ERR
#endif

EFL_START_TEST(eina_magic_simple)
{
   Eina_Magic_Struct *ems = NULL;
   struct log_ctx ctx;


   eina_log_print_cb_set(_eina_test_safety_print_cb, &ctx);

   eina_magic_string_set(EINA_MAGIC_TEST, EINA_MAGIC_STRING);

#ifdef EINA_SAFETY_CHECKS
#ifdef SHOW_LOG
   fprintf(stderr, "you should have a safety check failure below:\n");
#endif
   TEST_MAGIC_SAFETY("eina_magic_string_set",
                     "safety check failed: magic_name == NULL");
   eina_magic_string_set(EINA_MAGIC_TEST2, NULL);
   fail_unless(ctx.did);

#ifdef SHOW_LOG
   fprintf(stderr, "you should have a safety check failure below:\n");
#endif
   TEST_MAGIC_SAFETY("eina_magic_string_set",
                     "safety check failed: magic_name == NULL");
   eina_magic_string_set(EINA_MAGIC_TEST2, NULL);
   fail_unless(ctx.did);
#endif

   eina_magic_string_set(EINA_MAGIC_TEST2, EINA_MAGIC_STRING);

   fail_if(eina_magic_string_get(EINA_MAGIC_TEST) == NULL);
   fail_if(strcmp(eina_magic_string_get(
                     EINA_MAGIC_TEST), EINA_MAGIC_STRING) != 0);

   eina_log_print_cb_set(_eina_test_magic_print_cb, &ctx);

#ifdef EINA_MAGIC_DEBUG
   fail_if(EINA_MAGIC_CHECK(ems, EINA_MAGIC_TEST));
#ifdef SHOW_LOG
   fprintf(stderr, "you should see 'Input handle pointer is NULL' below\n");
#endif
   TEST_MAGIC_SAFETY(__FUNCTION__,
                     "*** Eina Magic Check Failed !!!\n"
                     "    Input handle pointer is NULL.\n"
                     "\n");
   EINA_MAGIC_FAIL(ems, EINA_MAGIC_TEST);
   fail_unless(ctx.did);

   ems = malloc(sizeof (Eina_Magic_Struct));
   fail_if(!ems);
   EINA_MAGIC_SET(ems, EINA_MAGIC_TEST);

   fail_if(!EINA_MAGIC_CHECK(ems, EINA_MAGIC_TEST));

   EINA_MAGIC_SET(ems, EINA_MAGIC_NONE);
#ifdef SHOW_LOG
   fprintf(stderr,
           "you should see 'Input handle has already been freed' below\n");
#endif
   TEST_MAGIC_SAFETY(__FUNCTION__,
                     "*** Eina Magic Check Failed at %p !!!\n"
                     "    Input handle has already been freed.\n"
                     "\n");
   ctx.expected_level = EINA_LOG_LEVEL_CRITICAL;
   EINA_MAGIC_FAIL(ems, EINA_MAGIC_TEST);
   fail_unless(ctx.did);

   EINA_MAGIC_SET(ems, 42424242);
#ifdef SHOW_LOG
   fprintf(stderr, "you should see 'Input handle is wrong type' below\n");
#endif
   TEST_MAGIC_SAFETY(__FUNCTION__,
                     "*** Eina Magic Check Failed at %p !!!\n"
                     "    Input handle is wrong type.\n"
                     "    Expected: %08x - %s\n"
                     "    Supplied: %08x - %s\n"
                     "\n");
   ctx.expected_level = EINA_LOG_LEVEL_CRITICAL;
   EINA_MAGIC_FAIL(ems, EINA_MAGIC_TEST);
   fail_unless(ctx.did);
#endif

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

}
EFL_END_TEST

void eina_test_magic(TCase *tc)
{
   tcase_add_test(tc, eina_magic_simple);
}


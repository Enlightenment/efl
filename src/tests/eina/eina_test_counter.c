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

#include <stdio.h>
#include <stdlib.h>

#include <Eina.h>
#include "eina_safety_checks.h"

#include "eina_suite.h"

#ifdef EINA_SAFETY_CHECKS
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
#endif

EFL_START_TEST(eina_counter_simple)
{
   Eina_Counter *cnt;
   char *dump;
   int i;

   cnt = eina_counter_new("eina_test");
   fail_if(!cnt);

   eina_counter_start(cnt);

   for (i = 0; i < 100000; ++i)
     {
        void *tmp = malloc(sizeof(long int));
        free(tmp);
     }

   eina_counter_stop(cnt, i);

   eina_counter_start(cnt);

   for (i = 0; i < 200000; ++i)
     {
        void *tmp = malloc(sizeof(long int));
        free(tmp);
     }

   eina_counter_stop(cnt, i);

   dump = eina_counter_dump(cnt);
   fail_if(!dump);

   /* TODO: parse dump and check if it's right */
   fprintf(stderr, "%s", dump);

   free(dump);

   eina_counter_free(cnt);

}
EFL_END_TEST

EFL_START_TEST(eina_counter_break)
{
   Eina_Counter *cnt;

   cnt = eina_counter_new("eina_test");
   fail_if(!cnt);

   eina_counter_stop(cnt, 10);

   eina_counter_free(cnt);

#ifdef EINA_SAFETY_CHECKS
   {
      struct log_ctx ctx;
      char *dump;

#define TEST_MAGIC_SAFETY(fn, _msg)              \
      ctx.msg = _msg;                            \
      ctx.fnc = fn;                              \
      ctx.did = EINA_FALSE

      eina_log_print_cb_set(_eina_test_safety_print_cb, &ctx);

#ifdef SHOW_LOG
      fprintf(stderr, "you should have a safety check failure below:\n");
#endif
      TEST_MAGIC_SAFETY("eina_counter_new",
                        "safety check failed: name == NULL");
      cnt = eina_counter_new(NULL);
      fail_if(cnt);
      fail_unless(ctx.did);

#ifdef SHOW_LOG
      fprintf(stderr, "you should have a safety check failure below:\n");
#endif
      TEST_MAGIC_SAFETY("eina_counter_free",
                        "safety check failed: counter == NULL");
      eina_counter_free(NULL);
      fail_unless(ctx.did);

#ifdef SHOW_LOG
      fprintf(stderr, "you should have a safety check failure below:\n");
#endif
      TEST_MAGIC_SAFETY("eina_counter_start",
                        "safety check failed: counter == NULL");
      eina_counter_start(NULL);
      fail_unless(ctx.did);

#ifdef SHOW_LOG
      fprintf(stderr, "you should have a safety check failure below:\n");
#endif
      TEST_MAGIC_SAFETY("eina_counter_stop",
                        "safety check failed: counter == NULL");
      eina_counter_stop(NULL, 0);
      fail_unless(ctx.did);


#ifdef SHOW_LOG
      fprintf(stderr, "you should have a safety check failure below:\n");
#endif
      TEST_MAGIC_SAFETY("eina_counter_dump",
                        "safety check failed: counter == NULL");
      dump = eina_counter_dump(NULL);
      fail_if(dump);
      fail_unless(ctx.did);

      eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);
   }
#endif

}
EFL_END_TEST

void eina_test_counter(TCase *tc)
{
   tcase_add_test(tc, eina_counter_simple);
   tcase_add_test(tc, eina_counter_break);
}


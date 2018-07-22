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

#include <Eina.h>
#include "eina_safety_checks.h"

#include "eina_suite.h"

typedef struct _Eina_Test_Inlist Eina_Test_Inlist;
struct _Eina_Test_Inlist
{
   int i;
   EINA_INLIST;
};

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

static Eina_Test_Inlist *
_eina_test_inlist_build(int i)
{
   Eina_Test_Inlist *tmp;

   tmp = malloc(sizeof(Eina_Test_Inlist));
   fail_if(!tmp);
   tmp->i = i;

   return tmp;
}

EFL_START_TEST(eina_inlist_simple)
{
   Eina_Inlist *lst = NULL;
   Eina_Inlist *tmpl = NULL;
   Eina_Test_Inlist *tmp;
   Eina_Test_Inlist *prev;
   int i = 0;
#ifdef EINA_SAFETY_CHECKS
   Eina_Inlist *bkp;
   struct log_ctx ctx;
#endif

   tmp = _eina_test_inlist_build(42);
   lst = eina_inlist_append(lst, EINA_INLIST_GET(tmp));
   fail_if(!lst);

   lst = eina_inlist_remove(lst, EINA_INLIST_GET(tmp));
   lst = eina_inlist_prepend(lst, EINA_INLIST_GET(tmp));

   tmp = _eina_test_inlist_build(1664);
   lst = eina_inlist_append_relative(lst, EINA_INLIST_GET(tmp), lst);
   fail_if(!lst);
   fail_if(EINA_INLIST_CONTAINER_GET(lst, Eina_Test_Inlist)->i != 42);

   prev = tmp;
   tmp = _eina_test_inlist_build(3227);
   lst = eina_inlist_prepend_relative(lst, EINA_INLIST_GET(
                                         tmp), EINA_INLIST_GET(prev));
   fail_if(!lst);
   fail_if(EINA_INLIST_CONTAINER_GET(lst, Eina_Test_Inlist)->i != 42);

   lst = eina_inlist_remove(lst, EINA_INLIST_GET(tmp));

   lst = eina_inlist_append_relative(lst, EINA_INLIST_GET(tmp), lst);
   lst = eina_inlist_remove(lst, EINA_INLIST_GET(tmp));

   lst = eina_inlist_prepend_relative(lst, EINA_INLIST_GET(tmp), lst);

   tmp = _eina_test_inlist_build(27);
   lst = eina_inlist_prepend_relative(lst, EINA_INLIST_GET(tmp), NULL);

   tmp = _eina_test_inlist_build(81);
   lst = eina_inlist_append_relative(lst, EINA_INLIST_GET(tmp), NULL);

   EINA_INLIST_FOREACH(lst, tmp)
   {
      switch (i)
        {
         case 0: fail_if(tmp->i != 27); break;

         case 1: fail_if(tmp->i != 3227); break;

         case 2: fail_if(tmp->i != 42); break;

         case 3: fail_if(tmp->i != 1664); break;

         case 4: fail_if(tmp->i != 81); break;
        }

      ++i;
   }

#ifdef EINA_SAFETY_CHECKS
   bkp = lst;
   eina_log_print_cb_set(_eina_test_safety_print_cb, &ctx);

#define TEST_MAGIC_SAFETY(fn, _msg)              \
   ctx.msg = _msg;                               \
   ctx.fnc = fn;                                 \
   ctx.did = EINA_FALSE

#ifdef SHOW_LOG
   fprintf(stderr, "you should have a safety check failure below:\n");
#endif
   {
      Eina_Inlist *tmp2;

      TEST_MAGIC_SAFETY("eina_inlist_remove",
                        "safety check failed: list == NULL");

      tmp2 = eina_inlist_remove(NULL, EINA_INLIST_GET(tmp));
      fail_if(tmp2 != NULL);
      fail_unless(ctx.did);
   }

#ifdef SHOW_LOG
   fprintf(stderr, "you should have a safety check failure below:\n");
#endif
   TEST_MAGIC_SAFETY("eina_inlist_remove",
                     "safety check failed: item == NULL");
   lst = eina_inlist_remove(lst, NULL);
   fail_unless(ctx.did);

#ifdef SHOW_LOG
   fprintf(stderr, "you should have a safety check failure below:\n");
#endif
   TEST_MAGIC_SAFETY("eina_inlist_append",
                     "safety check failed: new_l == NULL");
   lst = eina_inlist_append(lst, NULL);
   fail_unless(ctx.did);

#ifdef SHOW_LOG
   fprintf(stderr, "you should have a safety check failure below:\n");
#endif
   TEST_MAGIC_SAFETY("eina_inlist_append_relative",
                     "safety check failed: new_l == NULL");
   lst = eina_inlist_append_relative(lst, NULL, NULL);
   fail_unless(ctx.did);

#ifdef SHOW_LOG
   fprintf(stderr, "you should have a safety check failure below:\n");
#endif
   TEST_MAGIC_SAFETY("eina_inlist_prepend",
                     "safety check failed: new_l == NULL");
   lst = eina_inlist_prepend(lst, NULL);
   fail_unless(ctx.did);

#ifdef SHOW_LOG
   fprintf(stderr, "you should have a safety check failure below:\n");
#endif
   TEST_MAGIC_SAFETY("eina_inlist_prepend_relative",
                     "safety check failed: new_l == NULL");
   lst = eina_inlist_prepend_relative(lst, NULL, NULL);
   fail_unless(ctx.did);

#ifdef SHOW_LOG
   fprintf(stderr, "you should have a safety check failure below:\n");
#endif
   TEST_MAGIC_SAFETY("eina_inlist_find",
                     "safety check failed: item == NULL");
   lst = eina_inlist_find(lst, NULL);
   fail_unless(ctx.did);

#ifdef SHOW_LOG
   fprintf(stderr, "you should have a safety check failure below:\n");
#endif
   TEST_MAGIC_SAFETY("eina_inlist_demote",
                     "safety check failed: list == NULL");
   lst = eina_inlist_demote(NULL, NULL);
   fail_unless(ctx.did);

#ifdef SHOW_LOG
   fprintf(stderr, "you should have a safety check failure below:\n");
#endif
   TEST_MAGIC_SAFETY("eina_inlist_demote",
                     "safety check failed: item == NULL");
   lst = eina_inlist_demote((void*)1L, NULL);
   fail_unless(ctx.did);
   lst = NULL;

#ifdef SHOW_LOG
   fprintf(stderr, "you should have a safety check failure below:\n");
#endif
   TEST_MAGIC_SAFETY("eina_inlist_promote",
                     "safety check failed: list == NULL");
   lst = eina_inlist_promote(NULL, NULL);
   fail_unless(ctx.did);

#ifdef SHOW_LOG
   fprintf(stderr, "you should have a safety check failure below:\n");
#endif
   TEST_MAGIC_SAFETY("eina_inlist_promote",
                     "safety check failed: item == NULL");
   lst = eina_inlist_promote((void*)1L, NULL);
   fail_unless(ctx.did);
   lst = NULL;

#ifdef SHOW_LOG
   fprintf(stderr, "you should have a safety check failure below:\n");
#endif
   TEST_MAGIC_SAFETY("eina_inlist_sorted_insert",
                     "safety check failed: item == NULL");
   lst = eina_inlist_sorted_insert(NULL, NULL, NULL);
   fail_unless(ctx.did);

#ifdef SHOW_LOG
   fprintf(stderr, "you should have a safety check failure below:\n");
#endif
   TEST_MAGIC_SAFETY("eina_inlist_sorted_insert",
                     "safety check failed: func == NULL");
   lst = eina_inlist_sorted_insert(NULL, (void*)1L, NULL);
   fail_unless(ctx.did);
   lst = NULL;

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);
   lst = bkp;
#endif

   tmpl = eina_inlist_last(lst);
   fail_if(tmpl == lst);
   tmpl = eina_inlist_first(tmpl);
   fail_if(tmpl != lst);

   tmp = EINA_INLIST_CONTAINER_GET(lst, Eina_Test_Inlist);
   lst = eina_inlist_demote(lst, lst);
   fail_if(EINA_INLIST_CONTAINER_GET(lst, Eina_Test_Inlist) == tmp);

   lst = eina_inlist_promote(lst, EINA_INLIST_GET(tmp));
   fail_if(lst != EINA_INLIST_GET(tmp));

   tmp = EINA_INLIST_CONTAINER_GET(eina_inlist_find(lst, EINA_INLIST_GET(
                                                       prev)), Eina_Test_Inlist);
   lst = eina_inlist_remove(lst, EINA_INLIST_GET(tmp));
   prev = (Eina_Test_Inlist *)eina_inlist_find(lst, EINA_INLIST_GET(tmp));
   tmp = prev ? EINA_INLIST_CONTAINER_GET(prev, Eina_Test_Inlist) : NULL;
   fail_if(tmp != NULL);

   while (lst)
      lst = eina_inlist_remove(lst, lst);

}
EFL_END_TEST

typedef struct _Eina_Test_Inlist_Sorted Eina_Test_Inlist_Sorted;
struct _Eina_Test_Inlist_Sorted
{
   EINA_INLIST;

   int value;
};

static int
_eina_test_inlist_cmp(const void *d1, const void *d2)
{
   const Eina_Test_Inlist_Sorted *t1 = d1;
   const Eina_Test_Inlist_Sorted *t2 = d2;

   return t1->value - t2->value;
}

static int
_eina_test_inlist_cmp2(const void *d1, const void *d2)
{
   const Eina_Test_Inlist_Sorted *t1 = d1;
   const Eina_Test_Inlist_Sorted *t2 = d2;

   return t2->value - t1->value;
}

#define _eina_test_inlist_check(LIST) \
{ \
   const Eina_Test_Inlist_Sorted *_t; \
   int _i = 0; \
 \
   EINA_INLIST_FOREACH(LIST, _t) \
     { \
        ck_assert_int_eq(_t->value, values[_i]); \
        _i++; \
     } \
}

#define _eina_test_inlist_check_reverse(LIST) \
{ \
   const Eina_Test_Inlist_Sorted *_t; \
   int _i = EINA_C_ARRAY_LENGTH(values) - 1; \
 \
   EINA_INLIST_FOREACH(LIST, _t) \
     { \
        ck_assert_int_eq(_t->value, values[_i]); \
        _i--; \
     } \
}

static int values_unsorted[] =
{
   3,
   10,
   2,
   1,
   8,
   4,
   4,
   5,
   7,
   9,
   6,
};

static int values[] =
{
   1,
   2,
   3,
   4,
   4,
   5,
   6,
   7,
   8,
   9,
   10,
};


static void
_eina_test_inlist_check_insert(const Eina_Inlist *list)
{
   const Eina_Test_Inlist_Sorted *t;
   int last_value = 0;

   EINA_INLIST_FOREACH(list, t)
     {
        ck_assert_int_ge(t->value, last_value);
        last_value = t->value;
     }
}

static void
_eina_test_inlist_check_insert_reverse(const Eina_Inlist *list)
{
   const Eina_Test_Inlist_Sorted *t;
   int last_value = 0;

   EINA_INLIST_FOREACH(list, t)
     {
        if (last_value)
          ck_assert_int_le(t->value, last_value);
        last_value = t->value;
     }
}
 
EFL_START_TEST(eina_inlist_sorted)
{
   Eina_Test_Inlist_Sorted *t, tmp[EINA_C_ARRAY_LENGTH(values_unsorted)];
   Eina_Inlist *list = NULL;
   Eina_Inlist *sorted = NULL;
   unsigned int i;

   for (i = 0; i < EINA_C_ARRAY_LENGTH(values_unsorted); ++i)
     {
        tmp[i].value = values_unsorted[i];

        list = eina_inlist_prepend(list, EINA_INLIST_GET(&tmp[i]));
     }

   list = eina_inlist_sort(list, _eina_test_inlist_cmp);

   _eina_test_inlist_check(list);

   i = EINA_C_ARRAY_LENGTH(values_unsorted) - 1;
   EINA_INLIST_FOREACH(list, t)
     {
        t->value = values_unsorted[i];
        i--;
     }

   while (list)
     {
        Eina_Inlist *p = list;

        list = eina_inlist_remove(list, list);

        sorted = eina_inlist_sorted_insert(sorted, p, _eina_test_inlist_cmp);
        _eina_test_inlist_check_insert(sorted);
     }
   _eina_test_inlist_check(sorted);
}
EFL_END_TEST

EFL_START_TEST(eina_inlist_sorted_state)
{
   Eina_Test_Inlist_Sorted tmp[EINA_C_ARRAY_LENGTH(values_unsorted)];
   Eina_Inlist_Sorted_State *state;
   Eina_Inlist *list = NULL;
   unsigned int i;

   state = eina_inlist_sorted_state_new();
   fail_if(!state);

   for (i = 0; i < EINA_C_ARRAY_LENGTH(values_unsorted); ++i)
     {
        tmp[i].value = values_unsorted[i];

        list = eina_inlist_sorted_state_insert(list, EINA_INLIST_GET(&tmp[i]), _eina_test_inlist_cmp, state);
        _eina_test_inlist_check_insert(list);
     }
   eina_inlist_sorted_state_free(state);
   _eina_test_inlist_check(list);
}
EFL_END_TEST
 
EFL_START_TEST(eina_inlist_sorted2)
{
   Eina_Test_Inlist_Sorted *t, tmp[EINA_C_ARRAY_LENGTH(values_unsorted)];
   Eina_Inlist *list = NULL;
   Eina_Inlist *sorted = NULL;
   unsigned int i;

   for (i = 0; i < EINA_C_ARRAY_LENGTH(values_unsorted); ++i)
     {
        tmp[i].value = values_unsorted[i];

        list = eina_inlist_prepend(list, EINA_INLIST_GET(&tmp[i]));
     }

   list = eina_inlist_sort(list, _eina_test_inlist_cmp2);

   _eina_test_inlist_check_reverse(list);

   i = EINA_C_ARRAY_LENGTH(values_unsorted) - 1;
   EINA_INLIST_FOREACH(list, t)
     {
        t->value = values_unsorted[i];
        i--;
     }

   while (list)
     {
        Eina_Inlist *p = list;

        list = eina_inlist_remove(list, list);

        sorted = eina_inlist_sorted_insert(sorted, p, _eina_test_inlist_cmp2);
        _eina_test_inlist_check_insert_reverse(sorted);
     }
   _eina_test_inlist_check_reverse(sorted);
}
EFL_END_TEST

EFL_START_TEST(eina_inlist_sorted_state2)
{
   Eina_Test_Inlist_Sorted tmp[EINA_C_ARRAY_LENGTH(values_unsorted)];
   Eina_Inlist_Sorted_State *state;
   Eina_Inlist *list = NULL;
   unsigned int i;

   state = eina_inlist_sorted_state_new();
   fail_if(!state);

   for (i = 0; i < EINA_C_ARRAY_LENGTH(values_unsorted); ++i)
     {
        tmp[i].value = values_unsorted[i];

        list = eina_inlist_sorted_state_insert(list, EINA_INLIST_GET(&tmp[i]), _eina_test_inlist_cmp2, state);
        _eina_test_inlist_check_insert_reverse(list);
     }
   eina_inlist_sorted_state_free(state);
   _eina_test_inlist_check_reverse(list);
}
EFL_END_TEST

void
eina_test_inlist(TCase *tc)
{
   tcase_add_test(tc, eina_inlist_simple);
   tcase_add_test(tc, eina_inlist_sorted);
   tcase_add_test(tc, eina_inlist_sorted_state);
   tcase_add_test(tc, eina_inlist_sorted2);
   tcase_add_test(tc, eina_inlist_sorted_state2);
}

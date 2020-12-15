/* EINA - EFL data type library
 * Copyright (C) 2020 Expertise Solutions Cons em Inf
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

#include <check.h>
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_UNISTD
# include <unistd.h>
#endif

#ifdef _WIN32
# include <evil_private.h> /* mkdir */
#endif

#include <Eina.h>
#include "eina_suite.h"

static void
thread_cleanup_fn(void *arg)
{
   *(int *) arg  = 1;
}

static void *
thread_fn_execute(void *arg, Eina_Thread t EINA_UNUSED)
{
   EINA_THREAD_CLEANUP_PUSH(thread_cleanup_fn, arg);
   EINA_THREAD_CLEANUP_POP(1);
   return NULL;
}

static void *
thread_fn_skip(void *arg, Eina_Thread t EINA_UNUSED)
{
   EINA_THREAD_CLEANUP_PUSH(thread_cleanup_fn, arg);
   EINA_THREAD_CLEANUP_POP(0);
   return NULL;
}

static void *
thread_fn_cancel(void *arg, Eina_Thread t EINA_UNUSED)
{
   Eina_Condition *cond = arg;

   ck_assert(eina_thread_cancellable_set(EINA_TRUE, NULL));
   ck_assert(eina_condition_signal(cond));

   for (size_t i = 0; i < 100; ++i)
     {
        eina_thread_cancel_checkpoint();
#ifdef _WIN32
        Sleep(100);
#else
        usleep(100 * 1000);
#endif
     }

   return NULL;
}

EFL_START_TEST(eina_thread_test_cleanup_execute)
{
   Eina_Thread t;
   int flag = 0;
   ck_assert(eina_thread_create(&t, EINA_THREAD_NORMAL, -1, thread_fn_execute, &flag));
   eina_thread_join(t);
   ck_assert_uint_eq(flag, 1);
}
EFL_END_TEST

EFL_START_TEST(eina_thread_test_cleanup_skip)
{
   Eina_Thread t;
   int flag = 2;
   ck_assert(eina_thread_create(&t, EINA_THREAD_NORMAL, -1, thread_fn_skip, &flag));
   eina_thread_join(t);
   ck_assert_uint_eq(flag, 2);
}
EFL_END_TEST

EFL_START_TEST(eina_thread_test_cancel)
{
   Eina_Thread t;
   Eina_Lock mutex;
   Eina_Condition cond;

   ck_assert(eina_lock_new(&mutex));
   ck_assert(eina_condition_new(&cond, &mutex));

   ck_assert(eina_thread_create(&t, EINA_THREAD_NORMAL, -1, thread_fn_cancel, &cond));
   ck_assert(eina_lock_take(&mutex));
   ck_assert(eina_condition_wait(&cond));
   ck_assert(eina_thread_cancel(t));
   ck_assert_ptr_eq(eina_thread_join(t), EINA_THREAD_JOIN_CANCELED);

   eina_condition_free(&cond);
   eina_lock_free(&mutex);
}
EFL_END_TEST

void
eina_test_thread(TCase *tc)
{
   tcase_add_test(tc, eina_thread_test_cleanup_skip);
   tcase_add_test(tc, eina_thread_test_cleanup_execute);
   tcase_add_test(tc, eina_thread_test_cancel);
}

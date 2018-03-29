/* EINA - EFL data type library
 * Copyright (C) 2013 ProFUSION embedded systems
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

#include <unistd.h>

#include <Eina.h>

#include "eina_suite.h"

static Eina_Thread wk1, wk2, wk3, wk4, wk5;
static Eina_Barrier barrier;

static void *
wk_func(void *data EINA_UNUSED, Eina_Thread thread EINA_UNUSED)
{
    eina_barrier_wait(&barrier);
    return NULL;
}

static void *
wk1_func(void *data EINA_UNUSED, Eina_Thread thread EINA_UNUSED)
{
    eina_barrier_wait(&barrier);
    return NULL;
}

static void *
wk2_func(void *data EINA_UNUSED, Eina_Thread thread EINA_UNUSED)
{
    eina_barrier_wait(&barrier);
    return NULL;
}

static void *
wk3_func(void *data EINA_UNUSED, Eina_Thread thread EINA_UNUSED)
{
    eina_barrier_wait(&barrier);
    return NULL;
}

EFL_START_TEST(eina_barrier_test_simple)
{
    Eina_Bool r;
    int i;

    i = eina_threads_init();
    _ck_assert_int(i, >=, 1);

    r = eina_barrier_new(&barrier, 6);
    fail_unless(r);

    r = eina_thread_create(&wk1, EINA_THREAD_NORMAL, -1, wk_func, NULL);
    fail_unless(r);

    r = eina_thread_create(&wk2, EINA_THREAD_NORMAL, -1, wk_func, NULL);
    fail_unless(r);

    r = eina_thread_create(&wk3, EINA_THREAD_NORMAL, -1, wk1_func, NULL);
    fail_unless(r);

    r = eina_thread_create(&wk4, EINA_THREAD_NORMAL, -1, wk2_func, NULL);
    fail_unless(r);

    r = eina_thread_create(&wk5, EINA_THREAD_NORMAL, -1, wk3_func, NULL);
    fail_unless(r);

    eina_barrier_wait(&barrier);

    eina_thread_join(wk1);

    eina_thread_join(wk2);

    eina_thread_join(wk3);

    eina_thread_join(wk4);

    eina_thread_join(wk5);

    eina_barrier_free(&barrier);

    eina_threads_shutdown();
}
EFL_END_TEST

void
eina_test_barrier(TCase *tc)
{
#ifndef _WIN32
   tcase_set_timeout(tc, 1);
#endif
   tcase_add_test(tc, eina_barrier_test_simple);
}

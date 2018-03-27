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

#ifdef __linux__
#include <errno.h>
#include <sys/resource.h>
#endif

#include <Eina.h>

#include "eina_suite.h"

#ifdef __linux__

/*
 * TODO: Test if RT priorities are right. However, make check should be run as
 * root.
 */

static void *
_thread_run(void *arg EINA_UNUSED, Eina_Thread tid EINA_UNUSED)
{
    int niceval = getpriority(PRIO_PROCESS, 0);
    int niceval2;
    eina_sched_prio_drop();

    niceval2 = getpriority(PRIO_PROCESS, 0);
    if (niceval + 5 >= 19)
      ck_assert_int_eq(niceval2, 19);
    else
      ck_assert_int_eq(niceval2, (niceval + 5));

    return NULL;
}

EFL_START_TEST(eina_test_sched_prio_drop)
{
    int niceval = getpriority(PRIO_PROCESS, 0);
    int niceval2;
    Eina_Thread tid;
    Eina_Bool r;


    r = eina_thread_create(&tid, EINA_THREAD_NORMAL, -1, _thread_run, NULL);
    fail_unless(r);

    niceval2 = getpriority(PRIO_PROCESS, 0);
    /* niceness of main thread should not have changed */
    ck_assert_int_eq(niceval2, niceval);

    eina_thread_join(tid);
    /* niceness of main thread should not have changed */
    ck_assert_int_eq(niceval2, niceval);

}
EFL_END_TEST
#else
EFL_START_TEST(eina_test_sched_prio_drop)
{
   fprintf(stderr, "scheduler priority is not supported by your configuration.\n");
}
EFL_END_TEST
#endif

void
eina_test_sched(TCase *tc)
{
   tcase_add_test(tc, eina_test_sched_prio_drop);
}

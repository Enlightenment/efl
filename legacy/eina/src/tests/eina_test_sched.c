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

#if defined(EFL_HAVE_THREADS) && defined __linux__
#include <pthread.h>
#include <errno.h>
#include <sys/resource.h>
#endif

#include "eina_suite.h"
#include "Eina.h"

#if defined(EFL_HAVE_THREADS) && defined __linux__

/*
 * TODO: Test if RT priorities are right. However, make check should be run as
 * root.
 */

static void *
_thread_run(void *arg __UNUSED__)
{
    int niceval = getpriority(PRIO_PROCESS, 0);
    int niceval2;
    eina_sched_prio_drop();

    niceval2 = getpriority(PRIO_PROCESS, 0);
    fail_if((niceval2 != 19) && (niceval2 != niceval+5));

    return NULL;
}

START_TEST(eina_test_sched_prio_drop)
{
    int niceval = getpriority(PRIO_PROCESS, 0);
    int niceval2;
    pthread_t tid;

    eina_init();

    pthread_create(&tid, NULL, _thread_run, NULL);

    niceval2 = getpriority(PRIO_PROCESS, 0);
    /* niceness of main thread should not have changed */
    fail_if(niceval2 != niceval);

    pthread_join(tid, NULL);
    /* niceness of main thread should not have changed */
    fail_if(niceval2 != niceval);

    eina_shutdown();
}
END_TEST
#else
START_TEST(eina_test_sched_prio_drop)
{
   fprintf(stderr, "scheduler priority is not supported by your configuration.\n");
}
END_TEST
#endif

void
eina_test_sched(TCase *tc)
{
   tcase_add_test(tc, eina_test_sched_prio_drop);
}

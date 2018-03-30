/* EINA - EFL data type library
 * Copyright (C) 2013 Cedric Bail
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

#include <time.h>

#include <Eina.h>

#include "eina_suite.h"

#ifndef HAVE_CLOCK_GETTIME

# ifdef __MACH__
#  include <mach/clock.h>
#  include <mach/mach.h>

#  define CLOCK_REALTIME 0

int
clock_gettime(int mode, struct timespec* ts)
{
   clock_serv_t cclock;
   mach_timespec_t mts;
   kern_return_t err;

   err = host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
   if (EINA_UNLIKELY(err != KERN_SUCCESS)) return err;

   err = clock_get_time(cclock, &mts);
   mach_port_deallocate(mach_task_self(), cclock);
   if (EINA_UNLIKELY(err != KERN_SUCCESS)) return err;

   ts->tv_sec = mts.tv_sec;
   ts->tv_nsec = mts.tv_nsec;

   return 0;
}
# else /* ! __MACH__ */
# error No support for clock_gettime()
# endif /* __MACH__ */

#endif /* ! HAVE_CLOCK_GETTIME */

static Eina_Spinlock spin;
static Eina_Thread thread;
static unsigned int counter;

static void *
_eina_test_lock_thread(void *data, Eina_Thread t)
{
   unsigned int i;

   fail_if(!eina_thread_equal(t, thread));
   fail_if(strcmp("test", data));

   for (i = 0; i < 150; i++)
     {
        fail_if(eina_spinlock_take(&spin) != EINA_LOCK_SUCCEED);
        counter++;
        fail_if(eina_spinlock_release(&spin) != EINA_LOCK_SUCCEED);
     }

   return data;
}

EFL_START_TEST(eina_test_spinlock)
{
   unsigned int i;


   counter = 0;
   fail_if(!eina_spinlock_new(&spin));

   fail_if(!eina_thread_create(&thread, EINA_THREAD_NORMAL, -1, _eina_test_lock_thread, "test"));

   for (i = 0; i < 150; i++)
     {
        fail_if(eina_spinlock_take(&spin) != EINA_LOCK_SUCCEED);
        counter++;
        fail_if(eina_spinlock_release(&spin) != EINA_LOCK_SUCCEED);
     }

   fail_if(strcmp("test", eina_thread_join(thread)));

   fail_if(counter != 300);

   fail_if(eina_spinlock_take_try(&spin) != EINA_LOCK_SUCCEED);
   fail_if(eina_spinlock_release(&spin) != EINA_LOCK_SUCCEED);

   eina_spinlock_free(&spin);

}
EFL_END_TEST

static Eina_TLS key;
static int _eina_tls_free_count = 0;

static void *
_eina_test_tls_alloc(int v)
{
   int *ptr = malloc(sizeof(int));
   *ptr = v;

   return ptr;
}

static void
_eina_test_tls_free(void *ptr)
{
   _eina_tls_free_count++;
   free(ptr);
}

static void *
_eina_test_tls_thread(void *data EINA_UNUSED, Eina_Thread t EINA_UNUSED)
{
   int *ptr;

   ptr = eina_tls_get(key);
   fail_if(ptr != NULL);

   fail_if(!eina_tls_set(key, _eina_test_tls_alloc(24)));

   ptr = eina_tls_get(key);
   fail_if(ptr == NULL);
   fail_if(*ptr != 24);

   return NULL;
}

EFL_START_TEST(eina_test_tls)
{

   fail_if(!eina_tls_cb_new(&key, _eina_test_tls_free));

   fail_if(!eina_tls_set(key, _eina_test_tls_alloc(42)));

   fail_if(!eina_thread_create(&thread, EINA_THREAD_NORMAL, -1, _eina_test_tls_thread, NULL));

   eina_thread_join(thread);
   fail_if(_eina_tls_free_count != 1);

   int *ptr = eina_tls_get(key);
   fail_if(eina_tls_get(key) == NULL);
   fail_if(*ptr != 42);

   eina_tls_free(key);

}
EFL_END_TEST

static Eina_Barrier barrier;
static Eina_Lock mtcond;
static Eina_Condition cond;
static Eina_RWLock mutex;

static void *
_eina_test_rwlock_thread(void *data EINA_UNUSED, Eina_Thread t EINA_UNUSED)
{
   fail_if(!eina_barrier_wait(&barrier));
   fail_if(eina_lock_take(&mtcond) != EINA_LOCK_SUCCEED);
   fail_if(!eina_condition_broadcast(&cond));
   fail_if(eina_lock_release(&mtcond) != EINA_LOCK_SUCCEED);

   fail_if(eina_rwlock_take_write(&mutex) != EINA_LOCK_SUCCEED);
   counter = 7200;
   fail_if(eina_rwlock_release(&mutex) != EINA_LOCK_SUCCEED);

   fail_if(!eina_barrier_wait(&barrier));

   fail_if(eina_lock_take(&mtcond) != EINA_LOCK_SUCCEED);
   fail_if(!eina_condition_broadcast(&cond));
   fail_if(eina_lock_release(&mtcond) != EINA_LOCK_SUCCEED);

   return NULL;
}

EFL_START_TEST(eina_test_rwlock)
{
   struct timespec ts, ts2;
   long delay;


   fail_if(!eina_rwlock_new(&mutex));
   fail_if(!eina_lock_new(&mtcond));
   fail_if(!eina_condition_new(&cond, &mtcond));
   fail_if(!eina_barrier_new(&barrier, 2));

   counter = 42;

   eina_lock_debug(&mtcond);

   fail_if(eina_rwlock_take_read(&mutex) != EINA_LOCK_SUCCEED);
   fail_if(eina_lock_take(&mtcond) != EINA_LOCK_SUCCEED);

   fail_if(!eina_thread_create(&thread, EINA_THREAD_NORMAL, -1, _eina_test_rwlock_thread, NULL));

   fail_if(!eina_barrier_wait(&barrier));
   fail_if(!eina_condition_wait(&cond));
   fail_if(eina_lock_release(&mtcond) != EINA_LOCK_SUCCEED);

   fail_if(counter != 42);
   fail_if(eina_rwlock_release(&mutex) != EINA_LOCK_SUCCEED);

   fail_if(eina_lock_take(&mtcond) != EINA_LOCK_SUCCEED);
   fail_if(!eina_barrier_wait(&barrier));

   fail_if(!eina_condition_wait(&cond));
   fail_if(eina_lock_release(&mtcond) != EINA_LOCK_SUCCEED);

   fail_if(eina_rwlock_take_read(&mutex) != EINA_LOCK_SUCCEED);
   fail_if(counter != 7200);
   fail_if(eina_rwlock_release(&mutex) != EINA_LOCK_SUCCEED);

   fail_if(eina_lock_take(&mtcond) != EINA_LOCK_SUCCEED);
   clock_gettime(CLOCK_REALTIME, &ts);
   eina_condition_timedwait(&cond, 0.050);
   clock_gettime(CLOCK_REALTIME, &ts2);
   delay = (ts2.tv_sec - ts.tv_sec) * 1000L + (ts2.tv_nsec - ts.tv_nsec) / 1000000L;
   fail_if(delay < 50);
   fail_if(delay > 200);
   fail_if(eina_error_get() != ETIMEDOUT);
   fail_if(eina_lock_release(&mtcond) != EINA_LOCK_SUCCEED);

   eina_thread_join(thread);

   eina_condition_free(&cond);
   eina_lock_free(&mtcond);
   eina_rwlock_free(&mutex);

}
EFL_END_TEST

void
eina_test_locking(TCase *tc)
{
   tcase_add_test(tc, eina_test_spinlock);
   tcase_add_test(tc, eina_test_tls);
   tcase_add_test(tc, eina_test_rwlock);
}

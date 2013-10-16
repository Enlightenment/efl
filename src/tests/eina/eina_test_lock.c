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

#include "eina_suite.h"
#include "Eina.h"

static Eina_Spinlock spin;
static Eina_Thread thread;
static unsigned int counter;

static void *
_eina_test_lock_thread(void *data, Eina_Thread t)
{
   unsigned int i;

   fail_if(!eina_thread_equal(t, thread));
   fail_if(strcmp("test", data));

   for (i = 0; i < 10000; i++)
     {
        fail_if(eina_spinlock_take(&spin) != EINA_LOCK_SUCCEED);
        counter++;
        fail_if(eina_spinlock_release(&spin) != EINA_LOCK_SUCCEED);
     }

   return data;
}

START_TEST(eina_test_spinlock)
{
   unsigned int i;

   fail_if(!eina_init());

   counter = 0;
   fail_if(!eina_spinlock_new(&spin));

   fail_if(!eina_thread_create(&thread, EINA_THREAD_NORMAL, 0, _eina_test_lock_thread, "test"));

   for (i = 0; i < 10000; i++)
     {
        fail_if(eina_spinlock_take(&spin) != EINA_LOCK_SUCCEED);
        counter++;
        fail_if(eina_spinlock_release(&spin) != EINA_LOCK_SUCCEED);
     }

   fail_if(strcmp("test", eina_thread_join(thread)));

   fail_if(counter != 20000);

   eina_spinlock_free(&spin);

   eina_shutdown();
}
END_TEST

static Eina_TLS key;

static void *
_eina_test_tls_thread(void *data EINA_UNUSED, Eina_Thread t EINA_UNUSED)
{
   unsigned int mystack = 21;

   fail_if(!eina_tls_set(key, &mystack));
   fail_if(eina_tls_get(key) != &mystack);

   return NULL;
}

START_TEST(eina_test_tls)
{
   unsigned int ft = 42;

   fail_if(!eina_init());

   fail_if(!eina_tls_new(&key));

   fail_if(!eina_tls_set(key, &ft));

   fail_if(!eina_thread_create(&thread, EINA_THREAD_NORMAL, 0, _eina_test_tls_thread, NULL));

   eina_thread_join(thread);

   fail_if(eina_tls_get(key) != &ft);

   eina_tls_free(key);

   eina_shutdown();
}
END_TEST

static Eina_Lock mtcond;
static Eina_Condition cond;
static Eina_RWLock mutex;

static void *
_eina_test_rwlock_thread(void *data EINA_UNUSED, Eina_Thread t EINA_UNUSED)
{
   fail_if(!eina_condition_broadcast(&cond));

   fail_if(eina_rwlock_take_write(&mutex) != EINA_LOCK_SUCCEED);
   counter = 7200;
   fail_if(eina_rwlock_release(&mutex) != EINA_LOCK_SUCCEED);

   fail_if(!eina_condition_broadcast(&cond));

   return NULL;
}

START_TEST(eina_test_rwlock)
{
   fail_if(!eina_init());

   fail_if(!eina_rwlock_new(&mutex));
   fail_if(!eina_lock_new(&mtcond));
   fail_if(!eina_condition_new(&cond, &mtcond));

   counter = 42;

   fail_if(eina_rwlock_take_read(&mutex) != EINA_LOCK_SUCCEED);
   fail_if(eina_lock_take(&mtcond) != EINA_LOCK_SUCCEED);

   fail_if(!eina_thread_create(&thread, EINA_THREAD_NORMAL, 0, _eina_test_rwlock_thread, NULL));

   fail_if(!eina_condition_wait(&cond));
   fail_if(!eina_lock_release(&mtcond));

   fail_if(counter != 42);
   fail_if(eina_rwlock_release(&mutex) != EINA_LOCK_SUCCEED);

   fail_if(eina_lock_take(&mtcond) != EINA_LOCK_SUCCEED);
   fail_if(!eina_condition_wait(&cond));
   fail_if(eina_lock_release(&mtcond) != EINA_LOCK_SUCCEED);
   
   fail_if(eina_rwlock_take_read(&mutex) != EINA_LOCK_SUCCEED);
   fail_if(counter != 7200);
   fail_if(eina_rwlock_release(&mutex) != EINA_LOCK_SUCCEED);

   eina_condition_free(&cond);
   eina_lock_free(&mtcond);
   eina_rwlock_free(&mutex);

   eina_shutdown();
}
END_TEST

void
eina_test_locking(TCase *tc)
{
   tcase_add_test(tc, eina_test_spinlock);
   tcase_add_test(tc, eina_test_tls);
   tcase_add_test(tc, eina_test_rwlock);
}

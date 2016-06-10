/* EINA - EFL data type library
 * Copyright (C) 2016 Cedric Bail
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

#include <Eina.h>

#include "eina_suite.h"

static int test_array[1024] = { 0, 1, 2, 3, 4, 5, 6, 7 };
static int test_array2[1024] = { 8, 9, 10, 11, 12, 13, 14 };
static const void *pointers[2048] = { NULL };

START_TEST(eina_test_reusable)
{
   unsigned int i;

   eina_init();

   for (i = 0; i < sizeof (test_array) / sizeof (test_array[0]); i++)
     {
        pointers[i] = eina_safepointer_register(&test_array[i]);
        fail_if(pointers[i] == NULL);
        fail_if(pointers[i] == &test_array[i]);
        fail_if(&test_array[i] != eina_safepointer_get(pointers[i]));
     }

   for (i = 0; i < sizeof (test_array) / sizeof (test_array[0]); i++)
     {
        eina_safepointer_unregister(pointers[i]);
     }

   for (i = 0; i < sizeof (test_array2) / sizeof (test_array2[0]); i++)
     {
        pointers[i + (sizeof (test_array) / sizeof (test_array[0]))] = eina_safepointer_register(&test_array2[i]);
        fail_if(pointers[i + (sizeof (test_array) / sizeof (test_array[0]))] == NULL);
        fail_if(pointers[i + (sizeof (test_array) / sizeof (test_array[0]))] == &test_array2[i]);
        fail_if(&test_array2[i] != eina_safepointer_get(pointers[i + (sizeof (test_array) / sizeof (test_array[0]))]));
        eina_safepointer_unregister(pointers[i + (sizeof (test_array) / sizeof (test_array[0]))]);
     }

   for (i = 0; i < sizeof (pointers) / sizeof (pointers[0]); i++)
     {
        unsigned int j;

        for (j = i + 1; j < sizeof (pointers) / sizeof (pointers[0]); j++)
          {
             fail_if(pointers[j] == pointers[i]);
          }
     }

   eina_shutdown();
}
END_TEST

static Eina_Barrier b;

static void *
_thread1(void *data EINA_UNUSED, Eina_Thread t EINA_UNUSED)
{
   unsigned int i;

   fail_if(!eina_barrier_wait(&b));

   for (i = 0; i < sizeof (test_array) / sizeof (test_array[0]); i++)
     {
        pointers[i] = eina_safepointer_register(&test_array[i]);
        fail_if(pointers[i] == NULL);
        fail_if(pointers[i] == &test_array[i]);
        fail_if(&test_array[i] != eina_safepointer_get(pointers[i]));
     }

   for (i = 0; i < sizeof (test_array) / sizeof (test_array[0]); i++)
     {
        eina_safepointer_unregister(pointers[i]);
     }

   return NULL;
}

static void *
_thread2(void *data EINA_UNUSED, Eina_Thread t EINA_UNUSED)
{
   unsigned int i;

   fail_if(!eina_barrier_wait(&b));

   for (i = 0; i < sizeof (test_array2) / sizeof (test_array2[0]); i++)
     {
        pointers[i + (sizeof (test_array) / sizeof (test_array[0]))] = eina_safepointer_register(&test_array2[i]);
        fail_if(pointers[i + (sizeof (test_array) / sizeof (test_array[0]))] == NULL);
        fail_if(pointers[i + (sizeof (test_array) / sizeof (test_array[0]))] == &test_array2[i]);
        fail_if(&test_array2[i] != eina_safepointer_get(pointers[i + (sizeof (test_array) / sizeof (test_array[0]))]));
        eina_safepointer_unregister(pointers[i + (sizeof (test_array) / sizeof (test_array[0]))]);
     }

   return NULL;
}

START_TEST(eina_test_threading)
{
   Eina_Thread t1, t2;
   unsigned int i;

   eina_init();

   // We need a barrier so that both thread are more likely
   // to start running in parallel
   fail_if(!eina_barrier_new(&b, 2));

   // Spawn them
   fail_if(!eina_thread_create(&t1, 0, 0, _thread1, NULL));
   fail_if(!eina_thread_create(&t2, 0, 0, _thread2, NULL));

   // And wait for the outcome !
   eina_thread_join(t1);
   eina_thread_join(t2);

   eina_barrier_free(&b);

   for (i = 0; i < sizeof (pointers) / sizeof (pointers[0]); i++)
     {
        unsigned int j;

        for (j = i + 1; j < sizeof (pointers) / sizeof (pointers[0]); j++)
          {
             fail_if(pointers[j] == pointers[i]);
          }
     }

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_lowestbit)
{
   unsigned int i;

   eina_init();

   for (i = 0; i < sizeof (test_array) / sizeof (test_array[0]); i++)
     {
        pointers[i] = eina_safepointer_register(&test_array[i]);
        fail_if(pointers[i] == NULL);
        fail_if(pointers[i] == &test_array[i]);
        fail_if(&test_array[i] != eina_safepointer_get(pointers[i]));

        // We do guaranty that the two lower bit are always zero and will be internally ignored
        fail_if((((uintptr_t) pointers[i]) & 0x3) != 0);
        pointers[i] = (void*)(((uintptr_t) pointers[i]) | 0x3);

        fail_if(&test_array[i] != eina_safepointer_get(pointers[i]));

        eina_safepointer_unregister(pointers[i]);

        fail_if(eina_safepointer_get(pointers[i]) != NULL);
     }

   eina_shutdown();
}
END_TEST

void
eina_test_safepointer(TCase *tc)
{
   tcase_add_test(tc, eina_test_reusable);
   tcase_add_test(tc, eina_test_threading);
   tcase_add_test(tc, eina_test_lowestbit);
}

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

static int test_array[128] = { 0, 1, 2, 3, 4, 5, 6, 7 };
static int test_array2[64] = { 8, 9, 10, 11, 12, 13, 14 };
static const void *pointers[EINA_C_ARRAY_LENGTH(test_array) + EINA_C_ARRAY_LENGTH(test_array2)] = { NULL };

EFL_START_TEST(eina_test_safepointer_reusable)
{
   unsigned int i;


   for (i = 0; i < EINA_C_ARRAY_LENGTH(test_array); i++)
     {
        const void *ptr = pointers[i] = eina_safepointer_register(&test_array[i]);
        ck_assert_ptr_ne(ptr, NULL);
        ck_assert_ptr_ne(ptr, &test_array[i]);
        ck_assert_ptr_eq(&test_array[i], eina_safepointer_get(pointers[i]));
     }

   for (i = 0; i < EINA_C_ARRAY_LENGTH(test_array2); i++)
     {
        const void *ptr = pointers[i + EINA_C_ARRAY_LENGTH(test_array)] = eina_safepointer_register(&test_array2[i]);
        ck_assert_ptr_ne(ptr, NULL);
        ck_assert_ptr_ne(ptr, &test_array2[i]);
        ck_assert_ptr_eq(&test_array2[i], eina_safepointer_get(ptr));
        eina_safepointer_unregister(ptr);
     }

   for (i = 0; i < EINA_C_ARRAY_LENGTH(test_array); i++)
     {
        eina_safepointer_unregister(pointers[i]);
     }

   for (i = 0; i < EINA_C_ARRAY_LENGTH(pointers); i++)
     {
        unsigned int j;

        for (j = i + 1; j < EINA_C_ARRAY_LENGTH(pointers); j++)
          {
             ck_assert_ptr_ne(pointers[j], pointers[i]);
          }
     }

}
EFL_END_TEST

static Eina_Barrier b;

static void *
_thread1(void *data EINA_UNUSED, Eina_Thread t EINA_UNUSED)
{
   unsigned int i;

   ck_assert_int_ne(eina_barrier_wait(&b), 0);

   for (i = 0; i < EINA_C_ARRAY_LENGTH(test_array); i++)
     {
        const void *ptr = pointers[i] = eina_safepointer_register(&test_array[i]);
        ck_assert_ptr_ne(ptr, NULL);
        ck_assert_ptr_ne(ptr, &test_array[i]);
        ck_assert_ptr_eq(&test_array[i], eina_safepointer_get(ptr));
     }

   return NULL;
}

static void *
_thread2(void *data EINA_UNUSED, Eina_Thread t EINA_UNUSED)
{
   unsigned int i;

   ck_assert_int_ne(eina_barrier_wait(&b), 0);

   for (i = 0; i < EINA_C_ARRAY_LENGTH(test_array2); i++)
     {
        const void *ptr = pointers[i + (EINA_C_ARRAY_LENGTH(test_array))] = eina_safepointer_register(&test_array2[i]);
        ck_assert_ptr_ne(ptr, NULL);
        ck_assert_ptr_ne(ptr, &test_array2[i]);
        ck_assert_ptr_eq(&test_array2[i], eina_safepointer_get(ptr));
        eina_safepointer_unregister(ptr);
     }

   return NULL;
}

EFL_START_TEST(eina_test_safepointer_threading)
{
   Eina_Thread t1, t2;
   unsigned int i;


   // We need a barrier so that both thread are more likely
   // to start running in parallel
   ck_assert_int_ne(eina_barrier_new(&b, 2), 0);

   // Spawn them
   ck_assert_int_ne(eina_thread_create(&t1, EINA_THREAD_NORMAL, -1, _thread1, NULL), 0);
   ck_assert_int_ne(eina_thread_create(&t2, EINA_THREAD_NORMAL, -1, _thread2, NULL), 0);

   // And wait for the outcome !
   eina_thread_join(t1);
   eina_thread_join(t2);

   for (i = 0; i < EINA_C_ARRAY_LENGTH(test_array); i++)
     {
        eina_safepointer_unregister(pointers[i]);
     }

   eina_barrier_free(&b);

   for (i = 0; i < EINA_C_ARRAY_LENGTH(pointers); i++)
     {
        unsigned int j;

        for (j = i + 1; j < EINA_C_ARRAY_LENGTH(pointers); j++)
          {
             ck_assert_ptr_ne(pointers[j], pointers[i]);
          }
     }

}
EFL_END_TEST

EFL_START_TEST(eina_test_safepointer_lowestbit)
{
   unsigned int i;


   for (i = 0; i < EINA_C_ARRAY_LENGTH(test_array); i++)
     {
        const void *ptr = eina_safepointer_register(&test_array[i]);
        ck_assert_ptr_ne(ptr, NULL);
        ck_assert_ptr_ne(ptr, &test_array[i]);
        ck_assert_ptr_eq(&test_array[i], eina_safepointer_get(ptr));

        // We do guarantee that the two lower bit are always zero and will be internally ignored
        ck_assert_int_eq((((uintptr_t) ptr) & 0x3), 0);
        ptr = (void*)(((uintptr_t) ptr) | 0x3);

        ck_assert_ptr_eq(&test_array[i], eina_safepointer_get(ptr));

        eina_safepointer_unregister(ptr);

        ck_assert_ptr_eq(eina_safepointer_get(ptr), NULL);
     }
}
EFL_END_TEST

void
eina_test_safepointer(TCase *tc)
{
   tcase_add_test(tc, eina_test_safepointer_reusable);
   tcase_add_test(tc, eina_test_safepointer_threading);
   tcase_add_test(tc, eina_test_safepointer_lowestbit);
}

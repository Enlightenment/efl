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
#include <string.h>

#include <Eina.h>
#include "eina_hash.h"

#include "eina_suite.h"

#define EINA_HASH_BUCKET_SIZE 8

static unsigned int
_eina_string_key_length(const char *key)
{
   if (!key)
     return 0;

   return (int)strlen(key) + 1;
}

static int
_eina_string_key_cmp(const char *key1, int key1_length,
                     const char *key2, int key2_length)
{
   int delta;

   delta = key1_length - key2_length;
   if (delta) return delta;
   return strcmp(key1, key2);
}

static Eina_Bool
eina_foreach_check(EINA_UNUSED const Eina_Hash *hash,
                   const void *key,
                   void *data,
                   EINA_UNUSED void *fdata)
{
   int *j = data;
   int i;

   if (strlen(key) <= 0)
     return EINA_TRUE;

   i = atoi(key);
   fail_if(i != *j);

   return EINA_TRUE;
}

EFL_START_TEST(eina_test_hash_simple)
{
   Eina_Hash *hash = NULL;
   int *test;
   int array[] = { 1, 42, 4, 5, 6 };

   /* As mempool is already initialized and it use hash, we should have 2 init. */
   hash = eina_hash_string_superfast_new(NULL);
   fail_if(hash == NULL);

   fail_if(eina_hash_add(hash, "1", &array[0]) != EINA_TRUE);
   fail_if(eina_hash_add(hash, "42", &array[1]) != EINA_TRUE);
   fail_if(eina_hash_direct_add(hash, "4", &array[2]) != EINA_TRUE);
   fail_if(eina_hash_direct_add(hash, "5", &array[3]) != EINA_TRUE);
   fail_if(eina_hash_add(hash, "", "") != EINA_TRUE);

   test = eina_hash_find(hash, "4");
   fail_if(!test);
   fail_if(*test != 4);

   test = eina_hash_find(hash, "42");
   fail_if(!test);
   fail_if(*test != 42);

   eina_hash_foreach(hash, eina_foreach_check, NULL);

   test = eina_hash_modify(hash, "5", &array[4]);
   fail_if(!test);
   fail_if(*test != 5);

   test = eina_hash_find(hash, "5");
   fail_if(!test);
   fail_if(*test != 6);

   fail_if(eina_hash_population(hash) != 5);

   fail_if(eina_hash_find(hash, "120") != NULL);

   fail_if(eina_hash_del(hash, "5", NULL) != EINA_TRUE);
   fail_if(eina_hash_find(hash, "5") != NULL);

   fail_if(eina_hash_del(hash, NULL, &array[2]) != EINA_TRUE);
   fail_if(eina_hash_find(hash, "4") != NULL);

   fail_if(eina_hash_del(hash, NULL, &array[2]) != EINA_FALSE);

   fail_if(eina_hash_del(hash, "1", NULL) != EINA_TRUE);
   fail_if(eina_hash_del(hash, "42", NULL) != EINA_TRUE);

   eina_hash_free(hash);
}
EFL_END_TEST

EFL_START_TEST(eina_test_hash_crc)
{
   Eina_Hash *hash = NULL;
   int *test;
   int array[] = { 1, 42, 4, 5, 6 };

   hash = eina_hash_new(EINA_KEY_LENGTH(_eina_string_key_length),
                        EINA_KEY_CMP(_eina_string_key_cmp),
                        EINA_KEY_HASH(eina_hash_crc),
                        NULL,
                        EINA_HASH_BUCKET_SIZE);
   fail_if(hash == NULL);

   fail_if(eina_hash_add(hash, "1", &array[0]) != EINA_TRUE);
   fail_if(eina_hash_add(hash, "42", &array[1]) != EINA_TRUE);
   fail_if(eina_hash_direct_add(hash, "4", &array[2]) != EINA_TRUE);
   fail_if(eina_hash_direct_add(hash, "5", &array[3]) != EINA_TRUE);
   fail_if(eina_hash_add(hash, "", "") != EINA_TRUE);

   test = eina_hash_find(hash, "4");
   fail_if(!test);
   fail_if(*test != 4);

   test = eina_hash_find(hash, "42");
   fail_if(!test);
   fail_if(*test != 42);

   eina_hash_foreach(hash, eina_foreach_check, NULL);

   test = eina_hash_modify(hash, "5", &array[4]);
   fail_if(!test);
   fail_if(*test != 5);

   test = eina_hash_find(hash, "5");
   fail_if(!test);
   fail_if(*test != 6);

   fail_if(eina_hash_population(hash) != 5);

   fail_if(eina_hash_find(hash, "120") != NULL);

   fail_if(eina_hash_del(hash, "5", NULL) != EINA_TRUE);
   fail_if(eina_hash_find(hash, "5") != NULL);

   fail_if(eina_hash_del(hash, NULL, &array[2]) != EINA_TRUE);
   fail_if(eina_hash_find(hash, "4") != NULL);

   fail_if(eina_hash_del(hash, NULL, &array[2]) != EINA_FALSE);

   fail_if(eina_hash_del(hash, "1", NULL) != EINA_TRUE);
   fail_if(eina_hash_del(hash, "42", NULL) != EINA_TRUE);

   eina_hash_free(hash);
}
EFL_END_TEST

EFL_START_TEST(eina_test_hash_extended)
{
   Eina_Hash *hash = NULL;
   unsigned int i;
   unsigned int num_loops = 3011;
   char *array;

   hash = eina_hash_string_djb2_new(NULL);
   fail_if(hash == NULL);

   fail_if(eina_hash_direct_add(hash, "42", "42") != EINA_TRUE);

   array = malloc(num_loops * 10);
   ck_assert_ptr_ne(array, NULL);

   for (i = 0; i < num_loops; ++i)
     {
        char *tmp = array + (i * 10);
        eina_convert_itoa(i + 42, tmp);
        fail_if(eina_hash_direct_add(hash, tmp, tmp) != EINA_TRUE);
     }

   fail_if(eina_hash_find(hash, "42") == NULL);

   eina_hash_free(hash);
   free(array);
}
EFL_END_TEST

EFL_START_TEST(eina_test_hash_double_item)
{
   Eina_Hash *hash = NULL;
   int i[] = { 7, 7 };
   int *test;

   hash = eina_hash_string_superfast_new(NULL);
   fail_if(hash == NULL);

   fail_if(eina_hash_add(hash, "7", &i[0]) != EINA_TRUE);
   fail_if(eina_hash_add(hash, "7", &i[1]) != EINA_TRUE);

   fail_if(eina_hash_del(hash, "7", &i[1]) != EINA_TRUE);
   test = eina_hash_find(hash, "7");
   fail_if(test != &i[0]);

   eina_hash_free(hash);
}
EFL_END_TEST

EFL_START_TEST(eina_test_hash_all_int)
{
   Eina_Hash *hash;
   int64_t j[] = { 4321312301243122, 6, 7, 128 };
   int i[] = { 42, 6, 7, 0 };
   int64_t *test2;
   int *test;
   int it;

   hash = eina_hash_int32_new(NULL);
   fail_if(hash == NULL);

   for (it = 0; it < 4; ++it)
     fail_if(eina_hash_add(hash, &i[it], &i[it]) != EINA_TRUE);

   fail_if(eina_hash_del(hash, &i[1], &i[1]) != EINA_TRUE);
   test = eina_hash_find(hash, &i[2]);
   fail_if(test != &i[2]);

   test = eina_hash_find(hash, &i[3]);
   fail_if(test != &i[3]);

   eina_hash_free(hash);

   hash = eina_hash_int64_new(NULL);
   fail_if(hash == NULL);

   for (it = 0; it < 4; ++it)
     fail_if(eina_hash_add(hash, &j[it], &j[it]) != EINA_TRUE);

   fail_if(eina_hash_del(hash, &j[1], &j[1]) != EINA_TRUE);
   test2 = eina_hash_find(hash, &j[0]);
   fail_if(test2 != &j[0]);

   eina_hash_free(hash);
}
EFL_END_TEST

EFL_START_TEST(eina_test_hash_int32_fuzze)
{
   Eina_Hash *hash;
   unsigned int *r, *array;
   unsigned int i;
   unsigned int seed;
   unsigned int num_loops = 10000;

   seed = time(NULL);
   srand(seed);

   hash = eina_hash_int32_new(NULL);

   array = malloc(sizeof(int) * num_loops);
   for (i = 0; i < num_loops; ++i)
     {
        r = array + i;
        *r = rand() % 10000;
        ck_assert_int_ne(eina_hash_direct_add(hash, r, r), 0);
     }

   for (i = 0; i < num_loops / 2; ++i)
     {
        unsigned int tr = rand() + 10000;
        r = eina_hash_find(hash, &tr);
        ck_assert_ptr_eq(r, NULL);
     }

   for (i = 0; i < num_loops / 2; ++i)
     {
        unsigned int tr = (rand() % 10000) - (10000 * 2);
        r = eina_hash_find(hash, &tr);
        ck_assert_ptr_eq(r, NULL);
     }

   eina_hash_free(hash);
   free(array);
}
EFL_END_TEST

EFL_START_TEST(eina_test_hash_int64_fuzze)
{
   Eina_Hash *hash;
   uint64_t *r, *array;
   uint64_t i;
   uint64_t num_loops = 10000;

   srand(time(NULL));

   hash = eina_hash_int64_new(NULL);

   array = malloc(sizeof(int64_t) * num_loops);
   for (i = 0; i < num_loops; ++i)
     {
        r = array + i;
        *r = rand() % 10000;
        ck_assert_int_ne(eina_hash_direct_add(hash, r, r), 0);
     }

   for (i = 0; i < num_loops / 2; ++i)
     {
        uint64_t tr = rand() + 10000;
        r = eina_hash_find(hash, &tr);
        ck_assert_ptr_eq(r, NULL);
     }

   for (i = 0; i < num_loops / 2; ++i)
     {
        uint64_t tr = (rand() % 10000) - (10000 * 2);
        r = eina_hash_find(hash, &tr);
        ck_assert_ptr_eq(r, NULL);
     }

   eina_hash_free(hash);
   free(array);
}
EFL_END_TEST

EFL_START_TEST(eina_test_hash_string_fuzze)
{
   Eina_Hash *hash;
   unsigned int i;
   unsigned int seed;
   char *array;
   unsigned int num_loops = 100;

   seed = time(NULL);
   srand(seed);

   hash = eina_hash_string_superfast_new(NULL);

   array = malloc(num_loops * 10);
   ck_assert_ptr_ne(array, NULL);

   for (i = 0; i < num_loops; ++i)
     {
        char *tmp = array + (i * 10);
        eina_convert_itoa(i, tmp);
        fail_if(eina_hash_direct_add(hash, tmp, tmp) != EINA_TRUE);
     }

   for (i = 0; i < num_loops; ++i)
     {
        char convert[128];
        char *r;

        eina_convert_itoa(rand() + num_loops, convert);
        r = eina_hash_find(hash, convert);
        ck_assert_ptr_eq(r, NULL);
     }

   eina_hash_free(hash);
   free(array);
}
EFL_END_TEST

EFL_START_TEST(eina_test_hash_seed)
{
   fail_if(eina_seed == 0);
}
EFL_END_TEST

EFL_START_TEST(eina_test_hash_add_del_by_hash)
{
   Eina_Hash *hash = NULL;
   int array[] = { 1, 42, 4, 5, 6 };
   int key_len, key_hash;

   hash = eina_hash_new(EINA_KEY_LENGTH(_eina_string_key_length),
                        EINA_KEY_CMP(_eina_string_key_cmp),
                        EINA_KEY_HASH(eina_hash_crc),
                        NULL,
                        EINA_HASH_BUCKET_SIZE);
   fail_if(hash == NULL);
   fail_if(eina_hash_add(hash, "1", &array[0]) != EINA_TRUE);
   fail_if(eina_hash_add(hash, "42", &array[1]) != EINA_TRUE);
   fail_if(eina_hash_add(hash, "5", &array[3]) != EINA_TRUE);
   fail_if(eina_hash_add(hash, "6", &array[4]) != EINA_TRUE);

   key_len = _eina_string_key_length("4");
   key_hash = eina_hash_crc("4", key_len);

   fail_if(eina_hash_add_by_hash(hash, "4", key_len, key_hash, &array[2]) != EINA_TRUE);
   fail_if(eina_hash_del_by_hash(hash, "4", key_len, key_hash, &array[2]) != EINA_TRUE);
   fail_if(eina_hash_del_by_hash(hash, "4", key_len, key_hash, &array[2]) != EINA_FALSE);

   key_len = _eina_string_key_length("42");
   key_hash = eina_hash_crc("42", key_len);
   fail_if(eina_hash_del_by_hash(hash, "42", key_len, key_hash, &array[1]) != EINA_TRUE);

   fail_if(eina_hash_population(hash) != 3);
   eina_hash_free(hash);
}
EFL_END_TEST

void
eina_test_hash(TCase *tc)
{
   tcase_add_test(tc, eina_test_hash_simple);
   tcase_add_test(tc, eina_test_hash_crc);
   tcase_add_test(tc, eina_test_hash_extended);
   tcase_add_test(tc, eina_test_hash_double_item);
   tcase_add_test(tc, eina_test_hash_all_int);
   tcase_add_test(tc, eina_test_hash_seed);
   tcase_add_test(tc, eina_test_hash_int32_fuzze);
   tcase_add_test(tc, eina_test_hash_int64_fuzze);
   tcase_add_test(tc, eina_test_hash_string_fuzze);
   tcase_add_test(tc, eina_test_hash_add_del_by_hash);
}


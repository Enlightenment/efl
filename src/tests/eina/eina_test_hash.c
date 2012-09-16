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

#include "eina_suite.h"
#include "Eina.h"

static Eina_Bool
eina_foreach_check(__UNUSED__ const Eina_Hash *hash,
                   const void *key,
                   void *data,
                   __UNUSED__ void *fdata)
{
   int *j = data;
   int i;

   if (strlen(key) <= 0)
      return EINA_TRUE;

   i = atoi(key);
   fail_if(i != *j);

   return EINA_TRUE;
}

START_TEST(eina_hash_simple)
{
   Eina_Hash *hash = NULL;
   int *test;
   int array[] = { 1, 42, 4, 5, 6 };

   /* As mempool is already initialized and it use hash, we should have 2 init. */
   fail_if(eina_init() != 2);

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

   /* Same comment as eina_init */
        fail_if(eina_shutdown() != 1);
}
END_TEST

START_TEST(eina_hash_extended)
{
   Eina_Hash *hash = NULL;
   int i;

        fail_if(eina_init() != 2);

   hash = eina_hash_string_djb2_new(NULL);
        fail_if(hash == NULL);

        fail_if(eina_hash_direct_add(hash, "42", "42") != EINA_TRUE);

   for (i = 43; i < 3043; ++i)
     {
        char *tmp = malloc(10);
        fail_if(!tmp);
        eina_convert_itoa(i, tmp);
        fail_if(eina_hash_direct_add(hash, tmp, tmp) != EINA_TRUE);
     }

        fail_if(eina_hash_find(hash, "42") == NULL);

        eina_hash_free(hash);

   fail_if(eina_shutdown() != 1);
}
END_TEST

START_TEST(eina_hash_double_item)
{
   Eina_Hash *hash = NULL;
   int i[] = { 7, 7 };
   int *test;

   fail_if(eina_init() != 2);

   hash = eina_hash_string_superfast_new(NULL);
   fail_if(hash == NULL);

   fail_if(eina_hash_add(hash, "7", &i[0]) != EINA_TRUE);
   fail_if(eina_hash_add(hash, "7", &i[1]) != EINA_TRUE);

   fail_if(eina_hash_del(hash, "7", &i[1]) != EINA_TRUE);
   test = eina_hash_find(hash, "7");
   fail_if(test != &i[0]);

   eina_hash_free(hash);

      fail_if(eina_shutdown() != 1);
}
END_TEST

START_TEST(eina_hash_all_int)
{
   Eina_Hash *hash;
   int64_t j[] = { 4321312301243122, 6, 7, 128 };
   int i[] = { 42, 6, 7, 0 };
   int64_t *test2;
   int *test;
   int it;

      fail_if(eina_init() != 2);

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

   fail_if(eina_shutdown() != 1);
}
END_TEST

void eina_test_hash(TCase *tc)
{
   tcase_add_test(tc, eina_hash_simple);
   tcase_add_test(tc, eina_hash_extended);
   tcase_add_test(tc, eina_hash_double_item);
   tcase_add_test(tc, eina_hash_all_int);
}

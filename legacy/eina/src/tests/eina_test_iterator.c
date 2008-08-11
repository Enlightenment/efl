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

#include <stdio.h>

#include "eina_suite.h"
#include "eina_array.h"
#include "eina_hash.h"
#include "eina_private.h"

static Eina_Bool
eina_iterator_array_check(__UNUSED__ const Eina_Array *array,
			  int *data,  int *fdata)
{
   fail_if(*fdata > *data);
   *fdata = *data;

   return EINA_TRUE;
}

START_TEST(eina_iterator_array_simple)
{
   Eina_Iterator *it;
   Eina_Array *ea;
   int *tmp;
   int i;

   eina_array_init();

   ea = eina_array_new(11);
   fail_if(!ea);

   for (i = 0; i < 200; ++i)
     {
	tmp = malloc(sizeof(int));
	fail_if(!tmp);
	*tmp = i;

	eina_array_append(ea, tmp);
     }

   it = eina_array_iterator_new(ea);
   fail_if(!it);

   tmp = eina_iterator_data_get(it);
   fail_if(!tmp);
   fail_if(*tmp != 0);

   i = -1;
   eina_iterator_foreach(it, EINA_EACH(eina_iterator_array_check), &i);
   fail_if(i != 199);

   fail_if(eina_iterator_container_get(it) != ea);
   fail_if(eina_iterator_next(it) != EINA_FALSE);

   eina_iterator_free(it);

   eina_array_free(ea);

   eina_array_shutdown();
}
END_TEST

static Eina_Bool
eina_iterator_hash_key_check(const Eina_Hash *hash, const char *key, __UNUSED__ void *fdata)
{
   fail_if(eina_hash_find(hash, key) == NULL);

   return EINA_TRUE;
}
static Eina_Bool
eina_iterator_hash_data_check(const Eina_Hash *hash, int *data, __UNUSED__ void *fdata)
{
   char tmp[10];

   snprintf(tmp, 10, "%i", *data);
   fail_if(eina_hash_find(hash, tmp) != data);

   return EINA_TRUE;
}
static Eina_Bool
eina_iterator_hash_tuple_check(__UNUSED__ const Eina_Hash *hash, Eina_Hash_Tuple *tuple, __UNUSED__ void *fdata)
{
   fail_if(atoi((char*) tuple->key) != *((int*) tuple->data));

   return EINA_TRUE;
}

START_TEST(eina_iterator_hash_simple)
{
   Eina_Iterator *it;
   Eina_Hash *hash;
   int array[] = { 1, 42, 7, 8, 6 };

   eina_hash_init();

   hash = eina_hash_string_superfast_new();
   fail_if(hash == NULL);

   fail_if(eina_hash_add(hash, "1", &array[0]) != EINA_TRUE);
   fail_if(eina_hash_add(hash, "42", &array[1]) != EINA_TRUE);
   fail_if(eina_hash_add(hash, "7", &array[2]) != EINA_TRUE);
   fail_if(eina_hash_add(hash, "8", &array[3]) != EINA_TRUE);
   fail_if(eina_hash_add(hash, "6", &array[4]) != EINA_TRUE);

   it = eina_hash_iterator_key_new(hash);
   eina_iterator_foreach(it, EINA_EACH(eina_iterator_hash_key_check), NULL);
   eina_iterator_free(it);

   it = eina_hash_iterator_data_new(hash);
   eina_iterator_foreach(it, EINA_EACH(eina_iterator_hash_data_check), NULL);
   eina_iterator_free(it);

   it = eina_hash_iterator_tuple_new(hash);
   eina_iterator_foreach(it, EINA_EACH(eina_iterator_hash_tuple_check), NULL);
   eina_iterator_free(it);

   eina_hash_free(hash);

   eina_hash_shutdown();
}
END_TEST

void
eina_test_iterator(TCase *tc)
{
   tcase_add_test(tc, eina_iterator_array_simple);
   tcase_add_test(tc, eina_iterator_hash_simple);
}

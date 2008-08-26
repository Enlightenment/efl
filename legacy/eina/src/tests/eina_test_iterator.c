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
#include "eina_inlist.h"
#include "eina_list.h"
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

	eina_array_push(ea, tmp);
     }

   it = eina_array_iterator_new(ea);
   fail_if(!it);

   i = -1;
   eina_iterator_foreach(it, EINA_EACH(eina_iterator_array_check), &i);
   fail_if(i != 199);

   fail_if(eina_iterator_container_get(it) != ea);
   fail_if(eina_iterator_next(it, (void**) &tmp) != EINA_FALSE);

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

typedef struct _Eina_Test_Inlist Eina_Test_Inlist;
struct _Eina_Test_Inlist
{
   Eina_Inlist list;
   int i;
};

static Eina_Test_Inlist*
_eina_test_inlist_build(int i)
{
   Eina_Test_Inlist *tmp;

   tmp = malloc(sizeof(Eina_Test_Inlist));
   fail_if(!tmp);
   tmp->i = i;

   return tmp;
}

static Eina_Bool
eina_iterator_inlist_data_check(__UNUSED__ const Eina_Inlist *in_list, Eina_Test_Inlist *data, int *fdata)
{
   switch (*fdata)
     {
      case 0: fail_if(data->i != 27); break;
      case 1: fail_if(data->i != 42); break;
      case 2: fail_if(data->i != 3227); break;
      case 3: fail_if(data->i != 1664); break;
      case 4: fail_if(data->i != 81); break;
     }

   (*fdata)++;

   return EINA_TRUE;
}

START_TEST(eina_iterator_inlist_simple)
{
   Eina_Test_Inlist *lst = NULL;
   Eina_Test_Inlist *tmp;
   Eina_Test_Inlist *prev;
   Eina_Iterator *it;
   int i = 0;

   tmp = _eina_test_inlist_build(42);
   lst = eina_inlist_append(lst, tmp);
   fail_if(!lst);

   tmp = _eina_test_inlist_build(1664);
   lst = eina_inlist_append_relative(lst, tmp, lst);
   fail_if(!lst);
   fail_if(lst->i != 42);

   prev = tmp;
   tmp = _eina_test_inlist_build(3227);
   lst = eina_inlist_prepend_relative(lst, tmp, prev);
   fail_if(!lst);
   fail_if(lst->i != 42);

   tmp = _eina_test_inlist_build(27);
   lst = eina_inlist_prepend_relative(lst, tmp, NULL);

   tmp = _eina_test_inlist_build(81);
   lst = eina_inlist_append_relative(lst, tmp, NULL);

   it = eina_inlist_iterator_new(lst);
   fail_if(!it);

   eina_iterator_foreach(it, EINA_EACH(eina_iterator_inlist_data_check), &i);
   eina_iterator_free(it);

   fail_if(i != 5);
}
END_TEST

static Eina_Bool
eina_iterator_list_data_check(__UNUSED__ const Eina_List *list, int *data, int *fdata)
{
   switch (*fdata)
     {
      case 0: fail_if(*data != 81); break;
      case 1: fail_if(*data != 7); break;
      case 2: fail_if(*data != 9); break;
      case 3: fail_if(*data != 6); break;
      case 4: fail_if(*data != 42); break;
      case 5: fail_if(*data != 1); break;
      case 6: fail_if(*data != 1337); break;
     }

   (*fdata)++;

   return EINA_TRUE;
}

START_TEST(eina_iterator_list_simple)
{
   Eina_List *list = NULL;
   Eina_Iterator *it;
   int data[] = { 6, 9, 42, 1, 7, 1337, 81, 1664 };
   int i = 0;

   eina_list_init();

   list = eina_list_append(list, &data[0]);
   fail_if(list == NULL);

   list = eina_list_prepend(list, &data[1]);
   fail_if(list == NULL);

   list = eina_list_append(list, &data[2]);
   fail_if(list == NULL);

   list = eina_list_append(list, &data[3]);
   fail_if(list == NULL);

   list = eina_list_prepend(list, &data[4]);
   fail_if(list == NULL);

   list = eina_list_append(list, &data[5]);
   fail_if(list == NULL);

   list = eina_list_prepend(list, &data[6]);
   fail_if(list == NULL);

   it = eina_list_iterator_new(list);
   fail_if(!it);

   eina_iterator_foreach(it, EINA_EACH(eina_iterator_list_data_check), &i);
   eina_iterator_free(it);
}
END_TEST

void
eina_test_iterator(TCase *tc)
{
   tcase_add_test(tc, eina_iterator_array_simple);
   tcase_add_test(tc, eina_iterator_hash_simple);
   tcase_add_test(tc, eina_iterator_inlist_simple);
   tcase_add_test(tc, eina_iterator_list_simple);
}

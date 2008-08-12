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
#include "eina_inlist.h"
#include "eina_private.h"

static Eina_Bool
eina_accessor_check(__UNUSED__ const Eina_Array *array,
		    int *data,  int *fdata)
{
   fail_if(*fdata > *data);
   *fdata = *data;

   return EINA_TRUE;
}

START_TEST(eina_accessor_array_simple)
{
   Eina_Accessor *it;
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

   it = eina_array_accessor_new(ea);
   fail_if(!it);

   fail_if(eina_accessor_data_get(it, 100, (void**) &tmp) != EINA_TRUE);
   fail_if(!tmp);
   fail_if(*tmp != 100);

   i = 50;
   eina_accessor_over(it, EINA_EACH(eina_accessor_check), 50, 100, &i);
   fail_if(i != 99);

   fail_if(eina_accessor_container_get(it) != ea);

   eina_accessor_free(it);

   eina_array_free(ea);

   eina_array_shutdown();
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
eina_accessor_inlist_data_check(__UNUSED__ const Eina_Inlist *in_list, Eina_Test_Inlist *data, int *fdata)
{
   switch (*fdata)
     {
      case 0: fail_if(data->i != 3227); break;
      case 1: fail_if(data->i != 1664); break;
     }

   (*fdata)++;

   return EINA_TRUE;
}

START_TEST(eina_accessor_inlist_simple)
{
   Eina_Test_Inlist *lst = NULL;
   Eina_Test_Inlist *tmp;
   Eina_Test_Inlist *prev;
   Eina_Accessor *it;
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

   tmp = _eina_test_inlist_build(7);
   lst = eina_inlist_append(lst, tmp);

   it = eina_inlist_accessor_new(lst);
   fail_if(!it);
   fail_if(eina_accessor_container_get(it) != lst);

   eina_accessor_over(it, EINA_EACH(eina_accessor_inlist_data_check), 2, 4, &i);

   fail_if(eina_accessor_data_get(it, 5, (void**) &tmp) != EINA_TRUE);
   fail_if(eina_accessor_data_get(it, 3, (void**) &tmp) != EINA_TRUE);
   fail_if(tmp->i != 1664);
   fail_if(eina_accessor_data_get(it, 3, (void**) &tmp) != EINA_TRUE);
   fail_if(tmp->i != 1664);
   fail_if(eina_accessor_data_get(it, 1, (void**) &tmp) != EINA_TRUE);
   fail_if(tmp->i != 42);

   eina_accessor_free(it);

   fail_if(i != 2);
}
END_TEST


void
eina_test_accessor(TCase *tc)
{
   tcase_add_test(tc, eina_accessor_array_simple);
   tcase_add_test(tc, eina_accessor_inlist_simple);
}

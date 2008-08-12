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

#include <stdlib.h>

#include "eina_suite.h"
#include "eina_inlist.h"

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

START_TEST(eina_inlist_simple)
{
   Eina_Test_Inlist *lst = NULL;
   Eina_Test_Inlist *tmp;
   Eina_Test_Inlist *prev;
   int i = 0;

   tmp = _eina_test_inlist_build(42);
   lst = eina_inlist_append(lst, tmp);
   fail_if(!lst);

   lst = eina_inlist_remove(lst, tmp);
   lst = eina_inlist_prepend(lst, tmp);

   tmp = _eina_test_inlist_build(1664);
   lst = eina_inlist_append_relative(lst, tmp, lst);
   fail_if(!lst);
   fail_if(lst->i != 42);

   prev = tmp;
   tmp = _eina_test_inlist_build(3227);
   lst = eina_inlist_prepend_relative(lst, tmp, prev);
   fail_if(!lst);
   fail_if(lst->i != 42);

   lst = eina_inlist_remove(lst, tmp);

   lst = eina_inlist_append_relative(lst, tmp, lst);
   lst = eina_inlist_remove(lst, tmp);

   lst = eina_inlist_prepend_relative(lst, tmp, lst);

   tmp = _eina_test_inlist_build(27);
   lst = eina_inlist_prepend_relative(lst, tmp, NULL);

   tmp = _eina_test_inlist_build(81);
   lst = eina_inlist_append_relative(lst, tmp, NULL);

   EINA_INLIST_ITER_NEXT(lst, tmp)
     {
	switch (i)
	  {
	   case 0: fail_if(tmp->i != 27); break;
	   case 1: fail_if(tmp->i != 3227); break;
	   case 2: fail_if(tmp->i != 42); break;
	   case 3: fail_if(tmp->i != 1664); break;
	   case 4: fail_if(tmp->i != 81); break;
	  }

	++i;
     }

   eina_inlist_remove(NULL, tmp);
   lst = eina_inlist_remove(lst, NULL);

   tmp = eina_inlist_find(lst, prev);
   eina_inlist_remove(lst, tmp);
   tmp = eina_inlist_find(lst, tmp);
   fail_if(tmp != NULL);

   while (lst)
     lst = eina_inlist_remove(lst, lst);
}
END_TEST

void
eina_test_inlist(TCase *tc)
{
   tcase_add_test(tc, eina_inlist_simple);
}

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

#include <stdio.h>

#include "eina_list.h"
#include "eina_suite.h"

static int eina_int_cmp(const void *a, const void *b)
{
   const int *ia = a;
   const int *ib = b;

   return *ia - *ib;
}

START_TEST(eina_test_simple)
{
   Eina_List *list = NULL;
   Eina_List *tmp;
   int *test1;
   int *test2;
   int *test3;
   int data[] = { 6, 9, 42, 1, 7, 9, 81, 1664, 1337 };
   int result[] = { 81, 9, 9, 7, 1 };
   int i;

   eina_list_init();

   list = eina_list_append(list, &data[0]);
   fail_if(list == NULL);

   list = eina_list_prepend(list, &data[1]);
   fail_if(list == NULL);

   list = eina_list_append(list, &data[2]);
   fail_if(list == NULL);

   list = eina_list_remove(list, &data[0]);
   fail_if(list == NULL);

   list = eina_list_remove(list, &data[0]);
   fail_if(list == NULL);

   tmp = eina_list_data_find_list(list, &data[2]);
   fail_if(tmp == NULL);

   list = eina_list_append_relative_list(list, &data[3], tmp);
   fail_if(list == NULL);

   list = eina_list_prepend_relative_list(list, &data[4], tmp);
   fail_if(list == NULL);

   list = eina_list_promote_list(list, tmp);
   fail_if(list == NULL);

   list = eina_list_append_relative(list, &data[5], &data[2]);
   fail_if(list == NULL);

   list = eina_list_prepend_relative(list, &data[6], &data[2]);
   fail_if(list == NULL);

   list = eina_list_remove_list(list, tmp);
   fail_if(list == NULL);

   fail_if(eina_list_data_find_list(list, &data[2]) != NULL);
   fail_if(eina_list_data_find(list, &data[2]) != NULL);
   fail_if(eina_list_data_find(list, &data[5]) != &data[5]);

   fail_if(eina_list_count(list) != 5);
   fail_if(eina_list_nth(list, 4) != &data[3]);
   fail_if(eina_list_nth(list, 10) != NULL);
   fail_if(eina_list_nth_list(list, 10) != NULL);

   for (tmp = list, i = 0; tmp != NULL; tmp = eina_list_next(tmp), ++i)
     {
	int *data;

	data = eina_list_data_get(tmp);
	fail_if(data == NULL);
	fail_if(*data != result[i]);
     }

   list = eina_list_reverse(list);

   for (tmp = list; tmp != NULL; tmp = eina_list_next(tmp), --i)
     {
	int *data;

	data = eina_list_data_get(tmp);
	fail_if(data == NULL);
	fail_if(*data != result[i - 1]);
     }

   list = eina_list_append_relative(list, &data[7], &data[7]);
   fail_if(list == NULL);

   list = eina_list_prepend_relative(list, &data[8], &data[8]);
   fail_if(list == NULL);

   list = eina_list_sort(list, 2, eina_int_cmp);

   test1 = eina_list_nth(list, 0);
   test2 = eina_list_nth(list, 1);
   test3 = eina_list_nth(list, 2);

   fail_if(test1 == NULL || test2 == NULL || test3 == NULL);
   fail_if(*test1 > *test2);
   fail_if(*test3 == *test2);

   list = eina_list_append(list, &data[8]);
   fail_if(list == NULL);

   list = eina_list_append(list, &data[7]);
   fail_if(list == NULL);

   list = eina_list_sort(list, -1, eina_int_cmp);

   test1 = eina_list_nth(list, 0);
   for (tmp = list; tmp != NULL; tmp = eina_list_next(tmp))
     {
	int *data;

	data = eina_list_data_get(tmp);
	fail_if(*test1 > *data);

	test1 = data;
     }

   test3 = eina_list_nth(list, 5);
   fail_if(test3 == NULL);

   list = eina_list_promote_list(list, list);
   fail_if(list == NULL);

   list = eina_list_promote_list(list, eina_list_last(list));
   fail_if(list == NULL);

   test1 = eina_list_nth(list, 0);
   test2 = eina_list_nth(list, 1);

   list = eina_list_promote_list(eina_list_next(list), list);
   fail_if(list == NULL);
   fail_if(eina_list_data_get(list) != test1);
   fail_if(eina_list_data_get(eina_list_next(list)) != test2);

   list = eina_list_remove_list(list, list);
   fail_if(list == NULL);

   list = eina_list_remove_list(list, eina_list_last(list));
   fail_if(list == NULL);

   list = eina_list_free(list);
   fail_if(list != NULL);

   eina_list_shutdown();
}
END_TEST

void
eina_test_list(TCase *tc)
{
   tcase_add_test(tc, eina_test_simple);
}

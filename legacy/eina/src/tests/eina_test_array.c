
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

#include "eina_suite.h"
#include "eina_array.h"
#include "eina_convert.h"

START_TEST(eina_array_simple)
{
   Eina_Array *ea;
   char *tmp;
   Eina_Array_Iterator it;
   unsigned int i;

   eina_array_init();

   ea = eina_array_new(11);
   fail_if(!ea);

   for (i = 0; i < 201; ++i)
     {
	tmp = malloc(sizeof(char) * 10);
	fail_if(!tmp);
	eina_convert_itoa(i, tmp);

	eina_array_push(ea, tmp);
     }

   fail_if(eina_array_get(ea, 10) == NULL);
   fail_if(atoi(eina_array_get(ea, 10)) != 10);
   tmp = eina_array_pop(ea);
   fail_if(tmp == NULL);
   fail_if(atoi(tmp) != 200);
   free(tmp);

   EINA_ARRAY_ITER_NEXT(ea, i, tmp, it)
     {
	fail_if((unsigned int) atoi(tmp) != i);
	free(tmp);
     }

   fail_if(i != 200);

   eina_array_clean(ea);
   eina_array_flush(ea);
   eina_array_free(ea);

   eina_array_shutdown();
}
END_TEST

START_TEST(eina_array_static)
{
   Eina_Array sea = { NULL, 0, 0, 0 };
   char *tmp;
   Eina_Array_Iterator it;
   unsigned int i;

   eina_array_init();

   eina_array_step_set(&sea, 10);

   for (i = 0; i < 200; ++i)
     {
	tmp = malloc(sizeof(char) * 10);
	fail_if(!tmp);
	eina_convert_itoa(i, tmp);

	eina_array_push(&sea, tmp);
     }

   fail_if(eina_array_get(&sea, 10) == NULL);
   fail_if(atoi(eina_array_get(&sea, 10)) != 10);

   EINA_ARRAY_ITER_NEXT(&sea, i, tmp, it)
     {
	fail_if((unsigned int) atoi(tmp) != i);
	free(tmp);
     }

   fail_if(i != 200);

   eina_array_clean(&sea);
   eina_array_flush(&sea);

   eina_array_shutdown();
}
END_TEST

Eina_Bool
keep_int(void *data, void *gdata)
{
   int *tmp = data;

   fail_if(gdata);
   fail_if(!tmp);

   if (*tmp == 0) return EINA_FALSE;
   return EINA_TRUE;
}

START_TEST(eina_array_remove_stuff)
{
   Eina_Array *ea;
   int *tmp;
   Eina_Array_Iterator it;
   unsigned int i;

   eina_array_init();

   ea = eina_array_new(64);
   fail_if(!ea);

   for (i = 0; i < 1000; ++i)
     {
	tmp = malloc(sizeof(int));
	fail_if(!tmp);
	*tmp = i;

	eina_array_push(ea, tmp);
     }

   // Remove the first 10 items
   for (i = 0; i < 10; ++i)
     {
	tmp = eina_array_get(ea, i);
	fail_if(!tmp);
	*tmp = 0;
     }
   fail_if(eina_array_remove(ea, keep_int, NULL) != EINA_TRUE);

   fail_if(eina_array_count(ea) != 990);
   EINA_ARRAY_ITER_NEXT(ea, i, tmp, it)
     fail_if(*tmp == 0);

   // Remove the last items
   for (i = 980; i < 990; ++i)
     {
	tmp = eina_array_get(ea, i);
	fail_if(!tmp);
	*tmp = 0;
     }
   eina_array_remove(ea, keep_int, NULL);

   // Remove all items
   fail_if(eina_array_count(ea) != 980);
   EINA_ARRAY_ITER_NEXT(ea, i, tmp, it)
     {
	fail_if(*tmp == 0);
	*tmp = 0;
     }

   eina_array_remove(ea, keep_int, NULL);

   fail_if(eina_array_count(ea) != 0);

   eina_array_free(ea);

   eina_array_shutdown();
}
END_TEST

void
eina_test_array(TCase *tc)
{
   tcase_add_test(tc, eina_array_simple);
   tcase_add_test(tc, eina_array_static);
   tcase_add_test(tc, eina_array_remove_stuff);
}

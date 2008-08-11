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

   tmp = eina_accessor_data_get(it, 100);
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

void
eina_test_accessor(TCase *tc)
{
   tcase_add_test(tc, eina_accessor_array_simple);
}

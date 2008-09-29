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
#include <string.h>

#include "eina_suite.h"
#include "eina_file.h"

START_TEST(eina_file_split_simple)
{
   Eina_Array *ea;

   eina_array_init();

   ea = eina_file_split(NULL);
   fail_if(ea);

   ea = eina_file_split(strdup("/this/is/a/small/test"));

   fail_if(!ea);
   fail_if(eina_array_count_get(ea) != 5);
   fail_if(strcmp(eina_array_data_get(ea, 0), "this"));
   fail_if(strcmp(eina_array_data_get(ea, 1), "is"));
   fail_if(strcmp(eina_array_data_get(ea, 2), "a"));
   fail_if(strcmp(eina_array_data_get(ea, 3), "small"));
   fail_if(strcmp(eina_array_data_get(ea, 4), "test"));

   eina_array_free(ea);

   ea = eina_file_split(strdup("this//is///a /more/complex///case///"));

   fail_if(!ea);
   fail_if(eina_array_count_get(ea) != 6);
   fail_if(strcmp(eina_array_data_get(ea, 0), "this"));
   fail_if(strcmp(eina_array_data_get(ea, 1), "is"));
   fail_if(strcmp(eina_array_data_get(ea, 2), "a "));
   fail_if(strcmp(eina_array_data_get(ea, 3), "more"));
   fail_if(strcmp(eina_array_data_get(ea, 4), "complex"));
   fail_if(strcmp(eina_array_data_get(ea, 5), "case"));

   eina_array_free(ea);

   eina_array_shutdown();
}
END_TEST

void
eina_test_file(TCase *tc)
{
   tcase_add_test(tc, eina_file_split_simple);
}


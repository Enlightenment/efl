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
#include <string.h>

#include "eina_convert.h"
#include "eina_suite.h"

START_TEST(eina_convert_simple)
{
   char tmp[128];

   fail_if(eina_convert_itoa(0, tmp) != 1);
   fail_if(strcmp(tmp, "0") != 0);

   fail_if(eina_convert_itoa(-1, tmp) != 2);
   fail_if(strcmp(tmp, "-1") != 0);

   fail_if(eina_convert_itoa(100, tmp) != 3);
   fail_if(strcmp(tmp, "100") != 0);

   fail_if(eina_convert_itoa(-100, tmp) != 4);
   fail_if(strcmp(tmp, "-100") != 0);

   fail_if(eina_convert_itoa(10000000, tmp) != 8);
   fail_if(strcmp(tmp, "10000000") != 0);

   fail_if(eina_convert_xtoa(0, tmp) != 1);
   fail_if(strcmp(tmp, "0") != 0);

   fail_if(eina_convert_xtoa(0xA1, tmp) != 2);
   fail_if(strcmp(tmp, "a1") != 0);

   fail_if(eina_convert_xtoa(0xFF00EF0E, tmp) != 8);
   fail_if(strcmp(tmp, "ff00ef0e") != 0);
}
END_TEST

void
eina_test_convert(TCase *tc)
{
   tcase_add_test(tc, eina_convert_simple);
}

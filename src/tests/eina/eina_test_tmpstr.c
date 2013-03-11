/* EINA - EFL data type library
 * Copyright (C) 2013 Vlad Brovko
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

#include "eina_suite.h"
#include "Eina.h"
#include "eina_tmpstr.h"

START_TEST(tmpstr_simple)
{
   eina_init();

   const int cnt_tmp_strings = 10;
   const int max_str_len = 255;
   char buf[max_str_len + 1];
   Eina_Tmpstr *tmp_strings[cnt_tmp_strings];

   // Add several tmp strings
   for (int i = 0; i != cnt_tmp_strings; ++i)
     {
        snprintf(buf, max_str_len, "Tmp string %d", (i + 1));
        tmp_strings[i] = eina_tmpstr_add(buf);

        fail_if(strcmp(buf, tmp_strings[i]));
     }

   // Delete these tmp strings
   for (int i = 0; i != cnt_tmp_strings; ++i)
     {
        snprintf(buf, max_str_len, "Tmp string %d", (cnt_tmp_strings - i - 1 + 1));

        fail_if(strcmp(buf, tmp_strings[cnt_tmp_strings - i - 1]));

        eina_tmpstr_del(tmp_strings[cnt_tmp_strings - i - 1]);
        tmp_strings[cnt_tmp_strings - i - 1] = 0;
     }

   // Delete non tmp string (should do nothing)
   eina_tmpstr_del("Some non tmp string");

   eina_shutdown();
}
END_TEST

void
eina_test_tmpstr(TCase *tc)
{
   tcase_add_test(tc, tmpstr_simple);
}

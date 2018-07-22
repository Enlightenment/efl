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

#include <Eina.h>
#include "eina_tmpstr.h"

#include "eina_suite.h"

EFL_START_TEST(tmpstr_simple)
{

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

}
EFL_END_TEST

EFL_START_TEST(tmpstr_simple_len)
{

   const int cnt_tmp_strings = 10;
   const int max_str_len = 255;
   char buf[max_str_len + 1];
   Eina_Tmpstr *tmp_strings[cnt_tmp_strings];

   // Add several tmp strings
   for (int i = 0; i != cnt_tmp_strings; ++i)
     {
        snprintf(buf, max_str_len, "Tmp string %d", (i + 1));
        tmp_strings[i] = eina_tmpstr_add_length(buf, (max_str_len + 1));

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

}
EFL_END_TEST

EFL_START_TEST(tmpstr_manage)
{

   char *buf = malloc(7);
   strcpy(buf, "tmpstr");
   Eina_Tmpstr *tstr1 = eina_tmpstr_manage_new(buf);
   fail_if(strcmp(buf, tstr1));
   eina_tmpstr_del(tstr1);

}
EFL_END_TEST

EFL_START_TEST(tmpstr_manage_len)
{

   char *buf = malloc(10);
   strcpy(buf, "tmpstr");
   Eina_Tmpstr *tstr1 = eina_tmpstr_manage_new_length(buf, 7);
   fail_if(strcmp(buf, tstr1));
   eina_tmpstr_del(tstr1);

}
EFL_END_TEST

EFL_START_TEST(tmpstr_len)
{

   const char *str1 = "12345";
   const char *str2 = "123456789";
   Eina_Tmpstr *tstr1 = eina_tmpstr_add(str1);
   Eina_Tmpstr *tstr2 = eina_tmpstr_add(str2);
   Eina_Tmpstr *tstr_empty = eina_tmpstr_add("");

   ck_assert_int_eq(eina_tmpstr_len(tstr1), strlen(str1));
   ck_assert_int_eq(eina_tmpstr_len(tstr2), strlen(str2));

   ck_assert_int_eq(eina_tmpstr_len(tstr_empty), 0);

   eina_tmpstr_del(tstr1);
   eina_tmpstr_del(tstr2);

}
EFL_END_TEST

void
eina_test_tmpstr(TCase *tc)
{
   tcase_add_test(tc, tmpstr_simple);
   tcase_add_test(tc, tmpstr_simple_len);
   tcase_add_test(tc, tmpstr_manage);
   tcase_add_test(tc, tmpstr_manage_len);
   tcase_add_test(tc, tmpstr_len);
}

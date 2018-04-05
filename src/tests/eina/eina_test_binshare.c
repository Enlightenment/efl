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
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <Eina.h>

#include "eina_suite.h"

#define TEST0 "te\0st/0"
#define TEST0_SIZE 7
#define TEST1 "te\0st/11"
#define TEST1_SIZE 8

EFL_START_TEST(eina_binshare_simple)
{
   const char *t0;
   const char *t1;

   t0 = eina_binshare_add_length(TEST0, TEST0_SIZE);
   t1 = eina_binshare_add_length(TEST1, TEST1_SIZE);

   fail_if(t0 == NULL);
   fail_if(t1 == NULL);
   fail_if(memcmp(t0, TEST0, TEST0_SIZE) != 0);
   fail_if(memcmp(t1, TEST1, TEST1_SIZE) != 0);

   t0 = eina_binshare_ref(t0);
   fail_if(t0 == NULL);
   fail_if(memcmp(t0, TEST0, TEST0_SIZE) != 0);

   eina_binshare_del(t0);
   eina_binshare_del(t0);
   eina_binshare_del(t1);

}
EFL_END_TEST

EFL_START_TEST(eina_binshare_small)
{
   char buf[4];
   int i;

   for (i = 1; i < 3; i++)
     {
        const char *t0, *t1;
        int j;

        for (j = 0; j < i; j++)
          {
             char c;
             for (c = 'a'; c <= 'z'; c++)
                buf[j] = c;
          }
        buf[i] = '\0';
        t0 = eina_binshare_add_length(buf, i);
        t1 = eina_binshare_add_length(buf, i);

        fail_if(t0 == NULL);
        fail_if(t1 == NULL);
        fail_if(t0 != t1);
        fail_if(memcmp(t0, buf, i) != 0);

        eina_binshare_del(t0);
        eina_binshare_del(t1);
     }
}
EFL_END_TEST


EFL_START_TEST(eina_binshare_test_share)
{
   const char *t0;
   const char *t1;

   t0 = eina_binshare_add_length(TEST0, TEST0_SIZE);
   t1 = eina_binshare_add_length(TEST0, TEST0_SIZE);

   fail_if(t0 == NULL);
   fail_if(t1 == NULL);
   fail_if(memcmp(t0, TEST0, TEST0_SIZE) != 0);
   fail_if(memcmp(t1, TEST0, TEST0_SIZE) != 0);
   fail_if(t0 != t1);
   fail_if(TEST0_SIZE != eina_binshare_length(t0));

   eina_binshare_del(t0);
   eina_binshare_del(t1);
}
EFL_END_TEST

EFL_START_TEST(eina_binshare_putstuff)
{
   const char *tmp;
   int i;

   for (i = 100; i > 0; --i)
     {
        char build[64] = "string_";
        unsigned int len;

        eina_convert_xtoa(i, build + 7);
        len = strlen(build);
        tmp = eina_binshare_add_length(build, len);
        ck_assert_ptr_ne(tmp, NULL);
        ck_assert_ptr_eq(tmp, eina_binshare_add_length(build, len));
        ck_assert_int_eq(len, eina_binshare_length(tmp));
        eina_binshare_del(tmp);
        eina_binshare_del(tmp);
     }
}
EFL_END_TEST

EFL_START_TEST(eina_binshare_collision)
{
   Eina_Array *ea;
   char buffer[50];
   int i;
   const void *r;

   ea = eina_array_new(256);

   for (i = 0; i < 256; ++i)
     {
        unsigned int len;
        eina_convert_itoa(60000 - i, buffer);
        len = strlen(buffer);
        r = eina_binshare_add_length(buffer, len);
        ck_assert_ptr_ne(r, NULL);
        eina_array_push(ea, r);
        r = eina_binshare_add_length(buffer, len);
        ck_assert_ptr_ne(r, NULL);
        r = eina_binshare_add_length(buffer, len);
        ck_assert_ptr_ne(r, NULL);
     }

   while (eina_array_count(ea))
      {
         r = eina_array_pop(ea);
         eina_binshare_del(r);
         eina_binshare_del(r);
         eina_binshare_del(r);
      }

   eina_array_free(ea);
}
EFL_END_TEST

void
eina_test_binshare(TCase *tc)
{
   tcase_add_test(tc, eina_binshare_simple);
   tcase_add_test(tc, eina_binshare_small);
   tcase_add_test(tc, eina_binshare_test_share);
   tcase_add_test(tc, eina_binshare_collision);
   tcase_add_test(tc, eina_binshare_putstuff);
}

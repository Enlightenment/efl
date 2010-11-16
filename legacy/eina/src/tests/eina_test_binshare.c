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

#include "eina_suite.h"
#include "Eina.h"

#define TEST0 "te\0st/0"
#define TEST0_SIZE 7
#define TEST1 "te\0st/11"
#define TEST1_SIZE 8

START_TEST(eina_binshare_simple)
{
   const char *t0;
   const char *t1;

   eina_init();

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

   eina_shutdown();
}
END_TEST

START_TEST(eina_binshare_small)
{
   char buf[4];
   int i;

   eina_init();

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
   eina_shutdown();
}
END_TEST


START_TEST(eina_binshare_test_share)
{
   const char *t0;
   const char *t1;

   eina_init();

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
   eina_shutdown();
}
END_TEST

START_TEST(eina_binshare_putstuff)
{
   const char *tmp;
   int i;

   eina_init();

   for (i = 10000; i > 0; --i)
     {
        char build[64] = "string_";

        eina_convert_xtoa(i, build + 7);
        tmp = eina_binshare_add_length(build, strlen(build));
        fail_if(tmp != eina_binshare_add_length(build, strlen(build)));
        fail_if((int)strlen(build) != eina_binshare_length(tmp));
     }
   eina_shutdown();
}
END_TEST

START_TEST(eina_binshare_collision)
{
   Eina_Array *ea;
   char buffer[50];
   int i;

   srand(time(NULL));

   eina_init();

   ea = eina_array_new(256);
   fail_if(!ea);

   for (i = 0; i < 10000; ++i)
     {
        eina_convert_itoa(rand(), buffer);
        eina_array_push(ea,
                        (void *)eina_binshare_add_length(buffer, strlen(buffer)));
        if (rand() > RAND_MAX / 2)
          {
             const char *r = eina_binshare_add_length(buffer, strlen(buffer));
             fail_if(r == NULL);
          }
     }

   for (i = 0; i < 10000; ++i)
     {
        const char *r;

        eina_convert_itoa(60000 - i, buffer);
        eina_array_push(ea,
                        (void *)eina_binshare_add_length(buffer, strlen(buffer)));
        r = eina_binshare_add_length(buffer, strlen(buffer));
        fail_if(r == NULL);
        r = eina_binshare_add_length(buffer, strlen(buffer));
        fail_if(r == NULL);
     }

   for (i = 0; i < 200; ++i)
      eina_binshare_del(eina_array_data_get(ea, i));

   for (i = 0; i < 1000; ++i)
      eina_binshare_del(eina_array_pop(ea));

   eina_shutdown();

   eina_array_free(ea);
}
END_TEST

void
eina_test_binshare(TCase *tc)
{
   tcase_add_test(tc, eina_binshare_simple);
   tcase_add_test(tc, eina_binshare_small);
   tcase_add_test(tc, eina_binshare_test_share);
   tcase_add_test(tc, eina_binshare_collision);
   tcase_add_test(tc, eina_binshare_putstuff);
}

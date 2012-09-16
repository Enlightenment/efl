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

#define TEST0 "test/0"
#define TEST1 "test/1"

START_TEST(eina_stringshare_simple)
{
   const char *t0;
   const char *t1;

   eina_init();

   t0 = eina_stringshare_add(TEST0);
   t1 = eina_stringshare_add(TEST1);

   fail_if(t0 == NULL);
   fail_if(t1 == NULL);
   fail_if(strcmp(t0, TEST0) != 0);
   fail_if(strcmp(t1, TEST1) != 0);
   fail_if((int)strlen(TEST0) != eina_stringshare_strlen(t0));
   fail_if((int)strlen(TEST1) != eina_stringshare_strlen(t1));

   t0 = eina_stringshare_ref(t0);
   fail_if(t0 == NULL);
   fail_if((int)strlen(TEST0) != eina_stringshare_strlen(t0));

   eina_stringshare_del(t0);
   eina_stringshare_del(t0);
   eina_stringshare_del(t1);

   eina_shutdown();
}
END_TEST

START_TEST(eina_stringshare_small)
{
   char buf[4];
   int i;

   eina_init();

   for (i = 0; i < 3; i++)
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
        t0 = eina_stringshare_add(buf);
        t1 = eina_stringshare_add(buf);

        fail_if(t0 == NULL);
        fail_if(t1 == NULL);
        fail_if(t0 != t1);
        fail_if(strcmp(t0, buf) != 0);
        fail_if((int)strlen(buf) != eina_stringshare_strlen(t0));
        fail_if((int)strlen(buf) != eina_stringshare_strlen(t1));

        eina_stringshare_del(t0);
        eina_stringshare_del(t1);
     }

        eina_shutdown();
}
END_TEST


START_TEST(eina_stringshare_test_share)
{
   const char *t0;
   const char *t1;

   eina_init();

   t0 = eina_stringshare_add(TEST0);
   t1 = eina_stringshare_add(TEST0);

   fail_if(t0 == NULL);
   fail_if(t1 == NULL);
   fail_if(strcmp(t0, TEST0) != 0);
   fail_if(strcmp(t1, TEST0) != 0);
   fail_if(t0 != t1);
   fail_if((int)strlen(t0) != eina_stringshare_strlen(t0));

   eina_stringshare_del(t0);
   eina_stringshare_del(t1);

   eina_shutdown();
}
END_TEST

START_TEST(eina_stringshare_putstuff)
{
   const char *tmp;
   int i;

   eina_init();

   for (i = 10000; i > 0; --i)
     {
        char build[64] = "string_";

        eina_convert_xtoa(i, build + 7);
        tmp = eina_stringshare_add(build);
        fail_if(tmp != eina_stringshare_add(build));
     }

   eina_shutdown();
}
END_TEST

START_TEST(eina_stringshare_collision)
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
        eina_array_push(ea, (void *)eina_stringshare_add(buffer));
        if (rand() > RAND_MAX / 2)
          {
             const char *r = eina_stringshare_add(buffer);
             fail_if(r == NULL);
          }
     }

   for (i = 0; i < 10000; ++i)
     {
        const char *r;

        eina_convert_itoa(60000 - i, buffer);
        eina_array_push(ea, (void *)eina_stringshare_add(buffer));
        r = eina_stringshare_add(buffer);
        fail_if(r == NULL);
        r = eina_stringshare_add(buffer);
        fail_if(r == NULL);
     }

   for (i = 0; i < 200; ++i)
      eina_stringshare_del(eina_array_data_get(ea, i));

   for (i = 0; i < 1000; ++i)
      eina_stringshare_del(eina_array_pop(ea));

      eina_shutdown();

   eina_array_free(ea);
}
END_TEST

void
eina_test_stringshare(TCase *tc)
{
   tcase_add_test(tc, eina_stringshare_simple);
   tcase_add_test(tc, eina_stringshare_small);
   tcase_add_test(tc, eina_stringshare_test_share);
   tcase_add_test(tc, eina_stringshare_collision);
   tcase_add_test(tc, eina_stringshare_putstuff);
}

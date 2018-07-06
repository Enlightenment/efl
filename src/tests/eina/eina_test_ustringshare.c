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

static const Eina_Unicode TEST0[] = {'t', 'e', 's', 't', '/', '0', 0};
static const Eina_Unicode TEST1[] = {'t', 'e', 's', 't', '/', '1', 0};

EFL_START_TEST(eina_ustringshare_simple)
{
   const Eina_Unicode *t0;
   const Eina_Unicode *t1;


   t0 = eina_ustringshare_add(TEST0);
   t1 = eina_ustringshare_add(TEST1);

   fail_if(t0 == NULL);
   fail_if(t1 == NULL);
   fail_if(eina_unicode_strcmp(t0, TEST0) != 0);
   fail_if(eina_unicode_strcmp(t1, TEST1) != 0);
   fail_if((int)eina_unicode_strlen(TEST0) != eina_ustringshare_strlen(t0));
   fail_if((int)eina_unicode_strlen(TEST1) != eina_ustringshare_strlen(t1));

   t0 = eina_ustringshare_ref(t0);
   fail_if(t0 == NULL);
   fail_if((int)strlen((char*)TEST0) != eina_stringshare_strlen((const char*)t0));

   eina_ustringshare_del(t0);
   eina_ustringshare_del(t0);
   eina_ustringshare_del(t1);

}
EFL_END_TEST


EFL_START_TEST(eina_ustringshare_test_share)
{
   const Eina_Unicode *t0;
   const Eina_Unicode *t1;


   t0 = eina_ustringshare_add(TEST0);
   t1 = eina_ustringshare_add(TEST0);

   fail_if(t0 == NULL);
   fail_if(t1 == NULL);
   fail_if(eina_unicode_strcmp(t0, TEST0) != 0);
   fail_if(eina_unicode_strcmp(t1, TEST0) != 0);
   fail_if(t0 != t1);
   fail_if((int)eina_unicode_strlen(TEST0) != eina_ustringshare_strlen(t0));
   fail_if((int)eina_unicode_strlen(TEST0) != eina_ustringshare_strlen(t1));

   eina_ustringshare_del(t0);
   eina_ustringshare_del(t1);

}
EFL_END_TEST

EFL_START_TEST(eina_ustringshare_putstuff)
{
   const Eina_Unicode *tmp;
   int i;


   for (i = 100; i > 0; --i)
     {
        Eina_Unicode string_[] = {'s', 't', 'r', 'i', 'n', 'g', '_', 0};
        Eina_Unicode build[64];
        eina_unicode_strcpy(build, string_);

        build[7] = i;
        build[8] = 0;
        tmp = eina_ustringshare_add(build);
        fail_if(tmp != eina_ustringshare_add(build));
        fail_if((int)eina_unicode_strlen(build) != eina_ustringshare_strlen(tmp));
     }

}
EFL_END_TEST

void
eina_test_ustringshare(TCase *tc)
{
   tcase_add_test(tc, eina_ustringshare_simple);
   tcase_add_test(tc, eina_ustringshare_test_share);
   tcase_add_test(tc, eina_ustringshare_putstuff);
}

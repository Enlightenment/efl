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
#include <stdlib.h>

#include "eina_suite.h"
#include "Eina.h"
#include "eina_safety_checks.h"

START_TEST(eina_counter_simple)
{
   Eina_Counter *cnt;
   char *dump;
   int i;

   eina_init();

   cnt = eina_counter_new("eina_test");
   fail_if(!cnt);

   eina_counter_start(cnt);

   for (i = 0; i < 100000; ++i)
     {
        void *tmp = malloc(sizeof(long int));
        free(tmp);
     }

   eina_counter_stop(cnt, i);

   eina_counter_start(cnt);

   for (i = 0; i < 200000; ++i)
     {
        void *tmp = malloc(sizeof(long int));
        free(tmp);
     }

   eina_counter_stop(cnt, i);

   dump = eina_counter_dump(cnt);
   fail_if(!dump);

   fprintf(stderr, "%s", dump);

   free(dump);

   eina_counter_free(cnt);

   eina_shutdown();
}
END_TEST

START_TEST(eina_counter_break)
{
   Eina_Counter *cnt;

   eina_init();

   cnt = eina_counter_new("eina_test");
   fail_if(!cnt);

   eina_counter_stop(cnt, 10);

   eina_counter_free(cnt);

#ifdef EINA_SAFETY_CHECKS
   {
      char *dump;

      fprintf(stderr, "you should have a safety check failure below:\n");
      dump = eina_counter_dump(NULL);
      fail_if(dump);
      fail_if(eina_error_get() != EINA_ERROR_SAFETY_FAILED);
      free(dump);
   }
#endif

   eina_shutdown();
}
END_TEST

void eina_test_counter(TCase *tc)
{
   tcase_add_test(tc, eina_counter_simple);
   tcase_add_test(tc, eina_counter_break);
}


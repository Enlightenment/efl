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

#include <stdlib.h>

#include "eina_counter.h"
#include "eina_suite.h"

START_TEST(eina_counter_init_shutdown)
{
   eina_counter_init();
    eina_counter_init();
    eina_counter_shutdown();
    eina_counter_init();
     eina_counter_init();
     eina_counter_shutdown();
    eina_counter_shutdown();
   eina_counter_shutdown();
}
END_TEST

START_TEST(eina_counter_simple)
{
   Eina_Counter *cnt;
   int i;

   eina_counter_init();

   cnt = eina_counter_add("eina_test");
   fail_if(!cnt);

   eina_counter_start(cnt);

   for (i = 0; i < 100000; ++i)
     malloc(sizeof(long int));

   eina_counter_stop(cnt, i);

   eina_counter_start(cnt);

   for (i = 0; i < 200000; ++i)
     malloc(sizeof(long int));

   eina_counter_stop(cnt, i);

   eina_counter_dump(cnt, stderr);

   eina_counter_delete(cnt);

   eina_counter_shutdown();
}
END_TEST

void eina_test_counter(TCase *tc)
{
   tcase_add_test(tc, eina_counter_init_shutdown);
   tcase_add_test(tc, eina_counter_simple);
}


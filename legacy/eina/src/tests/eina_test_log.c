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

#include "eina_suite.h"
#include "Eina.h"

   START_TEST(eina_log_macro)
{
   fail_if(!eina_init());

   eina_log_level_set(EINA_LOG_LEVEL_DBG);
   eina_log_print_cb_set(eina_log_print_cb_file, stderr);

   EINA_LOG_CRIT("Critical message\n");
   EINA_LOG_ERR("An error\n");
   EINA_LOG_INFO("An info\n");
   EINA_LOG_WARN("A warning\n");
   EINA_LOG_DBG("A debug\n");

   eina_shutdown();
}
END_TEST

START_TEST(eina_log_domains_macros)
{
   fail_if(!eina_init());

   int d = eina_log_domain_register("MyDomain", EINA_COLOR_GREEN);
   fail_if(d < 0);

   EINA_LOG_DOM_CRIT(d, "A critical message\n");
   EINA_LOG_DOM_ERR(d, "An error\n");
   EINA_LOG_DOM_WARN(d, "A warning\n");
   EINA_LOG_DOM_DBG(d, "A debug\n");
   EINA_LOG_DOM_INFO(d, "An info\n");

   eina_shutdown();
}
END_TEST

START_TEST(eina_log_domains_registry)
{
        fail_if(!eina_init());

   int i;
   int d[50];

   for (i = 0; i < 50; i++)
     {
        d[i] = eina_log_domain_register("Test", EINA_COLOR_GREEN);
        fail_if(d[i] < 0);
     }

   for (i = 0; i < 50; i++)
      eina_log_domain_unregister(d[i]);

   eina_shutdown();
}
END_TEST

START_TEST(eina_log_domains_slot_reuse)
{
        fail_if(!eina_init());
        fail_if(!eina_threads_init());

   // Create 9 domains
   int idx[9];
   int i;

   for (i = 0; i < 9; i++)
     {
        idx[i] = eina_log_domain_register("Test1", EINA_COLOR_GREEN);
        fail_if(idx[i] < 0);
     }

   // Slot 0 by default contains the global logger. The above code created
   // domains for slots indexes from 1 to 9.
   //
   // The global logger allocated the first 8 initial slots. The 8th domain
   // registered on the for loop will create 8 more slots.
   //
   // Test will just unregister a domain between 1 and 9 and assure that a new
   // domain register will be placed on the available slot and not at the end.

   int removed = idx[5];
   eina_log_domain_unregister(removed);

   int new = eina_log_domain_register("Test Slot", EINA_COLOR_GREEN);

   // Check for slot reuse
   fail_if(new != removed);

   eina_threads_shutdown();
   eina_shutdown();
}
END_TEST

START_TEST(eina_log_level_indexes)
{
   fail_if(!eina_init());
   fail_if(!eina_threads_init());
   fail_if(!eina_threads_init());

   int d = eina_log_domain_register("Levels", EINA_COLOR_GREEN);
   fail_if(d < 0);

   // Displayed unless user sets level lower than -1
   EINA_LOG(d, -1, "Negative index message\n");

   // Displayed only if user sets level 6 or higher
   EINA_LOG(d, 6,  "Higher level debug\n");

   eina_threads_shutdown();
   eina_threads_shutdown();
   eina_shutdown();
}
END_TEST

START_TEST(eina_log_customize)
{
   int d;

   /* please don't define EINA_LOG_LEVELS for it */
#define TEST_DOM "_Test_Log_Dom"

   fail_if(!eina_init());

#define test_set_get(func, val)                 \
   eina_log_ ## func ## _set(val);                  \
   fail_if(eina_log_ ## func ## _get() != val)

   test_set_get(level, -1234);
   test_set_get(level, 4567);

#define test_set_get_bool(func)                 \
   test_set_get(func,  EINA_FALSE);              \
   test_set_get(func,  EINA_TRUE)

   test_set_get_bool(color_disable);
   test_set_get_bool(file_disable);
   test_set_get_bool(function_disable);
   test_set_get_bool(abort_on_critical);

   test_set_get(abort_on_critical_level, -1234);
   test_set_get(abort_on_critical_level, 4567);

   fail_if(eina_log_domain_level_get(TEST_DOM) != eina_log_level_get());

   eina_log_domain_level_set(TEST_DOM, -123);
   fail_if(eina_log_domain_level_get(TEST_DOM) != -123);

   eina_log_domain_level_set(TEST_DOM, 890);
   fail_if(eina_log_domain_level_get(TEST_DOM) != 890);

   d = eina_log_domain_register(TEST_DOM, EINA_COLOR_GREEN);
   fail_if(d < 0);

   fail_if(eina_log_domain_level_get(TEST_DOM) != 890);
   fail_if(eina_log_domain_registered_level_get(d) != 890);

   eina_log_domain_unregister(d);
   fputs("NOTE: You should see a failed safety check or "
         "a crash if compiled without safety checks support.\n",
         stderr);
   eina_log_abort_on_critical_set(EINA_FALSE);
   fail_if(eina_log_domain_registered_level_get(d) != EINA_LOG_LEVEL_UNKNOWN);

#undef test_set_get_bool
#undef test_set_get

   eina_shutdown();
}
END_TEST

START_TEST(eina_log_level_name)
{
   char name[4];

   fail_if(!eina_init());

#define tst(level, str)                         \
   eina_log_level_name_get(level, name);        \
   fail_if(strcmp(name, str) != 0)

   tst(0,   "CRI");
   tst(1,   "ERR");
   tst(2,   "WRN");
   tst(3,   "INF");
   tst(4,   "DBG");
   tst(5,   "005");
   tst(12,  "012");
   tst(369, "369");
   tst(-1,  "-01");
   tst(-48, "-48");

#undef tst

   eina_shutdown();
}
END_TEST

void
eina_test_log(TCase *tc)
{
   tcase_add_test(tc, eina_log_macro);
   tcase_add_test(tc, eina_log_domains_macros);
   tcase_add_test(tc, eina_log_domains_registry);
   tcase_add_test(tc, eina_log_domains_slot_reuse);
   tcase_add_test(tc, eina_log_level_indexes);
   tcase_add_test(tc, eina_log_customize);
   tcase_add_test(tc, eina_log_level_name);
}

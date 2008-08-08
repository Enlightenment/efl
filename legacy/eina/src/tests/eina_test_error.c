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
#include <stdio.h>

#include "eina_suite.h"
#include "eina_error.h"

START_TEST(eina_error_init_shutdown)
{
   eina_error_init();
    eina_error_init();
    eina_error_shutdown();
    eina_error_init();
     eina_error_init();
     eina_error_shutdown();
    eina_error_shutdown();
   eina_error_shutdown();
}
END_TEST

#define TEST_TEXT "The big test\n"

START_TEST(eina_error_errno)
{
   int test;

   setenv("EINA_ERROR_LEVEL", "1", 0);

   eina_error_init();

   test = eina_error_register(TEST_TEXT);
   fail_if(!eina_error_msg_get(test));
   fail_if(strcmp(eina_error_msg_get(test), TEST_TEXT) != 0);

   eina_error_set(test);
   fail_if(eina_error_get() != test);

   eina_error_shutdown();
}
END_TEST

START_TEST(eina_error_macro)
{
   eina_error_init();

   eina_error_log_level_set(EINA_ERROR_LEVEL_DBG);
   eina_error_print_cb_set(eina_error_print_cb_file, stderr);

   EINA_ERROR_PERR("An error\n");
   EINA_ERROR_PINFO("An info\n");
   EINA_ERROR_PWARN("A warning\n");
   EINA_ERROR_PDBG("A debug\n");

   eina_error_shutdown();
}
END_TEST

void
eina_test_error(TCase *tc)
{
   tcase_add_test(tc, eina_error_init_shutdown);
   tcase_add_test(tc, eina_error_errno);
   tcase_add_test(tc, eina_error_macro);
}

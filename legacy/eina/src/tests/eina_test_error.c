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

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "eina_suite.h"
#include "Eina.h"

#define TEST_TEXT "The big test\n"

START_TEST(eina_error_errno)
{
   int test;

   setenv("EINA_ERROR_LEVEL", "1", 0);

   eina_init();

   test = eina_error_msg_register(TEST_TEXT);
   fail_if(!eina_error_msg_get(test));
   fail_if(strcmp(eina_error_msg_get(test), TEST_TEXT) != 0);

   eina_error_set(test);
   fail_if(eina_error_get() != test);

   eina_shutdown();
}
END_TEST

void
eina_test_error(TCase *tc)
{
   tcase_add_test(tc, eina_error_errno);
}

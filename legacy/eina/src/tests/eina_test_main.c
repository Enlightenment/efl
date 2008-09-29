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

#include "Eina.h"
#include "eina_suite.h"

START_TEST(eina_simple)
{
   /* Eina_error as already been initialized by eina_hash
      that was called by eina_mempool_init that's why we don't have 0 here */
   fail_if(eina_init() != 2);
   fail_if(eina_shutdown() != 1);
}
END_TEST

void eina_test_main(TCase *tc)
{
   tcase_add_test(tc, eina_simple);
}

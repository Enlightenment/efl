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

#include <Ecore.h>

#include "efl_suite.h"
#include "../efl_check.h"

static const Efl_Test_Case etc[] = {
   { "Efl_Model_Container", efl_test_case_model_container },
   { "Efl_Model_Composite_Boolean", efl_test_case_model_composite_boolean },
   { NULL, NULL }
};

SUITE_INIT(efl)
{
   ck_assert_int_eq(ecore_init(), 1);
}

SUITE_SHUTDOWN(efl)
{
   ck_assert_int_eq(ecore_shutdown(), 0);
}

int
main(int argc, char **argv)
{
   int failed_count;

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

   putenv("EFL_RUN_IN_TREE=1");

   eina_init();

   failed_count = _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Efl", etc, SUITE_INIT_FN(efl), SUITE_SHUTDOWN_FN(efl));

   eina_shutdown();

   return (failed_count == 0) ? 0 : 255;
}

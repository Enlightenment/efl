/* ECTOR - EFL retained mode drawing library
 * Copyright (C) 2014 Cedric Bail
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
# include <config.h>
#endif

#include "ector_suite.h"
#include "../efl_check.h"
#include <Ector.h>

static const Efl_Test_Case etc[] = {
  { "init", ector_test_init },
  { NULL, NULL }
};

SUITE_INIT(ector)
{
   ck_assert_int_eq(ector_init(), 1);
}

SUITE_SHUTDOWN(ector)
{
   ck_assert_int_eq(ector_shutdown(), 0);
}

int
main(int argc, char *argv[])
{
   int failed_count;

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

#ifdef NEED_RUN_IN_TREE
   putenv("EFL_RUN_IN_TREE=1");
#endif

   failed_count = _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Ector", etc, SUITE_INIT_FN(ector), SUITE_SHUTDOWN_FN(ector));

   return (failed_count == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

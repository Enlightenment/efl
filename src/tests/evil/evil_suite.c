/* EVIL - EFL library for Windows port
 * Copyright (C) 2015 Vincent Torri
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
#include <string.h>

#include "evil_suite.h"
#include "../efl_check.h"
#include <Evil.h>

static const Efl_Test_Case etc[] = {
   { "Dlfcn", evil_test_dlfcn },
   /* { "Fcntl", evil_test_fcntl }, */
   /* { "Fnmatch", evil_test_fnmatch }, */
   /* { "Langinfo", evil_test_langinfo }, */
   { "Libgen", evil_test_libgen },
   { "Main", evil_test_main },
   /* { "Mman", evil_test_mman }, */
   /* { "Pwd", evil_test_pwd }, */
   { "Stdio", evil_test_stdio },
   { "Stdlib", evil_test_stdlib },
   /* { "String", evil_test_string }, */
   /* { "Time", evil_test_time }, */
   { "Unistd", evil_test_unistd },
   /* { "Util", evil_test_util }, */
   { NULL, NULL }
};


SUITE_INIT(evil)
{
   ck_assert_int_eq(evil_init(), 1);
}

SUITE_SHUTDOWN(evil)
{
   ck_assert_int_eq(evil_shutdown(), 0);
}

int
main(int argc, char **argv)
{
   int failed_count;

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

#ifdef NEED_RUN_IN_TREE
   putenv("EFL_RUN_IN_TREE=1");
#endif

   failed_count = _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Evil", etc, SUITE_INIT_FN(evil), SUITE_SHUTDOWN_FN(evil));

   return (failed_count == 0) ? 0 : 255;
}

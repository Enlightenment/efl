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

#include <Evil.h>

#include "evil_suite.h"

typedef struct _Evil_Test_Case Evil_Test_Case;
struct _Evil_Test_Case
{
   const char *test_case;
   void (*build)(TCase *tc);
};

static const Evil_Test_Case etc[] = {
   /* { "Dirent", evil_test_dirent }, */
   { "Dlfcn", evil_test_dlfcn },
   /* { "Fcntl", evil_test_fcntl }, */
   /* { "Fnmatch", evil_test_fnmatch }, */
   /* { "Inet", evil_test_inet }, */
   /* { "Langinfo", evil_test_langinfo }, */
   /* { "Link", evil_test_link }, */
   { "Main", evil_test_main },
   /* { "Mman", evil_test_mman }, */
   /* { "Pwd", evil_test_pwd }, */
   /* { "Stdio", evil_test_stdio }, */
   /* { "Stdlib", evil_test_stdlib }, */
   /* { "String", evil_test_string }, */
   /* { "Time", evil_test_time }, */
   /* { "Unistd", evil_test_unistd }, */
   /* { "Util", evil_test_util }, */
   { NULL, NULL }
};

static void
_list_tests(void)
{
   const Evil_Test_Case *itr = etc;
      fputs("Available Test Cases:\n", stderr);
   for (; itr->test_case; itr++)
      fprintf(stderr, "\t%s\n", itr->test_case);
}

static unsigned char
_use_test(int argc, const char **argv, const char *test_case)
{
   if (argc < 1)
      return 1;

   for (; argc > 0; argc--, argv++)
      if (strcmp(test_case, *argv) == 0)
         return 1;

   return 0;
}

Suite *
evil_build_suite(int argc, const char **argv)
{
   TCase *tc;
   Suite *s;
   int i;

   s = suite_create("Evil");

   for (i = 0; etc[i].test_case; ++i)
     {
        if (!_use_test(argc, argv, etc[i].test_case))
           continue;

        tc = tcase_create(etc[i].test_case);
#ifndef _WIN32
        tcase_set_timeout(tc, 0);
#endif

        etc[i].build(tc);
        suite_add_tcase(s, tc);
     }

   return s;
}

int
main(int argc, char **argv)
{
   Suite *s;
   SRunner *sr;
   int i, failed_count;

   for (i = 1; i < argc; i++)
      if ((strcmp(argv[i], "-h") == 0) ||
          (strcmp(argv[i], "--help") == 0))
        {
           fprintf(stderr, "Usage:\n\t%s [test_case1 .. [test_caseN]]\n",
                   argv[0]);
           _list_tests();
           return 0;
        }
      else if ((strcmp(argv[i], "-l") == 0) ||
               (strcmp(argv[i], "--list") == 0))
        {
           _list_tests();
           return 0;
        }

   putenv("EFL_RUN_IN_TREE=1");

   s = evil_build_suite(argc - 1, (const char **)argv + 1);
   sr = srunner_create(s);
   srunner_set_xml(sr, TESTS_BUILD_DIR "/check-results.xml");
   srunner_run_all(sr, CK_ENV);
   failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);

   return (failed_count == 0) ? 0 : 255;
}

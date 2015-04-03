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
#endif /* ifdef HAVE_CONFIG_H */

#include <stdio.h>
#include <string.h>

#include <check.h>

#include "Eina.h"

#include "ector_suite.h"

typedef struct _Ector_Test_Case Ector_Test_Case;
struct _Ector_Test_Case
{
   const char *test_case;
   void (*build)(TCase *tc);
};

static const Ector_Test_Case etc[] = {
  { "init", ector_test_init },
  { NULL, NULL }
};

static void
_list_tests(void)
{
   const Ector_Test_Case *itr = etc;
      fputs("Available Test Cases:\n", stderr);
   for (; itr->test_case; itr++)
      fprintf(stderr, "\t%s\n", itr->test_case);
}

static Eina_Bool
_use_test(int argc, const char **argv, const char *test_case)
{
   if (argc < 1)
      return 1;

   for (; argc > 0; argc--, argv++)
      if (strcmp(test_case, *argv) == 0)
         return 1;

   return 0;
}

int
main(int argc, char *argv[])
{
   TCase *tc;
   Suite *s;
   SRunner *sr;
   int failed_count, i;

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

   s = suite_create("Ector");

   for (i = 0; etc[i].test_case; ++i)
     {
        if (!_use_test(argc - 1, (const char **) argv + 1, etc[i].test_case))
          continue;

        tc = tcase_create(etc[i].test_case);
        tcase_set_timeout(tc, 0);

        etc[i].build(tc);
        suite_add_tcase(s, tc);
     }

   sr = srunner_create(s);
   srunner_set_xml(sr, TESTS_BUILD_DIR "/check-results.xml");
   srunner_run_all(sr, CK_ENV);
   failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);

   return (failed_count == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

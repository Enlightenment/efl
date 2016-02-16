#ifndef EFL_CHECK_H
#define EFL_CHECK_H

#include <stdlib.h> /* getenv */
#include <stdio.h> /* fprintf, fputs */
#include <string.h> /* strcmp */

typedef struct _Efl_Test_Case Efl_Test_Case;
struct _Efl_Test_Case
{
   const char *test_case;
   void (*build)(TCase *tc);
};

static void
_efl_tests_list(const Efl_Test_Case *etc)
{
   const Efl_Test_Case *itr = etc;
      fputs("Available Test Cases:\n", stderr);
   for (; itr->test_case; itr++)
      fprintf(stderr, "\t%s\n", itr->test_case);
}

static int
_efl_test_option_disp(int argc, char **argv, const Efl_Test_Case *etc)
{
   int i;

   for (i = 1; i < argc; i++)
     {
        if ((strcmp(argv[i], "-h") == 0) ||
              (strcmp(argv[i], "--help") == 0))
          {
             fprintf(stderr, "Usage:\n\t%s [test_case1 .. [test_caseN]]\n",
                   argv[0]);
             _efl_tests_list(etc);
             return 0;
          }
        else if ((strcmp(argv[i], "-l") == 0) ||
              (strcmp(argv[i], "--list") == 0))
          {
             _efl_tests_list(etc);
             return 0;
          }
     }

   return 1;
}

static int
_efl_test_use(int argc, const char **argv, const char *test_case)
{
   if (argc < 1)
     return 1;

   for (; argc > 0; argc--, argv++)
     if (strcmp(test_case, *argv) == 0)
       return 1;
   return 0;
}

static int
_efl_test_fork_has(SRunner *sr)
{
   if (srunner_fork_status(sr) == CK_FORK)
     return 1;
   else if (srunner_fork_status(sr) == CK_NOFORK)
     return 0;
   else if (srunner_fork_status(sr) == CK_FORK_GETENV)
     {
        char *res;

        res = getenv("CF_FORK");
        if (res && (strcmp(res, "no") == 0))
          return 0;
        else
          return 1;
     }

   /* should never get there */
   return 0;
}

static int
_efl_suite_build_and_run(int argc, const char **argv, const char *suite_name, const Efl_Test_Case *etc)
{
   Suite *s;
   SRunner *sr;
   TCase *tc;
   int i, failed_count;

   s = suite_create(suite_name);
   sr = srunner_create(s);

   for (i = 0; etc[i].test_case; ++i)
     {
        if (!_efl_test_use(argc, argv, etc[i].test_case))
           continue;

        tc = tcase_create(etc[i].test_case);

        if (_efl_test_fork_has(sr))
          tcase_set_timeout(tc, 0);

        etc[i].build(tc);
        suite_add_tcase(s, tc);
     }

   srunner_set_xml(sr, TESTS_BUILD_DIR "/check-results.xml");
   srunner_run_all(sr, CK_ENV);
   failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);

   return failed_count;
}

#endif

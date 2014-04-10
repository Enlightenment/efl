#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include "Eo.h"
#include "eo_suite.h"

typedef struct _Eo_Test_Case Eo_Test_Case;

struct _Eo_Test_Case
{
   const char *test_case;
   void      (*build)(TCase *tc);
};

static const Eo_Test_Case etc[] = {
  { "Eo init", eo_test_init },
  { "Eo general", eo_test_general },
  { "Eo class errors", eo_test_class_errors },
  { "Eo call errors", eo_test_call_errors },
  { "Eo eina value", eo_test_value },
  { "Eo threaded eo calls", eo_test_threaded_calls },
  { NULL, NULL }
};

static void
_list_tests(void)
{
  const Eo_Test_Case *itr;

   itr = etc;
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

static Suite *
eo_suite_build(int argc, const char **argv)
{
   TCase *tc;
   Suite *s;
   int i;

   s = suite_create("Eo");

   for (i = 0; etc[i].test_case; ++i)
     {
	if (!_use_test(argc, argv, etc[i].test_case)) continue;
	tc = tcase_create(etc[i].test_case);

	etc[i].build(tc);

	suite_add_tcase(s, tc);
	tcase_set_timeout(tc, 0);
     }

   return s;
}

int
main(int argc, char **argv)
{
   Suite *s;
   SRunner *sr;
   int i, failed_count;
   eo_init();
   setenv("CK_FORK", "no", 0);

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

   s = eo_suite_build(argc - 1, (const char **)argv + 1);
   sr = srunner_create(s);

   srunner_set_xml(sr, TESTS_BUILD_DIR "/check-results.xml");

   srunner_run_all(sr, CK_ENV);
   failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);

   eo_shutdown();

   return (failed_count == 0) ? 0 : 255;
}

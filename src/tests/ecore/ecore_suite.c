#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include <Ecore.h>

#include "ecore_suite.h"

typedef struct _Ecore_Test_Case Ecore_Test_Case;

struct _Ecore_Test_Case
{
   const char *test_case;
   void      (*build)(TCase *tc);
};

static const Ecore_Test_Case etc[] = {
  { "Ecore", ecore_test_ecore },
  { "Ecore_Con", ecore_test_ecore_con },
  { "Ecore_X", ecore_test_ecore_x },
  { "Ecore_Imf", ecore_test_ecore_imf },
#if HAVE_ECORE_AUDIO
  { "Ecore Audio", ecore_test_ecore_audio},
#endif
  { "Ecore_Timers", ecore_test_timer },
  { "Ecore_Evas", ecore_test_ecore_evas },
  { "Ecore_Animators", ecore_test_animator },
  { "Ecore_Test_Ccore_Thread_Eina_Thread_Queue", ecore_test_ecore_thread_eina_thread_queue },
  { NULL, NULL }
};

static void
_list_tests(void)
{
  const Ecore_Test_Case *itr;

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
ecore_suite_build(int argc, const char **argv)
{
   TCase *tc;
   Suite *s;
   int i;

   s = suite_create("Ecore");

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

   s = ecore_suite_build(argc - 1, (const char **)argv + 1);
   sr = srunner_create(s);

   srunner_set_xml(sr, TESTS_BUILD_DIR "/check-results.xml");

   srunner_run_all(sr, CK_ENV);
   failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);

   return (failed_count == 0) ? 0 : 255;
}

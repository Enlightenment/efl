#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ejson_suite.h"

#include <Eina.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int _test_ejson_model_log_dom = -1;

typedef struct _Ejson_Test_Case Ejson_Test_Case;

struct _Ejson_Test_Case
{
   const char *test_case;
   void      (*build)(TCase *tc);
};

static const Ejson_Test_Case etc[] = {
  { "Ejson_Model", ejson_test_ejson_model },
  { NULL, NULL }
};

static void
_list_tests(void)
{
   const Ejson_Test_Case *it = etc;
   fputs("Available Test Cases:\n", stderr);
   for (; it->test_case; it++)
     fprintf(stderr, "\t%s\n", it->test_case);
}

static bool
_use_test(int argc, const char **argv, const char *test_case)
{
   if (argc < 1)
     return true;

   for (; argc > 0; argc--, argv++)
     if (strcmp(test_case, *argv) == 0)
       return true;
   return false;
}

static Suite *
_ejson_suite_build(int argc, const char **argv)
{
   Suite *s = suite_create("Ejson");

   for (int i = 0; etc[i].test_case; ++i)
     {
        if (!_use_test(argc, argv, etc[i].test_case)) continue;
        TCase *tc = tcase_create(etc[i].test_case);

        etc[i].build(tc);

        suite_add_tcase(s, tc);
     }

   return s;
}

static void
_init_logging(void)
{
   _test_ejson_model_log_dom = eina_log_domain_register("test_ejson_model", EINA_COLOR_LIGHTBLUE);
   if (_test_ejson_model_log_dom < 0)
     ck_abort_msg("Could not register log domain: test_ejson_model");

   eina_log_domain_level_set("test_ejson_model", EINA_LOG_LEVEL_DBG);
}

static void
_shutdown_logging(void)
{
   eina_log_domain_unregister(_test_ejson_model_log_dom);
   _test_ejson_model_log_dom = -1;
}

int
main(int argc, char **argv)
{
   for (int i = 1; i < argc; ++i)
     {
        if ((strcmp(argv[i], "-h") == 0) ||
            (strcmp(argv[i], "--help") == 0))
          {
             fprintf(stderr, "Usage:\n\t%s [test_case1 .. [test_caseN]]\n", argv[0]);
             _list_tests();
             return 0;
          }
        else if ((strcmp(argv[i], "-l") == 0) ||
                 (strcmp(argv[i], "--list") == 0))
          {
             _list_tests();
             return 0;
          }
     }

   _init_logging();

   Suite *s = _ejson_suite_build(argc - 1, (const char **)argv + 1);
   SRunner *sr = srunner_create(s);

   srunner_set_xml(sr, TESTS_BUILD_DIR "/check-results.xml");

   srunner_run_all(sr, CK_ENV);
   int failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);

   _shutdown_logging();

   return (failed_count == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

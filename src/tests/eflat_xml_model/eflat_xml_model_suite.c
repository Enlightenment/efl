#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eflat_xml_model_suite.h"

#include <Eina.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int _test_eflat_xml_model_log_dom = -1;

typedef struct _Eflat_Xml_Test_Case Eflat_Xml_Test_Case;

struct _Eflat_Xml_Test_Case
{
   const char *test_case;
   void      (*build)(TCase *tc);
};

static const Eflat_Xml_Test_Case etc[] = {
  { "Eflat_Xml_Model", eflat_xml_test_eflat_xml_model },
  { NULL, NULL }
};

static void
_list_tests(void)
{
   const Eflat_Xml_Test_Case *it = etc;
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
_eflat_xml_suite_build(int argc, const char **argv)
{
   Suite *s = suite_create("Eflat_Xml");

   for (int i = 0; etc[i].test_case; ++i)
     {
        if (!_use_test(argc, argv, etc[i].test_case)) continue;
        TCase *tc = tcase_create(etc[i].test_case);

        etc[i].build(tc);

        suite_add_tcase(s, tc);
        //tcase_set_timeout(tc, 0);
     }

   return s;
}

static void
_init_logging(void)
{
   _test_eflat_xml_model_log_dom = eina_log_domain_register("test_eflat_xml_model", EINA_COLOR_LIGHTBLUE);
   if (_test_eflat_xml_model_log_dom < 0)
     ck_abort_msg("Could not register log domain: test_eflat_xml_model");

   //eina_log_domain_level_set("esskyuehl", EINA_LOG_LEVEL_DBG);
   //eina_log_domain_level_set("eflat_xml_model", EINA_LOG_LEVEL_DBG);
   eina_log_domain_level_set("test_eflat_xml_model", EINA_LOG_LEVEL_DBG);
}

static void
_shutdown_logging(void)
{
   eina_log_domain_unregister(_test_eflat_xml_model_log_dom);
   _test_eflat_xml_model_log_dom = -1;
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

   Suite *s = _eflat_xml_suite_build(argc - 1, (const char **)argv + 1);
   SRunner *sr = srunner_create(s);

   srunner_set_xml(sr, TESTS_BUILD_DIR "/check-results.xml");

   srunner_run_all(sr, CK_ENV);
   int failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);

   _shutdown_logging();

   return (failed_count == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

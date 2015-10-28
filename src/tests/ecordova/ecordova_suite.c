#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* #include "ecordova_contacts_test.h" */
/* #include "ecordova_device_test.h" */
/* #include "ecordova_devicemotion_test.h" */
/* #include "ecordova_deviceorientation_test.h" */
/* #include "ecordova_geolocation_test.h" */
/* #include "ecordova_batterystatus_test.h" */
#include "ecordova_console_test.h"
/* #include "ecordova_filetransfer_test.h" */
/* #include "ecordova_media_test.h" */
/* #include "ecordova_networkinformation_test.h" */
/* #include "ecordova_vibration_test.h" */
#include "ecordova_directoryreader_test.h"
#include "ecordova_directoryentry_test.h"
#include "ecordova_entry_test.h"
#include "ecordova_file_test.h"
#include "ecordova_fileentry_test.h"
#include "ecordova_filereader_test.h"
#include "ecordova_filewriter_test.h"
/* #include "ecordova_mediafile_test.h" */
/* #include "ecordova_globalization_test.h" */

#include <Eina.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ecordova_suite.h"

int _test_ecordova_log_dom = -1;

typedef struct _Ecordova_Test_Case Ecordova_Test_Case;

struct _Ecordova_Test_Case
{
   const char *test_case;
   void      (*build)(TCase *tc);
};

static const Ecordova_Test_Case etc[] = {
  //{ "contacts", ecordova_contacts_test },
  /* { "device", ecordova_device_test }, */
  /* { "devicemotion", ecordova_devicemotion_test }, */
  /* { "deviceorientation", ecordova_deviceorientation_test }, */
  /* { "geolocation", ecordova_geolocation_test }, */
  /* { "batterystatus", ecordova_batterystatus_test }, */
  { "console", ecordova_console_test },
  /* { "filetransfer", ecordova_filetransfer_test }, */
  /* { "media", ecordova_media_test }, */
  /* { "networkinformation", ecordova_networkinformation_test }, */
  /* { "vibration", ecordova_vibration_test }, */
  { "directoryreader", ecordova_directoryreader_test },
  { "directoryentry", ecordova_directoryentry_test },
  { "entry", ecordova_entry_test },
  { "file", ecordova_file_test },
  { "fileentry", ecordova_fileentry_test },
  { "filereader", ecordova_filereader_test },
  { "filewriter", ecordova_filewriter_test },
  /* { "mediafile", ecordova_mediafile_test }, */
  /* { "globalization", ecordova_globalization_test }, */
  { NULL, NULL }
};

static void
_list_tests(void)
{
   const Ecordova_Test_Case *it = etc;
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
_ecordova_suite_build(int argc, const char **argv)
{
   Suite *s = suite_create("Ecordova");

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
   if (!eina_init())
     ck_abort_msg("Ecordova: Unable to initialize eina");

   _test_ecordova_log_dom = eina_log_domain_register("test_ecordova", EINA_COLOR_LIGHTBLUE);
   if (_test_ecordova_log_dom < 0)
     ck_abort_msg("Could not register log domain: test_ecordova");
}

static void
_shutdown_logging(void)
{
   eina_log_domain_unregister(_test_ecordova_log_dom);
   _test_ecordova_log_dom = -1;
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

   putenv("EFL_RUN_IN_TREE=1");

   srand(time(NULL));

   _init_logging();

   Suite *s = _ecordova_suite_build(argc - 1, (const char **)argv + 1);
   SRunner *sr = srunner_create(s);

   srunner_set_xml(sr, TESTS_BUILD_DIR "/check-results.xml");

   srunner_run_all(sr, CK_ENV);
   int failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);

   _shutdown_logging();

   return (failed_count == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

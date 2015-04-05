
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <stdio.h>
#include <string.h>

#include <check.h>

void emodel_test_file(TCase* tc);
void emodel_test_monitor_add(TCase* tc);

typedef struct _Emodel_Test_Case Emodel_Test_Case;
struct _Emodel_Test_Case
{
   const char *test_case;
   void (*build)(TCase *tc);
};

static const Emodel_Test_Case etc[] = {
   { "File", emodel_test_file },
   { "Monitor Add", emodel_test_monitor_add },
   { NULL, NULL }
};

static void
_list_tests(void)
{
   const Emodel_Test_Case *itr = etc;
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

Suite *
emodel_build_suite(int argc, const char **argv)
{
   TCase *tc;
   Suite *s;
   int i;

   s = suite_create("Emodel");

   for (i = 0; etc[i].test_case; ++i)
     {
        if (!_use_test(argc, argv, etc[i].test_case))
           continue;

        tc = tcase_create(etc[i].test_case);
        tcase_set_timeout(tc, 0);

        etc[i].build(tc);
        suite_add_tcase(s, tc);
     }

   return s;
}

/* FIXME this is a copy from eina_test_mempool
 * we should remove the duplication
 */
static Eina_Array *_modules;
static void _mempool_init(void)
{
   eina_init();
   /* force modules to be loaded in case they are not installed */
   _modules = eina_module_list_get(NULL,
                                   PACKAGE_BUILD_DIR "/src/modules",
                                   EINA_TRUE,
                                   NULL,
                                   NULL);
   eina_module_list_load(_modules);
}

static void _mempool_shutdown(void)
{
   eina_module_list_free(_modules);
   if (_modules)
     eina_array_free(_modules);
   /* TODO delete the list */
   eina_shutdown();
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

   s = emodel_build_suite(argc - 1, (const char **)argv + 1);
   sr = srunner_create(s);

   srunner_set_xml(sr, TESTS_BUILD_DIR "/check-results.xml");

   _mempool_init();

   srunner_run_all(sr, CK_ENV);
   failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);

   _mempool_shutdown();

   return (failed_count == 0) ? 0 : 255;
}

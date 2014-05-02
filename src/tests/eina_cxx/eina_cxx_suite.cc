
#include "Eina.hh"

#include <cassert>
#include <algorithm>

#include <check.h>

void eina_test_inlist(TCase* tc);
void eina_test_inarray(TCase* tc);
void eina_test_ptrlist(TCase* tc);
void eina_test_ptrarray(TCase* tc);
void eina_test_iterator(TCase* tc);
void eina_test_stringshare(TCase* tc);
void eina_test_error(TCase* tc);
void eina_test_accessor(TCase* tc);
void eina_test_thread(TCase* tc);
void eina_test_optional(TCase* tc);
void eina_test_value(TCase* tc);
void eina_test_log(TCase* tc);

typedef struct _Eina_Test_Case Eina_Test_Case;
struct _Eina_Test_Case
{
   const char *test_case;
   void (*build)(TCase *tc);
};

static const Eina_Test_Case etc[] = {
   { "Ptr_List", eina_test_ptrlist },
   { "Ptr_Array", eina_test_ptrarray },
   { "Inlist", eina_test_inlist },
   { "Inarray", eina_test_inarray },
   { "Iterator", eina_test_iterator },
   { "Stringshare", eina_test_stringshare },
   { "Error", eina_test_error },
   { "Accessor", eina_test_accessor },
   { "Thread", eina_test_thread },
   { "Optional", eina_test_optional },
   { "Value", eina_test_value },
   { "Log", eina_test_log },
   { NULL, NULL }
};

static void
_list_tests(void)
{
   const Eina_Test_Case *itr = etc;
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
eina_build_suite(int argc, const char **argv)
{
   TCase *tc;
   Suite *s;
   int i;

   s = suite_create("Eina");

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

int main(int argc, char* argv[])
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

   putenv(const_cast<char*>("EFL_RUN_IN_TREE=1"));

   s = eina_build_suite(argc - 1, (const char **)argv + 1);
   sr = srunner_create(s);

   srunner_set_xml(sr, TESTS_BUILD_DIR "/check-results.xml");

   eina_init();

   srunner_run_all(sr, CK_ENV);
   failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);

   eina_shutdown();

   return (failed_count == 0) ? 0 : 255;
}

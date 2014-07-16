#include <stdio.h>
#include <stdlib.h>
#include <check.h>
#include <cassert>

void eolian_cxx_test_parse(TCase* tc);
void eolian_cxx_test_generate(TCase* tc);

typedef struct _Eolian_Cxx_Test_Case Eolian_Cxx_Test_Case;
struct _Eolian_Cxx_Test_Case
{
   const char *test_case;
   void (*build)(TCase *tc);
};

static const Eolian_Cxx_Test_Case etc[] = {
  { "Eolian-Cxx Parsing", eolian_cxx_test_parse },
  { "Eolian-Cxx Generation", eolian_cxx_test_generate },
  { NULL, NULL }
};

static void
_list_tests(void)
{
  const Eolian_Cxx_Test_Case *itr;

   itr = etc;
   fputs("Available Test Cases:\n", stderr);
   for (; itr->test_case; itr++)
     fprintf(stderr, "\t%s\n", itr->test_case);
}

static bool
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
eolian_cxx_build_suite(int argc, const char **argv)
{
   TCase *tc;
   Suite *s;
   int i;

   s = suite_create("Eolian C++");

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

   s = eolian_cxx_build_suite(argc - 1, (const char **)argv + 1);
   sr = srunner_create(s);

   srunner_set_xml(sr, TESTS_BUILD_DIR "/check-results.xml");

   srunner_run_all(sr, CK_ENV);
   failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);

   return (failed_count == 0) ? 0 : 255;
}

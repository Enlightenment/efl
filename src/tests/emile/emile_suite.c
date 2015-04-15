#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>

#include <check.h>

#include <Eina.h>
#include <Emile.h>

START_TEST(emile_test_init)
{
   fail_if(emile_init() <= 0);
   fail_if(emile_shutdown() != 0);
}
END_TEST

static void
emile_base_test(TCase *tc)
{
   tcase_add_test(tc, emile_test_init);
}

static const struct
{
   const char *name;
   void        (*build)(TCase *tc);
} tests[] = {
  {
    "Emile_Base", emile_base_test
  }
};

static void
_list_tests(void)
{
   unsigned int i;

   fputs("Available tests cases :\n", stderr);
   for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++)
     fprintf(stderr, "\t%s\n", tests[i].name);
}

static Eina_Bool
_use_test(const char *name, int argc, const char *argv[])
{
   argc--;
   argv--;

   if (argc < 1)
     return EINA_TRUE;

   for (; argc > 1; argc--, argv++)
     if (strcmp(name, *argv) == 0)
       return EINA_TRUE;
   return EINA_FALSE;
}

static Suite *
emile_suite_build(int argc, const char **argv)
{
   TCase *tc;
   Suite *s;
   unsigned int i;

   s = suite_create("Emile");

   for (i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i)
     {
        if (!_use_test(tests[i].name, argc, argv))
          continue;

        tc = tcase_create(tests[i].name);
        tests[i].build(tc);
        suite_add_tcase(s, tc);
#ifndef _WIN32
        tcase_set_timeout(tc, 0);
#endif
     }

   return s;
}

int
main(int argc, char *argv[])
{
   SRunner *sr;
   Suite *s;
   int failed_count;
   int j;

   for (j = 1; j < argc; j++)
     if ((strcmp(argv[j], "-h") == 0) || (strcmp(argv[j], "--help") == 0))
       {
          fprintf(stderr, "Usage:\n\t%s [test_case1 .. [test_caseN]]\n", argv[0]);
          _list_tests();
          return 0;
       }
     else if ((strcmp(argv[j], "-l") == 0) || (strcmp(argv[j], "--list") == 0))
       {
          _list_tests();
          return 0;
       }

   s = emile_suite_build(argc, (const char **)argv);
   sr = srunner_create(s);

   srunner_set_xml(sr, TESTS_BUILD_DIR "/check-results.xml");
   srunner_run_all(sr, CK_ENV);
   failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);

   return (failed_count == 0) ? 0 : 255;
}

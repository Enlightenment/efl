#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_suite.h"

typedef struct _Elementary_Test_Case Elementary_Test_Case;

struct _Elementary_Test_Case
{
   const char *test_case;
   void      (*build)(TCase *tc);
};

static const Elementary_Test_Case etc[] = {
  { "Elementary", elm_test_init },
  { "elm_check", elm_test_check },
  { "elm_colorselector", elm_test_colorselector },
  { "elm_entry", elm_test_entry},
  { "elm_atspi", elm_test_atspi},
  { NULL, NULL }
};

Suite *
elm_suite()
{
   TCase *tc;
   Suite *s;
   int i;

   s = suite_create("Elementary");

   for (i = 0; etc[i].test_case; ++i)
     {
        tc = tcase_create(etc[i].test_case);
        etc[i].build(tc);
        suite_add_tcase(s, tc);
        tcase_set_timeout(tc, 0);
     }

   return s;
}

int
main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   int failed_count;
   Suite *s;
   SRunner *sr;

   s = elm_suite();
   sr = srunner_create(s);

   srunner_set_xml(sr, TESTS_BUILD_DIR "/check-results.xml");

   srunner_run_all(sr, CK_ENV);
   failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);

   return (failed_count == 0) ? 0 : 255;
}

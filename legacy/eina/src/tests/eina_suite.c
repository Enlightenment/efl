#include "eina_suite.h"

typedef struct _Eina_Test_Case Eina_Test_Case;
struct _Eina_Test_Case
{
   const char *test_case;
   void (*build)(TCase *tc);
};

static const Eina_Test_Case etc[] = {
  { "Array", eina_test_array },
  { "String Share", eina_test_stringshare },
  { "Error", eina_test_error },
  { "Magic", eina_test_magic },
  { NULL, NULL }
};

Suite *
eina_build_suite(void)
{
   TCase *tc;
   Suite *s;
   int i;

   s = suite_create("Eina");

   for (i = 0; etc[i].test_case != NULL; ++i)
     {
	tc = tcase_create(etc[i].test_case);

	etc[i].build(tc);

	suite_add_tcase(s, tc);
     }

   return s;
}

int
main(void)
{
   Suite *s;
   SRunner *sr;
   int failed_count;


   s = eina_build_suite();
   sr = srunner_create(s);
   srunner_run_all(sr, CK_NORMAL);
   failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);

   return (failed_count == 0) ? 0 : 255;
}

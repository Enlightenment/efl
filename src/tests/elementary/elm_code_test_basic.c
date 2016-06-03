#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERNAL_API_ARGESFSDFEFC

#include "elm_suite.h"
#include "Elementary.h"

START_TEST (elm_code_create_test)
{
   char *path = TESTS_SRC_DIR "/testfile.txt";
   Elm_Code *code;

   elm_init(1, NULL);
   code = elm_code_create();
   elm_code_file_open(code, path);

   ck_assert(!!code);
   elm_code_free(code);
   elm_shutdown();
}
END_TEST

void elm_code_test_basic(TCase *tc)
{
   tcase_add_test(tc, elm_code_create_test);
}

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "elm_code_suite.h"

START_TEST (elm_code_create_test)
{
   char *path = "elm_code/src/tests/testfile.txt";
   Elm_Code *code;

   code = elm_code_create();
   elm_code_file_open(code, path);

   ck_assert(!!code);
   elm_code_free(code);
}
END_TEST

void elm_code_test_basic(TCase *tc)
{
   tcase_add_test(tc, elm_code_create_test);
}


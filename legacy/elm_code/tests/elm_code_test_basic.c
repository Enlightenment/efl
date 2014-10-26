#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "elm_code_suite.h"

START_TEST (elm_code_create_test)
{
   char *path = "elm_code/tests/testfile.txt";
   Elm_Code_File *file;
   Elm_Code *code;

   file = elm_code_file_open(path);
   code = elm_code_create(file);

   ck_assert(code);
   elm_code_free(code);
}
END_TEST

void elm_code_test_basic(TCase *tc)
{
   tcase_add_test(tc, elm_code_create_test);
}


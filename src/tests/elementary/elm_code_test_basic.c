#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <stdlib.h>

#include "elm_suite.h"
#include "Elementary.h"

EFL_START_TEST(elm_code_create_test)
{
   Elm_Code *code;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();

   ck_assert(!!code);
   ck_assert(elm_code_file_path_get(code->file) == NULL);
   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST(elm_code_open_test)
{
   char *path = TESTS_SRC_DIR "/testfile.txt";
   char realpath1[PATH_MAX], realpath2[PATH_MAX];
   Elm_Code *code;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   elm_code_file_open(code, path);

   ck_assert_ptr_ne(realpath(path, realpath1), NULL);
   ck_assert_ptr_ne(realpath(elm_code_file_path_get(code->file), realpath2), NULL);
   ck_assert(!!code);
   ck_assert_str_eq(realpath1, realpath2);
   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST


void elm_code_test_basic(TCase *tc)
{
   tcase_add_test(tc, elm_code_create_test);
   tcase_add_test(tc, elm_code_open_test);
}

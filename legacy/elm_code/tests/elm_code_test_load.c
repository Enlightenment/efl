#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "elm_code_suite.h"

START_TEST (elm_code_load)
{
   char *path = "elm_code/tests/testfile.txt";
   char real[EINA_PATH_MAX];
   Elm_Code_File *file;

   file = elm_code_open(path);
   realpath(path, real);

   ck_assert_str_eq(basename(path), elm_code_filename_get(file));
   ck_assert_str_eq(real, elm_code_path_get(file));
   elm_code_close(file);
}
END_TEST

START_TEST (elm_code_load_lines)
{
   char *path = "elm_code/tests/testfile.txt";
   Elm_Code_File *file;

   file = elm_code_open(path);

   ck_assert(4 == elm_code_lines_get(file));
   elm_code_close(file);
}
END_TEST

void elm_code_test_load(TCase *tc)
{
   tcase_add_test(tc, elm_code_load);
   tcase_add_test(tc, elm_code_load_lines);
}


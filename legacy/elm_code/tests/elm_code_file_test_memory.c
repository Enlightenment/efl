#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "elm_code_suite.h"

START_TEST (elm_code_file_memory_lines)
{
   Elm_Code_File *file;

   file = elm_code_file_new();
   ck_assert_uint_eq(0, elm_code_file_lines_get(file));

   elm_code_file_line_append(file, "a line");

   ck_assert_uint_eq(1, elm_code_file_lines_get(file));
   elm_code_file_free(file);
}
END_TEST

void elm_code_file_test_memory(TCase *tc)
{
   tcase_add_test(tc, elm_code_file_memory_lines);
}


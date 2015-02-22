#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "elm_code_suite.h"
#include "elm_code_text.h"

START_TEST (elm_code_text_get_test)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line;

   code = elm_code_create();
   file = elm_code_file_new(code);

   elm_code_file_line_append(file, "test", 4, NULL);
   line = elm_code_file_line_get(file, 1);
   ck_assert_str_eq("test", elm_code_line_text_get(line, NULL));
}
END_TEST

START_TEST (elm_code_text_insert_test)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line;

   code = elm_code_create();
   file = elm_code_file_new(code);

   elm_code_file_line_append(file, "test", 4, NULL);
   line = elm_code_file_line_get(file, 1);

   elm_code_line_text_insert(line, 5, "ing", 3);
   ck_assert_str_eq("testing", elm_code_line_text_get(line, NULL));
}
END_TEST

void elm_code_test_text(TCase *tc)
{
   tcase_add_test(tc, elm_code_text_get_test);
   tcase_add_test(tc, elm_code_text_insert_test);
}

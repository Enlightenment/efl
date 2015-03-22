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

START_TEST (elm_code_text_contains_test)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line;

   code = elm_code_create();
   file = elm_code_file_new(code);

   elm_code_file_line_append(file, "a test string...", 17, NULL);
   line = elm_code_file_line_get(file, 1);

   ck_assert_int_eq(EINA_TRUE, elm_code_line_text_contains(line, "test"));
   ck_assert_int_eq(EINA_FALSE, elm_code_line_text_contains(line, "text"));

   ck_assert_int_eq(EINA_TRUE, elm_code_line_text_contains(line, "a t"));
   ck_assert_int_eq(EINA_TRUE, elm_code_line_text_contains(line, "..."));
}
END_TEST

START_TEST (elm_code_text_strpos_test)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line;

   code = elm_code_create();
   file = elm_code_file_new(code);

   elm_code_file_line_append(file, "a test string...", 17, NULL);
   line = elm_code_file_line_get(file, 1);

   ck_assert_int_eq(2, elm_code_line_text_strpos(line, "test", 0));
   ck_assert_int_eq(2, elm_code_line_text_strpos(line, "test", 1));
   ck_assert_int_eq(2, elm_code_line_text_strpos(line, "test", 2));
   ck_assert_int_eq(ELM_CODE_TEXT_NOT_FOUND, elm_code_line_text_strpos(line, "test", 5));
   ck_assert_int_eq(ELM_CODE_TEXT_NOT_FOUND, elm_code_line_text_strpos(line, "text", 0));

   ck_assert_int_eq(0, elm_code_line_text_strpos(line, "a t", 0));
   ck_assert_int_eq(13, elm_code_line_text_strpos(line, "...", 0));
}
END_TEST

void elm_code_test_text(TCase *tc)
{
   tcase_add_test(tc, elm_code_text_get_test);
   tcase_add_test(tc, elm_code_text_insert_test);
   tcase_add_test(tc, elm_code_text_contains_test);
   tcase_add_test(tc, elm_code_text_strpos_test);
}

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

   elm_code_line_text_insert(line, 4, "ing", 3);
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

   elm_code_file_line_append(file, "a test string...", 16, NULL);
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

   elm_code_file_line_append(file, "a test string...", 16, NULL);
   line = elm_code_file_line_get(file, 1);

   ck_assert_int_eq(2, elm_code_line_text_strpos(line, "test", 0));
   ck_assert_int_eq(2, elm_code_line_text_strpos(line, "test", 1));
   ck_assert_int_eq(2, elm_code_line_text_strpos(line, "test", 2));
   ck_assert_int_eq(ELM_CODE_TEXT_NOT_FOUND, elm_code_line_text_strpos(line, "test", 5));
   ck_assert_int_eq(ELM_CODE_TEXT_NOT_FOUND, elm_code_line_text_strpos(line, "text", 0));

   ck_assert_int_eq(0, elm_code_line_text_strpos(line, "a t", 0));
   ck_assert_int_eq(ELM_CODE_TEXT_NOT_FOUND, elm_code_line_text_strpos(line, "a t", 2));
   ck_assert_int_eq(13, elm_code_line_text_strpos(line, "...", 0));
}
END_TEST

START_TEST (elm_code_text_newline_position_test)
{
   short nllen;
   const char *unixtext = "a test\nwith newline";
   const char *wintext = "a windows\r\nnewline";

   ck_assert_int_eq(6, elm_code_text_newlinenpos(unixtext, strlen(unixtext), &nllen));
   ck_assert_int_eq(1, nllen);
   ck_assert_int_eq(9, elm_code_text_newlinenpos(wintext, strlen(wintext), &nllen));
   ck_assert_int_eq(2, nllen);
}
END_TEST

START_TEST (elm_code_text_leading_whitespace_test)
{
   const char *text;

   text = "testing";
   ck_assert_int_eq(0, elm_code_text_leading_whitespace_length(text, strlen(text)));

   text = "  spaces";
   ck_assert_int_eq(2, elm_code_text_leading_whitespace_length(text, strlen(text)));

   text = "\t\ttabs";
   ck_assert_int_eq(2, elm_code_text_leading_whitespace_length(text, strlen(text)));

   text = " \t mix";
   ck_assert_int_eq(3, elm_code_text_leading_whitespace_length(text, strlen(text)));
}
END_TEST

START_TEST (elm_code_text_trailing_whitespace_test)
{
   const char *text;

   text = "testing";
   ck_assert_int_eq(0, elm_code_text_trailing_whitespace_length(text, strlen(text)));

   text = "spaces  ";
   ck_assert_int_eq(2, elm_code_text_trailing_whitespace_length(text, strlen(text)));

   text = "tabs\t\t";
   ck_assert_int_eq(2, elm_code_text_trailing_whitespace_length(text, strlen(text)));

   text = "mix \t ";
   ck_assert_int_eq(3, elm_code_text_trailing_whitespace_length(text, strlen(text)));
}
END_TEST

START_TEST (elm_code_text_is_whitespace_test)
{
   const char *text;

   text = " ";
   ck_assert_int_eq(1, elm_code_text_is_whitespace(text, strlen(text)));

   text = " \t\t ";
   ck_assert_int_eq(1, elm_code_text_is_whitespace(text, strlen(text)));

   text = " . ";
   ck_assert_int_eq(0, elm_code_text_is_whitespace(text, strlen(text)));
}
END_TEST

void elm_code_test_text(TCase *tc)
{
   tcase_add_test(tc, elm_code_text_get_test);
   tcase_add_test(tc, elm_code_text_insert_test);
   tcase_add_test(tc, elm_code_text_contains_test);
   tcase_add_test(tc, elm_code_text_strpos_test);
   tcase_add_test(tc, elm_code_text_newline_position_test);
   tcase_add_test(tc, elm_code_text_leading_whitespace_test);
   tcase_add_test(tc, elm_code_text_trailing_whitespace_test);
   tcase_add_test(tc, elm_code_text_is_whitespace_test);
}

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "elm_suite.h"
#include "Elementary.h"

EFL_START_TEST (elm_code_line_create_test)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);

   elm_code_file_line_append(file, "a test string...", 16, NULL);
   line = elm_code_file_line_get(file, 1);

   ck_assert(!!line);

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_line_token_count_test)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);

   elm_code_file_line_append(file, "a test string...", 16, NULL);
   line = elm_code_file_line_get(file, 1);

   ck_assert_int_eq(0, eina_list_count(line->tokens));
   elm_code_line_token_add(line, 2, 5, 1, ELM_CODE_TOKEN_TYPE_COMMENT);
   ck_assert_int_eq(1, eina_list_count(line->tokens));
   elm_code_line_tokens_clear(line);
   ck_assert_int_eq(0, eina_list_count(line->tokens));

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_line_split_test)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line, *newline;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);

   elm_code_file_line_append(file, "line1line2", 10, NULL);
   line = elm_code_file_line_get(file, 1);
   ck_assert_int_eq(1, elm_code_file_lines_get(file));
   ck_assert_int_eq(10, line->length);

   elm_code_line_split_at(line, 5);
   ck_assert_int_eq(2, elm_code_file_lines_get(file));
   newline = elm_code_file_line_get(file, 2);
   ck_assert_int_eq(5, line->length);
   ck_assert_int_eq(5, newline->length);
   elm_shutdown();
}
EFL_END_TEST

void elm_code_test_line(TCase *tc)
{
   tcase_add_test(tc, elm_code_line_create_test);
   tcase_add_test(tc, elm_code_line_token_count_test);
   tcase_add_test(tc, elm_code_line_split_test);
}

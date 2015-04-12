#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "elm_code_suite.h"

START_TEST (elm_code_line_create_test)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line;

   code = elm_code_create();
   file = elm_code_file_new(code);

   elm_code_file_line_append(file, "a test string...", 16, NULL);
   line = elm_code_file_line_get(file, 1);

   ck_assert(!!line);

   elm_code_free(code);
}
END_TEST

START_TEST (elm_code_line_token_count_test)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line;

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
}
END_TEST

void elm_code_test_line(TCase *tc)
{
   tcase_add_test(tc, elm_code_line_create_test);
   tcase_add_test(tc, elm_code_line_token_count_test);
}


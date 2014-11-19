#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "elm_code_suite.h"

START_TEST (elm_code_file_memory_lines)
{
   Elm_Code_File *file;
   Elm_Code *code;

   code = elm_code_create();
   file = elm_code_file_new(code);
   ck_assert_uint_eq(0, elm_code_file_lines_get(file));

   elm_code_file_line_append(file, "a line", 6);

   ck_assert_uint_eq(1, elm_code_file_lines_get(file));
   elm_code_free(code);
}
END_TEST

START_TEST (elm_code_file_memory_tokens)
{
   Elm_Code_File *file;
   Elm_Code_Line *line;
   Elm_Code *code;

   code = elm_code_create();
   file = elm_code_file_new(code);

   elm_code_file_line_append(file, "a line", 6);
   elm_code_file_line_token_add(file, 1, 2, 5, ELM_CODE_TOKEN_TYPE_COMMENT);

   line = elm_code_file_line_get(file, 1);
   ck_assert_uint_eq(1, eina_list_count(line->tokens));
   elm_code_free(code);
}
END_TEST

void elm_code_file_test_memory(TCase *tc)
{
   tcase_add_test(tc, elm_code_file_memory_lines);
   tcase_add_test(tc, elm_code_file_memory_tokens);
}


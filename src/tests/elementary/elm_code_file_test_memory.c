#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERNAL_API_ARGESFSDFEFC

#include "elm_suite.h"
#include "Elementary.h"

EFL_START_TEST (elm_code_file_memory_lines)
{
   Elm_Code *code;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   ck_assert_uint_eq(0, elm_code_file_lines_get(code->file));

   elm_code_file_line_append(code->file, "a line", 6, NULL);

   ck_assert_uint_eq(1, elm_code_file_lines_get(code->file));
   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_file_memory_tokens)
{
   Elm_Code_File *file;
   Elm_Code_Line *line;
   Elm_Code *code;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = code->file;
   elm_code_file_line_append(file, "a line", 6, NULL);

   line = elm_code_file_line_get(file, 1);
   elm_code_line_token_add(line, 2, 5, 1, ELM_CODE_TOKEN_TYPE_COMMENT);
   ck_assert_uint_eq(1, eina_list_count(line->tokens));
   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

void elm_code_file_test_memory(TCase *tc)
{
   tcase_add_test(tc, elm_code_file_memory_lines);
   tcase_add_test(tc, elm_code_file_memory_tokens);
}

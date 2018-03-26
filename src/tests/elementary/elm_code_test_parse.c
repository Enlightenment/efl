#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERNAL_API_ARGESFSDFEFC

#include "elm_suite.h"
#include "Elementary.h"
#include "elm_code_parse.h"

static int line_calls, file_calls;

static void _parser_line_callback(Elm_Code_Line *line EINA_UNUSED, void *data EINA_UNUSED)
{
   line_calls++;
}

static void _parser_file_callback(Elm_Code_File *file EINA_UNUSED, void *data EINA_UNUSED)
{
   file_calls++;
}

EFL_START_TEST (elm_code_parse_hook_memory_test)
{
   Elm_Code *code;
   Elm_Code_File *file;

   line_calls = 0;
   file_calls = 0;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);

   elm_code_parser_add(code, _parser_line_callback, _parser_file_callback, NULL);
   elm_code_file_line_append(file, "some \"test content\" for parsing", 31, NULL);

   ck_assert_int_eq(1, line_calls);
   ck_assert_int_eq(0, file_calls);

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_parse_hook_file_test)
{
   Elm_Code *code;
   Elm_Code_File *file;
   char *path = TESTS_SRC_DIR "/testfile.txt";

   line_calls = 0;
   file_calls = 0;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();

   elm_code_parser_add(code, _parser_line_callback, _parser_file_callback, NULL);
   file = elm_code_file_open(code, path);

   ck_assert_int_eq(4, line_calls);
   ck_assert_int_eq(1, file_calls);

   elm_code_file_close(file);
   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_parse_todo_test)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   elm_code_parser_standard_add(code, ELM_CODE_PARSER_STANDARD_TODO);
   file = elm_code_file_new(code);

   elm_code_file_line_append(file, "xxx TODO line", 13, NULL);
   line = elm_code_file_line_get(file, 1);
   ck_assert_int_eq(ELM_CODE_STATUS_TYPE_TODO, line->status);

   elm_code_line_text_set(line, "FIXME too", 9);
   ck_assert_int_eq(ELM_CODE_STATUS_TYPE_TODO, line->status);

   elm_code_line_text_set(line, "TOFIX", 5);
   ck_assert_int_eq(ELM_CODE_STATUS_TYPE_DEFAULT, line->status);
   elm_shutdown();
}
EFL_END_TEST

void elm_code_test_parse(TCase *tc)
{
   tcase_add_test(tc, elm_code_parse_hook_memory_test);
   tcase_add_test(tc, elm_code_parse_hook_file_test);
   tcase_add_test(tc, elm_code_parse_todo_test);
}

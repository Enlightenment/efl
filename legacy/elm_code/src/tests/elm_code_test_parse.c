#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "elm_code_suite.h"
#include "elm_code_parse.h"

static int line_calls, file_calls;

static void _parser_line_callback(Elm_Code_Line *line EINA_UNUSED)
{
   line_calls++;
}

static void _parser_file_callback(Elm_Code_File *file EINA_UNUSED)
{
   file_calls++;
}

START_TEST (elm_code_parse_hook_memory_test)
{
   Elm_Code *code;
   Elm_Code_File *file;

   line_calls = 0;
   file_calls = 0;

   code = elm_code_create();
   file = elm_code_file_new(code);

   elm_code_parser_add(code, _parser_line_callback, _parser_file_callback);
   elm_code_file_line_append(file, "some \"test content\" for parsing", 31, NULL);

   ck_assert_int_eq(1, line_calls);
   ck_assert_int_eq(0, file_calls);

   elm_code_free(code);
}
END_TEST

START_TEST (elm_code_parse_hook_file_test)
{
   Elm_Code *code;
   Elm_Code_File *file;
   char *path = TESTS_DIR "testfile.txt";

   line_calls = 0;
   file_calls = 0;

   code = elm_code_create();

   elm_code_parser_add(code, _parser_line_callback, _parser_file_callback);
   file = elm_code_file_open(code, path);

   ck_assert_int_eq(4, line_calls);
   ck_assert_int_eq(1, file_calls);

   elm_code_file_close(file);
   elm_code_free(code);
}
END_TEST

void elm_code_test_parse(TCase *tc)
{
   tcase_add_test(tc, elm_code_parse_hook_memory_test);
   tcase_add_test(tc, elm_code_parse_hook_file_test);
}


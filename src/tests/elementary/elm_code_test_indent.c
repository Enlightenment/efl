#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERNAL_API_ARGESFSDFEFC

#include "elm_suite.h"
#include "Elementary.h"
#include "elm_code_indent.h"

static void
_indent_check(Elm_Code_File *file, const char *prev, const char *expected)
{
   Elm_Code_Line *line;
   char *str;

   elm_code_file_clear(file);

   elm_code_file_line_append(file, prev, strlen(prev), NULL);
   elm_code_file_line_append(file, "", 0, NULL);
   line = elm_code_file_line_get(file, 2);

   str = elm_code_line_indent_get(line);
   ck_assert_str_eq(expected, str);

   free(str);
}

EFL_START_TEST (elm_code_indent_whitespace_test)
{
   Elm_Code *code;
   Elm_Code_File *file;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);

   _indent_check(file, "", "");
   _indent_check(file, "  ", "  ");
   _indent_check(file, "\t", "\t");
   _indent_check(file, "\t  ", "\t  ");

   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_indent_comments_test)
{
   Elm_Code *code;
   Elm_Code_File *file;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);

   _indent_check(file, " /**", "  * ");
   _indent_check(file, "  * ", "  * ");
   _indent_check(file, "  */", " ");
   _indent_check(file, "\t//", "\t//");

   // test these are not comments
   _indent_check(file, " / ", " ");
   _indent_check(file, " hi//", " ");

   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_indent_simple_braces)
{
   Elm_Code *code;
   Elm_Code_File *file;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);

   _indent_check(file, "if() {", "     ");
   _indent_check(file, "}", "");

   _indent_check(file, "  {", "     ");
   _indent_check(file, "  }", "");

   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_indent_matching_braces)
{
   Elm_Code_File *file;
   Elm_Code_Line *line;
   Elm_Code *code;
   const char *str;
   unsigned int str_len;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);

   elm_code_file_line_append(file, "", 8, NULL);
   line = elm_code_file_line_get(file, 1);

   elm_code_file_line_insert(file, 1, "   if ()", 8, NULL);
   str = elm_code_line_indent_matching_braces_get(line, &str_len);
   ck_assert_strn_eq(str, "", str_len);

   elm_code_file_line_insert(file, 2, "     {", 6, NULL);
   str = elm_code_line_indent_matching_braces_get(line, &str_len);
   ck_assert_strn_eq(str, "     ", str_len);

   elm_code_file_line_insert(file, 3, "        if (){", 14, NULL);
   str = elm_code_line_indent_matching_braces_get(line, &str_len);
   ck_assert_strn_eq(str, "        ", str_len);

   elm_code_file_line_insert(file, 4, "        }", 9, NULL);
   str = elm_code_line_indent_matching_braces_get(line, &str_len);
   ck_assert_strn_eq(str, "     ", str_len);

   elm_code_file_line_insert(file, 5, "     }", 6, NULL);
   str = elm_code_line_indent_matching_braces_get(line, &str_len);
   ck_assert_strn_eq(str, "", str_len);

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_indent_startswith_keyword)
{
   Elm_Code_File *file;
   Elm_Code *code;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);

   _indent_check(file, "if ()", "  ");
   _indent_check(file, "else", "  ");
   _indent_check(file, "else if ()", "  ");
   _indent_check(file, "for ()", "  ");
   _indent_check(file, "while ()", "  ");
   _indent_check(file, "do", "  ");
   _indent_check(file, "do {", "     ");

   _indent_check(file, "  switch ()", "    ");
   _indent_check(file, "   case a:", "     ");
   _indent_check(file, "   default:", "     ");

   _indent_check(file, "if ();", "");
   _indent_check(file, "  for ();", "  ");

   _indent_check(file, "  iffy()", "  ");
   _indent_check(file, "  fi()", "  ");
   _indent_check(file, "  elihw", "  ");

   _indent_check(file, "   if", "   ");
   _indent_check(file, "   while", "   ");

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

void elm_code_test_indent(TCase *tc)
{
   tcase_add_test(tc, elm_code_indent_whitespace_test);
   tcase_add_test(tc, elm_code_indent_comments_test);
   tcase_add_test(tc, elm_code_indent_simple_braces);
   tcase_add_test(tc, elm_code_indent_matching_braces);
   tcase_add_test(tc, elm_code_indent_startswith_keyword);
}

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERNAL_API_ARGESFSDFEFC

#include "elm_suite.h"
#include "Elementary.h"
#include "elm_code_indent.h"

START_TEST (elm_code_indent_whitespace_test)
{
   const char *str;

   str = elm_code_line_indent_get("", 0);
   ck_assert_str_eq("", str);
   str = elm_code_line_indent_get("  ", 2);
   ck_assert_str_eq("  ", str);
   str = elm_code_line_indent_get("\t", 1);
   ck_assert_str_eq("\t", str);
   str = elm_code_line_indent_get("\t  ", 3);
   ck_assert_str_eq("\t  ", str);
}
END_TEST

START_TEST (elm_code_indent_comments_test)
{
   const char *str;

   str = elm_code_line_indent_get(" /**", 4);
   ck_assert_str_eq("  * ", str);
   str = elm_code_line_indent_get("  * ", 4);
   ck_assert_str_eq("  * ", str);
   str = elm_code_line_indent_get("  */", 4);
   ck_assert_str_eq(" ", str);
   str = elm_code_line_indent_get("\t//", 3);
   ck_assert_str_eq("\t//", str);

   // test these are not comments
   str = elm_code_line_indent_get(" / ", 3);
   ck_assert_str_eq(" ", str);
   str = elm_code_line_indent_get(" hi//", 5);
   ck_assert_str_eq(" ", str);
}
END_TEST

START_TEST (elm_code_indent_simple_braces)
{
   const char *str;

   str = elm_code_line_indent_get("if() {", 6);
   ck_assert_str_eq("   ", str);
   str = elm_code_line_indent_get("}", 1);
   ck_assert_str_eq("", str);

   str = elm_code_line_indent_get("  {", 3);
   ck_assert_str_eq("     ", str);
   str = elm_code_line_indent_get("  }", 3);
   ck_assert_str_eq("", str);
}
END_TEST

START_TEST (elm_code_indent_matching_braces)
{
   Elm_Code_File *file;
   Elm_Code_Line *line;
   Elm_Code *code;
   const char *str;

   elm_init(1, NULL);
   code = elm_code_create();
   file = elm_code_file_new(code);

   elm_code_file_line_append(file, "", 8, NULL);
   line = elm_code_file_line_get(file, 1);

   elm_code_file_line_insert(file, 1, "   if ()", 8, NULL);
   str = elm_code_line_indent_matching_braces_get(line);
   ck_assert_str_eq("", str);

   elm_code_file_line_insert(file, 2, "     {", 6, NULL);
   str = elm_code_line_indent_matching_braces_get(line);
   ck_assert_str_eq("     ", str);

   elm_code_file_line_insert(file, 3, "        if (){", 14, NULL);
   str = elm_code_line_indent_matching_braces_get(line);
   ck_assert_str_eq("        ", str);

   elm_code_file_line_insert(file, 4, "        }", 9, NULL);
   str = elm_code_line_indent_matching_braces_get(line);
   ck_assert_str_eq("     ", str);

   elm_code_file_line_insert(file, 5, "     }", 6, NULL);
   str = elm_code_line_indent_matching_braces_get(line);
   ck_assert_str_eq("", str);

   elm_code_free(code);
   elm_shutdown();
}
END_TEST

void elm_code_test_indent(TCase *tc)
{
   tcase_add_test(tc, elm_code_indent_whitespace_test);
   tcase_add_test(tc, elm_code_indent_comments_test);
   tcase_add_test(tc, elm_code_indent_simple_braces);
   tcase_add_test(tc, elm_code_indent_matching_braces);
}

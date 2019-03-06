#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "elm_suite.h"
#include "Elementary.h"
#include "elm_code_syntax.h"

static void
_append_line(Elm_Code_File *file, const char *line)
{
   int length;

   length = strlen(line);
   elm_code_file_line_append(file, line, length, NULL);
}

static void
_assert_line_token_types(Elm_Code_File *file, unsigned int number,unsigned int count, Elm_Code_Token_Type types[])
{
   Elm_Code_Line *line;
   unsigned int found, i;

   line = elm_code_file_line_get(file, number);
   if (line->tokens)
     found = eina_list_count(line->tokens);
   else
     found = 0;
   ck_assert_msg(found == count, "Bad token count %d on line %d - expected %d", found, number, count);

   for (i = 0; i < found; i++)
     {
        Elm_Code_Token *token;

        token = eina_list_nth(line->tokens, i);
        ck_assert_msg(token->type == types[i], "Token mismatch (%d!=%d) on line %d", token->type, types[i], number);
     }
}

EFL_START_TEST(elm_code_syntax_lookup)
{
   Elm_Code_Syntax *syntax;

   syntax = elm_code_syntax_for_mime_get("text/x-csrc");
   ck_assert(!!syntax);
   syntax = elm_code_syntax_for_mime_get("text/x-chdr");
   ck_assert(!!syntax);
   syntax = elm_code_syntax_for_mime_get("text/unknown");
   ck_assert(!syntax);
}
EFL_END_TEST

EFL_START_TEST(elm_code_syntax_c)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Widget *widget;
   Evas_Object *win;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);

   win = win_add(NULL, "syntax", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);
   efl_ui_code_widget_code_get(widget)->file->mime = "text/x-csrc";
   elm_code_widget_syntax_enabled_set(widget, EINA_TRUE);

   _append_line(file, "#include <stdio.h>");
   _append_line(file, "#define MULTILINE \"Some text covering \"\\");
   _append_line(file, "   \"multiple lines\"");
   _append_line(file, "");
   _append_line(file, "/**");
   _append_line(file, " * The main method.");
   _append_line(file, " */");
   _append_line(file, "int main(int argc, char **argv)");
   _append_line(file, "{");
   _append_line(file, "   char c = '\\\\';");
   _append_line(file, "   // display a welcome greeting");
   _append_line(file, "   if (argc > 0)");
   _append_line(file, "     printf(\"Hello, %s!\\n\", argv[0]);");
   _append_line(file, "   else");
   _append_line(file, "     printf(\"Hello, World!\\n\");");
   _append_line(file, "   return 0;");
   _append_line(file, "}");

   _assert_line_token_types(file, 1, 1, (Elm_Code_Token_Type[1]){ELM_CODE_TOKEN_TYPE_PREPROCESSOR});
   _assert_line_token_types(file, 2, 1, (Elm_Code_Token_Type[1]){ELM_CODE_TOKEN_TYPE_PREPROCESSOR});
   _assert_line_token_types(file, 3, 1, (Elm_Code_Token_Type[1]){ELM_CODE_TOKEN_TYPE_PREPROCESSOR});

   _assert_line_token_types(file, 5, 1, (Elm_Code_Token_Type[1]){ELM_CODE_TOKEN_TYPE_COMMENT});
   _assert_line_token_types(file, 6, 1, (Elm_Code_Token_Type[1]){ELM_CODE_TOKEN_TYPE_COMMENT});
   _assert_line_token_types(file, 7, 1, (Elm_Code_Token_Type[1]){ELM_CODE_TOKEN_TYPE_COMMENT});
   _assert_line_token_types(file, 8, 8, (Elm_Code_Token_Type[8]){ELM_CODE_TOKEN_TYPE_KEYWORD, ELM_CODE_TOKEN_TYPE_BRACE,
      ELM_CODE_TOKEN_TYPE_KEYWORD, ELM_CODE_TOKEN_TYPE_BRACE, ELM_CODE_TOKEN_TYPE_KEYWORD, ELM_CODE_TOKEN_TYPE_BRACE,
      ELM_CODE_TOKEN_TYPE_BRACE, ELM_CODE_TOKEN_TYPE_BRACE});
   _assert_line_token_types(file, 9, 1, (Elm_Code_Token_Type[1]){ELM_CODE_TOKEN_TYPE_BRACE});
   _assert_line_token_types(file, 10, 4, (Elm_Code_Token_Type[4]){ELM_CODE_TOKEN_TYPE_KEYWORD, ELM_CODE_TOKEN_TYPE_BRACE,
      ELM_CODE_TOKEN_TYPE_STRING, ELM_CODE_TOKEN_TYPE_BRACE});

   _assert_line_token_types(file, 11, 1, (Elm_Code_Token_Type[1]){ELM_CODE_TOKEN_TYPE_COMMENT});
   _assert_line_token_types(file, 12, 5, (Elm_Code_Token_Type[5]){ELM_CODE_TOKEN_TYPE_KEYWORD, ELM_CODE_TOKEN_TYPE_BRACE,
      ELM_CODE_TOKEN_TYPE_BRACE, ELM_CODE_TOKEN_TYPE_NUMBER, ELM_CODE_TOKEN_TYPE_BRACE});
   _assert_line_token_types(file, 13, 8, (Elm_Code_Token_Type[8]){ELM_CODE_TOKEN_TYPE_BRACE, ELM_CODE_TOKEN_TYPE_STRING,
      ELM_CODE_TOKEN_TYPE_BRACE, ELM_CODE_TOKEN_TYPE_BRACE, ELM_CODE_TOKEN_TYPE_NUMBER, ELM_CODE_TOKEN_TYPE_BRACE,
      ELM_CODE_TOKEN_TYPE_BRACE, ELM_CODE_TOKEN_TYPE_BRACE});
   _assert_line_token_types(file, 14, 1, (Elm_Code_Token_Type[1]){ELM_CODE_TOKEN_TYPE_KEYWORD});
   _assert_line_token_types(file, 15, 4, (Elm_Code_Token_Type[4]){ELM_CODE_TOKEN_TYPE_BRACE, ELM_CODE_TOKEN_TYPE_STRING,
      ELM_CODE_TOKEN_TYPE_BRACE, ELM_CODE_TOKEN_TYPE_BRACE});
   _assert_line_token_types(file, 16, 3, (Elm_Code_Token_Type[3]){ELM_CODE_TOKEN_TYPE_KEYWORD,
      ELM_CODE_TOKEN_TYPE_NUMBER, ELM_CODE_TOKEN_TYPE_BRACE});
   _assert_line_token_types(file, 17, 1, (Elm_Code_Token_Type[1]){ELM_CODE_TOKEN_TYPE_BRACE});

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST(elm_code_syntax_scope_change_braces_test)
{
   Elm_Code_File *file;
   Elm_Code_Line *line;
   Elm_Code *code;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   code->file->mime = "text/x-csrc";
   elm_code_parser_standard_add(code, ELM_CODE_PARSER_STANDARD_SYNTAX);
   file = code->file;

   elm_code_file_line_append(file, "#include <stdio.h>", 18, NULL);
   line = elm_code_file_line_get(file, 1);
   ck_assert_int_eq(0, line->scope);

   elm_code_file_line_append(file, "int main() {", 12, NULL);
   line = elm_code_file_line_get(file, 2);
   ck_assert_int_eq(1, line->scope);

   elm_code_file_line_append(file, "}", 1, NULL);
   elm_code_file_line_append(file, "", 0, NULL);
   line = elm_code_file_line_get(file, 4);
   ck_assert_int_eq(0, line->scope);

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

void elm_code_test_syntax(TCase *tc)
{
   tcase_add_test(tc, elm_code_syntax_lookup);
   tcase_add_test(tc, elm_code_syntax_c);
   tcase_add_test(tc, elm_code_syntax_scope_change_braces_test);
}

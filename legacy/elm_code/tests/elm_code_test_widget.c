#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "elm_code_suite.h"

static void _assert_cell_type(Evas_Textgrid_Cell cell, Elm_Code_Token_Type type)
{
   ck_assert(cell.fg == type);
}

START_TEST (elm_code_widget_token_render_simple_test)
{
   Elm_Code_File *file;
   Elm_Code_Line *line;
   Elm_Code *code;
   int length;

   Evas_Textgrid_Cell cells[25];

   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "some \"test content\", 45", 23);
   line = elm_code_file_line_get(file, 1);
   length = strlen(line->content);

   elm_code_file_line_token_add(file, 1, 6+1, 18+1, ELM_CODE_TOKEN_TYPE_COMMENT);
   elm_code_file_line_token_add(file, 1, 22+1, 23+1, ELM_CODE_TOKEN_TYPE_COMMENT);

   elm_code_widget_fill_line_tokens(cells, length, line);
   _assert_cell_type(cells[1], ELM_CODE_TOKEN_TYPE_DEFAULT);
   _assert_cell_type(cells[4], ELM_CODE_TOKEN_TYPE_DEFAULT);
   _assert_cell_type(cells[6], ELM_CODE_TOKEN_TYPE_DEFAULT);
   _assert_cell_type(cells[16], ELM_CODE_TOKEN_TYPE_COMMENT);
   _assert_cell_type(cells[20], ELM_CODE_TOKEN_TYPE_DEFAULT);
   _assert_cell_type(cells[23], ELM_CODE_TOKEN_TYPE_COMMENT);

   elm_code_free(code);
}
END_TEST

void elm_code_test_widget(TCase *tc)
{
   tcase_add_test(tc, elm_code_widget_token_render_simple_test);
}


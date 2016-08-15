#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERNAL_API_ARGESFSDFEFC

#include "elm_suite.h"
#include "Elementary.h"

#include "elm_priv.h"

#undef CRI
#define CRI(...) do { } while (0);

#undef ERR
#define ERR(...) do { } while (0);

#undef WRN
#define WRN(...) do { } while (0);

#undef INF
#define INF(...) do { } while (0);

#undef DBG
#define DBG(...) do { } while (0);

#include "elm_code_widget.c"

static void _assert_cell_type(Evas_Textgrid_Cell cell, Elm_Code_Token_Type type, int id)
{
   ck_assert_msg(cell.fg == type, "Wrong type for cell %d", id);
}

START_TEST (elm_code_widget_token_render_simple_test)
{
   Elm_Code_File *file;
   Elm_Code_Line *line;
   Elm_Code *code;
   Elm_Code_Widget *widget;
   Evas_Object *win;

   int length;

   Evas_Textgrid_Cell cells[25];

   elm_init(1, NULL);
   code = elm_code_create();

   win = elm_win_add(NULL, "code", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);

   file = code->file;
   elm_code_file_line_append(file, "some \"test content\", 45", 23, NULL);
   line = elm_code_file_line_get(file, 1);
   length = line->length;

   elm_code_line_token_add(line, 6, 17, 1, ELM_CODE_TOKEN_TYPE_COMMENT);
   elm_code_line_token_add(line, 21, 22, 1, ELM_CODE_TOKEN_TYPE_COMMENT);

   _elm_code_widget_fill_line_tokens(widget, cells, length+1, line);
   _assert_cell_type(cells[1], ELM_CODE_TOKEN_TYPE_DEFAULT, 1);
   _assert_cell_type(cells[4], ELM_CODE_TOKEN_TYPE_DEFAULT, 4);
   _assert_cell_type(cells[5], ELM_CODE_TOKEN_TYPE_DEFAULT, 5);
   _assert_cell_type(cells[16], ELM_CODE_TOKEN_TYPE_COMMENT, 16);
   _assert_cell_type(cells[20], ELM_CODE_TOKEN_TYPE_DEFAULT, 20);
   _assert_cell_type(cells[22], ELM_CODE_TOKEN_TYPE_COMMENT, 22);

   elm_code_free(code);
   elm_shutdown();
}
END_TEST

START_TEST (elm_code_widget_construct)
{
   Elm_Code *code;
   Elm_Code_Widget *widget;
   Evas_Object *win;

   elm_init(1, NULL);
   code = elm_code_create();

   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);

   ck_assert(!!widget);
   elm_code_free(code);
   elm_shutdown();
}
END_TEST

START_TEST (elm_code_widget_construct_nocode)
{
   Elm_Code_Widget *widget;
   Evas_Object *win;

   elm_init(1, NULL);

   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = efl_add(ELM_CODE_WIDGET_CLASS, win);
   ck_assert(!widget);

   elm_shutdown();
}
END_TEST

void elm_code_test_widget(TCase *tc)
{
   tcase_add_test(tc, elm_code_widget_token_render_simple_test);
   tcase_add_test(tc, elm_code_widget_construct);
   tcase_add_test(tc, elm_code_widget_construct_nocode);
}

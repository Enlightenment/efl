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

#include "elm_code_parse.c"
#include "elm_code_widget_selection.c"
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
   unsigned int gutter;

   Evas_Textgrid_Cell cells[25];

   elm_init(1, NULL);
   code = elm_code_create();

   win = elm_win_add(NULL, "code", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);

   file = code->file;
   elm_code_file_line_append(file, "some \"test content\", 45", 23, NULL);
   line = elm_code_file_line_get(file, 1);
   length = line->length;

   elm_code_line_token_add(line, 5, 18, 1, ELM_CODE_TOKEN_TYPE_COMMENT);
   elm_code_line_token_add(line, 21, 22, 1, ELM_CODE_TOKEN_TYPE_NUMBER);

   _elm_code_widget_fill_line_tokens(widget, cells, length+1, line);
   gutter = elm_obj_code_widget_text_left_gutter_width_get(widget);

   _assert_cell_type(cells[gutter+15], ELM_CODE_TOKEN_TYPE_COMMENT, 16);
   _assert_cell_type(cells[gutter+21], ELM_CODE_TOKEN_TYPE_NUMBER, 22);

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

START_TEST (elm_code_widget_position)
{
   Elm_Code *code;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   Evas_Coord x, y, w, h, x2, y2, w2, h2;

   elm_init(1, NULL);
   code = elm_code_create();

   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);
   evas_object_show(widget);
   evas_object_resize(widget, 100, 100);

   elm_code_file_line_append(code->file, "some content", 12, NULL);
   elm_code_file_line_append(code->file, "more", 4, NULL);

   elm_code_widget_geometry_for_position_get(widget, 1, 1, &x, &y, &w, &h);
   elm_code_widget_geometry_for_position_get(widget, 1, 2, &x2, &y2, &w2, &h2);
   ck_assert(x2 > x);
   ck_assert(y2 == y);
   ck_assert(w2 == w);
   ck_assert(h2 == h);

   elm_code_widget_geometry_for_position_get(widget, 2, 1, &x2, &y2, &w2, &h2);
   ck_assert(x2 == x);
   ck_assert(w2 == w);
   ck_assert(h2 == h);

   elm_code_free(code);
   elm_shutdown();
}
END_TEST

void elm_code_test_widget(TCase *tc)
{
   tcase_add_test(tc, elm_code_widget_token_render_simple_test);
   tcase_add_test(tc, elm_code_widget_construct);
   tcase_add_test(tc, elm_code_widget_construct_nocode);
   tcase_add_test(tc, elm_code_widget_position);
}

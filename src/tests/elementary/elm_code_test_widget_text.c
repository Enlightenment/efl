#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERNAL_API_ARGESFSDFEFC

#include "elm_suite.h"
#include "Elementary.h"

EFL_START_TEST (elm_code_test_widget_text_tab_width)
{
   Elm_Code *code;
   Elm_Code_Widget *widget;
   Evas_Object *win;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);
   elm_code_widget_tabstop_set(widget, 8);

   ck_assert_int_eq(8, elm_code_widget_text_tabwidth_at_column_get(widget, 1));
   ck_assert_int_eq(8, elm_code_widget_text_tabwidth_at_column_get(widget, 9));
   ck_assert_int_eq(6, elm_code_widget_text_tabwidth_at_column_get(widget, 3));

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_test_widget_text_position)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line;
   Elm_Code_Widget *widget;
   Evas_Object *win;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "a\tb", 4, NULL);
   line = elm_code_file_line_get(file, 1);

   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);
   elm_code_widget_tabstop_set(widget, 8);

   ck_assert_int_eq(0, elm_code_widget_line_text_position_for_column_get(widget, line, 1));
   ck_assert_int_eq(1, elm_code_widget_line_text_position_for_column_get(widget, line, 2));

   ck_assert_int_eq(2, elm_code_widget_line_text_position_for_column_get(widget, line, 9));
   ck_assert_int_eq(1, elm_code_widget_line_text_position_for_column_get(widget, line, 7));
   elm_shutdown();
}
EFL_END_TEST

void elm_code_test_widget_text(TCase *tc)
{
   tcase_add_test(tc, elm_code_test_widget_text_tab_width);
   tcase_add_test(tc, elm_code_test_widget_text_position);
}

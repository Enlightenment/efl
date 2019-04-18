#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "elm_suite.h"
#include "Elementary.h"
#include "elm_code_widget_private.h"

EFL_START_TEST(elm_code_test_widget_undo_text_insert)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   unsigned int length;
   const char *content;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "test", 4, NULL);

   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);

   elm_code_widget_text_at_cursor_insert(widget, "a");
   line = elm_code_file_line_get(file, 1);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("atest", content, length);

   elm_code_widget_undo(widget);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("test", content, length);

   elm_code_widget_cursor_position_set(widget, 1, 3);
   elm_code_widget_text_at_cursor_insert(widget, "r");
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("terst", content, length);

   elm_code_widget_undo(widget);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("test", content, length);

   elm_code_widget_cursor_position_set(widget, 1, 4);
   elm_code_widget_text_at_cursor_insert(widget, "\t");
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("tes\tt", content, length);

   elm_code_widget_undo(widget);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("test", content, length);

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST(elm_code_test_widget_undo_text_insert_multiple)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   unsigned int length;
   const char *content;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "test", 4, NULL);

   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);

   elm_code_widget_text_at_cursor_insert(widget, "a");
   elm_code_widget_text_at_cursor_insert(widget, "b");
   line = elm_code_file_line_get(file, 1);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("abtest", content, length);

   elm_code_widget_undo(widget);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("atest", content, length);

   elm_code_widget_undo(widget);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("test", content, length);

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

void elm_code_test_widget_undo(TCase *tc)
{
   tcase_add_test(tc, elm_code_test_widget_undo_text_insert);
   tcase_add_test(tc, elm_code_test_widget_undo_text_insert_multiple);
}

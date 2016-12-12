#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERNAL_API_ARGESFSDFEFC

#include "elm_suite.h"
#include "Elementary.h"
#include "elm_code_widget_private.h"

START_TEST (elm_code_test_widget_undo_text_insert)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   unsigned int length;
   const char *content;

   elm_init(1, NULL);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "test", 4, NULL);

   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);

   _elm_code_widget_text_at_cursor_insert(widget, "a", 1);
   line = elm_code_file_line_get(file, 1);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("atest", content, length);

   elm_code_widget_undo(widget);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("test", content, length);

   elm_code_widget_cursor_position_set(widget, 1, 3);
   _elm_code_widget_text_at_cursor_insert(widget, "r", 1);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("terst", content, length);

   elm_code_widget_undo(widget);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("test", content, length);

   elm_code_widget_cursor_position_set(widget, 1, 4);
   _elm_code_widget_text_at_cursor_insert(widget, "\t", 1);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("tes\tt", content, length);

   elm_code_widget_undo(widget);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("test", content, length);

   elm_code_free(code);
   elm_shutdown();
}
END_TEST

START_TEST (elm_code_test_widget_undo_text_insert_multiple)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   unsigned int length;
   const char *content;

   elm_init(1, NULL);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "test", 4, NULL);

   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);

   _elm_code_widget_text_at_cursor_insert(widget, "a", 1);
   _elm_code_widget_text_at_cursor_insert(widget, "b", 1);
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
END_TEST

START_TEST (elm_code_test_widget_undo_newline)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   unsigned int length;
   const char *content;

   elm_init(1, NULL);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "test", 4, NULL);

   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);

   elm_code_widget_cursor_position_set(widget, 1, 5);
   _elm_code_widget_newline(widget);
   ck_assert_int_eq(2, elm_code_file_lines_get(file));
   line = elm_code_file_line_get(file, 1);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("test", content, 1);

   elm_code_widget_undo(widget);

   ck_assert_int_eq(1, elm_code_file_lines_get(file));
   line = elm_code_file_line_get(file, 1);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("test", content, 4);

   elm_code_free(code);
   elm_shutdown();
}
END_TEST

START_TEST (elm_code_test_widget_undo_delete)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   unsigned int length;
   const char *content;

   elm_init(1, NULL);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "test", 4, NULL);

   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);

   elm_code_widget_cursor_position_set(widget, 1, 4);
   _elm_code_widget_backspace(widget);

   line = elm_code_file_line_get(file, 1);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("tet", content, length);

   elm_code_widget_undo(widget);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("test", content, length);

   elm_code_widget_cursor_position_set(widget, 1, 2);
   _elm_code_widget_delete(widget);

   line = elm_code_file_line_get(file, 1);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("tst", content, length);

   elm_code_widget_undo(widget);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("test", content, length);

   elm_code_widget_cursor_position_set(widget, 1, 4);
   _elm_code_widget_text_at_cursor_insert(widget, "\t", 1);
   _elm_code_widget_backspace(widget);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("test", content, length);
   elm_code_widget_undo(widget);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("tes\tt", content, length);

   elm_code_widget_cursor_position_set(widget, 1, 4);
   _elm_code_widget_delete(widget);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("test", content, length);
   elm_code_widget_undo(widget);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("tes\tt", content, length);

   elm_code_free(code);
   elm_shutdown();
}
END_TEST

START_TEST (elm_code_test_widget_undo_delete_multiple)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   unsigned int length;
   const char *content;

   elm_init(1, NULL);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "test", 4, NULL);

   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);

   elm_code_widget_cursor_position_set(widget, 1, 4);
   _elm_code_widget_backspace(widget);
   _elm_code_widget_backspace(widget);

   line = elm_code_file_line_get(file, 1);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("tt", content, length);

   elm_code_widget_undo(widget);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("tet", content, length);

   elm_code_widget_undo(widget);
   content = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("test", content, length);

   elm_code_free(code);
   elm_shutdown();
}
END_TEST

void elm_code_test_widget_undo(TCase *tc)
{
   tcase_add_test(tc, elm_code_test_widget_undo_text_insert);
   tcase_add_test(tc, elm_code_test_widget_undo_text_insert_multiple);
   tcase_add_test(tc, elm_code_test_widget_undo_newline);
   tcase_add_test(tc, elm_code_test_widget_undo_delete);
   tcase_add_test(tc, elm_code_test_widget_undo_delete_multiple);
}

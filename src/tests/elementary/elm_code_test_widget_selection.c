#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "elm_suite.h"
#include "Elementary.h"
#include "elm_code_widget_private.h"
#include "elm_code_widget_selection.h"

EFL_START_TEST (elm_code_test_widget_selection_set)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Widget *widget;
   Evas_Object *win;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "test", 4, NULL);

   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);

   elm_code_widget_selection_start(widget, 1, 2);
   elm_code_widget_selection_end(widget, 1, 3);
   elm_code_widget_selection_clear(widget);

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_test_widget_selection_normalized_get)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Widget *widget;
   Elm_Code_Widget_Selection_Data *selection;
   Evas_Object *win;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "test", 4, NULL);

   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);

   elm_code_widget_selection_start(widget, 1, 3);
   elm_code_widget_selection_end(widget, 1, 2);
   selection = elm_code_widget_selection_normalized_get(widget);

   ck_assert_int_eq(selection->start_col, 2);
   ck_assert_int_eq(selection->end_col, 3);
   elm_code_widget_selection_clear(widget);
   free(selection);

   elm_code_file_line_append(file, "another", 7, NULL);
   elm_code_widget_selection_start(widget, 2, 2);
   elm_code_widget_selection_end(widget, 1, 3);
   selection = elm_code_widget_selection_normalized_get(widget);

   ck_assert_int_eq(selection->start_line, 1);
   ck_assert_int_eq(selection->start_col, 3);
   ck_assert_int_eq(selection->end_line, 2);
   ck_assert_int_eq(selection->end_col, 2);
   elm_code_widget_selection_clear(widget);
   free(selection);

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_test_widget_selection_text_get)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   char *selection;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "test", 4, NULL);

   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);

   ck_assert_str_eq("", elm_code_widget_selection_text_get(widget));

   elm_code_widget_selection_start(widget, 1, 2);
   elm_code_widget_selection_end(widget, 1, 3);

   selection = elm_code_widget_selection_text_get(widget);
   ck_assert_str_eq("es", selection);
   free(selection);

   elm_code_widget_selection_clear(widget);
   ck_assert_str_eq("", elm_code_widget_selection_text_get(widget));

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_test_widget_selection_reverse_text_get)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   char *selection;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "test", 4, NULL);

   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);

   ck_assert_str_eq("", elm_code_widget_selection_text_get(widget));

   elm_code_widget_selection_start(widget, 1, 3);
   elm_code_widget_selection_end(widget, 1, 2);

   selection = elm_code_widget_selection_text_get(widget);
   ck_assert_str_eq("es", selection);
   free(selection);

   elm_code_widget_selection_clear(widget);
   ck_assert_str_eq("", elm_code_widget_selection_text_get(widget));

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_test_widget_selection_text_get_twoline)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   char *selection;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "test", 4, NULL);
   elm_code_file_line_append(file, "test", 4, NULL);

   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);

   elm_code_widget_selection_start(widget, 1, 3);
   elm_code_widget_selection_end(widget, 2, 2);

   selection = elm_code_widget_selection_text_get(widget);
   ck_assert_str_eq("st\nte", selection);
   free(selection);

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_test_widget_selection_reverse_text_get_twoline)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   char *selection;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "test", 4, NULL);
   elm_code_file_line_append(file, "test", 4, NULL);

   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);

   elm_code_widget_selection_start(widget, 2, 2);
   elm_code_widget_selection_end(widget, 1, 3);

   selection = elm_code_widget_selection_text_get(widget);
   ck_assert_str_eq("st\nte", selection);
   free(selection);

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_test_widget_selection_text_get_multiline)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   char *selection;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "test", 4, NULL);
   elm_code_file_line_append(file, "test", 4, NULL);
   elm_code_file_line_append(file, "test", 4, NULL);

   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);

   elm_code_widget_selection_start(widget, 1, 3);
   elm_code_widget_selection_end(widget, 3, 2);

   selection = elm_code_widget_selection_text_get(widget);
   ck_assert_str_eq("st\ntest\nte", selection);
   free(selection);

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_test_widget_selection_reverse_text_get_multiline)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   char *selection;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "test", 4, NULL);
   elm_code_file_line_append(file, "test", 4, NULL);
   elm_code_file_line_append(file, "test", 4, NULL);

   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);

   elm_code_widget_selection_start(widget, 3, 2);
   elm_code_widget_selection_end(widget, 1, 3);

   selection = elm_code_widget_selection_text_get(widget);
   ck_assert_str_eq("st\ntest\nte", selection);
   free(selection);

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_test_widget_selection_delete)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   const char *text;
   unsigned int length;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "text", 4, NULL);

   win = win_add(NULL, "code", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);
   line = elm_code_file_line_get(file, 1);
   text = elm_code_line_text_get(line, &length);
   ck_assert_int_eq(4, length);
   ck_assert_strn_eq("text", text, length);

   elm_code_widget_selection_start(widget, 1, 2);
   elm_code_widget_selection_end(widget, 1, 3);
   elm_code_widget_selection_delete(widget);

   line = elm_code_file_line_get(file, 1);
   text = elm_code_line_text_get(line, &length);
   ck_assert_int_eq(2, length);
   ck_assert_strn_eq("tt", text, length);

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_test_widget_selection_reverse_delete)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   const char *text;
   unsigned int length;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "text", 4, NULL);

   win = win_add(NULL, "code", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);
   line = elm_code_file_line_get(file, 1);
   text = elm_code_line_text_get(line, &length);
   ck_assert_int_eq(4, length);
   ck_assert_strn_eq("text", text, length);

   elm_code_widget_selection_start(widget, 1, 3);
   elm_code_widget_selection_end(widget, 1, 2);
   elm_code_widget_selection_delete(widget);

   line = elm_code_file_line_get(file, 1);
   text = elm_code_line_text_get(line, &length);
   ck_assert_int_eq(2, length);
   ck_assert_strn_eq("tt", text, length);

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_test_widget_selection_delete_twoline)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   const char *text;
   unsigned int length;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "text", 4, NULL);
   elm_code_file_line_append(file, "TEXT", 4, NULL);
   elm_code_file_line_append(file, "remove", 6, NULL);

   win = win_add(NULL, "code", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);
   line = elm_code_file_line_get(file, 1);
   text = elm_code_line_text_get(line, NULL);
   ck_assert_str_eq("text", text);
   ck_assert_int_eq(3, elm_code_file_lines_get(file));

   elm_code_widget_selection_start(widget, 1, 3);
   elm_code_widget_selection_end(widget, 2, 2);
   elm_code_widget_selection_delete(widget);

   line = elm_code_file_line_get(file, 1);
   text = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("teXT", text, length);
   ck_assert_int_eq(2, elm_code_file_lines_get(file));

   elm_code_widget_selection_start(widget, 1, 5);
   elm_code_widget_selection_end(widget, 2, 1);
   elm_code_widget_selection_delete(widget);

   line = elm_code_file_line_get(file, 1);
   text = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("teXTemove", text, length);
   ck_assert_int_eq(1, elm_code_file_lines_get(file));

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_test_widget_selection_reverse_delete_twoline)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   const char *text;
   unsigned int length;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "text", 4, NULL);
   elm_code_file_line_append(file, "TEXT", 4, NULL);
   elm_code_file_line_append(file, "remove", 6, NULL);

   win = win_add(NULL, "code", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);
   line = elm_code_file_line_get(file, 1);
   text = elm_code_line_text_get(line, NULL);
   ck_assert_str_eq("text", text);
   ck_assert_int_eq(3, elm_code_file_lines_get(file));

   elm_code_widget_selection_start(widget, 2, 2);
   elm_code_widget_selection_end(widget, 1, 3);
   elm_code_widget_selection_delete(widget);

   line = elm_code_file_line_get(file, 1);
   text = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("teXT", text, length);
   ck_assert_int_eq(2, elm_code_file_lines_get(file));

   elm_code_widget_selection_start(widget, 2, 1);
   elm_code_widget_selection_end(widget, 1, 5);
   elm_code_widget_selection_delete(widget);

   line = elm_code_file_line_get(file, 1);
   text = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("teXTemove", text, length);
   ck_assert_int_eq(1, elm_code_file_lines_get(file));

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_test_widget_selection_delete_multiline)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   const char *text;
   unsigned int length;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "text", 4, NULL);
   elm_code_file_line_append(file, "remove", 6, NULL);
   elm_code_file_line_append(file, "TEXT", 4, NULL);
   elm_code_file_line_append(file, "delete", 6, NULL);
   elm_code_file_line_append(file, "REMOVE", 6, NULL);

   win = win_add(NULL, "code", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);
   line = elm_code_file_line_get(file, 1);
   text = elm_code_line_text_get(line, NULL);
   ck_assert_str_eq("text", text);
   ck_assert_int_eq(5, elm_code_file_lines_get(file));

   elm_code_widget_selection_start(widget, 1, 3);
   elm_code_widget_selection_end(widget, 3, 2);
   elm_code_widget_selection_delete(widget);

   line = elm_code_file_line_get(file, 1);
   text = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("teXT", text, length);
   ck_assert_int_eq(3, elm_code_file_lines_get(file));

   elm_code_widget_selection_start(widget, 1, 5);
   elm_code_widget_selection_end(widget, 3, 1);
   elm_code_widget_selection_delete(widget);

   line = elm_code_file_line_get(file, 1);
   text = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("teXTEMOVE", text, length);
   ck_assert_int_eq(1, elm_code_file_lines_get(file));

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_test_widget_selection_reverse_delete_multiline)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Line *line;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   const char *text;
   unsigned int length;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "text", 4, NULL);
   elm_code_file_line_append(file, "remove", 6, NULL);
   elm_code_file_line_append(file, "TEXT", 4, NULL);
   elm_code_file_line_append(file, "delete", 6, NULL);
   elm_code_file_line_append(file, "REMOVE", 6, NULL);

   win = win_add(NULL, "code", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);
   line = elm_code_file_line_get(file, 1);
   text = elm_code_line_text_get(line, NULL);
   ck_assert_str_eq("text", text);
   ck_assert_int_eq(5, elm_code_file_lines_get(file));

   elm_code_widget_selection_start(widget, 3, 2);
   elm_code_widget_selection_end(widget, 1, 3);
   elm_code_widget_selection_delete(widget);

   line = elm_code_file_line_get(file, 1);
   text = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("teXT", text, length);
   ck_assert_int_eq(3, elm_code_file_lines_get(file));

   elm_code_widget_selection_start(widget, 3, 1);
   elm_code_widget_selection_end(widget, 1, 5);
   elm_code_widget_selection_delete(widget);

   line = elm_code_file_line_get(file, 1);
   text = elm_code_line_text_get(line, &length);
   ck_assert_strn_eq("teXTEMOVE", text, length);
   ck_assert_int_eq(1, elm_code_file_lines_get(file));

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_test_widget_selection_select_line)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   char *selection;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "line selection", 14, NULL);
   elm_code_file_line_append(file, "line2", 5, NULL);
   elm_code_file_line_append(file, "\ttab", 4, NULL);

   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);

   elm_code_widget_selection_select_line(widget, 1);
   selection = elm_code_widget_selection_text_get(widget);
   ck_assert_str_eq("line selection", selection);
   free(selection);

   elm_code_widget_selection_select_line(widget, 2);
   selection = elm_code_widget_selection_text_get(widget);
   ck_assert_str_eq("line2", selection);
   free(selection);

   elm_code_widget_selection_select_line(widget, 3);
   selection = elm_code_widget_selection_text_get(widget);
   ck_assert_str_eq("\ttab", selection);
   free(selection);

   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_test_widget_selection_select_word)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   char *selection;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "word selection test", 19, NULL);
   elm_code_file_line_append(file, "more stuff\tto test", 18, NULL);
   elm_code_file_line_append(file, "word \"symbols\" test", 19, NULL);

   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);

   elm_code_widget_selection_select_word(widget, 1, 3);
   selection = elm_code_widget_selection_text_get(widget);
   ck_assert_str_eq("word", selection);
   free(selection);

   elm_code_widget_selection_select_word(widget, 1, 16);
   selection = elm_code_widget_selection_text_get(widget);
   ck_assert_str_eq("test", selection);
   free(selection);

   elm_code_widget_selection_select_word(widget, 2, 9);
   selection = elm_code_widget_selection_text_get(widget);
   ck_assert_str_eq("stuff", selection);
   free(selection);

   elm_code_widget_selection_select_word(widget, 3, 9);
   selection = elm_code_widget_selection_text_get(widget);
   ck_assert_str_eq("symbols", selection);
   free(selection);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_test_widget_selection_select_word_punctuation)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   char *selection;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "comma, stop. question? mark!", 38, NULL);

   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);

   elm_code_widget_selection_select_word(widget, 1, 3);
   selection = elm_code_widget_selection_text_get(widget);
   ck_assert_str_eq("comma", selection);
   free(selection);

   elm_code_widget_selection_select_word(widget, 1, 10);
   selection = elm_code_widget_selection_text_get(widget);
   ck_assert_str_eq("stop", selection);
   free(selection);

   elm_code_widget_selection_select_word(widget, 1, 20);
   selection = elm_code_widget_selection_text_get(widget);
   ck_assert_str_eq("question", selection);
   free(selection);

   elm_code_widget_selection_select_word(widget, 1, 25);
   selection = elm_code_widget_selection_text_get(widget);
   ck_assert_str_eq("mark", selection);
   free(selection);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_test_widget_selection_select_word_symbols)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   char *selection;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "colon: [array] (brackets) {braces}", 38, NULL);

   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);

   elm_code_widget_selection_select_word(widget, 1, 3);
   selection = elm_code_widget_selection_text_get(widget);
   ck_assert_str_eq("colon", selection);
   free(selection);

   elm_code_widget_selection_select_word(widget, 1, 10);
   selection = elm_code_widget_selection_text_get(widget);
   ck_assert_str_eq("array", selection);
   free(selection);

   elm_code_widget_selection_select_word(widget, 1, 20);
   selection = elm_code_widget_selection_text_get(widget);
   ck_assert_str_eq("brackets", selection);
   free(selection);

   elm_code_widget_selection_select_word(widget, 1, 30);
   selection = elm_code_widget_selection_text_get(widget);
   ck_assert_str_eq("braces", selection);
   free(selection);
   elm_shutdown();
}
EFL_END_TEST

void elm_code_test_widget_selection(TCase *tc)
{
   tcase_add_test(tc, elm_code_test_widget_selection_set);
   tcase_add_test(tc, elm_code_test_widget_selection_normalized_get);
   tcase_add_test(tc, elm_code_test_widget_selection_text_get);
   tcase_add_test(tc, elm_code_test_widget_selection_reverse_text_get);
   tcase_add_test(tc, elm_code_test_widget_selection_text_get_twoline);
   tcase_add_test(tc, elm_code_test_widget_selection_reverse_text_get_twoline);
   tcase_add_test(tc, elm_code_test_widget_selection_text_get_multiline);
   tcase_add_test(tc, elm_code_test_widget_selection_reverse_text_get_multiline);
   tcase_add_test(tc, elm_code_test_widget_selection_delete);
   tcase_add_test(tc, elm_code_test_widget_selection_reverse_delete);
   tcase_add_test(tc, elm_code_test_widget_selection_delete_twoline);
   tcase_add_test(tc, elm_code_test_widget_selection_reverse_delete_twoline);
   tcase_add_test(tc, elm_code_test_widget_selection_delete_multiline);
   tcase_add_test(tc, elm_code_test_widget_selection_reverse_delete_multiline);
   tcase_add_test(tc, elm_code_test_widget_selection_select_line);
   tcase_add_test(tc, elm_code_test_widget_selection_select_word);
   tcase_add_test(tc, elm_code_test_widget_selection_select_word_punctuation);
   tcase_add_test(tc, elm_code_test_widget_selection_select_word_symbols);
}

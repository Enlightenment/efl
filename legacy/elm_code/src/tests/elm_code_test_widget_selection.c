#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "elm_code_suite.h"

#include "elm_code_widget_selection.h"

START_TEST (elm_code_test_widget_selection_set)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Widget *widget;
   Evas_Object *win;

   elm_init(1, NULL);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "test", 4, NULL);

   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = eo_add(ELM_CODE_WIDGET_CLASS, win,
                   elm_code_widget_code_set(code));

   elm_code_widget_selection_start(widget, 1, 2);
   elm_code_widget_selection_end(widget, 1, 3);
   elm_code_widget_selection_clear(widget);

   elm_code_free(code);
   elm_shutdown();
}
END_TEST

START_TEST (elm_code_test_widget_selection_text_get)
{
   Elm_Code *code;
   Elm_Code_File *file;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   char *selection;

   elm_init(1, NULL);
   code = elm_code_create();
   file = elm_code_file_new(code);
   elm_code_file_line_append(file, "test", 4, NULL);

   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = eo_add(ELM_CODE_WIDGET_CLASS, win,
                   elm_code_widget_code_set(code));

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
END_TEST

void elm_code_test_widget_selection(TCase *tc)
{
   tcase_add_test(tc, elm_code_test_widget_selection_set);
   tcase_add_test(tc, elm_code_test_widget_selection_text_get);
}


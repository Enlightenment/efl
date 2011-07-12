//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` label_example_01.c -o label_example_01

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

EAPI int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg, *label, *label2, *label3, *label4, *label5;

   win = elm_win_add(NULL, "label", ELM_WIN_BASIC);
   elm_win_title_set(win, "Label");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   label = elm_label_add(win);
   elm_object_text_set(label, "Some long text for our label, that is long but "
                              "not too long.");
   elm_label_slide_duration_set(label, 3);
   elm_label_slide_set(label, EINA_TRUE);
   elm_object_style_set(label, "slide_bounce");
   evas_object_resize(label, 200, 15);
   evas_object_show(label);

   label2 = elm_label_add(win);
   elm_object_text_set(label2, "This is the text for our second label, which is"
                               " much longer than the previous one, maybe even "
                               "too long, but maybe not.");
   elm_label_ellipsis_set(label2, EINA_TRUE);
   evas_object_resize(label2, 200, 15);
   evas_object_move(label2, 0, 15);
   evas_object_show(label2);

   label3 = elm_label_add(win);
   elm_object_text_set(label3, "Some more long text much as before, long but "
                               "not too long.");
   elm_label_line_wrap_set(label3, ELM_WRAP_CHAR);
   elm_label_ellipsis_set(label3, EINA_TRUE);
   evas_object_resize(label3, 200, 15);
   evas_object_move(label3, 0, 30);
   evas_object_show(label3);

   label4 = elm_label_add(win);
   elm_object_text_set(label4, "And for this label we choose a different text, "
                               "for no reason other than that we can.");
   elm_label_line_wrap_set(label4, ELM_WRAP_CHAR);
   evas_object_resize(label4, 200, 30);
   evas_object_move(label4, 0, 45);
   evas_object_show(label4);

   label5 = elm_label_add(win);
   elm_object_text_set(label5, "And for this label we choose a different text, "
                               "for no reason other than that we can.");
   elm_label_line_wrap_set(label5, ELM_WRAP_WORD);
   evas_object_resize(label5, 200, 25);
   evas_object_move(label5, 0, 75);
   evas_object_show(label5);

   evas_object_resize(win, 200, 105);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()

//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` hover_example_01.c -o hover_example_01

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

static void
_show_hover(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_show(data);
}

static void
_hide_hover(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_hide(data);
}

EAPI int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg, *bt, *bt2, *rect, *rect2, *rect3, *hover;

   win = elm_win_add(NULL, "hover", ELM_WIN_BASIC);
   elm_win_title_set(win, "Hover");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_min_set(rect, 25, 25);
   evas_object_color_set(rect, 255, 0, 0, 255);
   evas_object_show(rect);

   rect2 = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_min_set(rect2, 25, 25);
   evas_object_color_set(rect2, 0, 255, 0, 255);
   evas_object_show(rect2);

   rect3 = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_min_set(rect3, 25, 25);
   evas_object_color_set(rect3, 0, 0, 255, 255);
   evas_object_show(rect3);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Show hover");
   evas_object_move(bt, 60, 90);
   evas_object_resize(bt, 80, 20);
   evas_object_show(bt);

   bt2 = elm_button_add(win);
   elm_object_text_set(bt2, "Hide hover");
   evas_object_show(bt2);

   hover = elm_hover_add(win);
   elm_hover_parent_set(hover, win);
   elm_hover_target_set(hover, bt);

   elm_object_style_set(hover, "popout");

   elm_hover_content_set(hover, "left", rect);
   elm_hover_content_set(hover, "top", rect2);
   elm_hover_content_set(hover, "right", rect3);
   elm_hover_content_set(hover, "middle", bt2);

   evas_object_smart_callback_add(bt, "clicked", _show_hover, hover);
   evas_object_smart_callback_add(bt2, "clicked", _hide_hover, hover);

   evas_object_resize(win, 200, 200);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()

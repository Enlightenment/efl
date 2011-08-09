//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` separator_example_01.c -o separator_example_01

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

EAPI int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg, *bx, *rect, *separator;
   char buf[256];

   win = elm_win_add(NULL, "separator", ELM_WIN_BASIC);
   elm_win_title_set(win, "Separator");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rect, 0, 255, 0, 255);
   evas_object_size_hint_min_set(rect, 90, 200);
   evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(rect, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(rect);
   elm_box_pack_end(bx, rect);

   separator = elm_separator_add(win);
   elm_separator_horizontal_set(separator, EINA_TRUE);
   evas_object_show(separator);
   elm_box_pack_end(bx, separator);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rect, 0, 0, 255, 255);
   evas_object_size_hint_min_set(rect, 90, 200);
   evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(rect, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(rect);
   elm_box_pack_end(bx, rect);

   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()

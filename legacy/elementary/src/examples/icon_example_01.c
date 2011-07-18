//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` image_example_01.c -o image_example_01

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
# define PACKAGE_DATA_DIR "."
#endif

int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *icon;
   const char *path, *group, *name;

   win = elm_win_add(NULL, "icon", ELM_WIN_BASIC);
   elm_win_title_set(win, "Icon");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_bg_color_set(bg, 255,255 ,255);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   icon = elm_icon_add(win);
   elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_THEME_FDO);
   elm_icon_standard_set(icon, "home");

   path = NULL;
   group = NULL;
   name = NULL;
   elm_icon_file_get(icon, &path, &group);
   name = elm_icon_standard_get(icon);
   printf("path = %s, group = %s, name = %s\n", path, group, name);

   elm_icon_no_scale_set(icon, EINA_TRUE);
   elm_icon_scale_set(icon, EINA_FALSE, EINA_TRUE);
   elm_icon_smooth_set(icon, EINA_FALSE);
   elm_icon_fill_outside_set(icon, EINA_TRUE);

   evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, icon);
   evas_object_show(icon);

   evas_object_size_hint_min_set(bg, 160, 160);
   evas_object_size_hint_max_set(bg, 640, 640);
   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   elm_run();

   return 0;
}

ELM_MAIN()

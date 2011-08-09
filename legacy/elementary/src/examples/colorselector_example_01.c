//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` colorselector_example_01.c -o colorselector_example_01

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

static void _change_color(void *data, Evas_Object *obj, void *event_info);

EAPI int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg, *cs, *frame, *rect;
   char buf[256];

   win = elm_win_add(NULL, "color selector", ELM_WIN_BASIC);
   elm_win_title_set(win, "Color selector");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_resize(rect, 50, 50);
   evas_object_move(rect, 125, 325);
   evas_object_color_set(rect, 0, 0, 255, 255);
   evas_object_show(rect);

   cs = elm_colorselector_add(win);
   elm_colorselector_color_set(cs, 0, 0, 255, 255);
   evas_object_resize(cs, 300, 300);
   evas_object_show(cs);
   evas_object_smart_callback_add(cs, "changed", _change_color, rect);

   evas_object_resize(win, 300, 400);
   evas_object_show(win);

   elm_run();
   evas_object_del(rect);

   return 0;
}
ELM_MAIN()

static void
_change_color(void *data, Evas_Object *obj, void *event_info)
{
   int r, g, b, a;
   elm_colorselector_color_get(obj, &r, &g, &b, &a);
   evas_object_color_set(data, r, g, b, a);
}

//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` pager_example_01.c -o pager_example_01

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

static void _promote(void *data, Evas_Object *obj, void *event_info);
static Evas_Object *pager;

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg, *rect, *bt;

   win = elm_win_add(NULL, "pager", ELM_WIN_BASIC);
   elm_win_title_set(win, "Pager");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   pager = elm_pager_add(win);
   elm_object_style_set(pager, "fade");
   evas_object_resize(pager, 180, 160);
   evas_object_move(pager, 10, 10);
   evas_object_show(pager);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rect, 0, 0, 255, 255);
   evas_object_show(rect);
   elm_pager_content_push(pager, rect);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "blue");
   evas_object_resize(bt, 50, 20);
   evas_object_move(bt, 10, 170);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", _promote, rect);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rect, 0, 255, 0, 255);
   evas_object_show(rect);
   elm_pager_content_push(pager, rect);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "green");
   evas_object_resize(bt, 60, 20);
   evas_object_move(bt, 70, 170);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", _promote, rect);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rect, 255, 0, 0, 255);
   evas_object_show(rect);
   elm_pager_content_push(pager, rect);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "red");
   evas_object_resize(bt, 50, 20);
   evas_object_move(bt, 140, 170);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", _promote, rect);

   evas_object_resize(win, 200, 200);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()

static void
_promote(void *data, Evas_Object *obj, void *event_info)
{
   if(elm_pager_content_top_get(pager) != data)
      elm_pager_content_promote(pager, data);
}

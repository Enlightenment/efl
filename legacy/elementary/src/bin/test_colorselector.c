#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

static void
_colorselector_clicked_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *re = data;
   int r, g, b, a;

   elm_colorselector_color_get(obj, &r, &g, &b, &a);
   printf("Current Color [r=%d g=%d b=%d a=%d]\n",r, g, b, a);

   /* Fix Alpha pre multiplication by edje */
   r = (r * a) / 255;
   g = (g * a) / 255;
   b = (b * a) / 255;

   evas_object_color_set(re, r, g, b, a);
}

static void
_colorpalette_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   int r = 0, g = 0, b = 0 ,a = 0;
   Elm_Object_Item *color_it = (Elm_Object_Item *) event_info;
   elm_colorselector_palette_item_color_get(color_it, &r, &g, &b, &a);
   evas_object_color_set((Evas_Object *) data, r, g, b , a);
}

static void
_colorpalette_longpressed_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   int r = 0,g = 0,b = 0 ,a = 0;
   Elm_Object_Item *color_it = (Elm_Object_Item *) event_info;
   elm_colorselector_palette_item_color_get(color_it, &r, &g, &b, &a);
   printf("\ncolor = %d-%d-%d-%d\n", r, g, b, a);
}

void
test_colorselector(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *cs, *fr, *re;
   int r, g, b, a;

   win = elm_win_add(NULL, "colorselector", ELM_WIN_BASIC);
   elm_win_title_set(win, "Color Selector");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_color_set(bg, 255, 255, 255, 255);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, 1.0, 0.5);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(fr, "Color View");
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   re = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_show(re);
   elm_object_content_set(fr, re);

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, 1.0, 0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, 0);
   elm_object_text_set(fr, "Color Selector");
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   cs = elm_colorselector_add(win);
   elm_colorselector_palette_color_add(cs, 255, 90, 18, 255);
   elm_colorselector_palette_color_add(cs, 255, 213, 0, 255);
   elm_colorselector_palette_color_add(cs, 146, 255, 11, 255);
   elm_colorselector_palette_color_add(cs, 9, 186, 10, 255);
   elm_colorselector_palette_color_add(cs, 86, 201, 242, 255);
   elm_colorselector_palette_color_add(cs, 18, 83, 128, 255);
   elm_colorselector_palette_color_add(cs, 140, 53, 238, 255);
   elm_colorselector_palette_color_add(cs, 255, 145, 145, 255);
   elm_colorselector_palette_color_add(cs, 255, 59, 119, 255);
   elm_colorselector_palette_color_add(cs, 133, 100, 69, 255);
   elm_colorselector_palette_color_add(cs, 255, 255, 119, 255);
   elm_colorselector_palette_color_add(cs, 133, 100, 255, 255);

   a = 180;
   r = 255;
   g = 160;
   b = 132;

   elm_colorselector_color_set(cs, r, g, b, a);
   evas_object_size_hint_weight_set(cs, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(cs, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(fr, cs);
   evas_object_show(cs);
   evas_object_smart_callback_add(cs, "changed", _colorselector_clicked_cb, re);
   evas_object_smart_callback_add(cs, "color,item,selected", _colorpalette_clicked_cb, re);
   evas_object_smart_callback_add(cs, "color,item,longpressed", _colorpalette_longpressed_cb, re);

   elm_colorselector_color_get(cs, &r, &g, &b, &a);
   /* Fix Alpha pre multiplication by edje */
   r = (r * a) / 255;
   g = (g * a) / 255;
   b = (b * a) / 255;
   evas_object_color_set(re, r, g, b, a);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
#endif

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
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

void
test_colorselector(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *cp, *fr, *ly, *re;
   char buf[PATH_MAX];
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

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/colorpreview.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "main");
   evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_content_set(fr, ly);
   evas_object_show(ly);

   re = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_show(re);
   elm_object_content_part_set(ly, "ColorPreview", re);

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, 1.0, 0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, 0);
   elm_object_text_set(fr, "Color Selector");
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   cp = elm_colorselector_add(win);

   a = 180;
   r = 255;
   g = 160;
   b = 132;

   elm_colorselector_color_set(cp, r, g, b, a);
   evas_object_size_hint_weight_set(cp, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(cp, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(fr, cp);
   evas_object_show(cp);
   evas_object_smart_callback_add(cp, "changed", _colorselector_clicked_cb, re);

   elm_colorselector_color_get(cp, &r, &g, &b, &a);
   /* Fix Alpha pre multiplication by edje */
   r = (r * a) / 255;
   g = (g * a) / 255;
   b = (b * a) / 255;
   evas_object_color_set(re, r, g, b, a);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);

}
#endif

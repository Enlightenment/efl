//Compile with:
//gcc -g colorselector_example_01.c -o colorselector_example_01 `pkg-config --cflags --libs elementary`

#include <Elementary.h>

static void _change_color(void *data, Evas_Object *obj, void *event_info);
static void _colorpalette_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _colorpalette_longpressed_cb(void *data, Evas_Object *obj, void *event_info);

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg, *cs, *rect, *bx, *fr;

   win = elm_win_add(NULL, "color selector", ELM_WIN_BASIC);
   elm_win_title_set(win, "Color selector");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, 1.0, 0.5);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(fr, "Color View");
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   elm_object_content_set(fr, rect);
   evas_object_color_set(rect, 255, 90, 18, 255);
   evas_object_show(rect);

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, 1.0, 0.5);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(fr, "Color Selector");
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   cs = elm_colorselector_add(win);
   elm_colorselector_palette_name_set(cs, "painting");
   evas_object_size_hint_weight_set(cs, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(cs, EVAS_HINT_FILL, 0.0);
   elm_colorselector_color_set(cs, 255, 90, 18, 255);
   evas_object_show(cs);
   evas_object_smart_callback_add(cs, "changed", _change_color, rect);
   evas_object_smart_callback_add(cs, "color,item,selected", _colorpalette_clicked_cb, rect);
   evas_object_smart_callback_add(cs, "color,item,longpressed", _colorpalette_longpressed_cb, rect);
   elm_object_content_set(fr, cs);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);

   elm_run();
   elm_shutdown();

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

static void
_colorpalette_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
   int r = 0, g = 0, b = 0 ,a = 0;
   Elm_Object_Item *color_it = (Elm_Object_Item *) event_info;
   elm_colorselector_palette_item_color_get(color_it, &r, &g, &b, &a);
   evas_object_color_set(data, r, g, b, a);
}

static void
_colorpalette_longpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
   int r = 0,g = 0,b = 0 ,a = 0;
   Elm_Object_Item *color_it = (Elm_Object_Item *) event_info;
   elm_colorselector_palette_item_color_get(color_it, &r, &g, &b, &a);
   printf("\ncolor = %d-%d-%d-%d\n", r, g, b, a);
}

/**
 * Simple Elementary's <b>map widget</b> example, illustrating its
 * creation.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g `pkg-config --cflags --libs elementary` map_example_01.c -o map_example_01
 * @endverbatim
 */

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

static void
_bt_zoom_in(void *data, Evas_Object *obj __UNUSED__, void *ev __UNUSED__)
{
   Evas_Object *map = data;
   int zoom;

   elm_map_zoom_mode_set(map, ELM_MAP_ZOOM_MODE_MANUAL);
   zoom = elm_map_zoom_get(map);
   elm_map_zoom_set(map, zoom + 1);
}

static void
_bt_zoom_out(void *data, Evas_Object *obj __UNUSED__, void *ev __UNUSED__)
{
   Evas_Object *map = data;
   int zoom;

   elm_map_zoom_mode_set(map, ELM_MAP_ZOOM_MODE_MANUAL);
   zoom = elm_map_zoom_get(map);
   elm_map_zoom_set(map, zoom - 1);
}

static void
_bt_zoom_fit(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *map = data;
   elm_map_zoom_mode_set(map, ELM_MAP_ZOOM_MODE_AUTO_FIT);
}

static void
_bt_zoom_fill(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *map = data;
   elm_map_zoom_mode_set(map, ELM_MAP_ZOOM_MODE_AUTO_FILL);
}

static void
_on_done(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_exit();
}

/* FIXME: it shouldn't be required. For unknown reason map won't call
 * pan_calculate until shot delay time, but then it will take a screenshot
 * when the map isn't loaded yet (actually it won't be downloaded, because
 * after the SS it will kill the example). */
static Eina_Bool
_nasty_hack(void *data)
{
   Evas_Object *o = data;
   Evas *e = evas_object_evas_get(o);
   evas_smart_objects_calculate(e);
   return ECORE_CALLBACK_CANCEL;
}

EAPI int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *map, *box, *bt;

   win = elm_win_add(NULL, "map", ELM_WIN_BASIC);
   elm_win_title_set(win, "Map Creation Example");
   evas_object_smart_callback_add(win, "delete,request", _on_done, NULL);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   map = elm_map_add(win);
   elm_win_resize_object_add(win, map);
   evas_object_size_hint_weight_set(map, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(map);

   box = elm_box_add(win);
   evas_object_show(box);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "+");
   elm_box_pack_end(box, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", _bt_zoom_in, map);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "-");
   elm_box_pack_end(box, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", _bt_zoom_out, map);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "X");
   elm_box_pack_end(box, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", _bt_zoom_fit, map);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "#");
   elm_box_pack_end(box, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", _bt_zoom_fill, map);

   elm_map_geo_region_show(map, -43.2, -22.9);
   elm_map_zoom_set(map, 12);

   evas_object_resize(win, 512, 512);
   evas_object_show(win);

   ecore_timer_add(0.5, _nasty_hack, win);

   elm_run();
   return 0;
}
ELM_MAIN()

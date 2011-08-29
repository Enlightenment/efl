/**
 * Simple Elementary's <b>map widget</b> example, illustrating route and
 * name usage.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g `pkg-config --cflags --libs elementary` map_example_03.c -o map_example_03
 * @endverbatim
 */

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

typedef struct _Example_Data
{
   Evas_Object *map, *entry;
   Elm_Map_Route *route;
   double start_lon, start_lat, dest_lon, dest_lat;
   Elm_Map_Name *name;
} Example_Data;

static Example_Data example_data;

static void
_name_loaded(void *data, Evas_Object *obj, void *ev __UNUSED__)
{
   Example_Data *example_data = data;
   Evas_Object *map = obj;

   if (example_data->route)
     elm_map_route_remove(example_data->route);

   elm_map_name_region_get(example_data->name, &(example_data->dest_lon),
                           &(example_data->dest_lat));

   example_data->route = elm_map_route_add(map, ELM_MAP_ROUTE_TYPE_FOOT,
                     ELM_MAP_ROUTE_METHOD_SHORTEST,
                     example_data->start_lon, example_data->start_lat,
                     example_data->dest_lon, example_data->dest_lat);
   elm_map_route_color_set(example_data->route, 0, 255, 0, 255);
}

static void
_bt_route(void *data, Evas_Object *obj __UNUSED__, void *ev __UNUSED__)
{
   Example_Data *example_data = data;
   Evas_Object *map;
   char *address;

   map = example_data->map;
   address = (char *)elm_object_text_get(example_data->entry);

   example_data->name = elm_map_utils_convert_name_into_coord(map, address);

   evas_object_smart_callback_add(map, "name,loaded", _name_loaded, data);
}

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

EAPI_MAIN int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *map, *box, *bt, *entry;

   win = elm_win_add(NULL, "map", ELM_WIN_BASIC);
   elm_win_title_set(win, "Map Route Example");
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
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "-");
   elm_box_pack_end(box, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", _bt_zoom_out, map);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "X");
   elm_box_pack_end(box, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", _bt_zoom_fit, map);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "#");
   elm_box_pack_end(box, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", _bt_zoom_fill, map);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);

   elm_box_horizontal_set(box, EINA_TRUE);
   elm_win_resize_object_add(win, box);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, 0);

   entry = elm_entry_add(win);
   elm_entry_scrollable_set(entry, EINA_TRUE);
   elm_entry_single_line_set(entry, EINA_TRUE);
   elm_object_text_set(entry, "Jockey Club Brasileiro");
   evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, 0);
   elm_box_pack_end(box, entry);
   evas_object_show(entry);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Route");
   elm_box_pack_end(box, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", _bt_route, &example_data);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);

   example_data.map = map;
   example_data.entry = entry;
   example_data.route = NULL;
   example_data.start_lon = -43.175;
   example_data.start_lat = -22.97;

   elm_map_geo_region_show(map, example_data.start_lon, example_data.start_lat);
   elm_map_zoom_set(map, 12);

   evas_object_resize(win, 512, 512);
   evas_object_show(win);

   ecore_timer_add(0.5, _nasty_hack, win);

   elm_run();
   return 0;
}
ELM_MAIN()

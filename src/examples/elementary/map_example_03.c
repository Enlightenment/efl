/**
 * Simple Elementary's <b>map widget</b> example, illustrating route and
 * name usage.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g map_example_03.c -o map_example_03 `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#include <Elementary.h>

typedef struct _Example_Data
{
   Evas_Object *map, *entry;
   Elm_Map_Route *route;
   double start_lon, start_lat, dest_lon, dest_lat;
   Elm_Map_Name *name;
   Elm_Map_Overlay *route_ovl;
} Example_Data;

static Example_Data example_data;

static void
_route_loaded(void *data, Evas_Object *obj, void *ev)
{
   Example_Data *exam_data = data;

   exam_data->route_ovl = elm_map_overlay_route_add(obj, exam_data->route);
   elm_map_overlay_color_set(exam_data->route_ovl, 0, 255, 0, 255);
}

static void
_name_loaded(void *data, Evas_Object *obj, void *ev)
{
   Example_Data *exam_data = data;
   Evas_Object *map = obj;

   if (exam_data->route)
     elm_map_route_del(exam_data->route);

   elm_map_name_region_get(exam_data->name, &(exam_data->dest_lon),
                           &(exam_data->dest_lat));

   exam_data->route = elm_map_route_add(map, ELM_MAP_ROUTE_TYPE_FOOT,
                                        ELM_MAP_ROUTE_METHOD_SHORTEST,
                                        exam_data->start_lon, exam_data->start_lat,
                                        exam_data->dest_lon, exam_data->dest_lat,
                                        NULL, NULL);
}

static void
_bt_route(void *data, Evas_Object *obj, void *ev)
{
   Example_Data *exam_data = data;
   Evas_Object *map;
   char *address;

   map = exam_data->map;
   address = (char *)elm_object_text_get(exam_data->entry);

   exam_data->name = elm_map_name_add(map, address, 0, 0, NULL, NULL);

   evas_object_smart_callback_add(map, "name,loaded", _name_loaded, data);
   evas_object_smart_callback_add(map, "route,loaded", _route_loaded, data);
}

static void
_bt_zoom_in(void *data, Evas_Object *obj, void *ev)
{
   int zoom;
   elm_map_zoom_mode_set(data, ELM_MAP_ZOOM_MODE_MANUAL);
   zoom = elm_map_zoom_get(data);
   elm_map_zoom_set(data, zoom + 1);
}

static void
_bt_zoom_out(void *data, Evas_Object *obj, void *ev)
{
   int zoom;
   elm_map_zoom_mode_set(data, ELM_MAP_ZOOM_MODE_MANUAL);
   zoom = elm_map_zoom_get(data);
   elm_map_zoom_set(data, zoom - 1);
}

static void
_bt_zoom_fit(void *data, Evas_Object *obj, void *event_info)
{
   elm_map_zoom_mode_set(data, ELM_MAP_ZOOM_MODE_AUTO_FIT);
}

static void
_bt_zoom_fill(void *data, Evas_Object *obj, void *event_info)
{
   elm_map_zoom_mode_set(data, ELM_MAP_ZOOM_MODE_AUTO_FILL);
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
elm_main(int argc, char **argv)
{
   Evas_Object *win, *map, *box, *bt, *entry;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("map", "Map Route Example");
   elm_win_autodel_set(win, EINA_TRUE);

   map = elm_map_add(win);
   evas_object_size_hint_weight_set(map, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, map);
   evas_object_show(map);

   box = elm_box_add(win);
   elm_box_horizontal_set(box, EINA_TRUE);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
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

   elm_map_zoom_set(map, 12);
   elm_map_region_show(map, example_data.start_lon, example_data.start_lat);

   evas_object_resize(win, 512, 512);
   evas_object_show(win);

   ecore_timer_add(0.5, _nasty_hack, win);

   elm_run();

   return 0;
}
ELM_MAIN()

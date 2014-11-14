/**
 * Simple Elementary's <b>map widget</b> example, illustrating overlays
 * usage.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g map_example_02.c -o map_example_02 `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#include <Elementary.h>

typedef struct _Overlay_Data
{
   const char *name;
   const char *file;
} Overlay_Data;

Overlay_Data data_argentina = {"Argentina", NULL};
Overlay_Data data_chile = {"Chile", NULL};
Overlay_Data data_sampa = {"São Paulo", NULL};
Overlay_Data data_rio = {"Rio de Janeiro", NULL};
Overlay_Data data_brasilia = {"Brasília", NULL};

static Elm_Map_Overlay *bubble;
const char *data_dir;

static Evas_Object *
_icon_get(Evas_Object *obj, const char *file)
{
   Evas_Object *icon = elm_icon_add(obj);
   elm_image_file_set(icon, file, NULL);
   evas_object_show(icon);
   return icon;
}

static Evas_Object *
_city_icon_get(Evas_Object *obj)
{
   char buf[256];
   snprintf(buf, sizeof(buf), "%s/images/icon_07.png", data_dir);
   return _icon_get(obj, buf);
}

static Evas_Object *
_clas_city_icon_get(Evas_Object *obj)
{
   char buf[256];
   snprintf(buf, sizeof(buf), "%s/images/icon_05.png", data_dir);
   return _icon_get(obj, buf);
}

static Evas_Object *
_country_icon_get(Evas_Object *obj)
{
   char buf[256];
   snprintf(buf, sizeof(buf), "%s/images/icon_06.png", data_dir);
   return _icon_get(obj, buf);
}

static Evas_Object *
_clas_country_icon_get(Evas_Object *obj)
{
   char buf[256];
   snprintf(buf, sizeof(buf), "%s/images/icon_04.png", data_dir);
   return _icon_get(obj, buf);
}

static Evas_Object *
_box_get(Evas_Object *obj, Overlay_Data *data)
{
   Evas_Object *bx, *img, *label;
   bx = elm_box_add(obj);
   evas_object_show(bx);

   img = evas_object_image_add(evas_object_evas_get(obj));
   evas_object_image_file_set(img, data->file, NULL);
   evas_object_image_filled_set(img, EINA_TRUE);
   evas_object_size_hint_min_set(img, 64, 64);
   evas_object_show(img);
   elm_box_pack_end(bx, img);

   label = elm_label_add(obj);
   elm_object_text_set(label, data->name);
   evas_object_show(label);
   elm_box_pack_end(bx, label);
   return bx;
}

static void
_overlay_cb(void *data, Evas_Object *map, void *ev)
{
   printf("Overlay clicked\n");
   Elm_Map_Overlay *overlay = ev;
   Evas_Object *bx;

   // prevent duplication
   if (!bubble)  bubble = elm_map_overlay_bubble_add(map);

   elm_map_overlay_bubble_follow(bubble, overlay);
   elm_map_overlay_bubble_content_clear(bubble);
   bx = _box_get(map, elm_map_overlay_data_get(overlay));
   elm_map_overlay_bubble_content_append(bubble, bx);
}

static void
_bt_zoom_in(void *data, Evas_Object *obj, void *ev)
{
   Evas_Object *map = data;
   int zoom;

   elm_map_zoom_mode_set(map, ELM_MAP_ZOOM_MODE_MANUAL);
   zoom = elm_map_zoom_get(map);
   elm_map_zoom_set(map, zoom + 1);
}

static void
_bt_zoom_out(void *data, Evas_Object *obj, void *ev)
{
   Evas_Object *map = data;
   int zoom;

   elm_map_zoom_mode_set(map, ELM_MAP_ZOOM_MODE_MANUAL);
   zoom = elm_map_zoom_get(map);
   elm_map_zoom_set(map, zoom - 1);
}

static void
_bt_zoom_fit(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *map = data;
   elm_map_zoom_mode_set(map, ELM_MAP_ZOOM_MODE_AUTO_FIT);
}

static void
_bt_zoom_fill(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *map = data;
   elm_map_zoom_mode_set(map, ELM_MAP_ZOOM_MODE_AUTO_FILL);
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
   Evas_Object *win, *map, *box, *bt;
   Eina_List *ovls = NULL;
   Elm_Map_Overlay *ovl, *city_clas, *country_clas;
   char buf[255];

   elm_app_info_set(elm_main, "elementary", "images");
   data_dir = elm_app_data_dir_get();

   snprintf(buf, sizeof(buf), "%s/images/rock_01.jpg", "sdf");
   data_argentina.file = strdup(buf);
   snprintf(buf, sizeof(buf), "%s/images/rock_02.jpg", "sdf");
   data_chile.file = strdup(buf);
   snprintf(buf, sizeof(buf), "%s/images/sky_01.jpg", "sdf");
   data_sampa.file = strdup(buf);
   snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", "sdf");
   data_rio.file = strdup(buf);
   snprintf(buf, sizeof(buf), "%s/images/sky_03.jpg", "sdf");
   data_brasilia.file = strdup(buf);

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("map", "Map Overlay Example");
   elm_win_autodel_set(win, EINA_TRUE);

   map = elm_map_add(win);
   evas_object_size_hint_weight_set(map, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, map);
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

   evas_object_smart_callback_add(map, "overlay,clicked", _overlay_cb, NULL);

   city_clas = elm_map_overlay_class_add(map);
   elm_map_overlay_displayed_zoom_min_set(city_clas, 3);
   elm_map_overlay_icon_set(city_clas, _clas_city_icon_get(map));

   country_clas = elm_map_overlay_class_add(map);
   elm_map_overlay_displayed_zoom_min_set(country_clas, 1);
   elm_map_overlay_icon_set(country_clas, _clas_country_icon_get(map));

   ovl = elm_map_overlay_add(map, -43.2, -22.9);
   elm_map_overlay_icon_set(ovl, _city_icon_get(map));
   elm_map_overlay_data_set(ovl, &data_rio);
   elm_map_overlay_class_append(city_clas, ovl);
   ovls = eina_list_append(ovls, ovl);

   ovl = elm_map_overlay_add(map, -46.63, -23.55);
   elm_map_overlay_icon_set(ovl, _city_icon_get(map));
   elm_map_overlay_data_set(ovl, &data_sampa);
   elm_map_overlay_class_append(city_clas, ovl);
   ovls = eina_list_append(ovls, ovl);

   ovl = elm_map_overlay_add(map, -47.88, -15.78);
   elm_map_overlay_icon_set(ovl, _city_icon_get(map));
   elm_map_overlay_data_set(ovl, &data_brasilia);
   elm_map_overlay_class_append(city_clas, ovl);
   ovls = eina_list_append(ovls, ovl);

   ovl = elm_map_overlay_add(map, -65.23, -35.1);
   elm_map_overlay_icon_set(ovl, _country_icon_get(map));
   elm_map_overlay_data_set(ovl, &data_argentina);
   elm_map_overlay_class_append(country_clas, ovl);
   ovls = eina_list_append(ovls, ovl);

   ovl = elm_map_overlay_add(map, -71.3, -31.75);
   elm_map_overlay_icon_set(ovl, _country_icon_get(map));
   elm_map_overlay_data_set(ovl, &data_chile);
   elm_map_overlay_class_append(country_clas, ovl);
   ovls = eina_list_append(ovls, ovl);

   elm_map_overlays_show(ovls);

   evas_object_resize(win, 512, 512);
   evas_object_show(win);

   ecore_timer_add(1, _nasty_hack, win);

   elm_run();

   return 0;
}
ELM_MAIN()

/**
 * Simple Elementary's <b>map widget</b> example, illustrating markers
 * usage.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g `pkg-config --cflags --libs elementary` map_example_02.c -o map_example_02
 * @endverbatim
 */

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

typedef struct _Marker_Data
{
   const char *name;
   const char *file;
} Marker_Data;

Marker_Data data_argentina = {"Argentina",
     PACKAGE_DATA_DIR"/images/rock_01.jpg"};
Marker_Data data_chile = {"Chile",
     PACKAGE_DATA_DIR"/images/rock_02.jpg"};
Marker_Data data_sampa = {"São Paulo",
     PACKAGE_DATA_DIR"/images/sky_01.jpg"};
Marker_Data data_rio = {"Rio de Janeiro",
     PACKAGE_DATA_DIR"/images/sky_02.jpg"};
Marker_Data data_brasilia = {"Brasília",
     PACKAGE_DATA_DIR"/images/sky_03.jpg"};

static Evas_Object *
_marker_get(Evas_Object *obj, Elm_Map_Marker *marker __UNUSED__, void *data)
{
   Evas_Object *bx, *im, *lbl;
   Marker_Data *md = data;

   bx = elm_box_add(obj);
   evas_object_show(bx);

   im = elm_image_add(obj);
   elm_image_file_set(im, md->file, NULL);
   evas_object_size_hint_min_set(im, 64, 64);
   evas_object_show(im);
   elm_box_pack_end(bx, im);

   lbl = elm_label_add(obj);
   elm_object_text_set(lbl, md->name);
   evas_object_show(lbl);
   elm_box_pack_end(bx, lbl);

   return bx;
}

static Evas_Object *
_marker_city_content_get(Evas_Object *obj, Elm_Map_Marker *marker __UNUSED__, void *data __UNUSED__)
{
   Evas_Object *icon = elm_icon_add(obj);
   elm_icon_file_set(icon, PACKAGE_DATA_DIR"/images/icon_07.png", NULL);
   evas_object_show(icon);

   return icon;
}

static Evas_Object *
_group_city_content_get(Evas_Object *obj, void *data __UNUSED__)
{
   Evas_Object *icon = elm_icon_add(obj);
   elm_icon_file_set(icon, PACKAGE_DATA_DIR"/images/icon_05.png", NULL);
   evas_object_show(icon);
   return icon;
}

static Evas_Object *
_marker_country_content_get(Evas_Object *obj, Elm_Map_Marker *marker __UNUSED__, void *data __UNUSED__)
{
   Evas_Object *icon = elm_icon_add(obj);
   elm_icon_file_set(icon, PACKAGE_DATA_DIR"/images/icon_06.png", NULL);
   evas_object_show(icon);
   return icon;
}

static Evas_Object *
_group_country_content_get(Evas_Object *obj, void *data __UNUSED__)
{
   Evas_Object *icon = elm_icon_add(obj);
   elm_icon_file_set(icon, PACKAGE_DATA_DIR"/images/icon_04.png", NULL);
   evas_object_show(icon);
   return icon;
}

static void
_map_downloaded(void *data __UNUSED__, Evas_Object *obj, void *ev __UNUSED__)
{
   elm_map_zoom_set(obj, 3);
   evas_object_smart_callback_del(obj, "downloaded", _map_downloaded);
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
   Evas_Object *win, *bg, *map, *box, *bt;
   static Elm_Map_Marker_Class *mc_city, *mc_country;
   static Elm_Map_Group_Class *gc_city, *gc_country;
   Eina_List *markers = NULL;
   Elm_Map_Marker *m;

   win = elm_win_add(NULL, "map", ELM_WIN_BASIC);
   elm_win_title_set(win, "Map Markers Example");
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

   mc_city = elm_map_marker_class_new(map);
   elm_map_marker_class_get_cb_set(mc_city, _marker_get);
   elm_map_marker_class_icon_cb_set(mc_city, _marker_city_content_get);
   elm_map_marker_class_style_set(mc_city, "radio");

   gc_city = elm_map_group_class_new(map);
   elm_map_group_class_style_set(gc_city, "radio2");
   elm_map_group_class_icon_cb_set(gc_city, _group_city_content_get);
   elm_map_group_class_zoom_displayed_set(gc_city, 3);

   mc_country = elm_map_marker_class_new(map);
   elm_map_marker_class_get_cb_set(mc_country, _marker_get);
   elm_map_marker_class_icon_cb_set(mc_country, _marker_country_content_get);
   elm_map_marker_class_style_set(mc_country, "empty");

   gc_country = elm_map_group_class_new(map);
   elm_map_group_class_style_set(gc_country, "empty");
   elm_map_group_class_icon_cb_set(gc_country, _group_country_content_get);
   elm_map_group_class_zoom_displayed_set(gc_country, 1);

   m = elm_map_marker_add(map, -43.2, -22.9, mc_city, gc_city, &data_rio);
   markers = eina_list_append(markers, m);
   m = elm_map_marker_add(map, -46.63, -23.55, mc_city, gc_city, &data_sampa);
   markers = eina_list_append(markers, m);
   m = elm_map_marker_add(map, -47.88, -15.78, mc_city, gc_city,
                          &data_brasilia);
   markers = eina_list_append(markers, m);

   m = elm_map_marker_add(map, -65.23, -35.1, mc_country, gc_country,
                      &data_argentina);
   markers = eina_list_append(markers, m);
   m = elm_map_marker_add(map, -71.3, -31.75, mc_country, gc_country,
                      &data_chile);
   markers = eina_list_append(markers, m);

   elm_map_markers_list_show(markers);
   evas_object_smart_callback_add(map, "downloaded", _map_downloaded, NULL);

   evas_object_resize(win, 512, 512);
   evas_object_show(win);

   ecore_timer_add(1, _nasty_hack, win);

   elm_run();
   return 0;
}
ELM_MAIN()

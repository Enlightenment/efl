#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

#define SOURCE_MAX 10
#define MARKER_MAX 1000
#define NAME_ENTRY_TEXT "Enter freeform address"

typedef struct Overlay_Data
{
   const char *file;
} Overlay_Data;

typedef struct Map_Source
{
   Evas_Object *map;
   Elm_Map_Source_Type type;
   char *source_name;
} Map_Source;

Overlay_Data data1 = {PACKAGE_DATA_DIR"/images/logo.png"};
Overlay_Data data2 = {PACKAGE_DATA_DIR"/images/logo_small.png"};
Overlay_Data data3 = {PACKAGE_DATA_DIR"/images/panel_01.jpg"};
Overlay_Data data4 = {PACKAGE_DATA_DIR"/images/plant_01.jpg"};
Overlay_Data data5 = {PACKAGE_DATA_DIR"/images/rock_01.jpg"};
Overlay_Data data6 = {PACKAGE_DATA_DIR"/images/rock_02.jpg"};
Overlay_Data data7 = {PACKAGE_DATA_DIR"/images/sky_01.jpg"};
Overlay_Data data8 = {PACKAGE_DATA_DIR"/images/sky_02.jpg"};
Overlay_Data data9 = {PACKAGE_DATA_DIR"/images/sky_03.jpg"};
Overlay_Data data10 = {PACKAGE_DATA_DIR"/images/sky_03.jpg"};
Overlay_Data data11= {PACKAGE_DATA_DIR"/images/wood_01.jpg"};
Overlay_Data parking= {PACKAGE_DATA_DIR"/images/parking.png"};
Overlay_Data icon_data = {PACKAGE_DATA_DIR"/images/icon_14.png"};

static Elm_Map_Overlay *route_start, *route_end, *route_clas;
static Elm_Map_Overlay *bubble_img;
static Elm_Map_Overlay *bubble_parking;
static Elm_Map_Overlay *route_ovl;
static Elm_Map_Overlay *line_start, *line_end, *line;
static Elm_Map_Overlay *poly;
static Elm_Map_Overlay *circle;
static Elm_Map_Overlay *scale;
static Eina_List *poly_points;

static Evas_Object *menu, *fs_win;
static Elm_Map_Route *route;
static Elm_Map_Name *name;
static Evas_Object *track;
static Evas_Coord down_x, down_y;
static Evas_Coord old_x, old_y, old_d;
static Map_Source ts[SOURCE_MAX];
static Map_Source rs[SOURCE_MAX];
static Map_Source ns[SOURCE_MAX];

static void
#ifdef ELM_EMAP
my_map_gpx_fileselector_done(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
#else
my_map_gpx_fileselector_done(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
#endif
{
   const char *selected = event_info;

   if (selected)
     {
        printf("Selected file: %s\n", selected);
#ifdef ELM_EMAP
        EMap_Route *emap = emap_route_gpx_new(selected);
        track = elm_map_track_add(data, emap);
#else
        printf("libEMap is required !\n");
#endif
     }
   evas_object_del(fs_win);
}

static Evas_Object *
_route_icon_get(Evas_Object *obj)
{
   Evas_Object *icon = elm_icon_add(obj);
   elm_image_file_set(icon, PACKAGE_DATA_DIR"/images/bubble.png", NULL);
   evas_object_show(icon);

   return icon;
}

static Evas_Object *
_box_get(Evas_Object *obj, Overlay_Data *data, Elm_Map_Overlay *ovl)
{
   Evas_Object *bx, *img, *label;
   double lon, lat;
   char buf[256];
   bx = elm_box_add(obj);
   evas_object_show(bx);

   img = evas_object_image_add(evas_object_evas_get(obj));
   evas_object_image_file_set(img, data->file, NULL);
   evas_object_image_filled_set(img, EINA_TRUE);
   evas_object_size_hint_min_set(img, 64, 64);
   evas_object_show(img);
   elm_box_pack_end(bx, img);

   label = elm_label_add(bx);
   elm_map_overlay_region_get(ovl, &lon, &lat);
   snprintf(buf, sizeof(buf), "%0.4lf %0.4lf", lon, lat);
   elm_object_text_set(label, buf);
   evas_object_show(label);
   elm_box_pack_end(bx, label);
   return bx;
}

static Evas_Object *
_label_get(Evas_Object *obj)
{
   Evas_Object *label;
   label = elm_label_add(obj);
   elm_object_text_set(label, "Here is a parking lot.");
   evas_object_show(label);
   return label;
}

static Evas_Object *
_icon_get(Evas_Object *obj, Overlay_Data *data)
{
   Evas_Object *icon = elm_icon_add(obj);
   elm_image_file_set(icon, data->file, NULL);
   evas_object_show(icon);

   return icon;
}

static void
_overlay_hide(void *data, Evas_Object *obj EINA_UNUSED, void *ev EINA_UNUSED)
{
   elm_map_overlay_hide_set(data, EINA_TRUE);
}

static void
_overlay_pause(void *data, Evas_Object *obj EINA_UNUSED, void *ev EINA_UNUSED)
{
   elm_map_overlay_paused_set(data, EINA_TRUE);
}

static void
_overlay_unpause(void *data, Evas_Object *obj EINA_UNUSED, void *ev EINA_UNUSED)
{
   elm_map_overlay_paused_set(data, EINA_FALSE);
}

static void
_overlay_show(void *data, Evas_Object *obj EINA_UNUSED, void *ev EINA_UNUSED)
{
   elm_map_overlay_show(data);
}

static Evas_Object *
_btn_box_get(Evas_Object *obj, Elm_Map_Overlay *ovl)
{
   Evas_Object *bx, *btn, *btn2, *btn3, *btn4;
   bx = elm_box_add(obj);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_show(bx);

   btn = elm_button_add(bx);
   elm_object_text_set(btn, "Hide");
   evas_object_smart_callback_add(btn, "clicked", _overlay_hide, ovl);
   evas_object_show(btn);
   elm_box_pack_end(bx, btn);

   btn2 = elm_button_add(bx);
   elm_object_text_set(btn2, "Pause");
   evas_object_smart_callback_add(btn2, "clicked", _overlay_pause, ovl);
   evas_object_show(btn2);
   elm_box_pack_end(bx, btn2);

   btn3 = elm_button_add(bx);
   elm_object_text_set(btn3, "Unpause");
   evas_object_smart_callback_add(btn3, "clicked", _overlay_unpause, ovl);
   evas_object_show(btn3);
   elm_box_pack_end(bx, btn3);

   btn4 = elm_button_add(bx);
   elm_object_text_set(btn4, "Show");
   evas_object_smart_callback_add(btn4, "clicked", _overlay_show, ovl);
   evas_object_show(btn4);
   elm_box_pack_end(bx, btn4);

   return bx;
}

static void
_bubble_parking_follow(Evas_Object *map)
{
   double lon, lat;
   Evas_Coord x, y;

   if (bubble_parking)
     {
        Elm_Map_Overlay *ovl = elm_map_overlay_data_get(bubble_parking);
        elm_map_overlay_region_get(ovl, &lon, &lat);
        elm_map_region_to_canvas_convert(map, lon, lat, &x, &y);
        elm_map_canvas_to_region_convert(map, x+40, y+50, &lon, &lat);
        elm_map_overlay_region_set(bubble_parking, lon, lat);
     }
}

static void
_overlays_num_check(Evas_Object *obj)
{
   Evas_Coord x, y, w, h;
   double lon, lat, max_lon, max_lat, min_lon, min_lat;
   Eina_List *overlays, *l;
   Elm_Map_Overlay *ovl;
   int cnt = 0;
   int cnt_visible = 0;

   overlays = elm_map_overlays_get(obj);
   evas_object_geometry_get(obj, &x, &y, &w, &h);
   elm_map_canvas_to_region_convert(obj, x, y, &min_lon, &max_lat);
   elm_map_canvas_to_region_convert(obj, x + w, y + h, &max_lon, &min_lat);

   EINA_LIST_FOREACH(overlays, l, ovl)
     {
        if (elm_map_overlay_type_get(ovl) == ELM_MAP_OVERLAY_TYPE_CLASS)
           continue;
        elm_map_overlay_region_get(ovl, &lon, &lat);
        if ((min_lon <= lon) && (lon <= max_lon) &&
            (min_lat <= lat) && (lat <= max_lat))
          {
             if (elm_map_overlay_visible_get(ovl)) cnt_visible++;
             cnt++;
          }
     }
   printf("Number of (visible/total) overlays in viewport: %d/%d\n",
          cnt_visible, cnt);
}

static void
_map_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("clicked\n");
}

static void
_map_clicked_double(void *data EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   printf("clicked,double\n");
   double lon, lat;
   Evas_Event_Mouse_Down *ev = event_info;
   if (!ev) return;
   if (elm_map_zoom_get(obj) < 5) return;

   elm_map_canvas_to_region_convert(obj, ev->canvas.x, ev->canvas.y, &lon, &lat);
   printf("x:%d, y:%d, lon:%lf, lat:%lf\n", ev->canvas.x, ev->canvas.y, lon, lat);

   if (!route_clas)
     {
        route_clas = elm_map_overlay_class_add(obj);
        elm_map_overlay_icon_set(route_clas, _route_icon_get(obj));
        elm_map_overlay_displayed_zoom_min_set(route_clas, 5);
     }

   if (route_start && route_end)
     {
        elm_map_overlay_del(route_start);
        elm_map_overlay_del(route_end);
        elm_map_route_del(route);
        route_start = NULL;
        route_end = NULL;
        route = NULL;
     }

   if (!route_start) route_start = elm_map_overlay_add(obj, lon, lat);
   else route_end = elm_map_overlay_add(obj, lon, lat);

   if (route_start && route_end)
     {
        double start_lon, start_lat, end_lon, end_lat;
        elm_map_overlay_class_append(route_clas, route_start);
        elm_map_overlay_class_append(route_clas, route_end);
        elm_map_overlay_region_get(route_start, &start_lon, &start_lat);
        elm_map_overlay_region_get(route_end, &end_lon, &end_lat);
        route = elm_map_route_add(obj, ELM_MAP_ROUTE_TYPE_MOTOCAR,
                                     ELM_MAP_ROUTE_METHOD_FASTEST,
                                     start_lon, start_lat, end_lon, end_lat,
                                     NULL, NULL);
     }
}

static void
_map_press(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("press\n");
}

static void
_map_longpressed(void *data EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   if (!event_info) return;
   double lon, lat;
   Evas_Event_Mouse_Down *ev = event_info;
   elm_map_canvas_to_region_convert(obj, ev->canvas.x, ev->canvas.y, &lon, &lat);
   printf("longpressed, x:%d, y:%d, lon:%lf, lat:%lf\n", ev->canvas.x, ev->canvas.y, lon, lat);

   if (elm_map_zoom_get(obj) < 8) return;
   if (name) elm_map_name_del(name);
   name = elm_map_name_add(obj, NULL, lon, lat, NULL, NULL);
}

static void
_map_scroll(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   double lon, lat;
   elm_map_region_get(obj, &lon, &lat);
   printf("scroll, longitude: %f latitude: %f\n", lon, lat);
   _bubble_parking_follow(obj);
}

static void
_map_drag_start(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("scroll,drag,start\n");
   evas_object_smart_callback_del(data, "longpressed", _map_longpressed);
}

static void
_map_drag_stop(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("scroll,drag,stop\n");
   evas_object_smart_callback_add(data, "longpressed", _map_longpressed, data);
   _overlays_num_check(obj);
}

static void
_map_anim_start(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("scroll,anim,start\n");
}

static void
_map_anim_stop(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("scroll,anim,stop\n");
}

static void
_map_zoom_start(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("zoom,start\n");
}

static void
_map_zoom_stop(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("zoom,stop\n");
   _overlays_num_check(obj);
}

static void
_map_zoom_change(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("zoom,change\n");
   _bubble_parking_follow(obj);
}

static void
_map_loaded(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("loaded\n");
}

static void
_map_tile_load(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("tile,load\n");
}

static void
_map_tile_loaded(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   int try_num, finish_num;
   elm_map_tile_load_status_get(data, &try_num, &finish_num);
   printf("tile,loaded: %d / %d\n", finish_num, try_num);
}

static void
_map_tile_loaded_fail(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   int try_num, finish_num;
   elm_map_tile_load_status_get(data, &try_num, &finish_num);
   printf("tile,loaded,fail: %d / %d\n", finish_num, try_num);
}

static void
_map_route_load(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("route,load\n");
}

static void
_map_route_loaded(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   printf("route,loaded\n");
   double d;
   const char *w, *n;

   d = elm_map_route_distance_get(route);
   printf("route distance = %lf km\n", d);

   w = elm_map_route_waypoint_get(route);
   if (w) printf("[waypoints]\n%s\n", w);

   n = elm_map_route_node_get(route);
   if (n) printf("[nodes]\n%s\n", n);

   if (route_ovl) elm_map_overlay_del(route_ovl);
   route_ovl = elm_map_overlay_route_add(obj, route);
}

static void
_map_route_loaded_fail(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("route,loaded,fail\n");
}

static void
_map_name_load(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("name,load\n");
}

static void
_map_name_loaded(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("name,loaded\n");
   if (!name) return;
   double lon, lat;
   const char *addr = elm_map_name_address_get(name);
   elm_map_name_region_get(name, &lon, &lat);
   if (addr)
     {
        printf("name of [lon = %lf, lat = %lf] is %s\n", lon, lat, addr);
        if ((lon != 0.0) && (lat !=0.0))
          {
             Eina_Bool b = elm_map_paused_get(data);
             elm_map_paused_set(data, EINA_TRUE);
             elm_map_zoom_mode_set(data, ELM_MAP_ZOOM_MODE_MANUAL);
             elm_map_zoom_set(data, elm_map_zoom_max_get(data));
             elm_map_region_show(data, lon, lat);
             elm_map_paused_set(data, b);
          }
     }

   elm_map_name_del(name);
   name = NULL;
}

static void
_map_name_loaded_fail(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("name,loaded,fail\n");
}

static void
_src_set(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Map_Source *s = data;

   if (!s) return;
   elm_map_source_set(s->map, s->type, s->source_name);
}

static void
_show_urmatt(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_map_zoom_mode_set(data, ELM_MAP_ZOOM_MODE_MANUAL);
   if (elm_map_zoom_get(data) < 12) elm_map_zoom_set(data, 12);
   elm_map_region_show(data,7.325201, 48.526813);
}

static void
_bring_seoul(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_map_zoom_mode_set(data, ELM_MAP_ZOOM_MODE_MANUAL);
   if (elm_map_zoom_get(data) < 12) elm_map_zoom_set(data, 12);
   elm_map_region_bring_in(data, 126.977969, 37.566535);
}

static void
_bring_zoom_suwon(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_map_region_zoom_bring_in(data, 16, 126.977969, 37.566535);
}

static void
_paused_set(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_map_paused_set(data, EINA_TRUE);
}

static void
_paused_unset(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_map_paused_set(data, EINA_FALSE);
}

static void
_zoom_in(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   int zoom;

   zoom = elm_map_zoom_get(data) + 1;
   elm_map_zoom_mode_set(data, ELM_MAP_ZOOM_MODE_MANUAL);
   elm_map_zoom_set(data, zoom);
}

static void
_zoom_out(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   int zoom;

   zoom = elm_map_zoom_get(data) - 1;
   elm_map_zoom_mode_set(data, ELM_MAP_ZOOM_MODE_MANUAL);
   elm_map_zoom_set(data, zoom);
}

static void
_zoom_fit(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_map_zoom_mode_set(data, ELM_MAP_ZOOM_MODE_AUTO_FIT);
}

static void
_zoom_fill(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_map_zoom_mode_set(data, ELM_MAP_ZOOM_MODE_AUTO_FILL);
}

static void
_zoom_manual(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_map_zoom_mode_set(data, ELM_MAP_ZOOM_MODE_MANUAL);
}

static void
_track_add(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *fs, *vbox, *hbox, *sep;
   char *path = NULL;

   fs_win = elm_win_util_standard_add("fileselector", "File Selector");
   elm_win_autodel_set(fs_win, EINA_TRUE);

   vbox = elm_box_add(fs_win);
   evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(fs_win, vbox);
   evas_object_show(vbox);

   fs = elm_fileselector_add(fs_win);
   elm_fileselector_is_save_set(fs, EINA_TRUE);
   elm_fileselector_expandable_set(fs, EINA_FALSE);
   path = getenv("HOME");
   //if "HOME" is not available, set current dir. path
   if (!path)
     path = ".";
   elm_fileselector_path_set(fs, path);
   evas_object_size_hint_weight_set(fs, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fs, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(vbox, fs);
   evas_object_show(fs);

   evas_object_smart_callback_add(fs, "done", my_map_gpx_fileselector_done, data);

   sep = elm_separator_add(fs_win);
   elm_separator_horizontal_set(sep, EINA_TRUE);
   elm_box_pack_end(vbox, sep);
   evas_object_show(sep);

   hbox = elm_box_add(fs_win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   elm_box_pack_end(vbox, hbox);
   evas_object_show(hbox);

   evas_object_resize(fs_win, 240, 350);
   evas_object_show(fs_win);
}


static void
_track_remove(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_map_track_remove(data, track);
}

static void
_rotate_cw(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   double d;
   Evas_Coord x, y, w, h;
   float half_w, half_h;
   evas_object_geometry_get(data, &x, &y, &w, &h);
   half_w = (float)w * 0.5;
   half_h = (float)h * 0.5;

   elm_map_rotate_get(data, &d, NULL, NULL);
   d += 15.0;
   elm_map_rotate_set(data, d, x + half_w, y + half_h);
}

static void
_rotate_ccw(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   double d;
   Evas_Coord x, y, w, h;
   float half_w, half_h;
   evas_object_geometry_get(data, &x, &y, &w, &h);
   half_w = (float)w * 0.5;
   half_h = (float)h * 0.5;

   elm_map_rotate_get(data, &d, NULL, NULL);
   d -= 15.0;
   elm_map_rotate_set(data, d, x + half_w, y + half_h);
}

static void
_rotate_reset(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Coord x, y, w, h;
   float half_w, half_h;
   evas_object_geometry_get(data, &x, &y, &w, &h);
   half_w = (float)w * 0.5;
   half_h = (float)h * 0.5;

   elm_map_rotate_set(data, 0.0, x + half_w, y + half_h);
}

static void
_wheel_disable(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_map_wheel_disabled_set(data, EINA_TRUE);
}

static void
_wheel_enable(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_map_wheel_disabled_set(data, EINA_FALSE);
}

static void
_zoom_min_set(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_map_zoom_min_set(data, 1);
}

static void
_zoom_max_set(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_map_zoom_max_set(data, 10);
}

static void
_line_add(void *data, Evas_Object *obj EINA_UNUSED, void *ei EINA_UNUSED)
{
   double lon, lat;

   elm_map_canvas_to_region_convert(data, down_x, down_y, &lon, &lat);
   printf("line marker: %d %d %lf %lf\n", down_x, down_y, lon, lat);
   if (line_start && line_end)
     {
        elm_map_overlay_del(line_start);
        elm_map_overlay_del(line_end);
        elm_map_overlay_del(line);
        line_start = NULL;
        line_end = NULL;
        line = NULL;
     }
   if (!line_start) line_start = elm_map_overlay_add(data, lon, lat);
   else if (!line_end) line_end = elm_map_overlay_add(data, lon, lat);

   if (line_start && line_end)
     {
        double flon, flat, tlon, tlat;
        elm_map_overlay_region_get(line_start, &flon, &flat);
        elm_map_overlay_region_get(line_end, &tlon, &tlat);
        line = elm_map_overlay_line_add(data, flon, flat, tlon, tlat);
        printf("line add: (%lf, %lf) --> (%lf, %lf)\n", flon, flat, tlon, tlat);
     }
}

static void
_poly_add(void *data, Evas_Object *obj EINA_UNUSED, void *ei EINA_UNUSED)
{
   double lon, lat;

   elm_map_canvas_to_region_convert(data, down_x, down_y, &lon, &lat);
   printf("%d %d %lf %lf\n", down_x, down_y, lon, lat);

   if (!poly) poly = elm_map_overlay_polygon_add(data);
   poly_points = eina_list_append(poly_points,
                                  elm_map_overlay_add(data, lon, lat));
   elm_map_overlay_polygon_region_add(poly, lon, lat);
}

static void
_poly_clear(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *ei EINA_UNUSED)
{
   Elm_Map_Overlay *ovl;
   if (poly) elm_map_overlay_del(poly);
   EINA_LIST_FREE(poly_points, ovl) elm_map_overlay_del(ovl);
   poly = NULL;
   poly_points = NULL;
}

static void
_circle_add(void *data, Evas_Object *obj EINA_UNUSED, void *ei EINA_UNUSED)
{
   double radius = 100;
   double lon, lat;

   if (circle) elm_map_overlay_del(circle);
   elm_map_canvas_to_region_convert(data, down_x, down_y, &lon, &lat);
   circle = elm_map_overlay_circle_add(data, lon, lat, radius);
}

static void
_scale_add(void *data, Evas_Object *obj EINA_UNUSED, void *ei EINA_UNUSED)
{
   if (scale) elm_map_overlay_del(scale);
   scale = elm_map_overlay_scale_add(data, down_x, down_y);
}

static void
_submenu_src_add(void *data, Elm_Object_Item *parent)
{
   int idx;
   const char **tile_srcs;
   const char **route_srcs;
   const char **name_srcs;

   if ((!data) || (!parent)) return;

   tile_srcs = elm_map_sources_get(data, ELM_MAP_SOURCE_TYPE_TILE);
   route_srcs = elm_map_sources_get(data, ELM_MAP_SOURCE_TYPE_ROUTE);
   name_srcs = elm_map_sources_get(data, ELM_MAP_SOURCE_TYPE_NAME);

   for (idx = 0; tile_srcs[idx]; idx++)
     {
        if (idx >= SOURCE_MAX) break;
        ts[idx].map = data;
        ts[idx].type = ELM_MAP_SOURCE_TYPE_TILE;
        ts[idx].source_name = strdup(tile_srcs[idx]);
        elm_menu_item_add(menu, parent, "", tile_srcs[idx], _src_set, &ts[idx]);
     }
   for (idx = 0; route_srcs[idx]; idx++)
     {
        if (idx >= SOURCE_MAX) break;
        rs[idx].map = data;
        rs[idx].type = ELM_MAP_SOURCE_TYPE_ROUTE;
        rs[idx].source_name = strdup(route_srcs[idx]);
        elm_menu_item_add(menu, parent, "", route_srcs[idx], _src_set, &rs[idx]);
     }
   for (idx = 0; name_srcs[idx]; idx++)
     {
        if (idx >= SOURCE_MAX) break;
        ns[idx].map = data;
        ns[idx].type = ELM_MAP_SOURCE_TYPE_NAME;
        ns[idx].source_name = strdup(name_srcs[idx]);
        elm_menu_item_add(menu, parent, "", name_srcs[idx], _src_set, &ns[idx]);
     }
}

static void
_submenu_move_add(void *data, Elm_Object_Item *parent)
{
   if ((!data) || (!parent)) return;
   elm_menu_item_add(menu, parent, NULL, "Show Urmatt", _show_urmatt, data);
   elm_menu_item_add(menu, parent, NULL, "Bring Seoul", _bring_seoul, data);
   elm_menu_item_add(menu, parent, NULL, "Zoom & Bring Suwon", _bring_zoom_suwon, data);

}

static void
_submenu_zoom_add(void *data, Elm_Object_Item *parent)
{
   if ((!data) || (!parent)) return;
   elm_menu_item_add(menu, parent, NULL, "Zoom +", _zoom_in, data);
   elm_menu_item_add(menu, parent, NULL, "Zoom -", _zoom_out, data);
   elm_menu_item_add(menu, parent, NULL, "Zoom Fit", _zoom_fit, data);
   elm_menu_item_add(menu, parent, NULL, "Zoom Fill", _zoom_fill, data);
   elm_menu_item_add(menu, parent, NULL, "Zoom Manual", _zoom_manual, data);
   elm_menu_item_add(menu, parent, NULL, "Zoom Min to 1", _zoom_min_set, data);
   elm_menu_item_add(menu, parent, NULL, "Zoom Max to 10", _zoom_max_set, data);
}

static void
_submenu_prop_add(void *data, Elm_Object_Item *parent)
{
   if ((!data) || (!parent)) return;
   elm_menu_item_add(menu, parent, NULL, "Paused Set", _paused_set, data);
   elm_menu_item_add(menu, parent, NULL, "Paused Unset", _paused_unset, data);
   elm_menu_item_add(menu, parent, NULL, "Rotate CW", _rotate_cw, data);
   elm_menu_item_add(menu, parent, NULL, "Rotate CCW", _rotate_ccw, data);
   elm_menu_item_add(menu, parent, NULL, "Reset Rotate", _rotate_reset, data);
   elm_menu_item_add(menu, parent, NULL, "Disable Wheel", _wheel_disable, data);
   elm_menu_item_add(menu, parent, NULL, "Enable Wheel", _wheel_enable, data);
}

static void
_submenu_track_add(void *data, Elm_Object_Item *parent)
{
   if ((!data) || (!parent)) return;
   elm_menu_item_add(menu, parent, NULL, "Add Track", _track_add, data);
   elm_menu_item_add(menu, parent, NULL, "Remove Track", _track_remove, data);
}

static void
_submenu_ovl_add(void *data, Elm_Object_Item *parent)
{
   if ((!data) || (!parent)) return;
   elm_menu_item_add(menu, parent, NULL, "Add line", _line_add, data);
   elm_menu_item_add(menu, parent, NULL, "Add polygon", _poly_add, data);
   elm_menu_item_add(menu, parent, NULL, "Clear polygon", _poly_clear, data);
   elm_menu_item_add(menu, parent, NULL, "Add circle", _circle_add, data);
   elm_menu_item_add(menu, parent, NULL, "Add scale", _scale_add, data);
}

static void
_submenu_info_add(Evas_Object *map, Elm_Object_Item *parent)
{
   if (!map || !parent) return;
   char buf[PATH_MAX] = { 0 };
   double lon = 0.0, lat = 0.0;

   elm_map_canvas_to_region_convert(map,
                                    down_x, down_y,
                                    &lon, &lat);

   snprintf(buf, PATH_MAX, "Longitude : %f", lon);
   elm_menu_item_add(menu, parent, NULL, buf, NULL, NULL);

   snprintf(buf, PATH_MAX, "Latitude : %f", lat);
   elm_menu_item_add(menu, parent, NULL, buf, NULL, NULL);
}

static void
_map_mouse_down(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Elm_Object_Item *menu_it;
   static Elm_Object_Item *info_it = NULL;

   if (!ev) return;

   if (ev->button == 2)
     {
        old_x = ev->canvas.x;
        old_y = ev->canvas.y;
        old_d = 0.0;
     }
   else if (ev->button == 3)
     {
        down_x = ev->canvas.x;
        down_y = ev->canvas.y;
        if (!menu)
          {
             menu = elm_menu_add(obj);
             elm_menu_parent_set(menu, obj);
             menu_it = elm_menu_item_add(menu, NULL, "", "Source", NULL, NULL);
             _submenu_src_add(data, menu_it);
             menu_it = elm_menu_item_add(menu, NULL, "", "Move", NULL, NULL);
             _submenu_move_add(data, menu_it);
             menu_it = elm_menu_item_add(menu, NULL, "", "Zoom", NULL, NULL);
             _submenu_zoom_add(data, menu_it);
             menu_it = elm_menu_item_add(menu, NULL, "", "Prop", NULL, NULL);
             _submenu_prop_add(data, menu_it);
             menu_it = elm_menu_item_add(menu, NULL, "", "Track", NULL, NULL);
             _submenu_track_add(data, menu_it);
             menu_it = elm_menu_item_add(menu, NULL, "", "Overlay", NULL, NULL);
             _submenu_ovl_add(data, menu_it);

             info_it = elm_menu_item_add(menu, NULL, "", "Info", NULL, NULL);
          }

        elm_menu_item_subitems_clear(info_it);
        _submenu_info_add(obj, info_it);

        elm_menu_move(menu, ev->canvas.x, ev->canvas.y);
        evas_object_show(menu);
     }
}

static void
_map_mouse_move(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Move *move = event_info;
   Evas_Coord x, y, w, h;
   float half_w, half_h;
   int d, d_diff;
   double cur_d;
   if (!move) return;

   if (move->buttons == 2)
     {
        evas_object_geometry_get(data, &x, &y, &w, &h);
        half_w = (float)w * 0.5;
        half_h = (float)h * 0.5;
        elm_map_rotate_get(data, &cur_d, NULL, NULL);

        d = move->cur.canvas.x - old_x;
        if (!old_d) old_d = d;
        else
          {
             d_diff = old_d - d;
             if (d_diff > 0)
               {
                  old_d --;
                  cur_d += 1.0;
               }
             else if (d_diff < 0)
               {
                  old_d ++;
                  cur_d -= 1.0;
               }
             old_d = d;
             elm_map_rotate_set(data, cur_d, x + half_w, y + half_h);
          }
     }
}

static void
_map_mouse_up(void *data EINA_UNUSED, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Up *up = event_info;
   if (!up) return;

   if (up->button == 2)
     {
        old_x = 0;
        old_y = 0;
     }
}

static void
_overlay_cb(void *data EINA_UNUSED, Evas_Object *map, void *ev)
{
   printf("Overlay clicked: ");
   Elm_Map_Overlay *overlay = ev;
   Overlay_Data *od;
   Elm_Map_Overlay_Type type = elm_map_overlay_type_get(overlay);

   if (type != ELM_MAP_OVERLAY_TYPE_GROUP &&
       type != ELM_MAP_OVERLAY_TYPE_DEFAULT) return;

   if (!bubble_img)  bubble_img = elm_map_overlay_bubble_add(map);
   elm_map_overlay_bubble_follow(bubble_img, overlay);
   elm_map_overlay_bubble_content_clear(bubble_img);

   if (type == ELM_MAP_OVERLAY_TYPE_GROUP)
     {
        Eina_List *l;
        Elm_Map_Overlay *memb;
        Eina_List *members = elm_map_overlay_group_members_get(overlay);
        printf("Group Members Num: %d\n", eina_list_count(members));
        EINA_LIST_FOREACH(members, l, memb)
          {
             od = elm_map_overlay_data_get(memb);
             if (od)
                elm_map_overlay_bubble_content_append(bubble_img,
                                                      _box_get(map, od, memb));
          }
      }
     else
     {
        od = elm_map_overlay_data_get(overlay);
        if (od)
           elm_map_overlay_bubble_content_append(bubble_img,
                                                 _box_get(map, od, overlay));
        elm_map_overlay_bubble_content_append(bubble_img,
                                              _btn_box_get(map, overlay));
     }
}

static void
_parking_cb(void *data EINA_UNUSED, Evas_Object *map, Elm_Map_Overlay *ovl)
{
   double lon, lat;
   Evas_Coord x, y;
   elm_map_overlay_region_get(ovl, &lon, &lat);
   elm_map_region_to_canvas_convert(map, lon, lat, &x, &y);
   printf("Parking clicked: %lf %lf %d %d\n", lon, lat, x, y);

   elm_map_canvas_to_region_convert(map, x+40, y+50, &lon, &lat);
   if (!bubble_parking)
     {
        Evas_Object *bubble, *label;
        bubble = elm_bubble_add(map);
        elm_bubble_pos_set(bubble, ELM_BUBBLE_POS_TOP_LEFT);
        elm_object_text_set(bubble, "Overlay object");
        elm_object_part_text_set(bubble, "info", "Bubble is overlayed");

        label = elm_label_add(bubble);
        elm_object_text_set(label, "Parking Here !!");
        evas_object_show(label);
        elm_object_content_set(bubble, label);

        evas_object_resize(bubble, 125, 50);
        evas_object_show(bubble);

        bubble_parking = elm_map_overlay_add(map, lon, lat);
        elm_map_overlay_content_set(bubble_parking, bubble);
     }
   else elm_map_overlay_region_set(bubble_parking, lon, lat);
   elm_map_overlay_data_set(bubble_parking, ovl);
}

static void
_del_map(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *ei EINA_UNUSED)
{
   if (route_start)    elm_map_overlay_del(route_start);
   if (route_end)      elm_map_overlay_del(route_end);
   if (route_clas)     elm_map_overlay_del(route_clas);
   if (bubble_img)     elm_map_overlay_del(bubble_img);
   if (bubble_parking) elm_map_overlay_del(bubble_parking);
   if (route_ovl)      elm_map_overlay_del(route_ovl);
   route_start = NULL;
   route_end = NULL;
   route_clas = NULL;
   bubble_img = NULL;
   bubble_parking = NULL;
   route_ovl = NULL;

   if (route) elm_map_route_del(route);
   if (name) elm_map_name_del(name);
   if (menu)  evas_object_del(menu);
   route = NULL;
   name = NULL;
   menu = NULL;
}

void
test_map(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *map;
   int idx = 0;
   const char **tile_srcs;
   const char **route_srcs;
   const char **name_srcs;

   win = elm_win_util_standard_add("map", "Map");
   elm_win_autodel_set(win, EINA_TRUE);

   map = elm_map_add(win);
   if (map)
     {
        Elm_Map_Overlay *ovl_1, *ovl_2, *ovl_3, *ovl_4, *ovl_5, *ovl_6;
        Elm_Map_Overlay *ovl_7, *ovl_8, *ovl_9, *ovl_10, *ovl_11;
        Elm_Map_Overlay *parking1, *parking2, *parking3, *parking4, *parking5;
        Elm_Map_Overlay *grp1, *grp2, *grp_parking;

        evas_object_event_callback_add(map, EVAS_CALLBACK_DEL, _del_map, NULL);

        srand(time(NULL));

        tile_srcs = elm_map_sources_get(map, ELM_MAP_SOURCE_TYPE_TILE);
        route_srcs = elm_map_sources_get(map, ELM_MAP_SOURCE_TYPE_ROUTE);
        name_srcs = elm_map_sources_get(map, ELM_MAP_SOURCE_TYPE_NAME);

        if (!tile_srcs) return;
        printf("Tile sources [ ");
        for (idx = 0; tile_srcs[idx] ; idx++) printf("%s, ", tile_srcs[idx]);
        printf("]\n");
        if (!route_srcs) return;
        printf("Route sources [ ");
        for (idx = 0; route_srcs[idx] ; idx++) printf("%s, ", route_srcs[idx]);
        printf("]\n");
        if (!name_srcs) return;
        printf("Name sources [ ");
        for (idx = 0; name_srcs[idx] ; idx++) printf("%s, ", name_srcs[idx]);
        printf("]\n");

        evas_object_size_hint_weight_set(map, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_win_resize_object_add(win, map);
        evas_object_data_set(map, "window", win);

        evas_object_event_callback_add(map, EVAS_CALLBACK_MOUSE_DOWN,
                                       _map_mouse_down, map);
        evas_object_event_callback_add(map, EVAS_CALLBACK_MOUSE_MOVE,
                                       _map_mouse_move, map);
        evas_object_event_callback_add(map, EVAS_CALLBACK_MOUSE_UP,
                                       _map_mouse_up, map);

        evas_object_smart_callback_add(map, "clicked", _map_clicked, map);
        evas_object_smart_callback_add(map, "clicked,double", _map_clicked_double, map);
        evas_object_smart_callback_add(map, "press", _map_press, map);
        evas_object_smart_callback_add(map, "longpressed", _map_longpressed, map);
        evas_object_smart_callback_add(map, "scroll", _map_scroll, map);
        evas_object_smart_callback_add(map, "scroll,drag,start", _map_drag_start, map);
        evas_object_smart_callback_add(map, "scroll,drag,stop", _map_drag_stop, map);
        evas_object_smart_callback_add(map, "scroll,anim,start", _map_anim_start, map);
        evas_object_smart_callback_add(map, "scroll,anim,stop", _map_anim_stop, map);
        evas_object_smart_callback_add(map, "zoom,start", _map_zoom_start, map);
        evas_object_smart_callback_add(map, "zoom,stop", _map_zoom_stop, map);
        evas_object_smart_callback_add(map, "zoom,change", _map_zoom_change, map);
        evas_object_smart_callback_add(map, "loaded", _map_loaded, map);
        evas_object_smart_callback_add(map, "tile,load", _map_tile_load, map);
        evas_object_smart_callback_add(map, "tile,loaded", _map_tile_loaded, map);
        evas_object_smart_callback_add(map, "tile,loaded,fail", _map_tile_loaded_fail, map);
        evas_object_smart_callback_add(map, "route,load", _map_route_load, map);
        evas_object_smart_callback_add(map, "route,loaded", _map_route_loaded, map);
        evas_object_smart_callback_add(map, "route,loaded,fail", _map_route_loaded_fail, map);
        evas_object_smart_callback_add(map, "name,load", _map_name_load, map);
        evas_object_smart_callback_add(map, "name,loaded", _map_name_loaded, map);
        evas_object_smart_callback_add(map, "name,loaded,fail", _map_name_loaded_fail, map);
        evas_object_smart_callback_add(map, "overlay,clicked", _overlay_cb, map);

        // Create Overlays
        ovl_1 = elm_map_overlay_add(map, 2.352, 48.857);
        elm_map_overlay_color_set(ovl_1, 0x00, 0xfa, 0x9a, 0xff);
        elm_map_overlay_displayed_zoom_min_set(ovl_1, 5);
        ovl_2 = elm_map_overlay_add(map, 3, 48.857);
        elm_map_overlay_color_set(ovl_2, 0xff, 0xd7, 0x00, 0xff);
        elm_map_overlay_displayed_zoom_min_set(ovl_2, 4);
        ovl_3 = elm_map_overlay_add(map, 2.352, 49);
        elm_map_overlay_displayed_zoom_min_set(ovl_3, 3);
        ovl_4 = elm_map_overlay_add(map, 7.31451, 48.857127);
        ovl_5 = elm_map_overlay_add(map, 7.314704, 48.857119);
        ovl_6 = elm_map_overlay_add(map, 7.31432, 48.856785);
        ovl_7 = elm_map_overlay_add(map, 7.3148, 48.85725);
        ovl_8 = elm_map_overlay_add(map, 7.316445, 48.8572210000694);
        ovl_9 = elm_map_overlay_add(map, 7.316527000125, 48.85609);
        ovl_10 = elm_map_overlay_add(map, 7.3165409990833, 48.856078);
        ovl_11 = elm_map_overlay_add(map, 7.319812, 48.856561);
        elm_map_overlay_data_set(ovl_1, &data1);
        elm_map_overlay_data_set(ovl_2, &data2);
        elm_map_overlay_data_set(ovl_3, &data3);
        elm_map_overlay_data_set(ovl_4, &data4);
        elm_map_overlay_data_set(ovl_5, &data5);
        elm_map_overlay_data_set(ovl_6, &data6);
        elm_map_overlay_data_set(ovl_7, &data7);
        elm_map_overlay_data_set(ovl_8, &data8);
        elm_map_overlay_data_set(ovl_9, &data9);
        elm_map_overlay_data_set(ovl_10, &data10);
        elm_map_overlay_data_set(ovl_11, &data11);

        // Append overlays to groups
        grp1 = elm_map_overlay_class_add(map);
        elm_map_overlay_class_zoom_max_set(grp1, 6);
        elm_map_overlay_class_append(grp1, ovl_1);
        elm_map_overlay_class_append(grp1, ovl_2);
        elm_map_overlay_class_append(grp1, ovl_3);
        elm_map_overlay_class_append(grp1, ovl_4);
        elm_map_overlay_class_append(grp1, ovl_5);
        elm_map_overlay_class_append(grp1, ovl_6);

        // Append overlays to groups
        grp2 = elm_map_overlay_class_add(map);
        elm_map_overlay_displayed_zoom_min_set(grp2, 9);
        elm_map_overlay_class_append(grp2, ovl_7);
        elm_map_overlay_class_append(grp2, ovl_8);
        elm_map_overlay_class_append(grp2, ovl_9);
        elm_map_overlay_class_append(grp2, ovl_10);
        elm_map_overlay_class_append(grp2, ovl_11);

        // Create overlays
        parking1 = elm_map_overlay_add(map, 127.04871, 37.25730);
        parking2 = elm_map_overlay_add(map, 127.05578, 37.25545);
        parking3 = elm_map_overlay_add(map, 127.05515, 37.25439);
        parking4 = elm_map_overlay_add(map, 127.05328, 37.25721);
        elm_map_overlay_icon_set(parking4, _icon_get(map, &icon_data));
        parking5 = elm_map_overlay_add(map, 127.05431, 37.25873);
        elm_map_overlay_content_set(parking5, _label_get(map));
        elm_map_overlay_get_cb_set(parking1, _parking_cb, NULL);
        elm_map_overlay_get_cb_set(parking2, _parking_cb, NULL);
        elm_map_overlay_get_cb_set(parking3, _parking_cb, NULL);
        elm_map_overlay_get_cb_set(parking4, _parking_cb, NULL);
        elm_map_overlay_get_cb_set(parking5, _parking_cb, NULL);

        // Append overlays to groups
        grp_parking = elm_map_overlay_class_add(map);
        elm_map_overlay_icon_set(grp_parking, _icon_get(map, &parking));
        elm_map_overlay_get_cb_set(grp_parking, _parking_cb, NULL);
        elm_map_overlay_class_append(grp_parking, parking1);
        elm_map_overlay_class_append(grp_parking, parking2);
        elm_map_overlay_class_append(grp_parking, parking3);
        elm_map_overlay_class_append(grp_parking, parking4);
        elm_map_overlay_class_append(grp_parking, parking5);

        evas_object_show(map);
     }

   evas_object_resize(win, 800, 800);
   evas_object_show(win);
}

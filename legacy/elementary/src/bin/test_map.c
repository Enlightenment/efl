#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

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

static Evas_Object *menu, *fs_win;
static Elm_Map_Route *route;
static Elm_Map_Name *name;
static Evas_Object *track;
static const char **source_names = NULL;
static Evas_Coord old_x, old_y;
static Evas_Coord old_d;
static Map_Source ms[SOURCE_MAX];

static void
#ifdef ELM_EMAP
my_map_gpx_fileselector_done(void *data, Evas_Object *obj __UNUSED__, void *event_info)
#else
my_map_gpx_fileselector_done(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
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
   elm_icon_file_set(icon, PACKAGE_DATA_DIR"/images/bubble.png", NULL);
   evas_object_show(icon);

   return icon;
}

static void
_map_clicked(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("clicked\n");
}

static void
_map_clicked_double(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   printf("clicked,double\n");
   double lon, lat;
   Evas_Event_Mouse_Down *down = (Evas_Event_Mouse_Down *)event_info;
   if (!down) return;
   if (elm_map_zoom_get(obj) < 5) return;

   elm_map_canvas_to_region_convert(obj, down->canvas.x, down->canvas.y, &lon, &lat);
   printf("x:%d, y:%d, lon:%lf, lat:%lf\n", down->canvas.x, down->canvas.y, lon, lat);

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
_map_press(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("press\n");
}

static void
_map_longpressed(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   if (!event_info) return;
   double lon, lat;
   Evas_Event_Mouse_Down *down = (Evas_Event_Mouse_Down *)event_info;
   elm_map_canvas_to_region_convert(obj, down->canvas.x, down->canvas.y, &lon, &lat);
   printf("longpressed, x:%d, y:%d, lon:%lf, lat:%lf\n", down->canvas.x, down->canvas.y, lon, lat);

   if (elm_map_zoom_get(obj) < 8) return;
   if (name) elm_map_name_del(name);
   name = elm_map_name_add(obj, NULL, lon, lat, NULL, NULL);
}

static void
_map_scroll(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   double lon, lat;
   elm_map_region_get(obj, &lon, &lat);
   printf("scroll, longitude: %f latitude: %f\n", lon, lat);
}

static void
_map_drag_start(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("scroll,drag,start\n");
   evas_object_smart_callback_del(data, "longpressed", _map_longpressed);
}

static void
_map_drag_stop(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("scroll,drag,stop\n");
   evas_object_smart_callback_add(data, "longpressed", _map_longpressed, data);
}

static void
_map_anim_start(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("scroll,anim,start\n");
}

static void
_map_anim_stop(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("scroll,anim,stop\n");
}

static void
_map_zoom_start(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("zoom,start\n");
}

static void
_map_zoom_stop(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("zoom,stop\n");
}

static void
_map_zoom_change(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("zoom,change\n");
}

static void
_map_tile_load(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("tile,load\n");
}

static void
_map_tile_loaded(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   int try_num, finish_num;
   elm_map_tile_load_status_get(data, &try_num, &finish_num);
   printf("tile,loaded: %d / %d\n", finish_num, try_num);
}

static void
_map_tile_loaded_fail(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   int try_num, finish_num;
   elm_map_tile_load_status_get(data, &try_num, &finish_num);
   printf("tile,loaded,fail: %d / %d\n", finish_num, try_num);
}

static void
_map_route_load(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("route,load\n");
}

static void
_map_route_loaded(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
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
   elm_map_overlay_color_set(route_ovl, 255, 0, 0, 255);
}

static void
_map_route_loaded_fail(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("route,loaded,fail\n");
}

static void
_map_name_load(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("name,load\n");
}

static void
_map_name_loaded(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
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
_map_name_loaded_fail(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("name,loaded,fail\n");
}

static void
map_show_urmatt(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Eina_Bool b = elm_map_paused_get(data);
   elm_map_paused_set(data, EINA_TRUE);
   elm_map_zoom_mode_set(data, ELM_MAP_ZOOM_MODE_MANUAL);
   elm_map_region_show(data,7.325201, 48.526813);
   elm_map_zoom_set(data, 12);
   elm_map_paused_set(data, b);
}

static void
map_bring_seoul(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_map_region_bring_in(data, 126.977969, 37.566535);
}

static void
map_paused_set(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_map_paused_set(data, EINA_TRUE);
}

static void
map_paused_unset(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_map_paused_set(data, EINA_FALSE);
}

static void
map_zoom_in(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   int zoom;

   zoom = elm_map_zoom_get(data) + 1;
   elm_map_zoom_mode_set(data, ELM_MAP_ZOOM_MODE_MANUAL);
   elm_map_zoom_set(data, zoom);
}

static void
map_zoom_out(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   int zoom;

   zoom = elm_map_zoom_get(data) - 1;
   elm_map_zoom_mode_set(data, ELM_MAP_ZOOM_MODE_MANUAL);
   elm_map_zoom_set(data, zoom);
}

static void
map_zoom_fit(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_map_zoom_mode_set(data, ELM_MAP_ZOOM_MODE_AUTO_FIT);
}

static void
map_zoom_fill(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_map_zoom_mode_set(data, ELM_MAP_ZOOM_MODE_AUTO_FILL);
}

static void
map_track_add(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *fs, *bg, *vbox, *hbox, *sep;
   char *path = NULL;

   fs_win = elm_win_add(NULL, "fileselector", ELM_WIN_BASIC);
   elm_win_title_set(fs_win, "File Selector");
   elm_win_autodel_set(fs_win, 1);

   bg = elm_bg_add(fs_win);
   elm_win_resize_object_add(fs_win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   vbox = elm_box_add(fs_win);
   elm_win_resize_object_add(fs_win, vbox);
   evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
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
map_track_remove(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_map_track_remove(data, track);
}

static void
map_rotate_cw(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
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
map_rotate_ccw(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
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
map_rotate_reset(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Coord x, y, w, h;
   float half_w, half_h;
   evas_object_geometry_get(data, &x, &y, &w, &h);
   half_w = (float)w * 0.5;
   half_h = (float)h * 0.5;

   elm_map_rotate_set(data, 0.0, x + half_w, y + half_h);
}

static void
map_zoom_min_set(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_map_zoom_min_set(data, 1);
}

static void
map_zoom_max_set(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_map_zoom_max_set(data, 10);
}

static void
map_source(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Map_Source *s = data;

   if (!s) return;
   elm_map_source_set(s->map, ELM_MAP_SOURCE_TYPE_TILE, s->source_name);
}

/*
static void
map_marker_add(void *data)
{
   int i;
   Elm_Map_Group_Class *g_clas;
   Elm_Map_Marker_Class *m_clas;
   Overlay_Data *d = &data7;

   if (*markers) return;
   for (i =0; i<MARKER_MAX; i++)
     {
        d = &data7;

        int r1 = rand() % (180*2*100);
        if (r1<=180) r1 = -r1;
        else r1 = r1 - 180*100;

        int r2 = rand() % (85*2*100);
        if (r2<=85) r2 = -r2;
        else r2 = r2 - 85*100;

        int style = rand() % 3;
        if (!style) m_clas = itc1;
        else if (style == 1) m_clas = itc2;
        else
          {
             m_clas = itc_parking;
             d = &parking;
          }

        style = rand() % 2;
        if (!style) g_clas = itc_group1;
        else g_clas = itc_group2;

        markers[i] = elm_map_marker_add(data, r1/100., r2/100., m_clas, g_clas, d);
     }
   nb_elts += 1000;
   printf("nb elements: %d\n", nb_elts);
}

static void
map_marker_remove(void *data __UNUSED__)
{
   int i;

   for (i = 0; i<MARKER_MAX; i++)
     {
        if (markers[i])
          {
             elm_map_marker_remove(markers[i]);
             markers[i] = NULL;
          }
     }
}

static void
my_map_entry_focused(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   const char *s = elm_scrolled_entry_entry_get(obj);
   if (!strcmp(s, NAME_ENTRY_TEXT)) elm_scrolled_entry_entry_set(obj, "");
}

static void
my_map_entry_activated(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   const char *s = elm_scrolled_entry_entry_get(obj);
   char *addr = strdup(s);
   name = elm_map_utils_convert_name_into_coord(data, addr);
   if (addr) free (addr);
}
*/

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
   elm_object_text_set(label, "Wolves go!!");
   evas_object_show(label);
   elm_box_pack_end(bx, label);
   return bx;
}

static Evas_Object *
_icon_get(Evas_Object *obj, Overlay_Data *data)
{
   Evas_Object *icon = elm_icon_add(obj);
   elm_icon_file_set(icon, data->file, NULL);
   evas_object_show(icon);

   return icon;
}

static void
_populate(void *data, Elm_Object_Item *menu_it)
{
   int idx;

   if ((!data) || (!menu_it) || (!source_names)) return;
   for (idx = 0; source_names[idx]; idx++)
     {
        if (idx >= SOURCE_MAX) break;
        ms[idx].map = data;
        ms[idx].source_name = strdup(source_names[idx]);
        elm_menu_item_add(menu, menu_it, "", source_names[idx], map_source,
                          &ms[idx]);
     }
}

static void
_map_mouse_down(void *data, Evas *evas __UNUSED__, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *down = event_info;
   Elm_Object_Item *menu_it;
   if (!down) return;

   if (down->button == 2)
     {
        old_x = down->canvas.x;
        old_y = down->canvas.y;
        old_d = 0.0;
     }
   else if (down->button == 3)
     {
        menu = elm_menu_add(obj);
        menu_it = elm_menu_item_add(menu, NULL, NULL, "Source", NULL, NULL);
        _populate(data, menu_it);
        elm_menu_item_add(menu, NULL, NULL, "Show Urmatt", map_show_urmatt, data);
        elm_menu_item_add(menu, NULL, NULL, "Bring Seoul", map_bring_seoul, data);
        elm_menu_item_add(menu, NULL, NULL, "Paused Set", map_paused_set, data);
        elm_menu_item_add(menu, NULL, NULL, "Paused Unset", map_paused_unset, data);
        elm_menu_item_add(menu, NULL, NULL, "Zoom +", map_zoom_in, data);
        elm_menu_item_add(menu, NULL, NULL, "Zoom -", map_zoom_out, data);
        elm_menu_item_add(menu, NULL, NULL, "Zoom Fit", map_zoom_fit, data);
        elm_menu_item_add(menu, NULL, NULL, "Zoom Fill", map_zoom_fill, data);
        elm_menu_item_add(menu, NULL, NULL, "Add Track", map_track_add, data);
        elm_menu_item_add(menu, NULL, NULL, "Remove Track", map_track_remove, data);
        elm_menu_item_add(menu, NULL, NULL, "Add Marker", NULL, NULL);
        elm_menu_item_add(menu, NULL, NULL, "Rotate CW", map_rotate_cw, data);
        elm_menu_item_add(menu, NULL, NULL, "Rotate CCW", map_rotate_ccw, data);
        elm_menu_item_add(menu, NULL, NULL, "Reset Rotate", map_rotate_reset, data);
        elm_menu_item_add(menu, NULL, NULL, "Set Zoom Min to 1", map_zoom_min_set, data);
        elm_menu_item_add(menu, NULL, NULL, "Set Zoom Max to 10", map_zoom_max_set, data);

        elm_menu_move(menu, down->canvas.x, down->canvas.y);
        evas_object_show(menu);
     }
}

static void
_map_mouse_move(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
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
_map_mouse_up(void *data __UNUSED__, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
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
_overlay_cb(void *data __UNUSED__, Evas_Object *map, void *ev)
{
   printf("Overlay clicked: ");
   Elm_Map_Overlay *overlay = ev;
   Evas_Object *bx;

   Overlay_Data *od = elm_map_overlay_data_get(overlay);
   if (!od)
     {
        printf("No overlay data\n");
     }
   else if (elm_map_overlay_type_get(overlay) == ELM_MAP_OVERLAY_TYPE_DEFAULT)
     {
        // prevent duplication
        if (!bubble_img)  bubble_img = elm_map_overlay_bubble_add(map);

        elm_map_overlay_bubble_follow(bubble_img, overlay);
        bx = _box_get(map, od);
        elm_map_overlay_bubble_content_clear(bubble_img);
        elm_map_overlay_bubble_content_append(bubble_img, bx);
        printf("overlay rendered\n");
     }
   else printf("not default type\n");
}

static void
_parking_cb(void *data __UNUSED__, Evas_Object *map, const Elm_Map_Overlay *ovl)
{
   if (elm_map_overlay_type_get(ovl) != ELM_MAP_OVERLAY_TYPE_DEFAULT) return;

   double lon, lat;
   Evas_Coord x, y;
   elm_map_overlay_region_get(ovl, &lon, &lat);
   elm_map_region_to_canvas_convert(map, lon, lat, &x, &y);
   printf("Parking clicked: %lf %lf %d %d\n", lon, lat, x, y);

   if (!bubble_parking)
     {
        Evas_Object *bubble, *label;
        bubble = elm_bubble_add(map);
        elm_bubble_pos_set(bubble, ELM_BUBBLE_POS_BOTTOM_LEFT);
        elm_object_text_set(bubble, "Overlay object");
        elm_object_part_text_set(bubble, "info", "Bubble is overlayed");

        label = elm_label_add(bubble);
        elm_object_text_set(label, "Parking Here !!");
        evas_object_show(label);
        elm_object_content_set(bubble, label);

        evas_object_resize(bubble, 125, 50);
        evas_object_show(bubble);

        bubble_parking = elm_map_overlay_add(map, lon + 0.0006, lat + 0.0006);
        elm_map_overlay_content_set(bubble_parking, bubble);
     }
   elm_map_overlay_region_set(bubble_parking, lon + 0.0006, lat + 0.0006);
}

static void
_del_map(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *ei __UNUSED__)
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
   route = NULL;
   name = NULL;
}

void
test_map(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *map;
   int idx = 0;

   win = elm_win_add(NULL, "map", ELM_WIN_BASIC);
   elm_win_title_set(win, "Map");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   map = elm_map_add(win);
   if (map)
     {
        Elm_Map_Overlay *ovl_1, *ovl_2, *ovl_3, *ovl_4, *ovl_5, *ovl6;
        Elm_Map_Overlay *ovl_7, *ovl_8, *ovl_9, *ovl_10, *ovl_11;
        Elm_Map_Overlay *parking1, *parking2, *parking3, *parking4, *parking5;
        Elm_Map_Overlay *grp1, *grp2, *grp_parking;

        evas_object_event_callback_add(map, EVAS_CALLBACK_DEL, _del_map, NULL);

        srand(time(NULL));

        source_names = elm_map_sources_get(map, ELM_MAP_SOURCE_TYPE_TILE);

        if (!source_names) return;
        printf("map sources [ ");
        for (idx = 0; source_names[idx] ; idx++) printf("%s, ", source_names[idx]);
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
        ovl_2 = elm_map_overlay_add(map, 3, 48.857);
        ovl_3 = elm_map_overlay_add(map, 2.352, 49);
        ovl_4 = elm_map_overlay_add(map, 7.31451, 48.857127);
        ovl_5 = elm_map_overlay_add(map, 7.314704, 48.857119);
        ovl6 = elm_map_overlay_add(map, 7.31432, 48.856785);
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
        elm_map_overlay_data_set(ovl6, &data6);
        elm_map_overlay_data_set(ovl_7, &data7);
        elm_map_overlay_data_set(ovl_8, &data8);
        elm_map_overlay_data_set(ovl_9, &data9);
        elm_map_overlay_data_set(ovl_10, &data10);
        elm_map_overlay_data_set(ovl_11, &data11);

        // Append overlays to groups
        grp1 = elm_map_overlay_class_add(map);
        elm_map_overlay_displayed_zoom_min_set(grp1, 3);
        elm_map_overlay_class_append(grp1, ovl_1);
        elm_map_overlay_class_append(grp1, ovl_2);
        elm_map_overlay_class_append(grp1, ovl_3);
        elm_map_overlay_class_append(grp1, ovl_4);
        elm_map_overlay_class_append(grp1, ovl_5);
        elm_map_overlay_class_append(grp1, ovl6);
        evas_object_smart_callback_add(map, "overlay,clicked", _overlay_cb, map);

        // Append overlays to groups
        grp2 = elm_map_overlay_class_add(map);
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
        parking5 = elm_map_overlay_add(map, 127.05431, 37.25873);
        elm_map_overlay_icon_set(parking4, _icon_get(map, &icon_data));
        elm_map_overlay_get_cb_set(parking1, _parking_cb, NULL);
        elm_map_overlay_get_cb_set(parking2, _parking_cb, NULL);
        elm_map_overlay_get_cb_set(parking3, _parking_cb, NULL);
        elm_map_overlay_get_cb_set(parking4, _parking_cb, NULL);
        elm_map_overlay_get_cb_set(parking5, _parking_cb, NULL);

        // Append overlays to groups
        grp_parking = elm_map_overlay_class_add(map);
        elm_map_overlay_icon_set(grp_parking, _icon_get(map, &parking));
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
#endif

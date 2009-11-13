#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

static void
my_map_clicked(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("clicked\n");
}

static void
my_map_press(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("press\n");
}

static void
my_map_longpressed(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("longpressed\n");
}

static void
my_map_clicked_double(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("clicked,double\n");
}

static void
my_map_load(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("load\n");
}

static void
my_map_loaded(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("loaded\n");
}

static void
my_map_load_details(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("load,details\n");
}

static void
my_map_loaded_details(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("loaded,details\n");
}

static void
my_map_zoom_start(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("zoom,start\n");
}

static void
my_map_zoom_stop(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("zoom,stop\n");
}

static void
my_map_zoom_change(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("zoom,change\n");
}

static void
my_map_anim_start(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("anim,start\n");
}

static void
my_map_anim_stop(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("anim,stop\n");
}

static void
my_map_drag_start(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("drag,start\n");
}

static void
my_map_drag_stop(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("drag_stop\n");
}

static void
my_map_scroll(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   double lon, lat;
   elm_map_geo_region_get(obj, &lat, &lon);
   printf("scroll latitude : %f longitude : %f\n", lat, lon);
}

static void
sel_done(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *map, *iw;

   map = data;
   iw = evas_object_data_get(map, "inwin");
   evas_object_del(iw);
}

static void
my_bt_show_reg(void *data, Evas_Object *obj, void *event_info)
{
   elm_map_paused_set(data, EINA_TRUE);
   elm_map_zoom_set(data, 18);
   elm_map_geo_region_show(data, 48.857, 2.352);
}

static void
my_bt_bring_reg(void *data, Evas_Object *obj, void *event_info)
{
   elm_map_geo_region_bring_in(data, 48.857, 2.352);
}

static void
my_bt_zoom_in(void *data, Evas_Object *obj, void *event_info)
{
   double zoom;

   zoom = elm_map_zoom_get(data);
   zoom += 1;
   elm_map_zoom_mode_set(data, ELM_MAP_ZOOM_MODE_MANUAL);
   if (zoom >= (1.0 / 32.0)) elm_map_zoom_set(data, zoom);
}

static void
my_bt_zoom_out(void *data, Evas_Object *obj, void *event_info)
{
   double zoom;

   zoom = elm_map_zoom_get(data);
   zoom -= 1;
   elm_map_zoom_mode_set(data, ELM_MAP_ZOOM_MODE_MANUAL);
   if (zoom <= 256.0) elm_map_zoom_set(data, zoom);
}

static void
my_bt_pause(void *data, Evas_Object *obj, void *event_info)
{
   elm_map_paused_set(data, !elm_map_paused_get(data));
}

static void
my_bt_zoom_fit(void *data, Evas_Object *obj, void *event_info)
{
   elm_map_zoom_mode_set(data, ELM_MAP_ZOOM_MODE_AUTO_FIT);
}

static void
my_bt_zoom_fill(void *data, Evas_Object *obj, void *event_info)
{
   elm_map_zoom_mode_set(data, ELM_MAP_ZOOM_MODE_AUTO_FILL);
}

void
test_map(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *map, *tb2, *bt;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "map", ELM_WIN_BASIC);
   elm_win_title_set(win, "Map");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   map = elm_map_add(win);
   evas_object_size_hint_weight_set(map, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, map);
   evas_object_data_set(map, "window", win);

   evas_object_smart_callback_add(map, "clicked", my_map_clicked, win);
   evas_object_smart_callback_add(map, "press", my_map_press, win);
   evas_object_smart_callback_add(map, "longpressed", my_map_longpressed, win);
   evas_object_smart_callback_add(map, "clicked,double", my_map_clicked_double, win);
   evas_object_smart_callback_add(map, "load", my_map_load, win);
   evas_object_smart_callback_add(map, "loaded", my_map_loaded, win);
   evas_object_smart_callback_add(map, "load,details", my_map_load_details, win);
   evas_object_smart_callback_add(map, "loaded,details", my_map_loaded_details, win);
   evas_object_smart_callback_add(map, "zoom,start", my_map_zoom_start, win);
   evas_object_smart_callback_add(map, "zoom,stop", my_map_zoom_stop, win);
   evas_object_smart_callback_add(map, "zoom,change", my_map_zoom_change, win);
   evas_object_smart_callback_add(map, "scroll,anim,start", my_map_anim_start, win);
   evas_object_smart_callback_add(map, "scroll,anim,stop", my_map_anim_stop, win);
   evas_object_smart_callback_add(map, "scroll,drag,start", my_map_drag_start, win);
   evas_object_smart_callback_add(map, "scroll,drag,stop", my_map_drag_stop, win);
   evas_object_smart_callback_add(map, "scroll", my_map_scroll, win);

   evas_object_show(map);

   tb2 = elm_table_add(win);
   evas_object_size_hint_weight_set(tb2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, tb2);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Z -");
   evas_object_smart_callback_add(bt, "clicked", my_bt_zoom_out, map);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.1, 0.1);
   elm_table_pack(tb2, bt, 0, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Z +");
   evas_object_smart_callback_add(bt, "clicked", my_bt_zoom_in, map);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.1);
   elm_table_pack(tb2, bt, 2, 0, 1, 1);
   evas_object_show(bt);


   bt = elm_button_add(win);
   elm_button_label_set(bt, "Show Paris");
   evas_object_smart_callback_add(bt, "clicked", my_bt_show_reg, map);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.1, 0.5);
   elm_table_pack(tb2, bt, 0, 1, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Bring Paris");
   evas_object_smart_callback_add(bt, "clicked", my_bt_bring_reg, map);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.5);
   elm_table_pack(tb2, bt, 2, 1, 1, 1);
   evas_object_show(bt);


   bt = elm_button_add(win);
   elm_button_label_set(bt, "Pause On/Off");
   evas_object_smart_callback_add(bt, "clicked", my_bt_pause, map);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.1, 0.9);
   elm_table_pack(tb2, bt, 0, 2, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Fit");
   evas_object_smart_callback_add(bt, "clicked", my_bt_zoom_fit, map);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.5, 0.9);
   elm_table_pack(tb2, bt, 1, 2, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Fill");
   evas_object_smart_callback_add(bt, "clicked", my_bt_zoom_fill, map);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.9);
   elm_table_pack(tb2, bt, 2, 2, 1, 1);
   evas_object_show(bt);

   evas_object_show(tb2);

   evas_object_resize(win, 800, 800);
   evas_object_show(win);
}
#endif

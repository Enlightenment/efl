#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

typedef struct Marker_Data
{
    const char *file;
} Marker_Data;


Elm_Map_Marker_Class itc;

static Evas_Object *rect;

Marker_Data data1 = {PACKAGE_DATA_DIR"/images/logo.png"};
Marker_Data data2 = {PACKAGE_DATA_DIR"/images/logo_small.png"};
Marker_Data data3 = {PACKAGE_DATA_DIR"/images/panel_01.jpg"};
Marker_Data data4 = {PACKAGE_DATA_DIR"/images/plant_01.jpg"};
Marker_Data data5 = {PACKAGE_DATA_DIR"/images/rock_01.jpg"};
Marker_Data data6 = {PACKAGE_DATA_DIR"/images/rock_02.jpg"};
Marker_Data data7 = {PACKAGE_DATA_DIR"/images/sky_01.jpg"};
Marker_Data data8 = {PACKAGE_DATA_DIR"/images/sky_02.jpg"};
Marker_Data data9 = {PACKAGE_DATA_DIR"/images/sky_03.jpg"};
Marker_Data data10 = {PACKAGE_DATA_DIR"/images/sky_03.jpg"};
Marker_Data data11= {PACKAGE_DATA_DIR"/images/wood_01.jpg"};

static void
my_map_clicked(void *data, Evas_Object *obj, void *event_info)
{
   //Evas_Object *win = data;
   printf("clicked\n");
}

static void
my_map_press(void *data, Evas_Object *obj, void *event_info)
{
   //Evas_Object *win = data;
   printf("press\n");
}

static void
my_map_longpressed(void *data, Evas_Object *obj, void *event_info)
{
   //Evas_Object *win = data;
   printf("longpressed\n");
}

static void
my_map_clicked_double(void *data, Evas_Object *obj, void *event_info)
{
   //Evas_Object *win = data;
   printf("clicked,double\n");
}

static void
my_map_load(void *data, Evas_Object *obj, void *event_info)
{
   //Evas_Object *win = data;
   printf("load\n");
}

static void
my_map_loaded(void *data, Evas_Object *obj, void *event_info)
{
   //Evas_Object *win = data;
   printf("loaded\n");
}

static void
my_map_load_details(void *data, Evas_Object *obj, void *event_info)
{
   //Evas_Object *win = data;
   printf("load,details\n");
}

static void
my_map_loaded_details(void *data, Evas_Object *obj, void *event_info)
{
   //Evas_Object *win = data;
   printf("loaded,details\n");
}

static void
my_map_zoom_start(void *data, Evas_Object *obj, void *event_info)
{
   //Evas_Object *win = data;
   printf("zoom,start\n");
}

static void
my_map_zoom_stop(void *data, Evas_Object *obj, void *event_info)
{
   //Evas_Object *win = data;
   printf("zoom,stop\n");
}

static void
my_map_zoom_change(void *data, Evas_Object *obj, void *event_info)
{
   //Evas_Object *win = data;
   printf("zoom,change\n");
}

static void
my_map_anim_start(void *data, Evas_Object *obj, void *event_info)
{
   //Evas_Object *win = data;
   printf("anim,start\n");
}

static void
my_map_anim_stop(void *data, Evas_Object *obj, void *event_info)
{
   //Evas_Object *win = data;
   printf("anim,stop\n");
}

static void
my_map_drag_start(void *data, Evas_Object *obj, void *event_info)
{
   //Evas_Object *win = data;
   printf("drag,start\n");
}

static void
my_map_drag_stop(void *data, Evas_Object *obj, void *event_info)
{
   //Evas_Object *win = data;
   printf("drag_stop\n");
}

static void
my_map_scroll(void *data, Evas_Object *obj, void *event_info)
{
   //Evas_Object *win = data;
   double lon, lat;
   elm_map_geo_region_get(obj, &lon, &lat);
   printf("scroll longitude : %f latitude : %f\n", lon, lat);
}

static void
my_bt_show_reg(void *data, Evas_Object *obj, void *event_info)
{
   Eina_Bool b = elm_map_paused_get(data);
   elm_map_paused_set(data, EINA_TRUE);
   elm_map_zoom_mode_set(data, ELM_MAP_ZOOM_MODE_MANUAL);
   elm_map_geo_region_show(data, 2.352, 48.857);
   elm_map_zoom_set(data, 18);
   elm_map_paused_set(data, b);
}

static void
my_bt_bring_reg(void *data, Evas_Object *obj, void *event_info)
{
   elm_map_geo_region_bring_in(data, 2.352, 48.857);
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

static Evas_Object *
_marker_get(Evas_Object *obj, Elm_Map_Marker *marker, void *data)
{
    Marker_Data *d = data;

    Evas_Object *bx = elm_box_add(obj);
    evas_object_show(bx);

    if(d == &data3)
    {
        Evas_Object *icon = elm_icon_add(obj);
        elm_icon_file_set(icon, d->file, NULL);
        evas_object_show(icon);

        Evas_Object *o = elm_button_add(obj);
        elm_button_icon_set(o, icon);
        evas_object_show(o);
        elm_box_pack_end(bx, o);
    }
    else
    {
        Evas_Object *o = evas_object_image_add(evas_object_evas_get(obj));
        evas_object_image_file_set(o, d->file, NULL);
        evas_object_image_filled_set(o, EINA_TRUE);
        evas_object_size_hint_min_set(o, 64, 64);
        evas_object_show(o);
        elm_box_pack_end(bx, o);

        Evas_Object *lbl = elm_label_add(obj);
        elm_label_label_set(lbl, "Wolves Go !");
        evas_object_show(lbl);
        elm_box_pack_end(bx, lbl);
    }

    return bx;
}

static void
_map_mouse_wheel_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Object *map = data;
   Evas_Event_Mouse_Wheel *ev = (Evas_Event_Mouse_Wheel*) event_info;
   int zoom;
   //unset the mouse wheel
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

   zoom = elm_map_zoom_get(map);

   if (ev->z > 0)
     zoom++;
   else
     zoom--;

   elm_map_zoom_mode_set(map, ELM_MAP_ZOOM_MODE_MANUAL);
   if (zoom >= 0 && zoom <= 18) elm_map_zoom_set(map, zoom);
}

static void 
_map_move_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   int x,y,w,h;

   evas_object_geometry_get(data,&x,&y,&w,&h);
   evas_object_resize(rect,w,h);
   evas_object_move(rect,x,y);
}

void
test_map(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *map, *tb2, *bt;

   win = elm_win_add(NULL, "map", ELM_WIN_BASIC);
   elm_win_title_set(win, "Map");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   map = elm_map_add(win);
   if (map) 
     {
        evas_object_size_hint_weight_set(map, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_win_resize_object_add(win, map);
        evas_object_data_set(map, "window", win);

        itc.func.get = _marker_get;
        itc.func.del = NULL;

        rect = evas_object_rectangle_add(evas_object_evas_get(win));
        evas_object_color_set(rect, 0, 0, 0, 0);
        evas_object_repeat_events_set(rect,1);
        evas_object_show(rect);
        evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_WHEEL, 
                                       _map_mouse_wheel_cb, map);
        evas_object_raise(rect);

        evas_object_event_callback_add(map, EVAS_CALLBACK_RESIZE, 
                                       _map_move_resize_cb, map);
        evas_object_event_callback_add(map, EVAS_CALLBACK_MOVE, 
                                       _map_move_resize_cb, map);

        Elm_Map_Marker *marker = 
          elm_map_marker_add(map, 2.352, 48.857, &itc, &data1);
        marker = elm_map_marker_add(map, 2.355, 48.857, &itc, &data3);
        marker = elm_map_marker_add(map, 3, 48.857, &itc, &data2);
        marker = elm_map_marker_add(map, 2.352, 49, &itc, &data1);

        marker = elm_map_marker_add(map, 7.31451, 48.857127, &itc, &data10);
        marker = elm_map_marker_add(map, 7.314704, 48.857119, &itc, &data4);
        marker = elm_map_marker_add(map, 7.314704, 48.857119, &itc, &data5);
        marker = elm_map_marker_add(map, 7.31432, 48.856785, &itc, &data6);
        marker = elm_map_marker_add(map, 7.3148, 48.85725, &itc, &data7);
        marker = elm_map_marker_add(map, 7.316445, 48.8572210000694, &itc, &data8);
        marker = elm_map_marker_add(map, 7.316527000125, 48.85609, &itc, &data9);
        marker = elm_map_marker_add(map, 7.3165409990833, 48.856078, &itc, &data11);
        marker = elm_map_marker_add(map, 7.319812, 48.856561, &itc, &data10);

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
     }

   evas_object_resize(win, 800, 800);
   evas_object_show(win);
}
#endif

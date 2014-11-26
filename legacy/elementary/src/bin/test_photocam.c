#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
my_ph_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("clicked\n");
}

static void
my_ph_press(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("press\n");
}

static void
my_ph_longpressed(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("longpressed\n");
}

static void
my_ph_clicked_double(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("clicked,double\n");
}

static void
my_ph_load(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("load\n");
}

static void
my_ph_loaded(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("loaded\n");
}

static void
my_ph_load_details(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("load,details\n");
}

static void
my_ph_loaded_details(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("loaded,details\n");
}

static void
my_ph_zoom_start(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("zoom,start\n");
}

static void
my_ph_zoom_stop(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("zoom,stop\n");
}

static void
my_ph_zoom_change(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("zoom,change\n");
}

static void
my_ph_anim_start(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("anim,start\n");
}

static void
my_ph_anim_stop(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("anim,stop\n");
}

static void
my_ph_drag_start(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("drag,start\n");
}

static void
my_ph_drag_stop(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("drag_stop\n");
}

static void
my_ph_scroll(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   int x, y, w, h;
   elm_photocam_image_region_get(obj, &x, &y, &w, &h);
   printf("scroll %i %i %ix%i\n", x, y, w, h);
}

static void
my_bt_open(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Object *ph = data;
   const char *file = event_info;

   if (file)
     elm_photocam_file_set(ph, file);
}

static void
my_bt_show_reg(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_photocam_image_region_show(data, 30, 50, 500, 300);
}

static void
my_bt_bring_reg(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)

{
   elm_photocam_image_region_bring_in(data, 800, 300, 500, 300);
}

static void
my_bt_zoom_in(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   double zoom;

   zoom = elm_photocam_zoom_get(data);
   if (zoom > 1.5)
     zoom -= 0.5;
   else if ((zoom > 1.0) && (zoom <= 1.5))
     zoom = 1.0;
   else if (zoom == 1.0)
     zoom = 0.8;
   else
     zoom = zoom * zoom;

   elm_photocam_zoom_mode_set(data, ELM_PHOTOCAM_ZOOM_MODE_MANUAL);
   if (zoom >= (1.0 / 32.0))
     {
        printf("zoom %f\n", zoom);
        elm_photocam_zoom_set(data, zoom);
     }
}

static void
my_bt_zoom_out(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   double zoom;

   zoom = elm_photocam_zoom_get(data);
   zoom += 0.5;
   elm_photocam_zoom_mode_set(data, ELM_PHOTOCAM_ZOOM_MODE_MANUAL);
   if (zoom <= 256.0)
     {
        printf("zoom %f\n", zoom);
        elm_photocam_zoom_set(data, zoom);
     }
}

static void
my_bt_pause(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_photocam_paused_set(data, !elm_photocam_paused_get(data));
}

static void
my_bt_zoom_fit_in(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_photocam_zoom_mode_set(data, ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT_IN);
}

static void
my_bt_zoom_fit(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_photocam_zoom_mode_set(data, ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT);
}

static void
my_bt_zoom_fill(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_photocam_zoom_mode_set(data, ELM_PHOTOCAM_ZOOM_MODE_AUTO_FILL);
}

static void
my_bt_gesture(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
  elm_photocam_gesture_enabled_set(data, !elm_photocam_gesture_enabled_get(data));
}

static void
my_ph_download_progress(void *data EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Elm_Photocam_Progress *info = (Elm_Photocam_Progress *) event_info;
   Evas_Object *pb = evas_object_data_get(obj, "progressbar");

   if (info->total > 0.0)
     elm_progressbar_value_set(pb, info->now / info->total);
}

static void
my_ph_download_done(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *pb = evas_object_data_get(obj, "progressbar");

   evas_object_hide(pb);
}

static void
_photocam_mouse_wheel_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Object *photocam = data;
   Evas_Event_Mouse_Wheel *ev = (Evas_Event_Mouse_Wheel*) event_info;
   int zoom;
   double val;

   //unset the mouse wheel
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

   zoom = elm_photocam_zoom_get(photocam);
   if ((ev->z>0) && (zoom == 1)) return;

   if (ev->z > 0)
     zoom /= 2;
   else
     zoom *= 2;

   val = 1;
   int _zoom = zoom;
   while (_zoom>1)
     {
        _zoom /= 2;
        val++;
     }

   elm_photocam_zoom_mode_set(photocam, ELM_PHOTOCAM_ZOOM_MODE_MANUAL);
   if (zoom >= 1) elm_photocam_zoom_set(photocam, zoom);
}

static void
_photocam_move_resize_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   int x, y, w, h;

   evas_object_geometry_get(obj, &x, &y, &w, &h);
   evas_object_resize(data, w, h);
   evas_object_move(data, x, y);
}

void
test_photocam(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *ph, *tb2, *bt, *box;
   Evas_Object *rect = NULL;

   win = elm_win_util_standard_add("photocam", "PhotoCam");
   elm_win_autodel_set(win, EINA_TRUE);

   ph = elm_photocam_add(win);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, ph);

   // this rectangle hooks the event prior to scroller
   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rect, 0, 0, 0, 0);
   evas_object_repeat_events_set(rect, EINA_TRUE);
   evas_object_show(rect);
   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_WHEEL, _photocam_mouse_wheel_cb, ph);
   evas_object_raise(rect);

   // add move/resize callbacks to resize rect manually
   evas_object_event_callback_add(ph, EVAS_CALLBACK_RESIZE, _photocam_move_resize_cb, rect);
   evas_object_event_callback_add(ph, EVAS_CALLBACK_MOVE, _photocam_move_resize_cb, rect);

   evas_object_smart_callback_add(ph, "clicked", my_ph_clicked, win);
   evas_object_smart_callback_add(ph, "press", my_ph_press, win);
   evas_object_smart_callback_add(ph, "longpressed", my_ph_longpressed, win);
   evas_object_smart_callback_add(ph, "clicked,double", my_ph_clicked_double, win);
   evas_object_smart_callback_add(ph, "load", my_ph_load, win);
   evas_object_smart_callback_add(ph, "loaded", my_ph_loaded, win);
   evas_object_smart_callback_add(ph, "load,details", my_ph_load_details, win);
   evas_object_smart_callback_add(ph, "loaded,details", my_ph_loaded_details, win);
   evas_object_smart_callback_add(ph, "zoom,start", my_ph_zoom_start, win);
   evas_object_smart_callback_add(ph, "zoom,stop", my_ph_zoom_stop, win);
   evas_object_smart_callback_add(ph, "zoom,change", my_ph_zoom_change, win);
   evas_object_smart_callback_add(ph, "scroll,anim,start", my_ph_anim_start, win);
   evas_object_smart_callback_add(ph, "scroll,anim,stop", my_ph_anim_stop, win);
   evas_object_smart_callback_add(ph, "scroll,drag,start", my_ph_drag_start, win);
   evas_object_smart_callback_add(ph, "scroll,drag,stop", my_ph_drag_stop, win);
   evas_object_smart_callback_add(ph, "scroll", my_ph_scroll, win);

   evas_object_show(ph);

   tb2 = elm_table_add(win);
   evas_object_size_hint_weight_set(tb2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, tb2);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Z -");
   evas_object_smart_callback_add(bt, "clicked", my_bt_zoom_out, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.1, 0.1);
   elm_table_pack(tb2, bt, 0, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_fileselector_button_add(win);
   elm_object_text_set(bt, "Select Photo");
   evas_object_smart_callback_add(bt, "file,chosen", my_bt_open, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.5, 0.1);
   elm_table_pack(tb2, bt, 1, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Z +");
   evas_object_smart_callback_add(bt, "clicked", my_bt_zoom_in, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.1);
   elm_table_pack(tb2, bt, 2, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Show 30,50 500x300");
   evas_object_smart_callback_add(bt, "clicked", my_bt_show_reg, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.1, 0.5);
   elm_table_pack(tb2, bt, 0, 1, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Bring 800,300 500x300");
   evas_object_smart_callback_add(bt, "clicked", my_bt_bring_reg, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.5);
   elm_table_pack(tb2, bt, 2, 1, 1, 1);
   evas_object_show(bt);

   box = elm_box_add(win);
   elm_box_horizontal_set(box, EINA_TRUE);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, 0.0, 0.9);
   elm_table_pack(tb2, box, 0, 2, 1, 1);
   evas_object_show(box);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Pause On/Off");
   evas_object_smart_callback_add(bt, "clicked", my_bt_pause, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(box, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Gesture On/Off");
   evas_object_smart_callback_add(bt, "clicked", my_bt_gesture, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(box, bt);
   evas_object_show(bt);

   box = elm_box_add(win);
   elm_box_horizontal_set(box, EINA_TRUE);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, 0.9, 0.9);
   elm_table_pack(tb2, box, 2, 2, 1, 1);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Fit");
   evas_object_smart_callback_add(bt, "clicked", my_bt_zoom_fit, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(box, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Fit_In");
   evas_object_smart_callback_add(bt, "clicked", my_bt_zoom_fit_in, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(box, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Fill");
   evas_object_smart_callback_add(bt, "clicked", my_bt_zoom_fill, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(box, bt);
   evas_object_show(bt);

   evas_object_show(box);
   evas_object_show(tb2);

   evas_object_resize(win, 800, 800);
   evas_object_show(win);
}

void
test_photocam_remote(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *ph, *tb2, *bt, *box, *pb;
   Evas_Object *rect = NULL;
   // these were just testing - use the "select photo" browser to select one
   static const char *url = "http://eoimages.gsfc.nasa.gov/images/imagerecords/73000/73751/world.topo.bathy.200407.3x21600x10800.jpg";

   win = elm_win_util_standard_add("photocam", "PhotoCam");
   elm_win_autodel_set(win, EINA_TRUE);

   ph = elm_photocam_add(win);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, ph);

   // this rectangle hooks the event prior to scroller
   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rect, 0, 0, 0, 0);
   evas_object_repeat_events_set(rect, EINA_TRUE);
   evas_object_show(rect);
   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_WHEEL, _photocam_mouse_wheel_cb, ph);
   evas_object_raise(rect);

   // add move/resize callbacks to resize rect manually
   evas_object_event_callback_add(ph, EVAS_CALLBACK_RESIZE, _photocam_move_resize_cb, rect);
   evas_object_event_callback_add(ph, EVAS_CALLBACK_MOVE, _photocam_move_resize_cb, rect);

   evas_object_smart_callback_add(ph, "clicked", my_ph_clicked, win);
   evas_object_smart_callback_add(ph, "press", my_ph_press, win);
   evas_object_smart_callback_add(ph, "longpressed", my_ph_longpressed, win);
   evas_object_smart_callback_add(ph, "clicked,double", my_ph_clicked_double, win);
   evas_object_smart_callback_add(ph, "load", my_ph_load, win);
   evas_object_smart_callback_add(ph, "loaded", my_ph_loaded, win);
   evas_object_smart_callback_add(ph, "load,details", my_ph_load_details, win);
   evas_object_smart_callback_add(ph, "loaded,details", my_ph_loaded_details, win);
   evas_object_smart_callback_add(ph, "zoom,start", my_ph_zoom_start, win);
   evas_object_smart_callback_add(ph, "zoom,stop", my_ph_zoom_stop, win);
   evas_object_smart_callback_add(ph, "zoom,change", my_ph_zoom_change, win);
   evas_object_smart_callback_add(ph, "scroll,anim,start", my_ph_anim_start, win);
   evas_object_smart_callback_add(ph, "scroll,anim,stop", my_ph_anim_stop, win);
   evas_object_smart_callback_add(ph, "scroll,drag,start", my_ph_drag_start, win);
   evas_object_smart_callback_add(ph, "scroll,drag,stop", my_ph_drag_stop, win);
   evas_object_smart_callback_add(ph, "scroll", my_ph_scroll, win);
   evas_object_smart_callback_add(ph, "download,progress", my_ph_download_progress, win);
   evas_object_smart_callback_add(ph, "download,done", my_ph_download_done, win);

   elm_photocam_file_set(ph, url);
   evas_object_show(ph);

   tb2 = elm_table_add(win);
   evas_object_size_hint_weight_set(tb2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, tb2);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Z -");
   evas_object_smart_callback_add(bt, "clicked", my_bt_zoom_out, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.1, 0.1);
   elm_table_pack(tb2, bt, 0, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Z +");
   evas_object_smart_callback_add(bt, "clicked", my_bt_zoom_in, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.1);
   elm_table_pack(tb2, bt, 2, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Show 30,50 500x300");
   evas_object_smart_callback_add(bt, "clicked", my_bt_show_reg, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.1, 0.5);
   elm_table_pack(tb2, bt, 0, 1, 1, 1);
   evas_object_show(bt);

   pb = elm_progressbar_add(win);
   elm_progressbar_unit_format_set(pb, "Loading %.2f %%");
   evas_object_size_hint_weight_set(pb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(pb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb2, pb, 1, 1, 1, 1);
   evas_object_show(pb);
   evas_object_data_set(ph, "progressbar", pb);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Bring 800,300 500x300");
   evas_object_smart_callback_add(bt, "clicked", my_bt_bring_reg, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.5);
   elm_table_pack(tb2, bt, 2, 1, 1, 1);
   evas_object_show(bt);

   box = elm_box_add(win);
   elm_box_horizontal_set(box, EINA_TRUE);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, 0.0, 0.9);
   elm_table_pack(tb2, box, 0, 2, 1, 1);
   evas_object_show(box);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Pause On/Off");
   evas_object_smart_callback_add(bt, "clicked", my_bt_pause, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(box, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Gesture On/Off");
   evas_object_smart_callback_add(bt, "clicked", my_bt_gesture, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(box, bt);
   evas_object_show(bt);

   box = elm_box_add(win);
   elm_box_horizontal_set(box, EINA_TRUE);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, 0.9, 0.9);
   elm_table_pack(tb2, box, 2, 2, 1, 1);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Fit");
   evas_object_smart_callback_add(bt, "clicked", my_bt_zoom_fit, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(box, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Fit_In");
   evas_object_smart_callback_add(bt, "clicked", my_bt_zoom_fit_in, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(box, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Fill");
   evas_object_smart_callback_add(bt, "clicked", my_bt_zoom_fill, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(box, bt);
   evas_object_show(bt);

   evas_object_show(box);
   evas_object_show(tb2);

   evas_object_resize(win, 800, 800);
   evas_object_show(win);
}

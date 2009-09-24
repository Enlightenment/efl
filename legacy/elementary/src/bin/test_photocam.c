#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH
void
my_bt_zoom_in(void *data, Evas_Object *obj, void *event_info)
{
   int zoom;
   
   zoom = elm_photocam_zoom_get(data);
   zoom /= 2;
   elm_photocam_zoom_mode_set(data, ELM_PHOTOCAM_ZOOM_MODE_MANUAL);
   if (zoom >= 1) elm_photocam_zoom_set(data, zoom);
}

void
my_bt_zoom_out(void *data, Evas_Object *obj, void *event_info)
{
   int zoom;
   
   zoom = elm_photocam_zoom_get(data);
   zoom *= 2;
   elm_photocam_zoom_mode_set(data, ELM_PHOTOCAM_ZOOM_MODE_MANUAL);
   if (zoom <= 32) elm_photocam_zoom_set(data, zoom);
}

void
test_photocam(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *ph, *tb2, *bt;
   char buf[PATH_MAX];
   const char *img[4] =
     {
        "/home/raster/t1.jpg",  //   5 mpixel
        "/home/raster/t2.jpg",  //  18 mpixel
        "/home/raster/t3.jpg",  //  39 mpixel
        "/home/raster/t4.jpg"   // 192 mpixel
     };

   win = elm_win_add(NULL, "photocam", ELM_WIN_BASIC);
   elm_win_title_set(win, "Photocam");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   ph = elm_photocam_add(win);
   evas_object_size_hint_weight_set(ph, 1.0, 1.0);
   elm_win_resize_object_add(win, ph);
   
   elm_photocam_file_set(ph, img[1]);
   
   evas_object_show(ph);
   
   tb2 = elm_table_add(win);
   evas_object_size_hint_weight_set(tb2, 1.0, 1.0);
   elm_win_resize_object_add(win, tb2);
   
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Z -");
   evas_object_smart_callback_add(bt, "clicked", my_bt_zoom_out, ph);
   evas_object_size_hint_weight_set(bt, 1.0, 1.0);
   evas_object_size_hint_align_set(bt, 0.1, 0.1);
   elm_table_pack(tb2, bt, 0, 0, 1, 1);
   evas_object_show(bt);
   
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Z +");
   evas_object_smart_callback_add(bt, "clicked", my_bt_zoom_in, ph);
   evas_object_size_hint_weight_set(bt, 1.0, 1.0);
   evas_object_size_hint_align_set(bt, 0.9, 0.1);
   elm_table_pack(tb2, bt, 0, 0, 1, 1);
   evas_object_show(bt);
   
   evas_object_show(tb2);
   
   evas_object_resize(win, 800, 800);
   evas_object_show(win);
}
#endif

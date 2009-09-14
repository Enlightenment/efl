#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH
void
test_photocam(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *ph;
   char buf[PATH_MAX];
   const char *img[3] =
     {
        "/home/raster/t1.jpg", 
        "/home/raster/t2.jpg", 
        "/home/raster/t3.jpg"
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
   
   elm_photocam_file_set(ph, img[2]);
   
   evas_object_show(ph);
   
   evas_object_resize(win, 800, 800);
   evas_object_show(win);
}
#endif

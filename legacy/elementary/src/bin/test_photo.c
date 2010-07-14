#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH
void
test_photo(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *sc, *tb, *ph;
   int i, j, n;
   char buf[PATH_MAX];
   const char *img[9] =
     {
        "panel_01.jpg",
          "mystrale.jpg",
          "mystrale_2.jpg",
          "rock_02.jpg",
          "sky_01.jpg",
          "sky_02.jpg",
          "sky_03.jpg",
          "sky_04.jpg",
          "wood_01.jpg"
     };

   win = elm_win_add(NULL, "photo", ELM_WIN_BASIC);
   elm_win_title_set(win, "Photo");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   
   n = 0;
   for (j = 0; j < 12; j++)
     {
        for (i = 0; i < 12; i++)
          {
             ph = elm_photo_add(win);
             snprintf(buf, sizeof(buf), "%s/images/%s",
                      PACKAGE_DATA_DIR, img[n]);
             n++;
             if (n >= 9) n = 0;
             elm_photo_file_set(ph, buf);
             evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, 
                                              EVAS_HINT_EXPAND);
             evas_object_size_hint_align_set(ph, EVAS_HINT_FILL, 
                                             EVAS_HINT_FILL);
             elm_photo_size_set(ph, 80);
             if(n == 2 || n == 3) {
            	 elm_photo_fill_inside_set(ph, EINA_TRUE);
            	 elm_widget_style_set(ph, "shadow");
             }
             elm_table_pack(tb, ph, i, j, 1, 1);
             evas_object_show(ph);
          }
     }
   
   sc = elm_scroller_add(win);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, sc);
   
   elm_scroller_content_set(sc, tb);
   evas_object_show(tb);
   evas_object_show(sc);
   
   evas_object_resize(win, 300, 300);
   evas_object_show(win);
}
#endif

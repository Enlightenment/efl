#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

static void drop_cb(void *mydata, Evas_Object *obj, void *evdata);
static void drag_stop_cb(void *mydata, Evas_Object *obj, void *evdata);
static void drag_start_cb(void *mydata, Evas_Object *obj, void *evdata);

void
test_photo(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
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

   elm_need_ethumb();

   win = elm_win_add(NULL, "photo", ELM_WIN_BASIC);
   elm_win_title_set(win, "Photo");
   elm_win_autodel_set(win, EINA_TRUE);

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
                      elm_app_data_dir_get(), img[n]);
             n++;
             if (n >= 9) n = 0;
             elm_photo_size_set(ph, 80);
             if (n == 8)
               elm_photo_thumb_set(ph, buf, NULL);
             else
               elm_photo_file_set(ph, buf);
             elm_photo_editable_set(ph, 1);
             evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND,
                                              EVAS_HINT_EXPAND);
             evas_object_size_hint_align_set(ph, EVAS_HINT_FILL,
                                             EVAS_HINT_FILL);
             evas_object_smart_callback_add(ph, "drop",
                                            drop_cb, NULL);
             evas_object_smart_callback_add(ph, "drag,start",
                                            drag_start_cb, NULL);
             evas_object_smart_callback_add(ph, "drag,stop",
                                            drag_stop_cb, NULL);

             if ((n == 2) || (n == 3))
               {
                  elm_photo_fill_inside_set(ph, EINA_TRUE);
                  elm_object_style_set(ph, "shadow");
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

/* Never called, elm_photo never call "drop" smart cb */
static void
drop_cb(void *mydata __UNUSED__, Evas_Object *obj, void *evdata __UNUSED__)
{
   printf("Drop on obj %p\n", obj);
}

static void
drag_start_cb(void *mydata __UNUSED__, Evas_Object *obj __UNUSED__, void *evdata __UNUSED__)
{

}

static void
drag_stop_cb(void *mydata __UNUSED__, Evas_Object *obj __UNUSED__, void *evdata __UNUSED__)
{

}

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/
#endif

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

void
test_thumb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *sc, *tb, *th;
   int i, j;
   char buf[PATH_MAX];
   const char *img[] =
     {
        "panel_01.jpg",
        "plant_01.jpg",
        "rock_01.jpg",
        "rock_02.jpg",
        "sky_01.jpg",
        "sky_02.jpg",
        "sky_03.jpg",
        "sky_04.jpg",
        "wood_01.jpg",
        "mystrale.jpg",
        "mystrale_2.jpg"
     };

   elm_need_ethumb();

   win = elm_win_util_standard_add("thumb", "Thumb");
   elm_win_autodel_set(win, EINA_TRUE);

   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   for (j = 0; j < (int)EINA_C_ARRAY_LENGTH(img); j++)
     {
        for (i = 0; i < (int)EINA_C_ARRAY_LENGTH(img); i++)
          {
             th = elm_thumb_add(win);
             snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(),
                      img[i]);
             elm_thumb_file_set(th, buf, NULL);
             evas_object_size_hint_weight_set(th, EVAS_HINT_EXPAND,
                                              EVAS_HINT_EXPAND);
             evas_object_size_hint_align_set(th, EVAS_HINT_FILL,
                                             EVAS_HINT_FILL);
             elm_table_pack(tb, th, i, j, 1, 1);
             elm_thumb_editable_set(th, EINA_TRUE);
             evas_object_show(th);
          }
     }

   sc = elm_scroller_add(win);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, sc);

   elm_object_content_set(sc, tb);
   evas_object_show(tb);
   evas_object_show(sc);

   evas_object_resize(win, 600, 600);
   evas_object_show(win);
}

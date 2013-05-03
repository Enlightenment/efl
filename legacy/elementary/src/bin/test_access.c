#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

static void
_cleanup_cb(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_config_access_set(EINA_FALSE);
}

void
test_access(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   int i, j, k;
   char buf[PATH_MAX];
   Evas_Object *win, *bx, *sc, *ly, *ly2, *ic;
   Evas_Object *ao, *to;

   win = elm_win_util_standard_add("access", "Access");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _cleanup_cb, NULL);

   elm_config_access_set(EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_homogeneous_set(bx, EINA_TRUE);
   elm_box_horizontal_set(bx, EINA_TRUE);

   sc = elm_scroller_add(win);
   elm_scroller_bounce_set(sc, EINA_TRUE, EINA_FALSE);
   elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, sc);

   elm_object_content_set(sc, bx);
   evas_object_show(bx);
   evas_object_show(sc);

   for (k = 0 ; k < 3; k++)
     {
        ly = elm_layout_add(win);
        snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
        elm_layout_file_set(ly, buf, "access_page");
        evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_show(ly);

        for (j = 0; j < 3; j++)
          {
             for (i = 0; i < 3; i++)
               {
                  ly2 = elm_layout_add(win);
                  snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
                  elm_layout_file_set(ly2, buf, "access_icon");
                  evas_object_size_hint_weight_set(ly2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                  evas_object_size_hint_align_set(ly2, EVAS_HINT_FILL, EVAS_HINT_FILL);

                  ic = elm_icon_add(win);
                  elm_object_scale_set(ic, 0.5);
                  snprintf(buf, sizeof(buf), "%s/images/icon_%02i.png", elm_app_data_dir_get(), (i + (k * 3)));
                  elm_image_file_set(ic, buf, NULL);
                  elm_image_resizable_set(ic, 0, 0);
                  evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                  evas_object_size_hint_align_set(ic, 0.5, 0.5);
                  elm_object_part_content_set(ly2, "slot", ic);
                  evas_object_show(ic);

                  snprintf(buf, sizeof(buf), "slot.%i.%i", i, j);
                  elm_object_part_content_set(ly, buf, ly2);
                  evas_object_show(ly2);

                  /* access */
                  to = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(ly2), "access");
                  ao = elm_access_object_register(to, ly2);
                  elm_object_focus_custom_chain_append(ly2, ao, NULL);
               }
          }

        elm_box_pack_end(bx, ly);
     }

   evas_object_resize(win, 300, 300);
   evas_object_show(win);
}
#endif

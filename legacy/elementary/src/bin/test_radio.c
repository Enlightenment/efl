#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
void
test_radio(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *ic, *rd, *rdg;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "radios", ELM_WIN_BASIC);
   elm_win_title_set(win, "Radios");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, 0);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(rd, EVAS_HINT_FILL, 0.5);
   elm_object_text_set(rd, "Icon sized to radio");
   elm_radio_icon_set(rd, ic);
   elm_box_pack_end(bx, rd);
   evas_object_show(rd);
   evas_object_show(ic);
   rdg = rd;

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, 1);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "Icon no scale");
   elm_radio_icon_set(rd, ic);
   elm_box_pack_end(bx, rd);
   evas_object_show(rd);
   evas_object_show(ic);

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, 2);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "Label Only");
   elm_box_pack_end(bx, rd);
   evas_object_show(rd);

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, 3);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "Disabled");
   elm_object_disabled_set(rd, 1);
   elm_box_pack_end(bx, rd);
   evas_object_show(rd);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, 4);
   elm_radio_group_add(rd, rdg);
   elm_radio_icon_set(rd, ic);
   elm_box_pack_end(bx, rd);
   evas_object_show(rd);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, 5);
   elm_radio_group_add(rd, rdg);
   elm_radio_icon_set(rd, ic);
   elm_object_disabled_set(rd, 1);
   elm_box_pack_end(bx, rd);
   evas_object_show(rd);
   evas_object_show(ic);

   elm_radio_value_set(rdg, 2);

   evas_object_show(win);
}
#endif

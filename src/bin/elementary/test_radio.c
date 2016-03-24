#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
_rd_changed_cb(void *data EINA_UNUSED, Evas_Object *obj,
               void *event_info EINA_UNUSED)
{
   printf("radio value : %d\n", elm_radio_value_get(obj));
}

static void
_rdg_changed_cb(void *data EINA_UNUSED, Evas_Object *obj,
                void *event_info EINA_UNUSED)
{
   printf("radio group value : %d\n", elm_radio_value_get(obj));
}

static void
_group_1_create(Evas_Object *bx)
{
   Evas_Object *bx2, *fr, *ic, *rd, *rdg;
   char buf[PATH_MAX];

   fr = elm_frame_add(bx);
   elm_object_text_set(fr, "Radio Group 1");
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   bx2 = elm_box_add(fr);
   elm_object_content_set(fr, bx2);
   evas_object_show(bx2);

   // radio 1
   ic = elm_icon_add(bx2);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   rd = elm_radio_add(bx2);
   elm_radio_state_value_set(rd, 0);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(rd, EVAS_HINT_FILL, 0.5);
   elm_object_text_set(rd, "Icon sized to radio");
   elm_object_part_content_set(rd, "icon", ic);
   elm_box_pack_end(bx2, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _rd_changed_cb, NULL);

   // rdg radio group
   rdg = rd;
   evas_object_smart_callback_add(rdg, "changed", _rdg_changed_cb, NULL);

   ic = elm_icon_add(bx2);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);

   rd = elm_radio_add(bx2);
   elm_radio_state_value_set(rd, 1);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "Icon no scale");
   elm_object_part_content_set(rd, "icon", ic);
   elm_box_pack_end(bx2, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _rd_changed_cb, NULL);

   rd = elm_radio_add(bx2);
   elm_radio_state_value_set(rd, 2);
   elm_radio_group_add(rd, rdg); // add a radio button rd to a radio group rdg.
   elm_object_text_set(rd, "Label Only");
   elm_box_pack_end(bx2, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _rd_changed_cb, NULL);

   rd = elm_radio_add(bx2);
   elm_radio_state_value_set(rd, 3);
   elm_radio_group_add(rd, rdg); // add a radio button rd to a radio group rdg.
   elm_object_text_set(rd, "Disabled");
   elm_object_disabled_set(rd, EINA_TRUE);
   elm_box_pack_end(bx2, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _rd_changed_cb, NULL);

   ic = elm_icon_add(bx2);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);

   rd = elm_radio_add(bx2);
   elm_radio_state_value_set(rd, 4);
   elm_radio_group_add(rd, rdg); // add a radio button rd to a radio group rdg.
   elm_object_part_content_set(rd, "icon", ic);
   elm_box_pack_end(bx2, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _rd_changed_cb, NULL);

   ic = elm_icon_add(bx2);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);

   rd = elm_radio_add(bx2);
   elm_radio_state_value_set(rd, 5);
   elm_radio_group_add(rd, rdg); // add a radio button rd to a radio group rdg.
   elm_object_part_content_set(rd, "icon", ic);
   elm_object_disabled_set(rd, EINA_TRUE);
   elm_box_pack_end(bx2, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _rd_changed_cb, NULL);

   elm_radio_value_set(rdg, 2);
}

static void
_group_2_create(Evas_Object *bx)
{
   Evas_Object *bx2, *fr, *rd, *rdg;

   fr = elm_frame_add(bx);
   elm_object_text_set(fr, "Radio Group 2");
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   bx2 = elm_box_add(fr);
   elm_object_content_set(fr, bx2);
   elm_box_align_set(bx2, 0.0, 0.0);
   evas_object_show(bx2);

   // radio 1
   rd = elm_radio_add(bx2);
   elm_radio_state_value_set(rd, 1);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(rd, EVAS_HINT_FILL, 0.5);
   elm_object_text_set(rd, "Radio Group Test #1");
   elm_box_pack_end(bx2, rd);
   evas_object_show(rd);

   // rdg radio group
   rdg = rd;
   // this works only for the first radio object
   evas_object_smart_callback_add(rdg, "changed", _rdg_changed_cb, NULL);

   // radio 2
   rd = elm_radio_add(bx2);
   elm_radio_state_value_set(rd, 2);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(rd, EVAS_HINT_FILL, 0.5);
   elm_object_text_set(rd, "Radio Group Test #2");
   elm_radio_group_add(rd, rdg);
   elm_box_pack_end(bx2, rd);
   evas_object_show(rd);

   // radio 2
   rd = elm_radio_add(bx2);
   elm_radio_state_value_set(rd, 3);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(rd, EVAS_HINT_FILL, 0.5);
   elm_object_text_set(rd, "Radio Group Test #3");
   elm_radio_group_add(rd, rdg);
   elm_box_pack_end(bx2, rd);
   evas_object_show(rd);
}

void
test_radio(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx;

   win = elm_win_util_standard_add("radios", "Radios");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_show(bx);

   _group_1_create(bx);
   _group_2_create(bx);

   evas_object_show(win);
}

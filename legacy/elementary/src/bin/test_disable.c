#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
_disable_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eina_Bool status = EINA_FALSE;
   status = elm_object_disabled_get(data);
   elm_object_disabled_set(data, !status);
}

void
test_box_disable(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *bt, *bx_out, *fr, *lb;
   Evas_Object *btn[4];

   win = elm_win_util_standard_add("box-transition", "Box Transition");
   elm_win_autodel_set(win, EINA_TRUE);

   bx_out = elm_box_add(win);
   evas_object_size_hint_weight_set(bx_out, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx_out);
   evas_object_show(bx_out);

   fr = elm_frame_add(bx_out);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx_out, fr);
   elm_object_text_set(fr, "Description");
   evas_object_show(fr);

   bx = elm_box_add(fr);
   elm_object_content_set(fr, bx);
   evas_object_show(bx);

   lb = elm_label_add(bx);
   elm_object_text_set(lb, "This test shows how enable/disable of Container widget works");
   evas_object_size_hint_align_set(lb, 0.0, EVAS_HINT_FILL);
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   btn[0] = elm_button_add(bx);
   elm_object_text_set(btn[0], "Disable/Enable Box");
   evas_object_size_hint_weight_set(btn[0], EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btn[0], EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, btn[0]);
   evas_object_show(btn[0]);

   btn[1] = elm_button_add(bx);
   elm_object_text_set(btn[1], "Disable/Enable Button 1");
   evas_object_size_hint_weight_set(btn[1], EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btn[1], EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, btn[1]);
   evas_object_show(btn[1]);

   btn[2] = elm_button_add(bx);
   elm_object_text_set(btn[2], "Disable/Enable Button 2");
   evas_object_size_hint_weight_set(btn[2], EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btn[2], EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, btn[2]);
   evas_object_show(btn[2]);

   btn[3] = elm_button_add(bx);
   elm_object_text_set(btn[3], "Disable/Enable Button 3");
   evas_object_size_hint_weight_set(btn[3], EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btn[3], EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, btn[3]);
   evas_object_show(btn[3]);

   bx = elm_box_add(bx_out);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx_out, bx);
   evas_object_show(bx);
   evas_object_smart_callback_add(btn[0], "clicked", _disable_cb, bx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 1");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(btn[1], "clicked", _disable_cb, bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 2");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(btn[2], "clicked", _disable_cb, bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 3");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(btn[3], "clicked", _disable_cb, bt);

   evas_object_resize(win, 300, 300);
   evas_object_show(win);
}

void
test_layout_disable(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *ly, *bt;
   Evas_Object *fr, *btn[4], *bx, *lb;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("layout", "Layout");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   fr = elm_frame_add(box);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, fr);
   elm_object_text_set(fr, "Description");
   evas_object_show(fr);

   bx = elm_box_add(fr);
   elm_object_content_set(fr, bx);
   evas_object_show(bx);

   lb = elm_label_add(bx);
   elm_object_text_set(lb, "This test shows how enable/disable of Container widget works");
   evas_object_size_hint_align_set(lb, 0.0, EVAS_HINT_FILL);
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   btn[0] = elm_button_add(bx);
   elm_object_text_set(btn[0], "Disable/Enable Layout");
   evas_object_size_hint_weight_set(btn[0], EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btn[0], EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, btn[0]);
   evas_object_show(btn[0]);

   btn[1] = elm_button_add(bx);
   elm_object_text_set(btn[1], "Disable/Enable Button 1");
   evas_object_size_hint_weight_set(btn[1], EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btn[1], EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, btn[1]);
   evas_object_show(btn[1]);

   btn[2] = elm_button_add(bx);
   elm_object_text_set(btn[2], "Disable/Enable Button 2");
   evas_object_size_hint_weight_set(btn[2], EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btn[2], EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, btn[2]);
   evas_object_show(btn[2]);

   btn[3] = elm_button_add(bx);
   elm_object_text_set(btn[3], "Disable/Enable Button 3");
   evas_object_size_hint_weight_set(btn[3], EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btn[3], EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, btn[3]);
   evas_object_show(btn[3]);

   ly = elm_layout_add(win);

   if (!elm_layout_theme_set(
         ly, "layout", "application", "titlebar"))
     fprintf(stderr, "Failed to set layout");

   elm_object_part_text_set(ly, "elm.text", "Layout Disable/Enable Test");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, ly);
   evas_object_show(ly);

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "layout");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(box, ly);
   evas_object_show(ly);
   evas_object_smart_callback_add(btn[0], "clicked", _disable_cb, ly);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 1");
   elm_object_part_content_set(ly, "element1", bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(btn[1], "clicked", _disable_cb, bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 2");
   elm_object_part_content_set(ly, "element2", bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(btn[2], "clicked", _disable_cb, bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 3");
   elm_object_part_content_set(ly, "element3", bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(btn[3], "clicked", _disable_cb, bt);

   evas_object_show(win);
}

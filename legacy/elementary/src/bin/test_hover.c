#include "elementary_config.h"
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

static void
_dismissed_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
   printf("hover dismissed callback is called!\n");
}

static void
my_hover_bt(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *hv = data;

   evas_object_show(hv);
}

static void
_top_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   Evas_Object *hv = (Evas_Object *)data;
   elm_hover_dismiss(hv);
}

void
test_hover(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *bt, *hv, *ic;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("hover", "Hover");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 440, 440);
   evas_object_show(win);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   hv = elm_hover_add(win);
   evas_object_smart_callback_add(hv, "dismissed", _dismissed_cb, NULL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button");
   evas_object_smart_callback_add(bt, "clicked", my_hover_bt, hv);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   elm_hover_parent_set(hv, win);
   elm_hover_target_set(hv, bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Popup");
   elm_object_part_content_set(hv, "middle", bt);
   evas_object_show(bt);

   bx = elm_box_add(win);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, 0, 0);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Top 1");
   evas_object_smart_callback_add(bt, "clicked", _top_bt_clicked, hv);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Top 2");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Top 3");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_show(bx);
   elm_object_part_content_set(hv, "top", bx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Bottom");
   elm_object_part_content_set(hv, "bottom", bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Left");
   elm_object_part_content_set(hv, "left", bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Right");
   elm_object_part_content_set(hv, "right", bt);
   evas_object_show(bt);
}

void
test_hover2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *bt, *hv, *ic;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("hover2", "Hover 2");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 440, 440);
   evas_object_show(win);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   hv = elm_hover_add(win);
   elm_object_style_set(hv, "popout");

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button");
   evas_object_smart_callback_add(bt, "clicked", my_hover_bt, hv);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   elm_hover_parent_set(hv, win);
   elm_hover_target_set(hv, bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Popup");
   elm_object_part_content_set(hv, "middle", bt);
   evas_object_show(bt);

   bx = elm_box_add(win);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, 0, 0);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Top 1");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Top 2");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Top 3");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_show(bx);
   elm_object_part_content_set(hv, "top", bx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Bot");
   elm_object_part_content_set(hv, "bottom", bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Left");
   elm_object_part_content_set(hv, "left", bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Right");
   elm_object_part_content_set(hv, "right", bt);
   evas_object_show(bt);
}
#endif

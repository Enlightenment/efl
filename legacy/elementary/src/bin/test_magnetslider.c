#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

static void _pos_selected_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   printf("Selection: %s\n", (char *)event_info);
   printf("Label selected: %s\n", elm_magnetslider_selected_label_get(obj));
}

static void
_position_change_magnetic_cb(void *data __UNUSED__, Evas_Object * obj, void *event_info)
{
   if (!strcmp((char *)event_info, "left"))
     elm_magnetslider_magnet_pos_set(obj, ELM_MAGNETSLIDER_LEFT);
   else if (!strcmp((char *)event_info, "right"))
     elm_magnetslider_magnet_pos_set(obj, ELM_MAGNETSLIDER_RIGHT);
}

void
test_magnetslider(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *ms;

   win = elm_win_add(NULL, "magnetslider", ELM_WIN_BASIC);
   elm_win_title_set(win, "Magnetslider");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   ms = elm_magnetslider_add(win);
   evas_object_size_hint_weight_set(ms, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ms, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_magnetslider_indicator_pos_set(ms, ELM_MAGNETSLIDER_RIGHT);
   elm_magnetslider_magnet_pos_set(ms, ELM_MAGNETSLIDER_RIGHT);
   elm_magnetslider_labels_set(ms, "Snooze", NULL, "Stop");
   elm_magnetslider_enabled_pos_set(ms, ELM_MAGNETSLIDER_LEFT |
                                    ELM_MAGNETSLIDER_RIGHT);
   evas_object_smart_callback_add(ms, "pos_changed",
                                  _position_change_magnetic_cb, NULL);
   evas_object_smart_callback_add(ms, "selected", _pos_selected_cb, NULL);
   evas_object_show(ms);
   elm_box_pack_end(bx, ms);

   ms = elm_magnetslider_add(win);
   evas_object_size_hint_weight_set(ms, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ms, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_magnetslider_indicator_pos_set(ms, ELM_MAGNETSLIDER_CENTER);
   elm_magnetslider_magnet_pos_set(ms, ELM_MAGNETSLIDER_CENTER);
   elm_magnetslider_labels_set(ms, "Snooze", NULL, "Stop");
   elm_magnetslider_enabled_pos_set(ms, ELM_MAGNETSLIDER_LEFT |
                                    ELM_MAGNETSLIDER_RIGHT);
   evas_object_smart_callback_add(ms, "selected", _pos_selected_cb, NULL);
   evas_object_show(ms);
   elm_box_pack_end(bx, ms);

   ms = elm_magnetslider_add(win);
   elm_object_style_set(ms, "bar");
   evas_object_size_hint_weight_set(ms, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ms, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_magnetslider_indicator_pos_set(ms, ELM_MAGNETSLIDER_LEFT);
   elm_magnetslider_magnet_pos_set(ms, ELM_MAGNETSLIDER_CENTER|
                                   ELM_MAGNETSLIDER_RIGHT);
   elm_magnetslider_enabled_pos_set(ms, ELM_MAGNETSLIDER_CENTER |
                                    ELM_MAGNETSLIDER_RIGHT);
   elm_magnetslider_labels_set(ms, NULL, "Accept", "Reject");
   evas_object_smart_callback_add(ms, "selected", _pos_selected_cb, NULL);
   evas_object_show(ms);
   elm_box_pack_end(bx, ms);

   ms = elm_magnetslider_add(win);
   elm_object_style_set(ms, "bar");
   evas_object_size_hint_weight_set(ms, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ms, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_magnetslider_indicator_pos_set(ms, ELM_MAGNETSLIDER_LEFT);
   elm_magnetslider_magnet_pos_set(ms, ELM_MAGNETSLIDER_LEFT);
   elm_magnetslider_labels_set(ms, NULL, "Accept", "Reject");
   evas_object_smart_callback_add(ms, "pos_changed",
                                  _position_change_magnetic_cb, NULL);
   evas_object_smart_callback_add(ms, "selected", _pos_selected_cb, NULL);
   evas_object_show(ms);
   elm_box_pack_end(bx, ms);


   ms = elm_magnetslider_add(win);
   evas_object_size_hint_weight_set(ms, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ms, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_magnetslider_indicator_pos_set(ms, ELM_MAGNETSLIDER_LEFT);
   elm_magnetslider_magnet_pos_set(ms, ELM_MAGNETSLIDER_ALL);
   elm_magnetslider_labels_set(ms, "Left", "Center", "Right");
   evas_object_smart_callback_add(ms, "selected", _pos_selected_cb, NULL);
   evas_object_show(ms);
   elm_box_pack_end(bx, ms);

   evas_object_resize(win, 320, 400);
   evas_object_show(win);
}
#endif

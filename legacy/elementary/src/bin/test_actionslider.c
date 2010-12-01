#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

static void _pos_selected_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   printf("Selection: %s\n", (char *)event_info);
   printf("Label selected: %s\n", elm_actionslider_selected_label_get(obj));
}

static void
_position_change_magnetic_cb(void *data __UNUSED__, Evas_Object * obj, void *event_info)
{
   if (!strcmp((char *)event_info, "left"))
     elm_actionslider_magnet_pos_set(obj, ELM_ACTIONSLIDER_LEFT);
   else if (!strcmp((char *)event_info, "right"))
     elm_actionslider_magnet_pos_set(obj, ELM_ACTIONSLIDER_RIGHT);
}

void
test_actionslider(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *ms;

   win = elm_win_add(NULL, "actionslider", ELM_WIN_BASIC);
   elm_win_title_set(win, "Actionslider");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   ms = elm_actionslider_add(win);
   evas_object_size_hint_weight_set(ms, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ms, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_actionslider_indicator_pos_set(ms, ELM_ACTIONSLIDER_RIGHT);
   elm_actionslider_magnet_pos_set(ms, ELM_ACTIONSLIDER_RIGHT);
   elm_actionslider_labels_set(ms, "Snooze", NULL, "Stop");
   elm_actionslider_enabled_pos_set(ms, ELM_ACTIONSLIDER_LEFT |
                                    ELM_ACTIONSLIDER_RIGHT);
   evas_object_smart_callback_add(ms, "pos_changed",
                                  _position_change_magnetic_cb, NULL);
   evas_object_smart_callback_add(ms, "selected", _pos_selected_cb, NULL);
   evas_object_show(ms);
   elm_box_pack_end(bx, ms);

   ms = elm_actionslider_add(win);
   evas_object_size_hint_weight_set(ms, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ms, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_actionslider_indicator_pos_set(ms, ELM_ACTIONSLIDER_CENTER);
   elm_actionslider_magnet_pos_set(ms, ELM_ACTIONSLIDER_CENTER);
   elm_actionslider_labels_set(ms, "Snooze", NULL, "Stop");
   elm_actionslider_enabled_pos_set(ms, ELM_ACTIONSLIDER_LEFT |
                                    ELM_ACTIONSLIDER_RIGHT);
   evas_object_smart_callback_add(ms, "selected", _pos_selected_cb, NULL);
   evas_object_show(ms);
   elm_box_pack_end(bx, ms);

   ms = elm_actionslider_add(win);
   elm_object_style_set(ms, "bar");
   evas_object_size_hint_weight_set(ms, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ms, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_actionslider_indicator_pos_set(ms, ELM_ACTIONSLIDER_LEFT);
   elm_actionslider_magnet_pos_set(ms, ELM_ACTIONSLIDER_CENTER|
                                   ELM_ACTIONSLIDER_RIGHT);
   elm_actionslider_enabled_pos_set(ms, ELM_ACTIONSLIDER_CENTER |
                                    ELM_ACTIONSLIDER_RIGHT);
   elm_actionslider_labels_set(ms, NULL, "Accept", "Reject");
   evas_object_smart_callback_add(ms, "selected", _pos_selected_cb, NULL);
   evas_object_show(ms);
   elm_box_pack_end(bx, ms);

   ms = elm_actionslider_add(win);
   elm_object_style_set(ms, "bar");
   evas_object_size_hint_weight_set(ms, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ms, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_actionslider_indicator_pos_set(ms, ELM_ACTIONSLIDER_LEFT);
   elm_actionslider_magnet_pos_set(ms, ELM_ACTIONSLIDER_LEFT);
   elm_actionslider_labels_set(ms, NULL, "Accept", "Reject");
   evas_object_smart_callback_add(ms, "pos_changed",
                                  _position_change_magnetic_cb, NULL);
   evas_object_smart_callback_add(ms, "selected", _pos_selected_cb, NULL);
   evas_object_show(ms);
   elm_box_pack_end(bx, ms);


   ms = elm_actionslider_add(win);
   evas_object_size_hint_weight_set(ms, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ms, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_actionslider_indicator_pos_set(ms, ELM_ACTIONSLIDER_LEFT);
   elm_actionslider_magnet_pos_set(ms, ELM_ACTIONSLIDER_ALL);
   elm_actionslider_labels_set(ms, "Left", "Center", "Right");
   evas_object_smart_callback_add(ms, "selected", _pos_selected_cb, NULL);
   evas_object_show(ms);
   elm_box_pack_end(bx, ms);

   evas_object_resize(win, 320, 400);
   evas_object_show(win);
}
#endif

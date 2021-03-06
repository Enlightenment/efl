//Compile with:
//gcc -g actionslider_example_01.c -o actionslider_example_01 `pkg-config --cflags --libs elementary`

#include <Elementary.h>

static void _pos_selected_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   printf("Selection: %s\n", (char *)event_info);
   printf("Label selected: %s\n", elm_actionslider_selected_label_get(obj));
}

static void
_position_change_magnetic_cb(void *data EINA_UNUSED, Evas_Object * obj, void *event_info)
{
   if (!strcmp((char *)event_info, "left"))
     elm_actionslider_magnet_pos_set(obj, ELM_ACTIONSLIDER_LEFT);
   else if (!strcmp((char *)event_info, "right"))
     elm_actionslider_magnet_pos_set(obj, ELM_ACTIONSLIDER_RIGHT);
}

static void
_magnet_enable_disable_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   if (!strcmp((char *)event_info, "left"))
     elm_actionslider_magnet_pos_set(obj, ELM_ACTIONSLIDER_CENTER);
   else if (!strcmp((char *)event_info, "right"))
     elm_actionslider_magnet_pos_set(obj, ELM_ACTIONSLIDER_NONE);
}

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Evas_Object *win, *bx, *as;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("actionslider", "Actionslider");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   as = elm_actionslider_add(win);
   evas_object_size_hint_weight_set(as, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(as, EVAS_HINT_FILL, 0);
   elm_actionslider_indicator_pos_set(as, ELM_ACTIONSLIDER_RIGHT);
   elm_actionslider_magnet_pos_set(as, ELM_ACTIONSLIDER_RIGHT);
   elm_object_part_text_set(as, "left", "Snooze");
   elm_object_part_text_set(as, "right", "Stop");
   elm_actionslider_enabled_pos_set(as, ELM_ACTIONSLIDER_LEFT |
                                    ELM_ACTIONSLIDER_RIGHT);
   evas_object_smart_callback_add(as, "pos_changed",
                                  _position_change_magnetic_cb, NULL);
   evas_object_smart_callback_add(as, "selected", _pos_selected_cb, NULL);
   evas_object_show(as);
   elm_box_pack_end(bx, as);

   as = elm_actionslider_add(win);
   evas_object_size_hint_weight_set(as, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(as, EVAS_HINT_FILL, 0);
   elm_actionslider_indicator_pos_set(as, ELM_ACTIONSLIDER_CENTER);
   elm_actionslider_magnet_pos_set(as, ELM_ACTIONSLIDER_CENTER);
   elm_object_part_text_set(as, "left", "Snooze");
   elm_object_part_text_set(as, "right", "Stop");
   elm_actionslider_enabled_pos_set(as, ELM_ACTIONSLIDER_LEFT |
                                    ELM_ACTIONSLIDER_RIGHT);
   evas_object_smart_callback_add(as, "selected", _pos_selected_cb, NULL);
   evas_object_show(as);
   elm_box_pack_end(bx, as);

   as = elm_actionslider_add(win);
   elm_object_style_set(as, "bar");
   evas_object_size_hint_weight_set(as, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(as, EVAS_HINT_FILL, 0);
   elm_actionslider_indicator_pos_set(as, ELM_ACTIONSLIDER_LEFT);
   elm_actionslider_magnet_pos_set(as, ELM_ACTIONSLIDER_CENTER|
                                   ELM_ACTIONSLIDER_RIGHT);
   elm_actionslider_enabled_pos_set(as, ELM_ACTIONSLIDER_CENTER |
                                    ELM_ACTIONSLIDER_RIGHT);
   elm_object_part_text_set(as, "center", "Accept");
   elm_object_part_text_set(as, "right", "Reject");
   evas_object_smart_callback_add(as, "selected", _pos_selected_cb, NULL);
   evas_object_show(as);
   elm_box_pack_end(bx, as);

   as = elm_actionslider_add(win);
   elm_object_style_set(as, "bar");
   evas_object_size_hint_weight_set(as, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(as, EVAS_HINT_FILL, 0);
   elm_actionslider_indicator_pos_set(as, ELM_ACTIONSLIDER_LEFT);
   elm_actionslider_magnet_pos_set(as, ELM_ACTIONSLIDER_LEFT);
   elm_object_part_text_set(as, "center", "Accept");
   elm_object_part_text_set(as, "right", "Reject");
   elm_object_text_set(as, "Go");
   evas_object_smart_callback_add(as, "pos_changed",
                                  _position_change_magnetic_cb, NULL);
   evas_object_smart_callback_add(as, "selected", _pos_selected_cb, NULL);
   evas_object_show(as);
   elm_box_pack_end(bx, as);

   as = elm_actionslider_add(win);
   evas_object_size_hint_weight_set(as, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(as, EVAS_HINT_FILL, 0);
   elm_actionslider_indicator_pos_set(as, ELM_ACTIONSLIDER_LEFT);
   elm_actionslider_magnet_pos_set(as, ELM_ACTIONSLIDER_ALL);
   elm_object_part_text_set(as, "left", "Left");
   elm_object_part_text_set(as, "center", "Center");
   elm_object_part_text_set(as, "right", "Right");
   elm_object_text_set(as, "Go");
   evas_object_smart_callback_add(as, "selected", _pos_selected_cb, NULL);
   evas_object_show(as);
   elm_box_pack_end(bx, as);

   as = elm_actionslider_add(win);
   evas_object_size_hint_weight_set(as, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(as, EVAS_HINT_FILL, 0);
   elm_actionslider_indicator_pos_set(as, ELM_ACTIONSLIDER_CENTER);
   elm_actionslider_magnet_pos_set(as, ELM_ACTIONSLIDER_CENTER);
   elm_object_part_text_set(as, "left", "Enable");
   elm_object_part_text_set(as, "center", "Magnet");
   elm_object_part_text_set(as, "right", "Disable");
   evas_object_smart_callback_add(as, "pos_changed",
                                  _magnet_enable_disable_cb, NULL);
   evas_object_smart_callback_add(as, "selected", _pos_selected_cb, NULL);
   evas_object_show(as);
   elm_box_pack_end(bx, as);

   evas_object_resize(win, 320, 400);
   evas_object_show(win);

   elm_run();
   return 0;
}
ELM_MAIN()

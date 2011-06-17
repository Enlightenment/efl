//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` actionslider_example_01.c -o actionslider_example_01

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

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

static void
_magnet_enable_disable_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   if (!strcmp((char *)event_info, "left"))
      elm_actionslider_magnet_pos_set(obj, ELM_ACTIONSLIDER_CENTER);
   else if (!strcmp((char *)event_info, "right"))
      elm_actionslider_magnet_pos_set(obj, ELM_ACTIONSLIDER_NONE);
}

static void
on_done(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_exit();
}

EAPI int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *as;

   win = elm_win_add(NULL, "actionslider", ELM_WIN_BASIC);
   elm_win_title_set(win, "Actionslider");
   evas_object_smart_callback_add(win, "delete,request", on_done, NULL);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, 0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   as = elm_actionslider_add(win);
   evas_object_size_hint_weight_set(as, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(as, EVAS_HINT_FILL, 0);
   elm_actionslider_indicator_pos_set(as, ELM_ACTIONSLIDER_RIGHT);
   elm_actionslider_magnet_pos_set(as, ELM_ACTIONSLIDER_RIGHT);
   elm_actionslider_labels_set(as, "Snooze", NULL, "Stop");
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
   elm_actionslider_labels_set(as, "Snooze", NULL, "Stop");
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
   elm_actionslider_labels_set(as, NULL, "Accept", "Reject");
   evas_object_smart_callback_add(as, "selected", _pos_selected_cb, NULL);
   evas_object_show(as);
   elm_box_pack_end(bx, as);

   as = elm_actionslider_add(win);
   elm_object_style_set(as, "bar");
   evas_object_size_hint_weight_set(as, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(as, EVAS_HINT_FILL, 0);
   elm_actionslider_indicator_pos_set(as, ELM_ACTIONSLIDER_LEFT);
   elm_actionslider_magnet_pos_set(as, ELM_ACTIONSLIDER_LEFT);
   elm_actionslider_labels_set(as, NULL, "Accept", "Reject");
   elm_actionslider_indicator_label_set(as, "Go");
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
   elm_actionslider_labels_set(as, "Left", "Center", "Right");
   elm_actionslider_indicator_label_set(as, "Go");
   evas_object_smart_callback_add(as, "selected", _pos_selected_cb, NULL);
   evas_object_show(as);
   elm_box_pack_end(bx, as);

   as = elm_actionslider_add(win);
   evas_object_size_hint_weight_set(as, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(as, EVAS_HINT_FILL, 0);
   elm_actionslider_indicator_pos_set(as, ELM_ACTIONSLIDER_CENTER);
   elm_actionslider_magnet_pos_set(as, ELM_ACTIONSLIDER_CENTER);
   elm_actionslider_labels_set(as, "Enable", "Magnet", "Disable");
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

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

static void
_tt_text_replace(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   static int count = 0;
   char buf[64];
   snprintf(buf, sizeof(buf), "count=%d", count);
   count++;
   elm_object_tooltip_text_set(obj, buf);
}

static void
_tt_timer_del(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Ecore_Timer *timer = evas_object_data_del(obj, "test-timer");
   if (!timer) return;
   ecore_timer_del(timer);
}

static Eina_Bool
_tt_text_replace_timer_cb(void *data)
{
   _tt_text_replace(NULL, data, NULL);
   return EINA_TRUE;
}

static void
_tt_text_replace_timed(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Ecore_Timer *timer = evas_object_data_get(obj, "test-timer");
   if (timer)
     {
        ecore_timer_del(timer);
        evas_object_data_del(obj, "test-timer");
        elm_button_label_set(obj, "Simple text tooltip, click to start changed timed");
        return;
     }

   timer = ecore_timer_add(1.5, _tt_text_replace_timer_cb, obj);
   evas_object_data_set(obj, "test-timer", timer);
   elm_button_label_set(obj, "Simple text tooltip, click to stop changed timed");
}

static Evas_Object *
_tt_icon(void *data __UNUSED__, Evas_Object *obj)
{
   Evas_Object *ic = elm_icon_add(obj);
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_resize(ic, 64, 64);
   return ic;
}

static Evas_Object *
_tt_icon2(void *data __UNUSED__, Evas_Object *obj)
{
   Evas_Object *ic = elm_icon_add(obj);
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/icon_00.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_resize(ic, 64, 64);
   return ic;
}

static void
_tt_icon_del(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   // test to check for del_cb behavior!
   printf("_tt_icon_del: data=%ld (== 123?), event_info=%p\n",
          (long)data, event_info);
}

static Eina_Bool
_tt_icon_replace_timer_cb(void *data)
{
   static int current = 0;

   elm_object_tooltip_content_cb_set
     (data, current ? _tt_icon2 : _tt_icon, NULL, NULL);

   current = !current;
   return EINA_TRUE;
}

static void
_tt_icon_replace_timed(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Ecore_Timer *timer = evas_object_data_get(obj, "test-timer");
   if (timer)
     {
        ecore_timer_del(timer);
        evas_object_data_del(obj, "test-timer");
        elm_button_label_set(obj, "Icon tooltip, click to start changed timed");
        return;
     }

   timer = ecore_timer_add(1.5, _tt_icon_replace_timer_cb, obj);
   evas_object_data_set(obj, "test-timer", timer);
   elm_button_label_set(obj, "Icon tooltip, click to stop changed timed");
}

static Eina_Bool
_tt_style_replace_timer_cb(void *data)
{
   static int current = 0;
   elm_object_tooltip_style_set(data, current ? NULL : "transparent");
   current = !current;
   return EINA_TRUE;
}

static void
_tt_style_replace_timed(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Ecore_Timer *timer = evas_object_data_get(obj, "test-timer");
   if (timer)
     {
        ecore_timer_del(timer);
        evas_object_data_del(obj, "test-timer");
        elm_button_label_set(obj, "Icon tooltip style, click to start changed timed");
        return;
     }

   timer = ecore_timer_add(1.5, _tt_style_replace_timer_cb, obj);
   evas_object_data_set(obj, "test-timer", timer);
   elm_button_label_set(obj, "Icon tooltip style, click to stop changed timed");
}

static void
_tt_visible_lock_toggle(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   static int locked = 0;

   locked = !locked;
   if (locked)
     {
        elm_button_label_set(obj, "Locked tooltip visibility");
        elm_object_tooltip_text_set(obj, "This tooltip is locked visible,<br> click the button to unlock!");
        elm_object_tooltip_show(obj);
     }
   else
     {
        elm_button_label_set(obj, "Unlocked tooltip visibility");
        elm_object_tooltip_text_set(obj, "This tooltip is unlocked visible,<br> click the button to lock!");
        elm_object_tooltip_hide(obj);
     }

}


void
test_tooltip(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *bt;

   win = elm_win_add(NULL, "tooltip", ELM_WIN_BASIC);
   elm_win_title_set(win, "Tooltip");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Simple text tooltip");
   elm_object_tooltip_text_set(bt, "Simple text tooltip");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Simple text tooltip, click to change");
   elm_object_tooltip_text_set(bt, "Initial");
   evas_object_smart_callback_add(bt, "clicked", _tt_text_replace, NULL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Simple text tooltip, click to start changed timed");
   elm_object_tooltip_text_set(bt, "Initial");
   evas_object_smart_callback_add(bt, "clicked", _tt_text_replace_timed, NULL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_event_callback_add(bt, EVAS_CALLBACK_DEL, _tt_timer_del, NULL);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Icon tooltip");
   elm_object_tooltip_content_cb_set(bt, _tt_icon, (void*)123L, _tt_icon_del);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Icon tooltip, click to start changed timed");
   elm_object_tooltip_content_cb_set(bt, _tt_icon, NULL, NULL);
   evas_object_smart_callback_add(bt, "clicked", _tt_icon_replace_timed, NULL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_event_callback_add(bt, EVAS_CALLBACK_DEL, _tt_timer_del, NULL);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Transparent Icon tooltip");
   elm_object_tooltip_content_cb_set(bt, _tt_icon, NULL, NULL);
   elm_object_tooltip_style_set(bt, "transparent");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Icon tooltip style, click to start changed timed");
   elm_object_tooltip_content_cb_set(bt, _tt_icon, NULL, NULL);
   evas_object_smart_callback_add(bt, "clicked", _tt_style_replace_timed, NULL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_event_callback_add(bt, EVAS_CALLBACK_DEL, _tt_timer_del, NULL);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Unlocked tooltip visibility");
   elm_object_tooltip_text_set(bt, "This tooltip is unlocked visible,<br> click the button to lock!");
   evas_object_smart_callback_add(bt, "clicked", _tt_visible_lock_toggle, NULL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);


   evas_object_show(win);
}
#endif

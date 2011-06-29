#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
void
test_clock(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *ck;
   unsigned int digedit;

   win = elm_win_add(NULL, "clock", ELM_WIN_BASIC);
   elm_win_title_set(win, "Clock");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   ck = elm_clock_add(win);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);

   ck = elm_clock_add(win);
   elm_clock_show_am_pm_set(ck, 1);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);

   ck = elm_clock_add(win);
   elm_clock_show_seconds_set(ck, 1);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);

   ck = elm_clock_add(win);
   elm_clock_show_seconds_set(ck, 1);
   elm_clock_show_am_pm_set(ck, 1);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);

   ck = elm_clock_add(win);
   elm_clock_show_seconds_set(ck, 1);
   elm_clock_show_am_pm_set(ck, 0);
   elm_clock_time_set(ck, 23, 59, 57);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);

   ck = elm_clock_add(win);
   elm_clock_edit_set(ck, 1);
   elm_clock_show_seconds_set(ck, 1);
   elm_clock_show_am_pm_set(ck, 1);
   elm_clock_time_set(ck, 10, 11, 12);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);

   ck = elm_clock_add(win);
   elm_clock_show_seconds_set(ck, 1);
   elm_clock_edit_set(ck, 1);
   digedit = ELM_CLOCK_HOUR_UNIT | ELM_CLOCK_MIN_UNIT | ELM_CLOCK_SEC_UNIT;
   elm_clock_digit_edit_set(ck, digedit);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);

   evas_object_show(win);
}

static void
_edit_bt_clicked(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *ck = data;

   if (!elm_clock_edit_get(ck)) {
	elm_object_text_set(obj, "Done");
	elm_clock_edit_set(ck, 1);
	return;
   }
   elm_object_text_set(obj, "Edit");
   elm_clock_edit_set(ck, 0);
}

static void
_hmode_bt_clicked(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *ck = data;

   if (!elm_clock_show_am_pm_get(ck)) {
	elm_object_text_set(obj, "24h");
	elm_clock_show_am_pm_set(ck, 1);
	return;
   }
   elm_object_text_set(obj, "12h");
   elm_clock_show_am_pm_set(ck, 0);
}

void
test_clock2(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *hbx, *ck, *bt;

   win = elm_win_add(NULL, "clock2", ELM_WIN_BASIC);
   elm_win_title_set(win, "Clock 2");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   ck = elm_clock_add(win);
   elm_clock_show_seconds_set(ck, 1);
   elm_clock_show_am_pm_set(ck, 1);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);

   hbx = elm_box_add(win);
   evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_horizontal_set(hbx, EINA_TRUE);
   elm_box_pack_end(bx, hbx);
   evas_object_show(hbx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Edit");
   evas_object_smart_callback_add(bt, "clicked", _edit_bt_clicked, ck);
   elm_box_pack_end(hbx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "24h");
   evas_object_smart_callback_add(bt, "clicked", _hmode_bt_clicked, ck);
   elm_box_pack_end(hbx, bt);
   evas_object_show(bt);

   evas_object_show(win);
}

#endif

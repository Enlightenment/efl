#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#endif
#include <Elementary.h>

/* A simple test, just displaying clock in its default format */

Evas_Object *dt1, *dt2, *dt3, *dt4;

static void
_changed_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("Clock value is changed\n");
}

static void
_bt_clicked(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   time_t t;
   struct tm new_time;

   t = time(NULL);
   localtime_r(&t, &new_time);

   new_time.tm_year = 85;
   new_time.tm_mon = 9;
   new_time.tm_mday = 26;
   new_time.tm_hour = 9;
   new_time.tm_min = 0;
   efl_ui_clock_field_visible_set(dt1, EFL_UI_CLOCK_TYPE_HOUR, EINA_TRUE);
   efl_ui_clock_field_visible_set(dt1, EFL_UI_CLOCK_TYPE_MINUTE, EINA_TRUE);
   efl_ui_clock_field_visible_set(dt1, EFL_UI_CLOCK_TYPE_AMPM, EINA_TRUE);
   efl_ui_clock_field_visible_set(dt1, EFL_UI_CLOCK_TYPE_SECOND, EINA_TRUE);
   efl_ui_clock_field_visible_set(dt1, EFL_UI_CLOCK_TYPE_DAY, EINA_TRUE);

   efl_ui_clock_value_set(dt1, &new_time);
   elm_object_disabled_set(dt1, EINA_TRUE);

   elm_object_disabled_set(obj, EINA_TRUE);
   evas_object_del(dt2);
   evas_object_del(dt3);
   dt2 = dt3 = NULL;
}

void
test_ui_clock(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *bt, *lb;

   win = elm_win_util_standard_add("ui_clock", "ui_clock");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   elm_box_horizontal_set(bx, EINA_FALSE);
   evas_object_show(bx);
   evas_object_size_hint_min_set(bx, 360, 240);

   dt1 = efl_add(EFL_UI_CLOCK_CLASS, bx);
   evas_object_size_hint_weight_set(dt1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(dt1, EVAS_HINT_FILL, 0.5);
   efl_ui_clock_field_visible_set(dt1, EFL_UI_CLOCK_TYPE_HOUR, EINA_FALSE);
   efl_ui_clock_field_visible_set(dt1, EFL_UI_CLOCK_TYPE_MINUTE, EINA_FALSE);
   efl_ui_clock_field_visible_set(dt1, EFL_UI_CLOCK_TYPE_AMPM, EINA_FALSE);
   efl_ui_clock_field_visible_set(dt1, EFL_UI_CLOCK_TYPE_SECOND, EINA_FALSE);
   efl_ui_clock_field_visible_set(dt1, EFL_UI_CLOCK_TYPE_DAY, EINA_FALSE);
   efl_ui_clock_pause_set(dt1, EINA_TRUE);
   elm_box_pack_end(bx, dt1);
   evas_object_smart_callback_add(dt1, "changed", _changed_cb, NULL);
   evas_object_show(dt1);

   dt2 = efl_add(EFL_UI_CLOCK_CLASS, bx);
   evas_object_size_hint_weight_set(dt2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(dt2, EVAS_HINT_FILL, 0.5);
   efl_ui_clock_field_visible_set(dt2, EFL_UI_CLOCK_TYPE_YEAR, EINA_FALSE);
   efl_ui_clock_field_visible_set(dt2, EFL_UI_CLOCK_TYPE_MONTH, EINA_FALSE);
   efl_ui_clock_field_visible_set(dt2, EFL_UI_CLOCK_TYPE_DATE, EINA_FALSE);
   efl_ui_clock_field_visible_set(dt1, EFL_UI_CLOCK_TYPE_SECOND, EINA_FALSE);
   elm_box_pack_end(bx, dt2);
   efl_ui_clock_pause_set(dt2, EINA_TRUE);
   elm_object_disabled_set(dt2, EINA_TRUE);
   evas_object_show(dt2);

   dt3 = efl_add(EFL_UI_CLOCK_CLASS, bx);
   evas_object_size_hint_weight_set(dt3, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(dt3, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, dt3);
   evas_object_show(dt3);

   //editable
   lb = efl_add(ELM_LABEL_CLASS, bx);
   elm_object_text_set(lb,
                       "<b>Editable Clock:</b>"
                       );
   evas_object_size_hint_weight_set(lb, 0.0, 0.0);
   evas_object_size_hint_align_set(lb, 0, EVAS_HINT_FILL);
   evas_object_size_hint_min_set(lb, 100, 25);
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   dt4 = efl_add(EFL_UI_CLOCK_CLASS, bx);
   evas_object_size_hint_weight_set(dt4, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(dt4, EVAS_HINT_FILL, 0.5);
   efl_ui_clock_edit_mode_set(dt4, EINA_TRUE);
   efl_ui_clock_pause_set(dt4, EINA_TRUE);
   elm_box_pack_end(bx, dt4);
   evas_object_show(dt4);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Back to the future...");
   evas_object_smart_callback_add(bt, "clicked", _bt_clicked, NULL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_show(win);
}

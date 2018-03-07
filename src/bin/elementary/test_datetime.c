#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#endif
#include <Elementary.h>

/* A simple test, just displaying datetime in its default format */

Evas_Object *dt1, *dt2, *dt3;

static void
_changed_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("Datetime value is changed\n");
}

static void
_bt_clicked(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   time_t t;
   struct tm new_time;

   t = time(NULL);
   localtime_r(&t, &new_time);

   new_time.tm_wday = 6;
   new_time.tm_year = 85;
   new_time.tm_mon = 9;
   new_time.tm_mday = 26;
   new_time.tm_hour = 9;
   new_time.tm_min = 0;
   new_time.tm_sec = 0;
   elm_datetime_field_visible_set(dt1, ELM_DATETIME_HOUR, EINA_TRUE);
   elm_datetime_field_visible_set(dt1, ELM_DATETIME_MINUTE, EINA_TRUE);
   efl_ui_clock_pause_set(dt1, EINA_TRUE);
   elm_datetime_value_set(dt1, &new_time);
   elm_object_disabled_set(dt1, EINA_TRUE);
   elm_object_disabled_set(obj, EINA_TRUE);
   evas_object_del(dt2);
   evas_object_del(dt3);
   dt2 = dt3 = NULL;
}

void
test_datetime(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *bt;
   time_t t;
   struct tm time1;

   win = elm_win_util_standard_add("datetime", "DateTime");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   elm_box_horizontal_set(bx, EINA_FALSE);
   evas_object_show(bx);
   evas_object_size_hint_min_set(bx, 360, 240);

   dt1 = elm_datetime_add(bx);
   evas_object_size_hint_weight_set(dt1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(dt1, EVAS_HINT_FILL, 0.5);
   elm_datetime_field_visible_set(dt1, ELM_DATETIME_HOUR, EINA_FALSE);
   elm_datetime_field_visible_set(dt1, ELM_DATETIME_MINUTE, EINA_FALSE);
   elm_datetime_field_visible_set(dt1, ELM_DATETIME_AMPM, EINA_FALSE);
   elm_box_pack_end(bx, dt1);
   evas_object_show(dt1);

   dt2 = elm_datetime_add(bx);
   evas_object_size_hint_weight_set(dt2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(dt2, EVAS_HINT_FILL, 0.5);
   elm_datetime_field_visible_set(dt2, ELM_DATETIME_YEAR, EINA_FALSE);
   elm_datetime_field_visible_set(dt2, ELM_DATETIME_MONTH, EINA_FALSE);
   elm_datetime_field_visible_set(dt2, ELM_DATETIME_DATE, EINA_FALSE);
   elm_box_pack_end(bx, dt2);
   elm_object_disabled_set(dt2, EINA_TRUE);
   evas_object_show(dt2);

   dt3 = elm_datetime_add(bx);
   evas_object_size_hint_weight_set(dt3, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(dt3, EVAS_HINT_FILL, 0.5);

   // get the current local time
   t = time(NULL);
   localtime_r(&t, &time1);
   // set the max year as 2030 and the remaining fields are equal to current time values
   time1.tm_year = 130;
   elm_datetime_value_max_set(dt3, &time1);
   // set the min time limit as "1980 January 10th 02:30 PM"
   time1.tm_year = 80;
   time1.tm_mon = 4;
   time1.tm_mday = 10;
   time1.tm_hour = 14;
   time1.tm_min = 30;
   elm_datetime_value_min_set(dt3, &time1);
   // minutes can be input only in between 15 and 45
   elm_datetime_field_limit_set(dt3, ELM_DATETIME_MINUTE, 15, 45);
   evas_object_smart_callback_add(dt3, "changed", _changed_cb, NULL);
   elm_box_pack_end(bx, dt3);
   evas_object_show(dt3);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Back to the future...");
   evas_object_smart_callback_add(bt, "clicked", _bt_clicked, NULL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_show(win);
}

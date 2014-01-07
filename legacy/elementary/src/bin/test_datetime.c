#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#endif
#include <Elementary.h>

/* A simple test, just displaying datetime in its default format */

static void
_changed_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("Datetime value is changed\n");
}

void
test_datetime(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *datetime;
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

   datetime = elm_datetime_add(bx);
   evas_object_size_hint_weight_set(datetime, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(datetime, EVAS_HINT_FILL, 0.5);
   elm_datetime_field_visible_set(datetime, ELM_DATETIME_HOUR, EINA_FALSE);
   elm_datetime_field_visible_set(datetime, ELM_DATETIME_MINUTE, EINA_FALSE);
   elm_datetime_field_visible_set(datetime, ELM_DATETIME_AMPM, EINA_FALSE);
   elm_box_pack_end(bx, datetime);
   evas_object_show(datetime);

   datetime = elm_datetime_add(bx);
   evas_object_size_hint_weight_set(datetime, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(datetime, EVAS_HINT_FILL, 0.5);
   elm_datetime_field_visible_set(datetime, ELM_DATETIME_YEAR, EINA_FALSE);
   elm_datetime_field_visible_set(datetime, ELM_DATETIME_MONTH, EINA_FALSE);
   elm_datetime_field_visible_set(datetime, ELM_DATETIME_DATE, EINA_FALSE);
   elm_box_pack_end(bx, datetime);
   elm_object_disabled_set(datetime, EINA_TRUE);
   evas_object_show(datetime);

   datetime = elm_datetime_add(bx);
   evas_object_size_hint_weight_set(datetime, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(datetime, EVAS_HINT_FILL, 0.5);

   // get the current local time
   t = time(NULL);
   localtime_r(&t, &time1);
   // set the max year as 2030 and the remaining fields are equal to current time values
   time1.tm_year = 130;
   elm_datetime_value_max_set(datetime, &time1);
   // set the min time limit as "1980 January 10th 02:30 PM"
   time1.tm_year = 80;
   time1.tm_mon = 4;
   time1.tm_mday = 10;
   time1.tm_hour = 14;
   time1.tm_min = 30;
   elm_datetime_value_min_set(datetime, &time1);
   // minutes can be input only in between 15 and 45
   elm_datetime_field_limit_set(datetime, ELM_DATETIME_MINUTE, 15, 45);
   evas_object_smart_callback_add(datetime, "changed", _changed_cb, datetime);
   elm_box_pack_end(bx, datetime);
   evas_object_show(datetime);

   evas_object_show(win);
}

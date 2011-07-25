#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

/* A simple test, just displaying calendar in it's default state */
void
test_calendar(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *cal;

   win = elm_win_add(NULL, "calendar", ELM_WIN_BASIC);
   elm_win_title_set(win, "Calendar");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   cal = elm_calendar_add(win);
   elm_win_resize_object_add(win, cal);
   evas_object_size_hint_weight_set(cal, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(cal);

   evas_object_show(win);
}

void
_print_cal_info(Evas_Object *cal, Evas_Object *en)
{
   char info[1024];
   double interval;
   int year_min, year_max;
   Eina_Bool sel_enabled;
   const char **wds;
   struct tm stime;

   if (!elm_calendar_selected_time_get(cal, &stime))
     return;

   interval = elm_calendar_interval_get(cal);
   elm_calendar_min_max_year_get(cal, &year_min, &year_max);
   sel_enabled = elm_calendar_day_selection_enabled_get(cal);
   wds = elm_calendar_weekdays_names_get(cal);

   snprintf(info, sizeof(info),
	 "  Day: %i, Mon: %i, Year %i, WeekDay: %i<br>"
	 "  Interval: %0.2f, Year_Min: %i, Year_Max %i, Sel Enabled : %i<br>"
	 "  Weekdays: %s, %s, %s, %s, %s, %s, %s<br>",
	 stime.tm_mday, stime.tm_mon, stime.tm_year + 1900, stime.tm_wday,
	 interval, year_min, year_max, sel_enabled,
	 wds[0], wds[1], wds[2], wds[3], wds[4], wds[5], wds[6]);

   elm_entry_entry_set(en, info);
}

static void
_print_cal_info_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   _print_cal_info(obj, data);
}

static char *
_format_month_year(struct tm *stime)
{
   char buf[32];
   if (!strftime(buf, sizeof(buf), "%b %y", stime)) return NULL;
   return strdup(buf);
}

/* A test intended to cover all the calendar api and much use cases as
   possible */
void
test_calendar2(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *bxh, *cal, *cal2, *cal3, *en;
   Elm_Calendar_Mark *mark;
   struct tm selected_time;
   time_t current_time;
   const char *weekdays[] =
   {
      "Sunday", "Monday", "Tuesday", "Wednesday",
      "Thursday", "Friday", "Saturday"
   };

   win = elm_win_add(NULL, "calendar2", ELM_WIN_BASIC);
   elm_win_title_set(win, "Calendar 2");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   bxh = elm_box_add(win);
   elm_box_horizontal_set(bxh, EINA_TRUE);
   evas_object_size_hint_weight_set(bxh, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bxh, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bxh);
   elm_box_pack_end(bx, bxh);

   cal = elm_calendar_add(win);
   evas_object_size_hint_weight_set(cal, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(cal, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(cal);
   elm_box_pack_end(bx, cal);

   cal2 = elm_calendar_add(win);
   evas_object_size_hint_weight_set(cal2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(cal2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_calendar_day_selection_enabled_set(cal2, EINA_FALSE);
   evas_object_show(cal2);
   elm_box_pack_end(bxh, cal2);

   cal3 = elm_calendar_add(win);
   evas_object_size_hint_weight_set(cal3, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(cal3, EVAS_HINT_FILL, EVAS_HINT_FILL);
   current_time = time(NULL) + 34 * 84600;
   localtime_r(&current_time, &selected_time);
   elm_calendar_selected_time_set(cal3, &selected_time);
   current_time = time(NULL) + 1 * 84600;
   localtime_r(&current_time, &selected_time);
   elm_calendar_mark_add(cal3, "checked", &selected_time, ELM_CALENDAR_UNIQUE);
   elm_calendar_marks_clear(cal3);
   current_time = time(NULL);
   localtime_r(&current_time, &selected_time);
   elm_calendar_mark_add(cal3, "checked", &selected_time, ELM_CALENDAR_DAILY);
   elm_calendar_mark_add(cal3, "holiday", &selected_time, ELM_CALENDAR_DAILY);
   elm_calendar_marks_draw(cal3);
   evas_object_show(cal3);
   elm_box_pack_end(bxh, cal3);

   en = elm_entry_add(win);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(en);
   elm_box_pack_end(bx, en);
   elm_entry_editable_set(en, EINA_FALSE);
   evas_object_show(win);

   elm_calendar_min_max_year_set(cal3, -1, -1);

   elm_calendar_weekdays_names_set(cal, weekdays);
   elm_calendar_interval_set(cal, 0.4);
   elm_calendar_format_function_set(cal, _format_month_year);
   elm_calendar_min_max_year_set(cal, 2010, 2020);

   current_time = time(NULL) + 4 * 84600;
   localtime_r(&current_time, &selected_time);
   elm_calendar_mark_add(cal, "holiday", &selected_time, ELM_CALENDAR_ANNUALLY);

   current_time = time(NULL) + 1 * 84600;
   localtime_r(&current_time, &selected_time);
   elm_calendar_mark_add(cal, "checked", &selected_time, ELM_CALENDAR_UNIQUE);

   current_time = time(NULL) - 363 * 84600;
   localtime_r(&current_time, &selected_time);
   elm_calendar_mark_add(cal, "checked", &selected_time, ELM_CALENDAR_MONTHLY);

   current_time = time(NULL) - 5 * 84600;
   localtime_r(&current_time, &selected_time);
   mark = elm_calendar_mark_add(cal, "holiday", &selected_time,
	 ELM_CALENDAR_WEEKLY);

   current_time = time(NULL) + 1 * 84600;
   localtime_r(&current_time, &selected_time);
   elm_calendar_mark_add(cal, "holiday", &selected_time, ELM_CALENDAR_WEEKLY);

   elm_calendar_mark_del(mark);
   elm_calendar_marks_draw(cal);

   _print_cal_info(cal, en);
   evas_object_smart_callback_add(cal, "changed", _print_cal_info_cb, en);
}

#endif

/**
 * Elementary's <b>calendar widget</b> example, illustrating smart callback
 * registry and getters usage.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -o calendar_example_05 calendar_example_05.c -g `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#include <Elementary.h>

static void
_print_cal_info_cb(void *data, Evas_Object *obj, void *event_info)
{
   int year_min, year_max;
   Eina_Bool sel_enabled;
   const char **wds;
   struct tm sel_time;
   double interval;

   if (!elm_calendar_selected_time_get(obj, &sel_time))
     return;

   interval = elm_calendar_interval_get(obj);
   elm_calendar_min_max_year_get(obj, &year_min, &year_max);
   sel_enabled = (elm_calendar_select_mode_get(obj) != ELM_CALENDAR_SELECT_MODE_NONE);
   wds = elm_calendar_weekdays_names_get(obj);

   printf("Day: %i, Mon: %i, Year %i, WeekDay: %i<br>\n"
          "Interval: %0.2f, Year_Min: %i, Year_Max %i, Sel Enabled : %i<br>\n"
          "Weekdays: %s, %s, %s, %s, %s, %s, %s<br>\n\n",
          sel_time.tm_mday, sel_time.tm_mon, sel_time.tm_year + 1900, sel_time.tm_wday,
          interval, year_min, year_max, sel_enabled,
          wds[0], wds[1], wds[2], wds[3], wds[4], wds[5], wds[6]);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *cal;

   win = elm_win_util_standard_add("calendar", "Calendar Getters Example");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   cal = elm_calendar_add(win);
   evas_object_size_hint_weight_set(cal, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, cal);
   /* Add callback to display calendar information every time user
    * selects a new date */
   evas_object_smart_callback_add(cal, "changed", _print_cal_info_cb, NULL);
   evas_object_show(cal);

   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()

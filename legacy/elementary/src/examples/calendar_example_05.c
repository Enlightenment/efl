/**
 * Elementary's <b>calendar widget</b> example, illustrating smart callback
 * registry and getters usage.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g `pkg-config --cflags --libs elementary` calendar_example_05.c -o calendar_example_05
 * @endverbatim
 */

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

static void
_print_cal_info_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   int year_min, year_max;
   Eina_Bool sel_enabled;
   const char **wds;
   struct tm stime;
   double interval;

   if (!elm_calendar_selected_time_get(obj, &stime))
     return;

   interval = elm_calendar_interval_get(obj);
   elm_calendar_min_max_year_get(obj, &year_min, &year_max);
   sel_enabled = elm_calendar_day_selection_enabled_get(obj);
   wds = elm_calendar_weekdays_names_get(obj);

   printf("Day: %i, Mon: %i, Year %i, WeekDay: %i<br>\n"
          "Interval: %0.2f, Year_Min: %i, Year_Max %i, Sel Enabled : %i<br>\n"
          "Weekdays: %s, %s, %s, %s, %s, %s, %s<br>\n\n",
          stime.tm_mday, stime.tm_mon, stime.tm_year + 1900, stime.tm_wday,
          interval, year_min, year_max, sel_enabled,
          wds[0], wds[1], wds[2], wds[3], wds[4], wds[5], wds[6]);
}

EAPI_MAIN int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *cal;

   win = elm_win_add(NULL, "calendar", ELM_WIN_BASIC);
   elm_win_title_set(win, "Calendar Getters Example");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   cal = elm_calendar_add(win);
   elm_win_resize_object_add(win, cal);
   evas_object_size_hint_weight_set(cal, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   /* Add callback to display calendar information every time user
    * selects a new date */
   evas_object_smart_callback_add(cal, "changed", _print_cal_info_cb, NULL);
   evas_object_show(cal);

   evas_object_show(win);

   elm_run();
   return 0;
}
ELM_MAIN()

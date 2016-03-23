/**
 * Elementary's <b>calendar widget</b> example, demonstrates how to modify
 * layout strings, using functions to set weekdays names and to format
 * month and year label.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -o calendar_example_02 calendar_example_02.c -g `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#include <Elementary.h>

static char *
_format_month_year(struct tm *format_time)
{
   char buf[32];
   /* abbreviates month and year */
   if (!strftime(buf, sizeof(buf), "%b %y", format_time)) return NULL;
   return strdup(buf);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *cal;
   const char *weekdays[] =
     {
        "S", "M", "T", "W", "T", "F", "S"
     };

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("calendar", "Calendar Layout Formatting Example");
   elm_win_autodel_set(win, EINA_TRUE);

   cal = elm_calendar_add(win);
   evas_object_size_hint_weight_set(cal, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, cal);

   elm_calendar_format_function_set(cal, _format_month_year);
   elm_calendar_weekdays_names_set(cal, weekdays);

   evas_object_show(cal);

   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()

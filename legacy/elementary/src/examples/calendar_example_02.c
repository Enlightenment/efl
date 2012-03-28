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
   Evas_Object *win, *bg, *cal;
   const char *weekdays[] =
     {
        "S", "M", "T", "W", "T", "F", "S"
     };

   win = elm_win_add(NULL, "calendar", ELM_WIN_BASIC);
   elm_win_title_set(win, "Calendar Layout Formatting Example");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   cal = elm_calendar_add(win);
   elm_win_resize_object_add(win, cal);
   evas_object_size_hint_weight_set(cal, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   elm_calendar_format_function_set(cal, _format_month_year);
   elm_calendar_weekdays_names_set(cal, weekdays);

   evas_object_show(cal);

   evas_object_show(win);

   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()

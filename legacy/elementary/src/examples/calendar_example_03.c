/**
 * Simple Elementary's <b>calendar widget</b> example, illustrating minimum
 * and maximum years restriction. User will see a calendar of years
 * 2020, 2021 and 2022.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -o calendar_example_03 calendar_example_03.c -g `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#include <Elementary.h>

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *cal;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("calendar", "Calendar Min/Max Year Example");
   elm_win_autodel_set(win, EINA_TRUE);

   cal = elm_calendar_add(win);
   evas_object_size_hint_weight_set(cal, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, cal);
   elm_calendar_min_max_year_set(cal, 2020, 2022);
   evas_object_show(cal);

   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()

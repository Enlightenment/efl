/**
 * Simple Elementary's <b>calendar widget</b> example, illustrating its
 * creation.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -o calendar_example_01 calendar_example_01.c -g `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#include <Elementary.h>

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *cal;

   win = elm_win_util_standard_add("calendar", "Calendar Creation Example");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   cal = elm_calendar_add(win);
   evas_object_size_hint_weight_set(cal, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, cal);
   evas_object_show(cal);

   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()

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
   Evas_Object *win, *bg, *cal;

   win = elm_win_add(NULL, "calendar", ELM_WIN_BASIC);
   elm_win_title_set(win, "Calendar Creation Example");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   cal = elm_calendar_add(win);
   elm_win_resize_object_add(win, cal);
   evas_object_size_hint_weight_set(cal, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(cal);

   evas_object_show(win);

   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()

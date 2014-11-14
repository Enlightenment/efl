/**
 * Elementary's <b>calendar widget</b> example to add / del / clear marks.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -o calendar_example_06 calendar_example_06.c -g `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#include <Elementary.h>

#define SECS_DAY 86400

static void
_btn_clear_cb(void *data, Evas_Object *btn, void *ev)
{
   Evas_Object *cal = data;
   elm_calendar_marks_clear(cal);
   elm_calendar_marks_draw(cal);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bt, *bx, *cal;
   Elm_Calendar_Mark *mark;
   struct tm selected_time;
   time_t current_time;
   struct tm sunday = { 0, 0, 12, 7, 0, 0, 0, 0, -1, 0, NULL };
   /* tm {sec, min, hour, mday, mon, year, wday, yday, isdst } */
   /* weekdays since Sunday, range 0 to 6 */
   struct tm christmas;
   christmas.tm_mday = 25;
   /* months since Jan, in the range 0 to 11 */
   christmas.tm_mon = 11;

   win = elm_win_util_standard_add("calendar", "Calendar Marks Example");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   cal = elm_calendar_add(win);
   evas_object_size_hint_weight_set(cal, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(cal, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, cal);
   evas_object_show(cal);

   /* check today - we'll remove it later */
   current_time = time(NULL);
   localtime_r(&current_time, &selected_time);
   mark = elm_calendar_mark_add(cal, "checked", &selected_time,
                                ELM_CALENDAR_UNIQUE);

   /* check tomorrow */
   current_time = time(NULL) + 1 * SECS_DAY;
   localtime_r(&current_time, &selected_time);
   elm_calendar_mark_add(cal, "checked", &selected_time, ELM_CALENDAR_UNIQUE);

   /* mark christmas as holiday */
   elm_calendar_mark_add(cal, "holiday", &christmas, ELM_CALENDAR_ANNUALLY);

   /* mark Sundays as holidays */
   elm_calendar_mark_add(cal, "holiday", &sunday, ELM_CALENDAR_WEEKLY);

   /* ok, let's remove today's check */
   elm_calendar_mark_del(mark);

   elm_calendar_marks_draw(cal);

   bt = elm_button_add(win);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(bt, "Clear marks");
   evas_object_smart_callback_add(bt, "clicked", _btn_clear_cb, cal);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()

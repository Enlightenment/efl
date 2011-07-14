/**
 * Elementary's <b>calendar widget</b> example to add / del / clear marks.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g `pkg-config --cflags --libs elementary` calendar_example_06.c -o calendar_example_06
 * @endverbatim
 */

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

#define SECS_DAY 86400

static void
_btn_clear_cb(void *data, Evas_Object *btn __UNUSED__, void *ev __UNUSED__)
{
   Evas_Object *cal = data;
   elm_calendar_marks_clear(cal);
   elm_calendar_marks_draw(cal);
}

EAPI int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *bt, *bx, *cal;
   Elm_Calendar_Mark *mark;
   struct tm selected_time;
   time_t current_time;
   struct tm sunday = {0, 0, 12, 7, 0, 0, 0, 0, -1 };
   /* tm {sec, min, hour, mday, mon, year, wday, yday, isdst } */
   /* weekdays since Sunday, range 0 to 6 */
   struct tm christmas;
   christmas.tm_mday = 25;
   /* months since Jan, in the range 0 to 11 */
   christmas.tm_mon = 11;

   win = elm_win_add(NULL, "calendar", ELM_WIN_BASIC);
   elm_win_title_set(win, "Calendar Marks Example");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
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
   elm_button_label_set(bt, "Clear marks");
   evas_object_smart_callback_add(bt, "clicked", _btn_clear_cb, cal);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_show(win);

   elm_run();
   return 0;
}
ELM_MAIN()

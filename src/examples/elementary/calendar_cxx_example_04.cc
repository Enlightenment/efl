#include <Elementary.hh>

#define SECS_DAY 86400

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   struct tm selected_time;
   time_t current_time;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   ::elm::win_standard win;
   win.title_set("Calendar Day Selection Example");
   win.autohide_set(true);

   ::elm::box bx(efl::eo::parent = win);
   bx.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   win.resize_object_add(bx);
   bx.visible_set(true);

   ::elm::calendar cal(efl::eo::parent = win);
   cal.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   cal.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
   cal.select_mode_set(ELM_CALENDAR_SELECT_MODE_NONE);
   cal.visible_set(true);
   bx.pack_end(cal);

   ::elm::calendar cal2(efl::eo::parent = win);
   cal2.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   cal2.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
   current_time = time(NULL) +2 * SECS_DAY;
   localtime_r(&current_time, &selected_time);
   cal2.selected_time_set(&selected_time);
   cal2.visible_set(true);
   bx.pack_end(cal2);

   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()

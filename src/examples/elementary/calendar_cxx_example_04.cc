#ifdef HAVE_CONFIG_H
#include "config.h"
#include "elementary_config.h"
#endif

#include <Elementary.hh>

#define SECS_DAY 86400

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   struct tm selected_time;
   time_t current_time;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   efl::ui::win::Standard win;
   //win.title_set("Calendar Day Selection Example");
   win.autohide_set(true);

   ::efl::ui::Box bx(win);
   //bx.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   //win.resize_object_add(bx);
   bx.eo_cxx::efl::Gfx::size_set(700,700);
   bx.visible_set(true);

   ::elm::Calendar cal(win);
   // cal.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   // cal.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
   cal.select_mode_set(ELM_CALENDAR_SELECT_MODE_NONE);
   cal.eo_cxx::efl::Gfx::size_set(125,135);
   cal.visible_set(true);
   //bx.pack_end(cal); no matching function for call to ‘efl::ui::Box::pack_end(elm::Calendar&)’
   //candidate: bool eo_cxx::efl::pack::Linear::pack_end(Efl_Gfx*) const 
   //  inline bool eo_cxx::efl::pack::Linear::pack_end(Efl_Gfx * subobj_) const

   ::elm::Calendar cal2(win);
   //cal2.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   //cal2.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
   current_time = time(NULL) +2 * SECS_DAY;
   localtime_r(&current_time, &selected_time);
   // cal2.selected_time_set(&selected_time);
   cal2.eo_cxx::efl::Gfx::size_set(125,135);
   cal2.visible_set(true);
   //bx.pack_end(cal2);

   win.eo_cxx::efl::Gfx::size_set(500,560);
   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()

#include <Elementary.hh>

#warning This example can not be implemented with EO APIs... FIXME

using efl::eo::instantiate;

#define SECS_DAY 86400

static void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
#if 0
   struct tm selected_time;
   time_t current_time;
#endif

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   efl::ui::Win win(instantiate);
   win.text_set("Calendar Day Selection Example");
   win.autohide_set(true);

   ::efl::ui::Box bx(instantiate, win);
   win.content_set(bx);

#if 0

   ::efl::ui::Calendar cal(instantiate, win);
   cal.select_mode_set(ELM_CALENDAR_SELECT_MODE_NONE);
   cal.eo_cxx::efl::Gfx::size_set({125,135});
   //bx.pack_end(cal); no matching function for call to ‘efl::ui::Box::pack_end(elm::Calendar&)’
   //candidate: bool eo_cxx::efl::pack::Linear::pack_end(Efl_Gfx*) const 
   //  inline bool eo_cxx::efl::pack::Linear::pack_end(Efl_Gfx * subobj_) const

   ::elm::Calendar cal2(instantiate, win);
   //cal2.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   //cal2.size_hint_fill_set(true, true);
   current_time = time(NULL) +2 * SECS_DAY;
   localtime_r(&current_time, &selected_time);
   // cal2.selected_time_set(&selected_time);
   cal2.eo_cxx::efl::Gfx::size_set({125,135});
   //bx.pack_end(cal2);

#endif

   win.size_set({500,560});
}
EFL_MAIN()

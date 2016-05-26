#ifdef HAVE_CONFIG_H
#include "config.h"
#include "elementary_config.h"
#endif

#include <Elementary.hh>

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   efl::ui::win::Standard win;
   //win.title_set("Calendar Min/Max Year Example");
   win.autohide_set(true);

   elm::Calendar cal(win);
   //cal.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   //win.resize_object_add(cal);
   cal.min_max_year_set(2020, 2022);
   cal.visible_set(true);

   cal.eo_cxx::efl::Gfx::size_set(140,140);
   win.eo_cxx::efl::Gfx::size_set(140,140);
   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()

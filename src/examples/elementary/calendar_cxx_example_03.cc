#ifdef HAVE_CONFIG_H
#include "config.h"
#include "elementary_config.h"
#endif

#include <Elementary.hh>

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   Efl_Time min, max;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   efl::ui::win::Standard win;
   //win.title_set("Calendar Min/Max Year Example");
   win.autohide_set(true);

   elm::Calendar cal(win);

   min.tm_year = 2020 - 1900;
   max.tm_year = 2022 - 1900;

   //cal.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   //win.resize_object_add(cal);
   cal.date_min_set(min);
   cal.date_max_set(max);
   cal.visible_set(true);

   cal.eo_cxx::efl::Gfx::size_set(140,140);
   win.eo_cxx::efl::Gfx::size_set(140,140);
   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()

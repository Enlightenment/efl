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
   //win.title_set("Calendar Creation Example");
   win.autohide_set(true);

   ::elm::Calendar cal(win);
   //cal.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   cal.eo_cxx::efl::Gfx::size_set(134,134);
   //win.resize_object_add(cal);
   cal.visible_set(true);

   win.eo_cxx::efl::Gfx::size_set(134,134);
   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()

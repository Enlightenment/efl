#ifdef HAVE_CONFIG_H
#include "config.h"
#include "elementary_config.h"
#endif

#include <Elementary.hh>

using efl::eo::instantiate;

static void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Time min = {}, max = {};

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   min.tm_year = 2020 - 1900;
   max.tm_year = 2022 - 1900;

   auto win = efl::ui::Win(instantiate);
   win.text_set("Calendar Min/Max Year Example");
   win.autohide_set(true);

   auto cal = efl::ui::Calendar(instantiate, win);
   win.content_set(cal);
   cal.date_min_set(min);
   cal.date_max_set(max);

   win.size_set({320,320});
}
EFL_MAIN()

#include <Elementary.hh>

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   ::elm::win_standard win;
   win.title_set("Calendar Min/Max Year Example");
   win.autohide_set(true);

   ::elm::calendar cal(efl::eo::parent = win);
   cal.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   win.resize_object_add(cal);
   cal.min_max_year_set(2020, 2022);
   cal.visible_set(true);

   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()

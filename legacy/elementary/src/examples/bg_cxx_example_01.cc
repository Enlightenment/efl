#include <Elementary.hh>

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   ::elm::win_standard win;
   win.title_set("Bg Plain");
   win.autohide_set(true);

   ::elm::bg bg(efl::eo::parent = win);
   bg.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   win.resize_object_add(bg);
   bg.visible_set(true);

   win.size_set(320,320);
   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()

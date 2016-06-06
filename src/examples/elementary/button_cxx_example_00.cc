#define EFL_EO_API_SUPPORT

#include <Elementary.hh>

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   ::elm::win::Standard win;
   //win.title_set("Hello, World!");
   win.autohide_set(true);

   ::elm::Button btn(win);
   btn.text_set("elm.text","Good-Bye, World!");
   btn.eo_cxx::efl::Gfx::size_set(120, 30);
   btn.eo_cxx::efl::Gfx::position_set(60, 15);
   // btn.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   // btn.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
   btn.visible_set(true);

   auto on_click = std::bind([] () { elm_exit(); });

   efl::eolian::event_add(evas::Clickable_Interface::clicked_event, btn, on_click);

   win.eo_cxx::efl::Gfx::size_set(240, 60);
   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()

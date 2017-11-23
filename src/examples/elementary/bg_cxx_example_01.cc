#include <Elementary.hh>

using efl::eo::instantiate;

EAPI_MAIN int
elm_main (int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   efl::ui::Win win(instantiate);
   win.text_set("Window Background");
   win.autohide_set(true);
   win.size_set({320,320});
   try {
      auto bg = efl::eo::downcast<efl::ui::win::Part>(win.part("background"));
      bg.color_set(139, 69, 19, 255);
   } catch (std::exception const&e) {
      std::cerr << "Failed to set bg color: " << e.what() << std::endl;
   }

   // Clean exit
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   efl::eolian::event_add(efl::ui::Win::delete_request_event, win,
                          std::bind([&](){ win = nullptr; }));

   elm_run();
   return 0;
}
ELM_MAIN()

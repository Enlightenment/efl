#define EFL_EO_API_SUPPORT

#include <Elementary.hh>

EAPI_MAIN int
elm_main (int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   efl::ui::Win win;
   win.text_set("Bg Plain");
   win.autohide_set(true);

   win.eo_cxx::efl::Gfx::size_set({320,320});
   //win.size_set(320,320);
   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()

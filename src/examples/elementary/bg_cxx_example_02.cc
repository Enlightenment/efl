/* To compile:
 * g++ `pkg-config --cflags --libs elementary-cxx eo-cxx efl-cxx eina-cxx ecore-cxx evas-cxx edje-cxx` ./bg_cxx_example_02.cc -o bg_cxx_example_02
 *
 * To run:
 * ./bg_cxx_example_02
 */

#include <Elementary.hh>

#include <sstream>

EAPI_MAIN int
elm_main (int argc EINA_UNUSED, char **args EINA_UNUSED)
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   using efl::eo::instantiate;

   efl::ui::Win win(instantiate);
   win.text_set("Bg Image");
   win.autohide_set(true);

   // FIXME: Part API needs some fixing to be nice in C++ :)
   //efl::eo::downcast<efl::ui::win::Part>(win.part("background"))
   //      .file_set("performance/background.png", nullptr);

   efl::ui::Bg bg(instantiate, win);
   bg.scale_type_set(EFL_UI_IMAGE_SCALE_TYPE_FILL);
   bg.file_set("performance/background.png", nullptr);
   win.content_set(bg);

   win.eo_cxx::efl::Gfx::size_set({640, 400});

   elm_run();
   return 0;
}
ELM_MAIN()

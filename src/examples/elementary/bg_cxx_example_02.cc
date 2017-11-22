/* To compile:
 * g++ `pkg-config --cflags --libs elementary-cxx eo-cxx efl-cxx eina-cxx ecore-cxx evas-cxx edje-cxx` ./bg_cxx_example_02.cc -o bg_cxx_example_02
 *
 * To run:
 * ./bg_cxx_example_02
 */

#define EFL_CXXPERIMENTAL

#include <Elementary.hh>

#include <sstream>

using efl::eo::instantiate;

efl::ui::Win win;

EAPI_MAIN void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   win = efl::ui::Win(instantiate);
   ::efl_ref(win._eo_ptr()); // FIXME: Window is doing BAD THINGS™!
   win.text_set("Bg Image");
   win.autohide_set(true);

   // FIXME: Part API needs some fixing to be nice in C++ :)
   //efl::eo::downcast<efl::ui::win::Part>(win.part("background"))
   //      .file_set("performance/background.png", nullptr);

   efl::ui::Bg bg(instantiate, win);
   bg.scale_type_set(EFL_UI_IMAGE_SCALE_TYPE_FILL);
   bg.file_set("performance/background.png", nullptr);
   win.content_set(bg);

   win.size_set({640, 400});
   std::cout << "win " << win._eo_ptr() << std::endl;
   win.delete_request_event_cb_add([](){ win = nullptr; efl_exit(0); });
}
EFL_MAIN()

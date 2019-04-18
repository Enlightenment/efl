/* To compile:
 * g++ `pkg-config --cflags --libs elementary-cxx eo-cxx efl-cxx eina-cxx ecore-cxx evas-cxx edje-cxx` ./bg_cxx_example_02.cc -o bg_cxx_example_02
 *
 * To run:
 * ./bg_cxx_example_02
 */

#define EFL_CXXPERIMENTAL
#define EFL_BETA_API_SUPPORT

#include <Elementary.hh>

using efl::eo::instantiate;

static efl::ui::Win win;

EAPI_MAIN void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Loop_Arguments *args = static_cast<Efl_Loop_Arguments *>(ev->info);

   win = efl::ui::Win(instantiate);
   win.text_set("Bg Image");
   win.autohide_set(true);
   win.delete_request_event_cb_add([](){ win = nullptr; efl_exit(0); });

   std::string path;
   if (eina_array_count(args->argv) > 0)
     path = static_cast<const char *>(eina_array_data_get(args->argv, 0));
   else
     path = "performance/background.png";

   efl::ui::Bg bg(instantiate, win);
   bg.scale_type_set(EFL_GFX_IMAGE_SCALE_TYPE_FILL);
   bg.file_set(path);
   bg.load();
   win.content_set(bg);

   win.size_set({640, 400});
}
EFL_MAIN()

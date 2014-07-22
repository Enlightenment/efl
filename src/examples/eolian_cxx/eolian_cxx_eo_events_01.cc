#include <iostream>
#include <thread>
#include <cassert>
#include <sstream>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Ecore.h>
#include <Evas.h>
#include <Ecore_Evas.h>

#include <Eina.hh>
#include <Eo.hh>
#include <Ecore.hh>
#include <Evas.hh>

#include <canvas/evas_canvas.eo.hh>
#include <canvas/evas_text.eo.hh>
#include <canvas/evas_image.eo.hh>
#include <canvas/evas_box.eo.hh>


namespace efl { namespace evas {
using ::evas::canvas;
using ::evas::object;
using ::evas::text;
using ::evas::grid;
using ::evas::rectangle;
using ::evas::common_interface;
using ::evas::zoomable_interface;
} }

namespace {

// XXX Ecore-Evas is not binded yet.
Ecore_Evas *ee;
void
_ecore_evas_init()
{
   if (!ee)
     {
        ::ecore_evas_init();
        ::evas_init();
        ee = ::ecore_evas_new(NULL, 0, 0, 500, 380, NULL);
        ::ecore_evas_show(ee);
   }
}
void
_ecore_evas_shutdown()
{
   if (ee)
     {
        ::ecore_evas_free(ee);
        ::ecore_evas_shutdown();
        ee = NULL;
     }
}

}

void
example_complex_types()
{
   _ecore_evas_init();

   {
     efl::evas::canvas canvas(::eo_ref(::ecore_evas_get(ee)));

     efl::evas::rectangle bg(efl::eo::parent = canvas);
     bg.color_set(255, 255, 255, 255);
     bg.position_set(0, 0);
     bg.size_set(500, 250);
     bg.visibility_set(true);

     efl::eo::signal_connection conn =
       bg.event_mouse_down_callback_add
       ([] (efl::evas::object obj EINA_UNUSED, Eo_Event_Description const& desc EINA_UNUSED, void* info EINA_UNUSED)
        {
          std::cout << "evas::box::mouse_down" << std::endl;
          return EO_CALLBACK_CONTINUE;
        });

     canvas.render();
   }

   ::ecore_main_loop_begin();
   _ecore_evas_shutdown();
}

int main()
{
   efl::eina::eina_init eina_;
   efl::eo::eo_init eo_;
   efl::ecore::ecore_init ecore_;

   std::cerr << "[+] Running ELF++ example: Eo Events" << std::endl;
   example_complex_types();

   return 0;
}

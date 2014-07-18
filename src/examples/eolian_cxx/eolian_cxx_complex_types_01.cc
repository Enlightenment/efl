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
#include <canvas/evas_rectangle.eo.hh>
#include <canvas/evas_zoomable_interface.eo.hh>

namespace efl { namespace evas { // XXX only while we don't have namespaces in EFL
using ::evas::canvas;
using ::evas::object;
using ::evas::text;
using ::evas::grid;
using ::evas::rectangle;
using ::evas::common_interface;
using ::evas::zoomable_interface;
} }

namespace {

// EFL Ecore-Evas doesn't have C++ bindings yet.
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
   efl::evas::canvas canvas(::eo_ref(::ecore_evas_get(ee)));

   efl::evas::rectangle bg(efl::eo::parent = canvas);
   bg.color_set(255, 255, 255, 255);
   bg.position_set(0, 0);
   bg.size_set(500, 250);
   bg.visibility_set(true);

   efl::evas::grid grid(efl::eo::parent = canvas);
   grid.position_set(0, 0);
   grid.color_set(0, 0, 0, 255);
   grid.size_set(5, 5);
   grid.visibility_set(true);

   efl::evas::text text1(efl::eo::parent = canvas);
   text1.style_set(EVAS_TEXT_STYLE_OUTLINE);
   text1.color_set(255, 0, 0, 255);
   text1.font_set("DejaVu", 32);
   text1.text_set("EFL++ Examples");
   text1.visibility_set(true);
   int t1w, t1h;
   text1.size_get(&t1w, &t1h);
   grid.pack(text1, 1, 1, t1w, t1h);

   efl::evas::text text2(efl::eo::parent = canvas);
   text2.style_set(EVAS_TEXT_STYLE_PLAIN);
   text2.color_set(0, 120, 0, 255);
   text2.position_set(t1w+50, t1h+50);
   text2.font_set("Courier", 16);
   std::stringstream ss;
   ss << "version " << EFL_VERSION_MAJOR << "." << EFL_VERSION_MINOR;
   text2.text_set(ss.str().c_str());
   text2.visibility_set(true);
   int t2w, t2h;
   text2.size_get(&t2w, &t2h);

   canvas.render();
   ::ecore_main_loop_begin();
   _ecore_evas_shutdown();
}

int main()
{
   efl::eina::eina_init eina_;
   efl::eo::eo_init eo_;
   efl::ecore::ecore_init ecore_;

   std::cerr << "[+] Running ELF++ example: Complex Types" << std::endl;
   example_complex_types();

   return 0;
}



// Example disabled while Eo Evas API is still rapidly changing
#if 0
#include <iostream>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>

#include <Eina.hh>
#include <Eo.hh>

#include "canvas/evas_canvas_eo.hh"
#include "canvas/evas_image_eo.hh"
#include "canvas/evas_rectangle.eo.hh"

#define WIDTH (320)
#define HEIGHT (240)

int main()
{
   efl::eina::eina_init eina_init;
   efl::eo::eo_init init;
   if (!::ecore_evas_init()) return EXIT_FAILURE;
   if (!::evas_init()) return EXIT_FAILURE;

   Ecore_Evas *ee = ::ecore_evas_new(NULL, 10, 10, WIDTH, HEIGHT, NULL);
   if (!ee)
     {
        fprintf(stderr,
                "error: Requires at least one Evas engine built and linked"
                " to ecore-evas for this example to run properly.\n");
        ::ecore_evas_free(ee);
        return -1;
     }

   ecore_evas_show(ee);

   {
      evas::canvas canvas(::eo_ref(::ecore_evas_get(ee)));
      evas::rectangle rect(efl::eo::parent = canvas);
      rect.color_set(255, 0, 0, 255);
      rect.position_set(10, 10);
      rect.size_set(100, 100);
      rect.visible_set(true);
      canvas.render();
   }

   ::ecore_main_loop_begin();

   ::ecore_evas_free(ee);
   ::ecore_evas_shutdown();
   return 0;
}
#else
int main()
{
}
#endif

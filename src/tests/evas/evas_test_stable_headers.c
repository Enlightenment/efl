// Compiled without EFL_BETA_API_SUPPORT: the stable Efl.Canvas.Object_Animation
// mixin must be declared by <Efl_Canvas.h>, the umbrella Efl_Ui.h uses. Taking
// the addresses is enough: the declarations come from the header, and a symbol
// that the library does not export is a null weak reference, which main()
// reports.

#ifdef EFL_BETA_API_SUPPORT
# error "this test is about the stable API, it must be built without the beta macro"
#endif

#include <stdio.h>
#include <Evas.h>
#include <Efl_Canvas.h>

static void *const _stable[] =
{
   efl_canvas_object_animation_mixin_get,
   efl_canvas_object_animation_start,
   efl_canvas_object_animation_stop,
   efl_canvas_object_animation_pause_set,
};

int
main(void)
{
   unsigned int i;
   int missing = 0;

   // A strong reference keeps libevas linked, as the weak ones do not.
   evas_init();
   for (i = 0; i < sizeof(_stable) / sizeof(_stable[0]); i++)
     {
        if (_stable[i]) continue;
        fprintf(stderr, "stable symbol %u is not exported\n", i);
        missing++;
     }
   evas_shutdown();
   return missing ? 1 : 0;
}

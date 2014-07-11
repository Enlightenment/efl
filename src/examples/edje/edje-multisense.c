/**
 * Simple example illustrating the very basic functions of multisense
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one) and multisense support ine edje. See stdout/stderr for output.
 *
 * @verbatim
 * edje_cc multisense.edc && gcc -o edje-multisense edje-multisense.c `pkg-config --libs --cflags eina evas ecore ecore-evas edje`
 * @endverbatim
 */

#include <Eina.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#ifndef PACKAGE_DATA_DIR
#define PACKAGE_DATA_DIR "."
#endif

#define WIDTH  300
#define HEIGHT 300

static Evas_Object *create_my_group(Evas *canvas)
{
   Evas_Object *edje;

   edje = edje_object_add(canvas);
   if (!edje)
     {
        EINA_LOG_CRIT("could not create edje object!");
        return NULL;
     }

   if (!edje_object_file_set(edje, PACKAGE_DATA_DIR"/multisense.edj",
                             "example_group"))
     {
        int err = edje_object_load_error_get(edje);
        const char *errmsg = edje_load_error_str(err);
        EINA_LOG_ERR("could not load 'example_group' from multisense.edj: %s",
                     errmsg);

        evas_object_del(edje);
        return NULL;
     }

   evas_object_move(edje, 0, 0);
   evas_object_resize(edje, WIDTH, HEIGHT);
   evas_object_show(edje);
   return edje;
}

int main()
{
   Ecore_Evas *window;
   Evas *canvas;
   Evas_Object *edje;

   ecore_evas_init();
   edje_init();

   window = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!window)
     {
        EINA_LOG_CRIT("could not create window.");
        return -1;
     }
   canvas = ecore_evas_get(window);

   edje = create_my_group(canvas);
   if (!edje)
     return -2;

   ecore_evas_show(window);
   ecore_main_loop_begin();

   evas_object_del(edje);
   ecore_evas_free(window);

   edje_shutdown();
   ecore_evas_shutdown();

   return 0;
}

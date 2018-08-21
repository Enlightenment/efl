/**
 * Simple Edje example for layouting parts with anchors.
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one). See stdout/stderr for output.
 *
 * @verbatim
 * edje_cc edje-anchors.edc && gcc -o edje-anchors edje-anchors.c `pkg-config --libs --cflags evas ecore ecore-evas edje`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define EINA_UNUSED
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>
#include <libgen.h>

#define WIDTH 400
#define HEIGHT 400

int
main(int argc EINA_UNUSED, char **argv)
{
   char path[PATH_MAX] = { 0, };

   ecore_init();
   ecore_evas_init();
   edje_init();

   Ecore_Evas *ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   Evas *e = ecore_evas_get(ee);
   ecore_evas_show(ee);

   Evas_Object *bg = evas_object_rectangle_add(e);
   evas_object_color_set(bg, 64, 64, 64, 255);
   evas_object_resize(bg, WIDTH, HEIGHT);
   evas_object_show(bg);

   snprintf(path, sizeof(path), "%s/edje-anchors.edj", dirname(*argv));

   Evas_Object *edje = edje_object_add(e);
   edje_object_file_set(edje, path, "main");
   evas_object_resize(edje, WIDTH, HEIGHT);
   evas_object_show(edje);

   ecore_main_loop_begin();

   edje_shutdown();
   ecore_evas_shutdown();
   ecore_shutdown();

   return 0;
}

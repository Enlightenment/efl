/**
 * Simple Edje example illustrating swallow functions.
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one). See stdout/stderr for output.
 *
 * @verbatim
 * edje_cc swallow.edc && gcc -o edje-swallow2 edje-swallow2.c `pkg-config --libs --cflags evas ecore ecore-evas edje`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define EINA_UNUSED
#endif

#ifndef PACKAGE_DATA_DIR
#define PACKAGE_DATA_DIR "."
#endif

#ifndef PACKAGE_SOURCE_DATA_DIR
#define PACKAGE_SOURCE_DATA_DIR "."
#endif


#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#define WIDTH  (300)
#define HEIGHT (300)

static void
_on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

int
main(int argc EINA_UNUSED, char *argv[] EINA_UNUSED)
{
   const char  *edje_file = PACKAGE_DATA_DIR"/swallow.edj";
   const char  *img_file = PACKAGE_SOURCE_DATA_DIR"/bubble.png";
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *bg;
   Evas_Object *img;
   Evas_Object *obj;
   Evas_Object *edje_obj;
   Evas_Load_Error err;

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   if (!edje_init())
     goto shutdown_ecore_evas;

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!ee) goto shutdown_edje;

   ecore_evas_callback_delete_request_set(ee, _on_delete);

   ecore_evas_title_set(ee, "Edje Swallow Example");

   evas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(bg, 0, 0); /* at canvas' origin */
   evas_object_resize(bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(bg);
   ecore_evas_data_set(ee, "background", bg);

   ecore_evas_object_associate(ee,bg, ECORE_EVAS_OBJECT_ASSOCIATE_BASE);

   edje_obj = edje_object_add(evas);

   edje_object_file_set(edje_obj, edje_file, "example_group");
   evas_object_move(edje_obj, 20, 20);
   evas_object_resize(edje_obj, WIDTH - 40, HEIGHT - 40);
   evas_object_show(edje_obj);

   img = evas_object_image_filled_add(evas);
   evas_object_image_file_set(img, img_file, NULL);

   err = evas_object_image_load_error_get(img);

   if (err != EVAS_LOAD_ERROR_NONE)
   {
      fprintf(stderr, "could not load image '%s'. error string is \"%s\"\n",
              img_file, evas_load_error_str(err));
     goto shutdown_edje;
   }

   edje_object_part_swallow(edje_obj, "part_one", img);

   obj = edje_object_part_swallow_get(edje_obj, "part_one");

   if(obj == img)
      printf("Swallowing worked!\n");

   ecore_evas_show(ee);

   ecore_main_loop_begin();

   ecore_evas_free(ee);
   ecore_evas_shutdown();
   edje_shutdown();

   return EXIT_SUCCESS;

 shutdown_edje:
   edje_shutdown();
 shutdown_ecore_evas:
   ecore_evas_shutdown();

   return EXIT_FAILURE;
}

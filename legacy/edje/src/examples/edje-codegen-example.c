/**
 * @since 1.8.0
 * Simple Edje example illustrating the edje_codegen usage.
 *
 * edje_codegen is a tool that generates code to acessing the parts and programs
 * with the keyword "api" of a specified group. This tool make easier working
 * with edje avoiding common errors caused by typos when acessing the parts
 * and/or programs.
 *
 * To use the edje_codegen:
 * edje_codegen --prefix <myapp_myobj> <input.edj> <a_group> <source.c> <header.h>
 *
 * In case of this example:
 * edje_codegen --prefix=codegen_example codegen.edj group_example \
	codegen_example_generated.c codegen_example_generated.h
 *
 * @verbatim
 * edje_cc codegen.edc && edje_codegen --prefix=codegen_example codegen.edj \
 * example_group codegen_example_generated.c codegen_example_generated.h
 * gcc -c codegen_example_generated.c `pkg-config --libs --cflags ecore-evas edje`
 * gcc -o edje-codegen-example codegen_example_generated.o \
 * edje-codegen-example.c `pkg-config --libs --cflags ecore-evas edje`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define __UNUSED__
#endif

#include "codegen_example_generated.h"

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#define WIDTH  (300)
#define HEIGHT (300)

static void
_on_delete(Ecore_Evas *ee)
{
   ecore_main_loop_quit();
}

static void
_on_mouse_over(void *data, Evas_Object *obj, const char *emission,
	       const char *source)
{
    static int i = 0;

    printf("Mouse over, source: %s - emission: %s\n",
	   source, emission);
    if (i++ == 5)
      {
	 Evas_Object *rect = codegen_example_part_two_get(obj);
	 evas_object_color_set(rect, 0, 255, 0, 255);
	 codegen_example_part_below_over_callback_del_full(obj,_on_mouse_over,
							   NULL);
      }
}

int
main(int argc, char *argv[])
{
   char         edje_file_path[PATH_MAX];
   const char  *edje_file = "codegen.edj";
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *bg;
   Evas_Object *edje_obj;
   Evas_Object *red_rect;
   Eina_Prefix *pfx;

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   if (!edje_init())
     goto shutdown_ecore_evas;

   pfx = eina_prefix_new(argv[0], main,
                         "EDJE_EXAMPLES",
                         "edje/examples",
                         edje_file,
                         PACKAGE_BIN_DIR,
                         PACKAGE_LIB_DIR,
                         PACKAGE_DATA_DIR,
                         PACKAGE_DATA_DIR);
   if (!pfx)
     goto shutdown_edje;

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!ee)
     goto free_prefix;

   ecore_evas_callback_delete_request_set(ee, _on_delete);
   ecore_evas_title_set(ee, "Edje text Example");

   evas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(bg, 0, 0); /* at canvas' origin */
   evas_object_resize(bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(bg);
   ecore_evas_object_associate(ee, bg, ECORE_EVAS_OBJECT_ASSOCIATE_BASE);

   edje_obj = edje_object_add(evas);
   edje_object_file_set(edje_obj, edje_file, "example_group");
   evas_object_move(edje_obj, 20, 20);
   evas_object_resize(edje_obj, WIDTH - 40, HEIGHT - 40);
   evas_object_show(edje_obj);

   codegen_example_part_one_set(edje_obj, "CODEGEN_EXAMPLE");
   codegen_example_part_below_over_callback_add(edje_obj, _on_mouse_over, NULL);

   red_rect = evas_object_rectangle_add(evas);
   evas_object_color_set(red_rect, 255, 0, 0, 255); /* white bg */
   codegen_example_part_two_set(edje_obj, red_rect);
   evas_object_show(red_rect);

   ecore_evas_show(ee);

   ecore_main_loop_begin();

   eina_prefix_free(pfx);
   ecore_evas_free(ee);
   ecore_evas_shutdown();
   edje_shutdown();

   return EXIT_SUCCESS;

 free_prefix:
   eina_prefix_free(pfx);
 shutdown_edje:
   edje_shutdown();
 shutdown_ecore_evas:
   ecore_evas_shutdown();

   return EXIT_FAILURE;
}

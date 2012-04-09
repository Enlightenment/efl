#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#define __UNUSED__
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>
#include <stdio.h>

#define WIDTH     (700)
#define HEIGHT    (700)

static void
_on_delete(Ecore_Evas *ee)
{
   ecore_main_loop_quit();
}

int
main(int argc __UNUSED__, char **argv)
{
   char edje_file_path[PATH_MAX];
   const char *edje_file = "animations2.edj";
   Evas *evas;
   Ecore_Evas *ee;
   Eina_Prefix *pfx;
   Evas_Object *edje_obj;
   Evas_Object *bg;

   if (!ecore_evas_init()) return EXIT_FAILURE;

   if (!edje_init()) goto shutdown_ecore_evas;

   pfx = eina_prefix_new(argv[0], main, "EDJE_EXAMPLES", "edje/examples",
                         edje_file, PACKAGE_BIN_DIR, PACKAGE_LIB_DIR,
                         PACKAGE_DATA_DIR, PACKAGE_DATA_DIR);

   if (!pfx) goto shutdown_edje;

   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);

   if (!ee) goto eina_prefix_free;

   ecore_evas_callback_delete_request_set(ee, _on_delete);
   ecore_evas_title_set(ee, "Animations");

   evas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 255, 255, 255, 255);
   evas_object_move(bg, 0, 0);
   evas_object_resize(bg, WIDTH, HEIGHT);
   evas_object_show(bg);

   ecore_evas_object_associate(ee, bg, ECORE_EVAS_OBJECT_ASSOCIATE_BASE);
   evas_object_focus_set(bg, EINA_TRUE);

   edje_obj = edje_object_add(evas);

   snprintf(edje_file_path, sizeof(edje_file_path), "%s/examples/%s",
            eina_prefix_data_get(pfx), edje_file);

   if (!edje_object_file_set(edje_obj, edje_file_path, "animation_group"))
     {
        int err = edje_object_load_error_get(edje_obj);
        const char *errmsg = edje_load_error_str(err);
        fprintf(stderr, "Could not load the edje file - reason:%s\n", errmsg);
        goto eina_prefix_free;
     }

   evas_object_resize(edje_obj, 700, 700);
   evas_object_move(edje_obj, 50, 50);
   evas_object_show(edje_obj);

   ecore_evas_show(ee);

   ecore_main_loop_begin();

   eina_prefix_free(pfx);
   ecore_evas_free(ee);
   edje_shutdown();
   ecore_evas_shutdown();

   return EXIT_SUCCESS;

   eina_prefix_free: eina_prefix_free(pfx);

   shutdown_edje: edje_shutdown();

   shutdown_ecore_evas: ecore_evas_shutdown();

   return EXIT_FAILURE;
}

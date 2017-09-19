//Compile with:
// edje_cc animations2.edc && gcc -o animations2 animations2.c `pkg-config --libs --cflags ecore ecore-evas edje evas`

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#define EINA_UNUSED
#endif

#ifndef PACKAGE_DATA_DIR
#define PACKAGE_DATA_DIR "."
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>
#include <stdio.h>

#define WIDTH     (700)
#define HEIGHT    (700)

static void
_on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

int
main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   const char *edje_file = PACKAGE_DATA_DIR"/animations2.edj";
   Evas *evas;
   Ecore_Evas *ee;
   Evas_Object *edje_obj;
   Evas_Object *bg;

   if (!ecore_evas_init()) return EXIT_FAILURE;

   if (!edje_init()) goto shutdown_ecore_evas;

   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);

   if (!ee) goto shutdown_edje;

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

   fprintf(stderr, "loading edje file; %s\n", edje_file);

   if (!edje_object_file_set(edje_obj, edje_file, "animation_group"))
     {
        int err = edje_object_load_error_get(edje_obj);
        const char *errmsg = edje_load_error_str(err);
        fprintf(stderr, "Could not load the edje file - reason:%s\n", errmsg);
        goto shutdown_edje;
     }

   evas_object_resize(edje_obj, 700, 700);
   evas_object_move(edje_obj, 50, 50);
   evas_object_show(edje_obj);

   ecore_evas_show(ee);

   ecore_main_loop_begin();

   ecore_evas_free(ee);
   edje_shutdown();
   ecore_evas_shutdown();

   return EXIT_SUCCESS;

   shutdown_edje: edje_shutdown();

   shutdown_ecore_evas: ecore_evas_shutdown();

   return EXIT_FAILURE;
}

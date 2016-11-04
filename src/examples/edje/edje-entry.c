/**
 * Edje example for a entry (editable text)
 *
 * @verbatim
 * edje_cc entry.edc && gcc -o edje-entry edje-entry.c `pkg-config --libs --cflags evas ecore ecore-evas edje`
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

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#define WIDTH  (300)
#define HEIGHT (300)

#define KEY_BG_OBJ "bg_obj"
#define KEY_EDJE_OBJ "edje_obj"

#define GROUPNAME_MAIN "example/main"
#define PARTNAME_TEXT "example/text"

static void
_on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

/* here just to keep our example's window size and background image's
 * size in synchrony */
static void
_on_canvas_resize(Ecore_Evas *ee)
{
   Evas_Object *bg, *edj;
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   bg = ecore_evas_data_get(ee, KEY_BG_OBJ);
   evas_object_resize(bg, w, h);
   edj = ecore_evas_data_get(ee, KEY_EDJE_OBJ);
   evas_object_resize(edj, w, h);
}

static void
_setup_evas_object(Ecore_Evas *ee, Evas_Object *obj, const char *key)
{
   evas_object_move(obj, 0, 0);
   evas_object_resize(obj, WIDTH, HEIGHT);
   evas_object_show(obj);
   ecore_evas_data_set(ee, key, obj);
}

int
main(int argc EINA_UNUSED, char *argv[] EINA_UNUSED)
{
   const char *edje_file = PACKAGE_DATA_DIR"/entry.edj";
   Evas_Object *bg, *edje_obj;
   Ecore_Evas *ee;
   Evas *evas;

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   if (!edje_init())
     goto shutdown_ecore_evas;

   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!ee) goto shutdown_edje;

   ecore_evas_callback_delete_request_set(ee, _on_delete);
   ecore_evas_callback_resize_set(ee, _on_canvas_resize);
   ecore_evas_title_set(ee, "Edje Entry");

   evas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 210, 210, 210, 255);
   _setup_evas_object(ee, bg, KEY_BG_OBJ);

   edje_obj = edje_object_add(evas);

   edje_object_file_set(edje_obj, edje_file, GROUPNAME_MAIN);
   _setup_evas_object(ee, edje_obj, KEY_EDJE_OBJ);

   /* important to focus it or it won't receive key down strokes */
   evas_object_focus_set(edje_obj, EINA_TRUE);

   edje_object_part_text_set(edje_obj, PARTNAME_TEXT, "Type here : ");
   edje_object_part_text_cursor_end_set(edje_obj, PARTNAME_TEXT,
                                        EDJE_CURSOR_MAIN);

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

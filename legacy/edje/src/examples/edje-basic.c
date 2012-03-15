/**
 * Simple Edje example illustrating the very basic functions of the
 * library
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one). See stdout/stderr for output.
 *
 * @verbatim
 * gcc -o edje-basic edje-basic.c `pkg-config --libs --cflags evas ecore ecore-evas edje`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define __UNUSED__
#endif

#include <stdio.h>

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#define WIDTH  (300)
#define HEIGHT (300)

static const char commands[] = \
  "commands are:\n"
  "\ts - change Edje's global scaling factor\n"
  "\tr - change center rectangle's scaling factor\n"
  "\tEsc - exit\n"
  "\th - print help\n";

static void
_on_keydown(void        *data,
            Evas        *evas __UNUSED__,
            Evas_Object *o __UNUSED__,
            void        *einfo)
{
   Evas_Event_Key_Down *ev;
   Evas_Object         *edje_obj;

   ev = (Evas_Event_Key_Down *)einfo;
   edje_obj = (Evas_Object *)data;

   if (strcmp(ev->keyname, "h") == 0) /* print help */
     {
        fprintf(stdout, commands);
        return;
     }
   else if (strcmp(ev->keyname, "s") == 0) /* global scaling factor */
     {
        double scale = edje_scale_get();

        printf("got scale %f\n", scale);

        if (scale != 1.0) scale = 1.0;
        else scale = 2.0;

        edje_scale_set(scale);

        fprintf(stdout, "Setting global scaling factor to %f.\n", scale);

        return;
     }
   else if (strcmp(ev->keyname, "r") == 0) /* individual scaling factor */
     {
        double scale = edje_object_scale_get(edje_obj);

        printf("got scale %f\n", scale);

        if (!scale) scale = 1.0;
        else if (scale == 1.0) scale = 2.0;
        else scale = 0.0;

        edje_object_scale_set(edje_obj, scale);

        fprintf(stdout, "Setting center rectangle's scaling factor to %f.\n",
                scale);

        return;
     }
   else if (!strcmp(ev->keyname, "Escape"))
     ecore_main_loop_quit();
   else
     {
        printf("unhandled key: %s\n", ev->keyname);
        fprintf(stdout, commands);
     }
}

static void
_on_delete(Ecore_Evas *ee __UNUSED__)
{
   ecore_main_loop_quit();
}

int
main(int argc __UNUSED__, char *argv[])
{
   char         border_img_path[PATH_MAX];
   char         edje_file_path[PATH_MAX];
   const char  *edje_file = "basic.edj";
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *bg;
   Evas_Object *border;
   Evas_Object *edje_obj;
   Eina_Prefix *pfx;
   int          x;
   int          y;
   int          w;
   int          h;

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
   ecore_evas_title_set(ee, "Edje Basics Example");

   evas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(bg, 0, 0); /* at canvas' origin */
   evas_object_resize(bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(bg);
   ecore_evas_object_associate(ee, bg, ECORE_EVAS_OBJECT_ASSOCIATE_BASE);

   evas_object_focus_set(bg, EINA_TRUE);

   edje_obj = edje_object_add(evas);

   snprintf(edje_file_path, sizeof(edje_file_path),
            "%s/examples/%s", eina_prefix_data_get(pfx), edje_file);
   printf("%s\n", edje_file_path);
   /* exercising Edje loading error, on purpose */
   if (!edje_object_file_set(edje_obj, edje_file_path, "unexistant_group"))
     {
        int err = edje_object_load_error_get(edje_obj);
        const char *errmsg = edje_load_error_str(err);
        fprintf(stderr, "Could not load 'unexistant_group' from basic.edj:"
                        " %s\n", errmsg);
     }

   if (!edje_object_file_set(edje_obj, edje_file_path, "example_group"))
     {
        int err = edje_object_load_error_get(edje_obj);
        const char *errmsg = edje_load_error_str(err);
        fprintf(stderr, "Could not load 'example_group' from basic.edj: %s\n",
                errmsg);

        evas_object_del(edje_obj);
        goto free_prefix;
     }

   fprintf(stdout, "Loaded Edje object bound to group 'example_group' from"
                   " file basic.edj with success!\n");

   evas_object_move(edje_obj, 20, 20);
   evas_object_resize(edje_obj, WIDTH - 40, HEIGHT - 40);
   evas_object_show(edje_obj);

   evas_object_event_callback_add(bg, EVAS_CALLBACK_KEY_DOWN, _on_keydown, edje_obj);

   snprintf(border_img_path, sizeof(border_img_path),
            "%s/edje/examples/red.png", eina_prefix_data_get(pfx));

   /* this is a border around the Edje object above, here just to
    * emphasize its geometry */
   border = evas_object_image_filled_add(evas);
   evas_object_image_file_set(border, border_img_path, NULL);
   evas_object_image_border_set(border, 2, 2, 2, 2);
   evas_object_image_border_center_fill_set(border, EVAS_BORDER_FILL_NONE);

   evas_object_resize(border, WIDTH - 40 + 4, HEIGHT - 40 + 4);
   evas_object_move(border, 20 - 2, 20 - 2);
   evas_object_show(border);

   fprintf(stdout, "'example_data' data field in group 'example_group' has "
                   "the value: %s\n", edje_object_data_get(edje_obj,
                                                           "example_data"));

   fprintf(stdout, "Testing if 'part_one' part exists: %s\n",
           edje_object_part_exists(edje_obj, "part_one") ? "yes!" : "no");

   edje_object_part_geometry_get(edje_obj, "part_one", &x, &y, &w, &h);
   fprintf(stdout, "The geometry of that part inside the Edje object's area "
                   "is: x = %d, y = %d, w = %d, h = %d\n", x, y, w, h);

   evas_object_color_get(edje_object_part_object_get(edje_obj, "part_one"),
                         &x, &y, &w, &h);
   fprintf(stdout, "That part's color components are: r = %d, g = %d, b = %d,"
                   " a = %d\n", x, y, w, h);

   edje_object_size_max_get(edje_obj, &w, &h);
   fprintf(stdout, "The Edje object's max. size is: %d, %d\n", w, h);

   edje_object_size_min_get(edje_obj, &w, &h);
   fprintf(stdout, "The Edje object's min. size is: %d, %d\n", w, h);

   edje_object_size_min_calc(edje_obj, &w, &h);
   fprintf(stdout, "The Edje object's min. size reported by min. size"
                   " calculation is: w = %d, h = %d\n", w, h);

   edje_object_size_min_restricted_calc(edje_obj, &w, &h, 500, 500);
   fprintf(stdout, "The Edje object's min. size reported by *restricted* "
                   "min. size calculation is: w = %d, h = %d\n", w, h);

   edje_object_parts_extends_calc(edje_obj, &x, &y, &w, &h);
   fprintf(stdout, "The Edje object's \"extended\" geometry is: x = %d, "
                   "y = %d, w = %d, h = %d\n", x, y, w, h);

   fprintf(stdout, commands);

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

/**
 * Simple Edje example illustrating text functions.
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one). See stdout/stderr for output.
 *
 * @verbatim
 * edje_cc text.edc && gcc -o edje-text edje-text.c `pkg-config --libs --cflags ecore-evas edje`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#define __UNUSED__
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#define WIDTH  (300)
#define HEIGHT (300)

static const char *edje_file_path = PACKAGE_EXAMPLES_DIR "/text.edj";

static Ecore_Evas *ee;
static Evas_Object *bg;

static void
_on_destroy(Ecore_Evas *ee __UNUSED__)
{
   ecore_main_loop_quit();
}

static void
_cb(void *data, Evas_Object *obj, const char *part)
{
   printf("text: %s\n", edje_object_part_text_unescaped_get(obj, part));
}

int
main(void)
{
   Evas_Object *edje_obj, *rect, *obj;
   Evas *evas;

   ecore_evas_init();
   edje_init();

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);

   ecore_evas_callback_destroy_set(ee, _on_destroy);
   ecore_evas_title_set(ee, "Edje text Example");
   ecore_evas_show(ee);

   evas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(bg, 0, 0); /* at canvas' origin */
   evas_object_resize(bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(bg);
   ecore_evas_object_associate(ee, bg, ECORE_EVAS_OBJECT_ASSOCIATE_BASE);

   edje_obj = edje_object_add(evas);

   edje_object_file_set(edje_obj, edje_file_path, "example_group");
   evas_object_move(edje_obj, 20, 20);
   evas_object_resize(edje_obj, WIDTH - 40, HEIGHT - 40);
   evas_object_show(edje_obj);

   edje_object_text_change_cb_set(edje_obj, _cb, NULL);
   edje_object_part_text_set(edje_obj, "part_one", "one");
   edje_object_part_text_set(edje_obj, "part_two", "<b>two");

   edje_object_part_text_select_allow_set(edje_obj, "part_two", EINA_TRUE);
   edje_object_part_text_select_all(edje_obj, "part_two");
   printf("selection: %s\n", edje_object_part_text_selection_get(edje_obj, "part_two"));
   edje_object_part_text_select_none(edje_obj, "part_two");
   printf("selection: %s\n", edje_object_part_text_selection_get(edje_obj, "part_two"));

   ecore_main_loop_begin();

   ecore_evas_free(ee);
   ecore_evas_shutdown();
   edje_shutdown();
   return 0;
}

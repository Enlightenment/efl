/**
 * Simple Edje example illustrating text functions.
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one). See stdout/stderr for output.
 *
 * @verbatim
 * edje_cc -md ~/efl/src/examples/edje/ text.edc && gcc -o edje-text edje-text.c `pkg-config --libs --cflags ecore-evas edje evas ecore`
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
#include <locale.h>

#define WIDTH  (300)
#define HEIGHT (300)

static int lang_idx = 0;
static const char *lang[] = {
  "en_IN",
  "ta_IN",
  "hi_IN"
};

static void
_on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void
_on_text_change(void *data EINA_UNUSED, Evas_Object *obj, const char *part)
{
   printf("text: %s\n", edje_object_part_text_unescaped_get(obj, part));
}

static void
_on_mouse_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *event_info EINA_UNUSED)
{
   lang_idx = (lang_idx + 1) % (sizeof (lang)/ sizeof (lang[0]));
   fprintf(stderr, "Setting lang to '%s'\n", lang[lang_idx]);
   setenv("LANGUAGE", lang[lang_idx], 1);
   edje_language_set(lang[lang_idx]);
}
int
main(int argc EINA_UNUSED, char *argv[] EINA_UNUSED)
{
   const char  *edje_file = PACKAGE_DATA_DIR"/text.edj";
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *bg;
   Evas_Object *edje_obj;

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   if (!edje_init())
     goto shutdown_ecore_evas;

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!ee) goto shutdown_edje;

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

   edje_language_set("en_IN");
   edje_object_text_change_cb_set(edje_obj, _on_text_change, NULL);
   edje_object_part_text_set(edje_obj, "part_two", "<b>Click here");

   edje_object_part_text_select_allow_set(edje_obj, "part_two", EINA_TRUE);
   edje_object_part_text_select_all(edje_obj, "part_two");
   printf("selection: %s\n", edje_object_part_text_selection_get(edje_obj, "part_two"));
   edje_object_part_text_select_none(edje_obj, "part_two");
   printf("selection: %s\n", edje_object_part_text_selection_get(edje_obj, "part_two"));

   evas_object_event_callback_add(edje_obj, EVAS_CALLBACK_MOUSE_DOWN, _on_mouse_down, NULL);
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

/**
 * Simple Edje example illustrating text functions.
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one). See stdout/stderr for output.
 *
 * @verbatim
 * edje_cc -md . text.edc && gcc -o edje-text edje-text.c `pkg-config --libs --cflags ecore-evas edje evas ecore eo`
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
#include "Eo.h"

#define WIDTH  (500)
#define HEIGHT (500)

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
   char *str = edje_object_part_text_unescaped_get(obj, part);
   printf("text: %s\n", str);
   free(str);
}

static void
_on_mouse_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *o, void *event_info EINA_UNUSED)
{
   static char envbuf[20]; // doesn't have to be static, but a good habit
   char *env;
   lang_idx = (lang_idx + 1) % (sizeof (lang) / sizeof (lang[0]));
   fprintf(stderr, "Setting lang of this edje object to '%s'\n", lang[lang_idx]);

   // unfortunately dealing with env vars portably means using putenv()
   // which has issues that lead to complexity like below. the envbuf is
   // static because in general  it's a good habit when dealing with putenv()
   // but in this case it doesn't need to be. it's good to show good habits
   // at any rate. read up pn putenv() and how it takes the string pointer
   // directly into the env and takes "ownership" (but will never actually
   // free it if its an allocated string etc.).
   env = getenv("LANGUAGE");
   if (env) env = strdup(env);
   snprintf(envbuf, sizeof(envbuf), "LANGUAGE=%s", lang[lang_idx]);
   putenv(envbuf);

   edje_object_language_set(o, lang[lang_idx]);

   snprintf(envbuf, sizeof(envbuf), "LANGUAGE=%s", env ? env : "");
   putenv(envbuf);
   free(env);
}

static void
_on_mouse_down_text(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *event_info EINA_UNUSED)
{
   static char *env_lang_str = NULL;
   char *s;

   lang_idx = (lang_idx + 1) % (sizeof (lang)/ sizeof (lang[0]));
   fprintf(stderr, "Setting lang to '%s'\n", lang[lang_idx]);
   s = malloc(10 + strlen(lang[lang_idx]));
   if (s)
     {
        strcpy(s, "LANGUAGE=");
        strcpy(s + 9, lang[lang_idx]);
        putenv(s);
        if (env_lang_str) free(env_lang_str);
        env_lang_str = s;
     }
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
   Evas_Object *edje_obj_one;
   Evas_Object *edje_obj_two;
   Evas_Object *edje_obj_three;

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   if (!edje_init())
     goto shutdown_ecore_evas;

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!ee) goto shutdown_edje;

   ecore_evas_callback_delete_request_set(ee, _on_delete);
   ecore_evas_title_set(ee, "Edje Text Example");

   evas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(bg, 0, 0); /* at canvas' origin */
   evas_object_resize(bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(bg);
   ecore_evas_object_associate(ee, bg, ECORE_EVAS_OBJECT_ASSOCIATE_BASE);

   edje_obj = edje_object_add(evas);
   edje_object_file_set(edje_obj, edje_file, "example_group");
   evas_object_move(edje_obj, 0, 20);
   evas_object_resize(edje_obj, WIDTH - 40, HEIGHT - 40);
   evas_object_show(edje_obj);
   putenv("LANGUAGE=en_IN");
   edje_object_language_set(edje_obj, "en_IN");
   edje_object_text_change_cb_set(edje_obj, _on_text_change, NULL);
   edje_object_part_text_set(edje_obj, "part_two", "<b>Click here");
   edje_object_part_text_select_allow_set(edje_obj, "part_two", EINA_TRUE);
   edje_object_part_text_select_all(edje_obj, "part_two");
   printf("selection: %s\n", edje_object_part_text_selection_get(edje_obj, "part_two"));
   edje_object_part_text_select_none(edje_obj, "part_two");
   printf("selection: %s\n", edje_object_part_text_selection_get(edje_obj, "part_two"));
   evas_object_event_callback_add(edje_obj, EVAS_CALLBACK_MOUSE_DOWN, _on_mouse_down, NULL);

   edje_obj_one = edje_object_add(evas);
   edje_object_file_set(edje_obj_one, edje_file, "example_group1");
   evas_object_move(edje_obj_one, 0, 50);
   evas_object_resize(edje_obj_one, WIDTH - 40, HEIGHT - 40);
   evas_object_show(edje_obj_one);
   edje_object_language_set(edje_obj_one, "en_IN");
   edje_object_text_change_cb_set(edje_obj_one, _on_text_change, NULL);
   edje_object_part_text_set(edje_obj_one, "part_two", "<b>Click here");
   edje_object_part_text_select_allow_set(edje_obj_one, "part_two", EINA_TRUE);
   edje_object_part_text_select_all(edje_obj_one, "part_two");
   printf("selection: %s\n", edje_object_part_text_selection_get(edje_obj_one, "part_two"));
   edje_object_part_text_select_none(edje_obj_one, "part_two");
   printf("selection: %s\n", edje_object_part_text_selection_get(edje_obj_one, "part_two"));
   evas_object_event_callback_add(edje_obj_one, EVAS_CALLBACK_MOUSE_DOWN, _on_mouse_down, NULL);

   //Generic Language change
   edje_obj_two = edje_object_add(evas);
   edje_object_file_set(edje_obj_two, edje_file, "example_group2");
   evas_object_move(edje_obj_two, 0, 250);
   evas_object_resize(edje_obj_two, WIDTH - 40, HEIGHT - 40);
   evas_object_show(edje_obj_two);
   edje_language_set("en_IN");
   edje_object_text_change_cb_set(edje_obj_two, _on_text_change, NULL);
   edje_object_part_text_set(edje_obj_two, "part_two", "<b>Click here");
   edje_object_part_text_select_allow_set(edje_obj_two, "part_two", EINA_TRUE);
   edje_object_part_text_select_all(edje_obj_two, "part_two");
   printf("selection: %s\n", edje_object_part_text_selection_get(edje_obj_two, "part_two"));
   edje_object_part_text_select_none(edje_obj_two, "part_two");
   printf("selection: %s\n", edje_object_part_text_selection_get(edje_obj_two, "part_two"));
   evas_object_event_callback_add(edje_obj_two, EVAS_CALLBACK_MOUSE_DOWN, _on_mouse_down_text, NULL);

   edje_obj_three = edje_object_add(evas);
   edje_object_file_set(edje_obj_three, edje_file, "example_group3");
   evas_object_move(edje_obj_three, 0, 350);
   evas_object_resize(edje_obj_three, WIDTH - 40, HEIGHT - 40);
   evas_object_show(edje_obj_three);
   edje_object_text_change_cb_set(edje_obj_three, _on_text_change, NULL);
   edje_object_part_text_set(edje_obj_three, "part_two", "<b>Click here");
   edje_object_part_text_select_allow_set(edje_obj_three, "part_two", EINA_TRUE);
   edje_object_part_text_select_all(edje_obj_three, "part_two");
   printf("selection: %s\n", edje_object_part_text_selection_get(edje_obj_three, "part_two"));
   edje_object_part_text_select_none(edje_obj_three, "part_two");
   printf("selection: %s\n", edje_object_part_text_selection_get(edje_obj_three, "part_two"));
   evas_object_event_callback_add(edje_obj_three, EVAS_CALLBACK_MOUSE_DOWN, _on_mouse_down_text, NULL);

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

/**
 * Simple Edje example illustrating animations functions.
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one). See stdout/stderr for output.
 *
 * @verbatim
 * edje_cc animations.edc && gcc -o edje-animations edje-animations.c `pkg-config --libs --cflags evas ecore ecore-evas edje`
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

#define WIDTH  (400)
#define HEIGHT (300)

static const char commands[] = \
  "commands are:\n"
  "\t+ - increase frametime\n"
  "\t- - decrease frametime\n"
  "\t= - status of the animation\n"
  "\ts - pause\n"
  "\tp - play\n"
  "\tf - freeze one object\n"
  "\tF - freeze all objects\n"
  "\tt - thaw one object\n"
  "\tT - thaw all objects\n"
  "\ta - start animation of one object\n"
  "\tA - stop animation of one object\n"
  "\tEsc - exit\n"
  "\th - print help\n";

static double frametime = 1.0 / 30.0; /* default value */

static void
_on_delete_cb(Ecore_Evas *ee EINA_UNUSED)
{
    ecore_main_loop_quit();
}

static void
_on_canvas_resize(Ecore_Evas *ee)
{
   Evas_Object *bg;
   Evas_Object *edje_obj;
   int          w;
   int          h;

   bg = ecore_evas_data_get(ee, "background");
   edje_obj = ecore_evas_data_get(ee, "edje_obj");
   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   evas_object_resize(bg, w, h);
   evas_object_resize(edje_obj, w, h);
}

static void
_on_key_down(void *data EINA_UNUSED, Evas *evas EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Evas_Event_Key_Down *ev;
   double               ft;

   ev = (Evas_Event_Key_Down *)event_info;

   if (!strcmp(ev->key, "h"))
     {
        printf(commands);
        return;
     }
   else if (!strcmp(ev->key, "plus"))
     {
        frametime *= 2.0;
        printf("Increasing frametime to: %f\n", frametime);
        edje_frametime_set(frametime);
     }
   else if (!strcmp(ev->key, "minus"))
     {
        frametime /= 2.0;
        printf("Decreasing frametime to: %f\n", frametime);
        edje_frametime_set(frametime);
     }
   else if (!strcmp(ev->key, "equal"))
     {
        ft = edje_frametime_get();
        printf("Frametime: %f\n", ft);
        if (edje_object_play_get(obj))
          printf("Object is playing\n");
        else
          printf("Object was paused\n");
        if (edje_object_animation_get(obj))
          printf("Animation is running\n");
        else
          printf("Animation was stopped\n");
     }
   else if (!strcmp(ev->key, "s"))
     {
        edje_object_play_set(obj, EINA_FALSE);
        printf("Pausing the object\n");
     }
   else if (!strcmp(ev->key, "p"))
     {
        edje_object_play_set(obj, EINA_TRUE);
        printf("Playing the object\n");
     }
   else if (!strcmp(ev->key, "f"))
       printf("Freezing object. Count: %d\n", edje_object_freeze(obj));
   else if (!strcmp(ev->key, "t"))
       printf("Thawing object. Count: %d\n", edje_object_thaw(obj));
   else if (!strcmp(ev->key, "F"))
     {
        edje_freeze();
        printf("Freezing all objects\n");
     }
   else if (!strcmp(ev->key, "T"))
     {
        edje_thaw();
        printf("Thawing all objects\n");
     }
   else if (!strcmp(ev->key, "a"))
     {
        edje_object_animation_set(obj, EINA_TRUE);
        printf("Starting the animation in the Edje object\n");
     }
   else if (!strcmp(ev->key, "A"))
     {
        edje_object_animation_set(obj, EINA_FALSE);
        printf("Stopping the animation in the Edje object\n");
     }
   else if (!strcmp(ev->key, "Escape"))
     ecore_main_loop_quit();
   else
     {
        printf("unhandled key: %s\n", ev->key);
        printf(commands);
     }
}

int
main(int argc EINA_UNUSED, char *argv[] EINA_UNUSED)
{
   const char  *edje_file = PACKAGE_DATA_DIR"/animations.edj";
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

   ecore_evas_callback_delete_request_set(ee, _on_delete_cb);
   ecore_evas_callback_resize_set(ee, _on_canvas_resize);
   ecore_evas_title_set(ee, "Edje Animations Example");

   evas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(bg, 0, 0); /* at canvas' origin */
   evas_object_resize(bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(bg);
   ecore_evas_data_set(ee, "background", bg);

   edje_obj = edje_object_add(evas);

   edje_object_file_set(edje_obj, edje_file, "animations_group");
   evas_object_move(edje_obj, 0, 0);
   evas_object_resize(edje_obj, WIDTH, HEIGHT);
   evas_object_show(edje_obj);
   ecore_evas_data_set(ee, "edje_obj", edje_obj);

   evas_object_event_callback_add(edje_obj, EVAS_CALLBACK_KEY_DOWN,
                                  _on_key_down, NULL);
   evas_object_focus_set(edje_obj, EINA_TRUE);

   printf(commands);

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

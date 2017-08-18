/**
 * Example of basic usage of Ecore-Evas and transparent windows.
 *
 * The evas-buffer-simple.c example shows how to manually create and
 * manage buffers, but mentioned that real code would use higher level
 * functionality from Ecore's Ecore-Evas submodule.  This example
 * shows the use of that submodule to create a simple Evas canvas.
 *
 * This example also demonstrates how to create transparent windows with
 * Evas.  Like in most other graphics software, transparency and
 * translucency is calculated using an integer parameter called an
 * 'alpha channel'.  Support for alpha transparency is platform
 * dependent, and even where it is supported it may not be needed,
 * so Ecore-Evas provides a simple API to turn it on and off, which
 * this example will demonstrate by allowing it to be toggled via
 * the 'n' and 'm' keys on the keyboard.
 *
 * The keyboard input will introduce 'event handling' in Ecore-Evas, but
 * only briefly - we'll be exploring event handling in later examples
 * more deeply.
 *
 * @verbatim
 * gcc -o evas-transparent evas-transparent.c `pkg-config --libs --cflags evas ecore ecore-evas eina`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define WIDTH  (640)
#define HEIGHT (480)

static const char *commands = \
  "commands are:\n"
  "\tn - turn on alpha"
  "\tm - turn off alpha"
  "\th - print help\n";

struct example_data
{
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *bg;
};

static struct example_data d;

/* Keyboard event callback routine, to enable toggling transparency on
 * and off.
 */
static void
_on_keydown(void        *data EINA_UNUSED,
            Evas        *evas EINA_UNUSED,
            Evas_Object *o EINA_UNUSED,
            void        *einfo)
{
   Evas_Event_Key_Down *ev = einfo;

   if (strcmp(ev->key, "h") == 0)
     {
        /* h - print help */
        printf("%s",  commands);
        return;
     }

   if (strcmp(ev->key, "n") == 0)
     {
        /* n - turn alpha transparency on */
        ecore_evas_alpha_set(d.ee, EINA_TRUE);

        printf("turn on alpha\n");
        return;
     }

   if (strcmp(ev->key, "m") == 0)
     {
        /* m - turn alpha transparency off */
        ecore_evas_alpha_set(d.ee, EINA_FALSE);

        printf("turn off alpha\n");
        return;
     }
}

static void
_on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   /* Communicate to ecore that the application is finished.  Calling
    * this routine allows any pending events to get processed and allow
    * the main loop to finish the current iteration.
    */
   ecore_main_loop_quit();
}

int
main(void)
{
   /* In other examples, evas_init() has been used to turn Evas on.  In this
    * example we're using Ecore-Evas' init routine, which takes care of
    * bringing up Evas.
    */
   if (!ecore_evas_init())
     return EXIT_FAILURE;

   /* In the evas-buffer-simple.c example, we coded our own
    * create_canvas() routine.  Here we make use of Ecore-Evas's
    * ecore_evas_new() routine to do it.  The first argument of this
    * function is used to specify the name of an engine we wish to use;
    * by passing NULL instead, we are requesting a window with an Evas
    * canvas using the first engine available.
    *
    * The next arguments set the canvas's position to 0,0 and its
    * height and width to our desired size.
    *
    * The last parameter for ecore_evas_new() allows setting extra
    * options, but for this example we don't need anything special
    * so just pass NULL.
    */
   d.ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!d.ee)
     goto panic;

   /* Like other windowing systems, Ecore-Evas provides hooks for a
    * number of different events.  We can register our own functions to
    * be called when the events occur in our window.  Here we'll register
    * a callback to be triggered when the window is closed.
    */
   ecore_evas_callback_delete_request_set(d.ee, _on_delete);

   /* As in evas-buffer-simple.c, we need to explicitly 'unhide' our
    * Evas objects.  But here we'll use the Ecore-Evas API to do this,
    * as it also manages some of the underlying device state.
    */
   ecore_evas_show(d.ee);

   /* Retrieve a pointer to the canvas we created. */
   d.evas = ecore_evas_get(d.ee);

   /* Add a black background rectangle */
   d.bg = evas_object_rectangle_add(d.evas);
   evas_object_color_set(d.bg, 0, 0, 0, 0);
   evas_object_show(d.bg);

   /* Callbacks can also be set on Evas objects.  We'll add a keyboard
    * handler routine to the background rectangle, for processing user
    * key hits.
    */
   evas_object_focus_set(d.bg, EINA_TRUE);
   evas_object_event_callback_add(d.bg, EVAS_CALLBACK_KEY_DOWN,
                                  _on_keydown, NULL);

   /* Run the application until ecore_main_loop_quit() gets called by
    * our _on_delete() handler.  While this function is active, it will
    * repeatedly call ecore_main_loop_iterate() to iterate through
    * various internal processes, checking for keyboard input, updating
    * the screen as needed, and so forth.
    */
   ecore_main_loop_begin();

   /* With the application finished, we now direct the Ecore and Evas
    * libraries to perform final cleanup and terminate the system.
    */
   ecore_evas_shutdown();
   return 0;

panic:
   fprintf(stderr, "error: Requires at least one Evas engine built and linked"
                   " to ecore-evas for this example to run properly.\n");
   return -2;
}

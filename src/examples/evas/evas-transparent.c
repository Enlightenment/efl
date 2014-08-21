/**
 * Simple Evas example illustrating a trasnparent window
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

struct exemple_data
{
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *bg;
};

static struct exemple_data d;

static void
_on_keydown(void        *data EINA_UNUSED,
            Evas        *evas EINA_UNUSED,
            Evas_Object *o EINA_UNUSED,
            void        *einfo)
{
   Evas_Event_Key_Down *ev = einfo;

   if (strcmp(ev->key, "h") == 0) /* print help */
     {
        fprintf(stdout, "%s",  commands);
        return;
     }

   if (strcmp(ev->key, "n") == 0)
     {
        ecore_evas_alpha_set(d.ee, EINA_TRUE);

        fprintf(stdout, "turn on alpha\n");
        return;
     }

   if (strcmp(ev->key, "m") == 0)
     {
        ecore_evas_alpha_set(d.ee, EINA_FALSE);

        fprintf(stdout, "turn off alpha\n");
        return;
     }
}

static void
_on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

int
main(void)
{
   if (!ecore_evas_init())
     return EXIT_FAILURE;

   d.ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!d.ee)
     goto panic;

   ecore_evas_callback_delete_request_set(d.ee, _on_delete);
   ecore_evas_show(d.ee);

   d.evas = ecore_evas_get(d.ee);

   d.bg = evas_object_rectangle_add(d.evas);
   evas_object_color_set(d.bg, 0, 0, 0, 0);
   evas_object_show(d.bg);

   evas_object_focus_set(d.bg, EINA_TRUE);
   evas_object_event_callback_add(d.bg, EVAS_CALLBACK_KEY_DOWN,
                                  _on_keydown, NULL);
   ecore_main_loop_begin();
   ecore_evas_shutdown();
   return 0;

panic:
   fprintf(stderr, "You got to have at least one evas engine built and linked"
                   " up to ecore-evas for this example to run properly.\n");
   return -2;
}


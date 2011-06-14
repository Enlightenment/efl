/**
 * Simple Evas example illustrating how to interact with canvas
 * events and other canvas operations.
 *
 * You'll need at least one engine built for it (excluding the buffer
 * one) and the png image loader also built. See stdout/stderr for
 * output.
 *
 * @verbatim
 * gcc -o evas-events evas-events.c `pkg-config --libs --cflags ecore-evas`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H

#include "config.h"
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <stdio.h>
#include <errno.h>

#define WIDTH  (320)
#define HEIGHT (240)

static const char *img_path = PACKAGE_EXAMPLES_DIR "/enlightenment.png";

struct test_data
{
   Ecore_Evas  *ee;
   Evas        *canvas;
   Eina_Bool    obscured;
   Evas_Object *img, *bg;
   Ecore_Timer *resize_timer, *freeze_timer;
};

static struct test_data d = {0};

/* here just to keep our example's window size and background image's
 * size in synchrony */
static void
_canvas_resize_cb(Ecore_Evas *ee)
{
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   evas_object_resize(d.bg, w, h);
}

static void
/* called when our rectangle gets focus */
_object_focus_in_cb(void *data __UNUSED__,
                    Evas *e,
                    void *event_info)
{
   fprintf(stdout, "An object got focused: %s\n",
           evas_object_name_get(event_info));

   fprintf(stdout, "Let's recheck it: %s\n",
           evas_object_name_get(evas_focus_get(e)));
}

static void
_render_flush_cb(void *data __UNUSED__,
                 Evas *e __UNUSED__,
                 void *event_info __UNUSED__)
{
   fprintf(stdout, "Canvas is about to flush its rendering pipeline!\n");
}

static Eina_Bool
/* put some action in the canvas */
_resize_cb(void *data __UNUSED__)
{
   int w, h, cw, ch;

   evas_object_geometry_get(d.img, NULL, NULL, &w, &h);
   ecore_evas_geometry_get(d.ee, NULL, NULL, &cw, &ch);

   if (w < cw)
     evas_object_resize(d.img, cw, ch);
   else
     evas_object_resize(d.img, cw / 2, ch / 2);

   return EINA_TRUE; /* re-issue the timer */
}

static Eina_Bool
/* let's have our events back */
_thaw_cb(void *data __UNUSED__)
{
   fprintf(stdout, "Canvas was frozen %d times, now thawing.\n",
           evas_event_freeze_get(d.canvas));
   evas_event_thaw(d.canvas);
   return EINA_FALSE; /* do not re-issue the timer */
}

static void
_on_keydown(void        *data __UNUSED__,
            Evas        *evas,
            Evas_Object *o __UNUSED__,
            void        *einfo)
{
   const Evas_Modifier *mods;
   Evas_Event_Key_Down *ev = einfo;

   fprintf(stdout, "we've got key input: %s\n", ev->keyname);

   if (strcmp(ev->keyname, "a") == 0) /* toggle animation timer */
     {
        if (d.resize_timer != NULL)
          {
             fprintf(stdout, "Stopping animation timer\n");
             ecore_timer_del(d.resize_timer);
             d.resize_timer = NULL;
          }
        else
          {
             fprintf(stdout, "Re-issuing animation timer\n");
             d.resize_timer = ecore_timer_add(2, _resize_cb, NULL);
          }
        return;
     }

   if (strcmp(ev->keyname, "d") == 0) /* delete canvas' callbacks */
     {
        fprintf(stdout, "Deleting canvas event callbacks\n");
        evas_event_callback_del_full(evas, EVAS_CALLBACK_RENDER_FLUSH_PRE,
                                     _render_flush_cb, NULL);
        evas_event_callback_del_full(
          evas, EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN,
          _object_focus_in_cb, NULL);
        return;
     }

   if (strcmp(ev->keyname, "f") == 0) /* freeze input for 3 seconds */
     {
        fprintf(stdout, "Freezing input for 3 seconds\n");
        evas_event_freeze(evas);
        d.freeze_timer = ecore_timer_add(3, _thaw_cb, NULL);
        return;
     }

   mods = evas_key_modifier_get(evas);
   if (evas_key_modifier_is_set(mods, "Control") &&
       (strcmp(ev->keyname, "o") == 0)) /* add an obscured
                                         * rectangle to the middle
                                         * of the canvas */
     {
        fprintf(stdout, "Toggling obscured rectangle on canvas\n");
        if (!d.obscured)
          {
             int w, h;
             evas_output_viewport_get(evas, NULL, NULL, &w, &h);
             evas_obscured_rectangle_add(evas, w / 4, h / 4, w / 2, h / 2);
          }
        else
          {
             int w, h;
             Eina_Rectangle *rect;
             Eina_List *updates, *l;

             evas_output_viewport_get(evas, NULL, NULL, &w, &h);
             evas_obscured_clear(evas);

             /* we have to flag a damage region here because
              * evas_obscured_clear() doesn't change the canvas'
              * state. we'd have to wait for an animation step, for
              * example, to get the result, without it */
             evas_damage_rectangle_add(evas, 0, 0, w, h);

             updates = evas_render_updates(evas);

             EINA_LIST_FOREACH(updates, l, rect)
               {
                  fprintf(stdout, "Rectangle (%d, %d, %d, %d) on canvas got a"
                          " rendering update.\n", rect->x, rect->y, rect->w,
                          rect->h);
               }
             evas_render_updates_free(updates);
          }
        d.obscured = !d.obscured;
     }
}

int
main(void)
{
   int err;

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   d.ee = ecore_evas_new(NULL, 10, 10, WIDTH, HEIGHT, NULL);
   if (!d.ee)
     goto error;

   ecore_evas_callback_resize_set(d.ee, _canvas_resize_cb);
   ecore_evas_show(d.ee);

   /* the canvas pointer, de facto */
   d.canvas = ecore_evas_get(d.ee);

   evas_event_callback_add(d.canvas, EVAS_CALLBACK_RENDER_FLUSH_PRE,
                           _render_flush_cb, NULL);
   if (evas_alloc_error() != EVAS_ALLOC_ERROR_NONE)
     {
        fprintf(stderr, "ERROR: Callback registering failed! Abort!\n");
        goto panic;
     }

   evas_event_callback_add(d.canvas, EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN,
                           _object_focus_in_cb, NULL);
   if (evas_alloc_error() != EVAS_ALLOC_ERROR_NONE)
     {
        fprintf(stderr, "ERROR: Callback registering failed! Abort!\n");
        goto panic;
     } /* two canvas event callbacks */

   d.bg = evas_object_rectangle_add(d.canvas);
   evas_object_name_set(d.bg, "our dear rectangle");
   evas_object_color_set(d.bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(d.bg, 0, 0); /* at canvas' origin */
   evas_object_resize(d.bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(d.bg);

   evas_object_focus_set(d.bg, EINA_TRUE);
   evas_object_event_callback_add(
     d.bg, EVAS_CALLBACK_KEY_DOWN, _on_keydown, NULL);

   d.img = evas_object_image_filled_add(d.canvas);
   evas_object_image_file_set(d.img, img_path, NULL);
   err = evas_object_image_load_error_get(d.img);
   if (err != EVAS_LOAD_ERROR_NONE)
     {
        goto panic;
     }
   else
     {
        evas_object_move(d.img, 0, 0);
        evas_object_resize(d.img, WIDTH, HEIGHT);
        evas_object_show(d.img);
     }

   d.resize_timer = ecore_timer_add(2, _resize_cb, NULL);

   ecore_main_loop_begin();

   ecore_evas_free(d.ee);
   ecore_evas_shutdown();
   return 0;

error:
   fprintf(stderr, "you got to have at least one evas engine built and linked"
                   " up to ecore-evas for this example to run properly.\n");
panic:
   ecore_evas_shutdown();
   return -1;
}

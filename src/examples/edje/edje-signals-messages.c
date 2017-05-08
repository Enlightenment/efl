/**
 * Simple Edje example illustrating the Edje signals and messages
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one). See stdout/stderr for output.
 *
 * @verbatim
 * edje_cc signals-messages.edc && gcc -o edje-signals-messages edje-signals-messages.c `pkg-config --libs --cflags evas ecore ecore-evas edje`
 * @endverbatim
 */

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

#define WIDTH     (300)
#define HEIGHT    (300)

#define MSG_COLOR 1
#define MSG_TEXT  2

static const char commands[] = \
  "commands are:\n"
  "\tt - toggle right rectangle's visibility\n"
  "\tEsc - exit\n"
  "\th - print help\n";

static Eina_Bool right_rect_show = EINA_TRUE;

static void
_on_keydown(void        *data,
            Evas        *evas EINA_UNUSED,
            Evas_Object *o EINA_UNUSED,
            void        *einfo)
{
   Evas_Event_Key_Down *ev;
   Evas_Object         *edje_obj;

   ev = (Evas_Event_Key_Down *)einfo;
   edje_obj = (Evas_Object *)data;

   if (!strcmp(ev->key, "h")) /* print help */
     {
        printf(commands);
        return;
     }
   else if (!strcmp(ev->key, "t")) /* toggle right rectangle's visibility */
     {
        char buf[1024];

        right_rect_show = !right_rect_show;
        snprintf(buf, sizeof(buf), "part_right,%s",
                 right_rect_show ? "show" : "hide");

        printf("emitting %s\n", buf);

        edje_object_signal_emit(edje_obj, buf, "");

        return;
     }
   else if (!strcmp(ev->key, "Escape"))
     ecore_main_loop_quit();
   else
     {
        printf("unhandled key: %s\n", ev->key);
        printf(commands);
     }
}

static void
_on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

/* print signals coming from theme */
static void
_sig_print(const char *emission,
           const char *source)
{
   printf("Signal %s coming from part %s!\n", emission, source);
}

static void
_on_mouse_wheel(void        *data EINA_UNUSED,
                  Evas_Object *obj EINA_UNUSED,
                  const char  *emission,
                  const char  *source)
{
   _sig_print(emission, source);
}

/* mouse over signals */
static void
_on_mouse_over(void        *data EINA_UNUSED,
               Evas_Object *edje_obj,
               const char  *emission,
               const char  *source)
{
   int i;

   _sig_print(emission, source);

   Edje_Message_Int_Set *msg = malloc(sizeof(*msg) + 3 * sizeof(int));
   msg->count = 4;
   for (i = 0; i < 4; i++)
     msg->val[i] = rand() % 256;

   edje_object_message_send(edje_obj, EDJE_MESSAGE_INT_SET, MSG_COLOR, msg);

   free(msg);
}

/* print out received message string */
static void
_message_handle(void             *data EINA_UNUSED,
                Evas_Object      *obj EINA_UNUSED,
                Edje_Message_Type type,
                int               id,
                void             *msg)
{
   Edje_Message_String *m;

   if (type != EDJE_MESSAGE_STRING) return;
   if (id != MSG_TEXT) return;

   m = msg;

   printf("String message received: %s\n", m->str);
}

int
main(int argc EINA_UNUSED, char *argv[] EINA_UNUSED)
{
   const char  *img_file = PACKAGE_DATA_DIR"/red.png";
   const char  *edje_file = PACKAGE_DATA_DIR"/signals-messages.edj";
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *bg;
   Evas_Object *edje_obj;
   Evas_Object *border;

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   if (!edje_init())
     goto shutdown_ecore_evas;

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!ee) goto shutdown_edje;

   ecore_evas_callback_delete_request_set(ee, _on_delete);
   ecore_evas_title_set(ee, "Edje Signals and Messages Example");

   evas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(bg, 0, 0); /* at canvas' origin */
   evas_object_resize(bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(bg);
   evas_object_focus_set(bg, EINA_TRUE);
   ecore_evas_object_associate(ee, bg, ECORE_EVAS_OBJECT_ASSOCIATE_BASE);

   edje_obj = edje_object_add(evas);

   if (!edje_object_file_set(edje_obj, edje_file, "example_group"))
     {
        int err = edje_object_load_error_get(edje_obj);
        const char *errmsg = edje_load_error_str(err);
        fprintf(stderr, "Could not load 'example_group' from "
                        "signals-messages.edj: %s\n", errmsg);

        evas_object_del(edje_obj);
        goto shutdown_edje;
     }

   edje_object_signal_callback_add(edje_obj, "mouse,wheel,*", "part_left",
                                   _on_mouse_wheel, NULL);

   edje_object_signal_callback_add(edje_obj, "mouse,over", "part_right",
                                   _on_mouse_over, NULL);

   edje_object_message_handler_set(edje_obj, _message_handle, NULL);

   evas_object_move(edje_obj, 20, 20);
   evas_object_resize(edje_obj, WIDTH - 40, HEIGHT - 40);
   evas_object_show(edje_obj);

   evas_object_event_callback_add(bg, EVAS_CALLBACK_KEY_DOWN, _on_keydown, edje_obj);

   /* this is a border around the Edje object above, here just to
    * emphasize its geometry */
   border = evas_object_image_filled_add(evas);
   evas_object_image_file_set(border, img_file, NULL);
   evas_object_image_border_set(border, 2, 2, 2, 2);
   evas_object_image_border_center_fill_set(border, EVAS_BORDER_FILL_NONE);

   evas_object_resize(border, WIDTH - 40 + 4, HEIGHT - 40 + 4);
   evas_object_move(border, 20 - 2, 20 - 2);
   evas_object_repeat_events_set(border, EINA_TRUE);
   evas_object_show(border);

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

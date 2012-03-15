/**
 * Simple Edje example illustrating the Edje signals and messages
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one). See stdout/stderr for output.
 *
 * @verbatim
 * gcc -o edje-signals-messages edje-signals-messages.c `pkg-config --libs --cflags evas ecore ecore-evas edje`
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
            Evas        *evas __UNUSED__,
            Evas_Object *o __UNUSED__,
            void        *einfo)
{
   Evas_Event_Key_Down *ev;
   Evas_Object         *edje_obj;

   ev = (Evas_Event_Key_Down *)einfo;
   edje_obj = (Evas_Object *)data;

   if (!strcmp(ev->keyname, "h")) /* print help */
     {
        fprintf(stdout, commands);
        return;
     }
   else if (!strcmp(ev->keyname, "t")) /* toggle right rectangle's visibility */
     {
        char buf[1024];

        right_rect_show = !right_rect_show;
        snprintf(buf, sizeof(buf), "part_right,%s",
                 right_rect_show ? "show" : "hide");

        printf("emitting %s\n", buf);

        edje_object_signal_emit(edje_obj, buf, "");

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

/* print signals coming from theme */
static void
_sig_print(const char *emission,
           const char *source)
{
   fprintf(stdout, "Signal %s coming from part %s!\n", emission, source);
}

static void
_on_mouse_wheel(void        *data __UNUSED__,
                  Evas_Object *obj __UNUSED__,
                  const char  *emission,
                  const char  *source)
{
   _sig_print(emission, source);
}

/* mouse over signals */
static void
_on_mouse_over(void        *data __UNUSED__,
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
_message_handle(void             *data __UNUSED__,
                Evas_Object      *obj __UNUSED__,
                Edje_Message_Type type,
                int               id,
                void             *msg)
{
   Edje_Message_String *m;

   if (type != EDJE_MESSAGE_STRING) return;
   if (id != MSG_TEXT) return;

   m = msg;

   fprintf(stdout, "String message received: %s\n", m->str);
}

int
main(int argc __UNUSED__, char *argv[])
{
   char         border_img_path[PATH_MAX];
   char         edje_file_path[PATH_MAX];
   const char  *edje_file = "signals-messages.edj";
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *bg;
   Evas_Object *edje_obj;
   Evas_Object *border;
   Eina_Prefix *pfx;

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
   evas_object_focus_set(bg, EINA_TRUE);
   ecore_evas_object_associate(ee, bg, ECORE_EVAS_OBJECT_ASSOCIATE_BASE);

   edje_obj = edje_object_add(evas);

   snprintf(edje_file_path, sizeof(edje_file_path),
            "%s/examples/%s", eina_prefix_data_get(pfx), edje_file);
   if (!edje_object_file_set(edje_obj, edje_file_path, "example_group"))
     {
        int err = edje_object_load_error_get(edje_obj);
        const char *errmsg = edje_load_error_str(err);
        fprintf(stderr, "Could not load 'example_group' from "
                        "signals-messages.edj: %s\n", errmsg);

        evas_object_del(edje_obj);
        goto free_prefix;
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
   evas_object_repeat_events_set(border, EINA_TRUE);
   evas_object_show(border);

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

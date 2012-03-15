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

#define WIDTH  (300)
#define HEIGHT (300)

static const char commands[] = \
  "commands are:\n"
  "\te - change te edje base\n"
  "\tl - change to lua base\n"
  "\tm - send message\n"
  "\ts - send signal\n"
  "\tEsc - exit\n"
  "\th - print help\n";

static void
_on_keydown(void        *data,
            Evas        *evas __UNUSED__,
            Evas_Object *o __UNUSED__,
            void        *einfo)
{
   Ecore_Evas          *ee;
   Evas_Event_Key_Down *ev;
   Evas_Object         *edje_obj;
   char                *edje_file_path;

   ee = (Ecore_Evas *)data;
   ev = (Evas_Event_Key_Down *)einfo;
   edje_obj = ecore_evas_data_get(ee, "edje_obj");
   edje_file_path = ecore_evas_data_get(ee, "file_path");

   if (!strcmp(ev->keyname, "h"))
     {
        fprintf(stdout, commands);
        return;
     }
   else if (!strcmp(ev->keyname, "e"))
     {
      if (!edje_object_file_set(edje_obj, edje_file_path, "plain/edje/group"))
        {
           int err = edje_object_load_error_get(edje_obj);
           const char *errmsg = edje_load_error_str(err);

           fprintf(stderr, "Could not load 'plain/edje/group' from sigtest.edj: %s\n", errmsg);
        }
      else
         fprintf(stdout, "Loaded Edje object bound to group 'plain/edje/group' from"
                         " file sigtest.edj with success!\n");
        return;
     }
   else if (!strcmp(ev->keyname, "l"))
     {
      if (!edje_object_file_set(edje_obj, edje_file_path, "lua_base"))
        {
           int err = edje_object_load_error_get(edje_obj);
           const char *errmsg = edje_load_error_str(err);

           fprintf(stderr, "Could not load 'lua_base' from sigtest.edj: %s\n", errmsg);
        }
      else
         fprintf(stdout, "Loaded Edje object bound to group 'lua_base' from"
                         " file sigtest.edj with success!\n");
        return;
     }
   else if (!strcmp(ev->keyname, "m"))
     {
        Edje_Message_String *msg = malloc(sizeof(*msg));

	fprintf(stdout, "\n");
        msg->str = strdup("C message text");
        edje_object_message_send(edje_obj, EDJE_MESSAGE_STRING, 2, msg);
        free(msg);
	fprintf(stdout, "C message sent\n");
        return;
     }
   else if (!strcmp(ev->keyname, "s"))
     {
	fprintf(stdout, "\n");
        edje_object_signal_emit(edje_obj, "C signal 1", "Csource");
        edje_object_signal_emit(edje_obj, "bubbles_lua:C signal 2", "Csource");
	fprintf(stdout, "C signal sent\n");
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
_on_message(void *data __UNUSED__, Evas_Object *obj __UNUSED__, Edje_Message_Type type, int id, void *msg)
{
   int i;

   fprintf(stdout, "C::message id=%d type=%d ", id, type);
   switch (type)
     {
        case EDJE_MESSAGE_NONE :
          {
             fprintf(stdout, " NONE");
             break;
          }

        case EDJE_MESSAGE_SIGNAL :
          {
             fprintf(stdout, " SIGNAL ");
             break;
          }

        case EDJE_MESSAGE_STRING :
          {
             Edje_Message_String *mmsg = msg;

             fprintf(stdout, " STRING %s", mmsg->str);
             break;
          }

        case EDJE_MESSAGE_INT :
          {
             Edje_Message_Int *mmsg = msg;

             fprintf(stdout, " INT %d", mmsg->val);
             break;
          }

        case EDJE_MESSAGE_FLOAT :
          {
             Edje_Message_Float *mmsg = msg;

             fprintf(stdout, " FLOAT %f", mmsg->val);
             break;
          }

        case EDJE_MESSAGE_STRING_SET :
          {
             Edje_Message_String_Set *mmsg = msg;

             fprintf(stdout, " STRING_SET -\n");
             for (i = 0; i < mmsg->count; i++)
                fprintf(stdout, "  %s\n", mmsg->str[i]);
             break;
          }

        case EDJE_MESSAGE_INT_SET :
          {
             Edje_Message_Int_Set *mmsg = msg;

             fprintf(stdout, " INT_SET -\n");
             for (i = 0; i < mmsg->count; i++)
                fprintf(stdout, "  %d\n", mmsg->val[i]);
             break;
          }

        case EDJE_MESSAGE_FLOAT_SET :
          {
             Edje_Message_Float_Set *mmsg = msg;

             fprintf(stdout, " FLOAT_SET -\n");
             for (i = 0; i < mmsg->count; i++)
                fprintf(stdout, "  %f\n", mmsg->val[i]);
             break;
          }

        case EDJE_MESSAGE_STRING_INT :
          {
             Edje_Message_String_Int *mmsg = msg;

             fprintf(stdout, " STRING_INT %s %d", mmsg->str, mmsg->val);
             break;
          }

        case EDJE_MESSAGE_STRING_FLOAT :
          {
             Edje_Message_String_Float *mmsg = msg;

             fprintf(stdout, " STRING_FLOAT %s %f", mmsg->str, mmsg->val);
             break;
          }

        case EDJE_MESSAGE_STRING_INT_SET :
          {
             Edje_Message_String_Int_Set *mmsg = msg;

             fprintf(stdout, " STRING_INT_SET %s -\n", mmsg->str);
             for (i = 0; i < mmsg->count; i++)
                fprintf(stdout, "  %d\n", mmsg->val[i]);
             break;
          }

        case EDJE_MESSAGE_STRING_FLOAT_SET :
          {
             Edje_Message_String_Float_Set *mmsg = msg;

             fprintf(stdout, " STRING_FLOAT_SET %s -\n", mmsg->str);
             for (i = 0; i < mmsg->count; i++)
                fprintf(stdout, "  %f\n", mmsg->val[i]);
             break;
          }
     }
   fprintf(stdout, "\n");
}

static void
_on_signal(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const char  *emission, const char  *source)
{
   fprintf(stdout, "C::signal sig=|%s| src=|%s|\n", emission, source);
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
   const char  *edje_file = "sigtest.edj";
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *bg;
   Evas_Object *border;
   Evas_Object *edje_obj;
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
   ecore_evas_title_set(ee, "Signals and wessages tester");

   evas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(bg, 0, 0); /* at canvas' origin */
   evas_object_resize(bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(bg);
   ecore_evas_object_associate(ee, bg, ECORE_EVAS_OBJECT_ASSOCIATE_BASE);

   evas_object_focus_set(bg, EINA_TRUE);

   edje_obj = edje_object_add(evas);

   edje_object_message_handler_set(edje_obj, _on_message, NULL);
   edje_object_signal_callback_add(edje_obj, "*", "*", _on_signal, NULL);

   snprintf(edje_file_path, sizeof(edje_file_path),
            "%s/examples/%s", eina_prefix_data_get(pfx), edje_file);
   if (!edje_object_file_set(edje_obj, edje_file_path, "lua_base"))
     {
        int err = edje_object_load_error_get(edje_obj);
        const char *errmsg = edje_load_error_str(err);
        fprintf(stderr, "Could not load 'lua_base' from sigtest.edj: %s\n",
                errmsg);

        evas_object_del(edje_obj);
        goto free_prefix;
     }

   fprintf(stdout, "Loaded Edje object bound to group 'lua_base' from"
                   " file sigtest.edj with success!\n");

   evas_object_move(edje_obj, 20, 20);
   evas_object_resize(edje_obj, WIDTH - 40, HEIGHT - 40);
   evas_object_show(edje_obj);
   ecore_evas_data_set(ee, "edje_obj", edje_obj);
   ecore_evas_data_set(ee, "file_path", edje_file_path);

   evas_object_event_callback_add(bg, EVAS_CALLBACK_KEY_DOWN, _on_keydown, ee);

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

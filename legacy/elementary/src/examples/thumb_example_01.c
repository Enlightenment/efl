//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` thumb_example_01.c -o thumb_example_01

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
# define PACKAGE_DATA_DIR "."
#endif

static void
_generation_started_cb(void *data __UNUSED__, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   printf("thumbnail generation started.\n");
}

static void
_generation_finished_cb(void *data __UNUSED__, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   printf("thumbnail generation finished.\n");
}

static void
_generation_error_cb(void *data __UNUSED__, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   printf("thumbnail generation error.\n");
}

int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg;
   Evas_Object *thumb;
   Ethumb_Client *client;
   char buf[PATH_MAX];

   elm_need_ethumb();

   win = elm_win_add(NULL, "thumb", ELM_WIN_BASIC);
   elm_win_title_set(win, "Thumbnailer");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   client = elm_thumb_ethumb_client_get();
   if (!client)
     {
	printf("error: could not get Ethumb client.\n");
	return 1;
     }
   ethumb_client_size_set(client, 160, 160);

   thumb = elm_thumb_add(win);

   evas_object_smart_callback_add(thumb, "generate,start", _generation_started_cb, NULL);
   evas_object_smart_callback_add(thumb, "generate,stop", _generation_finished_cb, NULL);
   evas_object_smart_callback_add(thumb, "generate,error", _generation_error_cb, NULL);

   elm_thumb_editable_set(thumb, EINA_FALSE);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", PACKAGE_DATA_DIR);
   elm_thumb_file_set(thumb, buf, NULL);
   elm_thumb_reload(thumb);

   evas_object_size_hint_weight_set(thumb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, thumb);
   evas_object_show(thumb);

   evas_object_size_hint_min_set(bg, 160, 160);
   evas_object_size_hint_max_set(bg, 640, 640);
   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   elm_run(); /* and run the program now, starting to handle all
               * events, etc. */
   elm_shutdown(); /* clean up and shut down */

   /* exit code */
   return 0;
}

ELM_MAIN()

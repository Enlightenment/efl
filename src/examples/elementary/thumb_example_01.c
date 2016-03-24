//Compile with:
//gcc -g thumb_example_01.c -o thumb_example_01 `pkg-config --cflags --libs elementary`

#include <Elementary.h>

static void
_generation_started_cb(void *data, Evas_Object *o, void *event_info)
{
   printf("thumbnail generation started.\n");
}

static void
_generation_finished_cb(void *data, Evas_Object *o, void *event_info)
{
   printf("thumbnail generation finished.\n");
}

static void
_generation_error_cb(void *data, Evas_Object *o, void *event_info)
{
   printf("thumbnail generation error.\n");
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win;
   Evas_Object *thumb;
   char buf[PATH_MAX];

   elm_need_ethumb();

   elm_app_info_set(elm_main, "elementary", "images/plant_01.jpg");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("thumb", "Thumbnailer");
   elm_win_autodel_set(win, EINA_TRUE);

   thumb = elm_thumb_add(win);

   evas_object_smart_callback_add(thumb, "generate,start", _generation_started_cb, NULL);
   evas_object_smart_callback_add(thumb, "generate,stop", _generation_finished_cb, NULL);
   evas_object_smart_callback_add(thumb, "generate,error", _generation_error_cb, NULL);

   elm_thumb_size_set(thumb, 160, 160);
   elm_thumb_editable_set(thumb, EINA_FALSE);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   elm_thumb_file_set(thumb, buf, NULL);
   elm_thumb_reload(thumb);

   evas_object_size_hint_weight_set(thumb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, thumb);
   evas_object_show(thumb);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   elm_run(); /* and run the program now, starting to handle all
               * events, etc. */
   /* exit code */
   return 0;
}
ELM_MAIN()

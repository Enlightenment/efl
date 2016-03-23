//Compile with:
//gcc -o prefs_example_03 prefs_example_03.c -g `pkg-config --cflags --libs elementary`

#include <Elementary.h>

#define WIDTH (500)
#define HEIGHT (700)
#define MSG_ID_VEL (1)

static void
_update_animation(Evas_Object *prefs, Evas_Object *layout)
{
   Eina_Value value;
   Eina_Bool animation;

   elm_prefs_item_value_get(prefs, "main:config:options:animation", &value);
   eina_value_get(&value, &animation);
   if (animation)
     elm_layout_signal_emit(layout, "start", "animation");
   else
     elm_layout_signal_emit(layout, "stop", "animation");
}

static void
_update_animation_time(Evas_Object *prefs, Evas_Object *layout)
{
   Eina_Value value;
   float animation_time;
   Edje_Message_Float msg;

   elm_prefs_item_value_get(prefs, "main:config:options:animation_time", &value);
   eina_value_get(&value, &animation_time);
   if (animation_time < 0.01) animation_time = 0.01;
   msg.val = animation_time;
   edje_object_message_send(elm_layout_edje_get(layout), EDJE_MESSAGE_FLOAT,
                            MSG_ID_VEL, &msg);
}

static void
_update(Evas_Object *prefs, Evas_Object *layout)
{
   _update_animation(prefs, layout);
   _update_animation_time(prefs, layout);
}

static void
_page_loaded_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *layout = data;

   _update(obj, layout);
}

static void
_item_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
   const char *item = event_info;
   Evas_Object *layout = data;

   if (!strcmp(item, "main:config:options:animation_time"))
     _update_animation_time(obj, layout);
   else if (!strcmp(item, "main:config:options:animation"))
     _update_animation(obj, layout);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *prefs, *layout;
   Elm_Prefs_Data *prefs_data;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("Prefs", "Prefs Example 03");
   elm_win_autodel_set(win, EINA_TRUE);

   layout = elm_layout_add(win);
   elm_layout_file_set(layout, "prefs_example_03.edj", "prefs_edje");

   prefs = elm_prefs_add(win);
   evas_object_size_hint_weight_set(prefs, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(prefs, WIDTH, HEIGHT);
   evas_object_show(prefs);

   evas_object_smart_callback_add(prefs, "page,loaded", _page_loaded_cb,
                                  layout);
   evas_object_smart_callback_add(prefs, "item,changed", _item_changed_cb,
                                  layout);

   elm_prefs_autosave_set(prefs, EINA_TRUE);

   prefs_data =
      elm_prefs_data_new("./prefs_example_03.cfg", NULL, EET_FILE_MODE_READ_WRITE);

   elm_prefs_file_set(prefs, "prefs_example_03.epb", NULL);
   elm_prefs_data_set(prefs, prefs_data);

   elm_prefs_item_swallow(prefs, "main:swal", layout);

   _update_animation_time(prefs, layout);
   _update_animation(prefs, layout);

   evas_object_resize(win, WIDTH, HEIGHT);
   evas_object_show(win);

   elm_run();

   elm_prefs_data_unref(prefs_data);

   return 0;
}
ELM_MAIN()

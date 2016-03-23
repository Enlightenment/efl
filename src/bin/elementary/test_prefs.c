#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

#define WIDTH (500)
#define HEIGHT (1000)
#define MSG_ID_VEL (1)

static void
_update_animation(Evas_Object *prefs, Evas_Object *layout)
{
   Eina_Value value;
   Eina_Bool animation;

   elm_prefs_item_value_get(prefs, "main:animation", &value);
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

   elm_prefs_item_value_get(prefs, "main:animation_time", &value);
   eina_value_get(&value, &animation_time);
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
_page_loaded_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *layout = data;

   _update(obj, layout);
}

static void
_item_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
   const char *item = event_info;
   Evas_Object *layout = data;

   if (!strcmp(item, "main:animation_time"))
      _update_animation_time(obj, layout);
   else if (!strcmp(item, "main:animation"))
      _update_animation(obj, layout);
}

void
test_prefs(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bg, *prefs, *layout;
   Elm_Prefs_Data *prefs_data;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "Prefs", ELM_WIN_BASIC);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_win_title_set(win, "Prefs");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_min_set(bg, WIDTH, HEIGHT);
   evas_object_size_hint_max_set(bg, WIDTH, HEIGHT);
   evas_object_show(bg);

   layout = elm_layout_add(win);
   snprintf(buf, sizeof(buf),
            "%s/objects/test_prefs.edj", elm_app_data_dir_get());
   elm_layout_file_set(layout, buf, "prefs_edje");

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
     elm_prefs_data_new("./test_prefs.cfg", NULL, EET_FILE_MODE_READ_WRITE);

   snprintf(buf, sizeof(buf),
            "%s/objects/test_prefs.epb", elm_app_data_dir_get());

   elm_prefs_file_set(prefs, buf, NULL);
   elm_prefs_data_set(prefs, prefs_data);

   elm_prefs_item_swallow(prefs, "main:swal", layout);

   _update_animation_time(prefs, layout);
   _update_animation(prefs, layout);

   evas_object_resize(win, WIDTH, HEIGHT);
   evas_object_show(win);

   elm_prefs_data_unref(prefs_data);
}

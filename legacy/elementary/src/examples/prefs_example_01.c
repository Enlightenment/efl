//Compile with:
//gcc -o prefs_example_01 prefs_example_01.c -g `pkg-config --cflags --libs elementary`

#include <Elementary.h>

//testing ui reflection on prefs data changes
static Eina_Bool
_elm_prefs_data_change(void *data)
{
   Evas_Object *notify = data;
   Elm_Prefs_Data *prefs_data;
   Elm_Prefs_Item_Type type;
   Eina_Value value;

   prefs_data = evas_object_data_get(notify, "prefs_data");

   if (elm_prefs_data_value_get(prefs_data, "main:universe", &type, &value))
     {
        eina_value_set(&value, 42);
        elm_prefs_data_value_set(prefs_data, "main:universe", type, &value);
     }

   if (elm_prefs_data_value_get(prefs_data, "main:text", &type, &value))
     {
        eina_value_set(&value, "This is a text field (:");
        elm_prefs_data_value_set(prefs_data, "main:text", type, &value);
     }

   evas_object_show(notify);

   return ECORE_CALLBACK_RENEW;
}

static void
_page_saved_cb(void *data, Evas_Object *obj, void *event_info)
{
   const char *page = event_info;

   printf("page %s was saved!\n", page);
}

static void
_page_loaded_cb(void *data, Evas_Object *obj, void *event_info)
{
   const char *page = event_info;

   printf("page %s was loaded!\n", page);
}

static void
_item_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
   const char *item = event_info;

   printf("item %s was changed!\n", item);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *prefs, *notify, *label;
   Elm_Prefs_Data *prefs_data;

   win = elm_win_util_standard_add("prefs", "Prefs Example 01");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_win_autodel_set(win, EINA_TRUE);

   prefs = elm_prefs_add(win);
   evas_object_size_hint_weight_set(prefs, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, prefs);
   evas_object_show(prefs);

   evas_object_smart_callback_add(prefs, "page,saved", _page_saved_cb, NULL);
   evas_object_smart_callback_add(prefs, "page,loaded", _page_loaded_cb, NULL);
   evas_object_smart_callback_add(prefs, "item,changed", _item_changed_cb, win);

   elm_prefs_autosave_set(prefs, EINA_TRUE);

   prefs_data = elm_prefs_data_new("./prefs_example_01.cfg", NULL,
                                   EET_FILE_MODE_READ_WRITE);

   elm_prefs_file_set(prefs, "prefs_example_01.epb", NULL);
   elm_prefs_data_set(prefs, prefs_data);

   label = elm_label_add(win);
   elm_object_text_set(label, "Setting Values Programmatically");
   evas_object_size_hint_align_set(label, 0.5, 0.5);

   notify = elm_notify_add(win);
   elm_notify_align_set(notify, 0.5, 1);
   elm_notify_timeout_set(notify, 2);
   elm_object_content_set(notify, label);
   evas_object_size_hint_weight_set(notify, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(notify);

   evas_object_data_set(notify, "prefs_data", prefs_data);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   ecore_timer_add(5.0, _elm_prefs_data_change, notify);

   elm_run();

   elm_prefs_data_unref(prefs_data);

   return 0;
}
ELM_MAIN()

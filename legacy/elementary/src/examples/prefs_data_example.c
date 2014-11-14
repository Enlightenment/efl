//Compile with:
//gcc -o prefs_data_example prefs_data_example.c -g `pkg-config --cflags --libs elementary`

#include <Elementary.h>

static void
_item_changed_cb(void *cb_data,
                 Elm_Prefs_Data_Event_Type type,
                 Elm_Prefs_Data *data,
                 void *event_info)
{
   Elm_Prefs_Data_Event_Changed *evt = event_info;
   Eina_Bool ok;
   char *str = eina_value_to_string(evt->value);

   printf("item %s had its value changed to %s\n", evt->key, str);
   free(str);

   ok = elm_prefs_data_event_callback_del(data, type, _item_changed_cb, NULL);
   printf("elm_prefs_data_event_callback_del(): %s\n", ok ? "ok" : "fail");
}

int
elm_main(int argc, char **argv)
{
   Elm_Prefs_Data *prefs_data;
   Eina_Value value;
   Eina_Bool ok;
   int version;
   char *str;

   prefs_data = elm_prefs_data_new("./test", NULL, EET_FILE_MODE_READ_WRITE);
   printf("elm_prefs_data_new(): %s\n", prefs_data ? "ok" : "fail");

   elm_prefs_data_autosave_set(prefs_data, EINA_TRUE);

   ok = elm_prefs_data_autosave_get(prefs_data);
   printf("elm_prefs_data_autosave_get() returns: %s\n", ok ? "yes" : "no");

   eina_value_setup(&value, EINA_VALUE_TYPE_INT);
   eina_value_set(&value, 50);

   ok = elm_prefs_data_value_set(prefs_data, "position", &value);
   printf("elm_prefs_data_value_set(): %s\n", ok ? "ok" : "fail");
   str = eina_value_to_string(&value);
   printf("value for \'position\' is now %s\n", str);
   free(str);

   printf("elm_prefs_data_version_get() returns %d\n",
          version = elm_prefs_data_version_get(prefs_data));

   elm_prefs_data_version_set(prefs_data, ++version);

   elm_prefs_data_unref(prefs_data);

   prefs_data = elm_prefs_data_new("./test", NULL, EET_FILE_MODE_READ_WRITE);

   ok = elm_prefs_data_value_get(prefs_data, "position", &value);
   printf("elm_prefs_data_value_get(): %s\n", ok ? "ok" : "fail");
   str = eina_value_to_string(&value);
   printf("value for \'position\' is %s\n", str);
   free(str);

   eina_value_set(&value, 3000);

   ok = elm_prefs_data_event_callback_add
      (prefs_data, ELM_PREFS_DATA_EVENT_ITEM_CHANGED, _item_changed_cb, NULL);
   printf("elm_prefs_data_event_callback_add(): %s\n", ok ? "ok" : "fail");

   ok = elm_prefs_data_value_set(prefs_data, "position", &value);
   printf("elm_prefs_data_value_set(): %s\n", ok ? "ok" : "fail");
   str = eina_value_to_string(&value);
   printf("value for \'position\' is now %s\n", str);
   free(str);

   ok = elm_prefs_data_save(prefs_data, NULL, NULL);
   printf("elm_prefs_data_save(): %s\n", ok ? "ok" : "fail");

   elm_prefs_data_ref(prefs_data);
   elm_prefs_data_ref(prefs_data);

   printf("elm_prefs_data_version_get() returns %d\n",
          version = elm_prefs_data_version_get(prefs_data));

   elm_prefs_data_unref(prefs_data);
   elm_prefs_data_unref(prefs_data);
   elm_prefs_data_unref(prefs_data);

   return 0;
}
ELM_MAIN()

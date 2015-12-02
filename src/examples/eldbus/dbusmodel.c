//Compile with:
// gcc -o busmodel busmodel.c `pkg-config --cflags --libs eldbus ecore eina`

# include <config.h>

#include <Eldbus.h>
#include <Eldbus_Model.h>
#include <Efl.h>
#include <Ecore.h>

#define DEFAULT_BUS  "org.freedesktop.DBus"
#define DEFAULT_PATH "/"

static unsigned int children_count = 0;

static Eina_Bool
_event_interface_load_status_cb(void *data EINA_UNUSED, Eo *model,
                           const Eo_Event_Description *desc EINA_UNUSED,
                           void *event_info)
{
   Efl_Model_Load *actual_load = (Efl_Model_Load*)event_info;
   Eina_Array *properties_list;
   Eina_Array_Iterator iterator;
   Eina_Value const* property_value;
   char *property, *prop_str;
   const char *name;
   unsigned int i;

   if (EFL_MODEL_LOAD_STATUS_LOADED != actual_load->status)
     return EINA_TRUE;

   eo_do(model,
         name = eldbus_model_proxy_name_get(),
         efl_model_properties_get(&properties_list));

   printf(" -> %s\n", name);
   if (eina_array_count(properties_list))
     printf("   Properties:\n");

   EINA_ARRAY_ITER_NEXT(properties_list, i, property, iterator)
     {
        eo_do(model, efl_model_property_get(property, &property_value));
        if (property_value)
          {
             prop_str = eina_value_to_string(property_value);
          }
        printf("    * %s: %s \n", property, prop_str);
        free(prop_str);
        prop_str = NULL;
     }

   children_count--;

   if (!children_count)
     ecore_main_loop_quit();

   return EINA_FALSE;
}

static Eina_Bool
_event_load_status_cb(void *data EINA_UNUSED, Eo *model,
                           const Eo_Event_Description *desc EINA_UNUSED,
                           void *event_info)
{
   Efl_Model_Load *actual_load = (Efl_Model_Load*)event_info;
   Eina_Accessor *accessor;
   Eo *child = NULL;
   unsigned int i;

   if (EFL_MODEL_LOAD_STATUS_LOADED != actual_load->status)
     return EINA_TRUE;

   eo_do(model, efl_model_children_count_get(&children_count));
   if (children_count == 0)
     {
        printf("Don't find Interfaces\n");
        ecore_main_loop_quit();
        return EINA_FALSE;
     }

   eo_do(model, efl_model_children_slice_get(0, 0, &accessor));
   printf("\nInterfaces:\n");
   EINA_ACCESSOR_FOREACH(accessor, i, child)
     {
        eo_do(child, eo_event_callback_add(EFL_MODEL_BASE_EVENT_LOAD_STATUS, _event_interface_load_status_cb, NULL));
        eo_do(child, efl_model_load());
     }

   return EINA_FALSE;
}

int
main(int argc, char **argv EINA_UNUSED)
{
   const char *bus, *path;
   Eo *root;

   eldbus_init();

   bus = DEFAULT_BUS;
   path = DEFAULT_PATH;

   if (argc > 1) bus = argv[1];
   if (argc > 2) path = argv[2];

   root = eo_add_ref(ELDBUS_MODEL_OBJECT_CLASS, NULL,
                     eldbus_model_object_constructor(ELDBUS_CONNECTION_TYPE_SESSION,
                                                     NULL, EINA_FALSE, bus, path));

   eo_do(root,
         eo_event_callback_add(EFL_MODEL_BASE_EVENT_LOAD_STATUS, _event_load_status_cb, NULL),
         efl_model_load());

   ecore_main_loop_begin();

   eo_do(root, eo_event_callback_del(EFL_MODEL_BASE_EVENT_LOAD_STATUS, _event_load_status_cb, NULL));

   eo_unref(root);
}

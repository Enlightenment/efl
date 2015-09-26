#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eldbus_model_arguments_private.h"
#include "eldbus_model_private.h"

#include <Eina.h>
#include <Eldbus.h>

#define MY_CLASS ELDBUS_MODEL_ARGUMENTS_CLASS
#define MY_CLASS_NAME "Eldbus_Model_Arguments"

#define ARGUMENT_FORMAT "arg%u"

static void _eldbus_model_arguments_efl_model_base_properties_load(Eo *, Eldbus_Model_Arguments_Data *);
static void _eldbus_model_arguments_efl_model_base_children_load(Eo *, Eldbus_Model_Arguments_Data *);
static void _eldbus_model_arguments_unload(Eldbus_Model_Arguments_Data *);
static bool _eldbus_model_arguments_is_input_argument(Eldbus_Model_Arguments_Data *, const char *);
static bool _eldbus_model_arguments_is_output_argument(Eldbus_Model_Arguments_Data *, const char *);
static Eina_Bool _eldbus_model_arguments_property_set(Eldbus_Model_Arguments_Data *, Eina_Value *, const char *);
static unsigned int _eldbus_model_arguments_argument_index_get(Eldbus_Model_Arguments_Data *, const char *);

static void
_eldbus_model_arguments_hash_free(Eina_Value *value)
{
   eina_value_free(value);
}

static Eo_Base*
_eldbus_model_arguments_eo_base_constructor(Eo *obj, Eldbus_Model_Arguments_Data *pd)
{
   DBG("(%p)", obj);
   obj = eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());

   pd->obj = obj;
   pd->load.status = EFL_MODEL_LOAD_STATUS_UNLOADED;
   pd->properties_array = NULL;
   pd->properties_hash = eina_hash_string_superfast_new(EINA_FREE_CB(_eldbus_model_arguments_hash_free));
   pd->pending_list = NULL;
   pd->proxy = NULL;
   pd->arguments = NULL;
   pd->name = NULL;
   return obj;
}

static void
_eldbus_model_arguments_arguments_set(Eo *obj EINA_UNUSED,
                                      Eldbus_Model_Arguments_Data *pd,
                                      const Eina_List *arguments)
{
   DBG("(%p)", obj);

   Eldbus_Introspection_Argument* data;
   EINA_LIST_FREE(pd->arguments, data)
     {
        free(data);
     }
   pd->arguments = eina_list_clone(arguments);
   Eina_List* l;
   EINA_LIST_FOREACH(pd->arguments, l, data)
     {
        void* cp = malloc(sizeof(Eldbus_Introspection_Argument));
        memcpy(cp, data, sizeof(Eldbus_Introspection_Argument));
        eina_list_data_set(l, cp);
     }
}

static const Eina_List*
_eldbus_model_arguments_arguments_get(Eo *obj EINA_UNUSED,
                                      Eldbus_Model_Arguments_Data *pd)
{
   DBG("(%p)", obj);

   return pd->arguments;
}

static void
_eldbus_model_arguments_proxy_set(Eo *obj EINA_UNUSED,
                                  Eldbus_Model_Arguments_Data *pd,
                                  Eldbus_Proxy *proxy)
{
   DBG("(%p)", obj);
   EINA_SAFETY_ON_NULL_RETURN(proxy);

   pd->proxy = eldbus_proxy_ref(proxy);
}

static Eldbus_Proxy*
_eldbus_model_arguments_proxy_get(Eo *obj EINA_UNUSED,
                                  Eldbus_Model_Arguments_Data *pd)
{
   DBG("(%p)", obj);

   return pd->proxy;
}

static void
_eldbus_model_arguments_name_set(Eo *obj EINA_UNUSED,
                                 Eldbus_Model_Arguments_Data *pd,
                                 const char *name)
{
   DBG("(%p)", obj);
   EINA_SAFETY_ON_NULL_RETURN(name);

   pd->name = eina_stringshare_add(name);
}

static const char*
_eldbus_model_arguments_name_get(Eo *obj EINA_UNUSED,
                                 Eldbus_Model_Arguments_Data *pd)
{
   DBG("(%p)", obj);

   return pd->name;
}

static void
_eldbus_model_arguments_eo_base_destructor(Eo *obj, Eldbus_Model_Arguments_Data *pd)
{
   DBG("(%p)", obj);

   _eldbus_model_arguments_unload(pd);

   eina_hash_free(pd->properties_hash);

   eina_stringshare_del(pd->name);
   eldbus_proxy_unref(pd->proxy);

   Eldbus_Introspection_Argument* data;
   EINA_LIST_FREE(pd->arguments, data)
     {
        free(data);
     }
   
   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static Efl_Model_Load_Status
_eldbus_model_arguments_efl_model_base_properties_get(Eo *obj EINA_UNUSED,
                                                   Eldbus_Model_Arguments_Data *pd,
                                                   Eina_Array * const* properties_array)
{
   DBG("(%p)", obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, EFL_MODEL_LOAD_STATUS_ERROR);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->obj, EFL_MODEL_LOAD_STATUS_ERROR);

   if (!(pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES))
     {
        ERR("%s", "o not loaded.");
        return EFL_MODEL_LOAD_STATUS_ERROR;
     }

   *(Eina_Array**)properties_array = pd->properties_array;
   return pd->load.status;
}

static void
_eldbus_model_arguments_efl_model_base_properties_load(Eo *obj, Eldbus_Model_Arguments_Data *pd)
{
   DBG("(%p)", obj);

   if (pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES)
     return;

   unsigned int arguments_count = eina_list_count(pd->arguments);

   pd->properties_array = eina_array_new(arguments_count);
   EINA_SAFETY_ON_NULL_RETURN(pd->properties_array);

   for (unsigned int i = 0; i < arguments_count; ++i)
     {
        Eina_Stringshare *name = eina_stringshare_printf(ARGUMENT_FORMAT, i);
        EINA_SAFETY_ON_NULL_RETURN(name);

        Eina_Bool ret = eina_array_push(pd->properties_array, name);
        EINA_SAFETY_ON_FALSE_RETURN(ret);

        Eldbus_Introspection_Argument *arg = eina_list_nth(pd->arguments, i);
        const Eina_Value_Type *type = _dbus_type_to_eina_value_type(arg->type[0]);

        Eina_Value *value = eina_value_new(type);
        EINA_SAFETY_ON_NULL_RETURN(value);

        ret = eina_hash_add(pd->properties_hash, name, value);
        EINA_SAFETY_ON_FALSE_RETURN(ret);
     }

   efl_model_load_set(pd->obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES);
}

static Efl_Model_Load_Status
_eldbus_model_arguments_efl_model_base_property_set(Eo *obj EINA_UNUSED,
                                            Eldbus_Model_Arguments_Data *pd,
                                            const char *property,
                                            Eina_Value const* value)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(property, EFL_MODEL_LOAD_STATUS_ERROR);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EFL_MODEL_LOAD_STATUS_ERROR);
   DBG("(%p): property=%s", obj, property);

   if (!(pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES))
     return EFL_MODEL_LOAD_STATUS_ERROR;

   if (!_eldbus_model_arguments_is_input_argument(pd, property))
     {
        WRN("Property (argument) not found or it is for output only: %s", property);
        return EFL_MODEL_LOAD_STATUS_ERROR;
     }

   Eina_Value *prop_value = eina_hash_find(pd->properties_hash, property);
   EINA_SAFETY_ON_NULL_RETURN_VAL(prop_value, EFL_MODEL_LOAD_STATUS_ERROR);

   eina_value_flush(prop_value);
   Eina_Bool ret = eina_value_copy(value, prop_value);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, EFL_MODEL_LOAD_STATUS_ERROR);

   return pd->load.status;
}

static Efl_Model_Load_Status
_eldbus_model_arguments_efl_model_base_property_get(Eo *obj EINA_UNUSED,
                                            Eldbus_Model_Arguments_Data *pd,
                                            const char *property,
                                            Eina_Value const ** value)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(property, EFL_MODEL_LOAD_STATUS_ERROR);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EFL_MODEL_LOAD_STATUS_ERROR);
   DBG("(%p): property=%s", obj, property);

   if (!(pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES))
     return EFL_MODEL_LOAD_STATUS_ERROR;

   if (!_eldbus_model_arguments_is_output_argument(pd, property))
     {
        WRN("Property (argument) not found or it is for input only: %s", property);
        return EFL_MODEL_LOAD_STATUS_ERROR;
     }

   *value = eina_hash_find(pd->properties_hash, property);
   EINA_SAFETY_ON_NULL_RETURN_VAL(*value, EFL_MODEL_LOAD_STATUS_ERROR);

   return pd->load.status;
}

static void
_eldbus_model_arguments_efl_model_base_load(Eo *obj, Eldbus_Model_Arguments_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);

   eo_do(obj, efl_model_properties_load());
   eo_do(obj, efl_model_children_load());
}

static Efl_Model_Load_Status
_eldbus_model_arguments_efl_model_base_load_status_get(Eo *obj EINA_UNUSED, Eldbus_Model_Arguments_Data *pd)
{
   DBG("(%p)", obj);
   return pd->load.status;
}

static void
_eldbus_model_arguments_efl_model_base_unload(Eo *obj EINA_UNUSED, Eldbus_Model_Arguments_Data *pd)
{
   DBG("(%p)", obj);

   _eldbus_model_arguments_unload(pd);

   efl_model_load_set(pd->obj, &pd->load, EFL_MODEL_LOAD_STATUS_UNLOADED);
}

Eo *
_eldbus_model_arguments_efl_model_base_child_add(Eo *obj EINA_UNUSED, Eldbus_Model_Arguments_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);
   return NULL;
}

static Efl_Model_Load_Status
_eldbus_model_arguments_efl_model_base_child_del(Eo *obj EINA_UNUSED,
                                         Eldbus_Model_Arguments_Data *pd EINA_UNUSED,
                                         Eo *child EINA_UNUSED)
{
   DBG("(%p)", obj);
   return EFL_MODEL_LOAD_STATUS_ERROR;
}

static Efl_Model_Load_Status
_eldbus_model_arguments_efl_model_base_children_slice_get(Eo *obj EINA_UNUSED,
                                                  Eldbus_Model_Arguments_Data *pd,
                                                  unsigned start EINA_UNUSED,
                                                  unsigned count EINA_UNUSED,
                                                  Eina_Accessor **children_accessor)
{
   DBG("(%p)", obj);
   *children_accessor = NULL;
   return pd->load.status;
}

static Efl_Model_Load_Status
_eldbus_model_arguments_efl_model_base_children_count_get(Eo *obj EINA_UNUSED,
                                                  Eldbus_Model_Arguments_Data *pd,
                                                  unsigned *children_count)
{
   DBG("(%p)", obj);
   *children_count = 0;
   return pd->load.status;
}

static void
_eldbus_model_arguments_efl_model_base_children_load(Eo *obj, Eldbus_Model_Arguments_Data *pd)
{
   DBG("(%p)", obj);

   if (pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN)
     return;

   efl_model_load_set(pd->obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN);
}

static void
_eldbus_model_arguments_unload(Eldbus_Model_Arguments_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN(pd);

   Eldbus_Pending *pending;
   EINA_LIST_FREE(pd->pending_list, pending)
     eldbus_pending_cancel(pending);

   if (pd->properties_array)
     {
        unsigned int i;
        Eina_Stringshare *property;
        Eina_Array_Iterator it;
        EINA_ARRAY_ITER_NEXT(pd->properties_array, i, property, it)
          eina_stringshare_del(property);
        eina_array_free(pd->properties_array);
        pd->properties_array = NULL;
     }

   eina_hash_free_buckets(pd->properties_hash);
}

bool
eldbus_model_arguments_process_arguments(Eldbus_Model_Arguments_Data *pd,
                                         const Eldbus_Message *msg,
                                         Eldbus_Pending *pending)
{
   DBG("(%p)", pd->obj);

   pd->pending_list = eina_list_remove(pd->pending_list, pending);

   const char *error_name, *error_text;
   if (eldbus_message_error_get(msg, &error_name, &error_text))
     {
        ERR("%s: %s", error_name, error_text);
        efl_model_error_notify(pd->obj);
        return false;
     }

   Eina_Value *value_struct = eldbus_message_to_eina_value(msg);
   if (NULL == value_struct)
     {
        INF("%s", "No output arguments");
        return true;
     }

   Eina_Array *changed_properties = eina_array_new(1);

   bool result = false;
   unsigned int i = 0;
   const Eina_List *it;
   const Eldbus_Introspection_Argument *argument;
   EINA_LIST_FOREACH(pd->arguments, it, argument)
     {
        if (ELDBUS_INTROSPECTION_ARGUMENT_DIRECTION_IN != argument->direction)
          {
             const Eina_Stringshare *property = eina_array_data_get(pd->properties_array, i);
             EINA_SAFETY_ON_NULL_GOTO(property, on_error);

             Eina_Bool ret = _eldbus_model_arguments_property_set(pd, value_struct, property);
             EINA_SAFETY_ON_FALSE_GOTO(ret, on_error);

             ret = eina_array_push(changed_properties, property);
             EINA_SAFETY_ON_FALSE_GOTO(ret, on_error);
          }

        ++i;
     }

   if (eina_array_count(changed_properties))
     {
        Efl_Model_Property_Event evt = {.changed_properties = changed_properties};
        eo_do(pd->obj, eo_event_callback_call(EFL_MODEL_BASE_EVENT_PROPERTIES_CHANGED, &evt));
     }

   result = true;

on_error:
   eina_array_free(changed_properties);
   eina_value_free(value_struct);
   return result;
}

static Eina_Bool
_eldbus_model_arguments_property_set(Eldbus_Model_Arguments_Data *pd,
                                     Eina_Value *value_struct,
                                     const char *property)
{
   Eina_Value *prop_value = eina_hash_find(pd->properties_hash, property);
   EINA_SAFETY_ON_NULL_RETURN_VAL(prop_value, EINA_FALSE);

   Eina_Value value;
   Eina_Bool ret = eina_value_struct_value_get(value_struct, "arg0", &value);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, EINA_FALSE);

   eina_value_flush(prop_value);
   ret = eina_value_copy(&value, prop_value);
   eina_value_flush(&value);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, EINA_FALSE);

   return ret;
}

static bool
_eldbus_model_arguments_is_input_argument(Eldbus_Model_Arguments_Data *pd, const char *argument)
{
   const unsigned int i = _eldbus_model_arguments_argument_index_get(pd, argument);
   if (i >= eina_array_count(pd->properties_array))
     {
        WRN("Argument not found: %s", argument);
        return false;
     }

   Eldbus_Introspection_Argument *argument_introspection =
     eina_list_nth(pd->arguments, i);
   EINA_SAFETY_ON_NULL_RETURN_VAL(argument_introspection, false);

   return ELDBUS_INTROSPECTION_ARGUMENT_DIRECTION_IN == argument_introspection->direction;
}

static bool
_eldbus_model_arguments_is_output_argument(Eldbus_Model_Arguments_Data *pd, const char *argument)
{
   const unsigned int i = _eldbus_model_arguments_argument_index_get(pd, argument);
   if (i >= eina_array_count(pd->properties_array))
     {
        WRN("Argument not found: %s", argument);
        return false;
     }

   Eldbus_Introspection_Argument *argument_introspection =
     eina_list_nth(pd->arguments, i);
   EINA_SAFETY_ON_NULL_RETURN_VAL(argument_introspection, false);

   return ELDBUS_INTROSPECTION_ARGUMENT_DIRECTION_IN != argument_introspection->direction;
}

static unsigned int
_eldbus_model_arguments_argument_index_get(Eldbus_Model_Arguments_Data *pd, const char *argument)
{
   unsigned int i = 0;
   const Eina_Stringshare *name;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(pd->properties_array, i, name, it)
     {
        if (strcmp(name, argument) == 0)
          return i;
     }

   return ++i;
}

#include "eldbus_model_arguments.eo.c"

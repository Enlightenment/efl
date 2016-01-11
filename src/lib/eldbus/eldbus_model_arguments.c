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
static Eina_Bool _eldbus_model_arguments_is_input_argument(Eldbus_Model_Arguments_Data *, const char *);
static Eina_Bool _eldbus_model_arguments_is_output_argument(Eldbus_Model_Arguments_Data *, const char *);
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
_eldbus_model_arguments_constructor(Eo *obj EINA_UNUSED,
                                    Eldbus_Model_Arguments_Data *pd,
                                    Eldbus_Proxy *proxy,
                                    const char *name,
                                    const Eina_List *arguments)
{
   EINA_SAFETY_ON_NULL_RETURN(proxy);
   EINA_SAFETY_ON_NULL_RETURN(name);

   pd->proxy = eldbus_proxy_ref(proxy);
   pd->arguments = arguments;
   pd->name = eina_stringshare_add(name);
}

static void
_eldbus_model_arguments_eo_base_destructor(Eo *obj, Eldbus_Model_Arguments_Data *pd)
{
   _eldbus_model_arguments_unload(pd);

   eina_hash_free(pd->properties_hash);

   eina_stringshare_del(pd->name);
   eldbus_proxy_unref(pd->proxy);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static Efl_Model_Load_Status
_eldbus_model_arguments_efl_model_base_properties_get(Eo *obj EINA_UNUSED,
                                                      Eldbus_Model_Arguments_Data *pd,
                                                      Eina_Array * const* properties_array)
{
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
_eldbus_model_arguments_efl_model_base_properties_load(Eo *obj EINA_UNUSED, Eldbus_Model_Arguments_Data *pd)
{
   unsigned int arguments_count;
   unsigned int i;

   if (pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES)
     return;

   arguments_count = eina_list_count(pd->arguments);

   pd->properties_array = eina_array_new(arguments_count);
   EINA_SAFETY_ON_NULL_RETURN(pd->properties_array);

   for (i = 0; i < arguments_count; ++i)
     {
        Eldbus_Introspection_Argument *arg;
        const Eina_Value_Type *type;
        Eina_Stringshare *name;
        Eina_Value *value;

        name = eina_stringshare_printf(ARGUMENT_FORMAT, i);
        if (!name) continue;

        eina_array_push(pd->properties_array, name);

        arg = eina_list_nth(pd->arguments, i);
        type = _dbus_type_to_eina_value_type(arg->type[0]);
        value = eina_value_new(type);
        eina_hash_add(pd->properties_hash, name, value);
     }

   efl_model_load_set(pd->obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES);
}

static Efl_Model_Load_Status
_eldbus_model_arguments_efl_model_base_property_set(Eo *obj EINA_UNUSED,
                                                    Eldbus_Model_Arguments_Data *pd,
                                                    const char *property,
                                                    Eina_Value const* value)
{
   Eina_Value *prop_value;
   Eina_Bool ret;

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

   prop_value = eina_hash_find(pd->properties_hash, property);
   EINA_SAFETY_ON_NULL_RETURN_VAL(prop_value, EFL_MODEL_LOAD_STATUS_ERROR);

   eina_value_flush(prop_value);
   ret = eina_value_copy(value, prop_value);
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
   eo_do(obj, efl_model_properties_load());
   eo_do(obj, efl_model_children_load());
}

static Efl_Model_Load_Status
_eldbus_model_arguments_efl_model_base_load_status_get(Eo *obj EINA_UNUSED, Eldbus_Model_Arguments_Data *pd)
{
   return pd->load.status;
}

static void
_eldbus_model_arguments_efl_model_base_unload(Eo *obj EINA_UNUSED, Eldbus_Model_Arguments_Data *pd)
{
   _eldbus_model_arguments_unload(pd);

   efl_model_load_set(pd->obj, &pd->load, EFL_MODEL_LOAD_STATUS_UNLOADED);
}

static Eo *
_eldbus_model_arguments_efl_model_base_child_add(Eo *obj EINA_UNUSED, Eldbus_Model_Arguments_Data *pd EINA_UNUSED)
{
   return NULL;
}

static Efl_Model_Load_Status
_eldbus_model_arguments_efl_model_base_child_del(Eo *obj EINA_UNUSED,
                                                 Eldbus_Model_Arguments_Data *pd EINA_UNUSED,
                                                 Eo *child EINA_UNUSED)
{
   return EFL_MODEL_LOAD_STATUS_ERROR;
}

static Efl_Model_Load_Status
_eldbus_model_arguments_efl_model_base_children_slice_get(Eo *obj EINA_UNUSED,
                                                          Eldbus_Model_Arguments_Data *pd,
                                                          unsigned start EINA_UNUSED,
                                                          unsigned count EINA_UNUSED,
                                                          Eina_Accessor **children_accessor)
{
   *children_accessor = NULL;
   return pd->load.status;
}

static Efl_Model_Load_Status
_eldbus_model_arguments_efl_model_base_children_count_get(Eo *obj EINA_UNUSED,
                                                          Eldbus_Model_Arguments_Data *pd,
                                                          unsigned *children_count)
{
   *children_count = 0;
   return pd->load.status;
}

static void
_eldbus_model_arguments_efl_model_base_children_load(Eo *obj EINA_UNUSED, Eldbus_Model_Arguments_Data *pd)
{
   if (pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN)
     return;

   efl_model_load_set(pd->obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN);
}

static const char *
_eldbus_model_arguments_name_get(Eo *obj EINA_UNUSED, Eldbus_Model_Arguments_Data *pd)
{
   return pd->name;
}

static void
_eldbus_model_arguments_unload(Eldbus_Model_Arguments_Data *pd)
{
   Eldbus_Pending *pending;

   EINA_SAFETY_ON_NULL_RETURN(pd);

   EINA_LIST_FREE(pd->pending_list, pending)
     eldbus_pending_cancel(pending);

   if (pd->properties_array)
     {
        Eina_Stringshare *property;
        Eina_Array_Iterator it;
        unsigned int i;

        EINA_ARRAY_ITER_NEXT(pd->properties_array, i, property, it)
          eina_stringshare_del(property);
        eina_array_free(pd->properties_array);
        pd->properties_array = NULL;
     }

   eina_hash_free_buckets(pd->properties_hash);
}

Eina_Bool
eldbus_model_arguments_process_arguments(Eldbus_Model_Arguments_Data *pd,
                                         const Eldbus_Message *msg,
                                         Eldbus_Pending *pending)
{
   const Eldbus_Introspection_Argument *argument;
   const char *error_name, *error_text;
   const Eina_List *it;
   Eina_Value *value_struct;
   Eina_Array *changed_properties;
   unsigned int i = 0;
   Eina_Bool result = EINA_FALSE;

   pd->pending_list = eina_list_remove(pd->pending_list, pending);
   if (eldbus_message_error_get(msg, &error_name, &error_text))
     {
        ERR("%s: %s", error_name, error_text);
        efl_model_error_notify(pd->obj);
        return EINA_FALSE;
     }

   value_struct = eldbus_message_to_eina_value(msg);
   if (value_struct == NULL)
     {
        INF("%s", "No output arguments");
        return EINA_TRUE;
     }

   changed_properties = eina_array_new(1);

   EINA_LIST_FOREACH(pd->arguments, it, argument)
     {
        if (ELDBUS_INTROSPECTION_ARGUMENT_DIRECTION_IN != argument->direction)
          {
             Eina_Stringshare *property;
             Eina_Bool ret;

             property = eina_array_data_get(pd->properties_array, i);
             EINA_SAFETY_ON_NULL_GOTO(property, on_error);

             ret = _eldbus_model_arguments_property_set(pd, value_struct, property);
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

   result = EINA_TRUE;

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
   Eina_Value *prop_value;
   Eina_Value value;
   Eina_Bool ret;

   prop_value = eina_hash_find(pd->properties_hash, property);
   EINA_SAFETY_ON_NULL_RETURN_VAL(prop_value, EINA_FALSE);

   ret = eina_value_struct_value_get(value_struct, "arg0", &value);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, EINA_FALSE);

   eina_value_flush(prop_value);
   ret = eina_value_copy(&value, prop_value);
   eina_value_flush(&value);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, EINA_FALSE);

   return ret;
}

static Eina_Bool
_eldbus_model_arguments_is(Eldbus_Model_Arguments_Data *pd,
                           const char *argument,
                           Eldbus_Introspection_Argument_Direction direction)
{
   Eldbus_Introspection_Argument *argument_introspection;
   unsigned int i;

   i = _eldbus_model_arguments_argument_index_get(pd, argument);
   if (i >= eina_array_count(pd->properties_array))
     {
        WRN("Argument not found: %s", argument);
        return false;
     }

   argument_introspection = eina_list_nth(pd->arguments, i);
   EINA_SAFETY_ON_NULL_RETURN_VAL(argument_introspection, EINA_FALSE);

   return argument_introspection->direction == direction;
}

static Eina_Bool
_eldbus_model_arguments_is_input_argument(Eldbus_Model_Arguments_Data *pd, const char *argument)
{
   return _eldbus_model_arguments_is(pd, argument, ELDBUS_INTROSPECTION_ARGUMENT_DIRECTION_IN);
}

static Eina_Bool
_eldbus_model_arguments_is_output_argument(Eldbus_Model_Arguments_Data *pd, const char *argument)
{
   return _eldbus_model_arguments_is(pd, argument, ELDBUS_INTROSPECTION_ARGUMENT_DIRECTION_OUT) ||
     _eldbus_model_arguments_is(pd, argument, ELDBUS_INTROSPECTION_ARGUMENT_DIRECTION_NONE);
}

static unsigned int
_eldbus_model_arguments_argument_index_get(Eldbus_Model_Arguments_Data *pd, const char *argument)
{
   Eina_Stringshare *name;
   Eina_Array_Iterator it;
   unsigned int i = 0;

   EINA_ARRAY_ITER_NEXT(pd->properties_array, i, name, it)
     {
        if (strcmp(name, argument) == 0)
          return i;
     }

   return ++i;
}

#include "eldbus_model_arguments.eo.c"

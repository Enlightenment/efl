#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eldbus_model_arguments_private.h"
#include "eldbus_model_method_private.h"
#include "eldbus_model_private.h"
#include "eldbus_proxy.h"

#include <Eina.h>

#include <stdbool.h>

#define MY_CLASS ELDBUS_MODEL_METHOD_CLASS
#define MY_CLASS_NAME "Eldbus_Model_Method"

static void _eldbus_model_method_call_cb(void *, const Eldbus_Message *, Eldbus_Pending *);

static Efl_Object*
_eldbus_model_method_efl_object_constructor(Eo *obj, Eldbus_Model_Method_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   pd->obj = obj;
   pd->method = NULL;
   return obj;
}

static Efl_Object*
_eldbus_model_method_efl_object_finalize(Eo *obj, Eldbus_Model_Method_Data *pd)
{
   if (!pd->proxy ||
       !pd->method)
     return NULL;

   eldbus_model_arguments_custom_constructor(obj,
                                             pd->proxy,
                                             pd->method->name, pd->method->arguments);

   return obj;
}

static void
_eldbus_model_method_proxy_set(Eo *obj EINA_UNUSED,
                               Eldbus_Model_Method_Data *pd,
                               Eldbus_Proxy *proxy)
{
   pd->proxy = proxy;
}

static void
_eldbus_model_method_method_set(Eo *obj EINA_UNUSED,
                                Eldbus_Model_Method_Data *pd,
                                const Eldbus_Introspection_Method *method)
{
   pd->method = method;
}

static void
_eldbus_model_method_call(Eo *obj EINA_UNUSED, Eldbus_Model_Method_Data *pd EINA_UNUSED)
{
   Eldbus_Model_Arguments_Data *data = efl_data_scope_get(obj, ELDBUS_MODEL_ARGUMENTS_CLASS);
   Eldbus_Message *msg = eldbus_proxy_method_call_new(data->proxy, data->name);
   Eldbus_Message_Iter *iter = eldbus_message_iter_get(msg);
   const Eldbus_Introspection_Argument *argument;
   const Eina_List *it;
   Eldbus_Pending *pending;
   unsigned int i = 0;

   EINA_LIST_FOREACH(data->arguments, it, argument)
     {
        Eina_Slstr *name;
        const Eina_Value *value;
        const char *signature;
        Eina_Bool ret;

        if (ELDBUS_INTROSPECTION_ARGUMENT_DIRECTION_IN != argument->direction)
          continue;

        name = eina_slstr_printf(ARGUMENT_FORMAT, i);
        EINA_SAFETY_ON_NULL_GOTO(name, on_error);

        value = eina_hash_find(data->properties, name);
        EINA_SAFETY_ON_NULL_GOTO(value, on_error);

        signature = argument->type;
        if (dbus_type_is_basic(signature[0]))
          ret = _message_iter_from_eina_value(signature, iter, value);
        else
          ret = _message_iter_from_eina_value_struct(signature, iter, value);

        EINA_SAFETY_ON_FALSE_GOTO(ret, on_error);

        ++i;
     }

   pending = eldbus_proxy_send(data->proxy, msg, _eldbus_model_method_call_cb, pd, -1);
   data->pending_list = eina_list_append(data->pending_list, pending);

   return;

on_error:
   eldbus_message_unref(msg);
}

static void
_eldbus_model_method_call_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   Eldbus_Model_Method_Data *pd = (Eldbus_Model_Method_Data*)data;
   Eldbus_Model_Arguments_Data *args_data = efl_data_scope_get(pd->obj, ELDBUS_MODEL_ARGUMENTS_CLASS);

   if (eldbus_model_arguments_process_arguments(args_data, msg, pending))
     efl_event_callback_call(pd->obj, ELDBUS_MODEL_METHOD_EVENT_SUCCESSFUL_CALL, NULL);
}

#include "eldbus_model_method.eo.c"

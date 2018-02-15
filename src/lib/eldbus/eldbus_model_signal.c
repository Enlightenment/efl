#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eldbus_model_arguments_private.h"
#include "eldbus_model_signal_private.h"
#include "eldbus_model_private.h"

#include <Ecore.h>
#include <Eina.h>

#define MY_CLASS ELDBUS_MODEL_SIGNAL_CLASS
#define MY_CLASS_NAME "Eldbus_Model_Signal"

static void _eldbus_model_signal_handler_cb(void *, const Eldbus_Message *);
static void _eldbus_model_signal_callback_add(Eldbus_Model_Signal_Data *);
static void _eldbus_model_signal_callback_del(Eldbus_Model_Signal_Data *);

static Efl_Object*
_eldbus_model_signal_efl_object_constructor(Eo *obj, Eldbus_Model_Signal_Data *pd)
{
   efl_constructor(efl_super(obj, MY_CLASS));

   pd->obj = obj;
   pd->handler = NULL;
   pd->signal = NULL;
   return obj;
}

static void
_eldbus_model_signal_signal_constructor(Eo *obj EINA_UNUSED,
                                 Eldbus_Model_Signal_Data *pd,
                                 Eldbus_Proxy *proxy,
                                 const Eldbus_Introspection_Signal *signal)
{
   EINA_SAFETY_ON_NULL_RETURN(proxy);
   EINA_SAFETY_ON_NULL_RETURN(signal);

   eldbus_model_arguments_custom_constructor(efl_super(obj, MY_CLASS), proxy, signal->name, signal->arguments);

   pd->signal = signal;
   _eldbus_model_signal_callback_add(pd);
}

static void
_eldbus_model_signal_efl_object_destructor(Eo *obj, Eldbus_Model_Signal_Data *pd)
{
   _eldbus_model_signal_callback_del(pd);

   efl_destructor(efl_super(obj, MY_CLASS));
}


static void
_eldbus_model_signal_callback_add(Eldbus_Model_Signal_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN(pd);
   EINA_SAFETY_ON_FALSE_RETURN(NULL == pd->handler);

   Eldbus_Model_Arguments_Data *args_data = efl_data_scope_get(pd->obj, ELDBUS_MODEL_ARGUMENTS_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(args_data);

   pd->handler = eldbus_proxy_signal_handler_add(args_data->proxy, pd->signal->name, _eldbus_model_signal_handler_cb, pd);
}

static void
_eldbus_model_signal_callback_del(Eldbus_Model_Signal_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN(pd);

   if (pd->handler)
     {
        eldbus_signal_handler_unref(pd->handler);
        pd->handler = NULL;
     }
}

static void
_eldbus_model_signal_handler_cb(void *data, const Eldbus_Message *msg)
{
   Eldbus_Model_Signal_Data *pd = (Eldbus_Model_Signal_Data*)data;

   Eldbus_Model_Arguments_Data *args_data = efl_data_scope_get(pd->obj, ELDBUS_MODEL_ARGUMENTS_CLASS);

   eldbus_model_arguments_process_arguments(args_data, msg, NULL);
}

#include "eldbus_model_signal.eo.c"

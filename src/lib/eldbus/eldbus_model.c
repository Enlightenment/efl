#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eldbus_model_private.h"

#include <Ecore.h>
#include <Eina.h>
#include <Eldbus.h>

#define MY_CLASS ELDBUS_MODEL_CLASS
#define MY_CLASS_NAME "Eldbus_Model"

static void
eldbus_model_connect_do(Eldbus_Model_Data *pd)
{
   if (pd->type == ELDBUS_CONNECTION_TYPE_ADDRESS)
     {
        if (pd->private)
          pd->connection = eldbus_address_connection_get(pd->address);
        else
          pd->connection = eldbus_private_address_connection_get(pd->address);
     }
   else
     {
        if (pd->private)
          pd->connection = eldbus_private_connection_get(pd->type);
        else
          pd->connection = eldbus_connection_get(pd->type);
     }

   // TODO: Register for disconnection event
   if (!pd->connection)
     {
        DBG("Unable to setup a connection [%i - %s] %i",
            pd->type, pd->address, pd->private);
     }
}

static void
_eldbus_model_connect(Eo *obj EINA_UNUSED,
                      Eldbus_Model_Data *pd,
                      Eldbus_Connection_Type type,
                      const char *address,
                      Eina_Bool priv)
{
   pd->type = type;
   pd->address = eina_stringshare_add(address);
   pd->private = priv;
}

static void
_eldbus_model_connection_set(Eo *obj EINA_UNUSED,
                             Eldbus_Model_Data *pd,
                             Eldbus_Connection *dbus)
{
   eldbus_connection_ref(dbus);
   if (pd->connection) eldbus_connection_unref(pd->connection);
   pd->connection = dbus;
}

static Eldbus_Connection *
_eldbus_model_connection_get(const Eo *obj EINA_UNUSED, Eldbus_Model_Data *pd)
{
   return pd->connection;
}

static Efl_Object *
_eldbus_model_efl_object_finalize(Eo *obj, Eldbus_Model_Data *pd)
{
   if (!pd->connection) eldbus_model_connect_do(pd);
   if (!pd->connection) return NULL;

   return efl_finalize(efl_super(obj, ELDBUS_MODEL_CLASS));
}

static void
_eldbus_model_efl_object_invalidate(Eo *obj, Eldbus_Model_Data *pd)
{
   if (pd->connection) eldbus_connection_unref(pd->connection);
   pd->connection = NULL;

   efl_invalidate(efl_super(obj, MY_CLASS));
}

static void
_eldbus_model_efl_object_destructor(Eo *obj, Eldbus_Model_Data *pd)
{
   eina_stringshare_del(pd->unique_name);
   pd->unique_name = NULL;

   eina_stringshare_del(pd->address);
   pd->address = NULL;

   efl_destructor(efl_super(obj, MY_CLASS));
}

static const char *
_eldbus_model_address_get(const Eo *obj EINA_UNUSED, Eldbus_Model_Data *pd)
{
   return pd->address;
}

static Eina_Bool
_eldbus_model_private_get(const Eo *obj EINA_UNUSED, Eldbus_Model_Data *pd)
{
   return pd->private;
}

static Eldbus_Connection_Type
_eldbus_model_type_get(const Eo *obj EINA_UNUSED, Eldbus_Model_Data *pd)
{
   return pd->type;
}

static Eina_Future *
_eldbus_model_efl_model_property_set(Eo *obj,
                                     Eldbus_Model_Data *pd EINA_UNUSED,
                                     const char *property,
                                     Eina_Value *value EINA_UNUSED)
{
   Eina_Error err = EFL_MODEL_ERROR_READ_ONLY;

   if (strcmp(property, UNIQUE_NAME_PROPERTY))
     err = EFL_MODEL_ERROR_NOT_FOUND;
   return eina_future_rejected(efl_loop_future_scheduler_get(obj), err);
}

static Eina_Value *
_eldbus_model_efl_model_property_get(const Eo *obj,
                                     Eldbus_Model_Data *pd,
                                     const char *property)
{
   DBG("(%p): property=%s", obj, property);

   if (strcmp(property, UNIQUE_NAME_PROPERTY) != 0) goto on_error;

   if (!pd->connection) eldbus_model_connect_do(pd);

   if (pd->unique_name == NULL)
     {
        const char *unique_name;

        unique_name = eldbus_connection_unique_name_get(pd->connection);
        if (!unique_name) goto on_error;
        pd->unique_name = eina_stringshare_add(unique_name);
     }

   return eina_value_string_new(pd->unique_name);

 on_error:
   return eina_value_error_new(EFL_MODEL_ERROR_NOT_FOUND);
}

static Eina_Iterator *
_eldbus_model_efl_model_properties_get(const Eo *obj EINA_UNUSED,
                                       Eldbus_Model_Data *pd EINA_UNUSED)
{
   char *unique[] = { UNIQUE_NAME_PROPERTY };

   return EINA_C_ARRAY_ITERATOR_NEW(unique);
}

static Efl_Object *
_eldbus_model_efl_model_child_add(Eo *obj EINA_UNUSED,
                                  Eldbus_Model_Data *pd EINA_UNUSED)
{
   return NULL;
}

static void
_eldbus_model_efl_model_child_del(Eo *obj EINA_UNUSED,
                                  Eldbus_Model_Data *pd EINA_UNUSED,
                                  Efl_Object *child EINA_UNUSED)
{
}

static Eina_Future *
_eldbus_model_efl_model_children_slice_get(Eo *obj EINA_UNUSED,
                                           Eldbus_Model_Data *pd EINA_UNUSED,
                                           unsigned int start EINA_UNUSED,
                                           unsigned int count EINA_UNUSED)
{
   return eina_future_rejected(efl_loop_future_scheduler_get(obj),
                               EFL_MODEL_ERROR_NOT_SUPPORTED);
}

static unsigned int
_eldbus_model_efl_model_children_count_get(const Eo *obj EINA_UNUSED,
                                           Eldbus_Model_Data *pd EINA_UNUSED)
{
   return 0;
}

#include "eldbus_model.eo.c"

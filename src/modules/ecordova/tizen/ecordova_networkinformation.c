#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_TIZEN_NETWORK_CONNECTION
#include "ecordova_networkinformation_private.h"

#define MY_CLASS ECORDOVA_NETWORKINFORMATION_CLASS
#define MY_CLASS_NAME "Ecordova_NetworkInformation"

static void _type_changed_cb(connection_type_e, void *);
static Ecordova_NetworkInformation_ConnectionType _to_connection_type(connection_type_e);

static Eo_Base *
_ecordova_networkinformation_eo_base_constructor(Eo *obj,
                                                 Ecordova_NetworkInformation_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;
   pd->connection = NULL;
   int ret = connection_create(&pd->connection);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONNECTION_ERROR_NONE == ret, NULL);

   ret = connection_set_type_changed_cb(pd->connection, _type_changed_cb, obj);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONNECTION_ERROR_NONE == ret, NULL);

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_networkinformation_eo_base_destructor(Eo *obj,
                                                Ecordova_NetworkInformation_Data *pd)
{
   DBG("(%p)", obj);

   int ret = connection_unset_type_changed_cb(pd->connection);
   EINA_SAFETY_ON_FALSE_RETURN(CONNECTION_ERROR_NONE == ret);

   ret = connection_destroy(pd->connection);
   EINA_SAFETY_ON_FALSE_RETURN(CONNECTION_ERROR_NONE == ret);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static Ecordova_NetworkInformation_ConnectionType
_ecordova_networkinformation_type_get(Eo *obj,
                                      Ecordova_NetworkInformation_Data *pd)
{
   DBG("(%p)", obj);

   connection_type_e type;

   int ret = connection_get_type(pd->connection, &type);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(CONNECTION_ERROR_NONE == ret,
                                   ECORDOVA_NETWORKINFORMATION_CONNECTIONTYPE_UNKNOWN);

   return _to_connection_type(type);
}

static Ecordova_NetworkInformation_ConnectionType
_to_connection_type(connection_type_e type)
{
   switch (type)
     {
      case CONNECTION_TYPE_DISCONNECTED:
        return ECORDOVA_NETWORKINFORMATION_CONNECTIONTYPE_NONE;
      case CONNECTION_TYPE_WIFI:
        return ECORDOVA_NETWORKINFORMATION_CONNECTIONTYPE_WIFI;
      case CONNECTION_TYPE_CELLULAR:
        return ECORDOVA_NETWORKINFORMATION_CONNECTIONTYPE_CELL;
      case CONNECTION_TYPE_ETHERNET:
        return ECORDOVA_NETWORKINFORMATION_CONNECTIONTYPE_ETHERNET;
      case CONNECTION_TYPE_BT:
        return ECORDOVA_NETWORKINFORMATION_CONNECTIONTYPE_UNKNOWN;
     }

   return ECORDOVA_NETWORKINFORMATION_CONNECTIONTYPE_UNKNOWN;
}

static void
_type_changed_cb(connection_type_e type, void *user_data)
{
   Eo *obj = user_data;

   const Eo_Event_Description *event =
     CONNECTION_TYPE_DISCONNECTED == type ? ECORDOVA_NETWORKINFORMATION_EVENT_OFFLINE
                                          : ECORDOVA_NETWORKINFORMATION_EVENT_ONLINE;
   eo_do(obj, eo_event_callback_call(event, NULL));
}

#include "ecordova_networkinformation.eo.c"
#endif

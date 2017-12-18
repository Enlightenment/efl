#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#define ELM_ATSPI_ADAPTOR_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_atspi_adaptor.eo.h"

#define ELM_ACCESS_OBJECT_PATH_PREFIX  "/org/a11y/atspi/accessible/"
#define ELM_ACCESS_OBJECT_PATH_ROOT "root"

const char *Accessible_Interface_Desc = "\
<interface name=\"org.a11y.atspi.Accessible\">\
   <method name=\"SetPosition\">\
      <arg type=\"i\" name=\"x\" direction=\"in\" />\
      <arg type=\"i\" name=\"y\" direction=\"in\" />\
      <arg type=\"u\" name=\"coord_type\" direction=\"in\" />\
      <arg type=\"b\" name=\"result\" direction=\"out\" />\
      <annotation name=\"org.enlightenment.eldbus.method\" value=\"Elm.Atspi.Access.Adaptor.position_set\">\
   </method>\
   <signal name=\"AttributesChanged\">\
      <arg direction=\"in\" type=\"(suuv)\"/>\
      <annotation name=\"org.enlightenment.eldbus.type.in0\" value=\"Efl_Access_Event\"/>\
      <annotation name=\"org.enlightenment.eldbus.event\" value=\"EFL_ACCESS_EVENT_CHILDREN_CHANGED\">\
   </signal>\
</interface>";

typedef struct _Elm_Atspi_Adaptor_Data
{
   Efl_Access_Cache *cache;
   Eldbus_Service_Interface *service;
   Eldbus_Service_Interface_Desc *interface;
   Elm_Atspi_Service_Interface_Desc  *atspi_interface_desc;
} Elm_Atspi_Adaptor_Data;

static Eina_Bool
_set_extents_handler(void *data, const char *path, int x, int y, unsigned coort_type)
{
   Eina_Coord x, y;
}

EOLIAN static void
_elm_atspi_adaptor_constructor(Eo *obj, Elm_Atspi_Adaptor_Data *pd)
{
   // set bindings to itself so it will be propagated to all children
   eldbus_model_service_interface_bind_context_set(obj, obj);
   eldbus_model_service_interface_constructor(obj, Accessible_Interface_Desc);
}

EOLIAN static Eo*
_elm_atspi_adaptor_efl_object_constructor(Eo *obj, Elm_Atspi_Adaptor_Data *pd)
{
   return efl_constructor(efl_super(obj, ELM_ATSPI_ADAPTOR_CLASS));
}

static Eldbus_Message*
_elm_atspi_adaptor_method_callback(Eldbus_Service_Interface *service, Eldbus_Message *msg)
{
   Elm_Atspi_Adaptor *obj = eldbus_service_interface_data_get(service, "__adaptor");
   if (!obj) return NULL;

   Elm_Atspi_Service_Interface_Desc *desc = elm_atspi_adaptor_interface_get(obj);
   if (!desc || !desc->methods) return NULL;

   const char *method_name = eldbus_message_member_get(msg);
   const char *path = eldbus_message_path_get(msg);

   EINA_SAFETY_ON_NULL_RETURN_VAL(method_name, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);

   Efl_Access *access = elm_atspi_adaptor_object_from_path(obj, path);
   Elm_Atspi_Method *m = eina_hash_find(desc->methods, method_name);
   if (!m) return NULL; //invalid method name error

   return m->cb ? m->cb(NULL, obj, access, eldbus_service_connection_get(service), msg) : NULL;
}

static Eldbus_Message*
_elm_atspi_adaptor_property_get_callback(Eldbus_Service_Interface *service, Eldbus_Message *msg)
{
}

static Eldbus_Message*
_elm_atspi_adaptor_property_set_callback(Eldbus_Service_Interface *service, Eldbus_Message *msg)
{
}

EOLIAN static void
_elm_atspi_adaptor_efl_object_destructor(Eo *obj, Elm_Atspi_Adaptor_Data *pd EINA_UNUSED)
{
   elm_atspi_adaptor_unregister(obj);
   efl_destructor(efl_super(obj, ELM_ATSPI_ADAPTOR_CLASS));
}

EOLIAN static Efl_Access_Cache*
_elm_atspi_adaptor_cache_get(Eo *obj, Elm_Atspi_Adaptor_Data *pd)
{
   return pd->cache;
}

EOLIAN static void
_elm_atspi_adaptor_cache_set(Eo *obj, Elm_Atspi_Adaptor_Data *pd, Efl_Access_Cache *cache)
{
   EINA_SAFETY_ON_NULL_RETURN(cache);
   pd->cache = cache;
}

EOLIAN static void
_elm_atspi_adaptor_register(Eo *obj, Elm_Atspi_Adaptor_Data *pd, Eldbus_Connection *conn, const char *path)
{
   EINA_SAFETY_ON_NULL_RETURN(conn);
   EINA_SAFETY_ON_NULL_RETURN(path);
   EINA_SAFETY_ON_NULL_RETURN(pd->interface);

   pd->service = eldbus_service_interface_fallback_register(conn, path, pd->interface);
}

EOLIAN static void
_elm_atspi_adaptor_unregister(Eo *obj, Elm_Atspi_Adaptor_Data *pd)
{
   if (pd->service)
     eldbus_service_interface_unregister(pd->service);
}

EOLIAN static void
_elm_atspi_adaptor_interface_set(Eo *obj, Elm_Atspi_Adaptor_Data *pd, const Elm_Atspi_Service_Interface_Desc *desc)
{
   pd->atspi_interface_desc = desc;
}

EOLIAN static const Elm_Atspi_Service_Interface_Desc*
_elm_atspi_adaptor_interface_get(Eo *obj, Elm_Atspi_Adaptor_Data *pd)
{
   return pd->atspi_interface_desc;
}


#include "elm_atspi_adaptor.eo.c"

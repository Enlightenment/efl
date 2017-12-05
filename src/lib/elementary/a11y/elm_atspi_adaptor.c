#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#define ELM_ATSPI_ADAPTOR_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_atspi_adaptor.eo.h"

#define ELM_ACCESS_OBJECT_PATH_PREFIX  "/org/a11y/atspi/accessible/"
#define ELM_ACCESS_OBJECT_PATH_ROOT "root"

typedef struct _Elm_Atspi_Adaptor_Data
{
   Efl_Access_Cache *cache;
   Eldbus_Service_Interface *service;
} Elm_Atspi_Adaptor_Data;

EOLIAN static Eo*
_elm_atspi_adaptor_efl_object_constructor(Eo *obj, Elm_Atspi_Adaptor_Data *pd)
{
   return efl_constructor(efl_super(obj, ELM_ATSPI_ADAPTOR_CLASS));
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

   pd->service = eldbus_service_interface_fallback_register2(conn, path, elm_atspi_adaptor_interface_get(obj));
}

EOLIAN static void
_elm_atspi_adaptor_unregister(Eo *obj, Elm_Atspi_Adaptor_Data *pd)
{
   if (pd->service)
     eldbus_service_interface_unregister(pd->service);
}

EOLIAN static const Eldbus_Service_Interface_Desc2*
_elm_atspi_adaptor_interface_get(Eo *obj, Elm_Atspi_Adaptor_Data *pd)
{
   return NULL;
}

static const char*
_object_path_encode(const char *prefix, intptr_t access_ptr, Eina_Bool is_root)
{
#if 0
   char buf[DBUS_MAX_PATH];

   if (is_root)
      snprintf(buf, sizeof(buf), "%s/%ld", prefix, access_ptr);
#endif
}

static void
_root_reference_print(char *buf, size_t buf_size)
{
}

static void
_object_reference_print(char *buf, size_t buf_size, const char *prefix, intptr_t obj_ptr)
{
}

static intptr_t
_object_reference_scan(const char *prefix, const char *path)
{
}

static intptr_t
_object_decode_from_path(Eo *obj, Elm_Atspi_Adaptor_Data *pd, const char *path)
{
   intptr_t access;

#if 0
   if (_root_path_is(path))
     access = (intptr_t)efl_access_cache_root_get(pd->cache);
   else
     access = _object_reference_scan(eldbus_service_object_path_get(pd->service, );

#endif
   return access;
}

static Eina_Bool
_root_path_is(const char *full_path)
{
   if (!full_path) return EINA_FALSE;
   return strcmp(full_path, ELM_ACCESS_OBJECT_PATH_ROOT) == 0;
}

EOLIAN static Elm_Atspi_Dbus_Reference
_elm_atspi_adaptor_reference_from_object(Eo *obj, Elm_Atspi_Adaptor_Data *pd, Efl_Access *access)
{
   Elm_Atspi_Dbus_Reference ref = { 0, 0 };

   EINA_SAFETY_ON_NULL_RETURN_VAL(access, ref);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->service, ref);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->cache, ref);

   if (!efl_access_cache_contains(pd->cache, access))
     return ref;

   ref.bus = eldbus_connection_unique_name_get(eldbus_service_connection_get(pd->service));
   ref.path = _object_path_encode(eldbus_service_object_path_get(pd->service), (intptr_t)access, efl_access_cache_root_get(pd->cache) == access);

   return ref;
}

EOLIAN static Efl_Access*
_elm_atspi_adaptor_object_from_reference(Eo *obj, Elm_Atspi_Adaptor_Data *pd, Elm_Atspi_Dbus_Reference ref)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->cache, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ref.bus, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ref.path, NULL);

#if 0
   if (_root_path_is(ref.path))
   intptr_t access = _object_decode_from_path(obj, pd, ref.path);

   if (efl_access_cache_contains(pd->cache, (Efl_Access*)access))
     return (Efl_Access*)access;
#endif

   return NULL;
}

#include "elm_atspi_adaptor.eo.c"

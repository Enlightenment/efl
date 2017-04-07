#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include <elm_priv.h>
#include <assert.h>

typedef struct _Elm_Atspi_Proxy_Data {
     const char *bus;
     const char *path;
} Elm_Atspi_Proxy_Data;

EOLIAN void
_elm_atspi_proxy_efl_object_destructor(Eo *obj EINA_UNUSED, Elm_Atspi_Proxy_Data *pd)
{
   eina_stringshare_del(pd->bus);
   eina_stringshare_del(pd->path);
}

EOLIAN const char*
_elm_atspi_proxy_path_get(Eo *obj EINA_UNUSED, Elm_Atspi_Proxy_Data *pd)
{
   return pd->path;
}

EOLIAN const char*
_elm_atspi_proxy_bus_name_get(Eo *obj EINA_UNUSED, Elm_Atspi_Proxy_Data *pd)
{
   return pd->bus;
}

EOLIAN void
_elm_atspi_proxy_constructor(Eo *obj EINA_UNUSED, Elm_Atspi_Proxy_Data *pd, const char *bus, const char *path)
{
   pd->bus = eina_stringshare_add(bus);
   pd->path = eina_stringshare_add(path);
}


#include "elm_atspi_proxy.eo.c"

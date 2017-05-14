#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include <elm_priv.h>
#include <assert.h>

typedef struct _Elm_Atspi_Proxy_Data {
     const char *id;
} Elm_Atspi_Proxy_Data;

EOLIAN void
_elm_atspi_proxy_efl_object_destructor(Eo *obj EINA_UNUSED, Elm_Atspi_Proxy_Data *pd)
{
   if (pd->id) eina_stringshare_del(pd->id);
   efl_destructor(efl_super(obj, ELM_ATSPI_PROXY_CLASS));
}

EOLIAN const char*
_elm_atspi_proxy_id_get(Eo *obj EINA_UNUSED, Elm_Atspi_Proxy_Data *pd)
{
   return pd->id;
}

EOLIAN void
_elm_atspi_proxy_id_constructor(Eo *obj EINA_UNUSED, Elm_Atspi_Proxy_Data *pd, const char *id)
{
   pd->id = eina_stringshare_add(id);
}

#include "elm_atspi_proxy.eo.c"

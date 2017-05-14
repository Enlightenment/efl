#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include <elm_priv.h>
#include <assert.h>


typedef struct _Elm_Atspi_Plug_Data {
     Elm_Atspi_Proxy *proxy_parent; // technical object for keep bridge coherent
} Elm_Atspi_Plug_Data;

EOLIAN Eo*
_elm_atspi_plug_efl_object_constructor(Eo *obj, Elm_Atspi_Plug_Data *pd)
{
   efl_constructor(efl_super(obj, ELM_ATSPI_PLUG_CLASS));

   Elm_Atspi_Bridge *bridge = _elm_atspi_bridge_get();
   if (bridge)
     elm_atspi_bridge_plug_register(bridge, obj);

   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_FRAME);
   return obj;
}

EOLIAN void
_elm_atspi_plug_efl_object_destructor(Eo *obj EINA_UNUSED, Elm_Atspi_Plug_Data *pd)
{
   efl_destructor(efl_super(obj, ELM_ATSPI_PLUG_CLASS));
}

EOLIAN void
_elm_atspi_plug_elm_interface_atspi_socket_on_embedded(Eo *obj EINA_UNUSED, Elm_Atspi_Plug_Data *pd, Elm_Atspi_Proxy *proxy)
{
   ERR("On embedded");
}

EOLIAN void
_elm_atspi_plug_elm_interface_atspi_socket_on_unembedded(Eo *obj EINA_UNUSED, Elm_Atspi_Plug_Data *pd, Elm_Atspi_Proxy *proxy)
{
   ERR("On unembedded");
}

#include "elm_atspi_plug.eo.c"


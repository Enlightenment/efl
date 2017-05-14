#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include <elm_priv.h>
#include <assert.h>


typedef struct _Elm_Atspi_Plug_Data {
     const char  *id;
} Elm_Atspi_Plug_Data;

EOLIAN Eo*
_elm_atspi_plug_efl_object_constructor(Eo *obj, Elm_Atspi_Plug_Data *pd)
{
   efl_constructor(efl_super(obj, ELM_ATSPI_PLUG_CLASS));
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_FRAME);
   return obj;
}

EOLIAN void
_elm_atspi_plug_efl_object_destructor(Eo *obj EINA_UNUSED, Elm_Atspi_Plug_Data *pd)
{
   efl_destructor(efl_super(obj, ELM_ATSPI_PLUG_CLASS));
}

EOLIAN const char*
_elm_atspi_plug_id_get(Eo *plug EINA_UNUSED, Elm_Atspi_Plug_Data *pd)
{
   return pd->id;
}

EOLIAN void
_elm_atspi_plug_id_set(Eo *plug, Elm_Atspi_Plug_Data *pd, const char *new_id)
{
   const char *id = eina_stringshare_add(new_id);
   if (pd->id != id)
     {
        eina_stringshare_del(pd->id);
        pd->id = id;
        efl_event_callback_call(plug, ELM_OBJ_ATSPI_PLUG_EVENT_ID_CHANGED, NULL);
     }
   else
     eina_stringshare_del(id);
}

EOLIAN void
_elm_atspi_plug_on_connected(Eo *plug, Elm_Atspi_Plug_Data *pd EINA_UNUSED)
{
}

EOLIAN void
_elm_atspi_plug_on_disconnected(Eo *plug, Elm_Atspi_Plug_Data *pd EINA_UNUSED)
{
}

#include "elm_atspi_plug.eo.c"

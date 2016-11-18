#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

typedef struct
{

} Efl_Net_Control_Data;

EOLIAN static void
_efl_net_control_efl_object_destructor(Eo *obj, Efl_Net_Control_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, EFL_NET_CONTROL_CLASS));
}

EOLIAN static Efl_Object *
_efl_net_control_efl_object_constructor(Eo *obj, Efl_Net_Control_Data *pd EINA_UNUSED)
{
   INF("EFL compiled with --with-net-control=none");
   return efl_constructor(efl_super(obj, EFL_NET_CONTROL_CLASS));
}

EOLIAN static Efl_Object *
_efl_net_control_efl_object_finalize(Eo *obj, Efl_Net_Control_Data *pd EINA_UNUSED)
{
   obj = efl_finalize(efl_super(obj, EFL_NET_CONTROL_CLASS));
   efl_event_callback_call(obj, EFL_NET_CONTROL_EVENT_STATE_CHANGED, NULL);
   return obj;
}

EOLIAN static void
_efl_net_control_radios_offline_set(Eo *obj EINA_UNUSED, Efl_Net_Control_Data *pd EINA_UNUSED, Eina_Bool radios_offline EINA_UNUSED)
{
}

EOLIAN static Eina_Bool
_efl_net_control_radios_offline_get(Eo *obj EINA_UNUSED, Efl_Net_Control_Data *pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Efl_Net_Control_State
_efl_net_control_state_get(Eo *obj EINA_UNUSED, Efl_Net_Control_Data *pd EINA_UNUSED)
{
   return EFL_NET_CONTROL_STATE_ONLINE; /* best default for unsupported, hope we're online */
}

EOLIAN static Eina_Iterator *
_efl_net_control_access_points_get(Eo *obj EINA_UNUSED, Efl_Net_Control_Data *pd EINA_UNUSED)
{
   return eina_list_iterator_new(NULL);
}

EOLIAN static Eina_Iterator *
_efl_net_control_technologies_get(Eo *obj EINA_UNUSED, Efl_Net_Control_Data *pd EINA_UNUSED)
{
   return eina_list_iterator_new(NULL);
}

EOLIAN static void
_efl_net_control_agent_enabled_set(Eo *obj EINA_UNUSED, Efl_Net_Control_Data *pd EINA_UNUSED, Eina_Bool agent_enabled EINA_UNUSED)
{
}

EOLIAN static Eina_Bool
_efl_net_control_agent_enabled_get(Eo *obj EINA_UNUSED, Efl_Net_Control_Data *pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static void
_efl_net_control_agent_reply(Eo *obj EINA_UNUSED, Efl_Net_Control_Data *pd EINA_UNUSED, const char *name EINA_UNUSED, const Eina_Slice *ssid EINA_UNUSED, const char *username EINA_UNUSED, const char *passphrase EINA_UNUSED, const char *wps EINA_UNUSED)
{
}

#include "efl_net_control.eo.c"

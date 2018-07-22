#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

typedef struct
{

} Efl_Net_Session_Data;

EOLIAN static void
_efl_net_session_efl_object_destructor(Eo *obj, Efl_Net_Session_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, EFL_NET_SESSION_CLASS));
}

EOLIAN static Efl_Object *
_efl_net_session_efl_object_constructor(Eo *obj, Efl_Net_Session_Data *pd EINA_UNUSED)
{
   INF("EFL compiled with --with-net-control=none");
   return efl_constructor(efl_super(obj, EFL_NET_SESSION_CLASS));
}

EOLIAN static Efl_Object *
_efl_net_session_efl_object_finalize(Eo *obj, Efl_Net_Session_Data *pd EINA_UNUSED)
{
   obj = efl_finalize(efl_super(obj, EFL_NET_SESSION_CLASS));
   efl_event_callback_call(obj, EFL_NET_SESSION_EVENT_CHANGED, NULL);
   return obj;
}

EOLIAN static const char *
_efl_net_session_network_name_get(const Eo *obj EINA_UNUSED, Efl_Net_Session_Data *pd EINA_UNUSED)
{
   return NULL;
}

EOLIAN static Efl_Net_Session_State
_efl_net_session_state_get(const Eo *obj EINA_UNUSED, Efl_Net_Session_Data *pd EINA_UNUSED)
{
   return EFL_NET_SESSION_STATE_ONLINE; /* best default for unsupported, hope we're online */
}

EOLIAN static Efl_Net_Session_Technology
_efl_net_session_technology_get(const Eo *obj EINA_UNUSED, Efl_Net_Session_Data *pd EINA_UNUSED)
{
   return EFL_NET_SESSION_TECHNOLOGY_UNKNOWN;
}

EOLIAN static const char *
_efl_net_session_interface_get(const Eo *obj EINA_UNUSED, Efl_Net_Session_Data *pd EINA_UNUSED)
{
   return NULL;
}

EOLIAN static void
_efl_net_session_ipv4_get(const Eo *obj EINA_UNUSED, Efl_Net_Session_Data *pd EINA_UNUSED, const char **address, const char **netmask, const char **gateway)
{
   if (address) *address = NULL;
   if (netmask) *netmask = NULL;
   if (gateway) *gateway = NULL;
}

EOLIAN static void
_efl_net_session_ipv6_get(const Eo *obj EINA_UNUSED, Efl_Net_Session_Data *pd EINA_UNUSED, const char **address, uint8_t *prefix_length, const char **netmask, const char **gateway)
{
   if (address) *address = NULL;
   if (prefix_length) *prefix_length = 0;
   if (netmask) *netmask = NULL;
   if (gateway) *gateway = NULL;
}

EOLIAN static void
_efl_net_session_connect(Eo *obj EINA_UNUSED, Efl_Net_Session_Data *pd EINA_UNUSED, Eina_Bool online_required EINA_UNUSED, Efl_Net_Session_Technology technologies_allowed EINA_UNUSED)
{
   INF("EFL compiled with --with-net-control=none, cannot connect.");
}

EOLIAN static void
_efl_net_session_disconnect(Eo *obj EINA_UNUSED, Efl_Net_Session_Data *pd EINA_UNUSED)
{
   INF("EFL compiled with --with-net-control=none, cannot disconnect.");
}

#include "efl_net_session.eo.c"

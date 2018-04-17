#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

typedef struct
{

} Efl_Net_Control_Access_Point_Data;

EOLIAN static void
_efl_net_control_access_point_efl_object_destructor(Eo *obj, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, EFL_NET_CONTROL_ACCESS_POINT_CLASS));
}

EOLIAN static Efl_Net_Control_Access_Point_State
_efl_net_control_access_point_state_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED)
{
   return 0;
}

EOLIAN static Efl_Net_Control_Access_Point_Error
_efl_net_control_access_point_error_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED)
{
   return 0;
}

EOLIAN static const char *
_efl_net_control_access_point_ssid_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED)
{
   return NULL;
}

EOLIAN static void
_efl_net_control_access_point_priority_set(Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED, unsigned int priority EINA_UNUSED)
{
}

EOLIAN static unsigned int
_efl_net_control_access_point_priority_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED)
{
   return 0;
}

EOLIAN static Efl_Net_Control_Technology *
_efl_net_control_access_point_technology_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED)
{
   return NULL;
}

EOLIAN static uint8_t
_efl_net_control_access_point_strength_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED)
{
   return 0;
}

EOLIAN static Eina_Bool
_efl_net_control_access_point_roaming_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static void
_efl_net_control_access_point_auto_connect_set(Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED, Eina_Bool auto_connect EINA_UNUSED)
{
}

EOLIAN static Eina_Bool
_efl_net_control_access_point_auto_connect_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_net_control_access_point_remembered_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_net_control_access_point_immutable_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Efl_Net_Control_Access_Point_Security
_efl_net_control_access_point_security_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED)
{
   return 0;
}

EOLIAN static Eina_Iterator *
_efl_net_control_access_point_name_servers_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED)
{
   return NULL;
}

EOLIAN static Eina_Iterator *
_efl_net_control_access_point_time_servers_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED)
{
   return NULL;
}

EOLIAN static Eina_Iterator *
_efl_net_control_access_point_domains_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED)
{
   return NULL;
}

EOLIAN static void
_efl_net_control_access_point_ipv4_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED, Efl_Net_Control_Access_Point_Ipv4_Method *method EINA_UNUSED, const char **address EINA_UNUSED, const char **netmask EINA_UNUSED, const char **gateway EINA_UNUSED)
{
}

EOLIAN static void
_efl_net_control_access_point_ipv6_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED, Efl_Net_Control_Access_Point_Ipv6_Method *method EINA_UNUSED, const char **address EINA_UNUSED, uint8_t *prefix_length EINA_UNUSED, const char **netmask EINA_UNUSED, const char **gateway EINA_UNUSED)
{
}

EOLIAN static void
_efl_net_control_access_point_proxy_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED, Efl_Net_Control_Access_Point_Proxy_Method *method EINA_UNUSED, const char **url EINA_UNUSED, Eina_Iterator **servers EINA_UNUSED, Eina_Iterator **excludes EINA_UNUSED)
{
}

EOLIAN static void
_efl_net_control_access_point_configuration_name_servers_set(Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED, Eina_Iterator *name_servers EINA_UNUSED)
{
}

EOLIAN static Eina_Iterator *
_efl_net_control_access_point_configuration_name_servers_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED)
{
   return NULL;
}

EOLIAN static void
_efl_net_control_access_point_configuration_time_servers_set(Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED, Eina_Iterator *time_servers EINA_UNUSED)
{
}

EOLIAN static Eina_Iterator *
_efl_net_control_access_point_configuration_time_servers_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED)
{
   return NULL;
}

EOLIAN static void
_efl_net_control_access_point_configuration_domains_set(Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED, Eina_Iterator *domains EINA_UNUSED)
{
}

EOLIAN static Eina_Iterator *
_efl_net_control_access_point_configuration_domains_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED)
{
   return NULL;
}

EOLIAN static void
_efl_net_control_access_point_configuration_ipv4_set(Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED, Efl_Net_Control_Access_Point_Ipv4_Method method EINA_UNUSED, const char *address EINA_UNUSED, const char *netmask EINA_UNUSED, const char *gateway EINA_UNUSED)
{
}

EOLIAN static void
_efl_net_control_access_point_configuration_ipv4_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED, Efl_Net_Control_Access_Point_Ipv4_Method *method EINA_UNUSED, const char **address EINA_UNUSED, const char **netmask EINA_UNUSED, const char **gateway EINA_UNUSED)
{
}

EOLIAN static void
_efl_net_control_access_point_configuration_ipv6_set(Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED, Efl_Net_Control_Access_Point_Ipv6_Method method EINA_UNUSED, const char *address EINA_UNUSED, uint8_t prefix_length EINA_UNUSED, const char *netmask EINA_UNUSED, const char *gateway EINA_UNUSED)
{
}

EOLIAN static void
_efl_net_control_access_point_configuration_ipv6_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED, Efl_Net_Control_Access_Point_Ipv6_Method *method EINA_UNUSED, const char **address EINA_UNUSED, uint8_t *prefix_length EINA_UNUSED, const char **netmask EINA_UNUSED, const char **gateway EINA_UNUSED)
{
}

EOLIAN static void
_efl_net_control_access_point_configuration_proxy_set(Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED, Efl_Net_Control_Access_Point_Proxy_Method method EINA_UNUSED, const char *url EINA_UNUSED, Eina_Iterator *servers EINA_UNUSED, Eina_Iterator *excludes EINA_UNUSED)
{
}

EOLIAN static void
_efl_net_control_access_point_configuration_proxy_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED, Efl_Net_Control_Access_Point_Proxy_Method *method EINA_UNUSED, const char **url EINA_UNUSED, Eina_Iterator **servers EINA_UNUSED, Eina_Iterator **excludes EINA_UNUSED)
{
}

EOLIAN static Eina_Future *
_efl_net_control_access_point_connect(Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED)
{
   return eina_future_rejected(efl_loop_future_scheduler_get(obj),
                               EINA_ERROR_NOT_IMPLEMENTED);
}

EOLIAN static void
_efl_net_control_access_point_disconnect(Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED)
{
}

EOLIAN static void
_efl_net_control_access_point_forget(Eo *obj EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd EINA_UNUSED)
{
}

#include "efl_net_control_access_point.eo.c"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"
#include "efl_net-connman.h"

typedef struct
{
   /* Eldbus_Proxy/Eldbus_Object keeps a list of pending calls, but
    * they are reference counted singletons and will only cancel
    * pending calls when everything is gone.  However we operate on
    * our private data, that may be gone before other refs. So
    * keep the pending list.
    */
   Eina_List *pending;
   Eina_List *signal_handlers;
   Eldbus_Proxy *proxy;
   Eina_Stringshare *path;
   Eina_Stringshare *name;
   Eina_Stringshare *techname;
   struct {
      Eina_List *name_servers;
      Eina_List *time_servers;
      Eina_List *domains;
      struct {
         Efl_Net_Control_Access_Point_Ipv4_Method method;
         Eina_Stringshare *address;
         Eina_Stringshare *netmask;
         Eina_Stringshare *gateway;
      } ipv4;
      struct {
         Efl_Net_Control_Access_Point_Ipv6_Method method;
         Eina_Stringshare *address;
         Eina_Stringshare *netmask;
         Eina_Stringshare *gateway;
         uint8_t prefix_length;
      } ipv6;
      struct {
         Efl_Net_Control_Access_Point_Proxy_Method method;
         Eina_Stringshare *url;
         Eina_List *servers;
         Eina_List *excludes;
      } proxy;
   } actual, configured;
   unsigned int priority;
   Efl_Net_Control_Access_Point_State state;
   Efl_Net_Control_Access_Point_Error error;
   Efl_Net_Control_Access_Point_Security security;
   uint8_t strength;
   Eina_Bool roaming;
   Eina_Bool auto_connect;
   Eina_Bool remembered;
   Eina_Bool immutable;
} Efl_Net_Control_Access_Point_Data;

#define MY_CLASS EFL_NET_CONTROL_ACCESS_POINT_CLASS


static void
_efl_net_control_access_point_property_set_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   Eo *o = data;
   Efl_Net_Control_Access_Point_Data *pd = efl_data_scope_get(o, MY_CLASS);
   const char *err_name, *err_msg;

   pd->pending = eina_list_remove(pd->pending, pending);
   if (eldbus_message_error_get(msg, &err_name, &err_msg))
     {
        ERR("Could not set property %p: %s=%s", o, err_name, err_msg);
        return;
     }
}

static void
_efl_net_control_access_point_property_set_string_array(const Eo *o, Efl_Net_Control_Access_Point_Data *pd, const char *name, Eina_Iterator *it)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *msg_itr, *var, *array;
   Eldbus_Pending *p;
   const char *str;

   msg = eldbus_proxy_method_call_new(pd->proxy, "SetProperty");
   EINA_SAFETY_ON_NULL_GOTO(msg, error_msg);

   msg_itr = eldbus_message_iter_get(msg);
   EINA_SAFETY_ON_NULL_GOTO(msg_itr, error_send);

   eldbus_message_iter_basic_append(msg_itr, 's', name);
   var = eldbus_message_iter_container_new(msg_itr, 'v', "as");

   eldbus_message_iter_arguments_append(var, "as", &array);
   EINA_ITERATOR_FOREACH(it, str)
     eldbus_message_iter_basic_append(array, 's', str);
   eldbus_message_iter_container_close(var, array);
   eldbus_message_iter_container_close(msg_itr, var);
   eina_iterator_free(it);

   p = eldbus_proxy_send(pd->proxy, msg, _efl_net_control_access_point_property_set_cb, o, DEFAULT_TIMEOUT);
   EINA_SAFETY_ON_NULL_GOTO(p, error_send);

   pd->pending = eina_list_append(pd->pending, p);
   DBG("Setting property %s", name);
   return;

 error_send:
   eldbus_message_unref(msg);
 error_msg:
   eina_iterator_free(it);
}

static void
_efl_net_control_access_point_property_set(const Eo *o, Efl_Net_Control_Access_Point_Data *pd, const char *name, const char *signature, ...)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *msg_itr, *var;
   Eldbus_Pending *p;
   va_list ap;

   msg = eldbus_proxy_method_call_new(pd->proxy, "SetProperty");
   EINA_SAFETY_ON_NULL_RETURN(msg);

   msg_itr = eldbus_message_iter_get(msg);
   EINA_SAFETY_ON_NULL_GOTO(msg_itr, error_send);

   eldbus_message_iter_basic_append(msg_itr, 's', name);
   var = eldbus_message_iter_container_new(msg_itr, 'v', signature);

   va_start(ap, signature);
   eldbus_message_iter_arguments_vappend(var, signature, ap);
   va_end(ap);
   eldbus_message_iter_container_close(msg_itr, var);

   p = eldbus_proxy_send(pd->proxy, msg, _efl_net_control_access_point_property_set_cb, o, DEFAULT_TIMEOUT);
   EINA_SAFETY_ON_NULL_GOTO(p, error_send);

   pd->pending = eina_list_append(pd->pending, p);
   DBG("Setting property %s", name);
   return;

 error_send:
   eldbus_message_unref(msg);
}

EOLIAN static void
_efl_net_control_access_point_efl_object_destructor(Eo *o, Efl_Net_Control_Access_Point_Data *pd)
{
   Eldbus_Pending *p;
   Eldbus_Signal_Handler *sh;
   const char *str;

   EINA_LIST_FREE(pd->pending, p)
     eldbus_pending_cancel(p);

   EINA_LIST_FREE(pd->signal_handlers, sh)
     eldbus_signal_handler_del(sh);

   if (pd->proxy)
     {
        Eldbus_Object *obj = eldbus_proxy_object_get(pd->proxy);
        eldbus_proxy_unref(pd->proxy);
        pd->proxy = NULL;
        eldbus_object_unref(obj);
     }

   efl_destructor(efl_super(o, MY_CLASS));
   eina_stringshare_replace(&pd->path, NULL);
   eina_stringshare_replace(&pd->name, NULL);

   /* actual values */
   EINA_LIST_FREE(pd->actual.name_servers, str) eina_stringshare_del(str);
   EINA_LIST_FREE(pd->actual.time_servers, str) eina_stringshare_del(str);
   EINA_LIST_FREE(pd->actual.domains, str) eina_stringshare_del(str);

   eina_stringshare_replace(&pd->actual.ipv4.address, NULL);
   eina_stringshare_replace(&pd->actual.ipv4.netmask, NULL);
   eina_stringshare_replace(&pd->actual.ipv4.gateway, NULL);

   eina_stringshare_replace(&pd->actual.ipv6.address, NULL);
   eina_stringshare_replace(&pd->actual.ipv6.netmask, NULL);
   eina_stringshare_replace(&pd->actual.ipv6.gateway, NULL);

   eina_stringshare_replace(&pd->actual.proxy.url, NULL);
   EINA_LIST_FREE(pd->actual.proxy.servers, str) eina_stringshare_del(str);
   EINA_LIST_FREE(pd->actual.proxy.excludes, str) eina_stringshare_del(str);

   /* configured values */
   EINA_LIST_FREE(pd->configured.name_servers, str) eina_stringshare_del(str);
   EINA_LIST_FREE(pd->configured.time_servers, str) eina_stringshare_del(str);
   EINA_LIST_FREE(pd->configured.domains, str) eina_stringshare_del(str);

   eina_stringshare_replace(&pd->configured.ipv4.address, NULL);
   eina_stringshare_replace(&pd->configured.ipv4.netmask, NULL);
   eina_stringshare_replace(&pd->configured.ipv4.gateway, NULL);

   eina_stringshare_replace(&pd->configured.ipv6.address, NULL);
   eina_stringshare_replace(&pd->configured.ipv6.netmask, NULL);
   eina_stringshare_replace(&pd->configured.ipv6.gateway, NULL);

   eina_stringshare_replace(&pd->configured.proxy.url, NULL);
   EINA_LIST_FREE(pd->configured.proxy.servers, str) eina_stringshare_del(str);
   EINA_LIST_FREE(pd->configured.proxy.excludes, str) eina_stringshare_del(str);
}

EOLIAN static Efl_Net_Control_Access_Point_State
_efl_net_control_access_point_state_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd)
{
   return pd->state;
}

EOLIAN static Efl_Net_Control_Access_Point_Error
_efl_net_control_access_point_error_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd)
{
   return pd->error;
}

EOLIAN static const char *
_efl_net_control_access_point_ssid_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd)
{
   return pd->name;
}

static void
_efl_net_control_access_point_priority_set_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   Eo *o = data;
   Efl_Net_Control_Access_Point_Data *pd = efl_data_scope_get(o, MY_CLASS);
   const char *err_name, *err_msg;

   pd->pending = eina_list_remove(pd->pending, pending);
   if (eldbus_message_error_get(msg, &err_name, &err_msg))
     {
        ERR("Could not reorder %p: %s=%s", o, err_name, err_msg);
        return;
     }

   DBG("finished reordering %p", o);
   /* NOTE: it seems connman is not emiting ServicesChanged as expected */
   efl_net_connman_control_access_points_reload(efl_parent_get(o));
}

EOLIAN static void
_efl_net_control_access_point_priority_set(Eo *o, Efl_Net_Control_Access_Point_Data *pd, unsigned int priority)
{
   Efl_Net_Control_Access_Point_Data *other_pd;
   Eldbus_Pending *p;
   Eina_Iterator *it;
   Eo *ap = NULL, *last_ap = NULL, *sibling = NULL;
   int direction = 0;

   if (pd->priority == priority)
     {
        DBG("same priority %u, nothing to do for %s", priority, pd->name);
        return;
     }
   else if (!pd->remembered)
     {
        ERR("cannot change priority of non-remembered access point '%s'", pd->name);
        return;
     }

   it = efl_net_control_manager_access_points_get(efl_parent_get(o));
   EINA_ITERATOR_FOREACH(it, ap)
     {
        unsigned other_prio;
        if (ap == o) continue;
        else if (!efl_net_control_access_point_remembered_get(ap)) break;
        else if (priority == 0)
          {
             sibling = ap;
             direction = -1;
             break;
          }

        other_prio = efl_net_control_access_point_priority_get(ap);
        if (priority < other_prio) break;
        last_ap = ap;
     }
   eina_iterator_free(it);

   if ((!sibling) && (last_ap))
     {
        sibling = last_ap;
        if (priority <= efl_net_control_access_point_priority_get(last_ap))
          direction = -1;
        else
          direction = 1;
     }

   if (!sibling)
     {
        DBG("nothing to reorder priority %u for %s", priority, pd->name);
        return;
     }

   other_pd = efl_data_scope_get(sibling, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(other_pd);

   p = eldbus_proxy_call(pd->proxy,
                         (direction < 0) ? "MoveBefore" : "MoveAfter",
                         _efl_net_control_access_point_priority_set_cb, o, DEFAULT_TIMEOUT,
                         "o", other_pd->path);
   EINA_SAFETY_ON_NULL_RETURN(p);
   pd->pending = eina_list_append(pd->pending, p);
   DBG("Moving %s (%s) %s %s (%s)",
       pd->name, pd->path,
       (direction < 0) ? "before" : "after",
       other_pd->name, other_pd->path);
   return;
}

EOLIAN static unsigned int
_efl_net_control_access_point_priority_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd)
{
   return pd->priority;
}

EOLIAN static Efl_Net_Control_Technology *
_efl_net_control_access_point_technology_get(const Eo *o, Efl_Net_Control_Access_Point_Data *pd)
{
   return efl_net_connman_control_find_technology_by_type(efl_parent_get(o), pd->techname);
}

EOLIAN static uint8_t
_efl_net_control_access_point_strength_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd)
{
   return pd->strength;
}

EOLIAN static Eina_Bool
_efl_net_control_access_point_roaming_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd)
{
   return pd->roaming;
}

EOLIAN static void
_efl_net_control_access_point_auto_connect_set(Eo *o, Efl_Net_Control_Access_Point_Data *pd, Eina_Bool auto_connect)
{
   _efl_net_control_access_point_property_set(o, pd, "AutoConnect", "b", auto_connect);
}

EOLIAN static Eina_Bool
_efl_net_control_access_point_auto_connect_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd)
{
   return pd->auto_connect;
}

EOLIAN static Eina_Bool
_efl_net_control_access_point_remembered_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd)
{
   return pd->remembered;
}

EOLIAN static Eina_Bool
_efl_net_control_access_point_immutable_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd)
{
   return pd->immutable;
}

EOLIAN static Efl_Net_Control_Access_Point_Security
_efl_net_control_access_point_security_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd)
{
   return pd->security;
}

EOLIAN static Eina_Iterator *
_efl_net_control_access_point_name_servers_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd)
{
   return eina_list_iterator_new(pd->actual.name_servers);
}

EOLIAN static Eina_Iterator *
_efl_net_control_access_point_time_servers_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd)
{
   return eina_list_iterator_new(pd->actual.time_servers);
}

EOLIAN static Eina_Iterator *
_efl_net_control_access_point_domains_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd)
{
   return eina_list_iterator_new(pd->actual.domains);
}

EOLIAN static void
_efl_net_control_access_point_ipv4_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Efl_Net_Control_Access_Point_Ipv4_Method *method, const char **address, const char **netmask, const char **gateway)
{
   if (method) *method = pd->actual.ipv4.method;
   if (address) *address = pd->actual.ipv4.address;
   if (netmask) *netmask = pd->actual.ipv4.netmask;
   if (gateway) *gateway = pd->actual.ipv4.gateway;
}

EOLIAN static void
_efl_net_control_access_point_ipv6_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Efl_Net_Control_Access_Point_Ipv6_Method *method, const char **address, uint8_t *prefix_length, const char **netmask, const char **gateway)
{
   if (method) *method = pd->actual.ipv6.method;
   if (address) *address = pd->actual.ipv6.address;
   if (netmask) *netmask = pd->actual.ipv6.netmask;
   if (gateway) *gateway = pd->actual.ipv6.gateway;
   if (prefix_length) *prefix_length = pd->actual.ipv6.prefix_length;
}

EOLIAN static void
_efl_net_control_access_point_proxy_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Efl_Net_Control_Access_Point_Proxy_Method *method, const char **url, Eina_Iterator **servers, Eina_Iterator **excludes)
{
   if (method) *method = pd->actual.proxy.method;
   if (url) *url = pd->actual.proxy.url;
   if (servers) *servers = eina_list_iterator_new(pd->actual.proxy.servers);
   if (excludes) *excludes = eina_list_iterator_new(pd->actual.proxy.excludes);
}

EOLIAN static void
_efl_net_control_access_point_configuration_name_servers_set(Eo *o, Efl_Net_Control_Access_Point_Data *pd, Eina_Iterator *name_servers)
{
   _efl_net_control_access_point_property_set_string_array(o, pd, "Nameservers.Configuration", name_servers);
}

EOLIAN static Eina_Iterator *
_efl_net_control_access_point_configuration_name_servers_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd)
{
   return eina_list_iterator_new(pd->configured.name_servers);
}

EOLIAN static void
_efl_net_control_access_point_configuration_time_servers_set(Eo *o, Efl_Net_Control_Access_Point_Data *pd, Eina_Iterator *time_servers)
{
   _efl_net_control_access_point_property_set_string_array(o, pd, "Timeservers.Configuration", time_servers);
}

EOLIAN static Eina_Iterator *
_efl_net_control_access_point_configuration_time_servers_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd)
{
   return eina_list_iterator_new(pd->configured.time_servers);
}

EOLIAN static void
_efl_net_control_access_point_configuration_domains_set(Eo *o, Efl_Net_Control_Access_Point_Data *pd, Eina_Iterator *domains)
{
   _efl_net_control_access_point_property_set_string_array(o, pd, "Domains.Configuration", domains);
}

EOLIAN static Eina_Iterator *
_efl_net_control_access_point_configuration_domains_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd)
{
   return eina_list_iterator_new(pd->configured.domains);
}

static void
_append_dict_entry(Eldbus_Message_Iter *array, const char *name, const char *signature, ...)
{
   Eldbus_Message_Iter *entry, *var;
   va_list ap;

   if (!eldbus_message_iter_arguments_append(array, "{sv}", &entry))
     {
        ERR("could not append dict entry");
        return;
     }

   eldbus_message_iter_basic_append(entry, 's', name);
   var = eldbus_message_iter_container_new(entry, 'v', signature);

   va_start(ap, signature);
   eldbus_message_iter_arguments_vappend(var, signature, ap);
   va_end(ap);
   eldbus_message_iter_container_close(entry, var);
   eldbus_message_iter_container_close(array, entry);
}

static void
_append_dict_entry_string_array(Eldbus_Message_Iter *array, const char *name, Eina_Iterator *it)
{
   Eldbus_Message_Iter *entry, *var, *sub;
   const char *str;

   if (!eldbus_message_iter_arguments_append(array, "{sv}", &entry))
     {
        ERR("could not append dict entry");
        return;
     }

   eldbus_message_iter_basic_append(entry, 's', name);
   var = eldbus_message_iter_container_new(entry, 'v', "as");

   eldbus_message_iter_arguments_append(var, "as", &sub);
   EINA_ITERATOR_FOREACH(it, str)
     eldbus_message_iter_basic_append(sub, 's', str);

   eldbus_message_iter_container_close(var, sub);
   eldbus_message_iter_container_close(entry, var);
   eldbus_message_iter_container_close(array, entry);
}

EOLIAN static void
_efl_net_control_access_point_configuration_ipv4_set(Eo *o, Efl_Net_Control_Access_Point_Data *pd, Efl_Net_Control_Access_Point_Ipv4_Method method, const char *address, const char *netmask, const char *gateway)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *msg_itr, *array, *var;
   Eldbus_Pending *p;

   if (method == EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_UNSET)
     {
        ERR("Invalid IPv4 Method (%d) EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_UNSET\n", method);
        return;
     }

   msg = eldbus_proxy_method_call_new(pd->proxy, "SetProperty");
   EINA_SAFETY_ON_NULL_RETURN(msg);

   msg_itr = eldbus_message_iter_get(msg);
   EINA_SAFETY_ON_NULL_GOTO(msg_itr, error_send);

   eldbus_message_iter_basic_append(msg_itr, 's', "IPv4.Configuration");
   var = eldbus_message_iter_container_new(msg_itr, 'v', "a{sv}");
   eldbus_message_iter_arguments_append(var, "a{sv}", &array);

   switch (method)
     {
      case EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_OFF:
         _append_dict_entry(array, "Method", "s", "off");
         break;
      case EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_DHCP:
         _append_dict_entry(array, "Method", "s", "dhcp");
         break;
      case EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_MANUAL:
         _append_dict_entry(array, "Method", "s", "manual");
         if (address)
           _append_dict_entry(array, "Address", "s", address);
         if (netmask)
           _append_dict_entry(array, "Netmask", "s", netmask);
         if (gateway)
           _append_dict_entry(array, "Gateway", "s", gateway);
         break;
      default:
         break;
     }

   eldbus_message_iter_container_close(var, array);
   eldbus_message_iter_container_close(msg_itr, var);

   p = eldbus_proxy_send(pd->proxy, msg, _efl_net_control_access_point_property_set_cb, o, DEFAULT_TIMEOUT);
   EINA_SAFETY_ON_NULL_GOTO(p, error_send);

   pd->pending = eina_list_append(pd->pending, p);
   DBG("Setting property IPv4");
   return;

 error_send:
   eldbus_message_unref(msg);
}

EOLIAN static void
_efl_net_control_access_point_configuration_ipv4_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Efl_Net_Control_Access_Point_Ipv4_Method *method, const char **address, const char **netmask, const char **gateway)
{
   if (method) *method = pd->configured.ipv4.method;
   if (address) *address = pd->configured.ipv4.address;
   if (netmask) *netmask = pd->configured.ipv4.netmask;
   if (gateway) *gateway = pd->configured.ipv4.gateway;
}

EOLIAN static void
_efl_net_control_access_point_configuration_ipv6_set(Eo *o, Efl_Net_Control_Access_Point_Data *pd, Efl_Net_Control_Access_Point_Ipv6_Method method, const char *address, uint8_t prefix_length, const char *netmask, const char *gateway)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *msg_itr, *array, *var;
   Eldbus_Pending *p;

   if (method == EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_UNSET)
     {
        ERR("Invalid IPv6 Method (%d) EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_UNSET\n", method);
        return;
     }
   else if (method == EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_FIXED)
     {
        ERR("Invalid IPv6 Method (%d) EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_FIXED\n", method);
        return;
     }
   else if (method == EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_TUNNEL6TO4)
     {
        ERR("Invalid IPv6 Method (%d) EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_TUNNEL6TO4\n", method);
        return;
     }

   msg = eldbus_proxy_method_call_new(pd->proxy, "SetProperty");
   EINA_SAFETY_ON_NULL_RETURN(msg);

   msg_itr = eldbus_message_iter_get(msg);
   EINA_SAFETY_ON_NULL_GOTO(msg_itr, error_send);

   eldbus_message_iter_basic_append(msg_itr, 's', "IPv6.Configuration");
   var = eldbus_message_iter_container_new(msg_itr, 'v', "a{sv}");
   eldbus_message_iter_arguments_append(var, "a{sv}", &array);

   switch (method)
     {
      case EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_OFF:
         _append_dict_entry(array, "Method", "s", "off");
         break;
      case EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_MANUAL:
         _append_dict_entry(array, "Method", "s", "manual");
         if (address)
           _append_dict_entry(array, "Address", "s", address);
         if (netmask)
           _append_dict_entry(array, "Netmask", "s", netmask);
         if (gateway)
           _append_dict_entry(array, "Gateway", "s", gateway);
         if (prefix_length)
           _append_dict_entry(array, "PrefixLength", "y", prefix_length);
         break;
      case EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_AUTO_PRIVACY_NONE:
         _append_dict_entry(array, "Method", "s", "auto");
         _append_dict_entry(array, "Privacy", "s", "disabled");
         break;
      case EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_AUTO_PRIVACY_PUBLIC:
         _append_dict_entry(array, "Method", "s", "auto");
         _append_dict_entry(array, "Privacy", "s", "enabled");
         break;
      case EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_AUTO_PRIVACY_TEMPORARY:
         _append_dict_entry(array, "Method", "s", "auto");
         _append_dict_entry(array, "Privacy", "s", "preferred");
         break;
      default:
         break;
     }

   eldbus_message_iter_container_close(var, array);
   eldbus_message_iter_container_close(msg_itr, var);

   p = eldbus_proxy_send(pd->proxy, msg, _efl_net_control_access_point_property_set_cb, o, DEFAULT_TIMEOUT);
   EINA_SAFETY_ON_NULL_GOTO(p, error_send);

   pd->pending = eina_list_append(pd->pending, p);
   DBG("Setting property IPv6");
   return;

 error_send:
   eldbus_message_unref(msg);
}

EOLIAN static void
_efl_net_control_access_point_configuration_ipv6_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Efl_Net_Control_Access_Point_Ipv6_Method *method, const char **address, uint8_t *prefix_length, const char **netmask, const char **gateway)
{
   if (method) *method = pd->configured.ipv6.method;
   if (address) *address = pd->configured.ipv6.address;
   if (netmask) *netmask = pd->configured.ipv6.netmask;
   if (gateway) *gateway = pd->configured.ipv6.gateway;
   if (prefix_length) *prefix_length = pd->configured.ipv6.prefix_length;
}

EOLIAN static void
_efl_net_control_access_point_configuration_proxy_set(Eo *o, Efl_Net_Control_Access_Point_Data *pd, Efl_Net_Control_Access_Point_Proxy_Method method, const char *url, Eina_Iterator *servers, Eina_Iterator *excludes)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *msg_itr, *array, *var;
   Eldbus_Pending *p;

   if (method == EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_UNSET)
     {
        ERR("Invalid Proxy Method (%d) EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_UNSET\n", method);
        if (servers) eina_iterator_free(servers);
        if (excludes) eina_iterator_free(excludes);
        return;
     }

   msg = eldbus_proxy_method_call_new(pd->proxy, "SetProperty");
   EINA_SAFETY_ON_NULL_RETURN(msg);

   msg_itr = eldbus_message_iter_get(msg);
   EINA_SAFETY_ON_NULL_GOTO(msg_itr, error_send);

   eldbus_message_iter_basic_append(msg_itr, 's', "Proxy.Configuration");
   var = eldbus_message_iter_container_new(msg_itr, 'v', "a{sv}");
   eldbus_message_iter_arguments_append(var, "a{sv}", &array);

   switch (method)
     {
      case EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_OFF:
         _append_dict_entry(array, "Method", "s", "direct");
         break;
      case EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_AUTO:
         _append_dict_entry(array, "Method", "s", "auto");
         if (url)
           _append_dict_entry(array, "URL", "s", url);
         break;
      case EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_MANUAL:
         _append_dict_entry(array, "Method", "s", "manual");
         if (servers)
           _append_dict_entry_string_array(array, "Servers", servers);
         if (excludes)
           _append_dict_entry_string_array(array, "Excludes", excludes);
         break;
      default:
         break;
     }

   eldbus_message_iter_container_close(var, array);
   eldbus_message_iter_container_close(msg_itr, var);

   p = eldbus_proxy_send(pd->proxy, msg, _efl_net_control_access_point_property_set_cb, o, DEFAULT_TIMEOUT);
   EINA_SAFETY_ON_NULL_GOTO(p, error_send);

   pd->pending = eina_list_append(pd->pending, p);
   DBG("Setting property Proxy");
   if (servers) eina_iterator_free(servers);
   if (excludes) eina_iterator_free(excludes);
   return;

 error_send:
   if (servers) eina_iterator_free(servers);
   if (excludes) eina_iterator_free(excludes);
   eldbus_message_unref(msg);
}

EOLIAN static void
_efl_net_control_access_point_configuration_proxy_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Efl_Net_Control_Access_Point_Proxy_Method *method, const char **url, Eina_Iterator **servers, Eina_Iterator **excludes)
{
   if (method) *method = pd->configured.proxy.method;
   if (url) *url = pd->configured.proxy.url;
   if (servers) *servers = eina_list_iterator_new(pd->configured.proxy.servers);
   if (excludes) *excludes = eina_list_iterator_new(pd->configured.proxy.excludes);
}

static void
_efl_net_control_access_point_connect_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   Eina_Promise *promise = data;
   Efl_Object *o = eina_promise_data_get(promise);
   Efl_Net_Control_Access_Point_Data *pd = efl_data_scope_get(o, MY_CLASS);
   const char *err_name, *err_msg;

   EINA_SAFETY_ON_NULL_RETURN(pd);

   pd->pending = eina_list_remove(pd->pending, pending);
   if (eldbus_message_error_get(msg, &err_name, &err_msg))
     {
        Eina_Error err = EINVAL;

        if (strcmp(err_name, "net.connman.Error.InProgress") == 0)
          err = EINPROGRESS;
        else if (strcmp(err_name, "net.connman.Error.AlreadyConnected") == 0)
          err = EALREADY;
        WRN("Could not Connect %p: %s=%s", o, err_name, err_msg);

        eina_promise_reject(promise, err);
        return;
     }

   eina_promise_resolve(promise, EINA_VALUE_EMPTY);
}

static void
_efl_net_control_access_point_connect_promise_del(void *data, const Eina_Promise *dead_ptr)
{
   Eldbus_Pending *p = data;
   Efl_Net_Control_Access_Point_Data *pd;

   p = eina_promise_data_get(dead_ptr);
   if (!p) return; /* already gone, nothing to do */

   pd = eldbus_pending_data_get(p, ".object");

   pd->pending = eina_list_remove(pd->pending, p);
   DBG("cancel pending connect %p", p);
   eldbus_pending_cancel(p);
}

EOLIAN static Eina_Future *
_efl_net_control_access_point_connect(Eo *o, Efl_Net_Control_Access_Point_Data *pd)
{
   Eldbus_Pending *p;
   Eina_Promise *promise;
   Eina_Future *f = NULL;

   promise = eina_promise_new(efl_loop_future_scheduler_get(o),
                              _efl_net_control_access_point_connect_promise_del, o);
   EINA_SAFETY_ON_NULL_RETURN_VAL(promise, NULL);

   f = eina_future_new(promise);

   p = eldbus_proxy_call(pd->proxy, "Connect",
                         _efl_net_control_access_point_connect_cb, promise, -1.0, "");
   EINA_SAFETY_ON_NULL_GOTO(p, error_dbus);

   pd->pending = eina_list_append(pd->pending, p);
   eldbus_pending_data_set(p, ".object", pd);

   return efl_future_Eina_FutureXXX_then(o, f);

 error_dbus:
   eina_promise_reject(promise, ENOSYS);
   return efl_future_Eina_FutureXXX_then(o, f);
}

static void
_efl_net_control_access_point_disconnect_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   Eo *o = data;
   Efl_Net_Control_Access_Point_Data *pd = efl_data_scope_get(o, MY_CLASS);
   const char *err_name, *err_msg;

   pd->pending = eina_list_remove(pd->pending, pending);
   if (eldbus_message_error_get(msg, &err_name, &err_msg))
     {
        ERR("Could not disconnect %p: %s=%s", o, err_name, err_msg);
        return;
     }
}

EOLIAN static void
_efl_net_control_access_point_disconnect(Eo *o, Efl_Net_Control_Access_Point_Data *pd)
{
   Eldbus_Pending *p;
   p = eldbus_proxy_call(pd->proxy, "Disconnect",
                         _efl_net_control_access_point_disconnect_cb, o, DEFAULT_TIMEOUT, "");
   EINA_SAFETY_ON_NULL_RETURN(p);
   pd->pending = eina_list_append(pd->pending, p);
}

static void
_efl_net_control_access_point_forget_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   Eo *o = data;
   Efl_Net_Control_Access_Point_Data *pd = efl_data_scope_get(o, MY_CLASS);
   const char *err_name, *err_msg;

   pd->pending = eina_list_remove(pd->pending, pending);
   if (eldbus_message_error_get(msg, &err_name, &err_msg))
     {
        ERR("Could not forget %p: %s=%s", o, err_name, err_msg);
        return;
     }
}

EOLIAN static void
_efl_net_control_access_point_forget(Eo *o, Efl_Net_Control_Access_Point_Data *pd)
{
   Eldbus_Pending *p;
   p = eldbus_proxy_call(pd->proxy, "Remove",
                         _efl_net_control_access_point_forget_cb, o, DEFAULT_TIMEOUT, "");
   EINA_SAFETY_ON_NULL_RETURN(p);
   pd->pending = eina_list_append(pd->pending, p);
}

static void
_efl_net_control_access_point_property_name_changed(Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Eldbus_Message_Iter *value)
{
   const char *name;

   if (!eldbus_message_iter_arguments_get(value, "s", &name))
     {
        ERR("Expected string, got %s", eldbus_message_iter_signature_get(value));
        return;
     }

   if (!eina_stringshare_replace(&pd->name, name)) return;
   DBG("name=%s", name);
}

static void
_efl_net_control_access_point_property_state_changed(Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Eldbus_Message_Iter *value)
{
   const char *str;
   const struct {
      Efl_Net_Control_Access_Point_State val;
      const char *str;
   } *itr, map[] = {
     {EFL_NET_CONTROL_ACCESS_POINT_STATE_IDLE, "idle"},
     {EFL_NET_CONTROL_ACCESS_POINT_STATE_ASSOCIATION, "association"},
     {EFL_NET_CONTROL_ACCESS_POINT_STATE_CONFIGURATION, "configuration"},
     {EFL_NET_CONTROL_ACCESS_POINT_STATE_LOCAL, "ready"},
     {EFL_NET_CONTROL_ACCESS_POINT_STATE_ONLINE, "online"},
     {EFL_NET_CONTROL_ACCESS_POINT_STATE_DISCONNECT, "disconnect"},
     {EFL_NET_CONTROL_ACCESS_POINT_STATE_FAILURE, "failure"},
     { }
   };

   if (!eldbus_message_iter_arguments_get(value, "s", &str))
     {
        ERR("Expected string, got %s", eldbus_message_iter_signature_get(value));
        return;
     }

   for (itr = map; itr->str != NULL; itr++)
     if (strcmp(itr->str, str) == 0) break;

   if (!itr->str)
     {
        ERR("Unknown state '%s'", str);
        return;
     }

   if (pd->state == itr->val) return;
   pd->state = itr->val;
   DBG("state=%d (%s)", pd->state, str);
}

static void
_efl_net_control_access_point_property_error_changed(Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Eldbus_Message_Iter *value)
{
   const char *str;
   const struct {
      Efl_Net_Control_Access_Point_Error val;
      const char *str;
   } *itr, map[] = {
     {EFL_NET_CONTROL_ACCESS_POINT_ERROR_NONE, "none"},
     {EFL_NET_CONTROL_ACCESS_POINT_ERROR_NONE, ""},
     {EFL_NET_CONTROL_ACCESS_POINT_ERROR_OUT_OF_RANGE, "range"},
     {EFL_NET_CONTROL_ACCESS_POINT_ERROR_PIN_MISSING, "missing"},
     {EFL_NET_CONTROL_ACCESS_POINT_ERROR_DHCP_FAILED, "failed"},
     {EFL_NET_CONTROL_ACCESS_POINT_ERROR_CONNECT_FAILED, "failed"},
     {EFL_NET_CONTROL_ACCESS_POINT_ERROR_LOGIN_FAILED, "failed"},
     { }
   };

   if (!eldbus_message_iter_arguments_get(value, "s", &str))
     {
        ERR("Expected string, got %s", eldbus_message_iter_signature_get(value));
        return;
     }

   for (itr = map; itr->str != NULL; itr++)
     if (strcmp(itr->str, str) == 0) break;

   if (!itr->str)
     {
        ERR("Unknown error '%s'", str);
        return;
     }

   if (pd->error == itr->val) return;
   pd->error = itr->val;
   DBG("error=%d (%s)", pd->error, str);
}

static void
_efl_net_control_access_point_property_type_changed(Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Eldbus_Message_Iter *value)
{
   const char *name;

   if (!eldbus_message_iter_arguments_get(value, "s", &name))
     {
        ERR("Expected string, got %s", eldbus_message_iter_signature_get(value));
        return;
     }

   if (!eina_stringshare_replace(&pd->techname, name)) return;
   DBG("technology name=%s", name);
}

static void
_efl_net_control_access_point_property_security_changed(Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Eldbus_Message_Iter *value)
{
   const char *str;
   Eldbus_Message_Iter *array;
   const struct {
      Efl_Net_Control_Access_Point_Security val;
      const char *str;
   } *itr, map[] = {
     {EFL_NET_CONTROL_ACCESS_POINT_SECURITY_NONE, "none"},
     {EFL_NET_CONTROL_ACCESS_POINT_SECURITY_WEP, "wep"},
     {EFL_NET_CONTROL_ACCESS_POINT_SECURITY_PSK, "psk"},
     {EFL_NET_CONTROL_ACCESS_POINT_SECURITY_IEEE802_1X, "ieee8021x"},
     { }
   };
   Efl_Net_Control_Access_Point_Security security = 0;

   if (!eldbus_message_iter_arguments_get(value, "as", &array))
     {
        ERR("Expected array of string, got %s", eldbus_message_iter_signature_get(value));
        return;
     }

   while (eldbus_message_iter_get_and_next(array, 's', &str))
     {
        for (itr = map; itr->str != NULL; itr++)
          if (strcmp(itr->str, str) == 0) break;

        if (!itr->str)
          {
             ERR("Unknown security '%s'", str);
             continue;
          }

        security |= itr->val;
     }

   if (pd->security == security) return;
   pd->security = security;
   DBG("security=%#x", pd->security);
}

static void
_efl_net_control_access_point_property_strength_changed(Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Eldbus_Message_Iter *value)
{
   uint8_t strength;

   if (!eldbus_message_iter_arguments_get(value, "y", &strength))
     {
        ERR("Expected unsigned byte, got %s", eldbus_message_iter_signature_get(value));
        return;
     }

   if (pd->strength == strength) return;
   pd->strength = strength;
   DBG("strength=%hhu", pd->strength);
}

static void
_efl_net_control_access_point_property_remembered_changed(Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Eldbus_Message_Iter *value)
{
   Eina_Bool remembered;

   if (!eldbus_message_iter_arguments_get(value, "b", &remembered))
     {
        ERR("Expected boolean, got %s", eldbus_message_iter_signature_get(value));
        return;
     }

   if (pd->remembered == remembered) return;
   pd->remembered = remembered;
   if (!remembered)
     {
        /* force this flag to be cleared */
        pd->auto_connect = EINA_FALSE;
     }
   DBG("remembered=%hhu", remembered);
}

static void
_efl_net_control_access_point_property_immutable_changed(Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Eldbus_Message_Iter *value)
{
   Eina_Bool immutable;

   if (!eldbus_message_iter_arguments_get(value, "b", &immutable))
     {
        ERR("Expected boolean, got %s", eldbus_message_iter_signature_get(value));
        return;
     }

   if (pd->immutable == immutable) return;
   pd->immutable = immutable;
   DBG("immutable=%hhu", immutable);
}

static void
_efl_net_control_access_point_property_auto_connect_changed(Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Eldbus_Message_Iter *value)
{
   Eina_Bool auto_connect;

   if (!eldbus_message_iter_arguments_get(value, "b", &auto_connect))
     {
        ERR("Expected boolean, got %s", eldbus_message_iter_signature_get(value));
        return;
     }

   if (pd->auto_connect == auto_connect) return;
   pd->auto_connect = auto_connect;
   DBG("auto_connect=%hhu", auto_connect);
}

static void
_efl_net_control_access_point_property_roaming_changed(Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Eldbus_Message_Iter *value)
{
   Eina_Bool roaming;

   if (!eldbus_message_iter_arguments_get(value, "b", &roaming))
     {
        ERR("Expected boolean, got %s", eldbus_message_iter_signature_get(value));
        return;
     }

   if (pd->roaming == roaming) return;
   pd->roaming = roaming;
   DBG("roaming=%hhu", roaming);
}

static void
_efl_net_control_access_point_list_updated(const char *name, Eina_List **p_list, Eldbus_Message_Iter *value)
{
   Eldbus_Message_Iter *array;
   Eina_List *old_list;
   const char *str;

   if (!eldbus_message_iter_arguments_get(value, "as", &array))
     {
        ERR("Expected array of strings for %s, got %s", name, eldbus_message_iter_signature_get(value));
        return;
     }

   old_list = *p_list;
   *p_list = NULL;
   while (eldbus_message_iter_get_and_next(array, 's', &str))
     *p_list = eina_list_append(*p_list, eina_stringshare_add(str));

   EINA_LIST_FREE(old_list, str) eina_stringshare_del(str);
}

/* Actual Values */

static void
_efl_net_control_access_point_property_actual_name_servers_changed(Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Eldbus_Message_Iter *value)
{
   _efl_net_control_access_point_list_updated("name_servers", &pd->actual.name_servers, value);
}

static void
_efl_net_control_access_point_property_actual_time_servers_changed(Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Eldbus_Message_Iter *value)
{
   _efl_net_control_access_point_list_updated("time_servers", &pd->actual.time_servers, value);
}

static void
_efl_net_control_access_point_property_actual_domains_changed(Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Eldbus_Message_Iter *value)
{
   _efl_net_control_access_point_list_updated("domains", &pd->actual.domains, value);
}

static void
_efl_net_control_access_point_property_actual_ipv4_changed(Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Eldbus_Message_Iter *value)
{
   Eldbus_Message_Iter *array, *entry;

   if (!eldbus_message_iter_arguments_get(value, "a{sv}", &array))
     {
        ERR("Expected dict for, got %s", eldbus_message_iter_signature_get(value));
        return;
     }

   pd->actual.ipv4.method = EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_UNSET;
   eina_stringshare_replace(&pd->actual.ipv4.address, NULL);
   eina_stringshare_replace(&pd->actual.ipv4.netmask, NULL);
   eina_stringshare_replace(&pd->actual.ipv4.gateway, NULL);

   while (eldbus_message_iter_get_and_next(array, 'e', &entry))
     {
        const char *name, *str;
        Eldbus_Message_Iter *var;

        if (!eldbus_message_iter_arguments_get(entry, "sv", &name, &var))
          {
             ERR("Unexpected dict entry signature: %s", eldbus_message_iter_signature_get(entry));
             continue;
          }

        if (!eldbus_message_iter_arguments_get(var, "s", &str))
          {
             ERR("Expected string value for %s, got %s", name, eldbus_message_iter_signature_get(var));
             continue;
          }

        if (strcmp(name, "Method") == 0)
          {
             if (strcmp(str, "dhcp") == 0)
               pd->actual.ipv4.method = EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_DHCP;
             else if (strcmp(str, "manual") == 0)
               pd->actual.ipv4.method = EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_MANUAL;
             else if (strcmp(str, "off") == 0)
               pd->actual.ipv4.method = EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_OFF;
             else
               WRN("Unexpected IPV4 Method value '%s'", str);
          }
        else if (strcmp(name, "Address") == 0)
          eina_stringshare_replace(&pd->actual.ipv4.address, str);
        else if (strcmp(name, "Netmask") == 0)
          eina_stringshare_replace(&pd->actual.ipv4.netmask, str);
        else if (strcmp(name, "Gateway") == 0)
          eina_stringshare_replace(&pd->actual.ipv4.gateway, str);
        else
          WRN("Unknown property name: %s", name);
     }
}

static void
_efl_net_control_access_point_property_actual_ipv6_changed(Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Eldbus_Message_Iter *value)
{
   Eldbus_Message_Iter *array, *entry;

   if (!eldbus_message_iter_arguments_get(value, "a{sv}", &array))
     {
        ERR("Expected dict for, got %s", eldbus_message_iter_signature_get(value));
        return;
     }

   pd->actual.ipv6.method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_UNSET;
   eina_stringshare_replace(&pd->actual.ipv6.address, NULL);
   eina_stringshare_replace(&pd->actual.ipv6.netmask, NULL);
   eina_stringshare_replace(&pd->actual.ipv6.gateway, NULL);
   pd->actual.ipv6.prefix_length = 0;

   while (eldbus_message_iter_get_and_next(array, 'e', &entry))
     {
        const char *name, *str;
        Eldbus_Message_Iter *var;

        if (!eldbus_message_iter_arguments_get(entry, "sv", &name, &var))
          {
             ERR("Unexpected dict entry signature: %s", eldbus_message_iter_signature_get(entry));
             continue;
          }

        if (strcmp(name, "PrefixLength") == 0)
          {
             if (!eldbus_message_iter_arguments_get(var, "y", &pd->actual.ipv6.prefix_length))
               ERR("Expected unsigned byte value for %s, got %s", name, eldbus_message_iter_signature_get(var));
             continue;
          }

        if (!eldbus_message_iter_arguments_get(var, "s", &str))
          {
             ERR("Expected string value for %s, got %s", name, eldbus_message_iter_signature_get(var));
             continue;
          }

        if (strcmp(name, "Method") == 0)
          {
             if (strcmp(str, "off") == 0)
               pd->actual.ipv6.method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_OFF;
             else if (strcmp(str, "fixed") == 0)
               pd->actual.ipv6.method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_FIXED;
             else if (strcmp(str, "manual") == 0)
               pd->actual.ipv6.method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_MANUAL;
             else if (strcmp(str, "6to4") == 0)
               pd->actual.ipv6.method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_TUNNEL6TO4;
             else if (strcmp(str, "auto") == 0)
               pd->actual.ipv6.method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_AUTO_PRIVACY_NONE;
             else
               WRN("Unexpected IPV6 Method value '%s'", str);
          }
        else if (strcmp(name, "Privacy") == 0)
          {
             if ((pd->actual.ipv6.method != EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_AUTO_PRIVACY_NONE) &&
                 (pd->actual.ipv6.method != EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_UNSET))
               {
                  DBG("Skip privacy %s, method already set to %d", str, pd->actual.ipv6.method);
                  continue;
               }
             if (strcmp(str, "disabled") == 0)
               pd->actual.ipv6.method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_AUTO_PRIVACY_NONE;
             else if (strcmp(str, "enabled") == 0)
               pd->actual.ipv6.method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_AUTO_PRIVACY_PUBLIC;
             else if (strcmp(str, "preferred") == 0)
               pd->actual.ipv6.method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_AUTO_PRIVACY_TEMPORARY;
             else
               WRN("Unexpected IPV6 Privacy value '%s'", str);
          }
        else if (strcmp(name, "Address") == 0)
          eina_stringshare_replace(&pd->actual.ipv6.address, str);
        else if (strcmp(name, "Netmask") == 0)
          eina_stringshare_replace(&pd->actual.ipv6.netmask, str);
        else if (strcmp(name, "Gateway") == 0)
          eina_stringshare_replace(&pd->actual.ipv6.gateway, str);
        else
          WRN("Unknown property name: %s", name);
     }
}

static void
_efl_net_control_access_point_property_actual_proxy_changed(Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Eldbus_Message_Iter *value)
{
   Eldbus_Message_Iter *array, *entry;
   const char *str;

   if (!eldbus_message_iter_arguments_get(value, "a{sv}", &array))
     {
        ERR("Expected dict for, got %s", eldbus_message_iter_signature_get(value));
        return;
     }

   pd->actual.proxy.method = EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_UNSET;
   eina_stringshare_replace(&pd->actual.proxy.url, NULL);
   EINA_LIST_FREE(pd->actual.proxy.servers, str) eina_stringshare_del(str);
   EINA_LIST_FREE(pd->actual.proxy.excludes, str) eina_stringshare_del(str);

   while (eldbus_message_iter_get_and_next(array, 'e', &entry))
     {
        const char *name;
        Eldbus_Message_Iter *var;

        if (!eldbus_message_iter_arguments_get(entry, "sv", &name, &var))
          {
             ERR("Unexpected dict entry signature: %s", eldbus_message_iter_signature_get(entry));
             continue;
          }

        if (strcmp(name, "Servers") == 0)
          {
             _efl_net_control_access_point_list_updated("Proxy Servers", &pd->actual.proxy.servers, var);
             continue;
          }
        else if (strcmp(name, "Excludes") == 0)
          {
             _efl_net_control_access_point_list_updated("Proxy Excludes", &pd->actual.proxy.excludes, var);
             continue;
          }

        if (!eldbus_message_iter_arguments_get(var, "s", &str))
          {
             ERR("Expected string value for %s, got %s", name, eldbus_message_iter_signature_get(var));
             continue;
          }

        if (strcmp(name, "Method") == 0)
          {
             if (strcmp(str, "auto") == 0)
               pd->actual.proxy.method = EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_AUTO;
             else if (strcmp(str, "manual") == 0)
               pd->actual.proxy.method = EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_MANUAL;
             else if (strcmp(str, "direct") == 0)
               pd->actual.proxy.method = EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_OFF;
             else
               WRN("Unexpected PROXY Method value '%s'", str);
          }
        else if (strcmp(name, "URL") == 0)
          eina_stringshare_replace(&pd->actual.proxy.url, str);
        else
          WRN("Unknown property name: %s", name);
     }
}

/* Configured Values */

static void
_efl_net_control_access_point_property_configured_name_servers_changed(Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Eldbus_Message_Iter *value)
{
   _efl_net_control_access_point_list_updated("name_servers", &pd->configured.name_servers, value);
}

static void
_efl_net_control_access_point_property_configured_time_servers_changed(Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Eldbus_Message_Iter *value)
{
   _efl_net_control_access_point_list_updated("time_servers", &pd->configured.time_servers, value);
}

static void
_efl_net_control_access_point_property_configured_domains_changed(Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Eldbus_Message_Iter *value)
{
   _efl_net_control_access_point_list_updated("domains", &pd->configured.domains, value);
}

static void
_efl_net_control_access_point_property_configured_ipv4_changed(Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Eldbus_Message_Iter *value)
{
   Eldbus_Message_Iter *array, *entry;

   if (!eldbus_message_iter_arguments_get(value, "a{sv}", &array))
     {
        ERR("Expected dict for, got %s", eldbus_message_iter_signature_get(value));
        return;
     }

   pd->configured.ipv4.method = EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_UNSET;
   eina_stringshare_replace(&pd->configured.ipv4.address, NULL);
   eina_stringshare_replace(&pd->configured.ipv4.netmask, NULL);
   eina_stringshare_replace(&pd->configured.ipv4.gateway, NULL);

   while (eldbus_message_iter_get_and_next(array, 'e', &entry))
     {
        const char *name, *str;
        Eldbus_Message_Iter *var;

        if (!eldbus_message_iter_arguments_get(entry, "sv", &name, &var))
          {
             ERR("Unexpected dict entry signature: %s", eldbus_message_iter_signature_get(entry));
             continue;
          }

        if (!eldbus_message_iter_arguments_get(var, "s", &str))
          {
             ERR("Expected string value for %s, got %s", name, eldbus_message_iter_signature_get(var));
             continue;
          }

        if (strcmp(name, "Method") == 0)
          {
             if (strcmp(str, "dhcp") == 0)
               pd->configured.ipv4.method = EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_DHCP;
             else if (strcmp(str, "manual") == 0)
               pd->configured.ipv4.method = EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_MANUAL;
             else if (strcmp(str, "off") == 0)
               pd->configured.ipv4.method = EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_OFF;
             else
               WRN("Unexpected IPV4 Method value '%s'", str);
          }
        else if (strcmp(name, "Address") == 0)
          eina_stringshare_replace(&pd->configured.ipv4.address, str);
        else if (strcmp(name, "Netmask") == 0)
          eina_stringshare_replace(&pd->configured.ipv4.netmask, str);
        else if (strcmp(name, "Gateway") == 0)
          eina_stringshare_replace(&pd->configured.ipv4.gateway, str);
        else
          WRN("Unknown property name: %s", name);
     }
}

static void
_efl_net_control_access_point_property_configured_ipv6_changed(Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Eldbus_Message_Iter *value)
{
   Eldbus_Message_Iter *array, *entry;

   if (!eldbus_message_iter_arguments_get(value, "a{sv}", &array))
     {
        ERR("Expected dict for, got %s", eldbus_message_iter_signature_get(value));
        return;
     }

   pd->configured.ipv6.method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_UNSET;
   eina_stringshare_replace(&pd->configured.ipv6.address, NULL);
   eina_stringshare_replace(&pd->configured.ipv6.netmask, NULL);
   eina_stringshare_replace(&pd->configured.ipv6.gateway, NULL);
   pd->configured.ipv6.prefix_length = 0;

   while (eldbus_message_iter_get_and_next(array, 'e', &entry))
     {
        const char *name, *str;
        Eldbus_Message_Iter *var;

        if (!eldbus_message_iter_arguments_get(entry, "sv", &name, &var))
          {
             ERR("Unexpected dict entry signature: %s", eldbus_message_iter_signature_get(entry));
             continue;
          }

        if (strcmp(name, "PrefixLength") == 0)
          {
             if (!eldbus_message_iter_arguments_get(var, "y", &pd->configured.ipv6.prefix_length))
               ERR("Expected unsigned byte value for %s, got %s", name, eldbus_message_iter_signature_get(var));
             continue;
          }

        if (!eldbus_message_iter_arguments_get(var, "s", &str))
          {
             ERR("Expected string value for %s, got %s", name, eldbus_message_iter_signature_get(var));
             continue;
          }

        if (strcmp(name, "Method") == 0)
          {
             if (strcmp(str, "off") == 0)
               pd->configured.ipv6.method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_OFF;
             else if (strcmp(str, "fixed") == 0)
               pd->configured.ipv6.method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_FIXED;
             else if (strcmp(str, "manual") == 0)
               pd->configured.ipv6.method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_MANUAL;
             else if (strcmp(str, "6to4") == 0)
               pd->configured.ipv6.method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_TUNNEL6TO4;
             else if (strcmp(str, "auto") == 0)
               pd->configured.ipv6.method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_AUTO_PRIVACY_NONE;
             else
               WRN("Unexpected IPV6 Method value '%s'", str);
          }
        else if (strcmp(name, "Privacy") == 0)
          {
             if ((pd->configured.ipv6.method != EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_AUTO_PRIVACY_NONE) &&
                 (pd->configured.ipv6.method != EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_UNSET))
               {
                  DBG("Skip privacy %s, method already set to %d", str, pd->configured.ipv6.method);
                  continue;
               }
             if (strcmp(str, "disabled") == 0)
               pd->configured.ipv6.method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_AUTO_PRIVACY_NONE;
             else if (strcmp(str, "enabled") == 0)
               pd->configured.ipv6.method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_AUTO_PRIVACY_PUBLIC;
             else if (strcmp(str, "preferred") == 0)
               pd->configured.ipv6.method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_AUTO_PRIVACY_TEMPORARY;
             else
               WRN("Unexpected IPV6 Privacy value '%s'", str);
          }
        else if (strcmp(name, "Address") == 0)
          eina_stringshare_replace(&pd->configured.ipv6.address, str);
        else if (strcmp(name, "Netmask") == 0)
          eina_stringshare_replace(&pd->configured.ipv6.netmask, str);
        else if (strcmp(name, "Gateway") == 0)
          eina_stringshare_replace(&pd->configured.ipv6.gateway, str);
        else
          WRN("Unknown property name: %s", name);
     }
}

static void
_efl_net_control_access_point_property_configured_proxy_changed(Eo *o EINA_UNUSED, Efl_Net_Control_Access_Point_Data *pd, Eldbus_Message_Iter *value)
{
   Eldbus_Message_Iter *array, *entry;
   const char *str;

   if (!eldbus_message_iter_arguments_get(value, "a{sv}", &array))
     {
        ERR("Expected dict for, got %s", eldbus_message_iter_signature_get(value));
        return;
     }

   pd->configured.proxy.method = EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_UNSET;
   eina_stringshare_replace(&pd->configured.proxy.url, NULL);
   EINA_LIST_FREE(pd->configured.proxy.servers, str) eina_stringshare_del(str);
   EINA_LIST_FREE(pd->configured.proxy.excludes, str) eina_stringshare_del(str);

   while (eldbus_message_iter_get_and_next(array, 'e', &entry))
     {
        const char *name;
        Eldbus_Message_Iter *var;

        if (!eldbus_message_iter_arguments_get(entry, "sv", &name, &var))
          {
             ERR("Unexpected dict entry signature: %s", eldbus_message_iter_signature_get(entry));
             continue;
          }

        if (strcmp(name, "Servers") == 0)
          {
             _efl_net_control_access_point_list_updated("Proxy Servers", &pd->configured.proxy.servers, var);
             continue;
          }
        else if (strcmp(name, "Excludes") == 0)
          {
             _efl_net_control_access_point_list_updated("Proxy Excludes", &pd->configured.proxy.excludes, var);
             continue;
          }

        if (!eldbus_message_iter_arguments_get(var, "s", &str))
          {
             ERR("Expected string value for %s, got %s", name, eldbus_message_iter_signature_get(var));
             continue;
          }

        if (strcmp(name, "Method") == 0)
          {
             if (strcmp(str, "auto") == 0)
               pd->configured.proxy.method = EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_AUTO;
             else if (strcmp(str, "manual") == 0)
               pd->configured.proxy.method = EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_MANUAL;
             else if (strcmp(str, "direct") == 0)
               pd->configured.proxy.method = EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_OFF;
             else
               WRN("Unexpected PROXY Method value '%s'", str);
          }
        else if (strcmp(name, "URL") == 0)
          eina_stringshare_replace(&pd->configured.proxy.url, str);
        else
          WRN("Unknown property name: %s", name);
     }
}


static void
_efl_net_control_access_point_property_changed_internal(Eo *o, Efl_Net_Control_Access_Point_Data *pd, Eldbus_Message_Iter *itr)
{
   Eldbus_Message_Iter *value;
   const char *name;

   if (!eldbus_message_iter_arguments_get(itr, "sv", &name, &value))
     {
        ERR("Unexpected signature: %s", eldbus_message_iter_signature_get(itr));
        return;
     }

   if (strcmp(name, "Name") == 0)
     _efl_net_control_access_point_property_name_changed(o, pd, value);
   else if (strcmp(name, "State") == 0)
     _efl_net_control_access_point_property_state_changed(o, pd, value);
   else if (strcmp(name, "Error") == 0)
     _efl_net_control_access_point_property_error_changed(o, pd, value);
   else if (strcmp(name, "Type") == 0)
     _efl_net_control_access_point_property_type_changed(o, pd, value);
   else if (strcmp(name, "Security") == 0)
     _efl_net_control_access_point_property_security_changed(o, pd, value);
   else if (strcmp(name, "Strength") == 0)
     _efl_net_control_access_point_property_strength_changed(o, pd, value);
   else if (strcmp(name, "Favorite") == 0)
     _efl_net_control_access_point_property_remembered_changed(o, pd, value);
   else if (strcmp(name, "Immutable") == 0)
     _efl_net_control_access_point_property_immutable_changed(o, pd, value);
   else if (strcmp(name, "AutoConnect") == 0)
     _efl_net_control_access_point_property_auto_connect_changed(o, pd, value);
   else if (strcmp(name, "Roaming") == 0)
     _efl_net_control_access_point_property_roaming_changed(o, pd, value);
   else if (strcmp(name, "Nameservers") == 0)
     _efl_net_control_access_point_property_actual_name_servers_changed(o, pd, value);
   else if (strcmp(name, "Timeservers") == 0)
     _efl_net_control_access_point_property_actual_time_servers_changed(o, pd, value);
   else if (strcmp(name, "Domains") == 0)
     _efl_net_control_access_point_property_actual_domains_changed(o, pd, value);
   else if (strcmp(name, "IPv4") == 0)
     _efl_net_control_access_point_property_actual_ipv4_changed(o, pd, value);
   else if (strcmp(name, "IPv6") == 0)
     _efl_net_control_access_point_property_actual_ipv6_changed(o, pd, value);
   else if (strcmp(name, "Proxy") == 0)
     _efl_net_control_access_point_property_actual_proxy_changed(o, pd, value);
   else if (strcmp(name, "Nameservers.Configuration") == 0)
     _efl_net_control_access_point_property_configured_name_servers_changed(o, pd, value);
   else if (strcmp(name, "Timeservers.Configuration") == 0)
     _efl_net_control_access_point_property_configured_time_servers_changed(o, pd, value);
   else if (strcmp(name, "Domains.Configuration") == 0)
     _efl_net_control_access_point_property_configured_domains_changed(o, pd, value);
   else if (strcmp(name, "IPv4.Configuration") == 0)
     _efl_net_control_access_point_property_configured_ipv4_changed(o, pd, value);
   else if (strcmp(name, "IPv6.Configuration") == 0)
     _efl_net_control_access_point_property_configured_ipv6_changed(o, pd, value);
   else if (strcmp(name, "Proxy.Configuration") == 0)
     _efl_net_control_access_point_property_configured_proxy_changed(o, pd, value);
   else if ((strcmp(name, "Provider") == 0) ||
            (strcmp(name, "Ethernet") == 0))
     DBG("Ignored property name: %s", name);
   else
     WRN("Unknown property name: %s", name);
}

static void
_efl_net_control_access_point_property_changed(void *data, const Eldbus_Message *msg)
{
   Eo *o = data;
   Efl_Net_Control_Access_Point_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eldbus_Message_Iter *itr;

   itr = eldbus_message_iter_get(msg);
   _efl_net_control_access_point_property_changed_internal(o, pd, itr);
   efl_event_callback_call(o, EFL_NET_CONTROL_ACCESS_POINT_EVENT_CHANGED, NULL);
}

Efl_Net_Control_Access_Point *
efl_net_connman_access_point_new(Efl_Net_Control_Manager *ctl, const char *path, Eldbus_Message_Iter *properties, unsigned int priority)
{
   Eo *o;
   Efl_Net_Control_Access_Point_Data *pd;
   Eldbus_Connection *conn;
   Eldbus_Object *obj;

   conn = efl_net_connman_connection_get();
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);

   o = efl_add(MY_CLASS, ctl);
   EINA_SAFETY_ON_NULL_RETURN_VAL(o, NULL);

   pd = efl_data_scope_get(o, MY_CLASS);
   EINA_SAFETY_ON_NULL_GOTO(pd, error);

   pd->path = eina_stringshare_add(path);
   EINA_SAFETY_ON_NULL_GOTO(pd->path, error);

   obj = eldbus_object_get(conn, "net.connman", pd->path);
   EINA_SAFETY_ON_NULL_GOTO(obj, error);
   pd->proxy = eldbus_proxy_get(obj, "net.connman.Service");
   EINA_SAFETY_ON_NULL_GOTO(pd->proxy, error);

   pd->configured.ipv4.method = EFL_NET_CONTROL_ACCESS_POINT_IPV4_METHOD_UNSET;
   pd->configured.ipv6.method = EFL_NET_CONTROL_ACCESS_POINT_IPV6_METHOD_UNSET;
   pd->configured.proxy.method = EFL_NET_CONTROL_ACCESS_POINT_PROXY_METHOD_UNSET;

#define SH(sig, cb) \
   do { \
     Eldbus_Signal_Handler *sh = eldbus_proxy_signal_handler_add(pd->proxy, sig, cb, o); \
     if (sh) pd->signal_handlers = eina_list_append(pd->signal_handlers, sh); \
     else ERR("could not add DBus signal handler %s", sig); \
   } while (0)

   SH("PropertyChanged", _efl_net_control_access_point_property_changed);
#undef SH

   efl_event_freeze(o);
   efl_net_connman_access_point_update(o, properties, priority);
   efl_event_thaw(o);

   return o;

 error:
   efl_del(o);
   return NULL;
}

const char *
efl_net_connman_access_point_path_get(Efl_Net_Control_Access_Point *o)
{
   Efl_Net_Control_Access_Point_Data *pd = efl_data_scope_get(o, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, NULL);
   return pd->path;
}

void
efl_net_connman_access_point_update(Efl_Net_Control_Manager *o, Eldbus_Message_Iter *properties, unsigned int priority)
{
   Eldbus_Message_Iter *entry;
   Efl_Net_Control_Access_Point_Data *pd = efl_data_scope_get(o, MY_CLASS);

   EINA_SAFETY_ON_NULL_RETURN(pd);

   pd->priority = priority;

   while (eldbus_message_iter_get_and_next(properties, 'e', &entry))
     _efl_net_control_access_point_property_changed_internal(o, pd, entry);

   efl_event_callback_call(o, EFL_NET_CONTROL_ACCESS_POINT_EVENT_CHANGED, NULL);
}

#include "efl_net_control_access_point.eo.c"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"
#include "efl_net-connman.h"

typedef struct
{
   Eldbus_Proxy *proxy; /* net.connman.Session */
   Eldbus_Service_Interface *notifier; /* net.connman.Notification */
   Eldbus_Pending *mgr_pending; /* on efl_net_connman_manager_get(), local proxy doesn't need it, done automatically */

   struct {
      Eldbus_Pending *pending;
      Eina_Bool connected; /* if should be connected or not */
      Eina_Bool online_required;
      Efl_Net_Session_Technology technologies_allowed;
   } connect;

   /* properties notified by session, local cache */
   Eina_Stringshare *name;
   Eina_Stringshare *interface;
   struct {
      Eina_Stringshare *address;
      Eina_Stringshare *netmask;
      Eina_Stringshare *gateway;
   } ipv4;
   struct {
      Eina_Stringshare *address;
      Eina_Stringshare *netmask;
      Eina_Stringshare *gateway;
      uint8_t prefix_length;
   } ipv6;
   Efl_Net_Session_State state;
   Efl_Net_Session_Technology technology;
} Efl_Net_Session_Data;

#define MY_CLASS EFL_NET_SESSION_CLASS

/* will SET BIT for technology, start with '0' for multiple techs */
static Eina_Bool
_efl_net_session_technology_from_str(const char *str, Efl_Net_Session_Technology *tech)
{
   if (0) { }
#define MAP(X, s) \
   else if (strcmp(str, s) == 0) *tech |= EFL_NET_SESSION_TECHNOLOGY_ ## X
   MAP(ALL, "*");
   MAP(ETHERNET, "ethernet");
   MAP(WIFI, "wifi");
   MAP(BLUETOOTH, "bluetooth");
   MAP(CELLULAR, "cellular");
   MAP(VPN, "vpn");
   MAP(GADGET, "gadget");
#undef MAP
   else if (str[0]) /* empty bearer = no technology */
     {
        WRN("Unknown technology name: %s", str);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static void _efl_net_session_connect_do(Eo *o, Efl_Net_Session_Data *pd);

/* NOTE: unlike most DBus servers where you create paths using
 * ObjectManager and monitor properties on them using PropertyManager,
 * ConnMan doesn't use any of those and their API for Session is
 * different from all others in ConnMan itself:
 *
 * 1 - create a local service 'notifier' implementing
 *     net.connman.Notification, with method Release() and
 *     Update(dict settings).
 *
 * 2 - call / CreateSession(dict settings, path notifier), get an
 *     object path representing your session
 *
 * 3 - call Change(setting, value), Connect(), Disconnect() on
 *     object path returned on #2
 *
 * 4 - get Update() to be called on your local service notifier specified
 *     on step #1.
 */

static Eldbus_Message *
_efl_net_session_notifier_release(const Eldbus_Service_Interface *service, const Eldbus_Message *msg)
{
   Eo *o = eldbus_service_object_data_get(service, "efl_net");
   DBG("Session %p is released %s", o, eldbus_message_path_get(msg));

   return eldbus_message_method_return_new(msg);
}

static Eina_Error
_efl_net_session_notifier_update_state(Efl_Net_Session_Data *pd, Eldbus_Message_Iter *var)
{
   const char *str;

   if (!eldbus_message_iter_arguments_get(var, "s", &str))
     return EINVAL;

   if (strcmp(str, "disconnected") == 0)
     pd->state = EFL_NET_SESSION_STATE_OFFLINE;
   else if (strcmp(str, "connected") == 0)
     pd->state = EFL_NET_SESSION_STATE_LOCAL;
   else if (strcmp(str, "online") == 0)
     pd->state = EFL_NET_SESSION_STATE_ONLINE;
   else
     return EINVAL;

   return 0;
}

static Eina_Error
_efl_net_session_notifier_update_name(Efl_Net_Session_Data *pd, Eldbus_Message_Iter *var)
{
   const char *str;

   if (!eldbus_message_iter_arguments_get(var, "s", &str))
     return EINVAL;

   eina_stringshare_replace(&pd->name, str);
   return 0;
}

static Eina_Error
_efl_net_session_notifier_update_technology(Efl_Net_Session_Data *pd, Eldbus_Message_Iter *var)
{
   const char *str;

   if (!eldbus_message_iter_arguments_get(var, "s", &str))
     return EINVAL;

   pd->technology = 0;
   if (!_efl_net_session_technology_from_str(str, &pd->technology))
     return EINVAL;

   return 0;
}

static Eina_Error
_efl_net_session_notifier_update_interface(Efl_Net_Session_Data *pd, Eldbus_Message_Iter *var)
{
   const char *str;

   if (!eldbus_message_iter_arguments_get(var, "s", &str))
     return EINVAL;

   eina_stringshare_replace(&pd->interface, str);
   return 0;
}

static Eina_Error
_efl_net_session_notifier_update_ipv4(Efl_Net_Session_Data *pd, Eldbus_Message_Iter *var)
{
   Eldbus_Message_Iter *sub, *entry;

   if (!eldbus_message_iter_arguments_get(var, "a{sv}", &sub))
     return EINVAL;

   while (eldbus_message_iter_get_and_next(sub, 'e', &entry))
     {
        const char *key;
        Eldbus_Message_Iter *value;

        if (!eldbus_message_iter_arguments_get(entry, "sv", &key, &value))
          {
             ERR("Unexpected dict entry signature: %s", eldbus_message_iter_signature_get(entry));
             continue;
          }

        if (strcmp(key, "Method") == 0)
          {
             const char *str;
             if (!eldbus_message_iter_arguments_get(value, "s", &str))
               ERR("expected string, property=%s", key);
             else
               DBG("configuration method %s", str);
          }
        else if (strcmp(key, "Address") == 0)
          {
             const char *str;
             if (!eldbus_message_iter_arguments_get(value, "s", &str))
               ERR("expected string, property=%s", key);
             else
               {
                  DBG("address %s", str);
                  eina_stringshare_replace(&pd->ipv4.address, str);
               }
          }
        else if (strcmp(key, "Gateway") == 0)
          {
             const char *str;
             if (!eldbus_message_iter_arguments_get(value, "s", &str))
               ERR("expected string, property=%s", key);
             else
               {
                  DBG("gateway %s", str);
                  eina_stringshare_replace(&pd->ipv4.gateway, str);
               }
          }
        else if (strcmp(key, "Netmask") == 0)
          {
             const char *str;
             if (!eldbus_message_iter_arguments_get(value, "s", &str))
               ERR("expected string, property=%s", key);
             else
               {
                  DBG("netmask %s", str);
                  eina_stringshare_replace(&pd->ipv4.netmask, str);
               }
          }
        else
          {
             WRN("Unsupported field %s (signature=%s)", key, eldbus_message_iter_signature_get(value));
             continue;
          }
     }

   return 0;
}

static Eina_Error
_efl_net_session_notifier_update_ipv6(Efl_Net_Session_Data *pd, Eldbus_Message_Iter *var)
{
   Eldbus_Message_Iter *sub, *entry;

   if (!eldbus_message_iter_arguments_get(var, "a{sv}", &sub))
     return EINVAL;

   while (eldbus_message_iter_get_and_next(sub, 'e', &entry))
     {
        const char *key;
        Eldbus_Message_Iter *value;

        if (!eldbus_message_iter_arguments_get(entry, "sv", &key, &value))
          {
             ERR("Unexpected dict entry signature: %s", eldbus_message_iter_signature_get(entry));
             continue;
          }

        if (strcmp(key, "Method") == 0)
          {
             const char *str;
             if (!eldbus_message_iter_arguments_get(value, "s", &str))
               ERR("expected string, property=%s", key);
             else
               DBG("configuration method %s", str);
          }
        else if (strcmp(key, "Address") == 0)
          {
             const char *str;
             if (!eldbus_message_iter_arguments_get(value, "s", &str))
               ERR("expected string, property=%s", key);
             else
               {
                  DBG("address %s", str);
                  eina_stringshare_replace(&pd->ipv6.address, str);
               }
          }
        else if (strcmp(key, "Gateway") == 0)
          {
             const char *str;
             if (!eldbus_message_iter_arguments_get(value, "s", &str))
               ERR("expected string, property=%s", key);
             else
               {
                  DBG("gateway %s", str);
                  eina_stringshare_replace(&pd->ipv6.gateway, str);
               }
          }
        else if (strcmp(key, "Netmask") == 0)
          {
             const char *str;
             if (!eldbus_message_iter_arguments_get(value, "s", &str))
               ERR("expected string, property=%s", key);
             else
               {
                  DBG("netmask %s", str);
                  eina_stringshare_replace(&pd->ipv6.netmask, str);
               }
          }
        else if (strcmp(key, "PrefixLength") == 0)
          {
             if (!eldbus_message_iter_arguments_get(value, "y", &pd->ipv6.prefix_length))
               ERR("expected unsigned byte, property=%s", key);
             else
               DBG("prefix_length %hhu", pd->ipv6.prefix_length);
          }
        else if (strcmp(key, "Privacy") == 0)
          {
             const char *str;
             if (!eldbus_message_iter_arguments_get(value, "s", &str))
               ERR("expected string, property=%s", key);
             else
               DBG("privacy %s (unused)", str);
          }
        else
          {
             WRN("Unsupported field %s (signature=%s)", key, eldbus_message_iter_signature_get(value));
             continue;
          }
     }

   return 0;
}

static Eina_Error
_efl_net_session_notifier_update_bearers(Efl_Net_Session_Data *pd EINA_UNUSED, Eldbus_Message_Iter *var)
{
   Eldbus_Message_Iter *sub;
   const char *str;

   if (!eldbus_message_iter_arguments_get(var, "as", &sub))
     {
        ERR("Expected array of strings, got %s", eldbus_message_iter_signature_get(var));
        return EINVAL;
     }
   while (eldbus_message_iter_get_and_next(sub, 's', &str))
     DBG("allowed bearer '%s'", str);

   return 0;
}

static Eina_Error
_efl_net_session_notifier_update_connection_type(Efl_Net_Session_Data *pd EINA_UNUSED, Eldbus_Message_Iter *var)
{
   const char *str;
   if (!eldbus_message_iter_arguments_get(var, "s", &str))
     return EINVAL;

   DBG("connection type '%s'", str);
   return 0;
}

/* step #4: get the initial state and changed applied locally */
static Eldbus_Message *
_efl_net_session_notifier_update(const Eldbus_Service_Interface *service, const Eldbus_Message *msg)
{
   Eo *o = eldbus_service_object_data_get(service, "efl_net");
   Efl_Net_Session_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eldbus_Message_Iter *array, *entry;
   Eina_Bool updated = EINA_FALSE;

   DBG("Session %p is updated %s", o, eldbus_message_path_get(msg));

   EINA_SAFETY_ON_NULL_GOTO(o, end);

   if (!eldbus_message_arguments_get(msg, "a{sv}", &array))
     {
        ERR("Unexpected net.connman.Notifier.Update() signature %s", eldbus_message_signature_get(msg));
        goto end;
     }

   while (eldbus_message_iter_get_and_next(array, 'e', &entry))
     {
        const char *key;
        Eldbus_Message_Iter *var;
        Eina_Error err;

        if (!eldbus_message_iter_arguments_get(entry, "sv", &key, &var))
          {
             ERR("Unexpected dict entry signature: %s", eldbus_message_iter_signature_get(entry));
             continue;
          }

        if (strcmp(key, "State") == 0)
          err = _efl_net_session_notifier_update_state(pd, var);
        else if (strcmp(key, "Name") == 0)
          err = _efl_net_session_notifier_update_name(pd, var);
        else if (strcmp(key, "Bearer") == 0)
          err = _efl_net_session_notifier_update_technology(pd, var);
        else if (strcmp(key, "Interface") == 0)
          err = _efl_net_session_notifier_update_interface(pd, var);
        else if (strcmp(key, "IPv4") == 0)
          err = _efl_net_session_notifier_update_ipv4(pd, var);
        else if (strcmp(key, "IPv6") == 0)
          err = _efl_net_session_notifier_update_ipv6(pd, var);
        else if (strcmp(key, "AllowedBearers") == 0)
          err = _efl_net_session_notifier_update_bearers(pd, var);
        else if (strcmp(key, "ConnectionType") == 0)
          err = _efl_net_session_notifier_update_connection_type(pd, var);
        else
          {
             WRN("Unsupported setting %s (signature=%s)", key, eldbus_message_iter_signature_get(var));
             continue;
          }

        if (err)
          {
             ERR("Could not handle property %s: %s", key, eina_error_msg_get(err));
             continue;
          }

        updated |= EINA_TRUE;
     }

   if (updated) efl_event_callback_call(o, EFL_NET_SESSION_EVENT_CHANGED, NULL);

 end:
   return eldbus_message_method_return_new(msg);
}

static const Eldbus_Service_Interface_Desc _efl_net_session_notifier_desc = {
  .interface = "net.connman.Notification",
  .methods = (const Eldbus_Method []){
     {
       .member = "Release",
       .cb = _efl_net_session_notifier_release,
     },
     {
       .member = "Update",
       .in = ELDBUS_ARGS({"a{sv}", "settings"}),
       .cb = _efl_net_session_notifier_update,
     },
     { }
   },
};

/* return of step #2: session was created, get a proxy for it */
static void
_efl_net_session_create_session_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   Eo *o = data;
   Efl_Net_Session_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eldbus_Connection *conn;
   Eldbus_Object *obj;
   const char *err_name, *err_msg, *path;

   if (pd->mgr_pending == pending)
     pd->mgr_pending = NULL;

   if (eldbus_message_error_get(msg, &err_name, &err_msg))
     {
        ERR("Could not create session %p: %s=%s", o, err_name, err_msg);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "o", &path))
     {
        ERR("Could not get session %p DBus path!", o);
        return;
     }

   conn = efl_net_connman_connection_get();
   obj = eldbus_object_get(conn, "net.connman", path);
   pd->proxy = eldbus_proxy_get(obj, "net.connman.Session");
   if (!pd->proxy)
     {
        ERR("could not create DBus proxy for interface='net.connman.Session', name='net.connman', path='%s' o=%p", path, o);
        eldbus_object_unref(obj);
        return;
     }

   DBG("Created session %p (session=%s) with ConnMan...", o, path);

   if (pd->connect.connected)
     {
        DBG("apply pending connect request: online_required=%d, technologies_allowed=%#x", pd->connect.online_required, pd->connect.technologies_allowed);
        _efl_net_session_connect_do(o, pd);
     }
}

static void
_efl_net_session_clear(Efl_Net_Session_Data *pd)
{
   eina_stringshare_replace(&pd->name, NULL);
   eina_stringshare_replace(&pd->interface, NULL);
   eina_stringshare_replace(&pd->ipv4.address, NULL);
   eina_stringshare_replace(&pd->ipv4.netmask, NULL);
   eina_stringshare_replace(&pd->ipv4.gateway, NULL);
   eina_stringshare_replace(&pd->ipv6.address, NULL);
   eina_stringshare_replace(&pd->ipv6.netmask, NULL);
   eina_stringshare_replace(&pd->ipv6.gateway, NULL);
   pd->ipv6.prefix_length = 0;
   pd->state = EFL_NET_SESSION_STATE_OFFLINE;
   pd->technology = EFL_NET_SESSION_TECHNOLOGY_UNKNOWN;
}

/* step #2: once connman is there, call CreateSession */
static void
_efl_net_session_connman_name_owner_changed(void *data, const char *bus, const char *old_id, const char *new_id)
{
   Eo *o = data;
   Efl_Net_Session_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eldbus_Message_Iter *msg_itr, *cont;
   Eldbus_Proxy *mgr;
   Eldbus_Message *msg;
   const char *path;

   DBG("Name Owner Changed %s: %s->%s", bus, old_id, new_id);
   if ((!new_id) || (new_id[0] == '\0'))
     {
        /* connman is gone, remove proxy as it became useless */
        if (pd->proxy)
          {
             Eldbus_Object *obj = eldbus_proxy_object_get(pd->proxy);
             eldbus_proxy_unref(pd->proxy);
             pd->proxy = NULL;
             eldbus_object_unref(obj);
          }
        _efl_net_session_clear(pd);
        efl_event_callback_call(o, EFL_NET_SESSION_EVENT_CHANGED, NULL);
        return;
     }

   path = eldbus_service_object_path_get(pd->notifier);
   EINA_SAFETY_ON_NULL_RETURN(path);

   INF("Create session %p notifier=%s with %s (%s)", o, path, bus, new_id);

   mgr = efl_net_connman_manager_get();
   EINA_SAFETY_ON_NULL_RETURN(mgr);

   msg = eldbus_proxy_method_call_new(mgr, "CreateSession");
   EINA_SAFETY_ON_NULL_RETURN(msg);

   msg_itr = eldbus_message_iter_get(msg);
   EINA_SAFETY_ON_NULL_GOTO(msg_itr, error_send);

   cont = eldbus_message_iter_container_new(msg_itr, 'a', "{sv}");
   eldbus_message_iter_container_close(msg_itr, cont); /* empty, use defaults */

   eldbus_message_iter_basic_append(msg_itr, 'o', path);

   if (pd->mgr_pending)
     eldbus_pending_cancel(pd->mgr_pending);

   pd->mgr_pending = eldbus_proxy_send(mgr, msg, _efl_net_session_create_session_cb, o, DEFAULT_TIMEOUT);
   EINA_SAFETY_ON_NULL_GOTO(pd->mgr_pending, error_send);
   return;

 error_send:
   eldbus_message_unref(msg);
}

EOLIAN static Eo *
_efl_net_session_efl_object_constructor(Eo *o, Efl_Net_Session_Data *pd EINA_UNUSED)
{
   if (!efl_net_connman_init())
     {
        ERR("could not initialize connman infrastructure");
        return NULL;
     }

   return efl_constructor(efl_super(o, MY_CLASS));
}

EOLIAN static Eo *
_efl_net_session_efl_object_finalize(Eo *o, Efl_Net_Session_Data *pd)
{
   Eldbus_Connection *conn;
   char path[128];

   o = efl_finalize(efl_super(o, MY_CLASS));
   if (!o) return NULL;

   conn = efl_net_connman_connection_get();
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);

   /* step #1: create local notifier path */
   snprintf(path, sizeof(path), "/connman/notifier_%p", o);
   pd->notifier = eldbus_service_interface_register(conn, path, &_efl_net_session_notifier_desc);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->notifier, NULL);
   eldbus_service_object_data_set(pd->notifier, "efl_net", o);

   eldbus_name_owner_changed_callback_add(conn, "net.connman", _efl_net_session_connman_name_owner_changed, o, EINA_TRUE);
   DBG("waiting for net.connman to show on the DBus system bus...");

   return o;
}

EOLIAN static void
_efl_net_session_efl_object_destructor(Eo *o, Efl_Net_Session_Data *pd)
{
   Eldbus_Connection *conn;

   conn = efl_net_connman_connection_get();
   eldbus_name_owner_changed_callback_del(conn, "net.connman", _efl_net_session_connman_name_owner_changed, o);

   if (pd->mgr_pending)
     {
        eldbus_pending_cancel(pd->mgr_pending);
        pd->mgr_pending = NULL;
     }

   if (pd->notifier)
     {
        eldbus_service_object_data_del(pd->notifier, "efl_net");
        eldbus_service_object_unregister(pd->notifier);
        pd->notifier = NULL;
     }

   if (pd->proxy)
     {
        Eldbus_Object *obj = eldbus_proxy_object_get(pd->proxy);
        Eldbus_Proxy *mgr = efl_net_connman_manager_get();

        /* DestroySession is required since the manager proxy and bus
         * may be alive, thus connman won't get any NameOwnerChanged
         * or related to know the object is gone
         */
        eldbus_proxy_call(mgr, "DestroySession", NULL, NULL, DEFAULT_TIMEOUT,
                          "o", eldbus_object_path_get(obj));

        eldbus_proxy_unref(pd->proxy);
        eldbus_object_unref(obj);
        pd->proxy = NULL;
     }

   _efl_net_session_clear(pd);

   efl_destructor(efl_super(o, MY_CLASS));
   efl_net_connman_shutdown();
}

EOLIAN static const char *
_efl_net_session_network_name_get(const Eo *o EINA_UNUSED, Efl_Net_Session_Data *pd)
{
   return pd->name;
}

EOLIAN static Efl_Net_Session_State
_efl_net_session_state_get(const Eo *o EINA_UNUSED, Efl_Net_Session_Data *pd)
{
   return pd->state;
}

EOLIAN static Efl_Net_Session_Technology
_efl_net_session_technology_get(const Eo *o EINA_UNUSED, Efl_Net_Session_Data *pd)
{
   return pd->technology;
}

EOLIAN static const char *
_efl_net_session_interface_get(const Eo *o EINA_UNUSED, Efl_Net_Session_Data *pd)
{
   return pd->interface;
}

EOLIAN static void
_efl_net_session_ipv4_get(const Eo *o EINA_UNUSED, Efl_Net_Session_Data *pd, const char **address, const char **netmask, const char **gateway)
{
   if (address) *address = pd->ipv4.address;
   if (netmask) *netmask = pd->ipv4.netmask;
   if (gateway) *gateway = pd->ipv4.gateway;
}

EOLIAN static void
_efl_net_session_ipv6_get(const Eo *o EINA_UNUSED, Efl_Net_Session_Data *pd, const char **address, uint8_t *prefix_length, const char **netmask, const char **gateway)
{
   if (address) *address = pd->ipv6.address;
   if (netmask) *netmask = pd->ipv6.netmask;
   if (gateway) *gateway = pd->ipv6.gateway;
   if (prefix_length) *prefix_length = pd->ipv6.prefix_length;
}

static void
_efl_net_session_connect_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   Eo *o = data;
   Efl_Net_Session_Data *pd = efl_data_scope_get(o, MY_CLASS);
   const char *err_name, *err_msg;

   if (pd->connect.pending == pending)
     pd->connect.pending = NULL;

   if (eldbus_message_error_get(msg, &err_name, &err_msg))
     {
        WRN("Could not Connect: %s=%s", err_name, err_msg);
        return;
     }
   DBG("Successfully requested a connection online_required=%hhu, technologies_allowed=%#x", pd->connect.online_required, pd->connect.technologies_allowed);
}

static void
_efl_net_session_connect_do_connect(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   Eo *o = data;
   Efl_Net_Session_Data *pd = efl_data_scope_get(o, MY_CLASS);
   const char *err_name, *err_msg;

   if (pd->connect.pending == pending)
     pd->connect.pending = NULL;

   if (eldbus_message_error_get(msg, &err_name, &err_msg))
     {
        WRN("Could not Change('ConnectionType'): %s=%s", err_name, err_msg);
        return;
     }

   pd->connect.pending = eldbus_proxy_call(pd->proxy, "Connect",
                                           _efl_net_session_connect_cb, o,
                                           DEFAULT_TIMEOUT, "");
   EINA_SAFETY_ON_NULL_RETURN(pd->connect.pending);
}

static void
_efl_net_session_connect_change_online_required(void *data, const Eldbus_Message *msg_ret, Eldbus_Pending *pending)
{
   Eo *o = data;
   Efl_Net_Session_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eldbus_Message *msg;
   Eldbus_Message_Iter *msg_itr, *itr;
   const char *err_name, *err_msg;
   Eina_Bool ret;

   if (pd->connect.pending == pending)
     pd->connect.pending = NULL;

   if (eldbus_message_error_get(msg_ret, &err_name, &err_msg))
     {
        WRN("Could not Change('AllowedBearers'): %s=%s", err_name, err_msg);
        return;
     }

   msg = eldbus_proxy_method_call_new(pd->proxy, "Change");
   EINA_SAFETY_ON_NULL_RETURN(msg);

   msg_itr = eldbus_message_iter_get(msg);
   EINA_SAFETY_ON_NULL_GOTO(msg_itr, error_msg);

   eldbus_message_iter_basic_append(msg_itr, 's', "ConnectionType");
   itr = eldbus_message_iter_container_new(msg_itr, 'v', "s");
   EINA_SAFETY_ON_NULL_GOTO(itr, error_msg);

   ret = eldbus_message_iter_basic_append(itr, 's', pd->connect.online_required ? "internet" : "local");
   EINA_SAFETY_ON_FALSE_GOTO(ret, error_msg);

   eldbus_message_iter_container_close(msg_itr, itr);

   pd->connect.pending = eldbus_proxy_send(pd->proxy, msg,
                                           _efl_net_session_connect_do_connect, o,
                                           DEFAULT_TIMEOUT);
   EINA_SAFETY_ON_NULL_RETURN(pd->connect.pending);
   return;

 error_msg:
   eldbus_message_unref(msg);
}

static void
_efl_net_session_connect_do(Eo *o, Efl_Net_Session_Data *pd)
{
   Eldbus_Message_Iter *msg_itr, *itr, *array;
   Eldbus_Message *msg;

   if (!pd->connect.connected) return;

   if (pd->connect.pending)
     {
        eldbus_pending_cancel(pd->connect.pending);
        pd->connect.pending = NULL;
     }

   msg = eldbus_proxy_method_call_new(pd->proxy, "Change");
   EINA_SAFETY_ON_NULL_RETURN(msg);

   msg_itr = eldbus_message_iter_get(msg);
   EINA_SAFETY_ON_NULL_GOTO(msg_itr, error_msg);

   eldbus_message_iter_basic_append(msg_itr, 's', "AllowedBearers");
   itr = eldbus_message_iter_container_new(msg_itr, 'v', "as");
   EINA_SAFETY_ON_NULL_GOTO(itr, error_msg);

   array = eldbus_message_iter_container_new(itr, 'a', "s");

#define MAP(X, s) \
   if ((pd->connect.technologies_allowed & EFL_NET_SESSION_TECHNOLOGY_ ## X) == EFL_NET_SESSION_TECHNOLOGY_ ## X) \
        eldbus_message_iter_basic_append(array, 's', s)
   MAP(ALL, "*");
   if (pd->connect.technologies_allowed == EFL_NET_SESSION_TECHNOLOGY_ALL) goto end;
   MAP(ETHERNET, "ethernet");
   MAP(WIFI, "wifi");
   MAP(BLUETOOTH, "bluetooth");
   MAP(CELLULAR, "cellular");
   MAP(VPN, "vpn");
   MAP(GADGET, "gadget");
#undef MAP

 end:
   eldbus_message_iter_container_close(itr, array);
   eldbus_message_iter_container_close(msg_itr, itr);

   pd->connect.pending = eldbus_proxy_send(pd->proxy, msg,
                                           _efl_net_session_connect_change_online_required, o,
                                           DEFAULT_TIMEOUT);
   EINA_SAFETY_ON_NULL_RETURN(pd->connect.pending);
   return;

 error_msg:
   eldbus_message_unref(msg);
}

EOLIAN static void
_efl_net_session_connect(Eo *o, Efl_Net_Session_Data *pd, Eina_Bool online_required, Efl_Net_Session_Technology technologies_allowed)
{
   pd->connect.connected = EINA_TRUE;
   pd->connect.online_required = online_required;
   pd->connect.technologies_allowed = technologies_allowed;

   if (pd->proxy) _efl_net_session_connect_do(o, pd);
}

EOLIAN static void
_efl_net_session_disconnect(Eo *o EINA_UNUSED, Efl_Net_Session_Data *pd)
{
   if (pd->connect.pending)
     {
        eldbus_pending_cancel(pd->connect.pending);
        pd->connect.pending = NULL;
     }
   pd->connect.connected = EINA_FALSE;

   eldbus_proxy_call(pd->proxy, "Disconnect", NULL, NULL, DEFAULT_TIMEOUT, "");
}

#include "efl_net_session.eo.c"

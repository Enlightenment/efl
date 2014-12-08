#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eldbus.h>
#include <Ecore.h>
#include <locale.h>

static int _log_dom = -1;
static Eldbus_Connection *_conn = NULL;

static Eldbus_Object *_obj = NULL;
static Eldbus_Proxy *_proxy = NULL;

#ifdef CRI
#undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_log_dom, __VA_ARGS__)

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_log_dom, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_log_dom, __VA_ARGS__)

static Eina_Bool _ecore_on_battery = EINA_FALSE;
static unsigned int _ecore_low_battery = 0;

static int uversions[3] = {0}; //major, minor, micro
static void _ecore_system_upower_shutdown(void);

static void
_battery_eval(void)
{
   if (uversions[0] >= 1) return;
   if (uversions[1] >= 99)
     ecore_power_state_set(_ecore_low_battery);
   else
     {
        if (_ecore_low_battery)
          ecore_power_state_set(ECORE_POWER_STATE_LOW);
        else if (_ecore_on_battery)
          ecore_power_state_set(ECORE_POWER_STATE_BATTERY);
        else
          ecore_power_state_set(ECORE_POWER_STATE_MAINS);
     }
}

static void
_on_low_battery_from_variant(Eldbus_Message_Iter *variant)
{
   Eina_Bool val;

   if (!eldbus_message_iter_get_and_next(variant, 'b', &val))
     {
        ERR("Error getting OnLowBattery.");
        return;
     }

   DBG("OnLowBattery=%hhu", val);
   _ecore_low_battery = val;
   _battery_eval();
}

static void
_on_low_battery_from_uint(unsigned int level)
{
   DBG("OnLowBattery=%hhu", level);
   _ecore_low_battery = level;
   _battery_eval();
}

static void
_on_low_battery_get_cb(void *data EINA_UNUSED, const Eldbus_Message *msg,
                        Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("Message error %s - %s", errname, errmsg);
        return;
     }

   if (uversions[0] >= 1)
     {
        ERR("Unsupported new UPower version!");
        return;
     }
   if (uversions[1] >= 99)
     {
        unsigned int level;

        if (!eldbus_message_arguments_get(msg, "u", &level))
          {
             ERR("Error getting arguments.");
             return;
          }
        _on_low_battery_from_uint(level);
     }
   else
     {
        Eldbus_Message_Iter *variant;

        if (!eldbus_message_arguments_get(msg, "v", &variant))
          {
             ERR("Error getting arguments.");
             return;
          }
        _on_low_battery_from_variant(variant);
     }
}

static void
_on_low_battery_get(Eldbus_Proxy *proxy)
{
   /* version specific battery properties */
   if (uversions[0] < 1)
     {
        if (uversions[1] >= 99)
          {/* FIXME: this module needs a huge refactoring since WarningLevel property is per-device */}
        else
          eldbus_proxy_property_get(proxy, "OnLowBattery", _on_low_battery_get_cb, NULL);
     }
   else
     CRI("SOMEBODY SHOULD BE MAINTAINING THIS MODULE!!!!!!!");
}

static void
_on_battery_from_variant(Eldbus_Message_Iter *variant)
{
   Eina_Bool val;

   if (!eldbus_message_iter_get_and_next(variant, 'b', &val))
     {
        ERR("Error getting OnBattery.");
        return;
     }

   DBG("OnBattery=%hhu", val);
   _ecore_on_battery = val;
   _battery_eval();
}

static void
_on_battery_get_cb(void *data EINA_UNUSED, const Eldbus_Message *msg,
                        Eldbus_Pending *pending EINA_UNUSED)
{
   Eldbus_Message_Iter *variant;
   const char *errname, *errmsg;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("Message error %s - %s", errname, errmsg);
        return;
     }
   if (!eldbus_message_arguments_get(msg, "v", &variant))
     {
        ERR("Error getting arguments.");
        return;
     }

   _on_battery_from_variant(variant);
}

static void
_on_battery_get(Eldbus_Proxy *proxy)
{
   eldbus_proxy_property_get(proxy, "OnBattery",
                             _on_battery_get_cb, NULL);
}

static void
_props_changed(void *data, const Eldbus_Message *msg)
{
   Eldbus_Proxy *proxy = data;
   Eldbus_Message_Iter *changed, *entry, *invalidated;
   const char *iface, *prop;

   if (!eldbus_message_arguments_get(msg, "sa{sv}as",
                                     &iface, &changed, &invalidated))
     {
        ERR("Error getting data from properties changed signal.");
        return;
     }

   while (eldbus_message_iter_get_and_next(changed, 'e', &entry))
     {
        const void *key;
        Eldbus_Message_Iter *var;
        if (!eldbus_message_iter_arguments_get(entry, "sv", &key, &var))
          continue;
        if (strcmp(key, "OnBattery") == 0)
          _on_battery_from_variant(var);
        if (uversions[0] >= 1)
          {
             ERR("Unsupported new UPower version!");
             return;
          }
        if (uversions[1] >= 99)
          {
             /* FIXME: this will never be hit since it's on the wrong proxy */
#if 0
             if (strcmp(key, "WarningLevel") == 0)
               {
                  unsigned int level;

                  if (!eldbus_message_iter_get_and_next(var, 'u', &level))
                    ERR("Error getting OnBattery.");
                  else
                    _on_low_battery_from_uint(level);
               }
#endif
          }
        else
          {
             if (strcmp(key, "OnLowBattery") == 0)
               _on_low_battery_from_variant(var);
          }
     }

   while (eldbus_message_iter_get_and_next(invalidated, 's', &prop))
     {
        if (strcmp(prop, "OnBattery") == 0)
          _on_battery_get(proxy);
        if (strcmp(prop, "OnLowBattery") == 0)
          _on_low_battery_get(proxy);
     }
}

static void
_version_get(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg, *v;
   char vers[128], *e;
   unsigned int i;
   Eldbus_Message_Iter *variant;

   /* FIXME: if either of these fail...do something? */
   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("Message error %s - %s", errname, errmsg);
        return;
     }
   if ((!eldbus_message_arguments_get(msg, "v", &variant)) ||
       (!eldbus_message_iter_get_and_next(variant, 's', &v)))
     {
        ERR("Error getting version.");
        return;
     }
   e = strncpy(vers, v, sizeof(vers) - 1);
   for (i = 0; e[0] && (i < 3); i++, e++)
     {
        errno = 0;
        uversions[i] = strtol(e, &e, 10);
        if (errno) break;
     }
   if ((uversions[0] >= 1) || (uversions[1] >= 99))
     {
        /* may as well kill the module since it'll do more harm than good */
        ERR("Unsupported new UPower version!");
        _ecore_system_upower_shutdown();
        return;
     }
   _on_low_battery_get(data);
}

static void _upower_name_owner_cb(void *data,
                                  const char *bus EINA_UNUSED,
                                  const char *old_id,
                                  const char *new_id)
{
   Eldbus_Proxy *proxy = data;

   DBG("org.freedesktop.UPower name owner changed from '%s' to '%s'",
       old_id, new_id);

   if ((new_id) && (new_id[0]))
     eldbus_proxy_property_get(proxy, "DaemonVersion", _version_get, proxy);
}

static void _ecore_system_upower_shutdown(void);

static Eina_Bool
_ecore_system_upower_init(void)
{
   Eldbus_Signal_Handler *s;

   eldbus_init();

   _log_dom = eina_log_domain_register("ecore_system_upower", NULL);
   if (_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: ecore_system_upower");
        goto error;
     }

   _conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);

   _obj = eldbus_object_get(_conn, "org.freedesktop.UPower",
                            "/org/freedesktop/UPower");
   if (!_obj)
     {
        ERR("could not get object name=org.freedesktop.UPower, "
            "path=/org/freedesktop/UPower");
        goto error;
     }

   _proxy = eldbus_proxy_get(_obj, "org.freedesktop.UPower");
   if (!_proxy)
     {
        ERR("could not get proxy interface=org.freedesktop.UPower, "
            "name=org.freedesktop.UPower, path=/org/freedesktop/UPower");
        goto error;
     }

   s = eldbus_proxy_properties_changed_callback_add(_proxy, _props_changed,
                                                    _proxy);
   if (!s)
     {
        ERR("could not add signal handler for properties changed for proxy "
            "interface=org.freedesktop.UPower, name=org.freedesktop.UPower, "
            "path=/org/freedesktop/UPower");
        goto error;
     }

   eldbus_name_owner_changed_callback_add(_conn, "org.freedesktop.UPower",
                                          _upower_name_owner_cb,
                                          _proxy, EINA_TRUE);

   DBG("ecore system 'upower' loaded");
   return EINA_TRUE;

 error:
   _ecore_system_upower_shutdown();
   return EINA_FALSE;
}

static void
_ecore_system_upower_shutdown(void)
{
   DBG("ecore system 'upower' unloaded");

   eldbus_name_owner_changed_callback_del(_conn, "org.freedesktop.UPower",
                                          _upower_name_owner_cb,
                                          NULL);

   if (_proxy)
     {
        eldbus_proxy_unref(_proxy);
        _proxy = NULL;
     }

   if (_obj)
     {
        eldbus_object_unref(_obj);
        _obj = NULL;
     }

   if (_conn)
     {
        eldbus_connection_unref(_conn);
        _conn = NULL;
     }

   if (_log_dom > 0)
     {
        eina_log_domain_unregister(_log_dom);
        _log_dom = -1;
     }

   eldbus_shutdown();
   memset(&uversions, 0, sizeof(uversions));
}

EINA_MODULE_INIT(_ecore_system_upower_init);
EINA_MODULE_SHUTDOWN(_ecore_system_upower_shutdown);

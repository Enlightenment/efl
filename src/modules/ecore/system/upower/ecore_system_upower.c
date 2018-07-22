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

static Eldbus_Object *_disp_obj = NULL;
static Eldbus_Proxy *_disp_proxy = NULL;

typedef enum {
     VERSION_ON_LOW_BATTERY,
     VERSION_WARNING_LEVEL
}Ecore_System_Upower_Version;
static Ecore_System_Upower_Version _version = 0;

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
static Eina_Bool _ecore_low_battery = EINA_FALSE;
static int _ecore_battery_level = -1;

static Eina_List *_eldbus_pending = NULL;

static Eina_Bool _ecore_system_upower_display_device_init(void);
static void _ecore_system_upower_shutdown(void);

static void
_battery_eval(void)
{
   Ecore_Power_State power_state = ECORE_POWER_STATE_MAINS;

   if (_ecore_low_battery)
     {
        power_state = ECORE_POWER_STATE_LOW;
     }
   else if (_ecore_on_battery)
     {
        power_state = ECORE_POWER_STATE_BATTERY;

        /* FIXME: get level value from libupower? */
        if (_ecore_battery_level >= 3)
          {
             power_state = ECORE_POWER_STATE_LOW;
          }
     }

   ecore_power_state_set(power_state);
}

static void
_warning_level_from_variant(Eldbus_Message_Iter *variant)
{
   unsigned int val;

   if (!eldbus_message_iter_get_and_next(variant, 'u', &val))
     {
        ERR("Error getting WarningLevel.");
        return;
     }

   _ecore_battery_level = val;
   _battery_eval();
}

static void
_warning_level_get_cb(void *data EINA_UNUSED,
                      const Eldbus_Message *msg,
                      Eldbus_Pending *pending)
{
   Eldbus_Message_Iter *variant;
   const char *errname, *errmsg;

   _eldbus_pending = eina_list_remove(_eldbus_pending, pending);
   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
// don't print errors because this results in complaints about upower not
// existing and it's OK if it doesn't exist. just no feature enabled then
//        ERR("Message error %s - %s", errname, errmsg);
        return;
     }
   if (!eldbus_message_arguments_get(msg, "v", &variant))
     {
        ERR("Error getting arguments.");
        return;
     }

   _warning_level_from_variant(variant);
}

static void
_warning_level_get(Eldbus_Proxy *proxy)
{
   Eldbus_Pending *pend;

   pend = eldbus_proxy_property_get(proxy, "WarningLevel",
                                    _warning_level_get_cb, NULL);
   _eldbus_pending = eina_list_append(_eldbus_pending, pend);
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
_on_low_battery_get_cb(void *data EINA_UNUSED, const Eldbus_Message *msg,
                        Eldbus_Pending *pending)
{
   Eldbus_Message_Iter *variant;
   const char *errname, *errmsg;

   _eldbus_pending = eina_list_remove(_eldbus_pending, pending);
   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        if (strcmp(errname, "org.enlightenment.DBus.Canceled"))
          ERR("Message error %s - %s", errname, errmsg);
        return;
     }
   if (!eldbus_message_arguments_get(msg, "v", &variant))
     {
        ERR("Error getting arguments.");
        return;
     }

   _on_low_battery_from_variant(variant);
}

static void
_on_low_battery_get(Eldbus_Proxy *proxy)
{
   Eldbus_Pending *pend;

   pend = eldbus_proxy_property_get(proxy, "OnLowBattery",
                                    _on_low_battery_get_cb, NULL);
   _eldbus_pending = eina_list_append(_eldbus_pending, pend);
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
                        Eldbus_Pending *pending)
{
   Eldbus_Message_Iter *variant;
   const char *errname, *errmsg;

   _eldbus_pending = eina_list_remove(_eldbus_pending, pending);
   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        if (strcmp(errname, "org.enlightenment.DBus.Canceled"))
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
   Eldbus_Pending *pend;

   pend = eldbus_proxy_property_get(proxy, "OnBattery",
                                    _on_battery_get_cb, NULL);
   _eldbus_pending = eina_list_append(_eldbus_pending, pend);
}

static void
_battery_state_get()
{
   switch (_version)
     {
      case VERSION_ON_LOW_BATTERY:
         _on_low_battery_get(_proxy);
         break;
      case VERSION_WARNING_LEVEL:
         if (_ecore_system_upower_display_device_init())
           _warning_level_get(_disp_proxy);
         break;
      default:
         break;
     }
}

static void
_daemon_version_from_variant(Eldbus_Message_Iter *variant)
{
   const char *val;
   char **version;
   int standard[3] = {0, 99, 0}; // upower >= 0.99.0 provides WarningLevel instead of OnLowBattery
   int i;

   if (!eldbus_message_iter_get_and_next(variant, 's', &val))
     {
        ERR("Error getting DaemonVersion.");
        return;
     }
   version = eina_str_split(val, ".", 3);

   for (i = 0; i < 3; i ++)
     {
        if (atoi(version[i]) > standard[i])
          {
             _version = VERSION_WARNING_LEVEL;
             break;
          }
        else if (atoi(version[i]) < standard[i])
          {
             _version = VERSION_ON_LOW_BATTERY;
             break;
          }
        else if (i == 2)
          {
             _version = VERSION_WARNING_LEVEL;
             break;
          }
     }
   free(version[0]);
   free(version);

   _battery_state_get();
}

static void
_daemon_version_get_cb(void *data EINA_UNUSED, const Eldbus_Message *msg,
                          Eldbus_Pending *pending)
{
   Eldbus_Message_Iter *variant;
   const char *errname, *errmsg;

   _eldbus_pending = eina_list_remove(_eldbus_pending, pending);
   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        if (strcmp(errname, "org.enlightenment.DBus.Canceled"))
          ERR("Message error %s - %s", errname, errmsg);
        return;
     }
   if (!eldbus_message_arguments_get(msg, "v", &variant))
     {
        ERR("Error getting arguments.");
        return;
     }

   _daemon_version_from_variant(variant);
}

static void
_daemon_version_get(Eldbus_Proxy *proxy)
{
   Eldbus_Pending *pend;

   pend = eldbus_proxy_property_get(proxy, "DaemonVersion",
                                    _daemon_version_get_cb, NULL);
   _eldbus_pending = eina_list_append(_eldbus_pending, pend);
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
        if (strcmp(key, "OnLowBattery") == 0)
          _on_low_battery_from_variant(var);
        if (strcmp(key, "WarningLevel") == 0)
          _warning_level_from_variant(var);
     }

   while (eldbus_message_iter_get_and_next(invalidated, 's', &prop))
     {
        if (strcmp(prop, "OnBattery") == 0)
          _on_battery_get(proxy);
        if (strcmp(prop, "OnLowBattery") == 0)
          _on_low_battery_get(proxy);
        if (strcmp(prop, "WarningLevel") == 0)
          _warning_level_get(proxy);
     }
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
     {
        _daemon_version_get(proxy);
     }
}

static Eina_Bool
_ecore_system_upower_display_device_init(void)
{
   Eldbus_Signal_Handler *s;

   _disp_obj =
      eldbus_object_get(_conn, "org.freedesktop.UPower",
                        "/org/freedesktop/UPower/devices/DisplayDevice");
   if (!_disp_obj)
     {
        ERR("could not get object name=org.freedesktop.UPower, "
            "path=/org/freedesktop/UPower/devices/DisplayDevice");
        goto disp_error;
     }

   _disp_proxy = eldbus_proxy_get(_disp_obj, "org.freedesktop.UPower");
   if (!_disp_proxy)
     {
        ERR("could not get proxy interface=org.freedesktop.UPower, "
            "name=org.freedesktop.UPower, "
            "path=/org/freedesktop/UPower/devices/DisplayDevice");
        goto disp_error;
     }

   s = eldbus_proxy_properties_changed_callback_add(_disp_proxy,
                                                    _props_changed,
                                                    _disp_proxy);
   if (!s)
     {
        ERR("could not add signal handler for properties changed for proxy "
            "interface=org.freedesktop.UPower, "
            "name=org.freedesktop.UPower, "
            "path=/org/freedesktop/UPower/devices/DisplayDevice");
        goto disp_error;
     }

   return EINA_TRUE;

disp_error:
   _ecore_system_upower_shutdown();
   return EINA_FALSE;
}

static Eina_Bool _ecore_system_upower_init(void);
static void _ecore_system_upower_shutdown(void);
static unsigned int reseting;

static void
_ecore_system_upower_reset()
{
   reseting = 1;
   _ecore_system_upower_shutdown();
   _ecore_system_upower_init();
   reseting = 0;
}
static Eina_Bool
_ecore_system_upower_init(void)
{
   Eldbus_Signal_Handler *s;

   eldbus_init();
   if (!reseting)
     ecore_fork_reset_callback_add(_ecore_system_upower_reset, NULL);

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
   Eldbus_Pending *pend;

   DBG("ecore system 'upower' unloaded");
   if (!reseting)
     ecore_fork_reset_callback_del(_ecore_system_upower_reset, NULL);

   eldbus_name_owner_changed_callback_del(_conn, "org.freedesktop.UPower",
                                          _upower_name_owner_cb,
                                          NULL);
  if (_disp_proxy)
     {
        eldbus_proxy_unref(_disp_proxy);
        _disp_proxy = NULL;
     }

   if (_disp_obj)
     {
        eldbus_object_unref(_disp_obj);
        _disp_obj = NULL;
     }

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

   EINA_LIST_FREE(_eldbus_pending, pend)
     {
        eldbus_pending_cancel(pend);
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
}

EINA_MODULE_INIT(_ecore_system_upower_init);
EINA_MODULE_SHUTDOWN(_ecore_system_upower_shutdown);

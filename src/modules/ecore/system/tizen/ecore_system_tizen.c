#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include <vconf.h>
#include <locale.h>

static int _log_dom = -1;

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

static void _low_mem_key_changed_cb(keynode_t *node EINA_UNUSED, void *data EINA_UNUSED)
{
   int status;

   if (vconf_get_int(VCONFKEY_SYSMAN_LOW_MEMORY, &status) < 0)
     return;

   if (status == VCONFKEY_SYSMAN_LOW_MEMORY_NORMAL)
     ecore_memory_state_set(ECORE_MEMORY_STATE_NORMAL);
   else
     ecore_memory_state_set(ECORE_MEMORY_STATE_LOW);
}

static void _charge_key_changed_cb(keynode_t *node EINA_UNUSED, void *data EINA_UNUSED)
{
   int charging, status;

   if (vconf_get_int(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, &charging) < 0)
     return;

   if (charging)
     {
        ecore_power_state_set(ECORE_POWER_STATE_MAINS);
        return;
     }

   if (vconf_get_int(VCONFKEY_SYSMAN_BATTERY_STATUS_LOW, &status) < 0)
     return;

   if (status == VCONFKEY_SYSMAN_BAT_NORMAL || status == VCONFKEY_SYSMAN_BAT_FULL)
     ecore_power_state_set(ECORE_POWER_STATE_BATTERY);
   else
     ecore_power_state_set(ECORE_POWER_STATE_LOW);
}

static void _low_batt_key_changed_cb(keynode_t *node EINA_UNUSED, void *data EINA_UNUSED)
{
   _charge_key_changed_cb(NULL, NULL);
}

static void _lang_key_changed_cb(keynode_t *node EINA_UNUSED, void *first)
{
   char *lang;

   lang = vconf_get_str(VCONFKEY_LANGSET);
   if (!lang)
     return;

   setenv("LANG", lang, 1);
   setenv("LC_MESSAGES", lang, 1);
   setlocale(__LC_ALL, "");

   if (!first)
     ecore_event_add(ECORE_EVENT_LOCALE_CHANGED, NULL, NULL, NULL);
   free(lang);
}

static void _region_fmt_key_changed_cb(keynode_t *node EINA_UNUSED, void *first)
{
   char *region;

   region = vconf_get_str(VCONFKEY_REGIONFORMAT);
   if (!region)
     return;

   setenv("LC_CTYPE", region, 1);
   setenv("LC_NUMERIC", region, 1);
   setenv("LC_TIME", region, 1);
   setenv("LC_COLLATE", region, 1);
   setenv("LC_MONETARY", region, 1);
   setenv("LC_PAPER", region, 1);
   setenv("LC_NAME", region, 1);
   setenv("LC_ADDRESS", region, 1);
   setenv("LC_TELEPHONE", region, 1);
   setenv("LC_MEASUREMENT", region, 1);
   setenv("LC_IDENTIFICATION", region, 1);
   setlocale(__LC_ALL, "");

   if (!first)
     ecore_event_add(ECORE_EVENT_LOCALE_CHANGED, NULL, NULL, NULL);
   free(region);
}

static void _time_fmt_key_changed_cb(keynode_t *node EINA_UNUSED, void *data EINA_UNUSED)
{
   ecore_event_add(ECORE_EVENT_LOCALE_CHANGED, NULL, NULL, NULL);
}

static Eina_Bool
_ecore_system_tizen_init(void)
{
   _log_dom = eina_log_domain_register("ecore_system_tizen", NULL);
   if (_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: ecore_system_tizen");
        return EINA_FALSE;
     }

   if (vconf_notify_key_changed(VCONFKEY_SYSMAN_LOW_MEMORY, _low_mem_key_changed_cb, NULL) < 0)
     {
        ERR("Unable to register a vconf changed cb to %s.", VCONFKEY_SYSMAN_LOW_MEMORY);
        goto err_low_memory;
     }
   _low_mem_key_changed_cb(NULL, NULL);

   if (vconf_notify_key_changed(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, _charge_key_changed_cb, NULL) < 0)
     {
        ERR("Unable to register a vconf changed cb to %s.", VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW);
        goto err_charge;
     }

   if (vconf_notify_key_changed(VCONFKEY_SYSMAN_BATTERY_STATUS_LOW, _low_batt_key_changed_cb, NULL) < 0)
     {
        ERR("Unable to register a vconf changed cb to %s.", VCONFKEY_SYSMAN_BATTERY_STATUS_LOW);
        goto err_batt_status_low;
     }
   _low_batt_key_changed_cb(NULL, NULL);

   if (vconf_notify_key_changed(VCONFKEY_LANGSET, _lang_key_changed_cb, NULL) < 0)
     {
        ERR("Unable to register a vconf changed cb to %s.", VCONFKEY_LANGSET);
        goto err_lang;
     }
   _lang_key_changed_cb(NULL, (void *)1);

   if (vconf_notify_key_changed(VCONFKEY_REGIONFORMAT, _region_fmt_key_changed_cb, NULL) < 0)
     {
        ERR("Unable to register a vconf changed cb to %s.", VCONFKEY_REGIONFORMAT);
        goto err_region_fmt;
     }
   _region_fmt_key_changed_cb(NULL, (void *)1);

   if (vconf_notify_key_changed(VCONFKEY_REGIONFORMAT_TIME1224, _time_fmt_key_changed_cb, NULL) < 0)
     {
        ERR("Unable to register a vconf changed cb to %s.", VCONFKEY_REGIONFORMAT_TIME1224);
        goto err_time_fmt;
     }

   return EINA_TRUE;

err_time_fmt:
   vconf_ignore_key_changed(VCONFKEY_REGIONFORMAT, _lang_key_changed_cb);
err_region_fmt:
   vconf_ignore_key_changed(VCONFKEY_LANGSET, _lang_key_changed_cb);
err_lang:
   vconf_ignore_key_changed(VCONFKEY_SYSMAN_BATTERY_STATUS_LOW, _low_batt_key_changed_cb);
err_batt_status_low:
   vconf_ignore_key_changed(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, _charge_key_changed_cb);
err_charge:
   vconf_ignore_key_changed(VCONFKEY_SYSMAN_LOW_MEMORY, _low_mem_key_changed_cb);
err_low_memory:
   eina_log_domain_unregister(_log_dom);
   _log_dom = -1;
   return EINA_FALSE;
}

static void
_ecore_system_tizen_shutdown(void)
{
   vconf_ignore_key_changed(VCONFKEY_REGIONFORMAT, _region_fmt_key_changed_cb);
   vconf_ignore_key_changed(VCONFKEY_LANGSET, _lang_key_changed_cb);
   vconf_ignore_key_changed(VCONFKEY_SYSMAN_BATTERY_STATUS_LOW, _low_batt_key_changed_cb);
   vconf_ignore_key_changed(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, _charge_key_changed_cb);
   vconf_ignore_key_changed(VCONFKEY_SYSMAN_LOW_MEMORY, _low_mem_key_changed_cb);
   eina_log_domain_unregister(_log_dom);
   _log_dom = -1;
}

EINA_MODULE_INIT(_ecore_system_tizen_init);
EINA_MODULE_SHUTDOWN(_ecore_system_tizen_shutdown);

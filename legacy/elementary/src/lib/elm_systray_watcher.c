#include "Elementary.h"
#include "elm_priv.h"

#include "elm_systray_watcher.h"

#ifdef ELM_EDBUS2
#define OBJ       "/StatusNotifierWatcher"
#define BUS       "org.kde.StatusNotifierWatcher"
#define INTERFACE "org.kde.StatusNotifierWatcher"

static Eina_Bool _elm_systray_watcher = EINA_FALSE;

static EDBus_Connection *_watcher_conn  = NULL;
static EDBus_Object     *_watcher_obj   = NULL;
static EDBus_Proxy      *_watcher_proxy = NULL;

static void
_status_notifier_item_register_cb(void *data EINA_UNUSED,
                                  const EDBus_Message *msg,
                                  EDBus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;

   if (edbus_message_error_get(msg, &errname, &errmsg))
     ERR("Edbus Error: %s %s", errname, errmsg);
}
#endif

Eina_Bool
_elm_systray_watcher_status_notifier_item_register(const char *obj)
{
#ifdef ELM_EDBUS2
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(_watcher_proxy, EINA_FALSE);

   if (!edbus_proxy_call(_watcher_proxy, "RegisterStatusNotifierItem",
                         _status_notifier_item_register_cb,
                         NULL, -1, "s", obj))
     {
        ERR("Error sending message: "INTERFACE".RegisterStatusNotifierItem.");
        return EINA_FALSE;
     }

   return EINA_TRUE;
#else
   (void) obj;
   return EINA_FALSE;
#endif
}

#ifdef ELM_EDBUS2
static void
_release(void)
{
   if (_watcher_proxy)
     {
        edbus_proxy_unref(_watcher_proxy);
        _watcher_proxy = NULL;
     }

   if (_watcher_obj)
     {
        edbus_object_unref(_watcher_obj);
        _watcher_obj = NULL;
     }
}

static void
_update(void)
{
   _release();

   _watcher_obj = edbus_object_get(_watcher_conn, BUS, OBJ);
   _watcher_proxy = edbus_proxy_get(_watcher_obj, INTERFACE);

   ecore_event_add(ELM_EVENT_SYSTRAY_READY, NULL, NULL, NULL);
}

static void
_name_owner_changed_cb(void *data EINA_UNUSED,
                       const char *bus EINA_UNUSED,
                       const char *old_id EINA_UNUSED,
                       const char *new_id)
{
   if ((!new_id) || (*new_id == '\0'))
     _release();
   else
     _update();
}
#endif

Eina_Bool
_elm_systray_watcher_init(void)
{
#ifdef ELM_EDBUS2
   if (_elm_systray_watcher) return EINA_TRUE;

   if (!elm_need_edbus()) return EINA_FALSE;

   _watcher_conn = edbus_connection_get(EDBUS_CONNECTION_TYPE_SESSION);

   edbus_name_owner_changed_callback_add(_watcher_conn, BUS,
                                         _name_owner_changed_cb, NULL,
                                         EINA_TRUE);

   _elm_systray_watcher = EINA_TRUE;
   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

void
_elm_systray_watcher_shutdown(void)
{
#ifdef ELM_EDBUS2
   if (!_elm_systray_watcher) return;

   _elm_systray_watcher = EINA_FALSE;

   _release();

   edbus_connection_unref(_watcher_conn);
   _watcher_conn = NULL;
#endif
}

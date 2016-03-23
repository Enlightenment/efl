#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"

#include "elm_systray_watcher.h"

#define OBJ       "/StatusNotifierWatcher"
#define BUS       "org.kde.StatusNotifierWatcher"
#define INTERFACE "org.kde.StatusNotifierWatcher"

static Eina_Bool _elm_systray_watcher = EINA_FALSE;

static Eldbus_Connection *_watcher_conn  = NULL;
static Eldbus_Object     *_watcher_obj   = NULL;
static Eldbus_Proxy      *_watcher_proxy = NULL;

static void
_status_notifier_item_register_cb(void *data EINA_UNUSED,
                                  const Eldbus_Message *msg,
                                  Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     ERR("Eldbus Error: %s %s", errname, errmsg);
}

Eina_Bool
_elm_systray_watcher_status_notifier_item_register(const char *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(_watcher_proxy, EINA_FALSE);

   if (!eldbus_proxy_call(_watcher_proxy, "RegisterStatusNotifierItem",
                         _status_notifier_item_register_cb,
                         NULL, -1, "s", obj))
     {
        ERR("Error sending message: "INTERFACE".RegisterStatusNotifierItem.");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static void
_release(void)
{
   if (_watcher_proxy)
     {
        eldbus_proxy_unref(_watcher_proxy);
        _watcher_proxy = NULL;
     }

   if (_watcher_obj)
     {
        eldbus_object_unref(_watcher_obj);
        _watcher_obj = NULL;
     }
}

static void
_update(void)
{
   _release();

   _watcher_obj = eldbus_object_get(_watcher_conn, BUS, OBJ);
   _watcher_proxy = eldbus_proxy_get(_watcher_obj, INTERFACE);

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

Eina_Bool
_elm_systray_watcher_init(void)
{
   if (_elm_systray_watcher) return EINA_TRUE;

   if (!elm_need_eldbus()) return EINA_FALSE;

   _watcher_conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);

   eldbus_name_owner_changed_callback_add(_watcher_conn, BUS,
                                         _name_owner_changed_cb, NULL,
                                         EINA_TRUE);

   _elm_systray_watcher = EINA_TRUE;
   return EINA_TRUE;
}

void
_elm_systray_watcher_shutdown(void)
{
   if (!_elm_systray_watcher) return;

   _elm_systray_watcher = EINA_FALSE;

   _release();

   eldbus_connection_unref(_watcher_conn);
   _watcher_conn = NULL;
}

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"

#define OBJ       "/org/freedesktop/Notifications"
#define BUS       "org.freedesktop.Notifications"
#define INTERFACE "org.freedesktop.Notifications"

EAPI int ELM_EVENT_SYS_NOTIFY_NOTIFICATION_CLOSED = 0;
EAPI int ELM_EVENT_SYS_NOTIFY_ACTION_INVOKED      = 0;

static Eina_Bool _elm_need_sys_notify = EINA_FALSE;

static Eldbus_Connection *_elm_sysnotif_conn  = NULL;
static Eldbus_Object     *_elm_sysnotif_obj   = NULL;
static Eldbus_Proxy      *_elm_sysnotif_proxy = NULL;

static Eina_Bool _has_markup = EINA_FALSE;

typedef struct _Elm_Sys_Notify_Send_Data
{
   Elm_Sys_Notify_Send_Cb cb;
   const void *data;
} Elm_Sys_Notify_Send_Data;

static void
_elm_sys_notify_marshal_dict_byte(Eldbus_Message_Iter *array,
                                  const char *key,
                                  const char value)
{
   Eldbus_Message_Iter *var, *entry;

   eldbus_message_iter_arguments_append(array, "{sv}", &entry);
   eldbus_message_iter_basic_append(entry, 's', key);

   var = eldbus_message_iter_container_new(entry, 'v', "y");
   eldbus_message_iter_basic_append(var, 'y', value);
   eldbus_message_iter_container_close(entry, var);
   eldbus_message_iter_container_close(array, entry);
}

static void
_elm_sys_notify_marshal_dict_string(Eldbus_Message_Iter *array,
                                   const char *key,
                                   const char *value)
{
   Eldbus_Message_Iter *var, *entry;

   eldbus_message_iter_arguments_append(array, "{sv}", &entry);
   eldbus_message_iter_basic_append(entry, 's', key);

   var = eldbus_message_iter_container_new(entry, 'v', "s");
   eldbus_message_iter_basic_append(var, 's', value);
   eldbus_message_iter_container_close(entry, var);
   eldbus_message_iter_container_close(array, entry);
}

static void
_get_capabilities_cb(void *data EINA_UNUSED,
                     const Eldbus_Message *msg,
                     Eldbus_Pending *pending EINA_UNUSED)
{
   char *val;
   Eldbus_Message_Iter *arr;

   if (eldbus_message_error_get(msg, NULL, NULL) ||
       !eldbus_message_arguments_get(msg, "as", &arr)) goto end;

   while (eldbus_message_iter_get_and_next(arr, 's', &val))
     if (!strcmp(val, "body-markup"))
       {
          _has_markup = EINA_TRUE;
          return;
       }

end:
   _has_markup = EINA_FALSE;
}

void
_elm_sys_notify_capabilities_get(void)
{
   EINA_SAFETY_ON_NULL_RETURN(_elm_sysnotif_proxy);

   if (!eldbus_proxy_call(_elm_sysnotif_proxy, "GetCapabilities",
                         _get_capabilities_cb, NULL, -1, ""))
     ERR("Error sending message: "INTERFACE".GetCapabilities.");
}

static void
_close_notification_cb(void *data EINA_UNUSED,
                       const Eldbus_Message *msg,
                       Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        if (errmsg && errmsg[0] == '\0')
          INF("Notification no longer exists.");
        else
          ERR("Eldbus Error: %s %s", errname, errmsg);
     }
}

EAPI void
elm_sys_notify_close(unsigned int id)
{
   EINA_SAFETY_ON_FALSE_RETURN(_elm_need_sys_notify);
   EINA_SAFETY_ON_NULL_RETURN(_elm_sysnotif_proxy);

   if (!eldbus_proxy_call(_elm_sysnotif_proxy, "CloseNotification",
                         _close_notification_cb, NULL, -1, "u", id))
     ERR("Error sending message: "INTERFACE".CloseNotification.");
}

static void
_notify_cb(void *data,
           const Eldbus_Message *msg,
           Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;
   Elm_Sys_Notify_Send_Data *d = data;
   unsigned int id = 0;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     ERR("Error: %s %s", errname, errmsg);
   else if (!eldbus_message_arguments_get(msg, "u", &id))
     {
        ERR("Error getting return values of "INTERFACE".Notify.");
        id = 0;
     }

   if (d->cb) d->cb((void *)d->data, id);
   free(d);
}

EAPI void
elm_sys_notify_send(unsigned int replaces_id, const char *icon,
                    const char *summary, const char *body,
                    Elm_Sys_Notify_Urgency urgency, int timeout,
                    Elm_Sys_Notify_Send_Cb cb, const void *cb_data)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *iter, *actions, *hints;
   Elm_Sys_Notify_Send_Data *data;
   char *body_free = NULL;
   char *desk_free = NULL;
   const char *deskentry = elm_app_desktop_entry_get();
   const char *appname = elm_app_name_get();

   EINA_SAFETY_ON_FALSE_RETURN(_elm_need_sys_notify);
   EINA_SAFETY_ON_NULL_RETURN(_elm_sysnotif_proxy);

   data = malloc(sizeof(Elm_Sys_Notify_Send_Data));
   EINA_SAFETY_ON_NULL_GOTO(data, error);
   data->cb = cb;
   data->data = cb_data;

   if (!icon) icon = "";
   if (!summary) summary = "";
   if (!body)
     body = "";
   else if (!_has_markup)
     body = body_free = elm_entry_markup_to_utf8(body);

   msg = eldbus_proxy_method_call_new(_elm_sysnotif_proxy, "Notify");

   iter = eldbus_message_iter_get(msg);
   eldbus_message_iter_arguments_append(iter, "susssas", appname, replaces_id,
                                       icon, summary, body, &actions);
   /* actions */
   eldbus_message_iter_container_close(iter, actions);

   /* hints */
   eldbus_message_iter_arguments_append(iter, "a{sv}", &hints);
   _elm_sys_notify_marshal_dict_byte(hints, "urgency", (char) urgency);

   if (strcmp(deskentry, ""))
     {
        deskentry = ecore_file_file_get(deskentry);
        deskentry = desk_free = ecore_file_strip_ext(deskentry);
        _elm_sys_notify_marshal_dict_string(hints, "desktop_entry", deskentry);
     }
   eldbus_message_iter_container_close(iter, hints);

   /* timeout */
   eldbus_message_iter_arguments_append(iter, "i", timeout);

   eldbus_proxy_send(_elm_sysnotif_proxy, msg, _notify_cb, data, -1);
   free(desk_free);
   free(body_free);
   return;

error:
   if (cb) cb((void *)cb_data, 0);
}

static void
_on_notification_closed(void *data EINA_UNUSED,
                        const Eldbus_Message *msg)
{
   const char *errname;
   const char *errmsg;
   Elm_Sys_Notify_Notification_Closed *d;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("Eldbus Error: %s %s", errname, errmsg);
        return;
     }

   d = malloc(sizeof(*d));

   if (!eldbus_message_arguments_get(msg, "uu", &(d->id), &(d->reason)))
     {
        ERR("Error processing signal: "INTERFACE".NotificationClosed.");
        goto cleanup;
     }

   if (!ecore_event_add(ELM_EVENT_SYS_NOTIFY_NOTIFICATION_CLOSED, d,
                        NULL, NULL)) goto cleanup;

   return;

cleanup:
   free(d);
}

static void
_ev_action_invoked_free(void *data EINA_UNUSED,
                        void *ev_data)
{
   Elm_Sys_Notify_Action_Invoked *d = ev_data;

   free(d->action_key);
   free(d);
}

static void
_on_action_invoked(void *data EINA_UNUSED,
                   const Eldbus_Message *msg)
{
   const char *errname;
   const char *aux;

   Elm_Sys_Notify_Action_Invoked *d;

   if (eldbus_message_error_get(msg, &errname, &aux))
     {
        ERR("Eldbus Error: %s %s", errname, aux);
        return;
     }

   d = calloc(1, sizeof(*d));

   if (!eldbus_message_arguments_get(msg, "us", &(d->id), &aux))
     {
        ERR("Error processing signal: "INTERFACE".ActionInvoked.");
        goto cleanup;
     }

   d->action_key = strdup(aux);

   if (!ecore_event_add(ELM_EVENT_SYS_NOTIFY_ACTION_INVOKED, d,
                        _ev_action_invoked_free, NULL)) goto cleanup;

   return;

cleanup:
   free(d->action_key);
   free(d);
}

static void
_release(void)
{
   if (_elm_sysnotif_proxy)
     {
        eldbus_proxy_unref(_elm_sysnotif_proxy);
        _elm_sysnotif_proxy = NULL;
     }

   if (_elm_sysnotif_obj)
     {
        eldbus_object_unref(_elm_sysnotif_obj);
        _elm_sysnotif_obj = NULL;
     }
}

static void
_update(void)
{
   _release();
   _elm_sysnotif_obj = eldbus_object_get(_elm_sysnotif_conn, BUS, OBJ);
   _elm_sysnotif_proxy = eldbus_proxy_get(_elm_sysnotif_obj, INTERFACE);
   _elm_sys_notify_capabilities_get();

   eldbus_proxy_signal_handler_add(_elm_sysnotif_proxy, "NotificationClosed",
                                  _on_notification_closed, NULL);

   eldbus_proxy_signal_handler_add(_elm_sysnotif_proxy, "ActionInvoked",
                                  _on_action_invoked, NULL);
}

static void
_name_owner_get_cb(void *data EINA_UNUSED,
                   const Eldbus_Message *msg,
                   Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     ERR("Eldbus Error: %s %s", errname, errmsg);
   else
     _update();
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

EAPI Eina_Bool
elm_need_sys_notify(void)
{
   if (_elm_need_sys_notify) return EINA_TRUE;

   if (!elm_need_eldbus()) return EINA_FALSE;

   if (!ELM_EVENT_SYS_NOTIFY_NOTIFICATION_CLOSED)
     ELM_EVENT_SYS_NOTIFY_NOTIFICATION_CLOSED = ecore_event_type_new();

   if (!ELM_EVENT_SYS_NOTIFY_ACTION_INVOKED)
     ELM_EVENT_SYS_NOTIFY_ACTION_INVOKED = ecore_event_type_new();

   _elm_sysnotif_conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);

   eldbus_name_owner_changed_callback_add(_elm_sysnotif_conn, BUS,
                                         _name_owner_changed_cb, NULL,
                                         EINA_FALSE);

   eldbus_name_owner_get(_elm_sysnotif_conn, BUS, _name_owner_get_cb, NULL);

   _elm_need_sys_notify = EINA_TRUE;

   return EINA_TRUE;
}

void
_elm_unneed_sys_notify(void)
{
   if (!_elm_need_sys_notify) return;

   _elm_need_sys_notify = EINA_FALSE;

   _release();

   eldbus_connection_unref(_elm_sysnotif_conn);
   _elm_sysnotif_conn  = NULL;
}

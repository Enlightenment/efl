#include "edbus_private.h"
#include "edbus_private_types.h"
#include <dbus/dbus.h>

/* TODO: mempool of EDBus_Pending */
#define EDBUS_PENDING_CHECK(pending)                        \
  do                                                        \
    {                                                       \
       EINA_SAFETY_ON_NULL_RETURN(pending);                 \
       if (!EINA_MAGIC_CHECK(pending, EDBUS_PENDING_MAGIC)) \
         {                                                  \
            EINA_MAGIC_FAIL(pending, EDBUS_PENDING_MAGIC);  \
            return;                                         \
         }                                                  \
    }                                                       \
  while (0)

#define EDBUS_PENDING_CHECK_RETVAL(pending, retval)         \
  do                                                        \
    {                                                       \
       EINA_SAFETY_ON_NULL_RETURN_VAL(pending, retval);     \
       if (!EINA_MAGIC_CHECK(pending, EDBUS_PENDING_MAGIC)) \
         {                                                  \
            EINA_MAGIC_FAIL(pending, EDBUS_PENDING_MAGIC);  \
            return retval;                                  \
         }                                                  \
    }                                                       \
  while (0)

static void edbus_pending_dispatch(EDBus_Pending *pending, EDBus_Message *msg);

Eina_Bool
edbus_pending_init(void)
{
   return EINA_TRUE;
}

void
edbus_pending_shutdown(void)
{
}

static void
cb_pending(DBusPendingCall *dbus_pending, void *user_data)
{
   EDBus_Message *msg;
   EDBus_Pending *pending = user_data;

   if (!dbus_pending_call_get_completed(dbus_pending))
     {
        INF("timeout to pending %p", pending);
        dbus_pending_call_cancel(dbus_pending);
        msg = edbus_message_error_new(pending->msg_sent,
                                      "org.enlightenment.DBus.Timeout",
                                      "This call was not completed.");
        edbus_pending_dispatch(pending, msg);
        return;
     }

   msg = edbus_message_new(EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN(msg);
   msg->dbus_msg = dbus_pending_call_steal_reply(dbus_pending);
   if (!msg->dbus_msg)
     {
        EINA_SAFETY_ON_NULL_GOTO(pending->cb, cleanup);

        msg->dbus_msg = dbus_message_new_error(NULL,
                            "org.enlightenment.DBus.NoReply",
                            "There was no reply to this method call.");
        EINA_SAFETY_ON_NULL_GOTO(msg->dbus_msg, cleanup);
     }

   dbus_message_iter_init(msg->dbus_msg, &msg->iterator->dbus_iterator);
   edbus_pending_dispatch(pending, msg);

   return;

cleanup:
   edbus_message_unref(msg);
}

static void
_on_pending_free(void *data, const void *dead_pointer)
{
   EDBus_Connection *conn = data;
   edbus_connection_pending_del(conn, (void *)dead_pointer);
}

EAPI EDBus_Pending *
edbus_connection_send(EDBus_Connection *conn, EDBus_Message *msg, EDBus_Message_Cb cb, const void *cb_data, double timeout)
{
   EDBus_Pending *pending;

   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(msg, NULL);

   pending = _edbus_connection_send(conn, msg, cb, cb_data, timeout);
   if (!cb) return NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(pending, NULL);

   edbus_connection_pending_add(conn, pending);
   edbus_pending_cb_free_add(pending, _on_pending_free, conn);
   return pending;
}

EDBus_Pending *
_edbus_connection_send(EDBus_Connection *conn, EDBus_Message *msg, EDBus_Message_Cb cb, const void *cb_data, double timeout)
{
   EDBus_Pending *pending;
   EDBus_Message *error_msg;
   DBG("conn=%p, msg=%p, cb=%p, cb_data=%p, timeout=%f",
          conn, msg, cb, cb_data, timeout);

   if (!cb)
     {
        dbus_connection_send(conn->dbus_conn, msg->dbus_msg, NULL);
        return NULL;
     }

   pending = calloc(1, sizeof(EDBus_Pending));
   EINA_SAFETY_ON_NULL_RETURN_VAL(pending, NULL);

   pending->cb = cb;
   pending->cb_data = cb_data;
   pending->conn = conn;
   pending->dest = eina_stringshare_add(dbus_message_get_member(msg->dbus_msg));
   pending->interface = eina_stringshare_add(dbus_message_get_interface(msg->dbus_msg));
   pending->method = eina_stringshare_add(dbus_message_get_member(msg->dbus_msg));
   pending->path = eina_stringshare_add(dbus_message_get_path(msg->dbus_msg));
   pending->msg_sent = edbus_message_ref(msg);
   EINA_MAGIC_SET(pending, EDBUS_PENDING_MAGIC);

   if (!dbus_connection_send_with_reply(conn->dbus_conn,
                                        msg->dbus_msg,
                                        &pending->dbus_pending, timeout))
     {
        error_msg = edbus_message_error_new(msg, "org.enlightenment.DBus.NoConnection",
                                            "EDBus_Connection was closed.");
        edbus_pending_dispatch(pending, error_msg);
        return NULL;
     }
   if (dbus_pending_call_set_notify(pending->dbus_pending, cb_pending, pending, NULL))
     return pending;

   dbus_pending_call_cancel(pending->dbus_pending);
   error_msg = edbus_message_error_new(pending->msg_sent,
                                       "org.enlightenment.DBus.Error",
                                       "Error when try set callback to message.");
   edbus_pending_dispatch(pending, error_msg);
   return NULL;
}

EAPI void
edbus_pending_data_set(EDBus_Pending *pending, const char *key, const void *data)
{
   EDBUS_PENDING_CHECK(pending);
   EINA_SAFETY_ON_NULL_RETURN(key);
   EINA_SAFETY_ON_NULL_RETURN(data);
   edbus_data_set(&(pending->data), key, data);
}

EAPI void *
edbus_pending_data_get(const EDBus_Pending *pending, const char *key)
{
   EDBUS_PENDING_CHECK_RETVAL(pending, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key, NULL);
   return edbus_data_get(&(((EDBus_Pending *)pending)->data), key);
}

EAPI void *
edbus_pending_data_del(EDBus_Pending *pending, const char *key)
{
   EDBUS_PENDING_CHECK_RETVAL(pending, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key, NULL);
   return edbus_data_del(&(((EDBus_Pending *)pending)->data), key);
}

static void
edbus_pending_dispatch(EDBus_Pending *pending, EDBus_Message *msg)
{
   DBG("pending=%p msg=%p", pending, msg);
   if (pending->cb)
     pending->cb((void *)pending->cb_data, msg, pending);

   edbus_cbs_free_dispatch(&(pending->cbs_free), pending);
   edbus_data_del_all(&(pending->data));

   if (msg) edbus_message_unref(msg);
   edbus_message_unref(pending->msg_sent);
   dbus_pending_call_unref(pending->dbus_pending);

   pending->cb = NULL;
   pending->dbus_pending = NULL;
   eina_stringshare_del(pending->dest);
   eina_stringshare_del(pending->path);
   eina_stringshare_del(pending->interface);
   eina_stringshare_del(pending->method);
   EINA_MAGIC_SET(pending, EINA_MAGIC_NONE);
   free(pending);
}

EAPI void
edbus_pending_cancel(EDBus_Pending *pending)
{
   EDBus_Message *error_message;
   EDBUS_PENDING_CHECK(pending);
   EINA_SAFETY_ON_NULL_RETURN(pending->dbus_pending);

   DBG("pending=%p", pending);
   dbus_pending_call_cancel(pending->dbus_pending);

   error_message = edbus_message_error_new(pending->msg_sent,
                                           "org.enlightenment.DBus.Canceled",
                                           "Canceled by user.");
   edbus_pending_dispatch(pending, error_message);
}

EAPI void
edbus_pending_cb_free_add(EDBus_Pending *pending, EDBus_Free_Cb cb, const void *data)
{
   EDBUS_PENDING_CHECK(pending);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   pending->cbs_free = edbus_cbs_free_add(pending->cbs_free, cb, data);
}

EAPI void
edbus_pending_cb_free_del(EDBus_Pending *pending, EDBus_Free_Cb cb, const void *data)
{
   EDBUS_PENDING_CHECK(pending);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   pending->cbs_free = edbus_cbs_free_del(pending->cbs_free, cb, data);
}

EAPI const char *
edbus_pending_destination_get(const EDBus_Pending *pending)
{
   EDBUS_PENDING_CHECK_RETVAL(pending, NULL);
   return pending->dest;
}

EAPI const char *
edbus_pending_path_get(const EDBus_Pending *pending)
{
   EDBUS_PENDING_CHECK_RETVAL(pending, NULL);
   return pending->path;
}

EAPI const char *
edbus_pending_interface_get(const EDBus_Pending *pending)
{
   EDBUS_PENDING_CHECK_RETVAL(pending, NULL);
   return pending->interface;
}

EAPI const char *
edbus_pending_method_get(const EDBus_Pending *pending)
{
   EDBUS_PENDING_CHECK_RETVAL(pending, NULL);
   return pending->method;
}

#include "eldbus_private.h"
#include "eldbus_private_types.h"
#include <dbus/dbus.h>

/* TODO: mempool of Eldbus_Pending */
#define ELDBUS_PENDING_CHECK(pending)                        \
  do                                                        \
    {                                                       \
       EINA_SAFETY_ON_NULL_RETURN(pending);                 \
       if (!EINA_MAGIC_CHECK(pending, ELDBUS_PENDING_MAGIC)) \
         {                                                  \
            EINA_MAGIC_FAIL(pending, ELDBUS_PENDING_MAGIC);  \
            return;                                         \
         }                                                  \
    }                                                       \
  while (0)

#define ELDBUS_PENDING_CHECK_RETVAL(pending, retval)         \
  do                                                        \
    {                                                       \
       EINA_SAFETY_ON_NULL_RETURN_VAL(pending, retval);     \
       if (!EINA_MAGIC_CHECK(pending, ELDBUS_PENDING_MAGIC)) \
         {                                                  \
            EINA_MAGIC_FAIL(pending, ELDBUS_PENDING_MAGIC);  \
            return retval;                                  \
         }                                                  \
    }                                                       \
  while (0)

static void eldbus_pending_dispatch(Eldbus_Pending *pending, Eldbus_Message *msg);

Eina_Bool
eldbus_pending_init(void)
{
   return EINA_TRUE;
}

void
eldbus_pending_shutdown(void)
{
}

static void
cb_pending(DBusPendingCall *dbus_pending, void *user_data)
{
   Eldbus_Message *msg;
   Eldbus_Pending *pending = user_data;

   if (!dbus_pending_call_get_completed(dbus_pending))
     {
        INF("timeout to pending %p", pending);
        dbus_pending_call_cancel(dbus_pending);
        msg = eldbus_message_error_new(pending->msg_sent,
                                       ELDBUS_ERROR_PENDING_TIMEOUT,
                                       "This call was not completed in time.");
        eldbus_pending_dispatch(pending, msg);
        return;
     }

   msg = eldbus_message_new(EINA_FALSE);
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
   eldbus_pending_dispatch(pending, msg);

   return;

cleanup:
   eldbus_message_unref(msg);
}

static void
_on_pending_free(void *data, const void *dead_pointer)
{
   Eldbus_Connection *conn = data;
   eldbus_connection_pending_del(conn, (void *)dead_pointer);
}

EAPI Eldbus_Pending *
eldbus_connection_send(Eldbus_Connection *conn, Eldbus_Message *msg, Eldbus_Message_Cb cb, const void *cb_data, double timeout)
{
   Eldbus_Pending *pending;

   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(msg, NULL);

   pending = _eldbus_connection_send(conn, msg, cb, cb_data, timeout);
   if (!cb) return NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(pending, NULL);

   eldbus_connection_pending_add(conn, pending);
   eldbus_pending_free_cb_add(pending, _on_pending_free, conn);
   return pending;
}

/*
 * On success @param msg is unref'd or its ref is stolen by the returned
 * Eldbus_Pending.
 */
Eldbus_Pending *
_eldbus_connection_send(Eldbus_Connection *conn, Eldbus_Message *msg, Eldbus_Message_Cb cb, const void *cb_data, double timeout)
{
   Eldbus_Pending *pending;
   Eldbus_Message *error_msg;
   DBG("conn=%p, msg=%p, cb=%p, cb_data=%p, timeout=%f",
       conn, msg, cb, cb_data, timeout);

   if (!cb)
     {
        dbus_connection_send(conn->dbus_conn, msg->dbus_msg, NULL);
        eldbus_message_unref(msg);
        return NULL;
     }

   pending = calloc(1, sizeof(Eldbus_Pending));
   EINA_SAFETY_ON_NULL_RETURN_VAL(pending, NULL);

   pending->cb = cb;
   pending->cb_data = cb_data;
   pending->conn = conn;
   pending->dest = eina_stringshare_add(dbus_message_get_destination(msg->dbus_msg));
   pending->interface = eina_stringshare_add(dbus_message_get_interface(msg->dbus_msg));
   pending->method = eina_stringshare_add(dbus_message_get_member(msg->dbus_msg));
   pending->path = eina_stringshare_add(dbus_message_get_path(msg->dbus_msg));

   /* Steal the reference */
   pending->msg_sent = msg;

   EINA_MAGIC_SET(pending, ELDBUS_PENDING_MAGIC);

   if (!dbus_connection_send_with_reply(conn->dbus_conn,
                                        msg->dbus_msg,
                                        &pending->dbus_pending, timeout))
     {
        error_msg = eldbus_message_error_new(msg, "org.enlightenment.DBus.NoConnection",
                                            "Eldbus_Connection was closed.");
        eldbus_pending_dispatch(pending, error_msg);
        return NULL;
     }
   if (dbus_pending_call_set_notify(pending->dbus_pending, cb_pending, pending, NULL))
     return pending;

   dbus_pending_call_cancel(pending->dbus_pending);
   error_msg = eldbus_message_error_new(pending->msg_sent,
                                       "org.enlightenment.DBus.Error",
                                       "Error when try set callback to message.");
   eldbus_pending_dispatch(pending, error_msg);
   return NULL;
}

Eldbus_Message *
_eldbus_connection_send_and_block(Eldbus_Connection *conn, Eldbus_Message *msg)
{
   Eldbus_Message *reply;

   if (ecore_main_loop_nested_get())
     WRN("Calling this function may result in dropped frames because the main loop is running");

   reply = eldbus_message_new(EINA_TRUE);
   EINA_SAFETY_ON_NULL_GOTO(reply, fail);

   reply->dbus_msg = 
     dbus_connection_send_with_reply_and_block(conn->dbus_conn, 
                                               msg->dbus_msg, -1, NULL);

   dbus_message_iter_init_append(reply->dbus_msg, 
                                 &reply->iterator->dbus_iterator);

   return reply;

fail:
   eldbus_message_unref(reply);
   return NULL;
}

EAPI void
eldbus_pending_data_set(Eldbus_Pending *pending, const char *key, const void *data)
{
   ELDBUS_PENDING_CHECK(pending);
   EINA_SAFETY_ON_NULL_RETURN(key);
   EINA_SAFETY_ON_NULL_RETURN(data);
   eldbus_data_set(&(pending->data), key, data);
}

EAPI void *
eldbus_pending_data_get(const Eldbus_Pending *pending, const char *key)
{
   ELDBUS_PENDING_CHECK_RETVAL(pending, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key, NULL);
   return eldbus_data_get(&(((Eldbus_Pending *)pending)->data), key);
}

EAPI void *
eldbus_pending_data_del(Eldbus_Pending *pending, const char *key)
{
   ELDBUS_PENDING_CHECK_RETVAL(pending, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key, NULL);
   return eldbus_data_del(&(((Eldbus_Pending *)pending)->data), key);
}

static void
eldbus_pending_dispatch(Eldbus_Pending *pending, Eldbus_Message *msg)
{
   DBG("pending=%p msg=%p", pending, msg);
   if (pending->cb)
     pending->cb((void *)pending->cb_data, msg, pending);

   eldbus_cbs_free_dispatch(&(pending->cbs_free), pending);
   eldbus_data_del_all(&(pending->data));

   if (msg) eldbus_message_unref(msg);
   eldbus_message_unref(pending->msg_sent);
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
eldbus_pending_cancel(Eldbus_Pending *pending)
{
   Eldbus_Message *error_message;
   ELDBUS_PENDING_CHECK(pending);
   EINA_SAFETY_ON_NULL_RETURN(pending->dbus_pending);

   DBG("pending=%p", pending);
   dbus_pending_call_cancel(pending->dbus_pending);

   error_message = eldbus_message_error_new(pending->msg_sent,
                                            ELDBUS_ERROR_PENDING_CANCELED,
                                            "Canceled by user.");
   eldbus_pending_dispatch(pending, error_message);
}

EAPI void
eldbus_pending_free_cb_add(Eldbus_Pending *pending, Eldbus_Free_Cb cb, const void *data)
{
   ELDBUS_PENDING_CHECK(pending);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   pending->cbs_free = eldbus_cbs_free_add(pending->cbs_free, cb, data);
}

EAPI void
eldbus_pending_free_cb_del(Eldbus_Pending *pending, Eldbus_Free_Cb cb, const void *data)
{
   ELDBUS_PENDING_CHECK(pending);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   pending->cbs_free = eldbus_cbs_free_del(pending->cbs_free, cb, data);
}

EAPI const char *
eldbus_pending_destination_get(const Eldbus_Pending *pending)
{
   ELDBUS_PENDING_CHECK_RETVAL(pending, NULL);
   return pending->dest;
}

EAPI const char *
eldbus_pending_path_get(const Eldbus_Pending *pending)
{
   ELDBUS_PENDING_CHECK_RETVAL(pending, NULL);
   return pending->path;
}

EAPI const char *
eldbus_pending_interface_get(const Eldbus_Pending *pending)
{
   ELDBUS_PENDING_CHECK_RETVAL(pending, NULL);
   return pending->interface;
}

EAPI const char *
eldbus_pending_method_get(const Eldbus_Pending *pending)
{
   ELDBUS_PENDING_CHECK_RETVAL(pending, NULL);
   return pending->method;
}

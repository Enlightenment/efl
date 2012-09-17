#include "edbus_private.h"
#include "edbus_private_types.h"
#include <dbus/dbus.h>

/* TODO: mempool of EDBus_Signal_Handler */

#define SENDER_KEY "sender"
#define PATH_KEY "path"
#define INTERFACE_KEY "interface"
#define MEMBER_KEY "member"
#define ARG_X_KEY "arg%u"

#define EDBUS_SIGNAL_HANDLER_CHECK(handler)                        \
  do                                                               \
    {                                                              \
       EINA_SAFETY_ON_NULL_RETURN(handler);                        \
       if (!EINA_MAGIC_CHECK(handler, EDBUS_SIGNAL_HANDLER_MAGIC)) \
         {                                                         \
            EINA_MAGIC_FAIL(handler, EDBUS_SIGNAL_HANDLER_MAGIC);  \
            return;                                                \
         }                                                         \
    }                                                              \
  while (0)

#define EDBUS_SIGNAL_HANDLER_CHECK_RETVAL(handler, retval)         \
  do                                                               \
    {                                                              \
       EINA_SAFETY_ON_NULL_RETURN_VAL(handler, retval);            \
       if (!EINA_MAGIC_CHECK(handler, EDBUS_SIGNAL_HANDLER_MAGIC)) \
         {                                                         \
            EINA_MAGIC_FAIL(handler, EDBUS_SIGNAL_HANDLER_MAGIC);  \
            return retval;                                         \
         }                                                         \
    }                                                              \
  while (0)

static void _edbus_signal_handler_del(EDBus_Signal_Handler *handler);
static void _edbus_signal_handler_clean(EDBus_Signal_Handler *handler);

Eina_Bool
edbus_signal_handler_init(void)
{
   return EINA_TRUE;
}

void
edbus_signal_handler_shutdown(void)
{
}

static void
_match_append(Eina_Strbuf *match, const char *key, const char *value)
{
   if (value == NULL || !value[0]) return;

   if ((eina_strbuf_length_get(match) + strlen(",=''") + strlen(key) + strlen(value))
       >= DBUS_MAXIMUM_MATCH_RULE_LENGTH)
     {
        ERR("cannot add match %s='%s' to %s: too long!", key, value,
            eina_strbuf_string_get(match));
        return;
     }

   eina_strbuf_append_printf(match, ",%s='%s'", key, value);
}

#define ARGX "arg"

static int
_sort_arg(const void *d1, const void *d2)
{
   const Signal_Argument *arg1, *arg2;
   arg1 = d1;
   arg2 = d2;
   return arg1->index - arg2->index;
}

EAPI Eina_Bool
edbus_signal_handler_match_extra_set(EDBus_Signal_Handler *sh, ...)
{
   va_list ap;
   char *key = NULL, *value;
   Signal_Argument *arg;
   DBusError err;

   EDBUS_SIGNAL_HANDLER_CHECK_RETVAL(sh, EINA_FALSE);

   dbus_error_init(&err);
   dbus_bus_remove_match(sh->conn->dbus_conn,
                         eina_strbuf_string_get(sh->match), &err);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(dbus_error_is_set(&err), EINA_FALSE);

   va_start(ap, sh);
   do
     {
        if (!key)
          {
             key = va_arg(ap, char *);
             continue;
          }
        value = va_arg(ap, char *);
        arg = calloc(1, sizeof(Signal_Argument));
        EINA_SAFETY_ON_NULL_GOTO(arg, error);

        if (!strncmp(key, ARGX, strlen(ARGX)))
          {
             int id = atoi(key+strlen(ARGX)-1);
             arg->index = (unsigned short)id;
             arg->value = eina_stringshare_add(value);
             sh->args = eina_inlist_sorted_state_insert(sh->args,
                                                        EINA_INLIST_GET(arg),
                                                        _sort_arg,
                                                        sh->state_args);
             _match_append(sh->match, key, value);
          }
        key = NULL;
     } while(key);
   va_end(ap);

   dbus_error_init(&err);
   dbus_bus_add_match(sh->conn->dbus_conn,
                      eina_strbuf_string_get(sh->match), &err);
   if (!dbus_error_is_set(&err))
     return EINA_TRUE;

   ERR("Error setting new match.");
   return EINA_FALSE;

error:
   va_end(ap);
   dbus_error_init(&err);
   dbus_bus_add_match(sh->conn->dbus_conn,
                      eina_strbuf_string_get(sh->match), &err);
   if (dbus_error_is_set(&err))
     ERR("Error setting partial extra arguments.");
   return EINA_FALSE;
}

EAPI EDBus_Signal_Handler *
edbus_signal_handler_add(EDBus_Connection *conn, const char *sender, const char *path, const char *interface, const char *member, EDBus_Signal_Cb cb, const void *cb_data)
{
   EDBus_Signal_Handler *sh;
   Eina_Strbuf *match;
   DBusError err;

   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cb, NULL);
   DBG("conn=%p, sender=%s, path=%s, interface=%s, member=%s, cb=%p %p",
       conn, sender, path, interface, member, cb, cb_data);

   sh = calloc(1, sizeof(EDBus_Signal_Handler));
   EINA_SAFETY_ON_NULL_RETURN_VAL(sh, NULL);

   match = eina_strbuf_new();
   EINA_SAFETY_ON_NULL_GOTO(match, cleanup_create_strbuf);
   eina_strbuf_append(match, "type='signal'");
   _match_append(match, SENDER_KEY, sender);
   _match_append(match, PATH_KEY, path);
   _match_append(match, INTERFACE_KEY, interface);
   _match_append(match, MEMBER_KEY, member);

   dbus_error_init(&err);
   dbus_bus_add_match(conn->dbus_conn, eina_strbuf_string_get(match), &err);
   if (dbus_error_is_set(&err)) goto cleanup;

   if (sender && sender[0] != ':' && strcmp(sender, EDBUS_FDO_BUS))
     {
        sh->bus = edbus_connection_name_get(conn, sender);
        if (!sh->bus) goto cleanup;
        edbus_connection_name_owner_monitor(conn, sh->bus, EINA_TRUE);
     }

   sh->cb = cb;
   sh->cb_data = cb_data;
   sh->conn = conn;
   sh->interface = eina_stringshare_add(interface);
   sh->member = eina_stringshare_add(member);
   sh->path = eina_stringshare_add(path);
   sh->sender = eina_stringshare_add(sender);
   sh->match = match;
   sh->refcount = 1;
   sh->dangling = EINA_FALSE;
   sh->state_args = eina_inlist_sorted_state_new();
   EINA_MAGIC_SET(sh, EDBUS_SIGNAL_HANDLER_MAGIC);

   edbus_connection_signal_handler_add(conn, sh);
   return sh;

cleanup:
   eina_strbuf_free(match);
cleanup_create_strbuf:
   free(sh);

   return NULL;
}

static Eina_Bool
signal_handler_deleter(void *data)
{
   EDBus_Signal_Handler *handler = data;
   _edbus_signal_handler_del(handler);
   return ECORE_CALLBACK_CANCEL;
}

static void
_edbus_signal_handler_clean(EDBus_Signal_Handler *handler)
{
   DBusError err;

   if (handler->dangling) return;

   edbus_connection_signal_handler_del(handler->conn, handler);
   if (handler->bus)
     edbus_connection_name_owner_monitor(handler->conn, handler->bus,
                                         EINA_FALSE);
   dbus_error_init(&err);
   dbus_bus_remove_match(handler->conn->dbus_conn,
                         eina_strbuf_string_get(handler->match), &err);
   handler->dangling = EINA_TRUE;
}

static void
_edbus_signal_handler_del(EDBus_Signal_Handler *handler)
{
   Eina_Inlist *list;
   Signal_Argument *arg;
   DBG("handler %p, refcount=%d, conn=%p %s",
       handler, handler->refcount, handler->conn, handler->sender);
   edbus_cbs_free_dispatch(&(handler->cbs_free), handler);
   EINA_MAGIC_SET(handler, EINA_MAGIC_NONE);

   /* after cbs_free dispatch these shouldn't exit, error if they do */

   eina_stringshare_del(handler->sender);
   eina_stringshare_del(handler->path);
   eina_stringshare_del(handler->interface);
   eina_stringshare_del(handler->member);
   eina_strbuf_free(handler->match);
   EINA_INLIST_FOREACH_SAFE(handler->args, list, arg)
     {
        eina_stringshare_del(arg->value);
        free(arg);
     }
   eina_inlist_sorted_state_free(handler->state_args);
   free(handler);
}

EAPI EDBus_Signal_Handler *
edbus_signal_handler_ref(EDBus_Signal_Handler *handler)
{
   EDBUS_SIGNAL_HANDLER_CHECK_RETVAL(handler, NULL);
   DBG("handler=%p, pre-refcount=%d, match=%s",
       handler, handler->refcount, eina_strbuf_string_get(handler->match));
   handler->refcount++;
   return handler;
}

EAPI void
edbus_signal_handler_unref(EDBus_Signal_Handler *handler)
{
   EDBUS_SIGNAL_HANDLER_CHECK(handler);
   DBG("handler=%p, pre-refcount=%d, match=%s",
       handler, handler->refcount, eina_strbuf_string_get(handler->match));
   handler->refcount--;
   if (handler->refcount > 0) return;

   _edbus_signal_handler_clean(handler);

   if (handler->conn->running_signal)
     ecore_idler_add(signal_handler_deleter, handler);
   else
     _edbus_signal_handler_del(handler);
}

EAPI void
edbus_signal_handler_del(EDBus_Signal_Handler *handler)
{
   EDBUS_SIGNAL_HANDLER_CHECK(handler);
   _edbus_signal_handler_clean(handler);
   edbus_signal_handler_unref(handler);
}

EAPI void
edbus_signal_handler_cb_free_add(EDBus_Signal_Handler *handler, EDBus_Free_Cb cb, const void *data)
{
   EDBUS_SIGNAL_HANDLER_CHECK(handler);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   handler->cbs_free = edbus_cbs_free_add(handler->cbs_free, cb, data);
}

EAPI void
edbus_signal_handler_cb_free_del(EDBus_Signal_Handler *handler, EDBus_Free_Cb cb, const void *data)
{
   EDBUS_SIGNAL_HANDLER_CHECK(handler);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   handler->cbs_free = edbus_cbs_free_del(handler->cbs_free, cb, data);
}

EAPI const char *
edbus_signal_handler_sender_get(const EDBus_Signal_Handler *handler)
{
   EDBUS_SIGNAL_HANDLER_CHECK_RETVAL(handler, NULL);
   return handler->sender;
}

EAPI const char *
edbus_signal_handler_path_get(const EDBus_Signal_Handler *handler)
{
   EDBUS_SIGNAL_HANDLER_CHECK_RETVAL(handler, NULL);
   return handler->path;
}

EAPI const char *
edbus_signal_handler_interface_get(const EDBus_Signal_Handler *handler)
{
   EDBUS_SIGNAL_HANDLER_CHECK_RETVAL(handler, NULL);
   return handler->interface;
}

EAPI const char *
edbus_signal_handler_member_get(const EDBus_Signal_Handler *handler)
{
   EDBUS_SIGNAL_HANDLER_CHECK_RETVAL(handler, NULL);
   return handler->member;
}

EAPI const char *
edbus_signal_handler_match_get(const EDBus_Signal_Handler *handler)
{
   EDBUS_SIGNAL_HANDLER_CHECK_RETVAL(handler, NULL);
   return eina_strbuf_string_get(handler->match);
}

EAPI EDBus_Connection *
edbus_signal_handler_connection_get(const EDBus_Signal_Handler *handler)
{
   EDBUS_SIGNAL_HANDLER_CHECK_RETVAL(handler, NULL);
   return handler->conn;
}


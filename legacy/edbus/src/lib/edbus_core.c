#include "edbus_private.h"
#include "edbus_private_types.h"
#include <dbus/dbus.h>

#include <Ecore.h>

#define EDBUS_CONNECTION_CHECK(conn)                        \
  do                                                        \
    {                                                       \
       EINA_SAFETY_ON_NULL_RETURN(conn);                    \
       if (!EINA_MAGIC_CHECK(conn, EDBUS_CONNECTION_MAGIC)) \
         {                                                  \
            EINA_MAGIC_FAIL(conn, EDBUS_CONNECTION_MAGIC);  \
            return;                                         \
         }                                                  \
       EINA_SAFETY_ON_TRUE_RETURN(conn->refcount <= 0);     \
    }                                                       \
  while (0)

#define EDBUS_CONNECTION_CHECK_RETVAL(conn, retval)                 \
  do                                                                \
    {                                                               \
       EINA_SAFETY_ON_NULL_RETURN_VAL(conn, retval);                \
       if (!EINA_MAGIC_CHECK(conn, EDBUS_CONNECTION_MAGIC))         \
         {                                                          \
            EINA_MAGIC_FAIL(conn, EDBUS_CONNECTION_MAGIC);          \
            return retval;                                          \
         }                                                          \
       EINA_SAFETY_ON_TRUE_RETURN_VAL(conn->refcount <= 0, retval); \
    }                                                               \
  while (0)

typedef struct _EDBus_Connection_Context_Event_Cb
{
   EINA_INLIST;
   EDBus_Connection_Event_Cb cb;
   const void               *cb_data;
   Eina_Bool                 deleted : 1;
} EDBus_Connection_Context_Event_Cb;

typedef struct _EDBus_Connection_Context_NOC_Cb
{
   EINA_INLIST;
   EDBus_Name_Owner_Changed_Cb cb;
   const void                 *cb_data;
   Eina_Bool                   deleted : 1;
   Ecore_Idler                *idler;
   Eina_Bool                   allow_initial : 1;
} EDBus_Connection_Context_NOC_Cb;

typedef struct _EDBus_Handler_Data
{
   EINA_INLIST;
   int               fd;
   Ecore_Fd_Handler *fd_handler;
   EDBus_Connection *conn;
   DBusWatch        *watch;
   int               enabled;
} EDBus_Handler_Data;

typedef struct _EDBus_Timeout_Data
{
   EINA_INLIST;
   Ecore_Timer      *handler;
   DBusTimeout      *timeout;
   EDBus_Connection *conn;
   int               interval;
} EDBus_Timeout_Data;

static const EDBus_Version _version = {VMAJ, VMIN, VMIC, VREV};
EAPI const EDBus_Version * edbus_version = &_version;

static int _edbus_init_count = 0;
int _edbus_log_dom = -1;

static EDBus_Connection *shared_connections[3];

static void _edbus_connection_event_callback_call(EDBus_Connection *conn, EDBus_Connection_Event_Type type, const void *event_info);
static void _edbus_connection_context_event_cb_del(EDBus_Connection_Context_Event *ce, EDBus_Connection_Context_Event_Cb *ctx);
static void edbus_dispatch_name_owner_change(EDBus_Connection_Name *cn, const char *old_id);

EAPI int
edbus_init(void)
{
   if (_edbus_init_count > 0)
     return ++_edbus_init_count;

   if (!eina_init())
     {
        fputs("EDBus: Enable to initialize eina\n", stderr);
        return 0;
     }

   _edbus_log_dom = eina_log_domain_register("edbus", EINA_COLOR_BLUE);
   if (_edbus_log_dom < 0)
     {
        EINA_LOG_ERR("Unable to create an 'edbus' log domain");
        eina_shutdown();
        return 0;
     }

   if (!ecore_init())
     {
        ERR("Unable to initialize ecore");
        eina_log_domain_unregister(_edbus_log_dom);
        _edbus_log_dom = -1;
        eina_shutdown();
        return 0;
     }

   eina_magic_string_set(EDBUS_CONNECTION_MAGIC, "EDBus_Connection");
   eina_magic_string_set(EDBUS_MESSAGE_MAGIC, "EDBus_Message");
   eina_magic_string_set(EDBUS_SIGNAL_HANDLER_MAGIC, "EDBus_Signal_Handler");
   eina_magic_string_set(EDBUS_PENDING_MAGIC, "EDBus_Pending");
   eina_magic_string_set(EDBUS_OBJECT_MAGIC, "EDBus_Object");
   eina_magic_string_set(EDBUS_PROXY_MAGIC, "EDBus_Proxy");
   eina_magic_string_set(EDBUS_MESSAGE_ITERATOR_MAGIC, "EDBus_Message_Iterator");
   eina_magic_string_set(EDBUS_SERVICE_INTERFACE_MAGIC, "EDBus_Service_Interface");


   if (!edbus_message_init()) goto message_failed;
   if (!edbus_signal_handler_init()) goto signal_handler_failed;
   if (!edbus_pending_init()) goto pending_failed;
   if (!edbus_object_init()) goto object_failed;
   if (!edbus_proxy_init()) goto proxy_failed;
   if (!edbus_service_init()) goto service_failed;

   return ++_edbus_init_count;

service_failed:
   edbus_proxy_shutdown();
proxy_failed:
   edbus_object_shutdown();
object_failed:
   edbus_pending_shutdown();
pending_failed:
   edbus_signal_handler_shutdown();
signal_handler_failed:
   edbus_message_shutdown();
message_failed:
   ecore_shutdown();
   eina_log_domain_unregister(_edbus_log_dom);
   _edbus_log_dom = -1;
   eina_shutdown();

   return 0;
}

static void
print_live_connection(EDBus_Connection *conn)
{
   if (!conn->names)
     ERR("conn=%p has no alive objects", conn);
   else
     {
        Eina_Iterator *iter = eina_hash_iterator_data_new(conn->names);
        EDBus_Connection_Name *name;
        EINA_ITERATOR_FOREACH(iter, name)
          {
             EDBus_Object *obj;
             Eina_Iterator *inner_itr;
             if (!name->objects) continue;

             inner_itr = eina_hash_iterator_data_new(name->objects);
             EINA_ITERATOR_FOREACH(inner_itr, obj)
                ERR("conn=%p alive object=%p %s of bus=%s", conn, obj,
                    obj->name, name->name);
             eina_iterator_free(inner_itr);
          }
        eina_iterator_free(iter);
     }

   if (!conn->pendings)
     ERR("conn=%p has no alive pending calls", conn);
   else
     {
        EDBus_Pending *p;
        EINA_INLIST_FOREACH(conn->pendings, p)
          ERR("conn=%p alive pending call=%p dest=%s path=%s %s.%s()",
              conn, p,
              edbus_pending_destination_get(p),
              edbus_pending_path_get(p),
              edbus_pending_interface_get(p),
              edbus_pending_method_get(p));
     }
}

EAPI int
edbus_shutdown(void)
{
   if (_edbus_init_count <= 0)
     {
        ERR("Init count not greater than 0 in shutdown.");
        _edbus_init_count = 0;
        return 0;
     }
   if (--_edbus_init_count)
     return _edbus_init_count;

   if (shared_connections[EDBUS_CONNECTION_TYPE_SESSION - 1])
     {
        CRITICAL("Alive TYPE_SESSION connection");
        print_live_connection(shared_connections[EDBUS_CONNECTION_TYPE_SESSION - 1]);
     }
   if (shared_connections[EDBUS_CONNECTION_TYPE_SYSTEM - 1])
     {
        CRITICAL("Alive TYPE_SYSTEM connection");
        print_live_connection(shared_connections[EDBUS_CONNECTION_TYPE_SYSTEM - 1]);
     }
   if (shared_connections[EDBUS_CONNECTION_TYPE_STARTER - 1])
     {
        CRITICAL("Alive TYPE_STARTER connection");
        print_live_connection(shared_connections[EDBUS_CONNECTION_TYPE_STARTER - 1]);
     }

   edbus_service_shutdown();
   edbus_proxy_shutdown();
   edbus_object_shutdown();
   edbus_pending_shutdown();
   edbus_signal_handler_shutdown();
   edbus_message_shutdown();

   ecore_shutdown();
   eina_log_domain_unregister(_edbus_log_dom);
   _edbus_log_dom = -1;
   eina_shutdown();

   return 0;
}

/* TODO: mempool of EDBus_Context_Free_Cb */
typedef struct _EDBus_Context_Free_Cb
{
   EINA_INLIST;
   EDBus_Free_Cb cb;
   const void   *data;
} EDBus_Context_Free_Cb;

void
edbus_cbs_free_dispatch(Eina_Inlist **p_lst, const void *dead_pointer)
{
   Eina_Inlist *lst = *p_lst;
   *p_lst = NULL;
   while (lst)
     {
        Eina_Inlist *next = lst->next;
        EDBus_Context_Free_Cb *ctx;

        ctx = EINA_INLIST_CONTAINER_GET(lst, EDBus_Context_Free_Cb);
        ctx->cb((void *)ctx->data, dead_pointer);
        free(ctx);

        lst = next;
     }
}

Eina_Inlist *
edbus_cbs_free_add(Eina_Inlist *lst, EDBus_Free_Cb cb, const void *data)
{
   EDBus_Context_Free_Cb *ctx = malloc(sizeof(EDBus_Context_Free_Cb));
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, lst);

   ctx->cb = cb;
   ctx->data = data;

   return eina_inlist_append(lst, EINA_INLIST_GET(ctx));
}

Eina_Inlist *
edbus_cbs_free_del(Eina_Inlist *lst, EDBus_Free_Cb cb, const void *data)
{
   EDBus_Context_Free_Cb *ctx;

   EINA_INLIST_FOREACH(lst, ctx)
     {
        if (ctx->cb != cb) continue;
        if ((data) && (ctx->data != data)) continue;

        lst = eina_inlist_remove(lst, EINA_INLIST_GET(ctx));
        free(ctx);
        return lst;
     }

   ERR("Couldn't find cb_free=%p data=%p", cb, data);
   return lst;
}

typedef struct _EDBus_Data
{
   EINA_INLIST;
   const void  *data;
   unsigned int keylen;
   char         key[];
} EDBus_Data;

static inline EDBus_Data *
edbus_data_find(Eina_Inlist **p_lst, const char *key)
{
   unsigned int keylen = strlen(key);
   EDBus_Data *d;

   EINA_INLIST_FOREACH(*p_lst, d)
     {
        if ((keylen == d->keylen) && (memcmp(key, d->key, keylen) == 0))
          {
             *p_lst = eina_inlist_promote(*p_lst, EINA_INLIST_GET(d));
             return d;
          }
     }

   return NULL;
}

void
edbus_data_set(Eina_Inlist **p_lst, const char *key, const void *data)
{
   EDBus_Data *d = edbus_data_find(p_lst, key);
   unsigned int keylen = strlen(key);

   if (d)
     {
        *p_lst = eina_inlist_remove(*p_lst, EINA_INLIST_GET(d));
        free(d);
     }

   d = malloc(sizeof(EDBus_Data) + keylen + 1);
   EINA_SAFETY_ON_NULL_RETURN(d);

   d->data = data;
   d->keylen = keylen;
   memcpy(d->key, key, keylen + 1);

   *p_lst = eina_inlist_prepend(*p_lst, EINA_INLIST_GET(d));
}

void *
edbus_data_get(Eina_Inlist **p_lst, const char *key)
{
   EDBus_Data *d = edbus_data_find(p_lst, key);
   return d ? (void *)d->data : NULL;
}

void *
edbus_data_del(Eina_Inlist **p_lst, const char *key)
{
   EDBus_Data *d = edbus_data_find(p_lst, key);
   void *ret;
   if (!d) return NULL;

   ret = (void *)d->data;
   *p_lst = eina_inlist_remove(*p_lst, EINA_INLIST_GET(d));
   free(d);

   return ret;
}

void
edbus_data_del_all(Eina_Inlist **p_list)
{
   Eina_Inlist *n = *p_list;
   *p_list = NULL;

   while (n)
     {
        EDBus_Data *d = EINA_INLIST_CONTAINER_GET(n, EDBus_Data);
        n = eina_inlist_remove(n, n);
        DBG("key=%s, data=%p", d->key, d->data);
        free(d);
     }
}

static EDBus_Connection_Name *
edbus_connection_name_new(const char *name)
{
   EDBus_Connection_Name *cn = calloc(1, sizeof(EDBus_Connection_Name));
   EINA_SAFETY_ON_NULL_RETURN_VAL(cn, NULL);
   cn->name = eina_stringshare_add(name);
   return cn;
}

static void
edbus_connection_name_free(void *data)
{
   EDBus_Connection_Name *cn = data;

   eina_stringshare_del(cn->name);
   eina_stringshare_del(cn->unique_id);

   if (cn->objects) eina_hash_free(cn->objects);

   while (cn->event_handlers.list)
     {
        EDBus_Connection_Context_NOC_Cb *ctx;
        ctx = EINA_INLIST_CONTAINER_GET(cn->event_handlers.list,
                                        EDBus_Connection_Context_NOC_Cb);
        cn->event_handlers.list = eina_inlist_remove(cn->event_handlers.list,
                                                     cn->event_handlers.list);
        free(ctx);
     }
   eina_list_free(cn->event_handlers.to_delete);

   free(cn);
}

static void
edbus_connection_name_gc(EDBus_Connection *conn, EDBus_Connection_Name *cn)
{
   Eina_Bool no_objs;
   Eina_Bool no_event_handlers;

   no_objs = ((!cn->objects) || (eina_hash_population(cn->objects) == 0));
   no_event_handlers = !!cn->event_handlers.list;

   if (no_objs && no_event_handlers && cn->refcount < 1)
     eina_hash_del(conn->names, cn->name, cn);
}

void
edbus_connection_name_object_del(EDBus_Connection *conn, const EDBus_Object *obj)
{
   EDBus_Connection_Name *cn = eina_hash_find(conn->names, obj->name);
   const EDBus_Connection_Event_Object_Removed ev = {
      obj->path
   };

   if (!cn) return;
   if (!cn->objects) return;
   eina_hash_del(cn->objects, obj->path, obj);

   _edbus_connection_event_callback_call
     (conn, EDBUS_CONNECTION_EVENT_OBJECT_REMOVED, &ev);

   edbus_connection_name_gc(conn, cn);
}

void
edbus_connection_name_object_set(EDBus_Connection *conn, EDBus_Object *obj)
{
   EDBus_Connection_Name *cn = eina_hash_find(conn->names, obj->name);
   Eina_Bool had_connection_name = !!cn;
   const EDBus_Connection_Event_Object_Added ev = {
      obj->path,
      obj
   };

   if (!cn)
     {
        cn = edbus_connection_name_new(obj->name);
        EINA_SAFETY_ON_NULL_RETURN(cn);
        eina_hash_direct_add(conn->names, cn->name, cn);
     }
   if (!cn->objects)
     {
        cn->objects = eina_hash_string_superfast_new(NULL);
        EINA_SAFETY_ON_NULL_GOTO(cn->objects, cleanup);
     }
   eina_hash_add(cn->objects, obj->path, obj);

   _edbus_connection_event_callback_call
     (conn, EDBUS_CONNECTION_EVENT_OBJECT_ADDED, &ev);

   return;

cleanup:
   if (!had_connection_name)
     {
        eina_hash_del(conn->names, cn->name, cn);
        edbus_connection_name_free(cn);
     }
}

static void
on_name_owner_changed(void *data, const EDBus_Message *msg)
{
   char *bus, *older_id, *new_id;
   const char *name, *text;
   EDBus_Connection_Name *cn = data;

   if (edbus_message_error_get(msg, &name, &text))
     ERR("NameOwnerChanged cn=%s name=%s text=%s",
         cn->name, name, text);
   if (!edbus_message_arguments_get(msg, "sss", &bus, &older_id, &new_id))
     ERR("Error getting arguments from NameOwnerChanged cn=%s", cn->name);

   eina_stringshare_del(cn->unique_id);
   cn->unique_id = eina_stringshare_add(new_id);
   edbus_dispatch_name_owner_change(cn, older_id);
}

static void
on_get_name_owner(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   const char *unique_id = "";
   EDBus_Connection_Name *cn = data;

   if (edbus_message_error_get(msg, NULL, NULL))
     DBG("GetNameOwner returned an error");
   else if (!edbus_message_arguments_get(msg, "s", &unique_id))
     ERR("Error getting arguments from GetNameOwner");

   cn->unique_id = eina_stringshare_add(unique_id);
   edbus_dispatch_name_owner_change(cn, NULL);
}

static void
_edbus_connection_name_ref(EDBus_Connection *conn, EDBus_Connection_Name *cn)
{
   cn->refcount++;
}

static void
_edbus_connection_name_unref(EDBus_Connection *conn, EDBus_Connection_Name *cn)
{
   EDBUS_CONNECTION_CHECK(conn);
   EINA_SAFETY_ON_NULL_RETURN(cn);

   cn->refcount--;

   if (cn->refcount > 0) return;
   if (cn->name_owner_changed)
     edbus_signal_handler_del(cn->name_owner_changed);
   cn->name_owner_changed = NULL;
   edbus_connection_name_gc(conn, cn);
}

void
edbus_connection_name_owner_monitor(EDBus_Connection *conn, EDBus_Connection_Name *cn, Eina_Bool enable)
{
   EDBUS_CONNECTION_CHECK(conn);
   EINA_SAFETY_ON_NULL_RETURN(cn);

   if (!enable)
     {
        _edbus_connection_name_unref(conn, cn);
        return;
     }
   if (cn->name_owner_changed)
     {
        _edbus_connection_name_ref(conn, cn);
        return;
     }

   edbus_name_owner_get(conn, cn->name, on_get_name_owner, cn);
   _edbus_connection_name_ref(conn, cn);
   cn->name_owner_changed = edbus_signal_handler_add(conn,
                                              EDBUS_FDO_BUS,
                                              EDBUS_FDO_PATH,
                                              EDBUS_FDO_INTERFACE,
                                              "NameOwnerChanged",
                                              on_name_owner_changed, cn);
   edbus_signal_handler_match_extra_set(cn->name_owner_changed, "arg0",
                                        cn->name, NULL);
}

EDBus_Connection_Name *
edbus_connection_name_get(EDBus_Connection *conn, const char *name)
{
   EDBus_Connection_Name *cn;
   EDBUS_CONNECTION_CHECK_RETVAL(conn, NULL);

   cn = eina_hash_find(conn->names, name);
   if (cn) return cn;

   cn = edbus_connection_name_new(name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cn, NULL);
   eina_hash_direct_add(conn->names, cn->name, cn);
   return cn;
}

EDBus_Object *
edbus_connection_name_object_get(EDBus_Connection *conn, const char *name, const char *path)
{
   EDBus_Connection_Name *cn;

   EDBUS_CONNECTION_CHECK_RETVAL(conn, NULL);
   cn = eina_hash_find(conn->names, name);
   if (!cn) return NULL;
   if (!cn->objects) return NULL;
   return eina_hash_find(cn->objects, path);
}


static void
edbus_fd_handler_del(EDBus_Handler_Data *hd)
{
   if (!hd->fd_handler) return;

   DBG("free EDBus_Handler_Data %d", hd->fd);
   hd->conn->fd_handlers = eina_inlist_remove(hd->conn->fd_handlers,
                                              EINA_INLIST_GET(hd));
   if (hd->fd_handler)
     ecore_main_fd_handler_del(hd->fd_handler);

   free(hd);
}

static Eina_Bool
edbus_fd_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   EDBus_Handler_Data *hd = data;
   unsigned int condition = 0;

   DBG("Got Ecore_Fd_Handle@%p", fd_handler);

   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ)) condition |= DBUS_WATCH_READABLE;
   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_WRITE)) condition |= DBUS_WATCH_WRITABLE;
   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_ERROR)) condition |= DBUS_WATCH_ERROR;

   DBG("dbus connection@%p fdh=%d flags: [%s%s%s]", hd->conn, hd->fd,
       (condition & DBUS_WATCH_READABLE) ? "read " : "",
       (condition & DBUS_WATCH_WRITABLE) ? "write " : "",
       (condition & DBUS_WATCH_ERROR) ? "error" : "");

   dbus_watch_handle(hd->watch, condition);

   return ECORE_CALLBACK_RENEW;
}

static void
edbus_fd_handler_add(EDBus_Handler_Data *hd)
{
   unsigned int dflags;
   Ecore_Fd_Handler_Flags eflags;

   if (hd->fd_handler) return;
   dflags = dbus_watch_get_flags(hd->watch);
   eflags = ECORE_FD_ERROR;
   if (dflags & DBUS_WATCH_READABLE) eflags |= ECORE_FD_READ;
   if (dflags & DBUS_WATCH_WRITABLE) eflags |= ECORE_FD_WRITE;

   DBG("Watching fd %d with flags: [%s%serror]", hd->fd,
      (eflags & ECORE_FD_READ) ? "read " : "",
      (eflags & ECORE_FD_WRITE) ? "write " : "");

   hd->fd_handler = ecore_main_fd_handler_add(hd->fd,
                                              eflags,
                                              edbus_fd_handler,
                                              hd,
                                              NULL,
                                              NULL);
}

static void
edbus_handler_data_free(void *data)
{
   EDBus_Handler_Data *hd = data;
   edbus_fd_handler_del(hd);
}

static dbus_bool_t
cb_watch_add(DBusWatch *watch, void *data)
{
   EDBus_Connection *conn = data;
   EDBus_Handler_Data *hd;

   if (!dbus_watch_get_enabled(watch)) return EINA_TRUE;

   DBG("cb_watch_add (enabled: %d)", dbus_watch_get_unix_fd(watch));

   hd = calloc(1, sizeof(EDBus_Handler_Data));
   EINA_SAFETY_ON_NULL_RETURN_VAL(hd, EINA_FALSE);
   dbus_watch_set_data(watch, hd, edbus_handler_data_free);
   hd->conn = conn;
   hd->watch = watch;
   hd->enabled = dbus_watch_get_enabled(watch);
   hd->fd = dbus_watch_get_unix_fd(hd->watch);

   conn->fd_handlers = eina_inlist_append(hd->conn->fd_handlers,
                                          EINA_INLIST_GET(hd));
   edbus_fd_handler_add(hd);

   return EINA_TRUE;
}

static void
cb_watch_del(DBusWatch *watch, void *data)
{
   DBG("cb_watch_del");
   /* will trigger edbus_handler_data_free() */
   dbus_watch_set_data(watch, NULL, NULL);
}

static void
cb_watch_toggle(DBusWatch *watch, void *data)
{
   EDBus_Handler_Data *hd;
   hd = dbus_watch_get_data(watch);
   DBG("cb_watch_toggle %d", hd->fd);

   hd->enabled = dbus_watch_get_enabled(watch);

   DBG("watch %p is %sabled", hd, hd->enabled ? "en" : "dis");
   if (hd->enabled) edbus_fd_handler_add(hd);
   else ecore_main_fd_handler_del(hd->fd_handler);
}

static void
edbus_timeout_data_free(void *timeout_data)
{
   EDBus_Timeout_Data *td = timeout_data;
   td->conn->timeouts = eina_inlist_remove(td->conn->timeouts,
                                           EINA_INLIST_GET(td));
   DBG("Timeout -- freeing timeout_data %p", td);
   if (td->handler) ecore_timer_del(td->handler);
   free(td);
}

static Eina_Bool
edbus_timeout_handler(void *data)
{
   EDBus_Timeout_Data *td = data;
   td->handler = NULL;

   if (!dbus_timeout_get_enabled(td->timeout))
     {
        DBG("timeout_handler (not enabled, ending)");
        return ECORE_CALLBACK_CANCEL;
     }

   DBG("Telling dbus to handle timeout with data %p", data);
   dbus_timeout_handle(td->timeout);
   return ECORE_CALLBACK_CANCEL;
}

static dbus_bool_t
cb_timeout_add(DBusTimeout *timeout, void *data)
{
   EDBus_Connection *conn = data;
   EDBus_Timeout_Data *td;

   if (!dbus_timeout_get_enabled(timeout))
     return EINA_TRUE;

   DBG("Adding timeout for connection@%p", conn);
   td = calloc(1, sizeof(EDBus_Timeout_Data));
   EINA_SAFETY_ON_NULL_RETURN_VAL(td, EINA_FALSE);
   td->conn = conn;
   dbus_timeout_set_data(timeout, (void *)td, edbus_timeout_data_free);
   td->interval = dbus_timeout_get_interval(timeout);
   td->timeout = timeout;

   td->handler = ecore_timer_add(td->interval, edbus_timeout_handler, td);
   conn->timeouts = eina_inlist_append(conn->timeouts,
                                       EINA_INLIST_GET(td));

   return EINA_TRUE;
}

static void
cb_timeout_del(DBusTimeout *timeout, void *data)
{
   DBG("timeout del!");
   /* will trigger edbus_timeout_data_free() */
   dbus_timeout_set_data(timeout, NULL, NULL);
}

static void
cb_timeout_toggle(DBusTimeout *timeout, void *data)
{
   EDBus_Timeout_Data *td;

   td = dbus_timeout_get_data(timeout);

   DBG("Timeout toggle; data@%p", td);
   if (dbus_timeout_get_enabled(td->timeout))
     {
        td->interval = dbus_timeout_get_interval(timeout);
        td->handler
          = ecore_timer_add(td->interval, edbus_timeout_handler, td);

        DBG("Timeout is enabled with interval %d, timer@%p",
           td->interval, td->handler);
     }
   else
     {
        DBG("Timeout is disabled, destroying timer@%p", td->handler);
        ecore_timer_del(td->handler);
        td->handler = NULL;
     }
}

static Eina_Bool
edbus_idler(void *data)
{
   EDBus_Connection *conn = data;

   DBG("Connection@%p: Dispatch status: %d", conn,
      dbus_connection_get_dispatch_status(conn->dbus_conn));

   if (DBUS_DISPATCH_COMPLETE ==
       dbus_connection_get_dispatch_status(conn->dbus_conn))
     {
        DBG("Connection@%p: Dispatch complete, idler@%p finishing",
           conn, conn->idler);
        conn->idler = NULL;
        return ECORE_CALLBACK_CANCEL;
     }

   dbus_connection_ref(conn->dbus_conn);
   DBG("Connection@%p: Dispatching", conn);
   dbus_connection_dispatch(conn->dbus_conn);
   dbus_connection_unref(conn->dbus_conn);
   return ECORE_CALLBACK_RENEW;
}

static void
cb_dispatch_status(DBusConnection *dbus_conn, DBusDispatchStatus new_status, void *data)
{
   EDBus_Connection *conn = data;

   DBG("Connection@%p: Dispatch status: %d", conn, new_status);

   if ((new_status == DBUS_DISPATCH_DATA_REMAINS) && (!conn->idler))
     {
        conn->idler = ecore_idler_add(edbus_idler, conn);
        DBG("Connection@%p: Adding idler@%p to handle remaining dispatch data",
           conn, conn->idler);
     }
   else if ((new_status != DBUS_DISPATCH_DATA_REMAINS) && (conn->idler))
     {
        DBG("Connection@%p: No remaining dispatch data, clearing idler@%p",
           conn, conn->idler);

        ecore_idler_del(conn->idler);
        conn->idler = NULL;
     }
}

static void
cb_signal_dispatcher(EDBus_Connection *conn, DBusMessage *msg)
{
   EDBus_Signal_Handler *sh;
   DBusMessageIter iter;
   int type, counter;
   char *arg_msg;
   EDBus_Message *edbus_msg;
   Signal_Argument *arg;

   edbus_msg = edbus_message_new(EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN(edbus_msg);

   edbus_msg->dbus_msg = dbus_message_ref(msg);
   dbus_message_iter_init(edbus_msg->dbus_msg,
                          &edbus_msg->iterator->dbus_iterator);
   conn->running_signal = EINA_TRUE;

   EINA_INLIST_FOREACH(conn->signal_handlers, sh)
     {
        if (sh->sender)
          {
             if (sh->bus)
               {
                  if ((sh->bus->unique_id == NULL) ||
                      (!sh->bus->unique_id[0]) ||
                      (!dbus_message_has_sender(msg, sh->bus->unique_id)))
                    continue;
               }
             else
                if (!dbus_message_has_sender(msg, sh->sender)) continue;
          }
        if (sh->path && !dbus_message_has_path(msg, sh->path)) continue;
        if (sh->member && !dbus_message_has_member(msg, sh->member)) continue;

        dbus_message_iter_init(msg, &iter);
        counter = 0;
        EINA_INLIST_FOREACH(sh->args, arg)
          {
             type = dbus_message_iter_get_arg_type(&iter);
             if (counter != arg->index || !(type == 's' || type == 'o'))
               goto next_sh;

             dbus_message_iter_get_basic(&iter, &arg_msg);
             if (strcmp(arg_msg, arg->value))
               goto next_sh;

             dbus_message_iter_next(&iter);
             counter++;
          }
        sh->cb((void *)sh->cb_data, edbus_msg);
        /*
         * Rewind iterator so another signal handler matching the same signal
         * can iterate over it.
         */
        dbus_message_iter_init(edbus_msg->dbus_msg,
                               &edbus_msg->iterator->dbus_iterator);

next_sh:
        type = 0;
     }

   edbus_message_unref(edbus_msg);
   conn->running_signal = EINA_FALSE;
}

static DBusHandlerResult
edbus_filter(DBusConnection *conn_dbus, DBusMessage *message, void *user_data)
{
   EDBus_Connection *conn = user_data;

   DBG("Connection@%p Got message:\n"
       "  Type: %s\n"
       "  Path: %s\n"
       "  Interface: %s\n"
       "  Member: %s\n"
       "  Sender: %s", conn,
       dbus_message_type_to_string(dbus_message_get_type(message)),
       dbus_message_get_path(message),
       dbus_message_get_interface(message),
       dbus_message_get_member(message),
       dbus_message_get_sender(message));

   switch (dbus_message_get_type(message))
     {
      case DBUS_MESSAGE_TYPE_METHOD_CALL:
        DBG("  Signature: %s", dbus_message_get_signature(message));
        break;

      case DBUS_MESSAGE_TYPE_METHOD_RETURN:
        DBG("  Reply serial: %d", dbus_message_get_reply_serial(message));
        break;

      case DBUS_MESSAGE_TYPE_ERROR:
        DBG("  Reply serial: %d", dbus_message_get_reply_serial(message));
        break;

      case DBUS_MESSAGE_TYPE_SIGNAL:
        DBG("  Signature: %s", dbus_message_get_signature(message));
        cb_signal_dispatcher(conn, message);
        break;

      default:
        break;
     }

   return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void
edbus_connection_setup(EDBus_Connection *conn)
{
   DBG("Setting up connection %p", conn);

   /* connection_setup */
   dbus_connection_set_exit_on_disconnect(conn->dbus_conn, EINA_FALSE);
   dbus_connection_set_watch_functions(conn->dbus_conn,
                                       cb_watch_add,
                                       cb_watch_del,
                                       cb_watch_toggle,
                                       conn,
                                       NULL);

   dbus_connection_set_timeout_functions(conn->dbus_conn,
                                         cb_timeout_add,
                                         cb_timeout_del,
                                         cb_timeout_toggle,
                                         conn,
                                         NULL);

   dbus_connection_set_dispatch_status_function(conn->dbus_conn,
                                                cb_dispatch_status,
                                                conn, NULL);
   dbus_connection_add_filter(conn->dbus_conn, edbus_filter, conn, NULL);

   cb_dispatch_status(conn->dbus_conn,
                      dbus_connection_get_dispatch_status(conn->dbus_conn),
                      conn);
}

EAPI EDBus_Connection *
edbus_connection_get(EDBus_Connection_Type type)
{
   EDBus_Connection *conn;
   DBusError err;

   DBG("Getting connection with type %d", type);

   if ((type < EDBUS_CONNECTION_TYPE_SESSION) ||
       (type > EDBUS_CONNECTION_TYPE_STARTER))
     return NULL;

   conn = shared_connections[type - 1];
   if (conn)
     {
        DBG("Connection with type %d exists at %p; reffing and returning",
           type, conn);
        return edbus_connection_ref(conn);
     }

   conn = calloc(1, sizeof(EDBus_Connection));
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);

   dbus_error_init(&err);
   conn->dbus_conn = dbus_bus_get_private(type - 1, &err);
   if (dbus_error_is_set(&err))
     {
        free(conn);
        ERR("Error connecting to bus: %s", err.message);
        return NULL;
     }
   edbus_connection_setup(conn);

   conn->type = type;
   shared_connections[type - 1] = conn;

   conn->refcount = 1;
   EINA_MAGIC_SET(conn, EDBUS_CONNECTION_MAGIC);

   conn->names = eina_hash_string_superfast_new(edbus_connection_name_free);

   DBG("Returned new connection at %p", conn);
   return conn;
}

EAPI EDBus_Connection *
edbus_connection_ref(EDBus_Connection *conn)
{
   EDBUS_CONNECTION_CHECK_RETVAL(conn, NULL);
   DBG("conn=%p, pre-refcount=%d", conn, conn->refcount);
   conn->refcount++;
   return conn;
}

static void
_edbus_connection_unref(EDBus_Connection *conn)
{
   unsigned int i;
   EDBus_Handler_Data *fd_handler;
   EDBus_Timeout_Data *timer;
   Eina_Inlist *list;
   EDBus_Signal_Handler *h;
   EDBus_Pending *p;

   DBG("Connection %p: unref (currently at %d refs)",
      conn, conn->refcount);

   if (--conn->refcount > 0) return;

   DBG("Freeing connection %p", conn);

   _edbus_connection_event_callback_call
     (conn, EDBUS_CONNECTION_EVENT_DEL, NULL);

   conn->refcount = 1;
   edbus_cbs_free_dispatch(&(conn->cbs_free), conn);

   EINA_INLIST_FOREACH_SAFE(conn->pendings, list, p)
     edbus_pending_cancel(p);

   while (conn->signal_handlers)
     {
        list = conn->signal_handlers;
        h = EINA_INLIST_CONTAINER_GET(conn->signal_handlers, EDBus_Signal_Handler);
        edbus_signal_handler_del(h);
     }
   conn->refcount = 0;

   /* after cbs_free dispatch these shouldn't exit, error if they do */

   if (conn->pendings)
     {
        CRITICAL("Connection %p released with live pending calls!",
                conn);
        EINA_INLIST_FOREACH(conn->pendings, p)
          ERR("conn=%p alive pending call=%p dest=%s path=%s %s.%s()",
              conn, p,
              edbus_pending_destination_get(p),
              edbus_pending_path_get(p),
              edbus_pending_interface_get(p),
              edbus_pending_method_get(p));
     }

   for (i = 0; i < EDBUS_CONNECTION_EVENT_LAST; i++)
     {
        EDBus_Connection_Context_Event *ce = conn->event_handlers + i;
        while (ce->list)
          {
             EDBus_Connection_Context_Event_Cb *ctx;

             ctx = EINA_INLIST_CONTAINER_GET(ce->list,
                                             EDBus_Connection_Context_Event_Cb);
             _edbus_connection_context_event_cb_del(ce, ctx);
          }
        eina_list_free(ce->to_delete);
     }

   eina_hash_free(conn->names);

   EINA_MAGIC_SET(conn, EINA_MAGIC_NONE);
   dbus_connection_close(conn->dbus_conn);
   dbus_connection_unref(conn->dbus_conn);
   conn->dbus_conn = NULL;

   EINA_INLIST_FOREACH_SAFE(conn->fd_handlers, list, fd_handler)
     edbus_fd_handler_del(fd_handler);

   EINA_INLIST_FOREACH_SAFE(conn->timeouts, list, timer)
     edbus_timeout_data_free(timer->handler);

   edbus_data_del_all(&conn->data);

   if (conn->idler) ecore_idler_del(conn->idler);
   shared_connections[conn->type - 1] = NULL;

   free(conn);
}

EAPI void
edbus_connection_unref(EDBus_Connection *conn)
{
   EDBUS_CONNECTION_CHECK(conn);
   DBG("conn=%p, pre-refcount=%d", conn, conn->refcount);
   _edbus_connection_unref(conn);
}

EAPI void
edbus_connection_cb_free_add(EDBus_Connection *conn, EDBus_Free_Cb cb, const void *data)
{
   EDBUS_CONNECTION_CHECK(conn);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   conn->cbs_free = edbus_cbs_free_add(conn->cbs_free, cb, data);
}

EAPI void
edbus_connection_cb_free_del(EDBus_Connection *conn, EDBus_Free_Cb cb, const void *data)
{
   EDBUS_CONNECTION_CHECK(conn);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   conn->cbs_free = edbus_cbs_free_del(conn->cbs_free, cb, data);
}

EAPI void
edbus_connection_data_set(EDBus_Connection *conn, const char *key, const void *data)
{
   EDBUS_CONNECTION_CHECK(conn);
   EINA_SAFETY_ON_NULL_RETURN(key);
   EINA_SAFETY_ON_NULL_RETURN(data);
   edbus_data_set(&(conn->data), key, data);
}

EAPI void *
edbus_connection_data_get(const EDBus_Connection *conn, const char *key)
{
   EDBUS_CONNECTION_CHECK_RETVAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key, NULL);
   return edbus_data_get(&(((EDBus_Connection *)conn)->data), key);
}

EAPI void *
edbus_connection_data_del(EDBus_Connection *conn, const char *key)
{
   EDBUS_CONNECTION_CHECK_RETVAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key, NULL);
   return edbus_data_del(&(((EDBus_Connection *)conn)->data), key);
}

static void
edbus_dispatch_name_owner_change(EDBus_Connection_Name *cn, const char *old_id)
{
   EDBus_Connection_Context_NOC_Cb *ctx;
   const char *previous_id = !old_id ? "" : old_id;
   cn->event_handlers.walking++;
   EINA_INLIST_FOREACH(cn->event_handlers.list, ctx)
     {
        if (ctx->deleted) continue;
        if (!old_id && !ctx->allow_initial)
          continue;
        ctx->cb((void *)ctx->cb_data, cn->name, previous_id, cn->unique_id);
     }
   cn->event_handlers.walking--;
}

typedef struct _dispach_name_owner_data
{
   EDBus_Connection_Context_NOC_Cb *ctx;
   const EDBus_Connection_Name *cn;
} dispatch_name_owner_data;

static Eina_Bool
dispach_name_owner_cb(void *context)
{
   dispatch_name_owner_data *data = context;
   data->ctx->cb((void *)data->ctx->cb_data, data->cn->name, "",
                 data->cn->unique_id);
   data->ctx->idler = NULL;
   free(data);
   return ECORE_CALLBACK_CANCEL;
}

EAPI void
edbus_name_owner_changed_callback_add(EDBus_Connection *conn, const char *bus, EDBus_Name_Owner_Changed_Cb cb, const void *cb_data, Eina_Bool allow_initial_call)
{
   EDBus_Connection_Name *cn;
   EDBus_Connection_Context_NOC_Cb *ctx;

   EDBUS_CONNECTION_CHECK(conn);
   EINA_SAFETY_ON_NULL_RETURN(bus);
   EINA_SAFETY_ON_NULL_RETURN(cb);

   cn = eina_hash_find(conn->names, bus);
   if (cn)
     edbus_connection_name_owner_monitor(conn, cn, EINA_TRUE);
   else
     {
        cn = edbus_connection_name_new(bus);
        EINA_SAFETY_ON_NULL_RETURN(cn);

        eina_hash_direct_add(conn->names, cn->name, cn);
        edbus_connection_name_owner_monitor(conn, cn, EINA_TRUE);
     }

   ctx = calloc(1, sizeof(EDBus_Connection_Context_NOC_Cb));
   EINA_SAFETY_ON_NULL_GOTO(ctx, cleanup);
   ctx->cb = cb;
   ctx->cb_data = cb_data;
   ctx->allow_initial = allow_initial_call;

   cn->event_handlers.list = eina_inlist_append(cn->event_handlers.list,
                                                EINA_INLIST_GET(ctx));
   if (cn->unique_id && allow_initial_call)
     {
        dispatch_name_owner_data *dispatch_data;
        dispatch_data = malloc(sizeof(dispatch_name_owner_data));
        EINA_SAFETY_ON_NULL_RETURN(dispatch_data);
        dispatch_data->cn = cn;
        dispatch_data->ctx = ctx;
        ctx->idler = ecore_idler_add(dispach_name_owner_cb, dispatch_data);
     }
   return;

cleanup:
   _edbus_connection_name_unref(conn, cn);
}

EAPI void
edbus_name_owner_changed_callback_del(EDBus_Connection *conn, const char *bus, EDBus_Name_Owner_Changed_Cb cb, const void *cb_data)
{
   EDBus_Connection_Name *cn;
   EDBus_Connection_Context_NOC_Cb *iter, *found = NULL;

   EDBUS_CONNECTION_CHECK(conn);
   EINA_SAFETY_ON_NULL_RETURN(bus);
   EINA_SAFETY_ON_NULL_RETURN(cb);

   cn = eina_hash_find(conn->names, bus);
   EINA_SAFETY_ON_NULL_RETURN(cn);

   EINA_INLIST_FOREACH(cn->event_handlers.list, iter)
     {
        if (cb != iter->cb) continue;
        if ((cb_data) && (cb_data != iter->cb_data)) continue;

        found = iter;
        break;
     }

   EINA_SAFETY_ON_NULL_RETURN(found);
   EINA_SAFETY_ON_TRUE_RETURN(found->deleted);

   if (cn->event_handlers.walking)
     {
        found->deleted = EINA_TRUE;
        cn->event_handlers.to_delete = eina_list_append
            (cn->event_handlers.to_delete, found);
        return;
     }

   cn->event_handlers.list = eina_inlist_remove(cn->event_handlers.list,
                                                EINA_INLIST_GET(found));
   if (found->idler)
     {
        dispatch_name_owner_data *data;
        data = ecore_idler_del(found->idler);
        free(data);
     }
   free(found);
   edbus_connection_name_owner_monitor(conn, cn, EINA_FALSE);
}

EAPI void
edbus_connection_event_callback_add(EDBus_Connection *conn, EDBus_Connection_Event_Type type, EDBus_Connection_Event_Cb cb, const void *cb_data)
{
   EDBus_Connection_Context_Event *ce;
   EDBus_Connection_Context_Event_Cb *ctx;

   EDBUS_CONNECTION_CHECK(conn);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   EINA_SAFETY_ON_TRUE_RETURN(type >= EDBUS_CONNECTION_EVENT_LAST);

   ce = conn->event_handlers + type;

   ctx = calloc(1, sizeof(EDBus_Connection_Context_Event_Cb));
   EINA_SAFETY_ON_NULL_RETURN(ctx);
   ctx->cb = cb;
   ctx->cb_data = cb_data;

   ce->list = eina_inlist_append(ce->list, EINA_INLIST_GET(ctx));
}

static void
_edbus_connection_context_event_cb_del(EDBus_Connection_Context_Event *ce, EDBus_Connection_Context_Event_Cb *ctx)
{
   ce->list = eina_inlist_remove(ce->list, EINA_INLIST_GET(ctx));
   free(ctx);
}

EAPI void
edbus_connection_event_callback_del(EDBus_Connection *conn, EDBus_Connection_Event_Type type, EDBus_Connection_Event_Cb cb, const void *cb_data)
{
   EDBus_Connection_Context_Event *ce;
   EDBus_Connection_Context_Event_Cb *iter, *found = NULL;

   EDBUS_CONNECTION_CHECK(conn);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   EINA_SAFETY_ON_TRUE_RETURN(type >= EDBUS_CONNECTION_EVENT_LAST);

   ce = conn->event_handlers + type;

   EINA_INLIST_FOREACH(ce->list, iter)
     {
        if (cb != iter->cb) continue;
        if ((cb_data) && (cb_data != iter->cb_data)) continue;

        found = iter;
        break;
     }

   EINA_SAFETY_ON_NULL_RETURN(found);
   EINA_SAFETY_ON_TRUE_RETURN(found->deleted);

   if (ce->walking)
     {
        found->deleted = EINA_TRUE;
        ce->to_delete = eina_list_append(ce->to_delete, found);
        return;
     }

   _edbus_connection_context_event_cb_del(ce, found);
}

static void
_edbus_connection_event_callback_call(EDBus_Connection *conn, EDBus_Connection_Event_Type type, const void *event_info)
{
   EDBus_Connection_Context_Event *ce;
   EDBus_Connection_Context_Event_Cb *iter;

   ce = conn->event_handlers + type;

   ce->walking++;
   EINA_INLIST_FOREACH(ce->list, iter)
     {
        if (iter->deleted) continue;
        iter->cb((void *)iter->cb_data, conn, (void *)event_info);
     }
   ce->walking--;
   if (ce->walking > 0) return;

   EINA_LIST_FREE(ce->to_delete, iter)
     _edbus_connection_context_event_cb_del(ce, iter);
}

void
edbus_connection_event_callback_call(EDBus_Connection *conn, EDBus_Connection_Event_Type type, const void *event_info)
{
   EDBUS_CONNECTION_CHECK(conn);
   EINA_SAFETY_ON_TRUE_RETURN(type >= EDBUS_CONNECTION_EVENT_LAST);
   EINA_SAFETY_ON_TRUE_RETURN(type == EDBUS_CONNECTION_EVENT_DEL);

   _edbus_connection_event_callback_call(conn, type, event_info);
}

void
edbus_connection_signal_handler_add(EDBus_Connection *conn, EDBus_Signal_Handler *handler)
{
   EDBUS_CONNECTION_CHECK(conn);
   EINA_SAFETY_ON_NULL_RETURN(handler);
   conn->signal_handlers = eina_inlist_append(conn->signal_handlers,
                                              EINA_INLIST_GET(handler));
}

void
edbus_connection_pending_add(EDBus_Connection *conn, EDBus_Pending *pending)
{
   EDBUS_CONNECTION_CHECK(conn);
   EINA_SAFETY_ON_NULL_RETURN(pending);
   conn->pendings = eina_inlist_append(conn->pendings,
                                       EINA_INLIST_GET(pending));
}

void
edbus_connection_signal_handler_del(EDBus_Connection *conn, EDBus_Signal_Handler *handler)
{
   EINA_SAFETY_ON_NULL_RETURN(conn);
   EINA_SAFETY_ON_NULL_RETURN(handler);
   conn->signal_handlers = eina_inlist_remove(conn->signal_handlers,
                                              EINA_INLIST_GET(handler));
}

void
edbus_connection_pending_del(EDBus_Connection *conn, EDBus_Pending *pending)
{
   EINA_SAFETY_ON_NULL_RETURN(conn);
   EINA_SAFETY_ON_NULL_RETURN(pending);
   conn->pendings = eina_inlist_remove(conn->pendings,
                                       EINA_INLIST_GET(pending));
}

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eldbus_private.h"
#include "eldbus_private_types.h"

#define ELDBUS_CONNECTION_CHECK(conn)                        \
  do                                                        \
    {                                                       \
       EINA_SAFETY_ON_NULL_RETURN(conn);                    \
       if (!EINA_MAGIC_CHECK(conn, ELDBUS_CONNECTION_MAGIC)) \
         {                                                  \
            EINA_MAGIC_FAIL(conn, ELDBUS_CONNECTION_MAGIC);  \
            return;                                         \
         }                                                  \
       EINA_SAFETY_ON_TRUE_RETURN(conn->refcount <= 0);     \
    }                                                       \
  while (0)

#define ELDBUS_CONNECTION_CHECK_RETVAL(conn, retval)                 \
  do                                                                \
    {                                                               \
       EINA_SAFETY_ON_NULL_RETURN_VAL(conn, retval);                \
       if (!EINA_MAGIC_CHECK(conn, ELDBUS_CONNECTION_MAGIC))         \
         {                                                          \
            EINA_MAGIC_FAIL(conn, ELDBUS_CONNECTION_MAGIC);          \
            return retval;                                          \
         }                                                          \
       EINA_SAFETY_ON_TRUE_RETURN_VAL(conn->refcount <= 0, retval); \
    }                                                               \
  while (0)

typedef struct _Eldbus_Connection_Context_Event_Cb
{
   EINA_INLIST;
   Eldbus_Connection_Event_Cb cb;
   const void               *cb_data;
   Eina_Bool                 deleted : 1;
} Eldbus_Connection_Context_Event_Cb;

typedef struct _Eldbus_Connection_Context_NOC_Cb
{
   EINA_INLIST;
   Eldbus_Name_Owner_Changed_Cb cb;
   const void                 *cb_data;
   Eina_Bool                   deleted : 1;
   Ecore_Idle_Enterer         *idle_enterer;
   Eina_Bool                   allow_initial : 1;
} Eldbus_Connection_Context_NOC_Cb;

typedef struct _Eldbus_Handler_Data
{
   EINA_INLIST;
   int               fd;
   Ecore_Fd_Handler *fd_handler;
   Eldbus_Connection *conn;
   DBusWatch        *watch;
   int               enabled;
} Eldbus_Handler_Data;

typedef struct _Eldbus_Timeout_Data
{
   EINA_INLIST;
   Ecore_Timer      *handler;
   DBusTimeout      *timeout;
   Eldbus_Connection *conn;
   int               interval;
} Eldbus_Timeout_Data;

static const Eldbus_Version _version = {VMAJ, VMIN, VMIC, VREV};
EAPI const Eldbus_Version * eldbus_version = &_version;

static int _eldbus_init_count = 0;
int _eldbus_log_dom = -1;
int eldbus_model_log_dom = -1;

/* We don't save ELDBUS_CONNECTION_TYPE_UNKNOWN in here so we need room for 
 * last - 1 elements */
static void *shared_connections[ELDBUS_CONNECTION_TYPE_LAST - 1];
static Eina_Hash *address_connections = NULL;

static void _eldbus_connection_event_callback_call(Eldbus_Connection *conn, Eldbus_Connection_Event_Type type, const void *event_info);
static void _eldbus_connection_context_event_cb_del(Eldbus_Connection_Context_Event *ce, Eldbus_Connection_Context_Event_Cb *ctx);
static void eldbus_dispatch_name_owner_change(Eldbus_Connection_Name *cn, const char *old_id);
static void _eldbus_connection_free(Eldbus_Connection *conn);

static void
eldbus_fd_handler_del(Eldbus_Handler_Data *hd)
{
   if (!hd->fd_handler) return;

   DBG("free Eldbus_Handler_Data %d", hd->fd);
   hd->conn->fd_handlers = eina_inlist_remove(hd->conn->fd_handlers,
                                              EINA_INLIST_GET(hd));
   if (hd->fd_handler)
     {
        ecore_main_fd_handler_del(hd->fd_handler);
        hd->fd_handler = NULL;
     }

   free(hd);
}

static void
_eldbus_fork_reset()
{
   int i;

   for (i =0; i < ELDBUS_CONNECTION_TYPE_LAST - 1; i++)
     {
        Eldbus_Connection *conn = shared_connections[i];
        if (conn)
          {
             Eina_Inlist *list;
             Eldbus_Handler_Data *fd_handler;

             EINA_INLIST_FOREACH_SAFE(conn->fd_handlers, list, fd_handler)
               dbus_watch_set_data(fd_handler->watch, NULL, NULL);
          }
        shared_connections[i] = NULL;
     }
   if (address_connections) eina_hash_free(address_connections);
   address_connections = NULL;
}

EAPI int
eldbus_init(void)
{
   if (_eldbus_init_count++ > 0)
     return _eldbus_init_count;

   if (!eina_init())
     {
        fputs("Eldbus: Unable to initialize eina\n", stderr);
        return 0;
     }

   if (!ecore_init())
     {
        fputs("Eldbus: Unable to initialize ecore\n", stderr);
        eina_shutdown();
        return 0;
     }

   _eldbus_log_dom = eina_log_domain_register("eldbus", EINA_COLOR_BLUE);
   if (_eldbus_log_dom < 0)
     {
        EINA_LOG_ERR("Unable to create an 'eldbus' log domain");
        ecore_shutdown();
        eina_shutdown();
        return 0;
     }

   eldbus_model_log_dom = eina_log_domain_register("eldbus_model", EINA_COLOR_CYAN);
   if (eldbus_model_log_dom < 0)
     {
        EINA_LOG_ERR("Unable to create an 'eldbus_model' log domain");
        eina_log_domain_unregister(_eldbus_log_dom);
        _eldbus_log_dom = -1;
        ecore_shutdown();
        eina_shutdown();
        return 0;
     }

   eina_magic_string_set(ELDBUS_CONNECTION_MAGIC, "Eldbus_Connection");
   eina_magic_string_set(ELDBUS_MESSAGE_MAGIC, "Eldbus_Message");
   eina_magic_string_set(ELDBUS_SIGNAL_HANDLER_MAGIC, "Eldbus_Signal_Handler");
   eina_magic_string_set(ELDBUS_PENDING_MAGIC, "Eldbus_Pending");
   eina_magic_string_set(ELDBUS_OBJECT_MAGIC, "Eldbus_Object");
   eina_magic_string_set(ELDBUS_PROXY_MAGIC, "Eldbus_Proxy");
   eina_magic_string_set(ELDBUS_MESSAGE_ITERATOR_MAGIC, "Eldbus_Message_Iterator");
   eina_magic_string_set(ELDBUS_SERVICE_INTERFACE_MAGIC, "Eldbus_Service_Interface");


   if (!eldbus_message_init()) goto message_failed;
   if (!eldbus_signal_handler_init()) goto signal_handler_failed;
   if (!eldbus_pending_init()) goto pending_failed;
   if (!eldbus_object_init()) goto object_failed;
   if (!eldbus_proxy_init()) goto proxy_failed;
   if (!eldbus_service_init()) goto service_failed;
   ecore_fork_reset_callback_add(_eldbus_fork_reset, NULL);
   return _eldbus_init_count;

service_failed:
   eldbus_proxy_shutdown();
proxy_failed:
   eldbus_object_shutdown();
object_failed:
   eldbus_pending_shutdown();
pending_failed:
   eldbus_signal_handler_shutdown();
signal_handler_failed:
   eldbus_message_shutdown();
message_failed:
   eina_log_domain_unregister(eldbus_model_log_dom);
   eldbus_model_log_dom = -1;
   eina_log_domain_unregister(_eldbus_log_dom);
   _eldbus_log_dom = -1;
   ecore_shutdown();
   eina_shutdown();

   return 0;
}

static void
print_live_connection(Eldbus_Connection *conn)
{
   if (!conn->names)
     ERR("conn=%p has no alive objects", conn);
   else
     {
        Eina_Iterator *iter = eina_hash_iterator_data_new(conn->names);
        Eldbus_Connection_Name *name;
        EINA_ITERATOR_FOREACH(iter, name)
          {
             Eldbus_Object *obj;
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
        Eldbus_Pending *p;
        EINA_INLIST_FOREACH(conn->pendings, p)
          ERR("conn=%p alive pending call=%p dest=%s path=%s %s.%s()",
              conn, p,
              eldbus_pending_destination_get(p),
              eldbus_pending_path_get(p),
              eldbus_pending_interface_get(p),
              eldbus_pending_method_get(p));
     }
}

EAPI int
eldbus_shutdown(void)
{
   if (_eldbus_init_count <= 0)
     {
        ERR("Init count not greater than 0 in shutdown.");
        _eldbus_init_count = 0;
        return 0;
     }
   if (--_eldbus_init_count)
     return _eldbus_init_count;

   ecore_fork_reset_callback_del(_eldbus_fork_reset, NULL);
   if (shared_connections[ELDBUS_CONNECTION_TYPE_SESSION - 1])
     {
        CRI("Alive TYPE_SESSION connection");
        print_live_connection(shared_connections[ELDBUS_CONNECTION_TYPE_SESSION - 1]);
     }
   if (shared_connections[ELDBUS_CONNECTION_TYPE_SYSTEM - 1])
     {
        CRI("Alive TYPE_SYSTEM connection");
        print_live_connection(shared_connections[ELDBUS_CONNECTION_TYPE_SYSTEM - 1]);
     }
   if (shared_connections[ELDBUS_CONNECTION_TYPE_STARTER - 1])
     {
        CRI("Alive TYPE_STARTER connection");
        print_live_connection(shared_connections[ELDBUS_CONNECTION_TYPE_STARTER - 1]);
     }
   if (shared_connections[ELDBUS_CONNECTION_TYPE_ADDRESS - 1])
     {
        if (eina_hash_population(address_connections))
          {
             Eina_Iterator *it;
             Eina_Hash_Tuple *tuple;

             it = eina_hash_iterator_tuple_new(address_connections);
             EINA_ITERATOR_FOREACH(it, tuple)
               {
                  CRI("Alive TYPE_ADDRESS connection: %s", (char*)tuple->key);
                  print_live_connection(tuple->data);
               }
             eina_iterator_free(it);
          }

        eina_hash_free(address_connections);
        address_connections = shared_connections[ELDBUS_CONNECTION_TYPE_ADDRESS - 1] = NULL;
     }

   eldbus_service_shutdown();
   eldbus_proxy_shutdown();
   eldbus_object_shutdown();
   eldbus_pending_shutdown();
   eldbus_signal_handler_shutdown();
   eldbus_message_shutdown();

   ecore_shutdown();

   eina_log_domain_unregister(eldbus_model_log_dom);
   eldbus_model_log_dom = -1;
   eina_log_domain_unregister(_eldbus_log_dom);
   _eldbus_log_dom = -1;

   eina_shutdown();

   return 0;
}

/* TODO: mempool of Eldbus_Context_Free_Cb */
typedef struct _Eldbus_Context_Free_Cb
{
   EINA_INLIST;
   Eldbus_Free_Cb cb;
   const void   *data;
} Eldbus_Context_Free_Cb;

void
eldbus_cbs_free_dispatch(Eina_Inlist **p_lst, const void *dead_pointer)
{
   Eina_Inlist *lst = *p_lst;
   *p_lst = NULL;
   while (lst)
     {
        Eina_Inlist *next = lst->next;
        Eldbus_Context_Free_Cb *ctx;

        ctx = EINA_INLIST_CONTAINER_GET(lst, Eldbus_Context_Free_Cb);
        ctx->cb((void *)ctx->data, dead_pointer);
        free(ctx);

        lst = next;
     }
}

Eina_Inlist *
eldbus_cbs_free_add(Eina_Inlist *lst, Eldbus_Free_Cb cb, const void *data)
{
   Eldbus_Context_Free_Cb *ctx = malloc(sizeof(Eldbus_Context_Free_Cb));
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, lst);

   ctx->cb = cb;
   ctx->data = data;

   return eina_inlist_append(lst, EINA_INLIST_GET(ctx));
}

Eina_Inlist *
eldbus_cbs_free_del(Eina_Inlist *lst, Eldbus_Free_Cb cb, const void *data)
{
   Eldbus_Context_Free_Cb *ctx;

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

typedef struct _Eldbus_Data
{
   EINA_INLIST;
   const void  *data;
   unsigned int keylen;
   char         key[];
} Eldbus_Data;

static inline Eldbus_Data *
eldbus_data_find(Eina_Inlist **p_lst, const char *key)
{
   unsigned int keylen = strlen(key);
   Eldbus_Data *d;

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
eldbus_data_set(Eina_Inlist **p_lst, const char *key, const void *data)
{
   Eldbus_Data *d = eldbus_data_find(p_lst, key);
   unsigned int keylen = strlen(key);

   if (d)
     {
        *p_lst = eina_inlist_remove(*p_lst, EINA_INLIST_GET(d));
        free(d);
     }

   d = malloc(sizeof(Eldbus_Data) + keylen + 1);
   EINA_SAFETY_ON_NULL_RETURN(d);

   d->data = data;
   d->keylen = keylen;
   memcpy(d->key, key, keylen + 1);

   *p_lst = eina_inlist_prepend(*p_lst, EINA_INLIST_GET(d));
}

void *
eldbus_data_get(Eina_Inlist **p_lst, const char *key)
{
   Eldbus_Data *d = eldbus_data_find(p_lst, key);
   return d ? (void *)d->data : NULL;
}

void *
eldbus_data_del(Eina_Inlist **p_lst, const char *key)
{
   Eldbus_Data *d = eldbus_data_find(p_lst, key);
   void *ret;
   if (!d) return NULL;

   ret = (void *)d->data;
   *p_lst = eina_inlist_remove(*p_lst, EINA_INLIST_GET(d));
   free(d);

   return ret;
}

void
eldbus_data_del_all(Eina_Inlist **p_list)
{
   Eina_Inlist *n = *p_list;
   *p_list = NULL;

   while (n)
     {
        Eldbus_Data *d = EINA_INLIST_CONTAINER_GET(n, Eldbus_Data);
        n = eina_inlist_remove(n, n);
        DBG("key=%s, data=%p", d->key, d->data);
        free(d);
     }
}

static void
eldbus_connection_name_gc(Eldbus_Connection *conn, Eldbus_Connection_Name *cn)
{
   if (cn->refcount > 0)
     return;
   if (cn->objects && eina_hash_population(cn->objects) > 0)
     return;
   if (cn->event_handlers.list != NULL)
     return;

   eina_hash_del(conn->names, cn->name, cn);
   if (cn->name_owner_changed)
     eldbus_signal_handler_del(cn->name_owner_changed);
   if (cn->objects)
     eina_hash_free(cn->objects);
   eina_stringshare_del(cn->name);
   if (cn->name_owner_get)
     eldbus_pending_cancel(cn->name_owner_get);
   if (cn->unique_id)
     eina_stringshare_del(cn->unique_id);
   eina_list_free(cn->event_handlers.to_delete);
   free(cn);
}

void
eldbus_connection_name_object_del(Eldbus_Connection *conn, const Eldbus_Object *obj)
{
   Eldbus_Connection_Name *cn = eina_hash_find(conn->names, obj->name);

   if (!cn) return;
   if (!cn->objects) return;
   eina_hash_del(cn->objects, obj->path, obj);

   eldbus_connection_name_gc(conn, cn);
}

void
eldbus_connection_name_object_set(Eldbus_Connection *conn, Eldbus_Object *obj)
{
   Eldbus_Connection_Name *cn;

   cn = eldbus_connection_name_get(conn, obj->name);
   eina_hash_add(cn->objects, obj->path, obj);

   return;
}

static void
on_name_owner_changed(void *data, const Eldbus_Message *msg)
{
   const char *bus, *older_id, *new_id;
   Eldbus_Connection_Name *cn = data;

   if (!eldbus_message_arguments_get(msg, "sss", &bus, &older_id, &new_id))
     {
        ERR("Error getting arguments from NameOwnerChanged cn=%s", cn->name);
        return;
     }

   eina_stringshare_replace(&cn->unique_id, new_id);
   eldbus_dispatch_name_owner_change(cn, older_id);
}

static void
on_get_name_owner(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *unique_id = "", *error_name;
   Eldbus_Connection_Name *cn = data;

   cn->name_owner_get = NULL;

   if (eldbus_message_error_get(msg, &error_name, NULL))
     {
        if (!strcmp(error_name, ELDBUS_ERROR_PENDING_CANCELED))
          return;
        DBG("GetNameOwner of bus = %s returned an error", cn->name);
     }
   else if (!eldbus_message_arguments_get(msg, "s", &unique_id))
     ERR("Error getting arguments from GetNameOwner");

   cn->unique_id = eina_stringshare_add(unique_id);
   eldbus_dispatch_name_owner_change(cn, NULL);
}

void
eldbus_connection_name_ref(Eldbus_Connection_Name *cn)
{
   EINA_SAFETY_ON_NULL_RETURN(cn);
   cn->refcount++;
}

void
eldbus_connection_name_unref(Eldbus_Connection *conn, Eldbus_Connection_Name *cn)
{
   ELDBUS_CONNECTION_CHECK(conn);
   EINA_SAFETY_ON_NULL_RETURN(cn);

   cn->refcount--;
   eldbus_connection_name_gc(conn, cn);
}

Eldbus_Connection_Name *
eldbus_connection_name_get(Eldbus_Connection *conn, const char *name)
{
   Eldbus_Connection_Name *cn;
   ELDBUS_CONNECTION_CHECK_RETVAL(conn, NULL);

   cn = eina_hash_find(conn->names, name);
   if (cn) return cn;

   cn = calloc(1, sizeof(Eldbus_Connection_Name));
   EINA_SAFETY_ON_NULL_RETURN_VAL(cn, NULL);
   cn->name = eina_stringshare_add(name);
   cn->objects = eina_hash_string_superfast_new(NULL);

   if (!strcmp(name, ELDBUS_FDO_BUS))
     {
        cn->unique_id = eina_stringshare_add(name);
        goto end;
     }

   if (name[0] == ':')
     cn->unique_id = eina_stringshare_add(name);
   else
     cn->name_owner_get = eldbus_name_owner_get(conn, cn->name, on_get_name_owner, cn);

   cn->name_owner_changed = _eldbus_signal_handler_add(conn, ELDBUS_FDO_BUS,
                                                      ELDBUS_FDO_PATH,
                                                      ELDBUS_FDO_INTERFACE,
                                                      "NameOwnerChanged",
                                                      on_name_owner_changed, cn);
   eldbus_signal_handler_match_extra_set(cn->name_owner_changed, "arg0",
                                        cn->name, NULL);

end:
   eina_hash_direct_add(conn->names, cn->name, cn);
   return cn;
}

Eldbus_Object *
eldbus_connection_name_object_get(Eldbus_Connection *conn, const char *name, const char *path)
{
   Eldbus_Connection_Name *cn;

   ELDBUS_CONNECTION_CHECK_RETVAL(conn, NULL);
   cn = eina_hash_find(conn->names, name);
   if (!cn) return NULL;
   if (!cn->objects) return NULL;
   return eina_hash_find(cn->objects, path);
}

static Eina_Bool
eldbus_fd_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   Eldbus_Handler_Data *hd = data;
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
eldbus_fd_handler_add(Eldbus_Handler_Data *hd)
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
                                              eldbus_fd_handler,
                                              hd,
                                              NULL,
                                              NULL);
}

static void
eldbus_handler_data_free(void *data)
{
   Eldbus_Handler_Data *hd = data;
   eldbus_fd_handler_del(hd);
}

static dbus_bool_t
cb_watch_add(DBusWatch *watch, void *data)
{
   Eldbus_Connection *conn = data;
   Eldbus_Handler_Data *hd;

   if (!dbus_watch_get_enabled(watch)) return EINA_TRUE;

   DBG("cb_watch_add (enabled: %d)", dbus_watch_get_unix_fd(watch));

   hd = calloc(1, sizeof(Eldbus_Handler_Data));
   EINA_SAFETY_ON_NULL_RETURN_VAL(hd, EINA_FALSE);
   dbus_watch_set_data(watch, hd, eldbus_handler_data_free);
   hd->conn = conn;
   hd->watch = watch;
   hd->enabled = dbus_watch_get_enabled(watch);
   hd->fd = dbus_watch_get_unix_fd(hd->watch);

   conn->fd_handlers = eina_inlist_append(hd->conn->fd_handlers,
                                          EINA_INLIST_GET(hd));
   eldbus_fd_handler_add(hd);

   return EINA_TRUE;
}

static void
cb_watch_del(DBusWatch *watch, void *data EINA_UNUSED)
{
   DBG("cb_watch_del");
   /* will trigger eldbus_handler_data_free() */
   dbus_watch_set_data(watch, NULL, NULL);
}

static void
cb_watch_toggle(DBusWatch *watch, void *data EINA_UNUSED)
{
   Eldbus_Handler_Data *hd;
   hd = dbus_watch_get_data(watch);
   if (!hd) return;
   DBG("cb_watch_toggle %d", hd->fd);

   hd->enabled = dbus_watch_get_enabled(watch);

   DBG("watch %p is %sabled", hd, hd->enabled ? "en" : "dis");
   if (hd->enabled)
     {
        eldbus_fd_handler_add(hd);
     }
   else
     {
        ecore_main_fd_handler_del(hd->fd_handler);
        hd->fd_handler = NULL;
     }
}

static void
eldbus_timeout_data_free(void *timeout_data)
{
   Eldbus_Timeout_Data *td = timeout_data;
   td->conn->timeouts = eina_inlist_remove(td->conn->timeouts,
                                           EINA_INLIST_GET(td));
   DBG("Timeout -- freeing timeout_data %p", td);
   if (td->handler) ecore_timer_del(td->handler);
   free(td);
}

static Eina_Bool
eldbus_timeout_handler(void *data)
{
   Eldbus_Timeout_Data *td = data;
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
   Eldbus_Connection *conn = data;
   Eldbus_Timeout_Data *td;

   if (!dbus_timeout_get_enabled(timeout))
     return EINA_TRUE;

   DBG("Adding timeout for connection@%p", conn);
   td = calloc(1, sizeof(Eldbus_Timeout_Data));
   EINA_SAFETY_ON_NULL_RETURN_VAL(td, EINA_FALSE);
   td->conn = conn;
   dbus_timeout_set_data(timeout, (void *)td, eldbus_timeout_data_free);
   td->interval = dbus_timeout_get_interval(timeout);
   td->timeout = timeout;

   td->handler = ecore_timer_add(td->interval / 1000.0, eldbus_timeout_handler, td);
   conn->timeouts = eina_inlist_append(conn->timeouts,
                                       EINA_INLIST_GET(td));

   return EINA_TRUE;
}

static void
cb_timeout_del(DBusTimeout *timeout, void *data EINA_UNUSED)
{
   DBG("timeout del!");
   /* will trigger eldbus_timeout_data_free() */
   dbus_timeout_set_data(timeout, NULL, NULL);
}

static void
cb_timeout_toggle(DBusTimeout *timeout, void *data EINA_UNUSED)
{
   Eldbus_Timeout_Data *td;

   td = dbus_timeout_get_data(timeout);

   DBG("Timeout toggle; data@%p", td);
   if (dbus_timeout_get_enabled(td->timeout))
     {
        td->interval = dbus_timeout_get_interval(timeout);
        td->handler
          = ecore_timer_add(td->interval, eldbus_timeout_handler, td);

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
eldbus_idle_enterer(void *data)
{
   Eldbus_Connection *conn = data;

   DBG("Connection@%p: Dispatch status: %d", conn,
       dbus_connection_get_dispatch_status(conn->dbus_conn));

   if (dbus_connection_get_dispatch_status(conn->dbus_conn) ==
       DBUS_DISPATCH_COMPLETE)
     {
        DBG("Connection@%p: Dispatch complete, idle_enterer@%p finishing",
            conn, conn->idle_enterer);
        conn->idle_enterer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   DBG("Connection@%p: Dispatching", conn);
   eldbus_init();
   eldbus_connection_ref(conn);

   DBusDispatchStatus status;
   do
     {
        status = dbus_connection_dispatch(conn->dbus_conn);
     }
   while(status == DBUS_DISPATCH_DATA_REMAINS);

   eldbus_connection_unref(conn);
   eldbus_shutdown();
   return ECORE_CALLBACK_RENEW;
}

static void
cb_dispatch_status(DBusConnection *dbus_conn EINA_UNUSED, DBusDispatchStatus new_status, void *data)
{
   Eldbus_Connection *conn = data;

   if (!conn->refcount)
     {
        DBG("Connection[%p] being freed, dispatch blocked", conn);
        return;
     }

   DBG("Connection@%p: Dispatch status: %d", conn, new_status);

   if ((new_status == DBUS_DISPATCH_DATA_REMAINS) && (!conn->idle_enterer))
     {
        conn->idle_enterer = ecore_idle_enterer_add(eldbus_idle_enterer, conn);
        DBG("Connection@%p: Adding idle_enterer@%p to handle remaining dispatch data",
            conn, conn->idle_enterer);
     }
   else if ((new_status != DBUS_DISPATCH_DATA_REMAINS) && (conn->idle_enterer))
     {
        DBG("Connection@%p: No remaining dispatch data, clearing idle_enterer@%p",
            conn, conn->idle_enterer);

        ecore_idle_enterer_del(conn->idle_enterer);
        conn->idle_enterer = NULL;
     }
}

static inline Eina_Bool
extra_arguments_check(DBusMessage *msg, Eldbus_Signal_Handler *sh)
{
   DBusMessageIter iter;
   Signal_Argument *arg;
   unsigned int arg_index = 0;

   dbus_message_iter_init(msg, &iter);
   EINA_INLIST_FOREACH(sh->args, arg)
     {
        const char *arg_msg;
        int type = 0;

        while((arg->index > arg_index) && dbus_message_iter_next(&iter))
          arg_index++;

        if (arg_index != arg->index)
          return EINA_FALSE;

        type = dbus_message_iter_get_arg_type(&iter);
        if (!(type == 's' || type == 'o'))
          return EINA_FALSE;

        dbus_message_iter_get_basic(&iter, &arg_msg);
        if (strcmp(arg_msg, arg->value))
          return EINA_FALSE;
     }
   return EINA_TRUE;
}

static void
cb_signal_dispatcher(Eldbus_Connection *conn, DBusMessage *msg)
{
   Eldbus_Message *eldbus_msg;
   Eina_Inlist *next;

   eldbus_msg = eldbus_message_new(EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN(eldbus_msg);

   eldbus_msg->dbus_msg = dbus_message_ref(msg);
   dbus_message_iter_init(eldbus_msg->dbus_msg,
                          &eldbus_msg->iterator->dbus_iterator);

   eldbus_connection_ref(conn);
   eldbus_init();
   /*
    * Do the walking open-coded so we don't crash if a callback
    * removes other signal handlers from the list and we don't own
    * yet a reference to them.
    */
   next = conn->signal_handlers;
   while (next != NULL)
     {
        Eldbus_Signal_Handler *sh;

        sh = EINA_INLIST_CONTAINER_GET(next, Eldbus_Signal_Handler);
        next = next->next;

        if (sh->dangling) continue;
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
        if (sh->interface && !dbus_message_has_interface(msg, sh->interface))
          continue;
        if (sh->member && !dbus_message_has_member(msg, sh->member)) continue;
        if (!extra_arguments_check(msg, sh)) continue;

        eldbus_signal_handler_ref(sh);
        sh->cb((void *)sh->cb_data, eldbus_msg);
        /* update next signal handler because the list may have changed */
        next = EINA_INLIST_GET(sh)->next;
        eldbus_signal_handler_unref(sh);

        /*
         * Rewind iterator so another signal handler matching the same signal
         * can iterate over it.
         */
        dbus_message_iter_init(eldbus_msg->dbus_msg,
                               &eldbus_msg->iterator->dbus_iterator);
     }

   eldbus_message_unref(eldbus_msg);
   eldbus_connection_unref(conn);
   eldbus_shutdown();
}

static DBusHandlerResult
eldbus_filter(DBusConnection *conn_dbus EINA_UNUSED, DBusMessage *message, void *user_data)
{
   Eldbus_Connection *conn = user_data;

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
      case DBUS_MESSAGE_TYPE_ERROR:
        DBG("  Reply serial: %d", dbus_message_get_reply_serial(message));
        break;
      case DBUS_MESSAGE_TYPE_SIGNAL:
        DBG("  Signature: %s", dbus_message_get_signature(message));
        cb_signal_dispatcher(conn, message);
        break;
     }

   return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void
eldbus_connection_setup(Eldbus_Connection *conn)
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
   dbus_connection_add_filter(conn->dbus_conn, eldbus_filter, conn, NULL);

   cb_dispatch_status(conn->dbus_conn,
                      dbus_connection_get_dispatch_status(conn->dbus_conn),
                      conn);
}

static void
_disconnected(void *data, const Eldbus_Message *msg EINA_UNUSED)
{
   Eldbus_Connection *conn = data;
   Ecore_Event_Signal_Exit *ev;

   _eldbus_connection_event_callback_call(
      conn, ELDBUS_CONNECTION_EVENT_DISCONNECTED, NULL);
   if (conn->type != ELDBUS_CONNECTION_TYPE_SESSION) return;

   ev = calloc(1, sizeof(Ecore_Event_Signal_Exit));
   if (!ev) return;

   ev->quit = EINA_TRUE;
   ecore_event_add(ECORE_EVENT_SIGNAL_EXIT, ev, NULL, NULL);
}

/* Param address is only used for ELDBUS_CONNECTION_TYPE_ADDRESS type */
static Eldbus_Connection *
_connection_get(Eldbus_Connection_Type type, const char *address, Eina_Bool shared)
{
   Eldbus_Connection *conn;
   DBusError err;
   Eldbus_Object *obj;

   EINA_SAFETY_ON_FALSE_RETURN_VAL((type < ELDBUS_CONNECTION_TYPE_LAST) &&
                                   (type > ELDBUS_CONNECTION_TYPE_UNKNOWN), NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL((type == ELDBUS_CONNECTION_TYPE_ADDRESS) &&
                                  (address == NULL), NULL);

   conn = calloc(1, sizeof(Eldbus_Connection));
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);

   dbus_error_init(&err);
   if (type == ELDBUS_CONNECTION_TYPE_ADDRESS)
      conn->dbus_conn = dbus_connection_open_private(address, &err);
   else
      conn->dbus_conn = dbus_bus_get_private(type - 1, &err);

   if (!conn->dbus_conn || dbus_error_is_set(&err))
     {
        free(conn);
        ERR("Error connecting to bus of type %d. error name: %s error message: %s",
            type, err.name, err.message);
        return NULL;
     }

   if (type == ELDBUS_CONNECTION_TYPE_ADDRESS &&
       !dbus_bus_register(conn->dbus_conn, &err))
     {
        dbus_connection_close(conn->dbus_conn);
        dbus_connection_unref(conn->dbus_conn);
        conn->dbus_conn = NULL;
        free(conn);
        ERR("Error registering with bus: %s", err.message);
        return NULL;
     }

   conn->type = type;
   conn->refcount = 1;
   conn->shared = !!shared;
   EINA_MAGIC_SET(conn, ELDBUS_CONNECTION_MAGIC);
   conn->names = eina_hash_string_superfast_new(NULL);
   eldbus_connection_setup(conn);

   eldbus_signal_handler_add(conn, NULL, DBUS_PATH_LOCAL, DBUS_INTERFACE_LOCAL,
                             "Disconnected", _disconnected, conn);
   obj = eldbus_object_get(conn, ELDBUS_FDO_BUS, ELDBUS_FDO_PATH);
   conn->fdo_proxy = eldbus_proxy_get(obj, ELDBUS_FDO_INTERFACE);

   DBG("Returned new connection at %p", conn);
   return conn;
}

EAPI Eldbus_Connection *
eldbus_private_connection_get(Eldbus_Connection_Type type)
{
   DBG("Getting private connection with type %d", type);
   return _connection_get(type, NULL, EINA_FALSE);
}

EAPI Eldbus_Connection *
eldbus_connection_get(Eldbus_Connection_Type type)
{
   Eldbus_Connection *conn;

   DBG("Getting connection with type %d", type);

   if (!type)
     return NULL;

   if (type == ELDBUS_CONNECTION_TYPE_ADDRESS)
     {
        ERR("CONNECTION_TYPE_ADDRESS must be used with appropriate address_connection_get() function");
        return NULL;
     }

   conn = (Eldbus_Connection *) shared_connections[type - 1];
   if (conn)
     {
        DBG("Connection with type %d exists at %p; reffing and returning",
            type, conn);
        return eldbus_connection_ref(conn);
     }

   conn = _connection_get(type, NULL, EINA_TRUE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   shared_connections[type - 1] = conn;

   return conn;
}

EAPI Eldbus_Connection *
eldbus_address_connection_get(const char *address)
{
   Eldbus_Connection *conn = NULL;

   DBG("Getting connection with address %s", address);

   if (address_connections == NULL)
     {
        address_connections = eina_hash_string_small_new(NULL);
        shared_connections[ELDBUS_CONNECTION_TYPE_ADDRESS - 1] = address_connections;
     }
   else
     {
        conn = (Eldbus_Connection *) eina_hash_find(address_connections, address);
     }

   if (conn != NULL)
     {
        DBG("Connection with address %s exists at %p; reffing and returning",
            address, conn);
        return eldbus_connection_ref(conn);
     }

   conn = _connection_get(ELDBUS_CONNECTION_TYPE_ADDRESS, address, EINA_TRUE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   eina_hash_add(address_connections, address, conn);

   return conn;
}

EAPI Eldbus_Connection *
eldbus_private_address_connection_get(const char *address)
{
   DBG("Getting private connection with address %s", address);
   return _connection_get(ELDBUS_CONNECTION_TYPE_ADDRESS, address, EINA_FALSE);
}

EAPI Eldbus_Connection *
eldbus_connection_ref(Eldbus_Connection *conn)
{
   ELDBUS_CONNECTION_CHECK_RETVAL(conn, NULL);
   DBG("conn=%p, pre-refcount=%d", conn, conn->refcount);
   conn->refcount++;
   return conn;
}

static void
_eldbus_connection_free(Eldbus_Connection *conn)
{
   unsigned int i;
   Eldbus_Handler_Data *fd_handler;
   Eldbus_Timeout_Data *timer;
   Eina_Inlist *list;
   Eldbus_Signal_Handler *h;
   Eldbus_Pending *p;
   Eina_Iterator *iter;
   Eldbus_Connection_Name *cn;
   Eina_Array *cns;
   const char *name;

   DBG("Freeing connection %p", conn);

   _eldbus_connection_event_callback_call
     (conn, ELDBUS_CONNECTION_EVENT_DEL, NULL);

   conn->refcount = 1;
   eldbus_cbs_free_dispatch(&(conn->cbs_free), conn);

   /**
    * Flush all messages in outgoing queue, also this will send all
    * ObjectManager and Property changed signals of all paths that
    * this connection is server.
    */
   dbus_connection_flush(conn->dbus_conn);

   EINA_INLIST_FOREACH_SAFE(conn->pendings, list, p)
     eldbus_pending_cancel(p);

   cns = eina_array_new(eina_hash_population(conn->names));
   iter = eina_hash_iterator_data_new(conn->names);
   EINA_ITERATOR_FOREACH(iter, cn)
     {
        while (cn->event_handlers.list)
          {
             Eldbus_Connection_Context_NOC_Cb *ctx;
             ctx = EINA_INLIST_CONTAINER_GET(cn->event_handlers.list,
                                             Eldbus_Connection_Context_NOC_Cb);
             cn->event_handlers.list = eina_inlist_remove(cn->event_handlers.list,
                                                          cn->event_handlers.list);
             free(ctx);
          }
        eina_array_push(cns, eina_stringshare_add(cn->name));
     }
   eina_iterator_free(iter);

   while ((name = eina_array_pop(cns)))
     {
        cn = eina_hash_find(conn->names, name);
        if (cn) eldbus_connection_name_gc(conn, cn);
        eina_stringshare_del(name);
     }

   eina_hash_free(conn->names);
   eina_array_free(cns);

   conn->refcount = 0;

   /* after cbs_free dispatch these shouldn't exit, error if they do */

   if (conn->pendings)
     {
        CRI("Connection %p released with live pending calls!",
                 conn);
        EINA_INLIST_FOREACH(conn->pendings, p)
          ERR("conn=%p alive pending call=%p dest=%s path=%s %s.%s()", conn, p,
              eldbus_pending_destination_get(p),
              eldbus_pending_path_get(p),
              eldbus_pending_interface_get(p),
              eldbus_pending_method_get(p));
     }

   if (conn->signal_handlers)
     {
        CRI("Connection %p released with live signal handlers!", conn);
        EINA_INLIST_FOREACH(conn->signal_handlers, h)
          ERR("conn=%p alive signal=%p %s.%s path=%s", conn, h, h->interface,
              h->member, h->path);
     }

   for (i = 0; i < ELDBUS_CONNECTION_EVENT_LAST; i++)
     {
        Eldbus_Connection_Context_Event *ce = conn->event_handlers + i;
        while (ce->list)
          {
             Eldbus_Connection_Context_Event_Cb *ctx;

             ctx = EINA_INLIST_CONTAINER_GET(ce->list,
                                             Eldbus_Connection_Context_Event_Cb);
             _eldbus_connection_context_event_cb_del(ce, ctx);
          }
        eina_list_free(ce->to_delete);
     }

   EINA_MAGIC_SET(conn, EINA_MAGIC_NONE);
   //will trigger a cb_dispatch_status()
   dbus_connection_close(conn->dbus_conn);
   dbus_connection_unref(conn->dbus_conn);
   conn->dbus_conn = NULL;

   EINA_INLIST_FOREACH_SAFE(conn->fd_handlers, list, fd_handler)
     eldbus_fd_handler_del(fd_handler);

   EINA_INLIST_FOREACH_SAFE(conn->timeouts, list, timer)
     eldbus_timeout_data_free(timer->handler);

   eldbus_data_del_all(&conn->data);

   if (conn->idle_enterer) ecore_idle_enterer_del(conn->idle_enterer);
   if (conn->type && conn->shared)
     {
        if (conn->type == ELDBUS_CONNECTION_TYPE_ADDRESS)
           {
              if (address_connections)
                eina_hash_del_by_data(address_connections, conn);
           }
        else if (shared_connections[conn->type - 1] == (void *) conn)
           shared_connections[conn->type - 1] = NULL;
     }

   free(conn);
}

EAPI void
eldbus_connection_unref(Eldbus_Connection *conn)
{
   ELDBUS_CONNECTION_CHECK(conn);
   DBG("conn=%p, pre-refcount=%d", conn, conn->refcount);
   if (--conn->refcount > 0)
      return;
   _eldbus_connection_free(conn);
}

EAPI void
eldbus_connection_free_cb_add(Eldbus_Connection *conn, Eldbus_Free_Cb cb, const void *data)
{
   ELDBUS_CONNECTION_CHECK(conn);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   conn->cbs_free = eldbus_cbs_free_add(conn->cbs_free, cb, data);
}

EAPI void
eldbus_connection_free_cb_del(Eldbus_Connection *conn, Eldbus_Free_Cb cb, const void *data)
{
   ELDBUS_CONNECTION_CHECK(conn);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   conn->cbs_free = eldbus_cbs_free_del(conn->cbs_free, cb, data);
}

EAPI void
eldbus_connection_data_set(Eldbus_Connection *conn, const char *key, const void *data)
{
   ELDBUS_CONNECTION_CHECK(conn);
   EINA_SAFETY_ON_NULL_RETURN(key);
   EINA_SAFETY_ON_NULL_RETURN(data);
   eldbus_data_set(&(conn->data), key, data);
}

EAPI void *
eldbus_connection_data_get(const Eldbus_Connection *conn, const char *key)
{
   ELDBUS_CONNECTION_CHECK_RETVAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key, NULL);
   return eldbus_data_get(&(((Eldbus_Connection *)conn)->data), key);
}

EAPI void *
eldbus_connection_data_del(Eldbus_Connection *conn, const char *key)
{
   ELDBUS_CONNECTION_CHECK_RETVAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key, NULL);
   return eldbus_data_del(&(((Eldbus_Connection *)conn)->data), key);
}

static void
eldbus_dispatch_name_owner_change(Eldbus_Connection_Name *cn, const char *old_id)
{
   Eldbus_Connection_Context_NOC_Cb *ctx;
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
   EINA_LIST_FREE(cn->event_handlers.to_delete, ctx)
     {
        cn->event_handlers.list = eina_inlist_remove(cn->event_handlers.list,
                                                     EINA_INLIST_GET(ctx));
        free(ctx);
     }
   eldbus_connection_name_gc(cn->name_owner_changed->conn, cn);
}

typedef struct _dispach_name_owner_data
{
   Eldbus_Connection_Context_NOC_Cb *ctx;
   const Eldbus_Connection_Name *cn;
} dispatch_name_owner_data;

static Eina_Bool
dispach_name_owner_cb(void *context)
{
   dispatch_name_owner_data *data = context;
   data->ctx->cb((void *)data->ctx->cb_data, data->cn->name, "",
                 data->cn->unique_id);
   data->ctx->idle_enterer = NULL;
   free(data);
   return ECORE_CALLBACK_CANCEL;
}

EAPI void
eldbus_name_owner_changed_callback_add(Eldbus_Connection *conn, const char *bus, Eldbus_Name_Owner_Changed_Cb cb, const void *cb_data, Eina_Bool allow_initial_call)
{
   Eldbus_Connection_Name *cn;
   Eldbus_Connection_Context_NOC_Cb *ctx;

   ELDBUS_CONNECTION_CHECK(conn);
   EINA_SAFETY_ON_NULL_RETURN(bus);
   EINA_SAFETY_ON_NULL_RETURN(cb);

   cn = eldbus_connection_name_get(conn, bus);
   EINA_SAFETY_ON_NULL_RETURN(cn);
   ctx = calloc(1, sizeof(Eldbus_Connection_Context_NOC_Cb));
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
        ctx->idle_enterer = ecore_idle_enterer_add(dispach_name_owner_cb, dispatch_data);
     }
   return;

cleanup:
   eldbus_connection_name_gc(conn, cn);
}

EAPI void
eldbus_name_owner_changed_callback_del(Eldbus_Connection *conn, const char *bus, Eldbus_Name_Owner_Changed_Cb cb, const void *cb_data)
{
   Eldbus_Connection_Name *cn;
   Eldbus_Connection_Context_NOC_Cb *iter, *found = NULL;

   ELDBUS_CONNECTION_CHECK(conn);
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
   if (found->idle_enterer)
     {
        dispatch_name_owner_data *data;
        data = ecore_idle_enterer_del(found->idle_enterer);
        free(data);
     }
   free(found);
   eldbus_connection_name_gc(conn, cn);
}

EAPI void
eldbus_connection_event_callback_add(Eldbus_Connection *conn, Eldbus_Connection_Event_Type type, Eldbus_Connection_Event_Cb cb, const void *cb_data)
{
   Eldbus_Connection_Context_Event *ce;
   Eldbus_Connection_Context_Event_Cb *ctx;

   ELDBUS_CONNECTION_CHECK(conn);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   EINA_SAFETY_ON_TRUE_RETURN(type >= ELDBUS_CONNECTION_EVENT_LAST);

   ce = conn->event_handlers + type;

   ctx = calloc(1, sizeof(Eldbus_Connection_Context_Event_Cb));
   EINA_SAFETY_ON_NULL_RETURN(ctx);
   ctx->cb = cb;
   ctx->cb_data = cb_data;

   ce->list = eina_inlist_append(ce->list, EINA_INLIST_GET(ctx));
}

static void
_eldbus_connection_context_event_cb_del(Eldbus_Connection_Context_Event *ce, Eldbus_Connection_Context_Event_Cb *ctx)
{
   ce->list = eina_inlist_remove(ce->list, EINA_INLIST_GET(ctx));
   free(ctx);
}

EAPI void
eldbus_connection_event_callback_del(Eldbus_Connection *conn, Eldbus_Connection_Event_Type type, Eldbus_Connection_Event_Cb cb, const void *cb_data)
{
   Eldbus_Connection_Context_Event *ce;
   Eldbus_Connection_Context_Event_Cb *iter, *found = NULL;

   ELDBUS_CONNECTION_CHECK(conn);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   EINA_SAFETY_ON_TRUE_RETURN(type >= ELDBUS_CONNECTION_EVENT_LAST);

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

   _eldbus_connection_context_event_cb_del(ce, found);
}

static void
_eldbus_connection_event_callback_call(Eldbus_Connection *conn, Eldbus_Connection_Event_Type type, const void *event_info)
{
   Eldbus_Connection_Context_Event *ce;
   Eldbus_Connection_Context_Event_Cb *iter;

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
     _eldbus_connection_context_event_cb_del(ce, iter);
}

void
eldbus_connection_event_callback_call(Eldbus_Connection *conn, Eldbus_Connection_Event_Type type, const void *event_info)
{
   ELDBUS_CONNECTION_CHECK(conn);
   EINA_SAFETY_ON_TRUE_RETURN(type >= ELDBUS_CONNECTION_EVENT_LAST);
   EINA_SAFETY_ON_TRUE_RETURN(type == ELDBUS_CONNECTION_EVENT_DEL);

   _eldbus_connection_event_callback_call(conn, type, event_info);
}

void
eldbus_connection_signal_handler_add(Eldbus_Connection *conn, Eldbus_Signal_Handler *handler)
{
   ELDBUS_CONNECTION_CHECK(conn);
   EINA_SAFETY_ON_NULL_RETURN(handler);
   conn->signal_handlers = eina_inlist_append(conn->signal_handlers,
                                              EINA_INLIST_GET(handler));
}

void
eldbus_connection_pending_add(Eldbus_Connection *conn, Eldbus_Pending *pending)
{
   ELDBUS_CONNECTION_CHECK(conn);
   EINA_SAFETY_ON_NULL_RETURN(pending);
   conn->pendings = eina_inlist_append(conn->pendings,
                                       EINA_INLIST_GET(pending));
}

void
eldbus_connection_signal_handler_del(Eldbus_Connection *conn, Eldbus_Signal_Handler *handler)
{
   EINA_SAFETY_ON_NULL_RETURN(conn);
   EINA_SAFETY_ON_NULL_RETURN(handler);
   conn->signal_handlers = eina_inlist_remove(conn->signal_handlers,
                                              EINA_INLIST_GET(handler));
}

void
eldbus_connection_pending_del(Eldbus_Connection *conn, Eldbus_Pending *pending)
{
   EINA_SAFETY_ON_NULL_RETURN(conn);
   EINA_SAFETY_ON_NULL_RETURN(pending);
   conn->pendings = eina_inlist_remove(conn->pendings,
                                       EINA_INLIST_GET(pending));
}

const char *
eldbus_connection_unique_name_get(Eldbus_Connection *conn)
{
   ELDBUS_CONNECTION_CHECK_RETVAL(conn, NULL);
   return dbus_bus_get_unique_name(conn->dbus_conn);
}

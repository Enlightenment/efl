#include "edbus_private.h"
#include "edbus_private_types.h"
#include <dbus/dbus.h>

/* TODO: mempool of EDBus_Object, Edbus_Object_Context_Event_Cb and
 * EDBus_Object_Context_Event
 */

#define EDBUS_OBJECT_CHECK(obj)                        \
  do                                                   \
    {                                                  \
       EINA_SAFETY_ON_NULL_RETURN(obj);                \
       if (!EINA_MAGIC_CHECK(obj, EDBUS_OBJECT_MAGIC)) \
         {                                             \
            EINA_MAGIC_FAIL(obj, EDBUS_OBJECT_MAGIC);  \
            return;                                    \
         }                                             \
       EINA_SAFETY_ON_TRUE_RETURN(obj->refcount <= 0); \
    }                                                  \
  while (0)

#define EDBUS_OBJECT_CHECK_RETVAL(obj, retval)                     \
  do                                                               \
    {                                                              \
       EINA_SAFETY_ON_NULL_RETURN_VAL(obj, retval);                \
       if (!EINA_MAGIC_CHECK(obj, EDBUS_OBJECT_MAGIC))             \
         {                                                         \
            EINA_MAGIC_FAIL(obj, EDBUS_OBJECT_MAGIC);              \
            return retval;                                         \
         }                                                         \
       EINA_SAFETY_ON_TRUE_RETURN_VAL(obj->refcount <= 0, retval); \
    }                                                              \
  while (0)

#define EDBUS_OBJECT_CHECK_GOTO(obj, label)                 \
  do                                                        \
    {                                                       \
       EINA_SAFETY_ON_NULL_GOTO(obj, label);                \
       if (!EINA_MAGIC_CHECK(obj, EDBUS_OBJECT_MAGIC))      \
         {                                                  \
            EINA_MAGIC_FAIL(obj, EDBUS_OBJECT_MAGIC);       \
            goto label;                                     \
         }                                                  \
       EINA_SAFETY_ON_TRUE_GOTO(obj->refcount <= 0, label); \
    }                                                       \
  while (0)

Eina_Bool
edbus_object_init(void)
{
   return EINA_TRUE;
}

void
edbus_object_shutdown(void)
{
}

static void _edbus_object_event_callback_call(EDBus_Object *obj, EDBus_Object_Event_Type type, const void *event_info);
static void _edbus_object_context_event_cb_del(EDBus_Object_Context_Event *ce, EDBus_Object_Context_Event_Cb *ctx);
static void _on_connection_free(void *data, const void *dead_pointer);
static void _on_signal_handler_free(void *data, const void *dead_pointer);

static void
_edbus_object_call_del(EDBus_Object *obj)
{
   EDBus_Object_Context_Event *ce;

   _edbus_object_event_callback_call(obj, EDBUS_OBJECT_EVENT_DEL, NULL);

   /* clear all del callbacks so we don't call them twice at
    * _edbus_object_clear()
    */
   ce = obj->event_handlers + EDBUS_OBJECT_EVENT_DEL;
   while (ce->list)
     {
        EDBus_Object_Context_Event_Cb *ctx;

        ctx = EINA_INLIST_CONTAINER_GET(ce->list,
                                        EDBus_Object_Context_Event_Cb);
        _edbus_object_context_event_cb_del(ce, ctx);
     }
}

static void
_edbus_object_clear(EDBus_Object *obj)
{
   EDBus_Signal_Handler *h;
   EDBus_Pending *p;
   Eina_List *iter, *iter_next;
   Eina_Inlist *in_l;
   DBG("obj=%p, refcount=%d, name=%s, path=%s",
       obj, obj->refcount, obj->name, obj->path);

   obj->refcount = 1;
   _edbus_object_call_del(obj);
   edbus_connection_name_object_del(obj->conn, obj);

   /* NOTE: obj->proxies is deleted from obj->cbs_free. */

   EINA_LIST_FOREACH_SAFE(obj->signal_handlers, iter, iter_next, h)
     {
        DBG("obj=%p delete owned signal handler %p %s",
            obj, h, edbus_signal_handler_match_get(h));
        edbus_signal_handler_del(h);
     }
   EINA_INLIST_FOREACH_SAFE(obj->pendings, in_l, p)
     {
        DBG("obj=%p delete owned pending call=%p dest=%s path=%s %s.%s()",
            obj, p,
            edbus_pending_destination_get(p),
            edbus_pending_path_get(p),
            edbus_pending_interface_get(p),
            edbus_pending_method_get(p));
        edbus_pending_cancel(p);
     }

   edbus_cbs_free_dispatch(&(obj->cbs_free), obj);
   obj->refcount = 0;
}

static void
_edbus_object_free(EDBus_Object *obj)
{
   unsigned int i;
   EDBus_Signal_Handler *h;

   if (obj->proxies)
     {
        Eina_Iterator *iterator = eina_hash_iterator_data_new(obj->proxies);
        EDBus_Proxy *proxy;
        EINA_ITERATOR_FOREACH(iterator, proxy)
          ERR("obj=%p alive proxy=%p %s", obj, proxy,
              edbus_proxy_interface_get(proxy));
        eina_iterator_free(iterator);
        eina_hash_free(obj->proxies);
     }

   EINA_LIST_FREE(obj->signal_handlers, h)
     {
        if (h->dangling)
          edbus_signal_handler_cb_free_del(h, _on_signal_handler_free, obj);
        else
          ERR("obj=%p alive handler=%p %s", obj, h,
              edbus_signal_handler_match_get(h));
     }

   if (obj->pendings)
     CRITICAL("Object %p released with live pending calls!", obj);

   for (i = 0; i < EDBUS_OBJECT_EVENT_LAST; i++)
     {
        EDBus_Object_Context_Event *ce = obj->event_handlers + i;
        while (ce->list)
          {
             EDBus_Object_Context_Event_Cb *ctx;

             ctx = EINA_INLIST_CONTAINER_GET(ce->list,
                                             EDBus_Object_Context_Event_Cb);
             _edbus_object_context_event_cb_del(ce, ctx);
          }
        eina_list_free(ce->to_delete);
     }

   eina_stringshare_del(obj->name);
   eina_stringshare_del(obj->path);
   EINA_MAGIC_SET(obj, EINA_MAGIC_NONE);

   free(obj);
}

static void
_on_connection_free(void *data, const void *dead_pointer)
{
   EDBus_Object *obj = data;
   EDBUS_OBJECT_CHECK(obj);
   _edbus_object_clear(obj);
   _edbus_object_free(obj);
}

EAPI EDBus_Object *
edbus_object_get(EDBus_Connection *conn, const char *bus, const char *path)
{
   EDBus_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(bus, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);

   obj = edbus_connection_name_object_get(conn, bus, path);
   if (obj) return obj;

   obj = calloc(1, sizeof(EDBus_Object));
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);

   obj->conn = conn;
   obj->refcount = 1;
   obj->path = eina_stringshare_add(path);
   obj->name = eina_stringshare_add(bus);
   obj->proxies = eina_hash_string_small_new(NULL);
   EINA_SAFETY_ON_NULL_GOTO(obj->proxies, cleanup);
   EINA_MAGIC_SET(obj, EDBUS_OBJECT_MAGIC);

   edbus_connection_name_object_set(conn, obj);
   edbus_connection_cb_free_add(obj->conn, _on_connection_free, obj);

   return obj;

cleanup:
   eina_stringshare_del(obj->path);
   eina_stringshare_del(obj->name);
   free(obj);

   return NULL;
}

static void _on_signal_handler_free(void *data, const void *dead_pointer);

static void
_edbus_object_unref(EDBus_Object *obj)
{
   obj->refcount--;
   if (obj->refcount > 0) return;

   edbus_connection_cb_free_del(obj->conn, _on_connection_free, obj);
   _edbus_object_clear(obj);
   _edbus_object_free(obj);
}

EAPI EDBus_Object *
edbus_object_ref(EDBus_Object *obj)
{
   EDBUS_OBJECT_CHECK_RETVAL(obj, NULL);
   DBG("obj=%p, pre-refcount=%d, name=%s, path=%s",
       obj, obj->refcount, obj->name, obj->path);
   obj->refcount++;
   return obj;
}

EAPI void
edbus_object_unref(EDBus_Object *obj)
{
   EDBUS_OBJECT_CHECK(obj);
   DBG("obj=%p, pre-refcount=%d, name=%s, path=%s",
       obj, obj->refcount, obj->name, obj->path);
   _edbus_object_unref(obj);
}

EAPI void
edbus_object_cb_free_add(EDBus_Object *obj, EDBus_Free_Cb cb, const void *data)
{
   EDBUS_OBJECT_CHECK(obj);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   obj->cbs_free = edbus_cbs_free_add(obj->cbs_free, cb, data);
}

EAPI void
edbus_object_cb_free_del(EDBus_Object *obj, EDBus_Free_Cb cb, const void *data)
{
   EDBUS_OBJECT_CHECK(obj);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   obj->cbs_free = edbus_cbs_free_del(obj->cbs_free, cb, data);
}

EAPI void
edbus_object_event_callback_add(EDBus_Object *obj, EDBus_Object_Event_Type type, EDBus_Object_Event_Cb cb, const void *cb_data)
{
   EDBus_Object_Context_Event *ce;
   EDBus_Object_Context_Event_Cb *ctx;

   EDBUS_OBJECT_CHECK(obj);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   EINA_SAFETY_ON_TRUE_RETURN(type >= EDBUS_OBJECT_EVENT_LAST);

   ce = obj->event_handlers + type;

   ctx = calloc(1, sizeof(EDBus_Object_Context_Event_Cb));
   EINA_SAFETY_ON_NULL_RETURN(ctx);

   ctx->cb = cb;
   ctx->cb_data = cb_data;

   ce->list = eina_inlist_append(ce->list, EINA_INLIST_GET(ctx));
}

static void
_edbus_object_context_event_cb_del(EDBus_Object_Context_Event *ce, EDBus_Object_Context_Event_Cb *ctx)
{
   ce->list = eina_inlist_remove(ce->list, EINA_INLIST_GET(ctx));
   free(ctx);
}

EAPI void
edbus_object_event_callback_del(EDBus_Object *obj, EDBus_Object_Event_Type type, EDBus_Object_Event_Cb cb, const void *cb_data)
{
   EDBus_Object_Context_Event *ce;
   EDBus_Object_Context_Event_Cb *iter, *found = NULL;

   EDBUS_OBJECT_CHECK(obj);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   EINA_SAFETY_ON_TRUE_RETURN(type >= EDBUS_OBJECT_EVENT_LAST);

   ce = obj->event_handlers + type;

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

   _edbus_object_context_event_cb_del(ce, found);
}

static void
_edbus_object_event_callback_call(EDBus_Object *obj, EDBus_Object_Event_Type type, const void *event_info)
{
   EDBus_Object_Context_Event *ce;
   EDBus_Object_Context_Event_Cb *iter;

   ce = obj->event_handlers + type;

   ce->walking++;
   EINA_INLIST_FOREACH(ce->list, iter)
     {
        if (iter->deleted) continue;
        iter->cb((void *)iter->cb_data, obj, (void *)event_info);
     }
   ce->walking--;
   if (ce->walking > 0) return;

   EINA_LIST_FREE(ce->to_delete, iter)
     _edbus_object_context_event_cb_del(ce, iter);
}

EAPI EDBus_Connection *
edbus_object_connection_get(const EDBus_Object *obj)
{
   EDBUS_OBJECT_CHECK_RETVAL(obj, NULL);
   return obj->conn;
}

EAPI const char *
edbus_object_bus_name_get(const EDBus_Object *obj)
{
   EDBUS_OBJECT_CHECK_RETVAL(obj, NULL);
   return obj->name;
}

EAPI const char *
edbus_object_bus_path_get(const EDBus_Object *obj)
{
   EDBUS_OBJECT_CHECK_RETVAL(obj, NULL);
   return obj->path;
}

static void
_on_pending_free(void *data, const void *dead_pointer)
{
   EDBus_Object *obj = data;
   EDBus_Pending *pending = (EDBus_Pending*) dead_pointer;
   EDBUS_OBJECT_CHECK(obj);
   obj->pendings = eina_inlist_remove(obj->pendings, EINA_INLIST_GET(pending));
}

EAPI EDBus_Pending *
edbus_object_send(EDBus_Object *obj, EDBus_Message *msg, EDBus_Message_Cb cb, const void *cb_data, double timeout)
{
   EDBus_Pending *pending;

   EDBUS_OBJECT_CHECK_RETVAL(obj, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(msg, NULL);

   pending = _edbus_connection_send(obj->conn, msg, cb, cb_data, timeout);
   if (!cb) return NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(pending, NULL);

   edbus_pending_cb_free_add(pending, _on_pending_free, obj);
   obj->pendings = eina_inlist_append(obj->pendings, EINA_INLIST_GET(pending));

   return pending;
}

static void
_on_signal_handler_free(void *data, const void *dead_pointer)
{
   EDBus_Object *obj = data;
   EDBUS_OBJECT_CHECK(obj);
   obj->signal_handlers = eina_list_remove(obj->signal_handlers, dead_pointer);
}

EAPI EDBus_Signal_Handler *
edbus_object_signal_handler_add(EDBus_Object *obj, const char *interface, const char *member, EDBus_Signal_Cb cb, const void *cb_data)
{
   EDBus_Signal_Handler *handler;

   EDBUS_OBJECT_CHECK_RETVAL(obj, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cb, NULL);

   handler = edbus_signal_handler_add(obj->conn, obj->name, obj->path,
                                      interface, member, cb, cb_data);
   EINA_SAFETY_ON_NULL_RETURN_VAL(handler, NULL);

   edbus_signal_handler_cb_free_add(handler, _on_signal_handler_free, obj);
   obj->signal_handlers = eina_list_append(obj->signal_handlers, handler);

   return handler;
}

Eina_Bool
edbus_object_proxy_add(EDBus_Object *obj, EDBus_Proxy *proxy)
{
   return eina_hash_add(obj->proxies, edbus_proxy_interface_get(proxy), proxy);
}

EDBus_Proxy *
edbus_object_proxy_get(EDBus_Object *obj, const char *interface)
{
   return eina_hash_find(obj->proxies, interface);
}

Eina_Bool
edbus_object_proxy_del(EDBus_Object *obj, EDBus_Proxy *proxy, const char *interface)
{
   return eina_hash_del(obj->proxies, interface, proxy);
}

static EDBus_Proxy *
get_peer_proxy(EDBus_Object *obj)
{
   return edbus_proxy_get(obj, "org.freedesktop.DBus.Peer");
}

EAPI EDBus_Pending *
edbus_object_peer_ping(EDBus_Object *obj, EDBus_Message_Cb cb, const void *data)
{
   EDBUS_OBJECT_CHECK_RETVAL(obj, NULL);
   return edbus_proxy_call(get_peer_proxy(obj), "Ping", cb,
                           data, -1, "");
}

EAPI EDBus_Pending *
edbus_object_peer_machine_id_get(EDBus_Object *obj, EDBus_Message_Cb cb, const void *data)
{
   EDBUS_OBJECT_CHECK_RETVAL(obj, NULL);
   return edbus_proxy_call(get_peer_proxy(obj), "GetMachineId", cb,
                           data, -1, "");
}

EAPI EDBus_Pending *
edbus_object_introspect(EDBus_Object *obj, EDBus_Message_Cb cb, const void *data)
{
   EDBus_Proxy *introspectable;
   EDBUS_OBJECT_CHECK_RETVAL(obj, NULL);

   introspectable = edbus_proxy_get(obj, "org.freedesktop.DBus.Introspectable");
   return edbus_proxy_call(introspectable, "Introspect", cb, data, -1, "");
}

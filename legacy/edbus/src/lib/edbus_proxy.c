#include "edbus_private.h"
#include "edbus_private_types.h"

/* TODO: mempool of EDBus_Proxy, Edbus_Proxy_Context_Event_Cb and
 * EDBus_Proxy_Context_Event
 */

typedef struct _EDBus_Proxy_Context_Event_Cb
{
   EINA_INLIST;
   EDBus_Proxy_Event_Cb cb;
   const void          *cb_data;
   Eina_Bool            deleted : 1;
} EDBus_Proxy_Context_Event_Cb;

typedef struct _EDBus_Proxy_Context_Event
{
   Eina_Inlist *list;
   int          walking;
   Eina_List   *to_delete;
} EDBus_Proxy_Context_Event;

struct _EDBus_Proxy
{
   EINA_MAGIC;
   int                       refcount;
   EDBus_Object             *obj;
   const char               *interface;
   Eina_Inlist              *pendings;
   Eina_List                *handlers;
   Eina_Inlist              *cbs_free;
   EDBus_Proxy_Context_Event event_handlers[EDBUS_PROXY_EVENT_LAST];
};

#define EDBUS_PROXY_CHECK(proxy)                         \
  do                                                     \
    {                                                    \
       EINA_SAFETY_ON_NULL_RETURN(proxy);                \
       if (!EINA_MAGIC_CHECK(proxy, EDBUS_PROXY_MAGIC))  \
         {                                               \
            EINA_MAGIC_FAIL(proxy, EDBUS_PROXY_MAGIC);   \
            return;                                      \
         }                                               \
       EINA_SAFETY_ON_TRUE_RETURN(proxy->refcount <= 0); \
    }                                                    \
  while (0)

#define EDBUS_PROXY_CHECK_RETVAL(proxy, retval)                      \
  do                                                                 \
    {                                                                \
       EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, retval);                \
       if (!EINA_MAGIC_CHECK(proxy, EDBUS_PROXY_MAGIC))              \
         {                                                           \
            EINA_MAGIC_FAIL(proxy, EDBUS_PROXY_MAGIC);               \
            return retval;                                           \
         }                                                           \
       EINA_SAFETY_ON_TRUE_RETURN_VAL(proxy->refcount <= 0, retval); \
    }                                                                \
  while (0)

#define EDBUS_PROXY_CHECK_GOTO(proxy, label)                  \
  do                                                          \
    {                                                         \
       EINA_SAFETY_ON_NULL_GOTO(proxy, label);                \
       if (!EINA_MAGIC_CHECK(proxy, EDBUS_PROXY_MAGIC))       \
         {                                                    \
            EINA_MAGIC_FAIL(proxy, EDBUS_PROXY_MAGIC);        \
            goto label;                                       \
         }                                                    \
       EINA_SAFETY_ON_TRUE_GOTO(proxy->refcount <= 0, label); \
    }                                                         \
  while (0)

Eina_Bool
edbus_proxy_init(void)
{
   return EINA_TRUE;
}

void
edbus_proxy_shutdown(void)
{
}

static void _edbus_proxy_event_callback_call(EDBus_Proxy *proxy, EDBus_Proxy_Event_Type type, const void *event_info);
static void _edbus_proxy_context_event_cb_del(EDBus_Proxy_Context_Event *ce, EDBus_Proxy_Context_Event_Cb *ctx);
static void _on_signal_handler_free(void *data, const void *dead_pointer);

static void
_edbus_proxy_call_del(EDBus_Proxy *proxy)
{
   EDBus_Proxy_Context_Event *ce;

   _edbus_proxy_event_callback_call(proxy, EDBUS_PROXY_EVENT_DEL, NULL);

   /* clear all del callbacks so we don't call them twice at
    * _edbus_proxy_clear()
    */
   ce = proxy->event_handlers + EDBUS_PROXY_EVENT_DEL;
   while (ce->list)
     {
        EDBus_Proxy_Context_Event_Cb *ctx;

        ctx = EINA_INLIST_CONTAINER_GET(ce->list,
                                        EDBus_Proxy_Context_Event_Cb);
        _edbus_proxy_context_event_cb_del(ce, ctx);
     }
}

static void
_edbus_proxy_clear(EDBus_Proxy *proxy)
{
   EDBus_Signal_Handler *h;
   EDBus_Pending *p;
   Eina_List *iter, *iter_next;
   Eina_Inlist *in_l;
   DBG("proxy=%p, refcount=%d, interface=%s, obj=%p",
       proxy, proxy->refcount, proxy->interface, proxy->obj);
   proxy->refcount = 1;
   edbus_object_proxy_del(proxy->obj, proxy, proxy->interface);
   _edbus_proxy_call_del(proxy);

   EINA_LIST_FOREACH_SAFE(proxy->handlers, iter, iter_next, h)
     {
        DBG("proxy=%p delete owned signal handler %p %s",
            proxy, h, edbus_signal_handler_match_get(h));
        edbus_signal_handler_del(h);
     }

   EINA_INLIST_FOREACH_SAFE(proxy->pendings, in_l, p)
     {
        DBG("proxy=%p delete owned pending call=%p dest=%s path=%s %s.%s()",
            proxy, p,
            edbus_pending_destination_get(p),
            edbus_pending_path_get(p),
            edbus_pending_interface_get(p),
            edbus_pending_method_get(p));
        edbus_pending_cancel(p);
     }

   edbus_cbs_free_dispatch(&(proxy->cbs_free), proxy);
   proxy->refcount = 0;
}

static void
_edbus_proxy_free(EDBus_Proxy *proxy)
{
   unsigned int i;
   EDBus_Signal_Handler *h;

   EINA_LIST_FREE(proxy->handlers, h)
     {
        if (h->dangling)
	  edbus_signal_handler_cb_free_del(h, _on_signal_handler_free, proxy);
        else
           ERR("proxy=%p alive handler=%p %s", proxy, h,
               edbus_signal_handler_match_get(h));
     }

   if (proxy->pendings)
     CRITICAL("Proxy %p released with live pending calls!", proxy);

   for (i = 0; i < EDBUS_PROXY_EVENT_LAST; i++)
     {
        EDBus_Proxy_Context_Event *ce = proxy->event_handlers + i;
        while (ce->list)
          {
             EDBus_Proxy_Context_Event_Cb *ctx;
             ctx = EINA_INLIST_CONTAINER_GET(ce->list,
                                             EDBus_Proxy_Context_Event_Cb);
             _edbus_proxy_context_event_cb_del(ce, ctx);
          }
        eina_list_free(ce->to_delete);
     }

   eina_stringshare_del(proxy->interface);
   EINA_MAGIC_SET(proxy, EINA_MAGIC_NONE);
   free(proxy);
}

static void
_on_object_free(void *data, const void *dead_pointer)
{
   EDBus_Proxy *proxy = data;
   EDBUS_PROXY_CHECK(proxy);
   DBG("proxy=%p, refcount=%d, interface=%s, obj=%p",
       proxy, proxy->refcount, proxy->interface, proxy->obj);
   _edbus_proxy_clear(proxy);
   _edbus_proxy_free(proxy);
}

EAPI EDBus_Proxy *
edbus_proxy_get(EDBus_Object *obj, const char *interface)
{
   EDBus_Proxy *proxy;

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(interface, NULL);

   proxy = edbus_object_proxy_get(obj, interface);
   if (proxy) return proxy;

   proxy = calloc(1, sizeof(EDBus_Proxy));
   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);

   proxy->refcount = 1;
   proxy->obj = obj;
   proxy->interface = eina_stringshare_add(interface);
   EINA_MAGIC_SET(proxy, EDBUS_PROXY_MAGIC);
   if (!edbus_object_proxy_add(obj, proxy))
     goto cleanup;
   edbus_object_cb_free_add(obj, _on_object_free, proxy);

   return proxy;

cleanup:
   eina_stringshare_del(proxy->interface);
   free(proxy);
   return NULL;
}

static void _on_signal_handler_free(void *data, const void *dead_pointer);

static void
_edbus_proxy_unref(EDBus_Proxy *proxy)
{
   proxy->refcount--;
   if (proxy->refcount > 0) return;

   edbus_object_cb_free_del(proxy->obj, _on_object_free, proxy);
   _edbus_proxy_clear(proxy);
   _edbus_proxy_free(proxy);
}

EAPI EDBus_Proxy *
edbus_proxy_ref(EDBus_Proxy *proxy)
{
   EDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   DBG("proxy=%p, pre-refcount=%d, interface=%s, obj=%p",
       proxy, proxy->refcount, proxy->interface, proxy->obj);
   proxy->refcount++;
   return proxy;
}

EAPI void
edbus_proxy_unref(EDBus_Proxy *proxy)
{
   EDBUS_PROXY_CHECK(proxy);
   DBG("proxy=%p, pre-refcount=%d, interface=%s, obj=%p",
       proxy, proxy->refcount, proxy->interface, proxy->obj);
   _edbus_proxy_unref(proxy);
}

EAPI void
edbus_proxy_cb_free_add(EDBus_Proxy *proxy, EDBus_Free_Cb cb, const void *data)
{
   EDBUS_PROXY_CHECK(proxy);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   proxy->cbs_free = edbus_cbs_free_add(proxy->cbs_free, cb, data);
}

EAPI void
edbus_proxy_cb_free_del(EDBus_Proxy *proxy, EDBus_Free_Cb cb, const void *data)
{
   EDBUS_PROXY_CHECK(proxy);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   proxy->cbs_free = edbus_cbs_free_del(proxy->cbs_free, cb, data);
}

EAPI void
edbus_proxy_event_callback_add(EDBus_Proxy *proxy, EDBus_Proxy_Event_Type type, EDBus_Proxy_Event_Cb cb, const void *cb_data)
{
   EDBus_Proxy_Context_Event *ce;
   EDBus_Proxy_Context_Event_Cb *ctx;

   EDBUS_PROXY_CHECK(proxy);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   EINA_SAFETY_ON_TRUE_RETURN(type >= EDBUS_PROXY_EVENT_LAST);

   ce = proxy->event_handlers + type;

   ctx = calloc(1, sizeof(EDBus_Proxy_Context_Event_Cb));
   EINA_SAFETY_ON_NULL_RETURN(ctx);
   ctx->cb = cb;
   ctx->cb_data = cb_data;

   ce->list = eina_inlist_append(ce->list, EINA_INLIST_GET(ctx));
}

static void
_edbus_proxy_context_event_cb_del(EDBus_Proxy_Context_Event *ce, EDBus_Proxy_Context_Event_Cb *ctx)
{
   ce->list = eina_inlist_remove(ce->list, EINA_INLIST_GET(ctx));
   free(ctx);
}

EAPI void
edbus_proxy_event_callback_del(EDBus_Proxy *proxy, EDBus_Proxy_Event_Type type, EDBus_Proxy_Event_Cb cb, const void *cb_data)
{
   EDBus_Proxy_Context_Event *ce;
   EDBus_Proxy_Context_Event_Cb *iter, *found = NULL;

   EDBUS_PROXY_CHECK(proxy);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   EINA_SAFETY_ON_TRUE_RETURN(type >= EDBUS_PROXY_EVENT_LAST);

   ce = proxy->event_handlers + type;

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

   _edbus_proxy_context_event_cb_del(ce, found);
}

static void
_edbus_proxy_event_callback_call(EDBus_Proxy *proxy, EDBus_Proxy_Event_Type type, const void *event_info)
{
   EDBus_Proxy_Context_Event *ce;
   EDBus_Proxy_Context_Event_Cb *iter;

   ce = proxy->event_handlers + type;

   ce->walking++;
   EINA_INLIST_FOREACH(ce->list, iter)
     {
        if (iter->deleted) continue;
        iter->cb((void *)iter->cb_data, proxy, (void *)event_info);
     }
   ce->walking--;
   if (ce->walking > 0) return;

   EINA_LIST_FREE(ce->to_delete, iter)
     _edbus_proxy_context_event_cb_del(ce, iter);
}

EAPI EDBus_Object *
edbus_proxy_object_get(const EDBus_Proxy *proxy)
{
   EDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   return proxy->obj;
}

EAPI const char *
edbus_proxy_interface_get(const EDBus_Proxy *proxy)
{
   EDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   return proxy->interface;
}

static void
_on_pending_free(void *data, const void *dead_pointer)
{
   EDBus_Proxy *proxy = data;
   EDBus_Pending *pending = (EDBus_Pending *)dead_pointer;
   EDBUS_PROXY_CHECK(proxy);
   proxy->pendings = eina_inlist_remove(proxy->pendings,
                                        EINA_INLIST_GET(pending));
}

static EDBus_Pending *
_edbus_proxy_send(EDBus_Proxy *proxy, EDBus_Message *msg, EDBus_Message_Cb cb, const void *cb_data, double timeout)
{
   EDBus_Pending *pending;

   pending = _edbus_connection_send(proxy->obj->conn, msg, cb, cb_data, timeout);
   if (!cb) return NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(pending, NULL);

   edbus_pending_cb_free_add(pending, _on_pending_free, proxy);
   proxy->pendings = eina_inlist_append(proxy->pendings,
                                        EINA_INLIST_GET(pending));

   return pending;
}

EAPI EDBus_Pending *
edbus_proxy_send(EDBus_Proxy *proxy, EDBus_Message *msg, EDBus_Message_Cb cb, const void *cb_data, double timeout)
{
   EDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(msg, NULL);

   return _edbus_proxy_send(proxy, msg, cb, cb_data, timeout);
}

EAPI EDBus_Message *
edbus_proxy_method_call_new(EDBus_Proxy *proxy, const char *member)
{
   EDBus_Message *msg;
   EDBUS_PROXY_CHECK_RETVAL(proxy, NULL);

   msg = edbus_message_method_call_new(
                           edbus_object_bus_name_get(proxy->obj),
                           edbus_object_bus_path_get(proxy->obj),
                           proxy->interface, member);
   return msg;
}

static EDBus_Pending *
_edbus_proxy_vcall(EDBus_Proxy *proxy, const char *member, EDBus_Message_Cb cb, const void *cb_data, double timeout, const char *signature, va_list ap)
{
   EDBus_Pending *pending;
   EDBus_Message *msg = edbus_proxy_method_call_new(proxy, member);
   EINA_SAFETY_ON_NULL_RETURN_VAL(msg, NULL);

   if (!edbus_message_arguments_vset(msg, signature, ap))
     {
        edbus_message_unref(msg);
        ERR("Error setting arguments");
        return NULL;
     }

   pending = _edbus_proxy_send(proxy, msg, cb, cb_data, timeout);
   edbus_message_unref(msg);
   return pending;
}

EAPI EDBus_Pending *
edbus_proxy_call(EDBus_Proxy *proxy, const char *member, EDBus_Message_Cb cb, const void *cb_data, double timeout, const char *signature, ...)
{
   EDBus_Pending *pending;
   va_list ap;

   EDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(member, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(signature, NULL);

   va_start(ap, signature);
   pending = _edbus_proxy_vcall(proxy, member, cb, cb_data, timeout,
                                signature, ap);
   va_end(ap);

   return pending;
}

EAPI EDBus_Pending *
edbus_proxy_vcall(EDBus_Proxy *proxy, const char *member, EDBus_Message_Cb cb, const void *cb_data, double timeout, const char *signature, va_list ap)
{
   EDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(member, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(signature, NULL);

   return _edbus_proxy_vcall(proxy, member, cb, cb_data, timeout,
                             signature, ap);
}

static void
_on_signal_handler_free(void *data, const void *dead_pointer)
{
   EDBus_Proxy *proxy = data;
   EDBUS_PROXY_CHECK(proxy);
   proxy->handlers = eina_list_remove(proxy->handlers, dead_pointer);
}

EAPI EDBus_Signal_Handler *
edbus_proxy_signal_handler_add(EDBus_Proxy *proxy, const char *member, EDBus_Signal_Cb cb, const void *cb_data)
{
   EDBus_Signal_Handler *handler;
   const char *name, *path;

   EDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cb, NULL);

   name = edbus_object_bus_name_get(proxy->obj);
   path = edbus_object_bus_path_get(proxy->obj);

   handler = edbus_signal_handler_add(proxy->obj->conn, name, path,
                                      proxy->interface, member, cb, cb_data);
   EINA_SAFETY_ON_NULL_RETURN_VAL(handler, NULL);

   edbus_signal_handler_cb_free_add(handler, _on_signal_handler_free, proxy);
   proxy->handlers = eina_list_append(proxy->handlers, handler);

   return handler;
}

static EDBus_Proxy *
get_properties_proxy(EDBus_Proxy *proxy)
{
   return edbus_proxy_get(proxy->obj, EDBUS_FDO_INTERFACE_PROPERTIES);
}

EAPI EDBus_Pending *
edbus_proxy_property_get(EDBus_Proxy *proxy, const char *name, EDBus_Message_Cb cb, const void *data)
{
   EDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);
   return edbus_proxy_call(get_properties_proxy(proxy), "Get", cb, data, -1,
                           "ss", proxy->interface, name);
}

EAPI EDBus_Pending *
edbus_proxy_property_set(EDBus_Proxy *proxy, const char *name, char type, const void *value, EDBus_Message_Cb cb, const void *data)
{
   EDBus_Message *msg;
   EDBus_Message_Iter *iter, *variant;
   EDBus_Pending *pending;
   char sig[2];
   EDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   if (!dbus_type_is_basic(type))
     {
        ERR("Only basic types may be set using edbus_proxy_property_set()");
        return NULL;
     }

   sig[0] = type;
   sig[1] = 0;
   msg = edbus_proxy_method_call_new(get_properties_proxy(proxy), "Set");
   iter = edbus_message_iter_get(msg);
   edbus_message_iter_basic_append(iter, 's', proxy->interface);
   edbus_message_iter_basic_append(iter, 's', name);
   variant = edbus_message_iter_container_new(iter, 'v', sig);
   edbus_message_iter_basic_append(variant, type, value);
   edbus_message_iter_container_close(iter, variant);

   pending = edbus_proxy_send(get_properties_proxy(proxy), msg, cb, data, -1);
   edbus_message_unref(msg);

   return pending;
}

EAPI EDBus_Pending *
edbus_proxy_property_get_all(EDBus_Proxy *proxy, EDBus_Message_Cb cb, const void *data)
{
   EDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   return edbus_proxy_call(get_properties_proxy(proxy), "GetAll", cb, data, -1,
                           "s", proxy->interface);
}

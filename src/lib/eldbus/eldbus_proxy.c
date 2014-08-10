#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eldbus_private.h"
#include "eldbus_private_types.h"

/* TODO: mempool of Eldbus_Proxy, Eldbus_Proxy_Context_Event_Cb and
 * Eldbus_Proxy_Context_Event
 */

typedef struct _Eldbus_Proxy_Context_Event_Cb
{
   EINA_INLIST;
   Eldbus_Proxy_Event_Cb cb;
   const void          *cb_data;
   Eina_Bool            deleted : 1;
} Eldbus_Proxy_Context_Event_Cb;

typedef struct _Eldbus_Proxy_Context_Event
{
   Eina_Inlist *list;
   int          walking;
   Eina_List   *to_delete;
} Eldbus_Proxy_Context_Event;

struct _Eldbus_Proxy
{
   EINA_MAGIC;
   int                       refcount;
   Eldbus_Object             *obj;
   const char               *interface;
   Eina_Inlist              *pendings;
   Eina_List                *handlers;
   Eina_Inlist              *cbs_free;
   Eina_Inlist              *data;
   Eldbus_Proxy_Context_Event event_handlers[ELDBUS_PROXY_EVENT_LAST];
   Eina_Hash *props;
   Eldbus_Signal_Handler *properties_changed;
   Eina_Bool monitor_enabled:1;
};

#define ELDBUS_PROXY_CHECK(proxy)                         \
  do                                                     \
    {                                                    \
       EINA_SAFETY_ON_NULL_RETURN(proxy);                \
       if (!EINA_MAGIC_CHECK(proxy, ELDBUS_PROXY_MAGIC))  \
         {                                               \
            EINA_MAGIC_FAIL(proxy, ELDBUS_PROXY_MAGIC);   \
            return;                                      \
         }                                               \
       EINA_SAFETY_ON_TRUE_RETURN(proxy->refcount <= 0); \
    }                                                    \
  while (0)

#define ELDBUS_PROXY_CHECK_RETVAL(proxy, retval)                      \
  do                                                                 \
    {                                                                \
       EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, retval);                \
       if (!EINA_MAGIC_CHECK(proxy, ELDBUS_PROXY_MAGIC))              \
         {                                                           \
            EINA_MAGIC_FAIL(proxy, ELDBUS_PROXY_MAGIC);               \
            return retval;                                           \
         }                                                           \
       EINA_SAFETY_ON_TRUE_RETURN_VAL(proxy->refcount <= 0, retval); \
    }                                                                \
  while (0)

#define ELDBUS_PROXY_CHECK_GOTO(proxy, label)                  \
  do                                                          \
    {                                                         \
       EINA_SAFETY_ON_NULL_GOTO(proxy, label);                \
       if (!EINA_MAGIC_CHECK(proxy, ELDBUS_PROXY_MAGIC))       \
         {                                                    \
            EINA_MAGIC_FAIL(proxy, ELDBUS_PROXY_MAGIC);        \
            goto label;                                       \
         }                                                    \
       EINA_SAFETY_ON_TRUE_GOTO(proxy->refcount <= 0, label); \
    }                                                         \
  while (0)

Eina_Bool
eldbus_proxy_init(void)
{
   return EINA_TRUE;
}

void
eldbus_proxy_shutdown(void)
{
}

static void _eldbus_proxy_event_callback_call(Eldbus_Proxy *proxy, Eldbus_Proxy_Event_Type type, const void *event_info);
static void _eldbus_proxy_context_event_cb_del(Eldbus_Proxy_Context_Event *ce, Eldbus_Proxy_Context_Event_Cb *ctx);
static void _on_signal_handler_free(void *data, const void *dead_pointer);

static void
_eldbus_proxy_call_del(Eldbus_Proxy *proxy)
{
   Eldbus_Proxy_Context_Event *ce;

   _eldbus_proxy_event_callback_call(proxy, ELDBUS_PROXY_EVENT_DEL, NULL);

   /* clear all del callbacks so we don't call them twice at
    * _eldbus_proxy_clear()
    */
   ce = proxy->event_handlers + ELDBUS_PROXY_EVENT_DEL;
   while (ce->list)
     {
        Eldbus_Proxy_Context_Event_Cb *ctx;

        ctx = EINA_INLIST_CONTAINER_GET(ce->list,
                                        Eldbus_Proxy_Context_Event_Cb);
        _eldbus_proxy_context_event_cb_del(ce, ctx);
     }
}

static void
_eldbus_proxy_clear(Eldbus_Proxy *proxy)
{
   Eldbus_Signal_Handler *h;
   Eldbus_Pending *p;
   Eina_List *iter, *iter_next;
   Eina_Inlist *in_l;
   DBG("proxy=%p, refcount=%d, interface=%s, obj=%p",
       proxy, proxy->refcount, proxy->interface, proxy->obj);
   proxy->refcount = 1;
   eldbus_object_proxy_del(proxy->obj, proxy, proxy->interface);
   _eldbus_proxy_call_del(proxy);

   EINA_LIST_FOREACH_SAFE(proxy->handlers, iter, iter_next, h)
     {
        DBG("proxy=%p delete owned signal handler %p %s",
            proxy, h, eldbus_signal_handler_match_get(h));
        eldbus_signal_handler_del(h);
     }

   EINA_INLIST_FOREACH_SAFE(proxy->pendings, in_l, p)
     {
        DBG("proxy=%p delete owned pending call=%p dest=%s path=%s %s.%s()",
            proxy, p,
            eldbus_pending_destination_get(p),
            eldbus_pending_path_get(p),
            eldbus_pending_interface_get(p),
            eldbus_pending_method_get(p));
        eldbus_pending_cancel(p);
     }

   eldbus_cbs_free_dispatch(&(proxy->cbs_free), proxy);
   if (proxy->props)
     eina_hash_free(proxy->props);
   proxy->refcount = 0;
}

static void
_eldbus_proxy_free(Eldbus_Proxy *proxy)
{
   unsigned int i;
   Eldbus_Signal_Handler *h;

   EINA_LIST_FREE(proxy->handlers, h)
     {
        if (h->dangling)
	  eldbus_signal_handler_free_cb_del(h, _on_signal_handler_free, proxy);
        else
           ERR("proxy=%p alive handler=%p %s", proxy, h,
               eldbus_signal_handler_match_get(h));
     }

   if (proxy->pendings)
     CRI("Proxy %p released with live pending calls!", proxy);

   for (i = 0; i < ELDBUS_PROXY_EVENT_LAST; i++)
     {
        Eldbus_Proxy_Context_Event *ce = proxy->event_handlers + i;
        while (ce->list)
          {
             Eldbus_Proxy_Context_Event_Cb *ctx;
             ctx = EINA_INLIST_CONTAINER_GET(ce->list,
                                             Eldbus_Proxy_Context_Event_Cb);
             _eldbus_proxy_context_event_cb_del(ce, ctx);
          }
        eina_list_free(ce->to_delete);
     }

   eina_stringshare_del(proxy->interface);
   EINA_MAGIC_SET(proxy, EINA_MAGIC_NONE);
   free(proxy);
}

static void
_on_object_free(void *data, const void *dead_pointer EINA_UNUSED)
{
   Eldbus_Proxy *proxy = data;
   ELDBUS_PROXY_CHECK(proxy);
   DBG("proxy=%p, refcount=%d, interface=%s, obj=%p",
       proxy, proxy->refcount, proxy->interface, proxy->obj);
   eldbus_data_del_all(&(proxy->data));
   _eldbus_proxy_clear(proxy);
   _eldbus_proxy_free(proxy);
}

EAPI Eldbus_Proxy *
eldbus_proxy_get(Eldbus_Object *obj, const char *interface)
{
   Eldbus_Proxy *proxy;

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(interface, NULL);

   proxy = eldbus_object_proxy_get(obj, interface);
   if (proxy)
     return eldbus_proxy_ref(proxy);

   proxy = calloc(1, sizeof(Eldbus_Proxy));
   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);

   proxy->refcount = 1;
   proxy->obj = obj;
   proxy->interface = eina_stringshare_add(interface);
   EINA_MAGIC_SET(proxy, ELDBUS_PROXY_MAGIC);
   if (!eldbus_object_proxy_add(obj, proxy))
     goto cleanup;
   eldbus_object_free_cb_add(obj, _on_object_free, proxy);

   return proxy;

cleanup:
   eina_stringshare_del(proxy->interface);
   free(proxy);
   return NULL;
}

static void _on_signal_handler_free(void *data, const void *dead_pointer);

static void
_eldbus_proxy_unref(Eldbus_Proxy *proxy)
{
   proxy->refcount--;
   if (proxy->refcount > 0) return;

   eldbus_object_free_cb_del(proxy->obj, _on_object_free, proxy);
   eldbus_data_del_all(&(proxy->data));
   _eldbus_proxy_clear(proxy);
   _eldbus_proxy_free(proxy);
}

EAPI Eldbus_Proxy *
eldbus_proxy_ref(Eldbus_Proxy *proxy)
{
   ELDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   DBG("proxy=%p, pre-refcount=%d, interface=%s, obj=%p",
       proxy, proxy->refcount, proxy->interface, proxy->obj);
   proxy->refcount++;
   return proxy;
}

EAPI void
eldbus_proxy_unref(Eldbus_Proxy *proxy)
{
   ELDBUS_PROXY_CHECK(proxy);
   DBG("proxy=%p, pre-refcount=%d, interface=%s, obj=%p",
       proxy, proxy->refcount, proxy->interface, proxy->obj);
   _eldbus_proxy_unref(proxy);
}

EAPI void
eldbus_proxy_free_cb_add(Eldbus_Proxy *proxy, Eldbus_Free_Cb cb, const void *data)
{
   ELDBUS_PROXY_CHECK(proxy);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   proxy->cbs_free = eldbus_cbs_free_add(proxy->cbs_free, cb, data);
}

EAPI void
eldbus_proxy_free_cb_del(Eldbus_Proxy *proxy, Eldbus_Free_Cb cb, const void *data)
{
   ELDBUS_PROXY_CHECK(proxy);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   proxy->cbs_free = eldbus_cbs_free_del(proxy->cbs_free, cb, data);
}

EAPI void
eldbus_proxy_data_set(Eldbus_Proxy *proxy, const char *key, const void *data)
{
   ELDBUS_PROXY_CHECK(proxy);
   EINA_SAFETY_ON_NULL_RETURN(key);
   EINA_SAFETY_ON_NULL_RETURN(data);
   eldbus_data_set(&(proxy->data), key, data);
}

EAPI void *
eldbus_proxy_data_get(const Eldbus_Proxy *proxy, const char *key)
{
   ELDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key, NULL);
   return eldbus_data_get(&(((Eldbus_Proxy *)proxy)->data), key);
}

EAPI void *
eldbus_proxy_data_del(Eldbus_Proxy *proxy, const char *key)
{
   ELDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key, NULL);
   return eldbus_data_del(&(((Eldbus_Proxy *)proxy)->data), key);
}

static void
_property_changed_iter(void *data, const void *key, Eldbus_Message_Iter *var)
{
   Eldbus_Proxy *proxy = data;
   const char *skey = key;
   Eina_Value *st_value, stack_value, *value;
   Eldbus_Proxy_Event_Property_Changed event;

   st_value = _message_iter_struct_to_eina_value(var);
   eina_value_struct_value_get(st_value, "arg0", &stack_value);

   value = eina_hash_find(proxy->props, skey);
   if (value)
     {
        eina_value_flush(value);
        eina_value_copy(&stack_value, value);
     }
   else
     {
        value = calloc(1, sizeof(Eina_Value));
        eina_value_copy(&stack_value, value);
        eina_hash_add(proxy->props, skey, value);
     }

   event.name = skey;
   event.value = value;
   event.proxy = proxy;
   _eldbus_proxy_event_callback_call(proxy, ELDBUS_PROXY_EVENT_PROPERTY_CHANGED,
                                     &event);
   eina_value_free(st_value);
   eina_value_flush(&stack_value);
}

static void
_properties_changed(void *data, const Eldbus_Message *msg)
{
   Eldbus_Proxy *proxy = data;
   Eldbus_Message_Iter *array, *invalidate;
   const char *iface;
   const char *invalidate_prop;

   if (!eldbus_message_arguments_get(msg, "sa{sv}as", &iface, &array, &invalidate))
     {
        ERR("Error getting data from properties changed signal.");
        return;
     }
   if (proxy->props)
     eldbus_message_iter_dict_iterate(array, "sv", _property_changed_iter,
                                     proxy);

   while (eldbus_message_iter_get_and_next(invalidate, 's', &invalidate_prop))
     {
        Eldbus_Proxy_Event_Property_Removed event;
        event.interface = proxy->interface;
        event.name = invalidate_prop;
        event.proxy = proxy;
        if (proxy->props)
          eina_hash_del(proxy->props, event.name, NULL);
        _eldbus_proxy_event_callback_call(proxy, ELDBUS_PROXY_EVENT_PROPERTY_REMOVED,
                                         &event);
     }
}

static void
_props_cache_free(void *data)
{
   Eina_Value *value = data;
   eina_value_free(value);
}

EAPI void
eldbus_proxy_event_callback_add(Eldbus_Proxy *proxy, Eldbus_Proxy_Event_Type type, Eldbus_Proxy_Event_Cb cb, const void *cb_data)
{
   Eldbus_Proxy_Context_Event *ce;
   Eldbus_Proxy_Context_Event_Cb *ctx;

   ELDBUS_PROXY_CHECK(proxy);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   EINA_SAFETY_ON_TRUE_RETURN(type >= ELDBUS_PROXY_EVENT_LAST);

   ce = proxy->event_handlers + type;

   ctx = calloc(1, sizeof(Eldbus_Proxy_Context_Event_Cb));
   EINA_SAFETY_ON_NULL_RETURN(ctx);
   ctx->cb = cb;
   ctx->cb_data = cb_data;

   ce->list = eina_inlist_append(ce->list, EINA_INLIST_GET(ctx));

   if (type == ELDBUS_PROXY_EVENT_PROPERTY_CHANGED)
     {
        if (proxy->properties_changed) return;
        if (!proxy->props)
          proxy->props = eina_hash_string_superfast_new(_props_cache_free);
        proxy->properties_changed =
                 eldbus_proxy_properties_changed_callback_add(proxy,
                                                             _properties_changed,
                                                             proxy);
     }
   else if (type == ELDBUS_PROXY_EVENT_PROPERTY_REMOVED)
     {
        if (proxy->properties_changed) return;
        proxy->properties_changed =
                 eldbus_proxy_properties_changed_callback_add(proxy,
                                                             _properties_changed,
                                                             proxy);
     }
}

static void
_eldbus_proxy_context_event_cb_del(Eldbus_Proxy_Context_Event *ce, Eldbus_Proxy_Context_Event_Cb *ctx)
{
   ce->list = eina_inlist_remove(ce->list, EINA_INLIST_GET(ctx));
   free(ctx);
}

EAPI void
eldbus_proxy_event_callback_del(Eldbus_Proxy *proxy, Eldbus_Proxy_Event_Type type, Eldbus_Proxy_Event_Cb cb, const void *cb_data)
{
   Eldbus_Proxy_Context_Event *ce;
   Eldbus_Proxy_Context_Event_Cb *iter, *found = NULL;

   ELDBUS_PROXY_CHECK(proxy);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   EINA_SAFETY_ON_TRUE_RETURN(type >= ELDBUS_PROXY_EVENT_LAST);

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

   _eldbus_proxy_context_event_cb_del(ce, found);

   if (type == ELDBUS_PROXY_EVENT_PROPERTY_CHANGED)
     {
        Eldbus_Proxy_Context_Event *ce_prop_remove;
        ce_prop_remove = proxy->event_handlers +
                 ELDBUS_PROXY_EVENT_PROPERTY_REMOVED;
        if (!ce->list && !proxy->monitor_enabled)
          {
             eina_hash_free(proxy->props);
             proxy->props = NULL;
          }

        if (!ce_prop_remove->list && !ce->list && !proxy->monitor_enabled)
          {
             eldbus_signal_handler_unref(proxy->properties_changed);
             proxy->properties_changed = NULL;
          }
     }
   else if (type == ELDBUS_PROXY_EVENT_PROPERTY_REMOVED)
     {
        Eldbus_Proxy_Context_Event *ce_prop_changed;
        ce_prop_changed = proxy->event_handlers +
                 ELDBUS_PROXY_EVENT_PROPERTY_CHANGED;

        if (!ce_prop_changed->list && !ce->list && !proxy->monitor_enabled)
          {
             eldbus_signal_handler_unref(proxy->properties_changed);
             proxy->properties_changed = NULL;
          }
     }
}

static void
_eldbus_proxy_event_callback_call(Eldbus_Proxy *proxy, Eldbus_Proxy_Event_Type type, const void *event_info)
{
   Eldbus_Proxy_Context_Event *ce;
   Eldbus_Proxy_Context_Event_Cb *iter;

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
     _eldbus_proxy_context_event_cb_del(ce, iter);
}

EAPI Eldbus_Object *
eldbus_proxy_object_get(const Eldbus_Proxy *proxy)
{
   ELDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   return proxy->obj;
}

EAPI const char *
eldbus_proxy_interface_get(const Eldbus_Proxy *proxy)
{
   ELDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   return proxy->interface;
}

static void
_on_pending_free(void *data, const void *dead_pointer)
{
   Eldbus_Proxy *proxy = data;
   Eldbus_Pending *pending = (Eldbus_Pending *)dead_pointer;
   ELDBUS_PROXY_CHECK(proxy);
   proxy->pendings = eina_inlist_remove(proxy->pendings,
                                        EINA_INLIST_GET(pending));
}

static Eldbus_Pending *
_eldbus_proxy_send(Eldbus_Proxy *proxy, Eldbus_Message *msg, Eldbus_Message_Cb cb, const void *cb_data, double timeout)
{
   Eldbus_Pending *pending;

   pending = _eldbus_connection_send(proxy->obj->conn, msg, cb, cb_data, timeout);
   if (!cb) return NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(pending, NULL);

   eldbus_pending_free_cb_add(pending, _on_pending_free, proxy);
   proxy->pendings = eina_inlist_append(proxy->pendings,
                                        EINA_INLIST_GET(pending));

   return pending;
}

EAPI Eldbus_Pending *
eldbus_proxy_send(Eldbus_Proxy *proxy, Eldbus_Message *msg, Eldbus_Message_Cb cb, const void *cb_data, double timeout)
{
   ELDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(msg, NULL);

   return _eldbus_proxy_send(proxy, msg, cb, cb_data, timeout);
}

EAPI Eldbus_Message *
eldbus_proxy_method_call_new(Eldbus_Proxy *proxy, const char *member)
{
   Eldbus_Message *msg;
   ELDBUS_PROXY_CHECK_RETVAL(proxy, NULL);

   msg = eldbus_message_method_call_new(
                           eldbus_object_bus_name_get(proxy->obj),
                           eldbus_object_path_get(proxy->obj),
                           proxy->interface, member);
   return msg;
}

static Eldbus_Pending *
_eldbus_proxy_vcall(Eldbus_Proxy *proxy, const char *member, Eldbus_Message_Cb cb, const void *cb_data, double timeout, const char *signature, va_list ap)
{
   Eldbus_Message *msg = eldbus_proxy_method_call_new(proxy, member);
   EINA_SAFETY_ON_NULL_RETURN_VAL(msg, NULL);

   if (!eldbus_message_arguments_vappend(msg, signature, ap))
     {
        eldbus_message_unref(msg);
        ERR("Error setting arguments");
        return NULL;
     }

   return _eldbus_proxy_send(proxy, msg, cb, cb_data, timeout);
}

EAPI Eldbus_Pending *
eldbus_proxy_call(Eldbus_Proxy *proxy, const char *member, Eldbus_Message_Cb cb, const void *cb_data, double timeout, const char *signature, ...)
{
   Eldbus_Pending *pending;
   va_list ap;

   ELDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(member, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(signature, NULL);

   va_start(ap, signature);
   pending = _eldbus_proxy_vcall(proxy, member, cb, cb_data, timeout,
                                signature, ap);
   va_end(ap);

   return pending;
}

EAPI Eldbus_Pending *
eldbus_proxy_vcall(Eldbus_Proxy *proxy, const char *member, Eldbus_Message_Cb cb, const void *cb_data, double timeout, const char *signature, va_list ap)
{
   ELDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(member, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(signature, NULL);

   return _eldbus_proxy_vcall(proxy, member, cb, cb_data, timeout,
                             signature, ap);
}

static void
_on_signal_handler_free(void *data, const void *dead_pointer)
{
   Eldbus_Proxy *proxy = data;
   ELDBUS_PROXY_CHECK(proxy);
   proxy->handlers = eina_list_remove(proxy->handlers, dead_pointer);
}

EAPI Eldbus_Signal_Handler *
eldbus_proxy_signal_handler_add(Eldbus_Proxy *proxy, const char *member, Eldbus_Signal_Cb cb, const void *cb_data)
{
   Eldbus_Signal_Handler *handler;
   const char *name, *path;

   ELDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cb, NULL);

   name = eldbus_object_bus_name_get(proxy->obj);
   path = eldbus_object_path_get(proxy->obj);

   handler = _eldbus_signal_handler_add(proxy->obj->conn, name, path,
                                       proxy->interface, member, cb, cb_data);
   EINA_SAFETY_ON_NULL_RETURN_VAL(handler, NULL);

   eldbus_signal_handler_free_cb_add(handler, _on_signal_handler_free, proxy);
   proxy->handlers = eina_list_append(proxy->handlers, handler);

   return handler;
}

EAPI Eldbus_Pending *
eldbus_proxy_property_get(Eldbus_Proxy *proxy, const char *name, Eldbus_Message_Cb cb, const void *data)
{
   ELDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);
   return eldbus_proxy_call(proxy->obj->properties, "Get", cb, data, -1,
                           "ss", proxy->interface, name);
}

static inline Eina_Bool
_type_is_number(char sig)
{
   switch (sig)
     {
      case 'y': case 'b': case 'n': case 'q': case 'i':
      case 'u': case 'x': case 't': case 'd': case 'h':
        return EINA_TRUE;
      default:
        break;
     }
   return EINA_FALSE;
}

EAPI Eldbus_Pending *
eldbus_proxy_property_set(Eldbus_Proxy *proxy, const char *name, const char *sig, const void *value, Eldbus_Message_Cb cb, const void *data)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *iter, *variant;

   ELDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(sig, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(dbus_signature_validate_single(sig, NULL), NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL((_type_is_number(sig[0]) || value), NULL);

   msg = eldbus_proxy_method_call_new(proxy->obj->properties, "Set");
   iter = eldbus_message_iter_get(msg);
   eldbus_message_iter_basic_append(iter, 's', proxy->interface);
   eldbus_message_iter_basic_append(iter, 's', name);
   variant = eldbus_message_iter_container_new(iter, 'v', sig);
   if (dbus_type_is_basic(sig[0]))
     dbus_message_iter_append_basic(&variant->dbus_iterator, sig[0], &value);
   else
     {
        if (!_message_iter_from_eina_value_struct(sig, variant, value))
          {
             eldbus_message_unref(msg);
             return NULL;
          }
     }
   eldbus_message_iter_container_close(iter, variant);

   return eldbus_proxy_send(proxy->obj->properties, msg, cb, data, -1);
}

EAPI Eldbus_Pending *
eldbus_proxy_property_get_all(Eldbus_Proxy *proxy, Eldbus_Message_Cb cb, const void *data)
{
   ELDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   return eldbus_proxy_call(proxy->obj->properties, "GetAll", cb, data, -1,
                            "s", proxy->interface);
}

EAPI Eldbus_Signal_Handler *
eldbus_proxy_properties_changed_callback_add(Eldbus_Proxy *proxy, Eldbus_Signal_Cb cb, const void *data)
{
   Eldbus_Signal_Handler *sh;
   ELDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   sh = eldbus_proxy_signal_handler_add(proxy->obj->properties,
                                        "PropertiesChanged", cb, data);
   EINA_SAFETY_ON_NULL_RETURN_VAL(sh, NULL);
   eldbus_signal_handler_match_extra_set(sh, "arg0", proxy->interface, NULL);
   return sh;
}

static void
_property_iter(void *data, const void *key, Eldbus_Message_Iter *var)
{
   Eldbus_Proxy *proxy = data;
   const char *skey = key;
   Eina_Value *st_value, stack_value, *value;

   st_value = _message_iter_struct_to_eina_value(var);
   eina_value_struct_value_get(st_value, "arg0", &stack_value);

   value = eina_hash_find(proxy->props, skey);
   if (!value)
     {
        value = eina_value_new(eina_value_type_get(&stack_value));
        eina_hash_add(proxy->props, skey, value);
     }
   eina_value_flush(value);
   eina_value_copy(&stack_value, value);

   eina_value_free(st_value);
   eina_value_flush(&stack_value);
}

static void
_on_monitored_proxy_del(void *data, Eldbus_Proxy *proxy EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eldbus_Pending *pending = data;
   eldbus_pending_cancel(pending);
}

static void
_props_get_all(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   Eldbus_Proxy *proxy = data;
   Eldbus_Message_Iter *dict;
   const char *name, *error_msg;
   Eldbus_Proxy_Event_Property_Loaded event;

   eldbus_proxy_event_callback_del(proxy, ELDBUS_PROXY_EVENT_DEL,
                                   _on_monitored_proxy_del, pending);

   if (eldbus_message_error_get(msg, &name, &error_msg))
     {
        ERR("Error getting all properties of %s %s, error message: %s %s",
            proxy->obj->name, proxy->obj->path, name, error_msg);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "a{sv}", &dict))
     {
        char *txt;

        if (eldbus_message_arguments_get(msg, "s", &txt))
          ERR("Error getting data from properties getAll: %s", txt);
        return;
     }
   eldbus_message_iter_dict_iterate(dict, "sv", _property_iter, proxy);

   event.proxy = proxy;
   _eldbus_proxy_event_callback_call(proxy, ELDBUS_PROXY_EVENT_PROPERTY_LOADED,
                                     &event);
}

EAPI Eina_Bool
eldbus_proxy_properties_monitor(Eldbus_Proxy *proxy, Eina_Bool enable)
{
   Eldbus_Pending *pending;
   ELDBUS_PROXY_CHECK_RETVAL(proxy, EINA_FALSE);
   if (proxy->monitor_enabled == enable)
     return proxy->props ? !!eina_hash_population(proxy->props) : EINA_FALSE;

   proxy->monitor_enabled = enable;
   if (!enable)
     {
        Eldbus_Proxy_Context_Event *ce_prop_changed, *ce_prop_removed;
        ce_prop_changed = proxy->event_handlers + ELDBUS_PROXY_EVENT_PROPERTY_CHANGED;
        ce_prop_removed = proxy->event_handlers + ELDBUS_PROXY_EVENT_PROPERTY_REMOVED;

        if (!ce_prop_changed->list)
          {
             eina_hash_free(proxy->props);
             proxy->props = NULL;
          }
        if (!ce_prop_changed->list && !ce_prop_removed->list)
          {
             eldbus_signal_handler_unref(proxy->properties_changed);
             proxy->properties_changed = NULL;
          }
        return EINA_TRUE;
     }

   if (!proxy->props)
     proxy->props = eina_hash_string_superfast_new(_props_cache_free);

   pending = eldbus_proxy_property_get_all(proxy, _props_get_all, proxy);
   eldbus_proxy_event_callback_add(proxy, ELDBUS_PROXY_EVENT_DEL,
                                   _on_monitored_proxy_del, pending);

   if (proxy->properties_changed)
     return !!eina_hash_population(proxy->props);
   proxy->properties_changed =
            eldbus_proxy_properties_changed_callback_add(proxy,
                                                         _properties_changed,
                                                         proxy);
   return !!eina_hash_population(proxy->props);
}

EAPI Eina_Value *
eldbus_proxy_property_local_get(Eldbus_Proxy *proxy, const char *name)
{
   ELDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy->props, NULL);
   return eina_hash_find(proxy->props, name);
}

EAPI const Eina_Hash *
eldbus_proxy_property_local_get_all(Eldbus_Proxy *proxy)
{
   ELDBUS_PROXY_CHECK_RETVAL(proxy, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy->props, NULL);
   return proxy->props;
}

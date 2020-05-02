#include <stdio.h>
#include <unistd.h>

#include <Eina.h>
#include <Ecore_Buffer.h>
#include <Ecore_Buffer_Queue.h>

#include "bq_mgr_protocol.h"
#include "shared_buffer.h"
#include "buffer_queue.h"
#include "ecore_buffer_private.h"
#include "ecore_buffer_con.h"

struct _Ecore_Buffer_Provider
{
   struct bq_provider *resource;
   Ecore_Buffer_Queue *ebq;
   int free_slot;
   struct
     {
        void  (*consumer_add) (Ecore_Buffer_Provider *provider, int queue_size, int w, int h, void *data);
        void  (*consumer_del) (Ecore_Buffer_Provider *provider, void *data);
        void  (*enqueue)      (Ecore_Buffer_Provider *provider, void *data);
        void *data;
     } cb;
};

static void _ecore_buffer_provider_cb_consumer_connected(void *data, struct bq_provider *bq_provider, int32_t queue_size, int32_t width, int32_t height);
static void _ecore_buffer_provider_cb_consumer_disconnected(void *data, struct bq_provider *bq_provider);
static void _ecore_buffer_provider_cb_add_buffer(void *data, struct bq_provider *bq_provider, struct bq_buffer *buffer, uint32_t serial);
static Shared_Buffer *_ecore_buffer_provider_shared_buffer_new(Ecore_Buffer_Provider *provider, Ecore_Buffer *buffer);
static void _ecore_buffer_provider_shared_buffer_free(Ecore_Buffer_Provider *provider, Shared_Buffer *sb);
static void _ecore_buffer_provider_cb_buffer_free(Ecore_Buffer *buf, void *data);

struct bq_provider_listener _ecore_buffer_provider_listener =
{
   _ecore_buffer_provider_cb_consumer_connected,
   _ecore_buffer_provider_cb_consumer_disconnected,
   _ecore_buffer_provider_cb_add_buffer
};

EAPI Ecore_Buffer_Provider *
ecore_buffer_provider_new(const char *name)
{
   Ecore_Buffer_Provider *provider;

   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   DBG("Provider New - name %s", name);

   _ecore_buffer_con_init_wait();

   provider = calloc(1, sizeof(Ecore_Buffer_Provider));
   if (!provider)
     {
        ERR("Failed to allocate Ecore_Buffer_Provider");
        return NULL;
     }

   provider->resource = _ecore_buffer_con_provider_create(name);
   if (!provider->resource)
     {
        ERR("Failed to get provider connection");
        free(provider);
        return NULL;
     }

   bq_provider_add_listener(provider->resource, &_ecore_buffer_provider_listener, provider);
   bq_provider_set_user_data(provider->resource, provider);

   return provider;
}

EAPI void
ecore_buffer_provider_free(Ecore_Buffer_Provider *provider)
{
   Eina_List *shared_buffers, *l;
   Shared_Buffer *sb;

   EINA_SAFETY_ON_NULL_RETURN(provider);

   DBG("Provider Free");

   if (provider->ebq)
     {
        shared_buffers =
          _ecore_buffer_queue_shared_buffer_list_get(provider->ebq);

        EINA_LIST_FOREACH(shared_buffers, l, sb)
           _ecore_buffer_provider_shared_buffer_free(provider, sb);

        _ecore_buffer_queue_free(provider->ebq);
     }

   bq_provider_destroy(provider->resource);
   free(provider);
}

EAPI Ecore_Buffer_Return
ecore_buffer_provider_buffer_acquire(Ecore_Buffer_Provider *provider, Ecore_Buffer **ret_buf)
{
   Shared_Buffer *sb;
   Ecore_Buffer_Return ret_flag = ECORE_BUFFER_RETURN_ERROR;

   EINA_SAFETY_ON_NULL_RETURN_VAL(provider, ret_flag);

   if (!provider->ebq)
     return ret_flag;

   DBG("Buffer Acquire");

   if (!_ecore_buffer_queue_dequeue(provider->ebq, &sb))
     {
        ret_flag = ECORE_BUFFER_RETURN_EMPTY;

        // Check if exist free slot.
        if (provider->free_slot > 0)
          ret_flag = ECORE_BUFFER_RETURN_NEED_ALLOC;
     }
   else
     {
        // This should not happen.
        if (_shared_buffer_state_get(sb) != SHARED_BUFFER_STATE_RELEASE)
          {
             ERR("Unknown error occured - Not on Released State: buffer %p state %s",
                 sb, _shared_buffer_state_string_get(sb));
             return ECORE_BUFFER_RETURN_ERROR;
          }

        _shared_buffer_state_set(sb, SHARED_BUFFER_STATE_ACQUIRE);

        ret_flag = ECORE_BUFFER_RETURN_SUCCESS;
        if (ret_buf) *ret_buf = _shared_buffer_buffer_get(sb);
     }

   return ret_flag;
}

EAPI Eina_Bool
ecore_buffer_provider_buffer_enqueue(Ecore_Buffer_Provider *provider, Ecore_Buffer *buffer)
{
   Shared_Buffer *sb;
   Shared_Buffer_State state;

   EINA_SAFETY_ON_NULL_RETURN_VAL(provider, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(buffer, EINA_FALSE);

   DBG("Buffer Enqueue");

   if (!provider->ebq)
     {
        WARN("Not connected with consumer yet.");
        return EINA_FALSE;
     }

   if (!(sb = _ecore_buffer_queue_shared_buffer_find(provider->ebq, buffer)))
     {
        // this buffer was never attached before.
        if (provider->free_slot > 0)
          {
             sb = _ecore_buffer_provider_shared_buffer_new(provider, buffer);
             if (!sb)
               {
                  ERR("Unkown error occured -"
                      "Failed to attach buffer: buffer %p", buffer);
                  return EINA_FALSE;
               }
             provider->free_slot--;
          }
        else
          {
             WARN("No Free slot in Queue."
                  "Need to ecore_buffer_free of enqueueed buffer first.");
             return EINA_FALSE;
          }
     }

   state = _shared_buffer_state_get(sb);
   if ((state != SHARED_BUFFER_STATE_NEW) &&
       (state != SHARED_BUFFER_STATE_ACQUIRE))
     {
        ERR("Failed to enqueue buffer - Not on acquired state: buffer %p state %s",
            sb, _shared_buffer_state_string_get(sb));
        return EINA_FALSE;
     }

   _shared_buffer_state_set(sb, SHARED_BUFFER_STATE_ENQUEUE);
   bq_provider_enqueue_buffer(provider->resource,
                              _shared_buffer_resource_get(sb), 0);

   return EINA_TRUE;
}

EAPI Ecore_Buffer_Return
ecore_buffer_provider_buffer_acquirable_check(Ecore_Buffer_Provider *provider)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(provider, EINA_FALSE);

   if (_ecore_buffer_queue_is_empty(provider->ebq))
     {
        if (provider->free_slot > 0)
          return ECORE_BUFFER_RETURN_NEED_ALLOC;

        return ECORE_BUFFER_RETURN_EMPTY;
     }

   return ECORE_BUFFER_RETURN_NOT_EMPTY;
}

EAPI void
ecore_buffer_provider_consumer_add_cb_set(Ecore_Buffer_Provider *provider, Ecore_Buffer_Provider_Consumer_Add_Cb func, void *data)
{
   EINA_SAFETY_ON_NULL_RETURN(provider);

   provider->cb.consumer_add = func;
   provider->cb.data = data;
}

EAPI void
ecore_buffer_provider_consumer_del_cb_set(Ecore_Buffer_Provider *provider, Ecore_Buffer_Provider_Consumer_Del_Cb func, void *data)
{
   EINA_SAFETY_ON_NULL_RETURN(provider);

   provider->cb.consumer_del = func;
   provider->cb.data = data;
}

EAPI void
ecore_buffer_provider_buffer_released_cb_set(Ecore_Buffer_Provider *provider, Ecore_Buffer_Provider_Enqueue_Cb func, void *data)
{
   EINA_SAFETY_ON_NULL_RETURN(provider);

   provider->cb.enqueue = func;
   provider->cb.data = data;
}

static void
_ecore_buffer_provider_cb_consumer_connected(void *data, struct bq_provider *bq_provider EINA_UNUSED, int32_t queue_size, int32_t width, int32_t height)
{
   Ecore_Buffer_Provider *provider = data;

   EINA_SAFETY_ON_NULL_RETURN(provider);

   DBG("Consumer Connected - queue_size %d, size (%dx%d)",
       queue_size, width, height);

   if (!(provider->ebq = _ecore_buffer_queue_new(width, height, queue_size)))
     {
        ERR("Failed to create Ecore_Buffer_Queue - queue_size %d, size (%dx%d)",
            queue_size, width, height);
        return;
     }

   _ecore_buffer_queue_connection_state_set(provider->ebq, EINA_TRUE);

   // set the number of free slot which means allocatable buffer number.
   provider->free_slot = queue_size;

   // CALLBACK_CALL
   if (provider->cb.consumer_add)
     provider->cb.consumer_add(provider, queue_size, width, height,
                               provider->cb.data);
}

static void
_ecore_buffer_provider_cb_consumer_disconnected(void *data, struct bq_provider *bq_provider EINA_UNUSED)
{
   Ecore_Buffer_Provider *provider = data;

   EINA_SAFETY_ON_NULL_RETURN(provider);

   DBG("Consumer Disconnected");

   _ecore_buffer_queue_connection_state_set(provider->ebq, EINA_FALSE);

   _ecore_buffer_queue_free(provider->ebq);
   provider->ebq = NULL;

   CALLBACK_CALL(provider, consumer_del);
}

static void
_ecore_buffer_provider_cb_add_buffer(void *data, struct bq_provider *bq_provider EINA_UNUSED, struct bq_buffer *buffer, uint32_t serial EINA_UNUSED)
{
   Ecore_Buffer_Provider *provider = data;
   Shared_Buffer *sb = bq_buffer_get_user_data(buffer);

   EINA_SAFETY_ON_NULL_RETURN(provider);

   if (!sb) return;

   DBG("Buffer Enqueued");

   // Mark it as a released buffer.
   _shared_buffer_state_set(sb, SHARED_BUFFER_STATE_RELEASE);
   _ecore_buffer_queue_enqueue(provider->ebq, sb);

   CALLBACK_CALL(provider, enqueue);
}

static void
_ecore_buffer_provider_cb_buffer_free(Ecore_Buffer *buffer, void *data)
{
   Ecore_Buffer_Provider *provider = data;
   Shared_Buffer *sb;

   sb = _ecore_buffer_queue_shared_buffer_find(provider->ebq, buffer);

   EINA_SAFETY_ON_NULL_RETURN(provider);

   if (!sb) return;

   _ecore_buffer_provider_shared_buffer_free(provider, sb);
}

static Shared_Buffer *
_ecore_buffer_provider_shared_buffer_new(Ecore_Buffer_Provider *provider, Ecore_Buffer *buffer)
{
   Shared_Buffer *sb;
   struct bq_buffer *buf_resource;
   unsigned int w = 0, h = 0, format = 0;
   Ecore_Export_Type export_type;
   int export_id;
   const char *engine;
   unsigned int flags;

   EINA_SAFETY_ON_NULL_RETURN_VAL(provider, NULL);

   DBG("Create Shared Buffer - buffer %p", buffer);

   if (!provider->ebq)
     {
        WARN("Not connected with consumer yet.");
        return NULL;
     }

   ecore_buffer_size_get(buffer, &w, &h);
   format = ecore_buffer_format_get(buffer);
   export_type = _ecore_buffer_export(buffer, &export_id);
   engine = _ecore_buffer_engine_name_get(buffer);
   flags = ecore_buffer_flags_get(buffer);

   buf_resource =
     bq_provider_attach_buffer(provider->resource, engine, w, h, format, flags);
   if (!buf_resource)
     {
        ERR("Fail to attach buffer - engine %s, size (%dx%d), format %d, flags %d",
            engine, w, h, format, flags);
        return NULL;
     }

   switch (export_type)
     {
      case EXPORT_TYPE_ID:
         bq_provider_set_buffer_id(provider->resource, buf_resource,
                                   export_id, 0, 0, 0, 0, 0, 0);
         break;
      case EXPORT_TYPE_FD:
         bq_provider_set_buffer_fd(provider->resource, buf_resource,
                                   export_id, 0, 0, 0, 0, 0, 0);
         close(export_id);
         break;
      default:
         break;
     }

   sb = _shared_buffer_new(engine, buf_resource, w, h, format, flags);
   _shared_buffer_buffer_set(sb, buffer);
   _ecore_buffer_queue_shared_buffer_add(provider->ebq, sb);
   bq_buffer_set_user_data(buf_resource, sb);

   ecore_buffer_free_callback_add(buffer,
                                  _ecore_buffer_provider_cb_buffer_free,
                                  provider);

   return sb;
}

static void
_ecore_buffer_provider_shared_buffer_free(Ecore_Buffer_Provider *provider, Shared_Buffer *sb)
{
   struct bq_buffer *buf_resource;

   EINA_SAFETY_ON_NULL_RETURN(provider);
   EINA_SAFETY_ON_NULL_RETURN(sb);

   buf_resource = _shared_buffer_resource_get(sb);
   if (!buf_resource)
     return;

   DBG("Free Shared Buffer");

   bq_provider_detach_buffer(provider->resource, buf_resource);
   bq_buffer_destroy(buf_resource);

   _ecore_buffer_queue_shared_buffer_remove(provider->ebq, sb);
   _shared_buffer_free(sb);

   provider->free_slot++;
}

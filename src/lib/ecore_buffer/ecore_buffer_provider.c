#include <stdio.h>

#include <Eina.h>
#include <Ecore_Buffer.h>
#include <Ecore_Buffer_Queue.h>

#include "bq_mgr_protocol.h"
#include "shared_buffer.h"
#include "buffer_queue.h"
#include "ecore_buffer_private.h"
#include "ecore_buffer_connection.h"

struct _Ecore_Buffer_Provider
{
   struct bq_provider *provider;
   Ecore_Buffer_Queue *ebq;
   int free_slot;
   struct
   {
      void  (*consumer_add)      (Ecore_Buffer_Provider *provider, int queue_size, int w, int h, void *data);
      void  (*consumer_del)      (Ecore_Buffer_Provider *provider, void *data);
      void  (*enqueue)           (Ecore_Buffer_Provider *provider, void *data);
      void *data;
   } cb;
};

static void           _ecore_buffer_provider_cb_consumer_connected(void *data, struct bq_provider *bq_provider, int32_t queue_size, int32_t width, int32_t height);
static void           _ecore_buffer_provider_cb_consumer_disconnected(void *data, struct bq_provider *bq_provider);
static void           _ecore_buffer_provider_cb_add_buffer(void *data, struct bq_provider *bq_provider, struct bq_buffer *buffer, uint32_t serial);
static Shared_Buffer *_ecore_buffer_provider_shared_buffer_new(Ecore_Buffer_Provider *pvdr, Ecore_Buffer *buffer);
static void           _ecore_buffer_provider_shared_buffer_free(Ecore_Buffer_Provider *pvdr, Shared_Buffer *sb);
static void           _ecore_buffer_provider_cb_buffer_free(Ecore_Buffer* buf, void *data);

struct bq_provider_listener _ecore_buffer_provider_listener =
{
   _ecore_buffer_provider_cb_consumer_connected,
   _ecore_buffer_provider_cb_consumer_disconnected,
   _ecore_buffer_provider_cb_add_buffer
};

EAPI Ecore_Buffer_Provider *
ecore_buffer_provider_new(const char *name)
{
   Ecore_Buffer_Provider *pvdr;

   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   _ecore_buffer_connection_init_wait();

   DBG("Provider New - name %s", name);

   pvdr = calloc(sizeof(Ecore_Buffer_Provider), 1);
   if (!pvdr)
     return NULL;

   pvdr->provider = _ecore_buffer_connection_provider_create(name);
   if (!pvdr->provider)
     {
        free(pvdr);
        return NULL;
     }

   bq_provider_add_listener(pvdr->provider, &_ecore_buffer_provider_listener, pvdr);
   bq_provider_set_user_data(pvdr->provider, pvdr);

   return pvdr;
}

EAPI void
ecore_buffer_provider_free(Ecore_Buffer_Provider *pvdr)
{
   Eina_List *shared_buffers;
   Shared_Buffer *sb;

   EINA_SAFETY_ON_NULL_RETURN(pvdr);

   DBG("Provider Free");

   if (pvdr->ebq)
     {
        shared_buffers = _ecore_buffer_queue_shared_buffer_list_get(pvdr->ebq);
        EINA_LIST_FREE(shared_buffers, sb)
           _ecore_buffer_provider_shared_buffer_free(pvdr, sb);

        _ecore_buffer_queue_free(pvdr->ebq);
        pvdr->ebq = NULL;
     }

   bq_provider_destroy(pvdr->provider);
   free(pvdr);
}

EAPI Ecore_Buffer_Return
ecore_buffer_provider_buffer_acquire(Ecore_Buffer_Provider *pvdr, Ecore_Buffer **ret_buf)
{
   Ecore_Buffer_Queue *ebq;
   Shared_Buffer *sb;
   Ecore_Buffer_Return ret_flag = ECORE_BUFFER_RETURN_ERROR;

   EINA_SAFETY_ON_NULL_RETURN_VAL(pvdr, ret_flag);

   if (!(ebq = pvdr->ebq))
     return ret_flag;

   DBG("Buffer Acquire");

   if (!_ecore_buffer_queue_dequeue(pvdr->ebq, &sb))
     {
        // Empty Queue.
        ret_flag = ECORE_BUFFER_RETURN_EMPTY;

        // Check if exist free slot.
        if (pvdr->free_slot > 0)
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
ecore_buffer_provider_buffer_enqueue(Ecore_Buffer_Provider *pvdr, Ecore_Buffer *buffer)
{
   Shared_Buffer *sb = _ecore_buffer_user_data_get(buffer, "shared-buffer");

   EINA_SAFETY_ON_NULL_RETURN_VAL(pvdr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(buffer, EINA_FALSE);

   DBG("Buffer Enqueue");

   if (!_ecore_buffer_queue_connection_state_get(pvdr->ebq))
     {
        WARN("Not connected with consumer yet");
        return EINA_FALSE;
     }

   if (!sb)
     {
        // this buffer was never attached before.
        if (pvdr->free_slot > 0)
          {
             sb = _ecore_buffer_provider_shared_buffer_new(pvdr, buffer);
             if (!sb)
               {
                  ERR("Unkown error occured -"
                      "Failed to attach buffer: buffer %p", buffer);
                  return EINA_FALSE;
               }

             pvdr->free_slot--;
          }
        else
          {
             WARN("No Free slot in Queue."
                  "Need to ecore_buffer_free of submitted buffer first.");
             return EINA_FALSE;
          }
     }

   if (_shared_buffer_state_get(sb) != SHARED_BUFFER_STATE_ACQUIRE)
     {
        ERR("Failed to enqueue buffer - Not on acquired state: buffer %p state %s",
            sb, _shared_buffer_state_string_get(sb));
        return EINA_FALSE;
     }

   _shared_buffer_state_set(sb, SHARED_BUFFER_STATE_ENQUEUE);
   bq_provider_enqueue_buffer(pvdr->provider, _shared_buffer_proxy_get(sb), 0); // serial?

   return EINA_TRUE;
}

EAPI Ecore_Buffer_Return
ecore_buffer_provider_buffer_acquirable_check(Ecore_Buffer_Provider *pvdr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pvdr, EINA_FALSE);

   if (_ecore_buffer_queue_is_empty(pvdr->ebq))
     {
        if (pvdr->free_slot > 0)
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
   Ecore_Buffer_Provider *pvdr = data;

   EINA_SAFETY_ON_NULL_RETURN(pvdr);

   DBG("Consumer Connected - queue_size %d, size (%dx%d)",
       queue_size, width, height);

   if (!(pvdr->ebq = _ecore_buffer_queue_new(width, height, queue_size)))
     {
        ERR("Failed to create Ecore_Buffer_Queue - queue_size %d, size (%dx%d)",
            queue_size, width, height);
        return;
     }

   _ecore_buffer_queue_connection_state_set(pvdr->ebq, EINA_TRUE);

   // set the number of free slot which means allocatable buffer number.
   pvdr->free_slot = queue_size;

   // CALLBACK_CALL
   if (pvdr->cb.consumer_add)
     pvdr->cb.consumer_add(pvdr, queue_size, width, height, pvdr->cb.data);
}

static void
_ecore_buffer_provider_cb_consumer_disconnected(void *data, struct bq_provider *bq_provider EINA_UNUSED)
{
   Ecore_Buffer_Provider *pvdr = data;

   if (!pvdr)
     return;

   DBG("Consumer Disconnected");

   _ecore_buffer_queue_connection_state_set(pvdr->ebq, EINA_FALSE);

   // maybe, we should free buffer queue before callback call.
   _ecore_buffer_queue_free(pvdr->ebq);

   CALLBACK_CALL(pvdr, consumer_del);
}

static void
_ecore_buffer_provider_cb_add_buffer(void *data, struct bq_provider *bq_provider EINA_UNUSED, struct bq_buffer *buffer, uint32_t serial EINA_UNUSED)
{
   Ecore_Buffer_Provider *pvdr = data;
   Shared_Buffer *sb = bq_buffer_get_user_data(buffer);

   if ((!pvdr) || (!sb))
     return;

   DBG("Buffer Enqueued");

   _ecore_buffer_queue_enqueue(pvdr->ebq, sb);

   _shared_buffer_state_set(sb, SHARED_BUFFER_STATE_RELEASE);

   CALLBACK_CALL(pvdr, enqueue);
}

static void
_ecore_buffer_provider_cb_buffer_free(Ecore_Buffer* buf, void *data)
{
   Ecore_Buffer_Provider *pvdr = data;
   Shared_Buffer *sb = _ecore_buffer_user_data_get(buf, "shared-buffer");

   if ((!pvdr) || (!sb))
     return;

   _ecore_buffer_provider_shared_buffer_free(pvdr, sb);
   pvdr->free_slot++;
}

static Shared_Buffer *
_ecore_buffer_provider_shared_buffer_new(Ecore_Buffer_Provider *pvdr, Ecore_Buffer *buffer)
{
   Shared_Buffer *sb = NULL;
   struct bq_buffer *bq_buffer;
   unsigned int w = 0, h = 0, format = 0;
   Ecore_Export_Type export_type;
   int export_id;
   const char* engine;
   unsigned int flags;

   if (!pvdr)
     return NULL;

   if (!buffer)
     {
        ERR("Cannot attach NULL buffer");
        return NULL;
     }

   DBG("Buffer Attach");

   if (!_ecore_buffer_queue_connection_state_get(pvdr->ebq))
     {
        WARN("Not Connected");
        return NULL;
     }

   ecore_buffer_size_get(buffer, &w, &h);
   format = ecore_buffer_format_get(buffer);
   export_type = _ecore_buffer_export(buffer, &export_id);
   engine = _ecore_buffer_engine_name_get(buffer);
   flags = ecore_buffer_flags_get(buffer);

   bq_buffer = bq_provider_attach_buffer(pvdr->provider, engine, w, h, format, flags);
   if (!bq_buffer)
     {
        ERR("Fail to attach buffer - engine %s, size (%dx%d), format %d, flags %d",
            engine, w, h, format, flags);
        return NULL;
     }

   switch (export_type)
     {
      case EXPORT_TYPE_ID:
         bq_provider_set_buffer_id(pvdr->provider, bq_buffer,
                                   export_id, 0, 0, 0, 0, 0, 0);
         break;
      case EXPORT_TYPE_FD:
         bq_provider_set_buffer_fd(pvdr->provider, bq_buffer,
                                   export_id, 0, 0, 0, 0, 0, 0);
         break;
      default:
         break;
     }

   sb = _shared_buffer_new(bq_buffer, w, h, format, flags);
   _shared_buffer_buffer_set(sb, buffer);
   _shared_buffer_state_set(sb, SHARED_BUFFER_STATE_ACQUIRE);

   bq_buffer_set_user_data(bq_buffer, sb);
   _ecore_buffer_user_data_set(buffer, "shared-buffer", sb);

   _ecore_buffer_queue_shared_buffer_add(pvdr->ebq, sb);

   ecore_buffer_free_callback_add(buffer, _ecore_buffer_provider_cb_buffer_free, pvdr);

   return sb;
}

static void
_ecore_buffer_provider_shared_buffer_free(Ecore_Buffer_Provider *pvdr, Shared_Buffer *sb)
{
   struct bq_buffer *proxy = _shared_buffer_proxy_get(sb);

   if ((!pvdr) || (!pvdr->ebq))
     return;

   if ((!sb) || (!proxy))
     return;

   DBG("Buffer Detach");

   if (!_ecore_buffer_queue_connection_state_get(pvdr->ebq))
     {
        WARN("Not connected with consumer yet");
        return;
     }

   bq_provider_detach_buffer(pvdr->provider, proxy);
   bq_buffer_destroy(proxy);

   _ecore_buffer_queue_shared_buffer_remove(pvdr->ebq, sb);

   _shared_buffer_free(sb);
}

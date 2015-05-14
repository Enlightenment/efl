#include <stdio.h>

#include <Eina.h>
#include <Ecore_Buffer.h>
#include <Ecore_Buffer_Queue.h>

#include "bq_mgr_protocol.h"
#include "shared_buffer.h"
#include "buffer_queue.h"
#include "ecore_buffer_private.h"
#include "ecore_buffer_connection.h"

struct _Ecore_Buffer_Consumer
{
   Ecore_Buffer_Queue *ebq;
   struct bq_consumer *consumer;
   struct
   {
      void  (*provider_add)      (Ecore_Buffer_Consumer *consumer, void *data);
      void  (*provider_del)      (Ecore_Buffer_Consumer *consumer, void *data);
      void  (*enqueue)    (Ecore_Buffer_Consumer *consumer, void *data);
      void *data;
   } cb;
};

static void       _ecore_buffer_consumer_cb_provider_connected(void *data, struct bq_consumer *bq_consumer);
static void       _ecore_buffer_consumer_cb_provider_disconnected(void *data, struct bq_consumer *bq_consumer);
static void       _ecore_buffer_consumer_cb_buffer_attached(void *data, struct bq_consumer *bq_consumer, struct bq_buffer *id, const char* engine, int32_t width, int32_t height, int32_t format, uint32_t flags);
static void       _ecore_buffer_consumer_cb_buffer_id_set(void *data, struct bq_consumer *bq_consumer, struct bq_buffer *buffer, int32_t id, int32_t offset0, int32_t stride0, int32_t offset1, int32_t stride1, int32_t offset2, int32_t stride2);
static void       _ecore_buffer_consumer_cb_buffer_fd_set(void *data, struct bq_consumer *bq_consumer, struct bq_buffer *buffer, int32_t fd, int32_t offset0, int32_t stride0, int32_t offset1, int32_t stride1, int32_t offset2, int32_t stride2);
static void       _ecore_buffer_consumer_cb_buffer_detached(void *data, struct bq_consumer *bq_consumer, struct bq_buffer *id);
static void       _ecore_buffer_consumer_cb_add_buffer(void *data, struct bq_consumer *bq_consumer, struct bq_buffer *buffer, uint32_t serial);
static void       _ecore_buffer_consumer_cb_buffer_free(Ecore_Buffer* buf, void *data);
static Eina_Bool  _ecore_buffer_consumer_buffer_import(Ecore_Buffer_Consumer *csmr, Shared_Buffer *sb, int32_t seed, Ecore_Export_Type export_type);

struct bq_consumer_listener _ecore_buffer_consumer_listener =
{
   _ecore_buffer_consumer_cb_provider_connected,
   _ecore_buffer_consumer_cb_provider_disconnected,
   _ecore_buffer_consumer_cb_buffer_attached,
   _ecore_buffer_consumer_cb_buffer_id_set,
   _ecore_buffer_consumer_cb_buffer_fd_set,
   _ecore_buffer_consumer_cb_buffer_detached,
   _ecore_buffer_consumer_cb_add_buffer
};

EAPI Ecore_Buffer_Consumer *
ecore_buffer_consumer_new(const char *name, int32_t queue_size, int32_t w, int32_t h)
{
   Ecore_Buffer_Consumer *csmr;
   const int default_queue_size = 2;

   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   if ((w < 1) || (h < 1))
     return NULL;

   if (queue_size < 2)
     queue_size = default_queue_size;

   _ecore_buffer_connection_init_wait();

   DBG("Consumer New - name %s, queue size %d, size (%dx%d)",
       name, queue_size, w, h);

   csmr = calloc(sizeof(Ecore_Buffer_Consumer), 1);
   if (!csmr)
     return NULL;

   csmr->ebq = _ecore_buffer_queue_new(w, h, queue_size);
   if (!csmr->ebq)
     {
        free(csmr);
        return NULL;
     }

   csmr->consumer =
      _ecore_buffer_connection_consumer_create(name, queue_size, w, h);
   if (!csmr->consumer)
     {
        _ecore_buffer_queue_free(csmr->ebq);
        free(csmr);
        return NULL;
     }

   bq_consumer_add_listener(csmr->consumer,
                            &_ecore_buffer_consumer_listener,
                            csmr);

   return csmr;
}

EAPI void
ecore_buffer_consumer_free(Ecore_Buffer_Consumer *csmr)
{
   EINA_SAFETY_ON_NULL_RETURN(csmr);

   DBG("Consumer Free");

   if (csmr->ebq)
     _ecore_buffer_queue_free(csmr->ebq);

   if (csmr->consumer)
     bq_consumer_destroy(csmr->consumer);

   free(csmr);
}

EAPI Eina_Bool
ecore_buffer_consumer_buffer_release(Ecore_Buffer_Consumer *csmr, Ecore_Buffer *buffer)
{
   Shared_Buffer *sb = _ecore_buffer_user_data_get(buffer, "shared-buffer");
   struct bq_buffer *proxy = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(csmr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(buffer, EINA_FALSE);

   if (!sb)
     {
        ERR("It doesn't seem to be shared buffer");
        return EINA_FALSE;
     }

   DBG("Buffer Release");

   if (!_ecore_buffer_queue_connection_state_get(csmr->ebq))
     {
        WARN("Not connected with provider yet");
        return EINA_FALSE;
     }

   // already detached buffer, free buffer by deputy.
   if (_shared_buffer_state_get(sb) == SHARED_BUFFER_STATE_DETACH)
     {
        DBG("Free buffer - buffer %p", sb);
        _ecore_buffer_queue_shared_buffer_remove(csmr->ebq, sb);
        ecore_buffer_free(buffer);
     }
   else
     {
        if (_shared_buffer_state_get(sb) != SHARED_BUFFER_STATE_DEQUEUE)
          {
             WARN("Failed to Release Buffer -"
                  "DO NOT Release buffer which is not Dequeued: buffer %p state %s",
                  sb, _shared_buffer_state_string_get(sb));
             return EINA_FALSE;
          }

        if ((proxy = _shared_buffer_proxy_get(sb)))
          bq_consumer_release_buffer(csmr->consumer, proxy);

        _shared_buffer_state_set(sb, SHARED_BUFFER_STATE_RELEASE);
     }

   return EINA_TRUE;
}

EAPI Ecore_Buffer *
ecore_buffer_consumer_buffer_dequeue(Ecore_Buffer_Consumer *csmr)
{
   Shared_Buffer *sb;

   EINA_SAFETY_ON_NULL_RETURN_VAL(csmr, EINA_FALSE);

   DBG("Buffer Acquire");

   if (!_ecore_buffer_queue_dequeue(csmr->ebq, &sb))
     {
        DBG("No Available Buffer in Queue");
        return NULL;
     }

   // This should not happen.
   if (_shared_buffer_state_get(sb) != SHARED_BUFFER_STATE_ENQUEUE)
     {
        ERR("Unknown error occured - Not on Enqueued State: buffer %p, state %s",
            sb, _shared_buffer_state_string_get(sb));
        return NULL;
     }

   _shared_buffer_state_set(sb, SHARED_BUFFER_STATE_DEQUEUE);

   return _shared_buffer_buffer_get(sb);
}

EAPI Eina_Bool
ecore_buffer_consumer_queue_is_empty(Ecore_Buffer_Consumer *csmr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(csmr, EINA_FALSE);

   return _ecore_buffer_queue_is_empty(csmr->ebq);
}


EAPI void
ecore_buffer_consumer_provider_add_cb_set(Ecore_Buffer_Consumer *consumer, Ecore_Buffer_Consumer_Provider_Add_Cb func, void *data)
{
   EINA_SAFETY_ON_NULL_RETURN(consumer);

   consumer->cb.provider_add = func;
   consumer->cb.data = data;
}

EAPI void
ecore_buffer_consumer_provider_del_cb_set(Ecore_Buffer_Consumer *consumer, Ecore_Buffer_Consumer_Provider_Del_Cb func, void *data)
{
   EINA_SAFETY_ON_NULL_RETURN(consumer);

   consumer->cb.provider_del = func;
   consumer->cb.data = data;
}

EAPI void
ecore_buffer_consumer_buffer_enqueued_cb_set(Ecore_Buffer_Consumer *consumer, Ecore_Buffer_Consumer_Enqueue_Cb func, void *data)
{
   EINA_SAFETY_ON_NULL_RETURN(consumer);

   consumer->cb.enqueue = func;
   consumer->cb.data = data;
}

static void
_ecore_buffer_consumer_cb_provider_connected(void *data, struct bq_consumer *bq_consumer EINA_UNUSED)
{
   Ecore_Buffer_Consumer *csmr = data;

   DBG("Provider Connected");

   if (!csmr)
     return;

   _ecore_buffer_queue_connection_state_set(csmr->ebq, EINA_TRUE);

   CALLBACK_CALL(csmr, provider_add);
}

static void
_ecore_buffer_consumer_cb_provider_disconnected(void *data, struct bq_consumer *bq_consumer EINA_UNUSED)
{
   Ecore_Buffer_Consumer *csmr = (Ecore_Buffer_Consumer *)data;

   if (!csmr)
     return;

   DBG("Provider Disconnected");

   _ecore_buffer_queue_connection_state_set(csmr->ebq, EINA_FALSE);

   CALLBACK_CALL(csmr, provider_del);

   _ecore_buffer_queue_free(csmr->ebq);
   csmr->ebq = NULL;
}

static void
_ecore_buffer_consumer_cb_buffer_attached(void *data, struct bq_consumer *bq_consumer EINA_UNUSED, struct bq_buffer *id, const char* engine, int32_t width, int32_t height, int32_t format, uint32_t flags)
{
   Ecore_Buffer_Consumer *csmr = data;
   Shared_Buffer *sb;

   EINA_SAFETY_ON_NULL_RETURN(csmr);

   if (!csmr->ebq)
     return;

   DBG("Buffer Attached - engine %s, size (%dx%d), format %d, flags %d",
       engine, width, height, format, flags);

   sb = _shared_buffer_new(id, width, height, format, flags);
   _shared_buffer_state_set(sb, SHARED_BUFFER_STATE_ATTACH);
   _ecore_buffer_queue_shared_buffer_add(csmr->ebq, sb);

   bq_buffer_set_user_data(id, sb);
}

static void
_ecore_buffer_consumer_cb_buffer_free(Ecore_Buffer* buf, void *data EINA_UNUSED)
{
   Shared_Buffer *sb = _ecore_buffer_user_data_get(buf, "shared-buffer");

   if (!sb) return;

   bq_buffer_destroy(_shared_buffer_proxy_get(sb));
   _shared_buffer_free(sb);
}

static void
_ecore_buffer_consumer_cb_buffer_id_set(void *data, struct bq_consumer *bq_consumer EINA_UNUSED, struct bq_buffer *buffer, int32_t id, int32_t offset0 EINA_UNUSED, int32_t stride0 EINA_UNUSED, int32_t offset1 EINA_UNUSED, int32_t stride1 EINA_UNUSED, int32_t offset2 EINA_UNUSED, int32_t stride2 EINA_UNUSED)
{
   Ecore_Buffer_Consumer *csmr = data;
   Shared_Buffer *sb = bq_buffer_get_user_data(buffer);

   EINA_SAFETY_ON_NULL_RETURN(csmr);
   EINA_SAFETY_ON_NULL_RETURN(sb);

   if (_ecore_buffer_consumer_buffer_import(csmr, sb, id, EXPORT_TYPE_ID))
     bq_buffer_set_user_data(buffer, sb);
}

static void
_ecore_buffer_consumer_cb_buffer_fd_set(void *data, struct bq_consumer *bq_consumer EINA_UNUSED, struct bq_buffer *buffer, int32_t fd, int32_t offset0 EINA_UNUSED, int32_t stride0 EINA_UNUSED, int32_t offset1 EINA_UNUSED, int32_t stride1 EINA_UNUSED, int32_t offset2 EINA_UNUSED, int32_t stride2 EINA_UNUSED)
{
   Ecore_Buffer_Consumer *csmr = data;
   Shared_Buffer *sb = bq_buffer_get_user_data(buffer);

   EINA_SAFETY_ON_NULL_RETURN(csmr);
   EINA_SAFETY_ON_NULL_RETURN(sb);

   if (_ecore_buffer_consumer_buffer_import(csmr, sb, fd, EXPORT_TYPE_FD))
     bq_buffer_set_user_data(buffer, sb);
}

static void
_ecore_buffer_consumer_cb_buffer_detached(void *data, struct bq_consumer *bq_consumer EINA_UNUSED, struct bq_buffer *id)
{
   Ecore_Buffer_Consumer *csmr = (Ecore_Buffer_Consumer *)data;
   Shared_Buffer *sb = (Shared_Buffer *)bq_buffer_get_user_data(id);

   EINA_SAFETY_ON_NULL_RETURN(csmr);
   EINA_SAFETY_ON_NULL_RETURN(sb);

   DBG("Buffer Detached");

   // buffer is not yet belong to consumer. free.
   // NOTE: do we need to notify this to application ??
   if (_shared_buffer_state_get(sb) != SHARED_BUFFER_STATE_DEQUEUE)
     {
        DBG("Free buffer - buffer %p, state %s",
            sb, _shared_buffer_state_string_get(sb));

        _ecore_buffer_queue_shared_buffer_remove(csmr->ebq, sb);
        ecore_buffer_free(_shared_buffer_buffer_get(sb));
        return;
     }

   // mark this buffer is detached, and then free on buffer release time.
   DBG("Just mark this buffer to free it when released - buffer %p, state %s",
       sb, "SHARED_BUFFER_STATE_ACQUIRE");
   _shared_buffer_state_set(sb, SHARED_BUFFER_STATE_DETACH);
}

static void
_ecore_buffer_consumer_cb_add_buffer(void *data, struct bq_consumer *bq_consumer EINA_UNUSED, struct bq_buffer *buffer, uint32_t serial EINA_UNUSED)
{
   Ecore_Buffer_Consumer *csmr = data;
   Shared_Buffer *sb = bq_buffer_get_user_data(buffer);
   Shared_Buffer_State state;

   EINA_SAFETY_ON_NULL_RETURN(csmr);

   DBG("Buffer Enqueued");

   if (!sb)
     {
        ERR("Unknown Error occured - maybe this buffer is not shared yet");
        return;
     }

   state = _shared_buffer_state_get(sb);
   if ((state != SHARED_BUFFER_STATE_IMPORT) &&
       (state != SHARED_BUFFER_STATE_RELEASE))
     {
        ERR("Unknown Error occured - Could not enqueued this state of buffer: buffer %p, state %s",
            sb, _shared_buffer_state_string_get(sb));
        return;
     }

   _ecore_buffer_queue_enqueue(csmr->ebq, sb);

   _shared_buffer_state_set(sb, SHARED_BUFFER_STATE_ENQUEUE);

   CALLBACK_CALL(csmr, enqueue);
}

static Eina_Bool
_ecore_buffer_consumer_buffer_import(Ecore_Buffer_Consumer *csmr, Shared_Buffer *sb, int32_t seed, Ecore_Export_Type export_type)
{
   Ecore_Buffer *buffer;
   int w, h, format;
   unsigned int flags;

   if ((!sb) ||
       (!_shared_buffer_info_get(sb, &w, &h, &format, &flags)))
     {
        ERR("Failed to Get Shared Buffer");
        return EINA_FALSE;
     }

   if (_shared_buffer_state_get(sb) != SHARED_BUFFER_STATE_ATTACH)
     {
        ERR("Not Attached Buffer - buffer %p state %s",
            sb, _shared_buffer_state_string_get(sb));
        return EINA_FALSE;
     }

   if (!(buffer = _ecore_buffer_import(NULL, w, h, format, export_type, seed, flags)))
     {
        ERR("Failed to Import Buffer - size (%dx%d), foramt %d, seed %d, export_type %d",
            w, h, format, seed, export_type);
        return EINA_FALSE;
     }

   _ecore_buffer_user_data_set(buffer, "shared-buffer", sb);
   ecore_buffer_free_callback_add(buffer, _ecore_buffer_consumer_cb_buffer_free, csmr);

   _shared_buffer_buffer_set(sb, buffer);
   _shared_buffer_state_set(sb, SHARED_BUFFER_STATE_IMPORT);

   return EINA_TRUE;
}

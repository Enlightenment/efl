#include "buffer_queue.h"

struct _Ecore_Buffer_Queue
{
   int w, h;
   Eina_List *shared_buffers;
   Eina_Bool connected;
   struct
   {
      unsigned int capacity;
      Eina_List *list;
   } queue;
};

static Eina_Bool
_queue_is_full(Ecore_Buffer_Queue *ebq)
{
   return (eina_list_count(ebq->queue.list) == ebq->queue.capacity);
}

static Eina_Bool
_queue_is_empty(Ecore_Buffer_Queue *ebq)
{
   return (eina_list_count(ebq->queue.list) == 0);
}

Ecore_Buffer_Queue *
_ecore_buffer_queue_new(int w, int h, int queue_size)
{
   Ecore_Buffer_Queue *ebq;

   if (queue_size < 1) return NULL;

   ebq = calloc(1, sizeof(Ecore_Buffer_Queue));
   if (!ebq)
     return NULL;

   ebq->w = w;
   ebq->h = h;
   ebq->queue.capacity = queue_size;

   return ebq;
}

void
_ecore_buffer_queue_free(Ecore_Buffer_Queue *ebq)
{
   if (!ebq) return;

   if (ebq->shared_buffers) eina_list_free(ebq->shared_buffers);
   if (ebq->queue.list) eina_list_free(ebq->queue.list);
   free(ebq);
}

void
_ecore_buffer_queue_enqueue(Ecore_Buffer_Queue *ebq, Shared_Buffer *sb)
{
   if (!ebq) return;
   if (_queue_is_full(ebq)) return;

   if (!eina_list_data_find(ebq->shared_buffers, sb))
     {
        WARN("Couldn't enqueue not shared buffer.");
        return;
     }

   ebq->queue.list = eina_list_prepend(ebq->queue.list, sb);
}

Eina_Bool
_ecore_buffer_queue_dequeue(Ecore_Buffer_Queue *ebq, Shared_Buffer **ret_sb)
{
   Eina_List *last;
   Shared_Buffer *sb;

   if (!ebq) return EINA_FALSE;
   if (_queue_is_empty(ebq)) return EINA_FALSE;

   sb = eina_list_last_data_get(ebq->queue.list);
   last = eina_list_last(ebq->queue.list);
   ebq->queue.list = eina_list_remove_list(ebq->queue.list, last);

   if (ret_sb) *ret_sb = sb;

   return EINA_TRUE;
}

Eina_Bool
_ecore_buffer_queue_is_empty(Ecore_Buffer_Queue *ebq)
{
   if (!ebq) return EINA_FALSE;

   return _queue_is_empty(ebq);
}

void
_ecore_buffer_queue_shared_buffer_add(Ecore_Buffer_Queue *ebq, Shared_Buffer *sb)
{
   if (!ebq) return;

   ebq->shared_buffers = eina_list_append(ebq->shared_buffers, sb);
}

void
_ecore_buffer_queue_shared_buffer_remove(Ecore_Buffer_Queue *ebq, Shared_Buffer *sb)
{
   if (!ebq) return;

   ebq->shared_buffers = eina_list_remove(ebq->shared_buffers, sb);
   while (eina_list_data_find(ebq->queue.list, sb) != NULL)
     ebq->queue.list = eina_list_remove(ebq->queue.list, sb);
}

Shared_Buffer *
_ecore_buffer_queue_shared_buffer_find(Ecore_Buffer_Queue *ebq, Ecore_Buffer *buffer)
{
   Eina_List *l;
   Shared_Buffer *sb;

   if (!ebq) return NULL;

   EINA_LIST_FOREACH(ebq->shared_buffers, l, sb)
     {
        if (_shared_buffer_buffer_get(sb) == buffer)
          return sb;
     }

   return NULL;
}

Eina_List *
_ecore_buffer_queue_shared_buffer_list_get(Ecore_Buffer_Queue *ebq)
{
   if (!ebq) return NULL;

   return ebq->shared_buffers;
}

void
_ecore_buffer_queue_connection_state_set(Ecore_Buffer_Queue *ebq, Eina_Bool connect)
{
   if (!ebq) return;

   ebq->connected = connect;
}

Eina_Bool
_ecore_buffer_queue_connection_state_get(Ecore_Buffer_Queue *ebq)
{
   if (!ebq) return EINA_FALSE;

   return ebq->connected;
}

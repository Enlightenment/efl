#include "buffer_queue.h"

typedef struct _Queue Queue;

struct _Ecore_Buffer_Queue
{
   Eina_Bool connected;

   int w, h;
   Queue *queue;
   Eina_List *shared_buffers;
};

struct _Queue
{
   Eina_List *data_list;
   unsigned int capacity;
};

static inline Queue       *_queue_new(int capacity);
static inline void         _queue_free(Queue *queue);
static inline void         _queue_data_remove(Queue *queue, void *data);
static inline int          _queue_size_get(Queue *queue);
static inline Eina_Bool    _queue_is_full(Queue *queue);
static inline Eina_Bool    _queue_is_empty(Queue *queue);
static inline Eina_Bool    _queue_dequeue(Queue *queue, void **ret_data);
static inline Eina_Bool    _queue_enqueue(Queue *queue, void *data);

Ecore_Buffer_Queue *
_ecore_buffer_queue_new(int w, int h, int queue_size)
{
   Ecore_Buffer_Queue *ebq;

   ebq = calloc(sizeof(Ecore_Buffer_Queue), 1);
   if (!ebq)
     return NULL;

   ebq->w = w;
   ebq->h = h;
   ebq->queue = _queue_new(queue_size);

   return ebq;
}

void
_ecore_buffer_queue_free(Ecore_Buffer_Queue *ebq)
{
   Shared_Buffer *sb;

   EINA_SAFETY_ON_NULL_RETURN(ebq);

   // FIXME
   EINA_LIST_FREE(ebq->shared_buffers, sb)
     {
        ecore_buffer_free(_shared_buffer_buffer_get(sb));
        // NOTE: shared buffer will be freed by callback of buffer free.
     }

   _queue_free(ebq->queue);
   free(ebq);
}

void
_ecore_buffer_queue_enqueue(Ecore_Buffer_Queue *ebq, Shared_Buffer *sb)
{
   EINA_SAFETY_ON_NULL_RETURN(ebq);

   if (!eina_list_data_find(ebq->shared_buffers, sb))
     {
        WARN("Not added buffer in Queue");
        return;
     }

   _queue_enqueue(ebq->queue, (void *)sb);
}

Eina_Bool
_ecore_buffer_queue_dequeue(Ecore_Buffer_Queue *ebq, Shared_Buffer **ret_sb)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ebq, EINA_FALSE);

   return _queue_dequeue(ebq->queue, (void **)ret_sb);
}

Eina_Bool
_ecore_buffer_queue_is_empty(Ecore_Buffer_Queue *ebq)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ebq, EINA_FALSE);

   return _queue_is_empty(ebq->queue);
}

void
_ecore_buffer_queue_shared_buffer_add(Ecore_Buffer_Queue *ebq, Shared_Buffer *sb)
{
   EINA_SAFETY_ON_NULL_RETURN(ebq);

   ebq->shared_buffers = eina_list_append(ebq->shared_buffers, sb);
}

void
_ecore_buffer_queue_shared_buffer_remove(Ecore_Buffer_Queue *ebq, Shared_Buffer *sb)
{
   EINA_SAFETY_ON_NULL_RETURN(ebq);

   _queue_data_remove(ebq->queue, sb);
   ebq->shared_buffers = eina_list_remove(ebq->shared_buffers, sb);
}

Eina_List *
_ecore_buffer_queue_shared_buffer_list_get(Ecore_Buffer_Queue *ebq)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ebq, NULL);

   return ebq->shared_buffers;
}

void
_ecore_buffer_queue_connection_state_set(Ecore_Buffer_Queue *ebq, Eina_Bool connect)
{
   EINA_SAFETY_ON_NULL_RETURN(ebq);

   ebq->connected = connect;
}

Eina_Bool
_ecore_buffer_queue_connection_state_get(Ecore_Buffer_Queue *ebq)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ebq, EINA_FALSE);

   return ebq->connected;
}

static inline Queue *
_queue_new(int capacity)
{
   Queue *queue;

   EINA_SAFETY_ON_FALSE_RETURN_VAL((capacity > 0), NULL);

   queue = (Queue *)calloc(sizeof(Queue), 1);
   if (!queue)
     return NULL;

   queue->capacity = capacity;

   return queue;
}

static inline void
_queue_free(Queue *queue)
{
   EINA_SAFETY_ON_NULL_RETURN(queue);

   if (queue->data_list)
     eina_list_free(queue->data_list);

   free(queue);
}

static inline int
_queue_size_get(Queue *queue)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(queue, 0);

   return queue->capacity;
}

static inline Eina_Bool
_queue_is_full(Queue *queue)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(queue, EINA_FALSE);

   return (eina_list_count(queue->data_list) == queue->capacity);
}

static inline Eina_Bool
_queue_is_empty(Queue *queue)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(queue, EINA_FALSE);

   return (eina_list_count(queue->data_list) == 0);
}

static inline Eina_Bool
_queue_dequeue(Queue *queue, void **ret_data)
{
   Eina_List *last;
   void *data;

   EINA_SAFETY_ON_NULL_RETURN_VAL(queue, EINA_FALSE);

   if (_queue_is_empty(queue)) return EINA_FALSE;

   data = eina_list_last_data_get(queue->data_list);
   last = eina_list_last(queue->data_list);
   queue->data_list = eina_list_remove_list(queue->data_list, last);

   if (ret_data) *ret_data = data;

   return EINA_TRUE;
}

static inline Eina_Bool
_queue_enqueue(Queue *queue, void *data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(queue, EINA_FALSE);

   if (_queue_is_full(queue)) return EINA_FALSE;

   queue->data_list = eina_list_prepend(queue->data_list, data);

   return EINA_TRUE;
}

static inline void
_queue_data_remove(Queue *queue, void *data)
{
   EINA_SAFETY_ON_NULL_RETURN(queue);

   if (_queue_is_empty(queue)) return;

   while (eina_list_data_find(queue->data_list, data) != NULL)
     queue->data_list = eina_list_remove(queue->data_list, data);
}

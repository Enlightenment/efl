#ifndef _BUFFER_QUEUE_H_
#define _BUFFER_QUEUE_H_

#include <stdio.h>
#include <Eina.h>

#include "shared_buffer.h"

typedef struct _Ecore_Buffer_Queue Ecore_Buffer_Queue;

Ecore_Buffer_Queue   *_ecore_buffer_queue_new(int w, int h, int queue_size);
void                  _ecore_buffer_queue_free(Ecore_Buffer_Queue *ebq);
void                  _ecore_buffer_queue_enqueue(Ecore_Buffer_Queue *ebq, Shared_Buffer *sb);
Eina_Bool             _ecore_buffer_queue_dequeue(Ecore_Buffer_Queue *ebq, Shared_Buffer **ret_sb);
Eina_Bool             _ecore_buffer_queue_is_empty(Ecore_Buffer_Queue *ebq);
void                  _ecore_buffer_queue_shared_buffer_add(Ecore_Buffer_Queue *ebq, Shared_Buffer *sb);
void                  _ecore_buffer_queue_shared_buffer_remove(Ecore_Buffer_Queue *ebq, Shared_Buffer *sb);
Shared_Buffer        *_ecore_buffer_queue_shared_buffer_find(Ecore_Buffer_Queue *ebq, Ecore_Buffer *buffer);
Eina_List            *_ecore_buffer_queue_shared_buffer_list_get(Ecore_Buffer_Queue *ebq);
void                  _ecore_buffer_queue_connection_state_set(Ecore_Buffer_Queue *ebq, Eina_Bool connect);
Eina_Bool             _ecore_buffer_queue_connection_state_get(Ecore_Buffer_Queue *ebq);

#endif

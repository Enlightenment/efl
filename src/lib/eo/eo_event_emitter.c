#include "eo_event_emitter.h"

typedef struct {
   Event_Node *node;
} Event_Node_Array_Wrapper;

//two compare functions for sorting two inarrays

static int
_event_node_cb(const void *a, const void *b)
{
   const Event_Node_Array_Wrapper *wa = a, *wb = b;

   if (wa->node->desc < wb->node->desc) return -1;
   else if (wa->node->desc == wb->node->desc) return 0;
   else return 1;

}

static int
_callback_node_cb(const void *a, const void *b)
{
   const Callback_Node *ca = a, *cb = b;

   if (ca->priority < cb->priority) return 1;
   else if (ca->priority == cb->priority) return 0;
   else  return -1;

}

static unsigned int
_generation_latest(Eo_Event_Emitter *emitter)
{
   if (emitter->most_recent) return emitter->most_recent->generation;
   return 0;
}

static inline Event_Node*
_emitter_ev_node_get(Eo_Event_Emitter *emitter, const Efl_Event_Description *desc, int *index)
{
   int event_index;
   Event_Node node = { .desc = desc };
   Event_Node_Array_Wrapper tmp = { &node };
   Event_Node_Array_Wrapper *res;

   event_index = eina_inarray_search_sorted(emitter->callbacks, &tmp, _event_node_cb);

   if (index) *index = event_index;
   if (event_index == -1) return NULL;

   res = eina_inarray_nth(emitter->callbacks, event_index);

   return res->node;
}

static inline void
callback_fire(Eo_Event_Emitter_Frame *frame, Callback_Node *node, Efl_Event *event_info)
{

}

#define NODE_USAGE(x, cb) \
if (ev_node->simple) \
  { \
     Callback_Node *node = &ev_node->callback.simple; \
     do { \
       cb \
     } while(0); \
  } \
else \
  { \
    for (; x > 0; x--) \
      { \
         Callback_Node *node = eina_inarray_nth(ev_node->callback.chain, x - 1); \
         cb \
      } \
  } \

void
eo_event_emitter_emit(Eo_Event_Emitter *emitter, Efl_Event *event_info)
{
   int event_index;
   Eo_Event_Emitter_Frame frame, *restart_frame = NULL;
   Event_Node *ev_node = NULL;

   ev_node = _emitter_ev_node_get(emitter, event_info->desc, NULL);

   if (!ev_node) return;

   frame.generation = _generation_latest(emitter) + 1;
   if (!ev_node->simple)
     frame.runner_index = eina_inarray_count(ev_node->callback.chain);
   else
     frame.runner_index = 1;

   if (event_info->desc->restart)
     {
        for (Eo_Event_Emitter_Frame *oframe = emitter->most_recent ; oframe; oframe = oframe->prev)
          {
             if (oframe->desc == event_info->desc)
               {
                  frame.runner_index = oframe->runner_index;
               }
          }
     }

   frame.desc = event_info->desc;
   frame.prev = emitter->most_recent;
   frame.cleanup = EINA_FALSE;
   frame.terminate = EINA_FALSE;
   emitter->most_recent = &frame;

   NODE_USAGE(frame.runner_index,{

    //ignore callbacks that are added later
    if (node->generation > frame.generation) return;

    node->func(node->data, event_info);

    if (frame.terminate) break;
   })

   //restore the old frame to make this other thing start at the end again like a new run
   if (restart_frame)
     {
        if (!ev_node->simple)
          restart_frame->runner_index = eina_inarray_count(ev_node->callback.chain) - 1;
        else
          restart_frame->runner_index = 1;
        restart_frame = 0;
     }

   emitter->most_recent = frame.prev;

   if (frame.cleanup)
     {
        int highest_generation = 0;
        int x;

        if (!ev_node->simple)
          x = eina_inarray_count(ev_node->callback.chain) - 1;
        else
          x = 0;

        for (Eo_Event_Emitter_Frame *frame = emitter->most_recent ; frame; frame = frame->prev)
          {
             if (frame->desc == event_info->desc)
               {
                  highest_generation = frame->generation;
               }
          }

        NODE_USAGE(x, {
          if (node->generation == UINT_MAX && highest_generation == 0)
            {
               eo_event_emitter_unregister(emitter, node->func, event_info->desc, node->priority, node->data);
            }
          else if (node->generation > highest_generation)
            {
               node->generation = highest_generation;
            }
        })
     }
}


#define NODE_FILL(node, emitter, CB, PRIO, DATA) \
  (node)->func = CB; \
  (node)->data = DATA; \
  (node)->priority = PRIO; \
  (node)->generation = _generation_latest(emitter); \

void
eo_event_emitter_register(Eo_Event_Emitter *emitter, Efl_Event_Cb cb, const Efl_Event_Description *ev, Efl_Callback_Priority priority, void *data)
{
   Callback_Node *node;
   Event_Node *ev_node;
   Callback_Node new_callback_node;
   int cb_index;

   ev_node = _emitter_ev_node_get(emitter, ev, NULL);
   if (!ev_node)
     {
        Event_Node_Array_Wrapper wrapper = { calloc(1, sizeof(Event_Node)) };

        eina_inarray_insert_sorted(emitter->callbacks, &wrapper, _event_node_cb);

        wrapper.node->desc = ev;
        wrapper.node->simple = EINA_TRUE;

        NODE_FILL(&wrapper.node->callback.simple, emitter, cb, priority, data);

        return;
     }

   NODE_FILL(&new_callback_node, emitter, cb, priority, data);

   if (ev_node->simple)
     {
        Eina_Inarray *soon_chain = eina_inarray_new(sizeof(Callback_Node), 3);

        //insertion order does not matter at all here, chain is empty
        eina_inarray_insert_sorted(soon_chain, &ev_node->callback.simple, _callback_node_cb);

        ev_node->simple = EINA_FALSE;
        ev_node->callback.chain = soon_chain;
     }

   cb_index = eina_inarray_search_sorted(ev_node->callback.chain, &new_callback_node, _callback_node_cb);
   //we are searching now for the first element that has a different element in priority, to the higher side
   while(cb_index + 1 < eina_inarray_count(ev_node->callback.chain))
     {
        Callback_Node *tmp = eina_inarray_nth(ev_node->callback.chain, cb_index + 1);
        if (tmp->priority != new_callback_node.priority)
          break;
        cb_index ++;
     }
   eina_inarray_insert_at(ev_node->callback.chain, cb_index + 1, &new_callback_node);

   for (Eo_Event_Emitter_Frame *frame = emitter->most_recent ; frame; frame = frame->prev)
     {
        if (frame->desc == ev)
          {
             if (frame->runner_index < cb_index) frame->runner_index ++;
          }
     }
}

void
eo_event_emitter_unregister(Eo_Event_Emitter *emitter, Efl_Event_Cb cb, const Efl_Event_Description *ev, Efl_Callback_Priority priority, void *data)
{
   int ev_index = -1, cb_index = -1;
   Event_Node *ev_node, *cb_node;
   Callback_Node  tmp = { .priority = priority};
   Callback_Node *node = NULL;
   Eina_Bool disable_deletion = EINA_FALSE;

   ev_node = _emitter_ev_node_get(emitter, ev, &ev_index);

   if (!ev_node) return;

   if (!ev_node->simple)
     {
        cb_index = eina_inarray_search_sorted(ev_node->callback.chain, &tmp, _callback_node_cb);
        node = eina_inarray_nth(ev_node->callback.chain, cb_index);
     }
   else
     {
        node = &ev_node->callback.simple;
     }

   if (!node) return;

   for (Eo_Event_Emitter_Frame *frame = emitter->most_recent ; frame; frame = frame->prev)
     {
        if (frame->desc == ev)
          {
             disable_deletion = EINA_TRUE;
             frame->cleanup = EINA_TRUE;
          }
     }

   if (disable_deletion)
     {
        if (node->generation == UINT_MAX) return; //this node is already flagged for deletion

        //this generation will cause the cb to be deleted once
        node->generation = UINT_MAX;
     }
   else
     {
        if (!ev_node->simple)
          eina_inarray_remove_at(ev_node->callback.chain, cb_index);
        if ((ev_node->simple) || (!eina_inarray_count(ev_node->callback.chain)))
          {
             eina_inarray_remove_at(emitter->callbacks, ev_index);
             if (!ev_node->simple) eina_inarray_free(ev_node->callback.chain);
             free(ev_node);
          }
     }
}

void
eo_event_emitter_init(Eo_Event_Emitter *emitter)
{
   emitter->callbacks = eina_inarray_new(sizeof(Event_Node_Array_Wrapper), 2);
   emitter->garbage = NULL;
   emitter->most_recent = NULL;
}

void
eo_event_emitter_free(Eo_Event_Emitter *emitter)
{
   eina_inarray_free(emitter->callbacks);
}

void
eo_event_emitter_stop(Eo_Event_Emitter *emitter)
{
   if (emitter->most_recent)
     {
        if (!emitter->most_recent->desc->unfreezable)
          emitter->most_recent->terminate = EINA_TRUE;
     }
}

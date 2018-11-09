#ifndef EO_EVENT_EMITTER_H
#define EO_EVENT_EMITTER_H

#include <Eo.h>

typedef struct {
   Efl_Event_Cb func; /**< The callback function. */
   unsigned int generation;
   Efl_Callback_Priority priority;
   void *data;
} Callback_Node;

typedef struct {
   const Efl_Event_Description *desc; /**< The event description. */
   Eina_Bool simple;
   union {
      Callback_Node simple;
      Eina_Inarray *chain;
   } callback;
} Event_Node;

typedef struct _Eo_Event_Emitter_Frame Eo_Event_Emitter_Frame;

struct _Eo_Event_Emitter_Frame
{
   Eo_Event_Emitter_Frame *prev;
   const Efl_Event_Description *desc;
   unsigned int generation;  // the generation this frame has
   unsigned int runner_index; // the index we are currently in
   Eina_Bool cleanup; //a cleanup is needed after this frame is done
   Eina_Bool terminate;
};

typedef struct {
  Eina_Inarray *callbacks;
  Eo_Event_Emitter_Frame *most_recent;
  Eina_List *garbage;
} Eo_Event_Emitter;

void eo_event_emitter_emit(Eo_Event_Emitter *emitter, Efl_Event *event_info);
void eo_event_emitter_register(Eo_Event_Emitter *emitter, Efl_Event_Cb cb, const Efl_Event_Description *ev, Efl_Callback_Priority priority, void *data);
void eo_event_emitter_unregister(Eo_Event_Emitter *emitter, Efl_Event_Cb cb, const Efl_Event_Description *ev, Efl_Callback_Priority priority, void *data);
void eo_event_emitter_init(Eo_Event_Emitter *emitter);
void eo_event_emitter_free(Eo_Event_Emitter *emitter);
void eo_event_emitter_stop(Eo_Event_Emitter *emitter);
#endif

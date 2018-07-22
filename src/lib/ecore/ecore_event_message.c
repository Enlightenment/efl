#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>

#include "ecore_private.h"

#define MY_CLASS ECORE_EVENT_MESSAGE_CLASS

//////////////////////////////////////////////////////////////////////////

typedef struct _Ecore_Event_Message_Data Ecore_Event_Message_Data;

struct _Ecore_Event_Message_Data
{
   int type;
   void *ev;
   Ecore_End_Cb free_func;
   void *data;
};

//////////////////////////////////////////////////////////////////////////

EOLIAN static void
_ecore_event_message_data_set(Eo *obj EINA_UNUSED, Ecore_Event_Message_Data *pd, int type, void *data, void *free_func, void *free_data)
{
   pd->type = type;
   pd->ev = data;
   pd->free_func = free_func;
   pd->data = free_data;
}

EOLIAN static void
_ecore_event_message_data_get(const Eo *obj EINA_UNUSED, Ecore_Event_Message_Data *pd, int *type, void **data, void **free_func, void **free_data)
{
   if (type) *type = pd->type;
   if (data) *data = pd->ev;
   if (free_func) *free_func = pd->free_func;
   if (free_data) *free_data = pd->data;
}

EOLIAN static void
_ecore_event_message_data_steal(Eo *obj EINA_UNUSED, Ecore_Event_Message_Data *pd, int *type, void **data, void **free_func, void **free_data)
{
   if (type) *type = pd->type;
   if (data) *data = pd->ev;
   if (free_func) *free_func = pd->free_func;
   if (free_data) *free_data = pd->data;
   pd->type = -1;
   pd->ev = NULL;
   pd->free_func = NULL;
   pd->data = NULL;
}

EOLIAN static Efl_Object *
_ecore_event_message_efl_object_constructor(Eo *obj, Ecore_Event_Message_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->type = -1;
   return obj;
}

EOLIAN static void
_ecore_event_message_efl_object_destructor(Eo *obj EINA_UNUSED, Ecore_Event_Message_Data *pd EINA_UNUSED)
{
   if (pd->ev)
     {
        Ecore_End_Cb fn_free = pd->free_func;
        void *ev = pd->ev;

        pd->ev = NULL;
        if (fn_free) fn_free(pd->data, ev);
        else free(ev);
     }
   efl_destructor(efl_super(obj, MY_CLASS));
}

//////////////////////////////////////////////////////////////////////////

#include "ecore_event_message.eo.c"

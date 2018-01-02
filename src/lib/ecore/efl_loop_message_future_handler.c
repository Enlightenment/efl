#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>

#include "ecore_private.h"

#define MY_CLASS EFL_LOOP_MESSAGE_FUTURE_HANDLER_CLASS

typedef struct _Efl_Loop_Message_Future_Handler_Data Efl_Loop_Message_Future_Handler_Data;

struct _Efl_Loop_Message_Future_Handler_Data
{
   void *data; // dummy;
};

//////////////////////////////////////////////////////////////////////////

EOLIAN static Efl_Loop_Message_Future *
_efl_loop_message_future_handler_message_type_add(Eo *obj, Efl_Loop_Message_Future_Handler_Data *pd EINA_UNUSED)
{
   // XXX: implemented event obj cache
   return efl_add(EFL_LOOP_MESSAGE_FUTURE_CLASS, obj);
}

EOLIAN static Efl_Object *
_efl_loop_message_future_handler_efl_object_constructor(Eo *obj, Efl_Loop_Message_Future_Handler_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   return obj;
}

EOLIAN static void
_efl_loop_message_future_handler_efl_object_destructor(Eo *obj, Efl_Loop_Message_Future_Handler_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_loop_message_future_handler_efl_loop_message_handler_message_call(Eo *obj, Efl_Loop_Message_Future_Handler_Data *pd EINA_UNUSED, Efl_Loop_Message *message)
{
   efl_event_callback_call
     (obj, EFL_LOOP_MESSAGE_FUTURE_HANDLER_EVENT_MESSAGE_FUTURE, message);
   efl_loop_message_handler_message_call
     (efl_super(obj, MY_CLASS), message);
}

//////////////////////////////////////////////////////////////////////////

#include "efl_loop_message_future_handler.eo.c"

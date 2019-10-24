#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "Ecore.h"
#include "ecore_private.h"

#define MY_CLASS EFL_LOOP_MESSAGE_HANDLER_CLASS

#include "ecore_main_common.h"

typedef struct _Efl_Loop_Message_Handler_Data Efl_Loop_Message_Handler_Data;

struct _Efl_Loop_Message_Handler_Data
{
   Eo *loop;
   Efl_Loop_Data *loop_data;
};

EOLIAN static Efl_Loop_Message *
_efl_loop_message_handler_message_add(Eo *obj, Efl_Loop_Message_Handler_Data *pd EINA_UNUSED)
{
   // XXX: implement message object cache
   Efl_Loop_Message *message = efl_add(EFL_LOOP_MESSAGE_CLASS, obj);
   if (!message) return NULL;
   // XXX: track added messages not sent yet ...
   return message;
}

EOLIAN static void
_efl_loop_message_handler_message_send(Eo *obj, Efl_Loop_Message_Handler_Data *pd, Efl_Loop_Message *message)
{
   Message *msg;

   if (EINA_UNLIKELY(!pd->loop))
     {
        pd->loop = efl_provider_find(obj, EFL_LOOP_CLASS);
        if (!pd->loop) return;
        pd->loop_data = efl_data_scope_get(pd->loop, EFL_LOOP_CLASS);
        if (!pd->loop_data)
          {
             pd->loop = NULL;
             return;
          }
     }
   msg = calloc(1, sizeof(Message));
   if (msg)
     {
        msg->handler = obj;
        msg->message = message;
        if (pd->loop_data->message_walking > 0)
          {
             pd->loop_data->message_pending_queue = eina_inlist_append
                (pd->loop_data->message_pending_queue, EINA_INLIST_GET(msg));
          }
        else
          {
             pd->loop_data->message_queue = eina_inlist_append
                (pd->loop_data->message_queue, EINA_INLIST_GET(msg));
          }
        _efl_loop_message_send_info_set(message, EINA_INLIST_GET(msg),
                                       pd->loop, pd->loop_data);
        return;
     }
   efl_del(message);
}

EOLIAN static void
_efl_loop_message_handler_message_call(Eo *obj, Efl_Loop_Message_Handler_Data *pd, Efl_Loop_Message *message)
{
   Message *msg;
   unsigned int n = 0;
   Eina_Bool found = EINA_FALSE;

   if (!pd->loop) return;
   EINA_INLIST_FOREACH(pd->loop_data->message_queue, msg)
     {
        n++;
        if (msg->message != message) continue;
        found = EINA_TRUE;
        msg->message = NULL;
        msg->handler = NULL;
        _efl_loop_message_send_info_set(message, NULL, NULL, NULL);
        if ((pd->loop_data->message_walking == 0) || (n == 1))
          {
             pd->loop_data->message_queue =
               eina_inlist_remove(pd->loop_data->message_queue,
                                  EINA_INLIST_GET(msg));
             free(msg);
          }
        else
          msg->delete_me = EINA_TRUE;
        break;
     }
   efl_event_callback_call(message, EFL_LOOP_MESSAGE_EVENT_MESSAGE,
                           message);
   efl_event_callback_call(obj, EFL_LOOP_MESSAGE_HANDLER_EVENT_MESSAGE,
                           message);
   // XXX: implement message object cache...
   if (message) efl_del(message);
   if (found) return;
   ERR("Cannot find message called object %p on message queue", message);
}

EOLIAN static Eina_Bool
_efl_loop_message_handler_message_clear(Eo *obj, Efl_Loop_Message_Handler_Data *pd)
{
   Eina_Inlist *tmp;
   Message *msg;

   if (!pd->loop) return EINA_FALSE;
   if (!pd->loop_data->message_queue) return EINA_FALSE;
   EINA_INLIST_FOREACH_SAFE(pd->loop_data->message_queue, tmp, msg)
     {
        if (msg->handler == obj)
          {
             Eo *message = msg->message;

             msg->delete_me = EINA_TRUE;
             msg->handler = NULL;
             msg->message = NULL;
             _efl_loop_message_send_info_set(message, NULL, NULL, NULL);
             if (pd->loop_data->message_walking == 0)
               {
                  pd->loop_data->message_queue =
                    eina_inlist_remove(pd->loop_data->message_queue,
                                       EINA_INLIST_GET(msg));
                  free(msg);
               }
             if (message) efl_del(message);
          }
     }
   return EINA_FALSE;
}

EOLIAN static Efl_Object *
_efl_loop_message_handler_efl_object_constructor(Eo *obj, Efl_Loop_Message_Handler_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   return obj;
}

EOLIAN static void
_efl_loop_message_handler_efl_object_destructor(Eo *obj, Efl_Loop_Message_Handler_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

#include "efl_loop_message_handler.eo.c"

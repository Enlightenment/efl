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

#define MY_CLASS EFL_LOOP_MESSAGE_CLASS

#include "ecore_main_common.h"

typedef struct _Efl_Loop_Message_Data Efl_Loop_Message_Data;

struct _Efl_Loop_Message_Data
{
   Eina_Inlist   *send_list_node;
   Eo            *loop;
   Efl_Loop_Data *loop_data;
};

/////////////////////////////////////////////////////////////////////////////

void
_efl_loop_message_send_info_set(Eo *obj, Eina_Inlist *node, Eo *loop, Efl_Loop_Data *loop_data)
{
   Efl_Loop_Message_Data *pd = efl_data_scope_get(obj, MY_CLASS);
   if (!pd) return;
   pd->send_list_node = node;
   pd->loop = loop;
   pd->loop_data = loop_data;
}

void
_efl_loop_message_unsend(Eo *obj)
{
   Efl_Loop_Message_Data *pd = efl_data_scope_get(obj, MY_CLASS);
   if (!pd) return;
   if ((!pd->send_list_node) || (!pd->loop)) return;

   Message *msg = (Message *)pd->send_list_node;
   msg->delete_me = EINA_TRUE;
}

/////////////////////////////////////////////////////////////////////////////

EOLIAN static Efl_Object *
_efl_loop_message_efl_object_constructor(Eo *obj, Efl_Loop_Message_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   return obj;
}

EOLIAN static void
_efl_loop_message_efl_object_destructor(Eo *obj, Efl_Loop_Message_Data *pd)
{
   if ((pd->send_list_node) && (pd->loop_data))
     {
        Message *msg = (Message *)pd->send_list_node;

        msg->delete_me = EINA_TRUE;
        msg->message = NULL;
        msg->handler = NULL;
        if (pd->loop_data->message_walking == 0)
          {
             pd->loop_data->message_queue =
               eina_inlist_remove(pd->loop_data->message_queue,
                                  pd->send_list_node);
          }
        pd->send_list_node = NULL;
        pd->loop = NULL;
        pd->loop_data = NULL;
        free(pd->send_list_node);
     }
   efl_destructor(efl_super(obj, MY_CLASS));
}

#include "efl_loop_message.eo.c"

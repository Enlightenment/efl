#include "edje_private.h"

static void _edje_message_propagate_send(Edje *ed, Edje_Queue queue, Edje_Message_Type type, int id, void *emsg, Eina_Bool prop);

static int _injob = 0;
static Ecore_Job *_job = NULL;
static Ecore_Timer *_job_loss_timer = NULL;

static Eina_List *msgq = NULL;
static Eina_List *tmp_msgq = NULL;
static int tmp_msgq_processing = 0;
static int tmp_msgq_restart = 0;

/*============================================================================*
*                                   API                                      *
*============================================================================*/

static void
_edje_object_message_propagate_send(Evas_Object *obj, Edje_Message_Type type, int id, void *msg, Eina_Bool prop)
{
   Edje *ed;
   Eina_List *l;
   Evas_Object *o;

   ed = _edje_fetch(obj);
   if (!ed) return;
   _edje_message_propagate_send(ed, EDJE_QUEUE_SCRIPT, type, id, msg, prop);
   EINA_LIST_FOREACH(ed->subobjs, l, o)
     {
        _edje_object_message_propagate_send(o, type, id, msg, EINA_TRUE);
     }
}

EOLIAN void
_edje_object_efl_canvas_layout_signal_message_send(Eo *obj, Edje *pd EINA_UNUSED, int id, const Eina_Value val)
{
   const Eina_Value_Type *valtype;
   Edje_Message_Type msgtype;

   /* Note: Only primitive types & arrays of them are supported.
    * This reduces complexity and I couldn't find many real uses for combo
    * types (string+int or string+float).
    */

   union {
      Edje_Message_String str;
      Edje_Message_Int i;
      Edje_Message_Float f;
      Edje_Message_String_Set ss;
      Edje_Message_Int_Set is;
      Edje_Message_Float_Set fs;
      //Edje_Message_String_Int si;
      //Edje_Message_String_Float sf;
      //Edje_Message_String_Int_Set sis;
      //Edje_Message_String_Float_Set sfs;
   } msg, *pmsg;

   valtype = eina_value_type_get(&val);
   if (!valtype) goto bad_type;

   pmsg = &msg;
   if ((valtype == EINA_VALUE_TYPE_STRING) ||
       (valtype == EINA_VALUE_TYPE_STRINGSHARE))
     {
        eina_value_get(&val, &msg.str.str);
        msgtype = EDJE_MESSAGE_STRING;
     }
   else if (valtype == EINA_VALUE_TYPE_INT)
     {
        eina_value_get(&val, &msg.i.val);
        msgtype = EDJE_MESSAGE_INT;
     }
   else if (valtype == EINA_VALUE_TYPE_FLOAT)
     {
        float f;
        eina_value_get(&val, &f);
        msg.f.val = (double) f;
        msgtype = EDJE_MESSAGE_FLOAT;
     }
   else if (valtype == EINA_VALUE_TYPE_DOUBLE)
     {
        eina_value_get(&val, &msg.f.val);
        msgtype = EDJE_MESSAGE_FLOAT;
     }
   else if (valtype == EINA_VALUE_TYPE_ARRAY)
     {
        Eina_Value_Array array = {};
        size_t sz, k, count;

        eina_value_get(&val, &array);
        count = eina_inarray_count(array.array);
        if ((array.subtype == EINA_VALUE_TYPE_STRING) ||
            (array.subtype == EINA_VALUE_TYPE_STRINGSHARE))
          {
             sz = sizeof(char *);
             msgtype = EDJE_MESSAGE_STRING_SET;
             pmsg = alloca(sizeof(*pmsg) + sz * count);
             pmsg->ss.count = count;
             for (k = 0; k < count; k++)
               pmsg->ss.str[k] = eina_inarray_nth(array.array, k);
          }
        else if (array.subtype == EINA_VALUE_TYPE_INT)
          {
             sz = sizeof(int);
             msgtype = EDJE_MESSAGE_INT_SET;
             pmsg = alloca(sizeof(*pmsg) + sz * count);
             pmsg->is.count = count;
             for (k = 0; k < count; k++)
               pmsg->is.val[k] = *((int *) eina_inarray_nth(array.array, k));
          }
        else if (array.subtype == EINA_VALUE_TYPE_DOUBLE)
          {
             sz = sizeof(double);
             msgtype = EDJE_MESSAGE_FLOAT_SET;
             pmsg = alloca(sizeof(*pmsg) + sz * count);
             pmsg->fs.count = count;
             for (k = 0; k < count; k++)
               pmsg->fs.val[k] = *((double *) eina_inarray_nth(array.array, k));
          }
        else if (array.subtype == EINA_VALUE_TYPE_FLOAT)
          {
             sz = sizeof(double);
             msgtype = EDJE_MESSAGE_FLOAT_SET;
             pmsg = alloca(sizeof(*pmsg) + sz * count);
             pmsg->fs.count = count;
             for (k = 0; k < count; k++)
               pmsg->fs.val[k] = (double) *((float *) eina_inarray_nth(array.array, k));
          }
        else goto bad_type;

     }
   else goto bad_type;

   _edje_object_message_propagate_send(obj, msgtype, id, pmsg, EINA_FALSE);
   return;

bad_type:
   ERR("Unsupported value type: %s. Only primitives types int, real "
       "(float or double), string or arrays of those types are supported.",
       eina_value_type_name_get(valtype));
   return;
}

static void
_edje_object_message_signal_process_do(Eo *obj EINA_UNUSED, Edje *ed)
{
   Eina_List *l, *ln, *tmpq = NULL;
   Edje *lookup_ed;
   Eina_List *lg;
   Edje_Message *em;
   Eina_List *groups = NULL;
   int gotos = 0;

   if (!ed) return;

   groups = ed->groups;

   EINA_LIST_FOREACH_SAFE(msgq, l, ln, em)
     {
        EINA_LIST_FOREACH(groups, lg, lookup_ed)
          if (em->edje == lookup_ed)
            {
               tmpq = eina_list_append(tmpq, em);
               msgq = eina_list_remove_list(msgq, l);
               break;
            }
     }
   /* a temporary message queue */
   if (tmp_msgq)
     {
        EINA_LIST_FREE(tmpq, em)
          tmp_msgq = eina_list_append(tmp_msgq, em);
     }
   else
     {
        tmp_msgq = tmpq;
        tmpq = NULL;
     }

   tmp_msgq_processing++;
again:
   EINA_LIST_FOREACH_SAFE(tmp_msgq, l, ln, em)
     {
        EINA_LIST_FOREACH(groups, lg, lookup_ed)
          if (em->edje == lookup_ed)
            break;
        if (em->edje != lookup_ed) continue;
        tmp_msgq = eina_list_remove_list(tmp_msgq, l);
        if (!lookup_ed->delete_me)
          {
             lookup_ed->processing_messages++;
             _edje_message_process(em);
             _edje_message_free(em);
             lookup_ed->processing_messages--;
          }
        else
          _edje_message_free(em);
        if (lookup_ed->processing_messages == 0)
          {
             if (lookup_ed->delete_me) _edje_del(lookup_ed);
          }
        // if some child callback in _edje_message_process called
        // edje_object_message_signal_process() or
        // edje_message_signal_process() then those will mark the restart
        // flag when they finish - it mabsicammyt means tmp_msgq and
        // any item in it has potentially become invalid - so that means l
        // and ln could be rogue pointers, so start again from the beginning
        // and skip anything that is not this object and process only what is.
        // to avoid self-feeding loops allow a max of 1024 loops.
        if (tmp_msgq_restart)
          {
             tmp_msgq_restart = 0;
             gotos++;
             if (gotos < 1024) goto again;
             else
               {
                  WRN("Edje is in a self-feeding message loop (> 1024 gotos needed in a row)");
                  goto end;
               }
          }
     }
end:
   tmp_msgq_processing--;
   if (tmp_msgq_processing == 0)
     tmp_msgq_restart = 0;
   else
     tmp_msgq_restart = 1;
}

EOLIAN void
_edje_object_efl_canvas_layout_signal_signal_process(Eo *obj, Edje *ed, Eina_Bool recurse)
{
   Eina_List *l;
   Evas_Object *o;

   if (ed->delete_me) return;
   _edje_object_message_signal_process_do(obj, ed);
   if (!recurse) return;

   EINA_LIST_FOREACH(ed->subobjs, l, o)
     efl_canvas_layout_signal_process(o, EINA_TRUE);
}

static Eina_Bool
_edje_dummy_timer(void *data EINA_UNUSED)
{
   return ECORE_CALLBACK_CANCEL;
}

static void
_edje_job(void *data EINA_UNUSED)
{
   if (_job_loss_timer)
     {
        ecore_timer_del(_job_loss_timer);
        _job_loss_timer = NULL;
     }
   _job = NULL;
   _injob++;
   _edje_message_queue_process();
   _injob--;
}

static Eina_Bool
_edje_job_loss_timer(void *data EINA_UNUSED)
{
   _job_loss_timer = NULL;
   if (!_job)
     {
        _job = ecore_job_add(_edje_job, NULL);
     }
   return ECORE_CALLBACK_CANCEL;
}

void
_edje_message_init(void)
{
}

void
_edje_message_shutdown(void)
{
   _edje_message_queue_clear();
   if (_job_loss_timer)
     {
        ecore_timer_del(_job_loss_timer);
        _job_loss_timer = NULL;
     }
   if (_job)
     {
        ecore_job_del(_job);
        _job = NULL;
     }
}

void
_edje_message_cb_set(Edje *ed, void (*func)(void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg), void *data)
{
   Eina_List *l;
   Evas_Object *o;

   ed->message.func = func;
   ed->message.data = data;
   EINA_LIST_FOREACH(ed->subobjs, l, o)
     {
        Edje *edj2 = _edje_fetch(o);
        if (!edj2) continue;
        _edje_message_cb_set(edj2, func, data);
     }
}

Edje_Message *
_edje_message_new(Edje *ed, Edje_Queue queue, Edje_Message_Type type, int id)
{
   Edje_Message *em;

   em = calloc(1, sizeof(Edje_Message));
   if (!em) return NULL;
   em->edje = ed;
   em->queue = queue;
   em->type = type;
   em->id = id;
   em->edje->message.num++;
   return em;
}

void
_edje_message_free(Edje_Message *em)
{
   if (em->msg)
     {
        int i;

        switch (em->type)
          {
           case EDJE_MESSAGE_STRING:
           {
              Edje_Message_String *emsg;

              emsg = (Edje_Message_String *)em->msg;
              free(emsg->str);
              free(emsg);
           }
           break;

           case EDJE_MESSAGE_INT:
           {
              Edje_Message_Int *emsg;

              emsg = (Edje_Message_Int *)em->msg;
              free(emsg);
           }
           break;

           case EDJE_MESSAGE_FLOAT:
           {
              Edje_Message_Float *emsg;

              emsg = (Edje_Message_Float *)em->msg;
              free(emsg);
           }
           break;

           case EDJE_MESSAGE_INT_SET:
           {
              Edje_Message_Int_Set *emsg;

              emsg = (Edje_Message_Int_Set *)em->msg;
              free(emsg);
           }
           break;

           case EDJE_MESSAGE_FLOAT_SET:
           {
              Edje_Message_Float_Set *emsg;

              emsg = (Edje_Message_Float_Set *)em->msg;
              free(emsg);
           }
           break;

           case EDJE_MESSAGE_STRING_FLOAT:
           {
              Edje_Message_String_Float *emsg;

              emsg = (Edje_Message_String_Float *)em->msg;
              free(emsg->str);
              free(emsg);
           }
           break;

           case EDJE_MESSAGE_STRING_INT:
           {
              Edje_Message_String_Int *emsg;

              emsg = (Edje_Message_String_Int *)em->msg;
              free(emsg->str);
              free(emsg);
           }
           break;

           case EDJE_MESSAGE_STRING_FLOAT_SET:
           {
              Edje_Message_String_Float_Set *emsg;

              emsg = (Edje_Message_String_Float_Set *)em->msg;
              free(emsg->str);
              free(emsg);
           }
           break;

           case EDJE_MESSAGE_STRING_INT_SET:
           {
              Edje_Message_String_Int_Set *emsg;

              emsg = (Edje_Message_String_Int_Set *)em->msg;
              free(emsg->str);
              free(emsg);
           }
           break;

           case EDJE_MESSAGE_SIGNAL:
           {
              Edje_Message_Signal *emsg;

              emsg = (Edje_Message_Signal *)em->msg;
              if (emsg->sig) eina_stringshare_del(emsg->sig);
              if (emsg->src) eina_stringshare_del(emsg->src);
              if (emsg->data && (--(emsg->data->ref) == 0))
                {
                   if (emsg->data->free_func)
                     {
                        emsg->data->free_func(emsg->data->data);
                     }
                   free(emsg->data);
                }
              free(emsg);
           }
           break;

           case EDJE_MESSAGE_STRING_SET:
           {
              Edje_Message_String_Set *emsg;

              emsg = (Edje_Message_String_Set *)em->msg;
              for (i = 0; i < emsg->count; i++)
                free(emsg->str[i]);
              free(emsg);
           }
           break;

           case EDJE_MESSAGE_NONE:
           default:
             break;
          }
     }
   free(em);
}

static void
_edje_message_propagate_send(Edje *ed, Edje_Queue queue, Edje_Message_Type type, int id, void *emsg, Eina_Bool prop)
{
   /* FIXME: check all malloc & strdup fails and gracefully unroll and exit */
   Edje_Message *em;
   int i;
   unsigned char *msg = NULL;

   em = _edje_message_new(ed, queue, type, id);
   if (!em) return;
   em->propagated = prop;
   if (_job)
     {
        ecore_job_del(_job);
        _job = NULL;
     }
   if (_injob > 0)
     {
        if (!_job_loss_timer)
          _job_loss_timer = ecore_timer_add(0.001, _edje_job_loss_timer, NULL);
     }
   else
     {
        if (!_job)
          {
             _job = ecore_job_add(_edje_job, NULL);
          }
        if (_job_loss_timer)
          {
             ecore_timer_del(_job_loss_timer);
             _job_loss_timer = NULL;
          }
     }
   switch (em->type)
     {
      case EDJE_MESSAGE_NONE:
        break;

      case EDJE_MESSAGE_SIGNAL:
      {
         Edje_Message_Signal *emsg2, *emsg3;

         emsg2 = (Edje_Message_Signal *)emsg;
         emsg3 = calloc(1, sizeof(Edje_Message_Signal));
         if (emsg2->sig) emsg3->sig = eina_stringshare_add(emsg2->sig);
         if (emsg2->src) emsg3->src = eina_stringshare_add(emsg2->src);
         if (emsg2->data)
           {
              emsg3->data = emsg2->data;
              emsg3->data->ref++;
           }
         msg = (unsigned char *)emsg3;
      }
      break;

      case EDJE_MESSAGE_STRING:
      {
         Edje_Message_String *emsg2, *emsg3;

         emsg2 = (Edje_Message_String *)emsg;

         emsg3 = malloc(sizeof(Edje_Message_String));
         emsg3->str = strdup(emsg2->str);
         msg = (unsigned char *)emsg3;
      }
      break;

      case EDJE_MESSAGE_INT:
      {
         Edje_Message_Int *emsg2, *emsg3;

         emsg2 = (Edje_Message_Int *)emsg;
         emsg3 = malloc(sizeof(Edje_Message_Int));
         emsg3->val = emsg2->val;
         msg = (unsigned char *)emsg3;
      }
      break;

      case EDJE_MESSAGE_FLOAT:
      {
         Edje_Message_Float *emsg2, *emsg3;

         emsg2 = (Edje_Message_Float *)emsg;
         emsg3 = malloc(sizeof(Edje_Message_Float));
         emsg3->val = emsg2->val;
         msg = (unsigned char *)emsg3;
      }
      break;

      case EDJE_MESSAGE_STRING_SET:
      {
         Edje_Message_String_Set *emsg2, *emsg3;

         emsg2 = (Edje_Message_String_Set *)emsg;
         emsg3 = malloc(sizeof(Edje_Message_String_Set) + ((emsg2->count - 1) * sizeof(char *)));
         emsg3->count = emsg2->count;
         for (i = 0; i < emsg3->count; i++)
           emsg3->str[i] = strdup(emsg2->str[i]);
         msg = (unsigned char *)emsg3;
      }
      break;

      case EDJE_MESSAGE_INT_SET:
      {
         Edje_Message_Int_Set *emsg2, *emsg3;

         emsg2 = (Edje_Message_Int_Set *)emsg;
         emsg3 = malloc(sizeof(Edje_Message_Int_Set) + ((emsg2->count - 1) * sizeof(int)));
         emsg3->count = emsg2->count;
         for (i = 0; i < emsg3->count; i++)
           emsg3->val[i] = emsg2->val[i];
         msg = (unsigned char *)emsg3;
      }
      break;

      case EDJE_MESSAGE_FLOAT_SET:
      {
         Edje_Message_Float_Set *emsg2, *emsg3;

         emsg2 = (Edje_Message_Float_Set *)emsg;
         emsg3 = malloc(sizeof(Edje_Message_Float_Set) + ((emsg2->count - 1) * sizeof(double)));
         emsg3->count = emsg2->count;
         for (i = 0; i < emsg3->count; i++)
           emsg3->val[i] = emsg2->val[i];
         msg = (unsigned char *)emsg3;
      }
      break;

      case EDJE_MESSAGE_STRING_INT:
      {
         Edje_Message_String_Int *emsg2, *emsg3;

         emsg2 = (Edje_Message_String_Int *)emsg;
         emsg3 = malloc(sizeof(Edje_Message_String_Int));
         emsg3->str = strdup(emsg2->str);
         emsg3->val = emsg2->val;
         msg = (unsigned char *)emsg3;
      }
      break;

      case EDJE_MESSAGE_STRING_FLOAT:
      {
         Edje_Message_String_Float *emsg2, *emsg3;

         emsg2 = (Edje_Message_String_Float *)emsg;
         emsg3 = malloc(sizeof(Edje_Message_String_Float));
         emsg3->str = strdup(emsg2->str);
         emsg3->val = emsg2->val;
         msg = (unsigned char *)emsg3;
      }
      break;

      case EDJE_MESSAGE_STRING_INT_SET:
      {
         Edje_Message_String_Int_Set *emsg2, *emsg3;

         emsg2 = (Edje_Message_String_Int_Set *)emsg;
         emsg3 = malloc(sizeof(Edje_Message_String_Int_Set) + ((emsg2->count - 1) * sizeof(int)));
         emsg3->str = strdup(emsg2->str);
         emsg3->count = emsg2->count;
         for (i = 0; i < emsg3->count; i++)
           emsg3->val[i] = emsg2->val[i];
         msg = (unsigned char *)emsg3;
      }
      break;

      case EDJE_MESSAGE_STRING_FLOAT_SET:
      {
         Edje_Message_String_Float_Set *emsg2, *emsg3;

         emsg2 = (Edje_Message_String_Float_Set *)emsg;
         emsg3 = malloc(sizeof(Edje_Message_String_Float_Set) + ((emsg2->count - 1) * sizeof(double)));
         emsg3->str = strdup(emsg2->str);
         emsg3->count = emsg2->count;
         for (i = 0; i < emsg3->count; i++)
           emsg3->val[i] = emsg2->val[i];
         msg = (unsigned char *)emsg3;
      }
      break;

      default:
        break;
     }

   em->msg = msg;
   msgq = eina_list_append(msgq, em);
}

void
_edje_util_message_send(Edje *ed, Edje_Queue queue, Edje_Message_Type type, int id, void *emsg)
{
   _edje_message_propagate_send(ed, queue, type, id, emsg, EINA_FALSE);
}

void
_edje_message_parameters_push(Edje_Message *em)
{
   int i;

   /* these params ALWAYS go on */
   /* first param is the message type - always */
   embryo_parameter_cell_push(em->edje->collection->script,
                              (Embryo_Cell)em->type);
   /* 2nd param is the integer of the event id - always there */
   embryo_parameter_cell_push(em->edje->collection->script,
                              (Embryo_Cell)em->id);
   /* the rest is varags of whatever is in the msg */
   switch (em->type)
     {
      case EDJE_MESSAGE_NONE:
        break;

      case EDJE_MESSAGE_STRING:
        embryo_parameter_string_push(em->edje->collection->script,
                                     ((Edje_Message_String *)em->msg)->str);
        break;

      case EDJE_MESSAGE_INT:
      {
         Embryo_Cell v;

         v = (Embryo_Cell)((Edje_Message_Int *)em->msg)->val;
         embryo_parameter_cell_array_push(em->edje->collection->script, &v, 1);
      }
      break;

      case EDJE_MESSAGE_FLOAT:
      {
         Embryo_Cell v;
         float fv;

         fv = ((Edje_Message_Float *)em->msg)->val;
         v = EMBRYO_FLOAT_TO_CELL(fv);
         embryo_parameter_cell_array_push(em->edje->collection->script, &v, 1);
      }
      break;

      case EDJE_MESSAGE_STRING_SET:
        for (i = 0; i < ((Edje_Message_String_Set *)em->msg)->count; i++)
          embryo_parameter_string_push(em->edje->collection->script,
                                       ((Edje_Message_String_Set *)em->msg)->str[i]);
        break;

      case EDJE_MESSAGE_INT_SET:
        for (i = 0; i < ((Edje_Message_Int_Set *)em->msg)->count; i++)
          {
             Embryo_Cell v;

             v = (Embryo_Cell)((Edje_Message_Int_Set *)em->msg)->val[i];
             embryo_parameter_cell_array_push(em->edje->collection->script, &v, 1);
          }
        break;

      case EDJE_MESSAGE_FLOAT_SET:
        for (i = 0; i < ((Edje_Message_Float_Set *)em->msg)->count; i++)
          {
             Embryo_Cell v;
             float fv;

             fv = ((Edje_Message_Float_Set *)em->msg)->val[i];
             v = EMBRYO_FLOAT_TO_CELL(fv);
             embryo_parameter_cell_array_push(em->edje->collection->script, &v, 1);
          }
        break;

      case EDJE_MESSAGE_STRING_INT:
        embryo_parameter_string_push(em->edje->collection->script,
                                     ((Edje_Message_String_Int *)em->msg)->str);
        {
           Embryo_Cell v;

           v = (Embryo_Cell)((Edje_Message_String_Int *)em->msg)->val;
           embryo_parameter_cell_array_push(em->edje->collection->script, &v, 1);
        }
        break;

      case EDJE_MESSAGE_STRING_FLOAT:
        embryo_parameter_string_push(em->edje->collection->script,
                                     ((Edje_Message_String_Float *)em->msg)->str);
        {
           Embryo_Cell v;
           float fv;

           fv = ((Edje_Message_String_Float *)em->msg)->val;
           v = EMBRYO_FLOAT_TO_CELL(fv);
           embryo_parameter_cell_array_push(em->edje->collection->script, &v, 1);
        }
        break;

      case EDJE_MESSAGE_STRING_INT_SET:
        embryo_parameter_string_push(em->edje->collection->script,
                                     ((Edje_Message_String_Int_Set *)em->msg)->str);
        for (i = 0; i < ((Edje_Message_String_Int_Set *)em->msg)->count; i++)
          {
             Embryo_Cell v;

             v = (Embryo_Cell)((Edje_Message_String_Int_Set *)em->msg)->val[i];
             embryo_parameter_cell_array_push(em->edje->collection->script, &v, 1);
          }
        break;

      case EDJE_MESSAGE_STRING_FLOAT_SET:
        embryo_parameter_string_push(em->edje->collection->script,
                                     ((Edje_Message_String_Float_Set *)em->msg)->str);
        for (i = 0; i < ((Edje_Message_String_Float_Set *)em->msg)->count; i++)
          {
             Embryo_Cell v;
             float fv;

             fv = ((Edje_Message_String_Float_Set *)em->msg)->val[i];
             v = EMBRYO_FLOAT_TO_CELL(fv);
             embryo_parameter_cell_array_push(em->edje->collection->script, &v, 1);
          }
        break;

      default:
        break;
     }
}

void
_edje_message_process(Edje_Message *em)
{
   Embryo_Function fn;
   void *pdata;
   int ret;

   /* signals are only handled one way */
   if (em->type == EDJE_MESSAGE_SIGNAL)
     {
        _edje_emit_handle(em->edje,
                          ((Edje_Message_Signal *)em->msg)->sig,
                          ((Edje_Message_Signal *)em->msg)->src,
                          ((Edje_Message_Signal *)em->msg)->data,
                          em->propagated);
        return;
     }
   /* if this has been queued up for the app then just call the callback */
   if (em->queue == EDJE_QUEUE_APP)
     {
        if (em->edje->message.func)
          em->edje->message.func(em->edje->message.data, em->edje->obj,
                                 em->type, em->id, em->msg);
        return;
     }
   /* now this message is destined for the script message handler fn */
   if (!(em->edje->collection)) return;
   if (em->edje->L)
     {
        _edje_lua_script_only_message(em->edje, em);
        return;
     }
   fn = embryo_program_function_find(em->edje->collection->script, "message");
   if (fn == EMBRYO_FUNCTION_NONE) return;
   /* reset the engine */
   _edje_embryo_script_reset(em->edje);

   _edje_message_parameters_push(em);

   embryo_program_vm_push(em->edje->collection->script);
   _edje_embryo_globals_init(em->edje);
   pdata = embryo_program_data_get(em->edje->collection->script);
   embryo_program_data_set(em->edje->collection->script, em->edje);
   embryo_program_max_cycle_run_set(em->edje->collection->script, 5000000);
   ret = embryo_program_run(em->edje->collection->script, fn);
   if (ret == EMBRYO_PROGRAM_FAIL)
     {
        ERR("ERROR with embryo script. "
            "OBJECT NAME: '%s', "
            "OBJECT FILE: '%s', "
            "ENTRY POINT: '%s', "
            "ERROR: '%s'",
            em->edje->collection->part,
            em->edje->file->path,
            "message",
            embryo_error_string_get(embryo_program_error_get(em->edje->collection->script)));
     }
   else if (ret == EMBRYO_PROGRAM_TOOLONG)
     {
        ERR("ERROR with embryo script. "
            "OBJECT NAME: '%s', "
            "OBJECT FILE: '%s', "
            "ENTRY POINT: '%s', "
            "ERROR: 'Script exceeded maximum allowed cycle count of %i'",
            em->edje->collection->part,
            em->edje->file->path,
            "message",
            embryo_program_max_cycle_run_get(em->edje->collection->script));
     }

   embryo_program_data_set(em->edje->collection->script, pdata);
   embryo_program_vm_pop(em->edje->collection->script);
}

void
_edje_message_queue_process(void)
{
   int i;

   if (!msgq) return;

   /* allow the message queue to feed itself up to 8 times before forcing */
   /* us to go back to normal processing and let a 0 timeout deal with it */
   for (i = 0; (i < 8) && (msgq); i++)
     {
        /* a temporary message queue */
        if (tmp_msgq)
          {
             while (msgq)
               {
                  tmp_msgq = eina_list_append(tmp_msgq, msgq->data);
                  msgq = eina_list_remove_list(msgq, msgq);
               }
          }
        else
          {
             tmp_msgq = msgq;
             msgq = NULL;
          }

        tmp_msgq_processing++;
        while (tmp_msgq)
          {
             Edje_Message *em;
             Edje *ed;

             em = tmp_msgq->data;
             ed = em->edje;
             tmp_msgq = eina_list_remove_list(tmp_msgq, tmp_msgq);
             em->edje->message.num--;
             if (!ed->delete_me)
               {
                  ed->processing_messages++;
                  _edje_message_process(em);
                  _edje_message_free(em);
                  ed->processing_messages--;
               }
             else
               _edje_message_free(em);
             if (ed->processing_messages == 0)
               {
                  if (ed->delete_me) _edje_del(ed);
               }
          }
        tmp_msgq_processing--;
        if (tmp_msgq_processing == 0)
          tmp_msgq_restart = 0;
        else
          tmp_msgq_restart = 1;
     }

   /* if the message queue filled again set a timer to expire in 0.0 sec */
   /* to get the idle enterer to be run again */
   if (msgq)
     {
        static int self_feed_debug = -1;

        if (self_feed_debug == -1)
          {
             const char *s = getenv("EDJE_SELF_FEED_DEBUG");
             if (s) self_feed_debug = atoi(s);
             else self_feed_debug = 0;
          }
        if (self_feed_debug)
          {
             WRN("Edje is in a self-feeding message loop (> 8 loops needed)");
          }
        ecore_timer_add(0.0, _edje_dummy_timer, NULL);
     }
}

void
_edje_message_queue_clear(void)
{
   while (msgq)
     {
        Edje_Message *em;

        em = msgq->data;
        msgq = eina_list_remove_list(msgq, msgq);
        em->edje->message.num--;
        _edje_message_free(em);
     }
   while (tmp_msgq)
     {
        Edje_Message *em;

        em = tmp_msgq->data;
        tmp_msgq = eina_list_remove_list(tmp_msgq, tmp_msgq);
        em->edje->message.num--;
        _edje_message_free(em);
     }
}

void
_edje_message_del(Edje *ed)
{
   Eina_List *l;

   if (ed->message.num <= 0) return;
   /* delete any messages on the main queue for this edje object */
   for (l = msgq; l; )
     {
        Edje_Message *em;
        Eina_List *lp;

        em = eina_list_data_get(l);
        lp = l;
        l = eina_list_next(l);
        if (em->edje == ed)
          {
             msgq = eina_list_remove_list(msgq, lp);
             em->edje->message.num--;
             _edje_message_free(em);
          }
        if (ed->message.num <= 0) return;
     }
   /* delete any on the processing queue */
   for (l = tmp_msgq; l; )
     {
        Edje_Message *em;
        Eina_List *lp;

        em = eina_list_data_get(l);
        lp = l;
        l = eina_list_next(l);
        if (em->edje == ed)
          {
             tmp_msgq = eina_list_remove_list(tmp_msgq, lp);
             em->edje->message.num--;
             _edje_message_free(em);
          }
        if (ed->message.num <= 0) return;
     }
}

/* Legacy EAPI */

EAPI void
edje_object_message_send(Eo *obj, Edje_Message_Type type, int id, void *msg)
{
   _edje_object_message_propagate_send(obj, type, id, msg, EINA_FALSE);
}

EAPI void
edje_message_signal_process(void)
{
   _edje_message_queue_process();
}

EAPI void
edje_object_message_handler_set(Eo *obj, Edje_Message_Handler_Cb func, void *data)
{
   Edje *ed;

   ed = _edje_fetch(obj);
   if (!ed) return;
   _edje_message_cb_set(ed, func, data);
}

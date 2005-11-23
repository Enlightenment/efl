#include "Edje.h"
#include <Ecore_Job.h>
#include "edje_private.h"

static Ecore_Job *job = NULL;
static Ecore_Timer *job_loss_timer = NULL;

static Evas_List *msgq = NULL;
static Evas_List *tmp_msgq = NULL;

void
edje_object_message_send(Evas_Object *obj, Edje_Message_Type type, int id, void *msg)
{
   Edje *ed;
   
   ed = _edje_fetch(obj);
   if (!ed) return;
   _edje_message_send(ed, EDJE_QUEUE_SCRIPT, type, id, msg);
}

void
edje_object_message_handler_set(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg), void *data)
{
   Edje *ed;
   
   ed = _edje_fetch(obj);
   if (!ed) return;
   _edje_message_cb_set(ed, func, data);
}

void
edje_object_message_signal_process(Evas_Object *obj)
{
   Evas_List *l, *tmpq = NULL;
   Edje *ed;
   
   ed = _edje_fetch(obj);
   if (!ed) return;
   
   for (l = msgq; l; l = l->next)
     {
	Edje_Message *em;
	
	em = l->data;
	if (em->edje == ed)
	  tmpq = evas_list_append(tmpq, em);
     }
   /* now remove them from the old queue */
   for (l = tmpq; l; l = l->next)
     msgq = evas_list_remove(msgq, l->data);
   /* a temporary message queue */
   if (tmp_msgq)
     {
	while (tmpq)
	  {
	     tmp_msgq = evas_list_append(tmp_msgq, tmpq->data);
	     tmpq = evas_list_remove_list(tmpq, tmpq);
	  }
     }
   else
     {
	tmp_msgq = tmpq;
	tmpq = NULL;
     }
   
   while (tmp_msgq)
     {
	Edje_Message *em;
	
	em = tmp_msgq->data;
	tmp_msgq = evas_list_remove_list(tmp_msgq, tmp_msgq);
	_edje_message_process(em);
	_edje_message_free(em);
     }
}

void
edje_message_signal_process(void)
{
   _edje_message_queue_process();
}

static int
_edje_dummy_timer(void *data)
{
   return 0;
}

static void
_edje_job(void *data)
{
//   printf("_edje_job()\n");
   job = NULL;
   if (job_loss_timer)
     {
	ecore_timer_del(job_loss_timer);
	job_loss_timer = NULL;
     }
   _edje_message_queue_process();
//   printf("_edje_job() END\n");
}

static int
_edje_job_loss_timer(void *data)
{
   job_loss_timer = NULL;
   if (job)
     {
//	printf("!!!!!!!!! EDJE!!! EEEK! lost a job!\n");
//	printf("!!! job = %p\n", job);
	job = NULL;
     }
   return 0;
}

void
_edje_message_init(void)
{
}

void
_edje_message_shutdown(void)
{
   _edje_message_queue_clear();
   if (job)
     {
	ecore_job_del(job);
	job = NULL;
     }
}

void
_edje_message_cb_set(Edje *ed, void (*func) (void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg), void *data)
{
   ed->message.func = func;
   ed->message.data = data;
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
		  if (emsg->sig) free(emsg->sig);
		  if (emsg->src) free(emsg->src);
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
   em->edje->message.num--;
   free(em);
}

void
_edje_message_send(Edje *ed, Edje_Queue queue, Edje_Message_Type type, int id, void *emsg)
{
   /* FIXME: check all malloc & strdup fails and gracefully unroll and exit */
   Edje_Message *em;
   int i;
   unsigned char *msg = NULL;

   if (!job)
     {
//	printf("no job... add\n");
	job = ecore_job_add(_edje_job, NULL);
	if (job_loss_timer) ecore_timer_del(job_loss_timer);
	job_loss_timer = ecore_timer_add(0.05, _edje_job_loss_timer, NULL);
     }
   em = _edje_message_new(ed, queue, type, id);
   if (!em) return;
   switch (em->type)
     {
      case EDJE_MESSAGE_NONE:
	break;
      case EDJE_MESSAGE_SIGNAL:
	  {
	     Edje_Message_Signal *emsg2, *emsg3;
	     
	     emsg2 = (Edje_Message_Signal *)emsg;
	     emsg3 = calloc(1, sizeof(Edje_Message_Signal));
	     if (emsg2->sig) emsg3->sig = strdup(emsg2->sig);
	     if (emsg2->src) emsg3->src = strdup(emsg2->src);
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
   msgq = evas_list_append(msgq, em);
}

void
_edje_message_process(Edje_Message *em)
{
   Embryo_Function fn;
   void *pdata;
   int i;
   
   /* signals are only handled one way */
   if (em->type == EDJE_MESSAGE_SIGNAL)
     {
	_edje_emit_handle(em->edje, 
			  ((Edje_Message_Signal *)em->msg)->sig, 
			  ((Edje_Message_Signal *)em->msg)->src);
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
   if (!((em->edje->collection) && (em->edje->collection->script))) return;
   fn = embryo_program_function_find(em->edje->collection->script, "message");
   if (fn == EMBRYO_FUNCTION_NONE) return;
   /* reset the engine */
   _edje_embryo_script_reset(em->edje);
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
   pdata = embryo_program_data_get(em->edje->collection->script);
   embryo_program_data_set(em->edje->collection->script, em->edje);
   embryo_program_run(em->edje->collection->script, fn);
   embryo_program_data_set(em->edje->collection->script, pdata);
}

void
_edje_message_queue_process(void)
{
   int i;
   
   if (msgq == NULL) return;

   /* allow the message queue to feed itself up to 8 times before forcing */
   /* us to go back to normal processing and let a 0 timeout deal with it */
   for (i = 0; (i < 8) && (msgq); i++)
     {
	/* a temporary message queue */
	if (tmp_msgq)
	  {
	     while (msgq)
	       {
		  tmp_msgq = evas_list_append(tmp_msgq, msgq->data);
		  msgq = evas_list_remove_list(msgq, msgq);
	       }
	  }
	else
	  {
	     tmp_msgq = msgq;
	     msgq = NULL;
	  }
	
	while (tmp_msgq)
	  {
	     Edje_Message *em;
	     Edje *ed;
	     
	     em = tmp_msgq->data;
	     ed = em->edje;
	     ed->processing_messages++;
	     tmp_msgq = evas_list_remove_list(tmp_msgq, tmp_msgq);
	     _edje_message_process(em);
	     _edje_message_free(em);
	     ed->processing_messages--;
	     if (ed->processing_messages == 0)
	       {
		  if (ed->delete_me) _edje_del(ed);
	       }
	  }
     }
   
   /* if the message queue filled again set a timer to expire in 0.0 sec */
   /* to get the idle enterer to be run again */
   if (msgq)
     ecore_timer_add(0.0, _edje_dummy_timer, NULL);
}

void
_edje_message_queue_clear(void)
{
   while (msgq)
     {
	Edje_Message *em;
	
	em = msgq->data;
	msgq = evas_list_remove_list(msgq, msgq);
	_edje_message_free(em);
     }
   while (tmp_msgq)
     {
	Edje_Message *em;
	
	em = tmp_msgq->data;
	tmp_msgq = evas_list_remove_list(tmp_msgq, tmp_msgq);
	_edje_message_free(em);
     }
}

void
_edje_message_del(Edje *ed)
{
   Evas_List *l;

   if (ed->message.num <= 0) return;
   /* delete any messages on the main queue for this edje object */
   for (l = msgq; l; )
     {
	Edje_Message *em;
	Evas_List *lp;
	
	em = l->data;
	lp = l;
	l = l->next;
	if (em->edje == ed)
	  {
	     msgq = evas_list_remove_list(msgq, lp);
	     _edje_message_free(em);
	  }
	if (ed->message.num <= 0) return;
     }
   /* delete any on the processing queue */
   for (l = tmp_msgq; l; )
     {
	Edje_Message *em;
	Evas_List *lp;
	
	em = l->data;
	lp = l;
	l = l->next;
	if (em->edje == ed)
	  {
	     tmp_msgq = evas_list_remove_list(tmp_msgq, lp);
	     _edje_message_free(em);
	  }
	if (ed->message.num <= 0) return;
     }
}

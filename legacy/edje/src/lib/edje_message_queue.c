#include "Edje.h"
#include "edje_private.h"

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

static Evas_List *msgq = NULL;
static Evas_List *tmp_msgq = NULL;
static Ecore_Idle_Enterer *idler = NULL;

static int
_edje_dummy_timer(void *data)
{
   return 0;
}

static int
_edje_idler(void *data)
{
   _edje_message_queue_process();
   return 1;
}

void
_edje_message_init(void)
{
   idler = ecore_idle_enterer_add(_edje_idler, NULL);
}

void
_edje_message_shutdown(void)
{
   _edje_message_queue_clear();
   ecore_idle_enterer_del(idler);
   idler = NULL;
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
	char *ptr;
	int i, count;
	
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
		  free(emsg->sig);
		  free(emsg->src);
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
	     emsg3 = malloc(sizeof(Edje_Message_Signal));
	     emsg3->sig = strdup(emsg2->sig);
	     emsg3->src = strdup(emsg2->src);
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
	embryo_parameter_cell_push(em->edje->collection->script, 
				   (Embryo_Cell)((Edje_Message_Int *)em->msg)->val);
	break;
      case EDJE_MESSAGE_FLOAT:
	  {
	     float v;
	     
	     v = (Embryo_Cell)((Edje_Message_Float *)em->msg)->val;
	     embryo_parameter_cell_push(em->edje->collection->script, 
					(Embryo_Cell)EMBRYO_FLOAT_TO_CELL(v));
	  }
	break;
      case EDJE_MESSAGE_STRING_SET:
	for (i = 0; i < ((Edje_Message_String_Set *)em->msg)->count; i++)
	  embryo_parameter_string_push(em->edje->collection->script, 
				       ((Edje_Message_String_Set *)em->msg)->str[i]);
	break;
      case EDJE_MESSAGE_INT_SET:
	for (i = 0; i < ((Edje_Message_Int_Set *)em->msg)->count; i++)
	  embryo_parameter_cell_push(em->edje->collection->script, 
				     (Embryo_Cell)((Edje_Message_Int_Set *)em->msg)->val[i]);
	break;
      case EDJE_MESSAGE_FLOAT_SET:
	for (i = 0; i < ((Edje_Message_Float_Set *)em->msg)->count; i++)
	  {
	     float v;
	     
	     v = ((Edje_Message_Float_Set *)em->msg)->val[i];
	     embryo_parameter_cell_push(em->edje->collection->script, 
					(Embryo_Cell)EMBRYO_FLOAT_TO_CELL(v));
	  }
	break;
      case EDJE_MESSAGE_STRING_INT:
	embryo_parameter_string_push(em->edje->collection->script, 
				     ((Edje_Message_String_Int *)em->msg)->str);
	embryo_parameter_cell_push(em->edje->collection->script, 
				   (Embryo_Cell)((Edje_Message_String_Int *)em->msg)->val);
	break;
      case EDJE_MESSAGE_STRING_FLOAT:
	embryo_parameter_string_push(em->edje->collection->script, 
				     ((Edje_Message_String_Float *)em->msg)->str);
	  {
	     float v;
	     
	     v = (Embryo_Cell)((Edje_Message_String_Float *)em->msg)->val;
	     embryo_parameter_cell_push(em->edje->collection->script, 
					(Embryo_Cell)EMBRYO_FLOAT_TO_CELL(v));
	  }
	break;
      case EDJE_MESSAGE_STRING_INT_SET:
	embryo_parameter_string_push(em->edje->collection->script, 
				     ((Edje_Message_String_Int_Set *)em->msg)->str);
	for (i = 0; i < ((Edje_Message_String_Int_Set *)em->msg)->count; i++)
	  embryo_parameter_cell_push(em->edje->collection->script, 
				     (Embryo_Cell)((Edje_Message_String_Int_Set *)em->msg)->val[i]);
	break;
      case EDJE_MESSAGE_STRING_FLOAT_SET:
	embryo_parameter_string_push(em->edje->collection->script, 
				     ((Edje_Message_String_Float_Set *)em->msg)->str);
	for (i = 0; i < ((Edje_Message_String_Float_Set *)em->msg)->count; i++)
	  {
	     float v;
	     
	     v = ((Edje_Message_String_Float_Set *)em->msg)->val[i];
	     embryo_parameter_cell_push(em->edje->collection->script, 
					(Embryo_Cell)EMBRYO_FLOAT_TO_CELL(v));
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
	tmp_msgq = msgq;
	msgq = NULL;
	
	while (tmp_msgq)
	  {
	     Edje_Message *em;
	     
	     em = tmp_msgq->data;
	     tmp_msgq = evas_list_remove_list(tmp_msgq, tmp_msgq);
	     _edje_message_process(em);
	     _edje_message_free(em);
	  }
     }
   
   /* if the message queue filled again set a timer to expire in 0.0 sec */
   /* to get the dle enterer to be run again */
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
}

void
_edje_message_del(Edje *ed)
{
   Evas_List *l;

   if (ed->message.num <= 0) return;
   /* delete any messages on the main queue for this edje object */
   for (l = msgq; ; )
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
   for (l = tmp_msgq; ; )
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
}

#include "Edje.h"
#include "edje_private.h"

/* NB: global message que to be processed on idle enterer */

/* NB: on deletion of an edje, remvoe all pending msg que items for it */

/* NB: each edje needs a messagequeue cointer to knwo how many messages on the
 * queue for that edje */

/* NB: need to temporarly remove queue so messages added while processing the
 * the queue so messages added as a result of processing dont loop. if the
 * queue is not empty after processing scheduly 0.0 timeout to induce a
 * new processing of the queue
 */

static Evas_List *msgq = NULL;
static Evas_List *tmp_msgq = NULL;
static Ecore_Idle_Enterer *idler = NULL;

static int
_edje_dummy_timer(void *data)
{
   printf("DUMMY\n");
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
   em->edje->message_num++;
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
	   case EDJE_MESSAGE_INT:
	       {
		  Edje_Message_Int *emsg;
		  
		  emsg = (Edje_Message_Int *)em->msg;
		  free(emsg);
	       }
	   case EDJE_MESSAGE_FLOAT:
	       {
		  Edje_Message_Float *emsg;
		  
		  emsg = (Edje_Message_Float *)em->msg;
		  free(emsg);
	       }
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
	   case EDJE_MESSAGE_STRING_INT:
	       {
		  Edje_Message_String_Int *emsg;
		  
		  emsg = (Edje_Message_String_Int *)em->msg;
		  free(emsg->str);
		  free(emsg);
	       }
	   case EDJE_MESSAGE_STRING_FLOAT_SET:
	       {
		  Edje_Message_String_Float_Set *emsg;
		  
		  emsg = (Edje_Message_String_Float_Set *)em->msg;
		  free(emsg->str);
		  free(emsg);
	       }
	   case EDJE_MESSAGE_STRING_INT_SET:
	       {
		  Edje_Message_String_Int_Set *emsg;
		  
		  emsg = (Edje_Message_String_Int_Set *)em->msg;
		  free(emsg->str);
		  free(emsg);
	       }
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
   em->edje->message_num--;
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
   switch (em->type)
     {
      case EDJE_MESSAGE_NONE:
	switch (em->queue)
	  {
	   case EDJE_QUEUE_APP:
	     /* simply call app callback */
	     break;
	   case EDJE_QUEUE_SCRIPT:
	     if ((em->edje->collection) && (em->edje->collection->script))
	       {
		  Embryo_Function fn;
		  
		  _edje_embryo_script_reset(em->edje);
		  fn = embryo_program_function_find(em->edje->collection->script, "message");
		  if (fn != EMBRYO_FUNCTION_NONE)
		    {
		       void *pdata;
		       Embryo_Cell cell;
		       
		       /* first param is the message type - always */
		       cell = em->type;
		       embryo_parameter_cell_push(em->edje->collection->script, cell);
		       /* 2nd param is the integer of the event id - always there */
		       cell = em->id;
		       embryo_parameter_cell_push(em->edje->collection->script, cell);
		       pdata = embryo_program_data_get(em->edje->collection->script);
		       embryo_program_data_set(em->edje->collection->script, em->edje);
		       embryo_program_run(em->edje->collection->script, fn);
		       embryo_program_data_set(em->edje->collection->script, pdata);
		    }
	       }
	     break;
	   default:
	     break;
	  }
	break;
      case EDJE_MESSAGE_SIGNAL:
	  {
	     char *str1;
	     char *str2;
	     
	     memcpy(&str1, em->msg, sizeof(char *));
	     memcpy(&str2, em->msg + sizeof(char *), sizeof(char *));
	     _edje_emit_handle(em->edje, str1, str2);
	  }
	break;
      case EDJE_MESSAGE_STRING:
	switch (em->queue)
	  {
	   case EDJE_QUEUE_APP:
	     /* simply call app callback */
	     break;
	   case EDJE_QUEUE_SCRIPT:
	     if ((em->edje->collection) && (em->edje->collection->script))
	       {
		  Embryo_Function fn;
		  
		  _edje_embryo_script_reset(em->edje);
		  fn = embryo_program_function_find(em->edje->collection->script, "message");
		  if (fn != EMBRYO_FUNCTION_NONE)
		    {
		       void *pdata;
		       Embryo_Cell cell;
		       
		       /* first param is the message type - always */
		       cell = em->type;
		       embryo_parameter_cell_push(em->edje->collection->script, cell);
		       /* 2nd param is the integer of the event id - always there */
		       cell = em->id;
		       embryo_parameter_cell_push(em->edje->collection->script, cell);
		       /* 3rd param is the string */
		       embryo_parameter_string_push(em->edje->collection->script, em->msg);
		       pdata = embryo_program_data_get(em->edje->collection->script);
		       embryo_program_data_set(em->edje->collection->script, em->edje);
		       embryo_program_run(em->edje->collection->script, fn);
		       embryo_program_data_set(em->edje->collection->script, pdata);
		    }
	       }
	     break;
	   default:
	     break;
	  }
	break;
      case EDJE_MESSAGE_INT:
	break;
      case EDJE_MESSAGE_FLOAT:
	break;
      case EDJE_MESSAGE_STRING_SET:
	break;
      case EDJE_MESSAGE_INT_SET:
	break;
      case EDJE_MESSAGE_FLOAT_SET:
	break;
      case EDJE_MESSAGE_STRING_INT:
	break;
      case EDJE_MESSAGE_STRING_FLOAT:
	break;
      case EDJE_MESSAGE_STRING_INT_SET:
	break;
      case EDJE_MESSAGE_STRING_FLOAT_SET:
	break;
      default:
	break;
     }
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

   if (ed->message_num <= 0) return;
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
	if (ed->message_num <= 0) return;
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
	if (ed->message_num <= 0) return;
     }
}

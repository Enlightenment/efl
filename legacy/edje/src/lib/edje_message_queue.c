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
	   case EDJE_MESSAGE_INT:
	   case EDJE_MESSAGE_FLOAT:
	   case EDJE_MESSAGE_INT_SET:
	   case EDJE_MESSAGE_FLOAT_SET:
	     free(em->msg);
	     break;
	   case EDJE_MESSAGE_STRING_FLOAT:
	   case EDJE_MESSAGE_STRING_INT:
	   case EDJE_MESSAGE_STRING_FLOAT_SET:
	   case EDJE_MESSAGE_STRING_INT_SET:
	     memcpy(&ptr, em->msg, sizeof(char *));
	     free(ptr);
	     free(em->msg);
	     break;
	   case EDJE_MESSAGE_SIGNAL:
	     memcpy(&ptr, em->msg, sizeof(char *));
	     free(ptr);
	     memcpy(&ptr, em->msg + sizeof(char *), sizeof(char *));
	     free(ptr);
	     free(em->msg);
	     break;
	   case EDJE_MESSAGE_STRING_SET:
	     memcpy(&count, em->msg, sizeof(int));
	     for (i = 0; i < count; i++)
	       {
		  memcpy(&ptr, em->msg + sizeof(int) + (i * sizeof(char *)), sizeof(char *));
		  free(ptr);
	       }
	     free(em->msg);
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
_edje_message_send(Edje *ed, Edje_Queue queue, Edje_Message_Type type, int id, ...)
{
   /* FIXME: check all malloc fails and gracefully unroll and exit */
   Edje_Message *em;
   va_list args;
   int count = 0, i;
   char *str = NULL, *str2 = NULL, *s;
   int num = 0;
   double flt = 0.0;
   unsigned char *msg = NULL;
   
   em = _edje_message_new(ed, queue, type, id);
   if (!em) return;
   va_start(args, id);

   /* this is evil code - but all we do is pack pointers, ints and doubles */
   /* into the msg generic pointer one after the other */
   switch (em->type)
     {
      case EDJE_MESSAGE_NONE:
	break;
      case EDJE_MESSAGE_SIGNAL:
	str = va_arg(args, char *);
	str2 = va_arg(args, char *);
	msg = malloc(sizeof(char *) * 2);
	s = strdup(str);
	memcpy(msg + (0 * sizeof(char *)), &s, sizeof(char *));
	s = strdup(str2);
	memcpy(msg + (1 * sizeof(char *)), &s, sizeof(char *));
	break;
      case EDJE_MESSAGE_STRING:
	str = va_arg(args, char *);
	msg = strdup(str);
	break;
      case EDJE_MESSAGE_INT:
	num = va_arg(args, int);
	msg = malloc(sizeof(int));
	memcpy(msg, &num, sizeof(int));
	break;
      case EDJE_MESSAGE_FLOAT:
	flt = va_arg(args, double);
	msg = malloc(sizeof(double));
	memcpy(msg, &flt, sizeof(double));
	break;
      case EDJE_MESSAGE_STRING_SET:
	count = va_arg(args, int);
	msg = malloc(sizeof(int) + (count * sizeof(char *)));
	memcpy(msg, &count, sizeof(int));
	for (i = 0; i < count; i++)
	  {
	     str = va_arg(args, char *);
	     s = strdup(str);
	     memcpy(msg + sizeof(int) + (i * sizeof(char *)), &s, sizeof(char *));
	  }
	break;
      case EDJE_MESSAGE_INT_SET:
	count = va_arg(args, int);
	msg = malloc(sizeof(int) + (count * sizeof(int)));
	memcpy(msg, &count, sizeof(int));
	for (i = 0; i < count; i++)
	  {
	     num = va_arg(args, int);
	     memcpy(msg + sizeof(int) + (i * sizeof(int)), &num, sizeof(int));
	  }
	break;
      case EDJE_MESSAGE_FLOAT_SET:
	count = va_arg(args, int);
	msg = malloc(sizeof(int) + (count * sizeof(double)));
	memcpy(msg, &count, sizeof(int));
	for (i = 0; i < count; i++)
	  {
	     flt = va_arg(args, double);
	     memcpy(msg + sizeof(int) + (i * sizeof(double)), &flt, sizeof(double));
	  }
	break;
      case EDJE_MESSAGE_STRING_INT:
	str = va_arg(args, char *);
	num = va_arg(args, int);
	msg = malloc(sizeof(char *) + sizeof(int));
	s = strdup(str);
	memcpy(msg, &s, sizeof(char *));
	memcpy(msg + sizeof(char *), &num, sizeof(int));
	break;
      case EDJE_MESSAGE_STRING_FLOAT:
	str = va_arg(args, char *);
	flt = va_arg(args, double);
	msg = malloc(sizeof(char *) + sizeof(double));
	s = strdup(str);
	memcpy(msg, &s, sizeof(char *));
	memcpy(msg + sizeof(char *), &flt, sizeof(double));
	break;
      case EDJE_MESSAGE_STRING_INT_SET:
	str = va_arg(args, char *);
	count = va_arg(args, int);
	msg = malloc(sizeof (char *) + sizeof(int) + (count * sizeof(int)));
	s = strdup(str);
	memcpy(msg, &s, sizeof(char *));
	memcpy(msg + sizeof(char *), &count, sizeof(int));
	for (i = 0; i < count; i++)
	  {
	     num = va_arg(args, int);
	     memcpy(msg + sizeof(char *) + sizeof(int) + (i * sizeof(int)), &num, sizeof(int));
	  }
	break;
      case EDJE_MESSAGE_STRING_FLOAT_SET:
	str = va_arg(args, char *);
	count = va_arg(args, int);
	msg = malloc(sizeof (char *) + sizeof(int) + (count * sizeof(double)));
	s = strdup(str);
	memcpy(msg, &s, sizeof(char *));
	memcpy(msg + sizeof(char *), &count, sizeof(int));
	for (i = 0; i < count; i++)
	  {
	     flt = va_arg(args, double);
	     memcpy(msg + sizeof(char *) + sizeof(int) + (i * sizeof(double)), &flt, sizeof(double));
	  }
	break;
      default:
	break;
     }
   
   em->msg = msg;
   msgq = evas_list_append(msgq, em);
   
   va_end(args);
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
		  fn = embryo_program_function_find(em->edje->collection->script, "_msg");
		  if (fn != EMBRYO_FUNCTION_NONE)
		    {
		       void *pdata;
		       Embryo_Cell cell;
		       
//		       embryo_parameter_string_push(em->edje->collection->script, sig);
		       /* first param is the integer of the event id - always there */
		       cell = em->id;
		       embryo_parameter_cell_push(em->edje->collection->script, cell);
		       pdata = embryo_program_data_get(em->edje->collection->script);
		       embryo_program_data_set(em->edje->collection->script, em->edje);
		       embryo_program_run(em->edje->collection->script, fn);
		       embryo_program_data_set(em->edje->collection->script, pdata);
		    }
	       }
	     /* call script fn */
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
//	     printf("HANDLE SIG!\n");
	     _edje_emit_handle(em->edje, str1, str2);
	  }
	break;
      case EDJE_MESSAGE_STRING:
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
   if (msgq == NULL) return;

//   printf("PROCESS\n");
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

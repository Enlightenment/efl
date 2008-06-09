#include "ecore_private.h"
#include "Ecore.h"

static int                  events_num = 0;
static Ecore_Event         *events = NULL;

static Ecore_Event_Handler **event_handlers = NULL;
static int                   event_handlers_num = 0;
static int                   event_handlers_alloc_num = 0;
static Ecore_List2_Data     *event_handlers_delete_list = NULL;

static Ecore_Event_Filter  *event_filters = NULL;
static int                  event_filters_delete_me = 0;

static int                  event_id_max = ECORE_EVENT_COUNT;

static int                  ecore_raw_event_type = ECORE_EVENT_NONE;
static void                *ecore_raw_event_event =  NULL;


/**
 * Add an event handler.
 * @param type The type of the event this handler will get called for
 * @param func The function to call when the event is found in the queue
 * @param data A data pointer to pass to the called function @p func
 * @return A new Event handler, or NULL on failure
 * 
 * Add an event handler to the list of handlers. This will, on success, return
 * a handle to the event handler object that was created, that can be used
 * later to remove the handler using ecore_event_handler_del(). The @p type 
 * parameter is the iteger of the event type that will trigger this callback
 * to be called. The callback @p func is called when this event is processed
 * and will be passed the event type, a pointer to the private event
 * structure that is specific to that event type, and a data pointer that is 
 * provided in this call as the @p data parameter.
 * 
 * When the callback @p func is called, it must return 1 or 0. If it returns 
 * 1 (or ECORE_CALLBACK_RENEW), It will keep being called as per normal, for 
 * each handler set up for that event type. If it returns 0 (or 
 * ECORE_CALLBACK_CANCEL), it will cease processing handlers for that particular
 * event, so all handler set to handle that event type that have not already 
 * been called, will not be.
 */
EAPI Ecore_Event_Handler *
ecore_event_handler_add(int type, int (*func) (void *data, int type, void *event), const void *data)
{
   Ecore_Event_Handler *eh;

   if (!func) return NULL;
   if ((type <= ECORE_EVENT_NONE) || (type >= event_id_max)) return NULL;
   eh = calloc(1, sizeof(Ecore_Event_Handler));
   if (!eh) return NULL;
   ECORE_MAGIC_SET(eh, ECORE_MAGIC_EVENT_HANDLER);
   eh->type = type;
   eh->func = func;
   eh->data = (void *)data;
   if (type >= (event_handlers_num - 1))
     {
	int p_alloc_num;
	
	p_alloc_num = event_handlers_alloc_num;
	event_handlers_num = type + 1;
	if (event_handlers_num > event_handlers_alloc_num)
	  {
	     Ecore_Event_Handler **new_handlers;
	     int i;
	     
	     event_handlers_alloc_num = ((event_handlers_num + 16) / 16) * 16;
	     new_handlers = realloc(event_handlers, event_handlers_alloc_num * sizeof(Ecore_Event_Handler *));
	     if (!new_handlers)
	       {
		  free(eh);
		  return NULL;
	       }
	     event_handlers = new_handlers;
	     for (i = p_alloc_num; i < event_handlers_alloc_num; i++)
	       event_handlers[i] = NULL;
	  }
     }
   event_handlers[type] = _ecore_list2_append(event_handlers[type], eh);
   return eh;
}

/**
 * Delete an event handler.
 * @param event_handler Event handler handle to delete
 * @return Data passed to handler
 * 
 * Delete a specified event handler from the handler list. On success this will
 * delete the event handler and return the pointer passed as @p data when the
 * handler was added by ecore_event_handler_add(). On failure NULL will be
 * returned. Once a handler is deleted it will no longer be called.
 */
EAPI void *
ecore_event_handler_del(Ecore_Event_Handler *event_handler)
{
   Ecore_List2_Data *node;
   
   if (!ECORE_MAGIC_CHECK(event_handler, ECORE_MAGIC_EVENT_HANDLER)) 
     {
	ECORE_MAGIC_FAIL(event_handler, ECORE_MAGIC_EVENT_HANDLER,
			 "ecore_event_handler_del");
	return NULL;
     }
   event_handler->delete_me = 1;
   node = calloc(1, sizeof(Ecore_List2_Data));
   node->data = event_handler;
   event_handlers_delete_list = _ecore_list2_append(event_handlers_delete_list, node);
   return event_handler->data;
}

static void 
_ecore_event_generic_free (void *data __UNUSED__, void *event)
{
   free (event);
}

/**
 * Add an event to the event queue.
 * @param type The event type to add to the end of the event queue
 * @param ev The private data structure for this event type
 * @param func_free The function to be called to free this private structure
 * @param data The data pointer to be passed to the free function
 * @return A Handle for that event
 * 
 * On success this function returns a handle to an event on the event queue, or
 * NULL if it fails. If it succeeds, an event of type @p type will be added
 * to the queue for processing by event handlers added by 
 * ecore_event_handler_add(). The @p ev parameter will be a pointer to the event
 * private data that is specific to that event type. When the event is no
 * longer needed, @p func_free will be called and passed the private structure
 * pointer for cleaning up. If @p func_free is NULL, free() will be called
 * with the private structure pointer.
 * func_free is passed @p data as its data parameter.
 */
EAPI Ecore_Event *
ecore_event_add(int type, void *ev, void (*func_free) (void *data, void *ev), void *data)
{
/*   if (!ev) return NULL;*/
   if (type <= ECORE_EVENT_NONE) return NULL;
   if (type >= event_id_max) return NULL;
   if ((ev) && (!func_free)) func_free = _ecore_event_generic_free;
   return _ecore_event_add(type, ev, func_free, data);
}

/**
 * Delete an event from the queue.
 * @param event The event handle to delete
 * @return The data pointer originally set for the event free function
 * 
 * This deletes the event @p event from the event queue, and returns the
 * @p data parameer originally set when adding it with ecore_event_add(). This
 * does not immediately call the free function, and it may be called later on
 * cleanup, and so if the free function depends on the data pointer to work,
 * you should defer cleaning of this till the free function is called later.
 */
EAPI void *
ecore_event_del(Ecore_Event *event)
{
   if (!ECORE_MAGIC_CHECK(event, ECORE_MAGIC_EVENT)) 
     {
	ECORE_MAGIC_FAIL(event, ECORE_MAGIC_EVENT, "ecore_event_del");	
	return NULL;
     }
   event->delete_me = 1;
   return event->data;
}

/**
 * Allocate a new event type id sensibly and return the new id.
 * @return A new event type id.
 * 
 * This function allocates a new event type id and returns it. Once an event 
 * type has been allocated it can never be de-allocated during the life of
 * the program. There is no guarantee of the contents of this event ID, or how
 * it is calculated, except that the ID will be unique to the current instance
 * of the process.
 */
EAPI int
ecore_event_type_new(void)
{
   event_id_max++;
   return event_id_max - 1;
}

/**
 * Add a filter the current event queue.
 * @param func_start Function to call just before filtering and return data
 * @param func_filter Function to call on each event
 * @param func_end Function to call after the queu has been filtered
 * @param data Data to pass to the filter functions
 * @return A filter handle
 * 
 * This adds a filter to call callbacks to loop through the event queue and 
 * filter events out of the queue. On failure NULL is returned. On success a
 * Filter handle is returned. Filters are called on the queue just before
 * Event handler processing to try and remove redundant events. Just as
 * processing starts @p func_start is called and passed the @p data pointer.
 * This function returns a pointer that is used as loop_data that is now passed to
 * @p func_filter as loop_data. @p func_filter is also passed @p data and the
 * event type and private event structure. If this callback returns 0, the
 * event is removed from the queue. If it returns 1, the event is kept. When
 * processing is finished @p func_end is called and is passed the loop_data
 * and @p data pointer to clean up.
 */
EAPI Ecore_Event_Filter *
ecore_event_filter_add(void * (*func_start) (void *data), int (*func_filter) (void *data, void *loop_data, int type, void *event), void (*func_end) (void *data, void *loop_data), const void *data)
{
   Ecore_Event_Filter *ef;
   
   if (!func_filter) return NULL;
   ef = calloc(1, sizeof(Ecore_Event_Filter));
   if (!ef) return NULL;
   ECORE_MAGIC_SET(ef, ECORE_MAGIC_EVENT_FILTER);
   ef->func_start = func_start;
   ef->func_filter = func_filter;
   ef->func_end = func_end;
   ef->data = (void *)data;
   event_filters = _ecore_list2_append(event_filters, ef);
   return ef;
}

/**
 * Delete an event filter.
 * @param ef The event filter handle
 * @return The data set for the filter
 * 
 * Delete a filter that has been added by its @p ef handle. On success this
 * will return the data pointer set when this filter was added. On failure
 * NULL is returned.
 */
EAPI void *
ecore_event_filter_del(Ecore_Event_Filter *ef)
{   
   if (!ECORE_MAGIC_CHECK(ef, ECORE_MAGIC_EVENT_FILTER))
     {
	ECORE_MAGIC_FAIL(ef, ECORE_MAGIC_EVENT_FILTER, "ecore_event_filter_del");
	return NULL;
     }
   ef->delete_me = 1;
   event_filters_delete_me = 1;
   return ef->data;
}

/**
 * Return the current event type being handled.
 * @return The current event type being handled if inside a handler callback
 * 
 * If the program is currently inside an Ecore event handler callback this
 * will return the type of the current event being processed. If Ecore is
 * not inside an event handler, ECORE_EVENT_NONE is returned.
 * 
 * This is useful when certain Ecore modules such as Ecore_Evas "swallow"
 * events and not all the original information is passed on. In special cases
 * this extra information may be useful or needed and using this call can let
 * the program know if the event type being handled is one it wants to get more
 * information about.
 */
EAPI int
ecore_event_current_type_get(void)
{
   return ecore_raw_event_type;
}

/**
 * Return the current event type pointer handled.
 * @return The current event pointer being handled if inside a handler callback
 * 
 * If the program is currently inside an Ecore event handler callback this
 * will return the pointer of the current event being processed. If Ecore is
 * not inside an event handler, NULL will be returned.
 * 
 * This is useful when certain Ecore modules such as Ecore_Evas "swallow"
 * events and not all the original information is passed on. In special cases
 * this extra information may be useful or needed and using this call can let
 * the program access the event data if the type of the event is handled by
 * the program.
 */
EAPI void *
ecore_event_current_event_get(void)
{
   return ecore_raw_event_event;
}

void
_ecore_event_shutdown(void)
{
   int i;
   
   while (events) _ecore_event_del(events);
   for (i = 0; i < event_handlers_num; i++)
     {
	while (event_handlers[i])
	  {
	     Ecore_Event_Handler *eh;
	     
	     eh = event_handlers[i];
	     event_handlers[i] = _ecore_list2_remove(event_handlers[i], eh);
	     ECORE_MAGIC_SET(eh, ECORE_MAGIC_NONE);
	     free(eh);
	  }
     }
   while (event_handlers_delete_list)
     {
	Ecore_List2_Data *ehd;
	
	ehd = event_handlers_delete_list;
	event_handlers_delete_list = _ecore_list2_remove(event_handlers_delete_list, ehd);
	free(ehd);
     }
   if (event_handlers) free(event_handlers);
   event_handlers = NULL;
   event_handlers_num = 0;
   event_handlers_alloc_num = 0;
   while (event_filters)
     {
	Ecore_Event_Filter *ef;
	
	ef = event_filters;
	event_filters = _ecore_list2_remove(event_filters, ef);   
	ECORE_MAGIC_SET(ef, ECORE_MAGIC_NONE);
	free(ef);
     }
   event_filters_delete_me = 0;
}

int
_ecore_event_exist(void)
{
   if (events) return 1;
   return 0;
}

Ecore_Event *
_ecore_event_add(int type, void *ev, void (*func_free) (void *data, void *ev), void *data)
{
   Ecore_Event *e;
   
   e = calloc(1, sizeof(Ecore_Event));
   if (!e) return NULL;
   ECORE_MAGIC_SET(e, ECORE_MAGIC_EVENT);
   e->type = type;
   e->event = ev;
   e->func_free = func_free;
   e->data = data;
   events = _ecore_list2_append(events, e);
   events_num++;
   return e;
}

void *
_ecore_event_del(Ecore_Event *event)
{
   void *data;
   
   data = event->data;
   if (event->func_free) event->func_free(event->data, event->event);
   events = _ecore_list2_remove(events, event);
   ECORE_MAGIC_SET(event, ECORE_MAGIC_NONE);
   free(event);
   events_num--;
   return data;
}

void
_ecore_event_call(void)
{
   Ecore_List2 *l, *ll;
   Ecore_Event *e;
   Ecore_Event_Filter *ef;
   Ecore_Event_Handler *eh;
   Ecore_List2_Data *ehd;
   int handle_count;

   for (l = (Ecore_List2 *)event_filters; l; l = l->next)
     {
	ef = (Ecore_Event_Filter *)l;
	if (!ef->delete_me)
	  {
	     if (ef->func_start)
	       ef->loop_data = ef->func_start(ef->data);
	     for (ll = (Ecore_List2 *)events; ll; ll = ll->next)
	       {
		  e = (Ecore_Event *)ll;
		  if (!ef->func_filter(ef->loop_data, ef->data,
				       e->type, e->event))
		    {
//		       printf("FILTER SAID TO DEL ev %p\n", e->event);
		       ecore_event_del(e);
		    }
	       }
	     if (ef->func_end)
	       ef->func_end(ef->data, ef->loop_data);
	  }
     }
   if (event_filters_delete_me)
     {
	for (l = (Ecore_List2 *)event_filters; l;)
	  {
	     ef = (Ecore_Event_Filter *)l;
	     l = l->next;
	     if (ef->delete_me)
	       {
		  event_filters = _ecore_list2_remove(event_filters, ef);
		  ECORE_MAGIC_SET(ef, ECORE_MAGIC_NONE);
		  free(ef);
	       }
	  }
	event_filters_delete_me = 0;
     }
//   printf("EVENT BATCH...\n");
   for (l = (Ecore_List2 *)events; l; l = l->next)
     {
	e = (Ecore_Event *)l;
	if (!e->delete_me)
	  {
	     handle_count = 0;
	     ecore_raw_event_type = e->type;
	     ecore_raw_event_event = e->event;
//	     printf("HANDLE ev type %i, %p\n", e->type, e->event);
	     if ((e->type >= 0) && (e->type < event_handlers_num))
	       {
		  for (ll = (Ecore_List2 *)event_handlers[e->type]; ll; ll = ll->next)
		    {
		       eh = (Ecore_Event_Handler *)ll;
		       if (!eh->delete_me)
			 {
			    handle_count++;
			    if (!eh->func(eh->data, e->type, e->event))
			      break;  /* 0 == "call no further handlers" */
			 }
		    }
	       }
	     /* if no handlers were set for EXIT signal - then default is */
	     /* to quit the main loop */
	     if ((e->type == ECORE_EVENT_SIGNAL_EXIT) && (handle_count == 0))
	       ecore_main_loop_quit();
	  }
     }
//   printf("EVENT BATCH DONE\n");
   ecore_raw_event_type = ECORE_EVENT_NONE;
   ecore_raw_event_event = NULL;
   
   while (events) _ecore_event_del(events);
   while (event_handlers_delete_list)
     {
	ehd = event_handlers_delete_list;
	eh = ehd->data;
	event_handlers[eh->type] = _ecore_list2_remove(event_handlers[eh->type], eh);
	event_handlers_delete_list = _ecore_list2_remove(event_handlers_delete_list, ehd);
	ECORE_MAGIC_SET(eh, ECORE_MAGIC_NONE);
	free(eh);
	free(ehd);
     }
}

EAPI void *
_ecore_event_signal_user_new(void)
{
   Ecore_Event_Signal_User *e;
   
   e = calloc(1, sizeof(Ecore_Event_Signal_User));
   return e;
}

void *
_ecore_event_signal_hup_new(void)
{
   Ecore_Event_Signal_Hup *e;
   
   e = calloc(1, sizeof(Ecore_Event_Signal_Hup));
   return e;
}

void *
_ecore_event_signal_exit_new(void)
{
   Ecore_Event_Signal_Exit *e;
   
   e = calloc(1, sizeof(Ecore_Event_Signal_Exit));
   return e;
}

void *
_ecore_event_signal_power_new(void)
{
   Ecore_Event_Signal_Power *e;
   
   e = calloc(1, sizeof(Ecore_Event_Signal_Power));
   return e;
}

void *
_ecore_event_signal_realtime_new(void)
{
   return calloc(1, sizeof(Ecore_Event_Signal_Realtime));
}

#include "Ecore.h"

typedef struct _ecore_event_handler Ecore_Event_Handler;
typedef struct _ecore_event_idle_handler Ecore_Event_Idle_Handler;

struct _ecore_event_handler
{
  void                (*func) (Ecore_Event * ev);
  Ecore_Event_Handler *next;
};

struct _ecore_event_idle_handler
{
  void                (*func) (void *data);
  void               *data;
  Ecore_Event_Idle_Handler *next;
};

static Ecore_Event_Handler *handler[ECORE_EVENT_MAX];
static Ecore_Event_Idle_Handler *idle_handlers = NULL;

void
ecore_event_filter(Ecore_Event * ev)
{
  Ecore_Event        *evp;
  int                 motion_events = 0;
  int                 dnd_pos_events = 0;
  int                 dnd_status_events = 0;

  /* count events to only use last events of some types */
  for (evp = ev; evp; evp = evp->next)
    {
      if (evp->type == ECORE_EVENT_MOUSE_MOVE)
	motion_events++;
      if (evp->type == ECORE_EVENT_DND_DROP_POSITION)
	dnd_pos_events++;
      if (evp->type == ECORE_EVENT_DND_DROP_STATUS)
	dnd_status_events++;
    }
  for (evp = ev; evp; evp = evp->next)
    {
      if (evp->type == ECORE_EVENT_MOUSE_MOVE)
	{
	  if (motion_events > 1)
	    {
	      evp->ignore = 1;
	      motion_events--;
	    }
	}
      else if (evp->type == ECORE_EVENT_DND_DROP_POSITION)
	{
	  if (dnd_pos_events > 1)
	    {
	      evp->ignore = 1;
	      dnd_pos_events--;
	    }
	}
      else if (evp->type == ECORE_EVENT_DND_DROP_STATUS)
	{
	  if (dnd_status_events > 1)
	    {
	      evp->ignore = 1;
	      dnd_status_events--;
	    }
	}
    }
}

void
ecore_event_filter_events_handle(Ecore_Event * ev)
{
  Ecore_Event        *evp;

  for (evp = ev; evp; evp = evp->next)
    {
      Ecore_Event_Handler *h;

      if (!evp->ignore)
	{
	  for (h = handler[evp->type]; h; h = h->next)
	    {
	      if (h->func)
		h->func(evp);
	    }
	}
    }
}

void
ecore_event_filter_idle_handle(void)
{
  Ecore_Event_Idle_Handler *h;

  for (h = idle_handlers; h; h = h->next)
    h->func(h->data);
}

extern int          __quit_ev_loop;

void
ecore_event_filter_init(void)
{
  int                 i;

  __quit_ev_loop = 0;
  for (i = 0; i < ECORE_EVENT_MAX; i++)
    handler[i] = NULL;
}

void
ecore_event_filter_handler_add(Ecore_Event_Type type,
			       void (*func) (Ecore_Event * ev))
{
  Ecore_Event_Handler *h;

  h = NEW(Ecore_Event_Handler, 1);
  h->func = func;
  h->next = handler[type];
  handler[type] = h;
}

void
ecore_event_filter_idle_handler_add(void (*func) (void *data), void *data)
{
  Ecore_Event_Idle_Handler *h;

  h = NEW(Ecore_Event_Idle_Handler, 1);
  h->func = func;
  h->data = data;
  h->next = idle_handlers;
  idle_handlers = h;
}

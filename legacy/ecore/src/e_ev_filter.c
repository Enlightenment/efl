#include "Ecore.h"

typedef struct _ev_handler Ev_Handler;
typedef struct _ev_idle_handler Ev_Idle_Handler;

struct _ev_handler
{
   void                (*func) (Eevent * ev);
   Ev_Handler         *next;
};

struct _ev_idle_handler
{
   void                (*func) (void *data);
   void               *data;
   Ev_Idle_Handler    *next;
};

static Ev_Handler  *handler[EV_MAX];
static Ev_Idle_Handler *idle_handlers = NULL;

void
e_event_filter(Eevent * ev)
{
   Eevent             *evp;
   int                 motion_events = 0;
   int                 dnd_pos_events = 0;
   int                 dnd_status_events = 0;

   /* count events to only use last events of some types */
   for (evp = ev; evp; evp = evp->next)
     {
	if (evp->type == EV_MOUSE_MOVE)
	   motion_events++;
	if (evp->type == EV_DND_DROP_POSITION)
	   dnd_pos_events++;
	if (evp->type == EV_DND_DROP_STATUS)
	   dnd_status_events++;
     }
   for (evp = ev; evp; evp = evp->next)
     {
	if (evp->type == EV_MOUSE_MOVE)
	  {
	     if (motion_events > 1)
	       {
		  evp->ignore = 1;
		  motion_events--;
	       }
	  }
	else if (evp->type == EV_DND_DROP_POSITION)
	  {
	     if (dnd_pos_events > 1)
	       {
		  evp->ignore = 1;
		  dnd_pos_events--;
	       }
	  }
	else if (evp->type == EV_DND_DROP_STATUS)
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
e_event_filter_events_handle(Eevent * ev)
{
   Eevent             *evp;

   for (evp = ev; evp; evp = evp->next)
     {
	Ev_Handler         *h;

	if (!evp->ignore)
	  {
	     for (h = handler[evp->type]; h; h = h->next)
	       {
		  if (h->func) h->func(evp);
	       }
	  }
     }
}

void
e_event_filter_idle_handle(void)
{
   Ev_Idle_Handler    *h;

   for (h = idle_handlers; h; h = h->next)
      h->func(h->data);
}

extern int __quit_ev_loop;

void
e_event_filter_init(void)
{
   int                 i;

   __quit_ev_loop = 0;
   for (i = 0; i < EV_MAX; i++)
      handler[i] = NULL;
}

void
e_event_filter_handler_add(Eevent_Type type, void (*func) (Eevent * ev))
{
   Ev_Handler         *h;

   h = NEW(Ev_Handler, 1);
   h->func = func;
   h->next = handler[type];
   handler[type] = h;
}

void
e_event_filter_idle_handler_add(void (*func) (void *data), void *data)
{
   Ev_Idle_Handler    *h;

   h = NEW(Ev_Idle_Handler, 1);
   h->func = func;
   h->data = data;
   h->next = idle_handlers;
   idle_handlers = h;
}

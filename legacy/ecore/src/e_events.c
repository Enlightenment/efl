#include "Ecore.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* glocal (yes global/local) variabels for events */
Ecore_Event_Fd_Handler *fd_handlers = NULL;
Ecore_Event_Ipc_Handler *ipc_handlers = NULL;
Ecore_Event_Pid_Handler *pid_handlers = NULL;
Ecore_Event_Timer  *timers = NULL;

Ecore_Event        *events = NULL;
Ecore_Event        *last_event = NULL;

int                 __quit_ev_loop = 0;

/* local functions for event handling */
static void         ecore_handle_event_timer(void);
static void         ecore_handle_zero_event_timer(void);

/* public functions */

/* add an event to the end of the event queue */
void
ecore_add_event(Ecore_Event_Type type, void *event,
		void (*ev_free) (void *event))
{
  Ecore_Event        *ev;

  ev = NEW(Ecore_Event, 1);
  ev->type = type;
  ev->ignore = 0;
  ev->event = event;
  ev->next = NULL;
  ev->ev_free = ev_free;
  if (!events)
    events = ev;
  else
    last_event->next = ev;
  last_event = ev;
}

/* delete an event from the event queue */
void
ecore_del_event(void *event)
{
  Ecore_Event        *ev, *pev;

  pev = NULL;
  ev = events;
  while (ev)
    {
      if (ev->event == event)
	{
	  if (pev)
	    pev->next = ev->next;
	  else
	    events = ev->next;
	  if (!ev->next)
	    last_event = pev;
	  if ((ev->event) && (ev->ev_free))
	    (*ev->ev_free) (ev->event);
	  FREE(ev);
	  return;
	}
      pev = ev;
      ev = ev->next;
    }
}

void
ecore_del_all_events(void)
{
  Ecore_Event        *ev, *pev;

  ev = events;
  while (ev)
    {
      pev = ev;
      ev = ev->next;
      if ((pev->event) && (pev->ev_free))
	pev->ev_free(pev->event);
      FREE(pev);
    }
  events = NULL;
  last_event = NULL;
}

Ecore_Event        *
ecore_get_last_event(void)
{
  return last_event;
}

/* add a callback handler if fd is active for reading */
void
ecore_add_event_fd(int fd, void (*func) (int fd))
{
  Ecore_Event_Fd_Handler *fd_h;

  /* new handler struct */
  fd_h = NEW(Ecore_Event_Fd_Handler, 1);
  fd_h->next = fd_handlers;
  fd_h->fd = fd;
  fd_h->func = func;
  fd_handlers = fd_h;
}

/* delete handler for fd */
void
ecore_del_event_fd(int fd)
{
  START_LIST_DEL(Ecore_Event_Fd_Handler, fd_handlers, (_p->fd == fd));
  FREE(_p);
  END_LIST_DEL;
}

void
ecore_add_event_pid(pid_t pid, void (*func) (pid_t pid))
{
  Ecore_Event_Pid_Handler *pid_h;

  /* delete the old handler */
  ecore_del_event_pid(pid);
  /* new handler struct */
  pid_h = NEW(Ecore_Event_Pid_Handler, 1);
  pid_h->next = pid_handlers;
  pid_h->pid = pid;
  pid_h->func = func;
  pid_handlers = pid_h;
}

void
ecore_del_event_pid(pid_t pid)
{
  START_LIST_DEL(Ecore_Event_Pid_Handler, pid_handlers, (_p->pid == pid));
  FREE(_p);
  END_LIST_DEL;
}

void
ecore_add_event_ipc(int ipc, void (*func) (int ipc))
{
  Ecore_Event_Ipc_Handler *ipc_h;

  /* delete the old handler */
  ecore_del_event_ipc(ipc);
  /* new handler struct */
  ipc_h = NEW(Ecore_Event_Ipc_Handler, 1);
  ipc_h->next = ipc_handlers;
  ipc_h->ipc = ipc;
  ipc_h->func = func;
  ipc_handlers = ipc_h;
}

void
ecore_del_event_ipc(int ipc)
{
  START_LIST_DEL(Ecore_Event_Ipc_Handler, ipc_handlers, (_p->ipc == ipc));
  FREE(_p);
  END_LIST_DEL;
}

/* sit in this loop forever and process events */
void
ecore_event_loop(void)
{
  int                 fdcount, fdsize, ipccount, ipcsize;
  int                 timed_out, were_events;
  double              time1, time2, prev_time = 0.0;
  struct timeval      tval;
  fd_set              fdset, ipcset;
  Ecore_Event_Fd_Handler *fd_h;
  Ecore_Event_Pid_Handler *pid_h;
  Ecore_Event_Ipc_Handler *ipc_h;

  /* init some of the time variables */
  time1 = ecore_get_time();
  time2 = time1 - prev_time;
  prev_time = time1;
  while (__quit_ev_loop == 0)
    {
      /* state setup */
      timed_out = 0;
      were_events = 0;

      /* setup fd array from list of listening fd's */
      fdsize = 0;
      FD_ZERO(&fdset);
      /* for ever fd handler add the fd to the array and incriment fdsize */
      for (fd_h = fd_handlers; fd_h; fd_h = fd_h->next)
	{
	  FD_SET(fd_h->fd, &fdset);
	  if (fd_h->fd > fdsize)
	    fdsize = fd_h->fd;
	}
      fdcount = 1;
      ipcsize = 0;
      FD_ZERO(&ipcset);
      /* for ever fd handler add the fd to the array and incriment fdsize */
      for (ipc_h = ipc_handlers; ipc_h; ipc_h = ipc_h->next)
	{
	  FD_SET(ipc_h->ipc, &ipcset);
	  if (ipc_h->ipc > ipcsize)
	    ipcsize = ipc_h->ipc;
	}
      ipccount = 1;
      /* if there are timers setup adjust timeout value and select */
      if (timers)
	{
	  if (timers->just_added)
	    {
	      timers->just_added = 0;
	      time1 = timers->in;
	    }
	  else
	    {
	      time1 = timers->in - time2;
	      if (time1 < 0.0)
		time1 = 0.0;
	      timers->in = time1;
	    }
	  tval.tv_sec = (long)time1;
	  tval.tv_usec = (long)((time1 - ((double)tval.tv_sec)) * 1000000);
	  if (tval.tv_sec < 0)
	    tval.tv_sec = 0;
	  if (tval.tv_usec <= 1000)
	    tval.tv_usec = 1000;
	  ecore_handle_zero_event_timer();
	  if ((!ecore_events_pending()) && (!ecore_event_signal_events_pending()))
	    fdcount = select(fdsize + 1, &fdset, NULL, NULL, &tval);
	}
      /* no timers - just sit and block */
      else
	{
	  if ((!ecore_events_pending()) && (!ecore_event_signal_events_pending()))
	    fdcount = select(fdsize + 1, &fdset, NULL, NULL, NULL);
	}
      for (pid_h = pid_handlers; pid_h; pid_h = pid_h->next)
	pid_h->func(pid_h->pid);

      /* see if we have any new ipc connections */
      tval.tv_sec = 0;
      tval.tv_usec = 0;
      ipccount += select(ipcsize + 1, &ipcset, NULL, NULL, &tval);

      /* return < 0 - error or signal interrupt */
      if (fdcount < 0)
	{
	  /* error */
	  if ((errno == ENOMEM) || (errno == EINVAL) || (errno == EBADF))
	    {
	      fprintf(stderr, "Lost connection to X display.\n");
	      exit(1);
	    }
	}
      /* timers are available and its a timeout */
      if ((timers) && (fdcount == 0))
	{
	  ecore_handle_event_timer();
	  timed_out = 1;
	}
      if (fdcount < 0)
	fdcount = 0;
      if (ecore_events_pending())
	{
	  fdcount++;
	  FD_SET(ecore_x_get_fd(), &fdset);
	}
      /* fd's are active */
      if (fdcount > 0)
	{
	  /* for every fd handler - if its fd is set - call the func */
	  for (fd_h = fd_handlers; fd_h;)
	    {
	      Ecore_Event_Fd_Handler *fdh;

	      fdh = fd_h;
	      fd_h = fd_h->next;
	      if (FD_ISSET(fdh->fd, &fdset))
		fdh->func(fdh->fd);
	    }
	}

      /* ipc clients are active */
      if (ipccount > 0)
	{
	  for (ipc_h = ipc_handlers; ipc_h;)
	    {
	      Ecore_Event_Ipc_Handler *ipch;

	      ipch = ipc_h;
	      ipc_h = ipc_h->next;
	      if (FD_ISSET(ipch->ipc, &ipcset))
		ipch->func(ipch->ipc);
	    }
	}
      if (events)
	ecore_event_filter(events);
      if (events)
	{
	  ecore_event_filter_events_handle(events);
	  were_events = 1;
	}
      ecore_del_all_events();
      if ((timed_out) || (were_events))
	ecore_event_filter_idle_handle();
      ecore_flush();
      time1 = ecore_get_time();
      time2 = time1 - prev_time;
      prev_time = time1;
    }
   __quit_ev_loop = 0;
}

/* set a flag to 0 so that we can quit the event loop and shutdown 
 * properly */
void
ecore_event_loop_quit(void)
{
  __quit_ev_loop = 1;
}

/* add a timeout funcitont o be called in "in" seconds with name name */
void
ecore_add_event_timer(char *name, double in, void (*func) (int val, void *data),
		      int val, void *data)
{
  Ecore_Event_Timer  *timer, *ptr, *pptr;
  double              tally;

  if (name)
    ecore_del_event_timer(name);
  timer = NEW(Ecore_Event_Timer, 1);
  timer->next = NULL;
  timer->func = func;
  timer->data = data;
  timer->val = val;
  timer->just_added = 1;
  timer->in = in;
  timer->name = strdup(name);
  if (!timers)
    timers = timer;
  else
    {
      pptr = NULL;
      ptr = timers;
      tally = 0.0;
      while (ptr)
	{
	  tally += ptr->in;
	  if (tally > in)
	    {
	      tally -= ptr->in;
	      timer->next = ptr;
	      if (pptr)
		pptr->next = timer;
	      else
		timers = timer;
	      timer->in -= tally;
	      if (timer->next)
		timer->next->in -= timer->in;
	      return;
	    }
	  pptr = ptr;
	  ptr = ptr->next;
	}
      if (pptr)
	pptr->next = timer;
      else
	timers = timer;
      timer->in -= tally;
    }
}

/* delete a timer timeout entry named "name" */
void               *
ecore_del_event_timer(char *name)
{
  Ecore_Event_Timer  *timer, *ptr, *pptr;

  pptr = NULL;
  ptr = timers;
  while (ptr)
    {
      timer = ptr;
      if (!strcmp(timer->name, name))
	{
	  void               *data;

	  if (pptr)
	    pptr->next = timer->next;
	  else
	    timers = timer->next;
	  if (timer->next)
	    timer->next->in += timer->in;
	  IF_FREE(timer->name);
	  data = timer->data;
	  FREE(timer);
	  return data;
	}
      pptr = ptr;
      ptr = ptr->next;
    }
  return NULL;
}

/* private functions */
static void
ecore_handle_event_timer(void)
{
  Ecore_Event_Timer  *timer;

  if (!timers)
    return;
  timer = timers;
  timers = timer->next;
  (*(timer->func)) (timer->val, timer->data);
  IF_FREE(timer->name);
  FREE(timer);
}

static void
ecore_handle_zero_event_timer(void)
{
  while ((timers) && (timers->in == 0.0))
    ecore_handle_event_timer();
}

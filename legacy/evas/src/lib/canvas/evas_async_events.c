#include "evas_common.h"
#include "evas_private.h"

#ifdef BUILD_ASYNC_EVENTS

#ifndef _MSC_VER
# include <unistd.h>
#endif
#include <fcntl.h>
#include <errno.h>

static int _fd_write = -1;
static int _fd_read = -1;

static int _init_evas_event = 0;

typedef struct _Evas_Event_Async	Evas_Event_Async;

struct _Evas_Event_Async
{
   const void		    *target;
   void			    *event_info;
   Evas_Async_Events_Put_Cb  func;
   Evas_Callback_Type	     type;
};

int
evas_async_events_init(void)
{
   int filedes[2];

   _init_evas_event++;
   if (_init_evas_event > 1) return _init_evas_event;

   if (pipe(filedes) == -1)
     {
	_init_evas_event = 0;
	return 0;
     }

   _fd_read = filedes[0];
   _fd_write = filedes[1];

   fcntl(_fd_read, F_SETFL, O_NONBLOCK);

   return _init_evas_event;
}

int
evas_async_events_shutdown(void)
{
   _init_evas_event--;
   if (_init_evas_event > 0) return _init_evas_event;

   close(_fd_read);
   close(_fd_write);
   _fd_read = -1;
   _fd_write = -1;

   return _init_evas_event;
}

#endif

EAPI int
evas_async_events_fd_get(void)
{
#ifdef BUILD_ASYNC_EVENTS
   return _fd_read;
#else
   return -1;
#endif
}

EAPI int
evas_async_events_process(void)
{
#ifdef BUILD_ASYNC_EVENTS
   Evas_Event_Async *ev;
   int check;
   int count = 0;

   if (_fd_read == -1) return 0;

   do 
     {
	check = read(_fd_read, &ev, sizeof (Evas_Event_Async *));
        
	if (check == sizeof (Evas_Event_Async *))
	  {
             if (ev->func) ev->func((void *)ev->target, ev->type, ev->event_info);
	     free(ev);
	     count++;
	  }
     } 
   while (check > 0);

   evas_cache_image_wakeup();

   if (check < 0)
     {
        switch (errno)
          {
           case EBADF:
           case EINVAL:
           case EIO:
           case EISDIR:
             _fd_read = -1;
          }
     }

   return count;
#else
   return 0;
#endif
}

EAPI Eina_Bool
evas_async_events_put(const void *target, Evas_Callback_Type type, void *event_info, Evas_Async_Events_Put_Cb func)
{
#ifdef BUILD_ASYNC_EVENTS
   Evas_Event_Async *ev;
   ssize_t check;
   Eina_Bool result = EINA_FALSE;

   if (!func) return 0;
   if (_fd_write == -1) return 0;

   ev = calloc(1, sizeof (Evas_Event_Async));
   if (!ev) return 0;

   ev->func = func;
   ev->target = target;
   ev->type = type;
   ev->event_info = event_info;

   do 
     {
        check = write(_fd_write, &ev, sizeof (Evas_Event_Async*));
     }
   while ((check != sizeof (Evas_Event_Async*)) && 
          ((errno == EINTR) || (errno == EAGAIN)));

   evas_cache_image_wakeup();

   if (check == sizeof (Evas_Event_Async*))
     result = EINA_TRUE;
   else
     {
        switch (errno)
          {
           case EBADF:
           case EINVAL:
           case EIO:
           case EPIPE:
             _fd_write = -1;
          }
     }

   return result;
#else
   func(target, type, event_info);
   return EINA_TRUE;
#endif
}

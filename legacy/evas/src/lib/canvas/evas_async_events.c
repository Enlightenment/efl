#include "evas_common.h"
#include "evas_private.h"

#ifdef BUILD_ASYNC_EVENTS

#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>

static int _fd_write = -1;
static int _fd_read = -1;

static int _init_evas_event = 0;
static pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct _Evas_Event_Async	Evas_Event_Async;
struct _Evas_Event_Async
{
   const void		 *target;
   void			 *event_info;
   void			(*func)(void *target, Evas_Callback_Type type, void *event_info);
   Evas_Callback_Type	  type;
} __PACKED__ ;

#endif

int
evas_async_events_init(void)
{
#ifdef BUILD_ASYNC_EVENTS
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
#else
   return 0;
#endif
}

int
evas_async_events_shutdown(void)
{
#ifdef BUILD_ASYNC_EVENTS
   _init_evas_event--;
   if (_init_evas_event > 0) return _init_evas_event;

   close(_fd_read);
   close(_fd_write);
   _fd_read = -1;
   _fd_write = -1;

   return _init_evas_event;
#else
   return 0;
#endif
}

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
   static Evas_Event_Async current;
   static int size = 0;
   int check;
   int count = 0;

   if (_fd_read == -1) return 0;

   do
     {
	check = read(_fd_read, ((char*) &current) + size, sizeof(current) - size);

	if (check > 0)
	  {
	     size += check;
	     if (size == sizeof(current))
	       {
		  if (current.func) current.func((void*) current.target, current.type, current.event_info);
		  size = 0;
		  count++;
	       }
	  }
     }
   while (check > 0);

   if (check < 0)
     switch (errno)
       {
	case EBADF:
	case EINVAL:
	case EIO:
	case EISDIR:
	   _fd_read = -1;
       }

   return count;
#else
   return 0;
#endif
}

EAPI Evas_Bool
evas_async_events_put(const void *target, Evas_Callback_Type type, void *event_info, void (*func)(void *target, Evas_Callback_Type type, void *event_info))
{
#ifdef BUILD_ASYNC_EVENTS
   Evas_Event_Async new;
   ssize_t check;
   int offset = 0;
   Evas_Bool result = 0;

   if (!func) return 0;
   if (_fd_write == -1) return 0;

   new.func = func;
   new.target = target;
   new.type = type;
   new.event_info = event_info;

   pthread_mutex_lock(&_mutex);

   do {
      check = write(_fd_write, ((char*)&new) + offset, sizeof(new) - offset);
      offset += check;
   } while (offset != sizeof(new) && (errno == EINTR || errno == EAGAIN));

   if (offset == sizeof(new))
     result = 1;
   else
     switch (errno)
       {
	case EBADF:
	case EINVAL:
	case EIO:
	case EPIPE:
	   _fd_write = -1;
       }

   pthread_mutex_unlock(&_mutex);

   return result;
#else
   func(target, type, event_info);

   return 1;
#endif
}


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

/**
 * @brief Get evas' internal asynchronous events read file descriptor.
 *
 * @return The canvas' asynchronous events read file descriptor.
 *
 * Evas' asynchronous events are meant to be dealt with internally,
 * i. e., when building stuff to be glued together into the EFL
 * infrastructure -- a module, for example. The context which demands
 * its use is when calculations need to be done out of the main
 * thread, asynchronously, and some action must be performed after
 * that.
 *
 * An example of actual use of this API is for image asynchronous
 * preload inside evas. If the canvas was instantiated throught
 * ecore-evas usage, ecore itself will take care of calling those
 * events' processing.
 *
 * This function returns the read file descriptor where to get the
 * asynchronous events of the canvas. Naturally, other mainloops,
 * apart from ecore, may make use of it.
 *
 */
EAPI int
evas_async_events_fd_get(void)
{
#ifdef BUILD_ASYNC_EVENTS
   return _fd_read;
#else
   return -1;
#endif
}

/**
 * @brief Trigger the processing of all events waiting on the file
 * descriptor returned by evas_async_events_fd_get().
 *
 * @return The number of events processed.
 *
 * All asynchronous events queued up by evas_async_events_put() are
 * processed here. More precisely, the callback functions, informed
 * together with other event parameters, when queued, get called (with
 * those parameters), in that order.
 *
 */
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

/**
 * Insert asynchronous events on the canvas.
 *
 * @param target The target to be affected by the events.
 * @param type The type of callback function.
 * @param event_info Information about the event.
 * @param func The callback function pointer.
 *
 * This is the way, for a routine running outside evas' main thread,
 * to report an asynchronous event. A callback function is informed,
 * whose call is to happen after evas_async_events_process() is
 * called.
 *
 */
EAPI Eina_Bool
evas_async_events_put(const void *target, Evas_Callback_Type type, void *event_info, void (*func)(void *target, Evas_Callback_Type type, void *event_info))
{
#ifdef BUILD_ASYNC_EVENTS
   Evas_Event_Async new;
   ssize_t check;
   int offset = 0;
   Eina_Bool result = EINA_FALSE;

   if (!func) return 0;
   if (_fd_write == -1) return 0;

   memset(&new, 0, sizeof (new));

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
     result = EINA_TRUE;
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

   return EINA_TRUE;
#endif
}

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

static int               queue_num = 0;
static int               queue_alloc = 0;
static Evas_Event_Async *queue = NULL;

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

int
evas_async_target_del(const void *target)
{
   int i, j, d = 0;
   
   pthread_mutex_lock(&_mutex);
   if (queue)
     {
        for (i = 0; i < queue_num; i++)
          {
             if (queue[i].target == target)
               {
                  for (j = i + 1; j < queue_num; j++)
                    memcpy(&(queue[j - 1]), &(queue[j]), sizeof(Evas_Event_Async));
                  i--;
                  queue_num--;
                  d++;
               }
          }
        if (queue_num == 0)
          {
             free(queue);
             queue = NULL;
             queue_alloc = 0;
          }
     }
   pthread_mutex_unlock(&_mutex);
   return d;
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
   Evas_Event_Async *ev;
   unsigned char buf[1];
   int i;
   int check;
   int count = 0;
   int               myqueue_num = 0;
   int               myqueue_alloc = 0;
   Evas_Event_Async *myqueue = NULL;
   
   if (_fd_read == -1) return 0;
   
   pthread_mutex_lock(&_mutex);
   do
     {
	check = read(_fd_read, buf, 1);
     }
   while (check > 0);
   
   if (queue)
     {
        myqueue_num = queue_num;
        myqueue_alloc = queue_alloc;
        myqueue = queue;
        queue_num = 0;
        queue_alloc = 0;
        queue = NULL;
        pthread_mutex_unlock(&_mutex);
        
        for (i = 0; i < myqueue_num; i++)
          {
             ev = &(myqueue[i]);
             if (ev->func) ev->func((void *)ev->target, ev->type, ev->event_info);
             count++;
          }
        free(myqueue);
     }
   else
     pthread_mutex_unlock(&_mutex);
   
   if (check < 0)
     switch (errno)
       {
       case EBADF:
       case EINVAL:
       case EIO:
       case EISDIR:
          _fd_read = -1;
       }
   
   evas_cache_pending_process();
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
   Evas_Event_Async *ev;
   ssize_t check;
   Eina_Bool result = EINA_FALSE;

   if (!func) return 0;
   if (_fd_write == -1) return 0;

   pthread_mutex_lock(&_mutex);
   
   queue_num++;
   if (queue_num > queue_alloc)
     {
        Evas_Event_Async *q2;
        
        queue_alloc += 32; // 32 slots at a time for async events
        q2 = realloc(queue, queue_alloc * sizeof(Evas_Event_Async));
        if (!q2)
          {
             queue_alloc -= 32;
             queue_num--;
             pthread_mutex_unlock(&_mutex);
             return 0;
          }
        queue = q2;
     }
   ev = &(queue[queue_num - 1]);
   memset(ev, 0, sizeof(Evas_Event_Async));
   ev->func = func;
   ev->target = target;
   ev->type = type;
   ev->event_info = event_info;

   do
     {
        unsigned char buf[1] = { 0xf0 };
        check = write(_fd_write, buf, 1);
     } while ((check != 1) && ((errno == EINTR) || (errno == EAGAIN)));

   if (check == 1)
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

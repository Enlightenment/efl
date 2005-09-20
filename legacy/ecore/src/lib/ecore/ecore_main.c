#include "ecore_private.h"
#include "Ecore.h"

#ifdef WIN32
#include <winsock.h>
#endif

#define FIX_HZ 1   

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#ifdef FIX_HZ
#include <sys/param.h>
#ifndef HZ
#define HZ 100
#endif
#endif


static int  _ecore_main_select(double timeout);
static void _ecore_main_fd_handlers_cleanup(void);
static void _ecore_main_fd_handlers_call(void);
static int  _ecore_main_fd_handlers_buf_call(void);
static void _ecore_main_loop_iterate_internal(int once_only);

static int               in_main_loop = 0;
static int               do_quit = 0;
static Ecore_Fd_Handler *fd_handlers = NULL;
static int               fd_handlers_delete_me = 0;

static double            t1 = 0.0;
static double            t2 = 0.0;

/**
 * @defgroup Ecore_Main_Loop_Group Main Loop Functions
 *
 * These functions control the Ecore event handling loop.  This loop is
 * designed to work on embedded systems all the way to large and
 * powerful mutli-cpu workstations.
 *
 * It serialises all system signals and events into a single event
 * queue, that can be easily processed without needing to worry about
 * concurrency.  A properly written, event-driven program using this
 * kind of programming does not need threads.  It makes the program very
 * robust and easy to follow.
 * 
 * Here is an example of simple program and its basic event loop flow:
 * @image html prog_flow.png
 *
 * For examples of setting up and using a main loop, see
 * @ref event_handler_example.c and @ref timer_example.c.
 */

/**
 * Runs a single iteration of the main loop to process everything on the
 * queue.
 * @ingroup Ecore_Main_Loop_Group
 */
void
ecore_main_loop_iterate(void)
{
   _ecore_main_loop_iterate_internal(1);
}

/**
 * Runs the application main loop.
 *
 * This function will not return until @ref ecore_main_loop_quit is called.
 *
 * @ingroup Ecore_Main_Loop_Group
 */
void
ecore_main_loop_begin(void)
{
   in_main_loop++;
   for (;do_quit == 0;) _ecore_main_loop_iterate_internal(0);
   do_quit = 0;
   in_main_loop--;
}

/**
 * Quits the main loop once all the events currently on the queue have
 * been processed.
 * @ingroup Ecore_Main_Loop_Group
 */
void
ecore_main_loop_quit(void)
{
   do_quit = 1;
}

/**
 * @defgroup Ecore_FD_Handler_Group File Event Handling Functions
 *
 * Functions that deal with file descriptor handlers.
 */

/**
 * Adds a callback for activity on the given file descriptor.
 *
 * @p func will be called during the execution of @ref ecore_main_loop_begin
 * when the file descriptor is available for reading, or writing, or both.
 * @p buf_func is called during event loop handling to check if data that has
 * been read from the file descriptor is in a buffer and is available to
 * read.
 *
 * @param   fd       The file descriptor to watch.
 * @param   flags    To watch it for read (@c ECORE_FD_READ) and/or
 *                   (@c ECORE_FD_WRITE) write ability.  @c ECORE_FD_ERROR
 *                   
 * @param   func     The callback function.
 * @param   data     The data to pass to the callback.
 * @param   buf_func The function to call to check if any data has been
 *                   buffered and already read from the fd.  Can be @c NULL.
 * @param   buf_data The data to pass to the @p buf_func function.
 * @return  A fd handler handle if successful.  @c NULL otherwise.
 * @ingroup Ecore_FD_Handler_Group
 */
Ecore_Fd_Handler *
ecore_main_fd_handler_add(int fd, Ecore_Fd_Handler_Flags flags, int (*func) (void *data, Ecore_Fd_Handler *fd_handler), const void *data, int (*buf_func) (void *buf_data, Ecore_Fd_Handler *fd_handler), const void *buf_data)
{
   Ecore_Fd_Handler *fdh;

   if ((fd < 0) ||
       (flags == 0) ||
       (!func)) return NULL;
   fdh = calloc(1, sizeof(Ecore_Fd_Handler));
   if (!fdh) return NULL;
   ECORE_MAGIC_SET(fdh, ECORE_MAGIC_FD_HANDLER);
   fdh->fd = fd;
   fdh->flags = flags;
   fdh->read_active = 0;
   fdh->write_active = 0;
   fdh->error_active = 0;
   fdh->delete_me = 0;
   fdh->func = func;
   fdh->data = (void *)data;
   fdh->buf_func = buf_func;
   fdh->buf_data = (void *)buf_data;
   fd_handlers = _ecore_list2_append(fd_handlers, fdh);
   return fdh;
}

/**
 * Deletes the given FD handler.
 * @param   fd_handler The given FD handler.
 * @return  The data pointer set using @ref ecore_main_fd_handler_add,
 *          for @p fd_handler on success.  @c NULL otherwise.
 * @ingroup Ecore_FD_Handler_Group
 */
void *
ecore_main_fd_handler_del(Ecore_Fd_Handler *fd_handler)
{
   if (!ECORE_MAGIC_CHECK(fd_handler, ECORE_MAGIC_FD_HANDLER))
     {
	ECORE_MAGIC_FAIL(fd_handler, ECORE_MAGIC_FD_HANDLER, 
			 "ecore_main_fd_handler_del");
	return NULL;
     }
   fd_handler->delete_me = 1;
   fd_handlers_delete_me = 1;
   return fd_handler->data;
}

void
ecore_main_fd_handler_prepare_callback_set(Ecore_Fd_Handler *fd_handler, void (*func) (void *data, Ecore_Fd_Handler *fd_handler), const void *data)
{
   if (!ECORE_MAGIC_CHECK(fd_handler, ECORE_MAGIC_FD_HANDLER))
     {
	ECORE_MAGIC_FAIL(fd_handler, ECORE_MAGIC_FD_HANDLER, 
			 "ecore_main_fd_handler_prepare_callback_set");
	return;
     }
   fd_handler->prep_func = func;
   fd_handler->prep_data = (void *) data;
}

/**
 * Retrieves the file descriptor that the given handler is handling.
 * @param   fd_handler The given FD handler.
 * @return  The file descriptor the handler is watching.
 * @ingroup Ecore_FD_Handler_Group
 */
int
ecore_main_fd_handler_fd_get(Ecore_Fd_Handler *fd_handler)
{
   if (!ECORE_MAGIC_CHECK(fd_handler, ECORE_MAGIC_FD_HANDLER))
     {
	ECORE_MAGIC_FAIL(fd_handler, ECORE_MAGIC_FD_HANDLER, 
			 "ecore_main_fd_handler_fd_get");
	return -1;
     }
   return fd_handler->fd;
}

/**
 * Return if read, write or error, or a combination thereof, is active on the
 * file descriptor of the given FD handler.
 * @param   fd_handler The given FD handler.
 * @param   flags      The flags, @c ECORE_FD_READ, @c ECORE_FD_WRITE or
 *                     @c ECORE_FD_ERROR to query.
 * @return  @c 1 if any of the given flags are active. @c 0 otherwise.
 * @ingroup Ecore_FD_Handler_Group
 */
int
ecore_main_fd_handler_active_get(Ecore_Fd_Handler *fd_handler, Ecore_Fd_Handler_Flags flags)
{
   int ret;
   
   if (!ECORE_MAGIC_CHECK(fd_handler, ECORE_MAGIC_FD_HANDLER))
     {
	ECORE_MAGIC_FAIL(fd_handler, ECORE_MAGIC_FD_HANDLER, 
			 "ecore_main_fd_handler_active_get");
	return 0;
     }
   ret = 0;
   if ((flags & ECORE_FD_READ) && (fd_handler->read_active)) ret = 1;
   if ((flags & ECORE_FD_WRITE) && (fd_handler->write_active)) ret = 1;
   if ((flags & ECORE_FD_ERROR) && (fd_handler->error_active)) ret = 1;
   return ret;
}

/**
 * Set what active streams the given FD handler should be monitoring.
 * @param   fd_handler The given FD handler.
 * @param   flags      The flags to be watching.
 * @ingroup Ecore_FD_Handler_Group
 */
void
ecore_main_fd_handler_active_set(Ecore_Fd_Handler *fd_handler, Ecore_Fd_Handler_Flags flags)
{
   if (!ECORE_MAGIC_CHECK(fd_handler, ECORE_MAGIC_FD_HANDLER))
     {
	ECORE_MAGIC_FAIL(fd_handler, ECORE_MAGIC_FD_HANDLER,
			 "ecore_main_fd_handler_active_set");
	return;
     }
   fd_handler->flags = flags;
}

void
_ecore_main_shutdown(void)
{
   if (in_main_loop)
     {
	fprintf(stderr,
		"\n"
		"*** ECORE WARINING: Calling ecore_shutdown() while still in the main loop.\n"
		"***                 Program may crash or behave strangely now.\n");
	return;
     }
   while (fd_handlers)
     {
	Ecore_Fd_Handler *fdh;
	
	fdh = fd_handlers;
	fd_handlers = _ecore_list2_remove(fd_handlers, fdh);
	ECORE_MAGIC_SET(fdh, ECORE_MAGIC_NONE);
	free(fdh);
     }
   fd_handlers_delete_me = 0;
}

static int
_ecore_main_select(double timeout)
{
   struct timeval tv, *t;
   fd_set         rfds, wfds, exfds;
   int            max_fd;
   int            ret;
   Ecore_List2    *l;

   t = NULL;
   if (timeout > 0.0)
     {
	int sec, usec;

#ifdef FIX_HZ
	timeout += (0.5 / HZ);
	sec = (int)timeout;
	usec = (int)((timeout - (double)sec) * 1000000);
#else	
	sec = (int)timeout;
	usec = (int)((timeout - (double)sec) * 1000000);
#endif	
	tv.tv_sec = sec;
	tv.tv_usec = usec;
	t = &tv;
     }
   else if (timeout == 0.0)
     {
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	t = &tv;
     }
   max_fd = 0;
   FD_ZERO(&rfds);
   FD_ZERO(&wfds);
   FD_ZERO(&exfds);

   /* call the prepare callback for all handlers */
   for (l = (Ecore_List2 *)fd_handlers; l; l = l->next)
     {
	Ecore_Fd_Handler *fdh;
	
	fdh = (Ecore_Fd_Handler *)l;

	if (!fdh->delete_me && fdh->prep_func)
		fdh->prep_func (fdh->prep_data, fdh);
     }
   for (l = (Ecore_List2 *)fd_handlers; l; l = l->next)
     {
	Ecore_Fd_Handler *fdh;
	
	fdh = (Ecore_Fd_Handler *)l;
	if (fdh->flags & ECORE_FD_READ)
	  {
	     FD_SET(fdh->fd, &rfds);
	     if (fdh->fd > max_fd) max_fd = fdh->fd;
	  }
	if (fdh->flags & ECORE_FD_WRITE)
	  {
	     FD_SET(fdh->fd, &wfds);
	     if (fdh->fd > max_fd) max_fd = fdh->fd;
	  }
	if (fdh->flags & ECORE_FD_ERROR)
	  {
	     FD_SET(fdh->fd, &exfds);
	     if (fdh->fd > max_fd) max_fd = fdh->fd;
	  }
     }
#ifndef WIN32
   if (_ecore_signal_count_get()) return -1;
#endif
   ret = select(max_fd + 1, &rfds, &wfds, &exfds, t);
   if (ret < 0)
     {
	if (errno == EINTR) return -1;
     }
   if (ret > 0)
     {
	for (l = (Ecore_List2 *)fd_handlers; l; l = l->next)
	  {
	     Ecore_Fd_Handler *fdh;
	     
	     fdh = (Ecore_Fd_Handler *)l;
	     if (!fdh->delete_me)
	       {
		  if (FD_ISSET(fdh->fd, &rfds))
		    fdh->read_active = 1;
		  if (FD_ISSET(fdh->fd, &wfds))
		    fdh->write_active = 1;
		  if (FD_ISSET(fdh->fd, &exfds))
		    fdh->error_active = 1;
	       }
	  }
	_ecore_main_fd_handlers_cleanup();
 	return 1;
     }
   return 0;
}

static void
_ecore_main_fd_handlers_cleanup(void)
{
   Ecore_List2 *l;
   
   if (!fd_handlers_delete_me) return;
   for (l = (Ecore_List2 *)fd_handlers; l;)
     {
	Ecore_Fd_Handler *fdh;
	
	fdh = (Ecore_Fd_Handler *)l;
	l = l->next;
	if (fdh->delete_me)
	  {
	     fd_handlers = _ecore_list2_remove(fd_handlers, fdh);
	     ECORE_MAGIC_SET(fdh, ECORE_MAGIC_NONE);
	     free(fdh);
	  }
     }
   fd_handlers_delete_me = 0;
}

static void
_ecore_main_fd_handlers_call(void)
{
   Ecore_List2    *l;
   
   for (l = (Ecore_List2 *)fd_handlers; l; l = l->next)
     {
	Ecore_Fd_Handler *fdh;
	
	fdh = (Ecore_Fd_Handler *)l;
	if (!fdh->delete_me)
	  {
	     if ((fdh->read_active) || 
		 (fdh->write_active) || 
		 (fdh->error_active))
	       {
		  if (!fdh->func(fdh->data, fdh))
		    {
		       fdh->delete_me = 1;
		       fd_handlers_delete_me = 1;
		    }
		  fdh->read_active = 0;
		  fdh->write_active = 0;
		  fdh->error_active = 0;
	       }
	  }
     }
}

static int
_ecore_main_fd_handlers_buf_call(void)
{
   Ecore_List2    *l;
   int ret;
   
   ret = 0;
   for (l = (Ecore_List2 *)fd_handlers; l; l = l->next)
     {
	Ecore_Fd_Handler *fdh;
	
	fdh = (Ecore_Fd_Handler *)l;
	if (!fdh->delete_me)
	  {
	     if (fdh->buf_func)
	       {
		  if (fdh->buf_func(fdh->buf_data, fdh))
		    {
		       ret |= fdh->func(fdh->data, fdh);
		       fdh->read_active = 1;
		    }
	       }
	  }
     }
   return ret;
}

static void
_ecore_main_loop_iterate_internal(int once_only)
{
   double next_time;
   int    have_event = 0;
   int    have_signal;
   
   in_main_loop++;
   /* expire any timers */
     {
	double now;
	
	now = ecore_time_get();
	while (_ecore_timer_call(now));
	_ecore_timer_cleanup();
     }
   /* any timers re-added as a result of these are allowed to go */
   _ecore_timer_enable_new();
#ifndef WIN32
   /* process signals into events .... */
   while (_ecore_signal_count_get()) _ecore_signal_call();
#endif
   if (_ecore_event_exist())
     {
	int ret;
	
	have_event = 1;
	have_signal = 1;
	ret = _ecore_main_select(0);
	goto process_events;
     }
   /* call idle enterers ... */
   if (!once_only)
     _ecore_idle_enterer_call();
   else
     {
        int ret;

	have_event = have_signal = 0;
	ret = _ecore_main_select(0);

	if (ret > 0) have_event = 1;
#ifndef WIN32
	if (_ecore_signal_count_get() > 0) have_signal = 1;
#endif

	if (have_signal || have_event)
	  goto process_events;
     }

   /* if these calls caused any biuffered events to appear - deal with them */
   while (_ecore_main_fd_handlers_buf_call());
   /* if ther are any - jump to processing them */
   if (_ecore_event_exist())
     {
	int ret;
	
	have_event = 1;
	have_signal = 1;
	ret = _ecore_main_select(0);
	goto process_events;
     }
   if (once_only)
     {
	_ecore_idle_enterer_call();
	in_main_loop--;
	return;
     }
   
#ifndef WIN32
   if (_ecore_fps_debug)
     {
	t2 = ecore_time_get();
	if ((t1 > 0.0) && (t2 > 0.0))
	  _ecore_fps_debug_runtime_add(t2 - t1);
     }
#endif
   start_loop:
   if (do_quit) 
     {
	in_main_loop--;
	return;
     }
   if (!_ecore_event_exist())
     {
	/* init flags */
	have_event = have_signal = 0;
	next_time = _ecore_timer_next_get();	
	/* no timers */
	if (next_time < 0)
	  {
	     /* no idlers */
	     if (!_ecore_idler_exist())
	       {
		  int ret;
		  
		  ret = _ecore_main_select(-1);
		  if (ret > 0) have_event = 1;
#ifndef WIN32
		  if (_ecore_signal_count_get() > 0) have_signal = 1;
#endif
	       }
	     /* idlers */
	     else
	       {
		  for (;;)
		    {
		       int ret;
		       
		       if (!_ecore_idler_call()) goto start_loop;
		       if (_ecore_event_exist()) break;
		       ret = _ecore_main_select(0);
		       if (ret > 0) have_event = 1;
#ifndef WIN32
		       if (_ecore_signal_count_get() > 0) have_signal = 1;
#endif
		       if (have_event || have_signal) break;
		       next_time = _ecore_timer_next_get();
		       if (next_time >= 0) goto start_loop;
		       if (do_quit) break;
		    }
	       }
	  }
	/* timers */
	else
	  {
	     /* no idlers */
	     if (!_ecore_idler_exist())
	       {
		  int ret;
		  
		  ret = _ecore_main_select(next_time);
		  if (ret > 0) have_event = 1;
#ifndef WIN32
		  if (_ecore_signal_count_get() > 0) have_signal = 1;
#endif
	       }
	     /* idlers */
	     else
	       {
		  for (;;)
		    {
		       double cur_time, t;
		       int ret;
		       
		       if (!_ecore_idler_call()) goto start_loop;
		       if (_ecore_event_exist()) break;
		       ret = _ecore_main_select(0);
		       if (ret > 0) have_event = 1;
#ifndef WIN32
		       if (_ecore_signal_count_get() > 0) have_signal = 1;
#endif
		       if ((have_event) || (have_signal)) break;
		       cur_time = ecore_time_get();
		       t = ecore_time_get() - cur_time;
		       if (t >= next_time) break;
		       next_time = _ecore_timer_next_get();
		       if (next_time < 0) goto start_loop;
		       if (do_quit) break;
		    }
	       }
	  }
     }
#ifndef WIN32
   if (_ecore_fps_debug)
     {
	t1 = ecore_time_get();
     }
#endif
   /* we came out of our "wait state" so idle has exited */
   if (!once_only)
     _ecore_idle_exiter_call();
   /* call the fd handler per fd that became alive... */
   /* this should read or write any data to the monitored fd and then */
   /* post events onto the ecore event pipe if necessary */
   process_events:
   if (have_event) _ecore_main_fd_handlers_call();
   do
     {
#ifndef WIN32
	/* process signals into events .... */
	while (_ecore_signal_count_get()) _ecore_signal_call();
#endif
	
	/* handle events ... */
	_ecore_event_call();	
	_ecore_main_fd_handlers_cleanup();
     }
   while (_ecore_main_fd_handlers_buf_call());
   if (once_only) _ecore_idle_enterer_call();
   in_main_loop--;
}

#include "ecore_private.h"
#include "Ecore.h"

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

static int  _ecore_main_select(double timeout);
static void _ecore_main_fd_handlers_cleanup(void);
static void _ecore_main_fd_handlers_call(void);
static int  _ecore_main_fd_handlers_buf_call(void);
static void _ecore_main_loop_iterate_internal(int once_only);

static int               in_main_loop = 0;
static int               do_quit = 0;
static Ecore_Fd_Handler *fd_handlers = NULL;
static int               fd_handlers_delete_me = 0;

/**
 * Run 1 iteration of the main loop and process everything on the queue.
 * 
 * This function Processes 1 iteration of the main loop, handling anything on
 * the queue. See ecore_main_loop_begin() for more information.
 */
void
ecore_main_loop_iterate(void)
{
   _ecore_main_loop_iterate_internal(1);
}

/**
 * Run the application main loop.
 * 
 * This function does not return until ecore_main_loop_quit() is called. It
 * will keep looping internally and call all callbacks set up to handle timers,
 * idle state and events Ecore recieves from X, fd's, IPC, signals etc. and
 * anything else that has registered a handler with ecore itself.
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
 * Quit the main loop after it is done processing.
 * 
 * This function will flag a quit of the main loop once the current loop has
 * finished processing all events. It will not quit instantly, so expect more
 * callbacks to be called after this command has been issued.
 */
void
ecore_main_loop_quit(void)
{
   do_quit = 1;
}

/**
 * Add a handler for read/write notification of a file descriptor.
 * @param fd The file descriptor to watch
 * @param flags To watch it for read and/or write ability
 * @param func The function to call when the file descriptor becomes active
 * @param data The data to pass to the @p func call
 * @param buf_func The function to call to check if any data has been buffered and already read from the fd.
 * @param buf_data The data to pass to the @p buf_func call
 * @return A fd handler handle
 * 
 * This adds a fd handler, calling @p func whenever the fd is active for
 * read or write (or both) depending what flags were set as @p flags. On
 * failure NULL is returned. The @p func call will the triggered during
 * execution of ecore_main_loop_begin() when a file descriptor (fd) is
 * available for reading or writing (or both). The #p buf_func call is called
 * during event loop handling to check if data that has already been read from
 * the file descriptor is in a buffer and is available to read. This function
 * is optional and can be NULL. If it is called it will be passed @p buf_data
 * as the data parameter.
 * 
 * Example:
 * 
 * @code
 * int fd;
 * Ecore_Fd_Handler *fdh;
 * 
 * fd = open("/tmp/fifo", O_RDONLY);
 * fdh = ecore_main_fd_handler_add(fd, ECORE_FD_READ, func_read_fifo, NULL);
 * @endcode
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
   fd_handlers = _ecore_list_append(fd_handlers, fdh);
   return fdh;
}

/**
 * Delete a handler for file descriptors.
 * 
 * @param fd_handler The handler to delete
 * @return The data pointer set for the fd_handler
 * 
 * This deletes a file descriptore (fd) handler and on success returns the data
 * pointer that was being passed to it, set by ecore_main_fd_handler_add().
 * NUll is returned on failure.
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

/**
 * Return the file descriptor that the handler is handling
 * @param fd_handler The fd handler to query
 * @return The fd the handler is watching
 * 
 * This returns the fd the @p fd_handler is monitoring.
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
 * Return if read or write is active on the fd
 * @param fd_handler The handler to query
 * @param flags The flags to query
 * @return The active state of read or write on the fd, or both
 * 
 * Return if the fd the handler is watching is active for read, write or both.
 * The @p flags parameter determines what youw ant to query, and 1 is returned
 * if the hanldere reports an active state for any of the values in
 * @p flags since this is the same bitmask the handler was set up with. 0 is
 * returned if the fd is not active for the @p flags passed in.
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
 * Set what active stream the fd should be monitoring
 * @param fd_handler The handler to modify
 * @param flags The flags to be watching
 * 
 * This function changes if an fd shoudl be watched for read, write or both.
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
	fd_handlers = _ecore_list_remove(fd_handlers, fdh);
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
   Ecore_List    *l;
   
   t = NULL;
   if (timeout > 0.0)
     {
	int sec, usec;
	
	sec = (int)timeout;
	usec = (int)((timeout - (double)sec) * 1000000);
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
   for (l = (Ecore_List *)fd_handlers; l; l = l->next)
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
   if (_ecore_signal_count_get()) return -1;
   ret = select(max_fd + 1, &rfds, &wfds, &exfds, t);
   if (ret < 0)
     {
	if (errno == EINTR) return -1;
     }
   if (ret > 0)
     {
	for (l = (Ecore_List *)fd_handlers; l; l = l->next)
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
   Ecore_List *l;
   
   if (!fd_handlers_delete_me) return;
   for (l = (Ecore_List *)fd_handlers; l;)
     {
	Ecore_Fd_Handler *fdh;
	
	fdh = (Ecore_Fd_Handler *)l;
	l = l->next;
	if (fdh->delete_me)
	  {
	     fd_handlers = _ecore_list_remove(fd_handlers, fdh);
	     ECORE_MAGIC_SET(fdh, ECORE_MAGIC_NONE);
	     free(fdh);
	  }
     }
   fd_handlers_delete_me = 0;
}

static void
_ecore_main_fd_handlers_call(void)
{
   Ecore_List    *l;
   
   for (l = (Ecore_List *)fd_handlers; l; l = l->next)
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
   Ecore_List    *l;
   int ret;
   
   ret = 0;
   for (l = (Ecore_List *)fd_handlers; l; l = l->next)
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
   int    have_event;
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
   /* process signals into events .... */
   while (_ecore_signal_count_get()) _ecore_signal_call();
   if (_ecore_event_exist())
     {
	have_event = 1;
	have_signal = 1;
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
	if (_ecore_signal_count_get() > 0) have_signal = 1;

	if (have_signal || have_event)
	  goto process_events;
     }

   /* if these calls caused any biuffered events to appear - deal with them */
   while (_ecore_main_fd_handlers_buf_call());
   /* if ther are any - jump to processing them */
   if (_ecore_event_exist())
     {
	have_event = 1;
	have_signal = 1;
	goto process_events;
     }
   if (once_only)
     {
	_ecore_idle_enterer_call();
	in_main_loop--;
	return;
     }
   
   start_loop:
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
	     if (_ecore_signal_count_get() > 0) have_signal = 1;
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
		  if (_ecore_signal_count_get() > 0) have_signal = 1;
		  if (have_event || have_signal) break;
		  next_time = _ecore_timer_next_get();
		  if (next_time >= 0) goto start_loop;
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
	     if (_ecore_signal_count_get() > 0) have_signal = 1;
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
		  if (_ecore_signal_count_get() > 0) have_signal = 1;
		  if ((have_event) || (have_signal)) break;
		  cur_time = ecore_time_get();
		  t = ecore_time_get() - cur_time;
		  if (t >= next_time) break;
		  next_time = _ecore_timer_next_get();
		  if (next_time < 0) goto start_loop;
	       }
	  }
     }
   /* call the fd handler per fd that became alive... */
   /* this should read or write any data to the monitored fd and then */
   /* post events onto the ecore event pipe if necessary */
   process_events:
   if (have_event) _ecore_main_fd_handlers_call();
   do
     {
	/* process signals into events .... */
	while (_ecore_signal_count_get()) _ecore_signal_call();
	
	/* handle events ... */
	_ecore_event_call();	
	_ecore_main_fd_handlers_cleanup();
     }
   while (_ecore_main_fd_handlers_buf_call());
   if (once_only) _ecore_idle_enterer_call();
   in_main_loop--;
}

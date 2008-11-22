
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif

#ifdef HAVE_LANGINFO_H
# include <langinfo.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"

#ifdef HAVE_EVIL
# include <Evil.h>
#endif
#include <Eina.h>

static const char *_ecore_magic_string_get(Ecore_Magic m);
static int _ecore_init_count = 0;

int _ecore_fps_debug = 0;

/** OpenBSD does not define CODESET
 * FIXME ??
 */

#ifndef CODESET
# define CODESET "INVALID"
#endif

/**
 * Set up connections, signal handlers, sockets etc.
 * @return 1 or greater on success, 0 otherwise
 *
 * This function sets up all singal handlers and the basic event loop. If it
 * succeeds, 1 will be returned, otherwise 0 will be returned.
 *
 * @code
 * #include <Ecore.h>
 *
 * int main(int argc, char **argv)
 * {
 *   if (!ecore_init())
 *   {
 *     printf("ERROR: Cannot init Ecore!\n");
 *     return -1;
 *   }
 *   ecore_main_loop_begin();
 *   ecore_shutdown();
 * }
 * @endcode
 */
EAPI int
ecore_init(void)
{
   if (++_ecore_init_count == 1)
     {
#ifdef HAVE_LOCALE_H
	setlocale(LC_CTYPE, "");
#endif
	/*
	if (strcmp(nl_langinfo(CODESET), "UTF-8"))
	  {
	     printf("WARNING: not a utf8 locale!\n");
	  }
	 */
#ifdef HAVE_EVIL
        evil_init();
#endif
	eina_init();
	if (getenv("ECORE_FPS_DEBUG")) _ecore_fps_debug = 1;
	if (_ecore_fps_debug) _ecore_fps_debug_init();
	_ecore_signal_init();
	_ecore_exe_init();
	_ecore_loop_time = ecore_time_get();
     }

   return _ecore_init_count;
}

/**
 * Shut down connections, signal handlers sockets etc.
 *
 * This function shuts down all things set up in ecore_init() and cleans up all
 * event queues, handlers, filters, timers, idlers, idle enterers/exiters
 * etc. set up after ecore_init() was called.
 *
 * Do not call this function from any callback that may be called from the main
 * loop, as the main loop will then fall over and not function properly.
 */
EAPI int
ecore_shutdown(void)
{
   if (--_ecore_init_count)
      return _ecore_init_count;

   if (_ecore_fps_debug) _ecore_fps_debug_shutdown();
   _ecore_poller_shutdown();
   _ecore_animator_shutdown();
   _ecore_exe_shutdown();
   _ecore_idle_enterer_shutdown();
   _ecore_idle_exiter_shutdown();
   _ecore_idler_shutdown();
   _ecore_timer_shutdown();
   _ecore_event_shutdown();
   _ecore_main_shutdown();
   _ecore_signal_shutdown();
   eina_shutdown();
#ifdef HAVE_EVIL
   evil_shutdown();
#endif

   return _ecore_init_count;
}

EAPI void
_ecore_magic_fail(const void *d, Ecore_Magic m, Ecore_Magic req_m, const char *fname)
{
   fprintf(stderr,
	   "\n"
	   "*** ECORE ERROR: Ecore Magic Check Failed!!!\n"
	   "*** IN FUNCTION: %s()\n", fname);
   if (!d)
     fprintf(stderr, "  Input handle pointer is NULL!\n");
   else if (m == ECORE_MAGIC_NONE)
     fprintf(stderr, "  Input handle has already been freed!\n");
   else if (m != req_m)
     fprintf(stderr, "  Input handle is wrong type\n"
	             "    Expected: %08x - %s\n"
	             "    Supplied: %08x - %s\n",
	     (unsigned int)req_m, _ecore_magic_string_get(req_m),
	     (unsigned int)m, _ecore_magic_string_get(m));
   fprintf(stderr,
	   "*** NAUGHTY PROGRAMMER!!!\n"
	   "*** SPANK SPANK SPANK!!!\n"
	   "*** Now go fix your code. Tut tut tut!\n"
	   "\n");
   if (getenv("ECORE_ERROR_ABORT")) abort();
}

static const char *
_ecore_magic_string_get(Ecore_Magic m)
{
   switch (m)
     {
      case ECORE_MAGIC_NONE:
	return "None (Freed Object)";
	break;
      case ECORE_MAGIC_EXE:
	return "Ecore_Exe (Executable)";
	break;
      case ECORE_MAGIC_TIMER:
	return "Ecore_Timer (Timer)";
	break;
      case ECORE_MAGIC_IDLER:
	return "Ecore_Idler (Idler)";
	break;
      case ECORE_MAGIC_IDLE_ENTERER:
	return "Ecore_Idle_Enterer (Idler Enterer)";
	break;
      case ECORE_MAGIC_IDLE_EXITER:
	return "Ecore_Idle_Exiter (Idler Exiter)";
	break;
      case ECORE_MAGIC_FD_HANDLER:
	return "Ecore_Fd_Handler (Fd Handler)";
	break;
      case ECORE_MAGIC_EVENT_HANDLER:
	return "Ecore_Event_Handler (Event Handler)";
	break;
      case ECORE_MAGIC_EVENT:
	return "Ecore_Event (Event)";
	break;
      default:
	return "<UNKNOWN>";
     };
   return "<UNKNOWN>";
}

/* fps debug calls - for debugging how much time your app actually spends */
/* "running" (and the inverse being time spent running)... this does not */
/* account for other apps and multitasking... */

static int _ecore_fps_debug_init_count = 0;
static int _ecore_fps_debug_fd = -1;
unsigned int *_ecore_fps_runtime_mmap = NULL;

void
_ecore_fps_debug_init(void)
{
   char  buf[4096];
   char *tmp;
   int   pid;

   _ecore_fps_debug_init_count++;
   if (_ecore_fps_debug_init_count > 1) return;

#ifndef HAVE_EVIL
   tmp = "/tmp";
#else
   tmp = (char *)evil_tmpdir_get ();
#endif /* HAVE_EVIL */
   pid = (int)getpid();
   snprintf(buf, sizeof(buf), "%s/.ecore_fps_debug-%i", tmp, pid);
   _ecore_fps_debug_fd = open(buf, O_CREAT | O_TRUNC | O_RDWR, 0644);
   if (_ecore_fps_debug_fd < 0)
     {
	unlink(buf);
	_ecore_fps_debug_fd = open(buf, O_CREAT | O_TRUNC | O_RDWR, 0644);
     }
   if (_ecore_fps_debug_fd >= 0)
     {
	unsigned int zero = 0;

	write(_ecore_fps_debug_fd, &zero, sizeof(unsigned int));
	_ecore_fps_runtime_mmap = mmap(NULL, sizeof(unsigned int),
				       PROT_READ | PROT_WRITE,
				       MAP_SHARED,
				       _ecore_fps_debug_fd, 0);
	if (_ecore_fps_runtime_mmap == MAP_FAILED)
	  _ecore_fps_runtime_mmap = NULL;
     }
}

void
_ecore_fps_debug_shutdown(void)
{
   _ecore_fps_debug_init_count--;
   if (_ecore_fps_debug_init_count > 0) return;
   if (_ecore_fps_debug_fd >= 0)
     {
	char buf[4096];
        char *tmp;
        int   pid;

#ifndef HAVE_EVIL
   tmp = "/tmp";
#else
   tmp = (char *)evil_tmpdir_get ();
#endif /* HAVE_EVIL */
   pid = (int)getpid();
	snprintf(buf, sizeof(buf), "%s/.ecore_fps_debug-%i", tmp, pid);
	unlink(buf);
	if (_ecore_fps_runtime_mmap)
	  {
	     munmap(_ecore_fps_runtime_mmap, sizeof(int));
	     _ecore_fps_runtime_mmap = NULL;
	  }
	close(_ecore_fps_debug_fd);
	_ecore_fps_debug_fd = -1;
     }
}

void
_ecore_fps_debug_runtime_add(double t)
{
   if ((_ecore_fps_debug_fd >= 0) &&
       (_ecore_fps_runtime_mmap))
     {
	unsigned int tm;

	tm = (unsigned int)(t * 1000000.0);
	/* i know its not 100% theoretically guaranteed, but i'd say a write */
	/* of an int could be considered atomic for all practical purposes */
	/* oh and since this is cumulative, 1 second = 1,000,000 ticks, so */
	/* this can run for about 4294 seconds becore looping. if you are */
	/* doing performance testing in one run for over an hour... well */
	/* time to restart or handle a loop condition :) */
	*(_ecore_fps_runtime_mmap) += tm;
     }
}

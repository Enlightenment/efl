#include "ecore_private.h"
#include "Ecore.h"

static const char *_ecore_magic_string_get(Ecore_Magic m);

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
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
int
ecore_init(void)
{
   _ecore_signal_init();
   return 1;
}

/**
 * Shut down connections, signal handlers sockets etc.
 * 
 * This function shuts down all things set up in ecore_init() and cleans up all
 * event queues, handlers, filters, timers, idlers, idle enterers etc. set up
 * after ecore_init() was called.
 * 
 * Do not call this function from any callback that may be called from the main
 * loop, as the main loop will then fall over and not function properly.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
*/
void
ecore_shutdown(void)
{
   _ecore_exe_shutdown();
   _ecore_idle_enterer_shutdown();
   _ecore_idler_shutdown();
   _ecore_timer_shutdown();
   _ecore_event_shutdown();
   _ecore_main_shutdown();
   _ecore_signal_shutdown();
}

void
_ecore_magic_fail(void *d, Ecore_Magic m, Ecore_Magic req_m, const char *fname)
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

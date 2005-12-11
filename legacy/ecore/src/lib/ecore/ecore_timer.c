#include "ecore_private.h"
#include "Ecore.h"

static void _ecore_timer_set(Ecore_Timer *timer, double at, double in, int (*func) (void *data), void *data);

static int          timers_added = 0;
static int          timers_delete_me = 0;
static Ecore_Timer *timers = NULL;
static double       last_check = 0.0;

/**
 * @defgroup Ecore_Time_Group Ecore Time Functions
 *
 * Functions that deal with time.  These functions include those that simply
 * retrieve it in a given format, and those that create events based on it.
 */

/**
 * Creates a timer to call the given function in the given period of time.
 * @param   in   The interval in seconds.
 * @param   func The given function.  If @p func returns 1, the timer is
 *               rescheduled for the next interval @p in.
 * @param   data Data to pass to @p func when it is called.
 * @return  A timer object on success.  @c NULL on failure.
 * @ingroup Ecore_Time_Group
 */
Ecore_Timer *
ecore_timer_add(double in, int (*func) (void *data), const void *data)
{
   double now;
   Ecore_Timer *timer;

   if (!func) return NULL;
   if (in < 0.0) in = 0.0;
   timer = calloc(1, sizeof(Ecore_Timer));
   if (!timer) return NULL;
   ECORE_MAGIC_SET(timer, ECORE_MAGIC_TIMER);
   now = ecore_time_get();
   _ecore_timer_set(timer, now + in, in, func, (void *)data);
   return timer;
}

/**
 * Delete the specified timer from the timer list.
 * @param   timer The timer to delete.
 * @return  The data pointer set for the timer when @ref ecore_timer_add was
 *          called.  @c NULL is returned if the function is unsuccessful.
 * @ingroup Ecore_Time_Group
 */
void *
ecore_timer_del(Ecore_Timer *timer)
{
   if (!ECORE_MAGIC_CHECK(timer, ECORE_MAGIC_TIMER))
     {
	ECORE_MAGIC_FAIL(timer, ECORE_MAGIC_TIMER,
			 "ecore_timer_del");
	return NULL;
     }
   if (timer->delete_me) return timer->data;
   timers_delete_me++;
   timer->delete_me = 1;
   return timer->data;
}

/**
 * Change the interval the timer ticks of. If set during
 * a timer call, this will affect the next interval.
 *
 * @param   timer The timer to change.
 * @param   in    The interval in seconds.
 * @ingroup Ecore_Time_Group
 */
void
ecore_timer_interval_set(Ecore_Timer *timer, double in)
{
   if (!ECORE_MAGIC_CHECK(timer, ECORE_MAGIC_TIMER))
     {
	ECORE_MAGIC_FAIL(timer, ECORE_MAGIC_TIMER,
			 "ecore_timer_interval_set");
	return;
     }
   timer->in = in;
}

void
_ecore_timer_shutdown(void)
{
   while (timers)
     {
	Ecore_Timer *timer;
	
	timer = timers;
	timers = _ecore_list2_remove(timers, timer);
	ECORE_MAGIC_SET(timer, ECORE_MAGIC_NONE);
	free(timer);
     }
}

void
_ecore_timer_cleanup(void)
{
   Ecore_List2 *l;

   if (!timers_delete_me) return;
   for (l = (Ecore_List2 *)timers; l;)
     {
	Ecore_Timer *timer;
	
	timer = (Ecore_Timer *)l;
	l = l->next;
	if (timer->delete_me)
	  {
	     timers = _ecore_list2_remove(timers, timer);
	     ECORE_MAGIC_SET(timer, ECORE_MAGIC_NONE);
	     free(timer);
	     timers_delete_me--;
	     if (timers_delete_me == 0) return;
	  }
     }
   timers_delete_me = 0;
}

void
_ecore_timer_enable_new(void)
{
   Ecore_List2 *l;   

   if (!timers_added) return;
   timers_added = 0;
   for (l = (Ecore_List2 *)timers; l; l = l->next)
     {
	Ecore_Timer *timer;
	
	timer = (Ecore_Timer *)l;
	timer->just_added = 0;
     }
}

double
_ecore_timer_next_get(void)
{
   double now;
   double in;
   
   if (!timers) return -1;
   now = ecore_time_get();
   in = timers->at - now;
   if (in < 0) in = 0;
   return in;
}  

int
_ecore_timer_call(double when)
{
   Ecore_List2 *l;   
   Ecore_Timer *timer;
   
   if (!timers) return 0;
   if (last_check > when)
     {
	/* User set time backwards */
	for (l = (Ecore_List2 *)timers; l; l = l->next)
	  {
	     timer = (Ecore_Timer *)l;
	     timer->at -= (last_check - when);
	  }
     }
   last_check = when;
   for (l = (Ecore_List2 *)timers; l; l = l->next)
     {
	timer = (Ecore_Timer *)l;
	if ((timer->at <= when) &&
	    (!timer->just_added) &&
	    (!timer->delete_me))
	  {
	     timers = _ecore_list2_remove(timers, timer);
	     _ecore_timer_call(when);
	     if ((!timer->delete_me) && (timer->func(timer->data)))
	       {
		  /* if the timer would have gone off more than 30 seconds ago,
		   * assume that the system hung and set the timer to go off
		   * timer->in from now.
		   */
		  if (!timer->delete_me)
		    {
		       if ((timer->at + timer->in) < (when - 30.0))
			 _ecore_timer_set(timer, when + timer->in, timer->in, timer->func, timer->data);
		       else
			 _ecore_timer_set(timer, timer->at + timer->in, timer->in, timer->func, timer->data);
		    }
		  else
		    free(timer);
	       }
	     else
	       free(timer);
	     return 1;
	  }
     }
   return 0;
}  

static void
_ecore_timer_set(Ecore_Timer *timer, double at, double in, int (*func) (void *data), void *data)
{
   Ecore_List2 *l;
   
   timers_added = 1;
   timer->at = at;
   timer->in = in;
   timer->func = func;
   timer->data = data;
   timer->just_added = 1;
   if (timers)
     {
	for (l = ((Ecore_List2 *)(timers))->last; l; l = l->prev)
	  {
	     Ecore_Timer *t2;
	     
	     t2 = (Ecore_Timer *)l;
	     if (timer->at > t2->at)
	       {
		  timers = _ecore_list2_append_relative(timers, timer, t2);
		  return;
	       }
	  }
     }
   timers = _ecore_list2_prepend(timers, timer);
}

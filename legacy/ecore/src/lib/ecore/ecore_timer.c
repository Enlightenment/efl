/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include "ecore_private.h"
#include "Ecore.h"

static void _ecore_timer_set(Ecore_Timer *timer, double at, double in, int (*func) (void *data), void *data);

static int          timers_added = 0;
static int          timers_delete_me = 0;
static Ecore_Timer *timers = NULL;
static Ecore_Timer *suspended = NULL;
static double       last_check = 0.0;
static double       precision = 10.0 / 1000000.0;

/**
 * @defgroup Ecore_Time_Group Ecore Time Functions
 *
 * Functions that deal with time.  These functions include those that simply
 * retrieve it in a given format, and those that create events based on it.
 */

/**
 * Retrieves the current precision used by timer infrastructure.
 *
 * @see ecore_timer_precision_set()
 */
EAPI double
ecore_timer_precision_get(void)
{
   return precision;
}

/**
 * Sets the precision to be used by timer infrastructure.
 *
 * When system calculates time to expire the next timer we'll be able
 * to delay the timer by the given amount so more timers will fit in
 * the same dispatch, waking up the system less often and thus being
 * able to save power.
 *
 * Be aware that kernel may delay delivery even further, these delays
 * are always possible due other tasks having higher priorities or
 * other scheduler policies.
 *
 * Example:
 *  We have 2 timers, one that expires in a 2.0s and another that
 *  expires in 2.1s, if precision is 0.1s, then the Ecore will request
 *  for the next expire to happen in 2.1s and not 2.0s and another one
 *  of 0.1 as it would before.
 *
 * @note Ecore is smart enough to see if there are timers in the
 * precision range, if it does not, in our example if no second timer
 * in (T + precision) existed, then it would use the minimum timeout.
 *
 * @param value allowed introduced timeout delay, in seconds.
 */
EAPI void
ecore_timer_precision_set(double value)
{
   if (value < 0.0)
     {
	fprintf(stderr, "ERROR: precision %f less than zero, ignored\n", value);
	return;
     }
   precision = value;
}

/**
 * Creates a timer to call the given function in the given period of time.
 * @param   in   The interval in seconds.
 * @param   func The given function.  If @p func returns 1, the timer is
 *               rescheduled for the next interval @p in.
 * @param   data Data to pass to @p func when it is called.
 * @return  A timer object on success.  @c NULL on failure.
 * @ingroup Ecore_Time_Group
 *
 * This function adds a timer and returns its handle on success and NULL on
 * failure. The function @p func will be called every @p in seconds. The
 * function will be passed the @p data pointer as its parameter.
 *
 * When the timer @p func is called, it must return a value of either 1
 * (or ECORE_CALLBACK_RENEW) or 0 (or ECORE_CALLBACK_CANCEL).
 * If it returns 1, it will be called again at the next tick, or if it returns
 * 0 it will be deleted automatically making any references/handles for it
 * invalid.
 */
EAPI Ecore_Timer *
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
 *
 * Note: @p timer must be a valid handle. If the timer function has already
 * returned 0, the handle is no longer valid (and does not need to be delete).
 */
EAPI void *
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
EAPI void
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

/**
 * Add some delay for the next occurence of a timer.
 * This doesn't affect the interval of a timer.
 *
 * @param   timer The timer to change.
 * @param   add   The dalay to add to the next iteration.
 * @ingroup Ecore_Time_Group
 */
EAPI void
ecore_timer_delay(Ecore_Timer *timer, double add)
{
   if (!ECORE_MAGIC_CHECK(timer, ECORE_MAGIC_TIMER))
     {
	ECORE_MAGIC_FAIL(timer, ECORE_MAGIC_TIMER,
			 "ecore_timer_delay");
	return;
     }

   if (timer->frozen)
     {
	timer->pending += add;
     }
   else
     {
	timers = _ecore_list2_remove(timers, timer);
	_ecore_timer_set(timer, timer->at + add, timer->in, timer->func, timer->data);
     }
}

/**
 * Get the pending time regarding a timer.
 *
 * @param	timer The timer to learn from.
 * @ingroup	Ecore_Time_Group
 */
EAPI double
ecore_timer_pending_get(Ecore_Timer *timer)
{
   double	now;

   if (!ECORE_MAGIC_CHECK(timer, ECORE_MAGIC_TIMER))
     {
	ECORE_MAGIC_FAIL(timer, ECORE_MAGIC_TIMER,
			 "ecore_timer_pending_get");
	return 0;
     }

   now = ecore_time_get();

   if (timer->frozen)
     return timer->pending;
   return timer->at - now;
}

/**
 *
 *
 */
EAPI void
ecore_timer_freeze(Ecore_Timer *timer)
{
   double now;

   if (!ECORE_MAGIC_CHECK(timer, ECORE_MAGIC_TIMER))
     {
	ECORE_MAGIC_FAIL(timer, ECORE_MAGIC_TIMER,
                         "ecore_timer_freeze");
        return ;
     }

   /* Timer already frozen */
   if (timer->frozen)
     return ;

   timers = _ecore_list2_remove(timers, timer);
   suspended = _ecore_list2_prepend(suspended, timer);

   now = ecore_time_get();

   timer->pending = timer->at - now;
   timer->at = 0.0;
   timer->frozen = 1;
}

EAPI void
ecore_timer_thaw(Ecore_Timer *timer)
{
   double now;

   if (!ECORE_MAGIC_CHECK(timer, ECORE_MAGIC_TIMER))
     {
	ECORE_MAGIC_FAIL(timer, ECORE_MAGIC_TIMER,
                         "ecore_timer_thaw");
        return ;
     }

   /* Timer not frozen */
   if (!timer->frozen)
     return ;

   suspended = _ecore_list2_remove(suspended, timer);
   now = ecore_time_get();

   _ecore_timer_set(timer, timer->pending + now, timer->in, timer->func, timer->data);
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

   while (suspended)
     {
        Ecore_Timer *timer;

        timer = suspended;
        suspended = _ecore_list2_remove(suspended, timer);
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
   for (l = (Ecore_List2 *)suspended; l;)
     {
	Ecore_Timer *timer;

	timer = (Ecore_Timer *)l;
	l = l->next;
	if (timer->delete_me)
	  {
	     suspended = _ecore_list2_remove(suspended, timer);
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

static inline Ecore_Timer *
_ecore_timer_first_get(void)
{
   Ecore_Timer *timer = (Ecore_Timer *)timers;

   while ((timer) && ((timer->delete_me) || (timer->just_added)))
     timer = (Ecore_Timer *)((Ecore_List2 *)timer)->next;

   return timer;
}

static inline Ecore_Timer *
_ecore_timer_after_get(Ecore_Timer *base)
{
   Ecore_Timer *timer = (Ecore_Timer *)((Ecore_List2 *)base)->next;
   double maxtime = base->at + precision;

   while ((timer) && ((timer->delete_me) || (timer->just_added)) && (timer->at <= maxtime))
     timer = (Ecore_Timer *)((Ecore_List2 *)timer)->next;

   if ((!timer) || (timer->at > maxtime))
     return NULL;

   return timer;
}

double
_ecore_timer_next_get(void)
{
   double now;
   double in;
   Ecore_Timer *first, *second;

   first = _ecore_timer_first_get();
   if (!first) return -1;

   second = _ecore_timer_after_get(first);
   if (second)
     first = second;

   now = ecore_loop_time_get();
   in = first->at - now;
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
	    (timer->just_added == 0) &&
	    (timer->delete_me == 0))
	  {
	     timers = _ecore_list2_remove(timers, timer);
	     _ecore_timer_call(when);
	     if ((!timer->delete_me) && (timer->func(timer->data)))
	       {
		  /* if the timer would have gone off more than 15 seconds ago,
		   * assume that the system hung and set the timer to go off
		   * timer->in from now. this handles system hangs, suspends
		   * and more, so ecore will only "replay" the timers while
		   * the system is suspended if it is suspended for less than
		   * 15 seconds (basically). this also handles if the process
		   * is stopped in a debugger or IO and other handling gets
		   * really slow within the main loop.
		   */
		  if (!timer->delete_me)
		    {
		       if ((timer->at + timer->in) < (when - 15.0))
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
   timer->frozen = 0;
   timer->pending = 0.0;
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

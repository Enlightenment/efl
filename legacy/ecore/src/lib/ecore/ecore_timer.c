#include "ecore_private.h"
#include "Ecore.h"

/* FIXME: write code for timers... */

static void _ecore_timer_set(Ecore_Timer *timer, double at, double in, int (*func) (void *data), void *data);

static int          timers_added = 0;
static int          timers_delete_me = 0;
static Ecore_Timer *timers = NULL;

/**
 * Add a timer to tick off in a specified time during main loop execution.
 * @param in The number of seconds in which to expire the timer
 * @param func The function to call when it expires
 * @param data The data to pass to the function
 * @return
 * 
 * This function adds a timer and returns its handle on success and NULL on
 * failure. The function @p func will be called in @p in seconds from the
 * time this function call was made. The function @p func is passed the
 * @p data pointer as its parameter.
 * 
 * When the timer @p func is called, it must return a value of either 1 or 0. 
 * If it returns 1, it will be re-scheduled to repeat in the same interval
 * after this timer was triggered (ie if this timer was triggered with an
 * @p in value of 1.0 then the next timer will be triggered at the time this
 * timer was called plus 1.0).
 * 
 * @code
 * #include <Ecore.h>
 * 
 * Ecore_Timer *timer1 = NULL, *timer2 = NULL, *timer3 = NULL;
 *   
 * int timer_tick(void *data)
 * {
 *   printf("Tick timer %3.2f\n", ecore_time_get());
 *   return 1;
 * }
 * 
 * int timer_repeat(void *data)
 * {
 *   printf("Repeat timer called at %3.2f seconds, data %p\n",
 *          ecore_time_get(), data);
 *   return 1;
 * }
 * 
 * int timer_once(void *data)
 * {
 *   printf("Once only timer called at %3.2f seconds, data %p\n", 
 *          ecore_time_get(), data);
 *   ecore_timer_del(timer2);
 *   return 0;
 * }
 * 
 * int main(int argc, char **argv)
 * {
 *   ecore_init();
 *   ecore_app_args_set(argc, argv);
 *   timer1 = ecore_timer_add(5.0, timer_once, NULL);
 *   timer2 = ecore_timer_add(0.5, timer_repeat, NULL);
 *   timer3 = ecore_timer_add(1.0, timer_tick, NULL);
 *   ecore_main_loop_begin();
 *   ecore_shutdown();
 * }
 * @endcode
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
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
 * @param timer
 * @return The data pointer set for the timer
 * 
 * Delete the specified @p timer from the set of timers that are executed
 * during main loop execution. This function returns the data parameter that
 * was being passed to the callback on success, or NULL on failure. After this
 * call returns the specified timer object @p timer is invalid and should not
 * be used again. It will not get called again after deletion.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
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

void
_ecore_timer_shutdown(void)
{
   while (timers)
     {
	Ecore_Timer *timer;
	
	timer = timers;
	timers = _ecore_list_remove(timers, timer);
	ECORE_MAGIC_SET(timer, ECORE_MAGIC_NONE);
	free(timer);
     }
}

void
_ecore_timer_cleanup(void)
{
   Ecore_List *l;

   if (!timers_delete_me) return;
   for (l = (Ecore_List *)timers; l;)
     {
	Ecore_Timer *timer;
	
	timer = (Ecore_Timer *)l;
	l = l->next;
	if (timer->delete_me)
	  {
	     timers = _ecore_list_remove(timers, timer);
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
   Ecore_List *l;   

   if (!timers_added) return;
   timers_added = 0;
   for (l = (Ecore_List *)timers; l; l = l->next)
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
   Ecore_List *l;   
   Ecore_Timer *timer;
   
   if (!timers) return 0;
   for (l = (Ecore_List *)timers; l; l = l->next)
     {
	timer = (Ecore_Timer *)l;
	if ((timer->at <= when) &&
	    (!timer->just_added) &&
	    (!timer->delete_me))
	  {
	     timers = _ecore_list_remove(timers, timer);
	     _ecore_timer_call(when);
	     if (timer->func(timer->data))
	       _ecore_timer_set(timer, timer->at + timer->in, timer->in, timer->func, timer->data);
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
   Ecore_List *l;
   
   timers_added = 1;
   timer->at = at;
   timer->in = in;
   timer->func = func;
   timer->data = data;
   timer->just_added = 1;
   if (timers)
     {
	for (l = ((Ecore_List *)(timers))->last; l; l = l->prev)
	  {
	     Ecore_Timer *t2;
	     
	     t2 = (Ecore_Timer *)l;
	     if (timer->at > t2->at)
	       {
		  timers = _ecore_list_append_relative(timers, timer, t2);
		  return;
	       }
	  }
     }
   timers = _ecore_list_prepend(timers, timer);
}

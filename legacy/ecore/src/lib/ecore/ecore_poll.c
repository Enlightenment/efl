#include "ecore_private.h"
#include "Ecore.h"

static Ecore_Timer    *timer = NULL;
static int             min_interval = -1;
static int             interval_incr = 0;
static int             at_tick = 0;
static int             just_added_poller = 0;
static int             poller_delete_count = 0;
static int             poller_walking = 0;
static double          poll_interval = 0.125;
static double          poll_cur_interval = 0.0;
static double          last_tick = 0.0;
static Ecore_Poller   *pollers[16] = 
{
   NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
   NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL
};
static unsigned short  poller_counters[16] = 
{
   0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0
};

static void _ecore_poller_next_tick_eval(void);
static int _ecore_poller_cb_timer(void *data);

static void
_ecore_poller_next_tick_eval(void)
{
   int i;
   double interval;
   
   min_interval = -1;
   for (i = 0; i < 15; i++)
     {
	if (pollers[i])
	  {
	     min_interval = i;
	     break;
	  }
     }
   if (min_interval < 0)
     {
	/* no pollers */
	if (timer)
	  {
	     ecore_timer_del(timer);
	     timer = NULL;
	  }
	return;
     }
   interval_incr = (1 << min_interval);
   interval = interval_incr * poll_interval;
   /* we are at the tick callback - so no need to do inter-tick adjustments
    * so we can fasttrack this as t -= last_tick in theory is 0.0 (though
    * in practice it will be a very very very small value. also the tick
    * callback will adjust the timer interval at the end anyway */
   if (at_tick)
     {
	if (!timer)
	  timer = ecore_timer_add(interval, _ecore_poller_cb_timer, NULL);
     }
   else
     {
	double t;
	
	if (!timer)
	  timer = ecore_timer_add(interval, _ecore_poller_cb_timer, NULL);
	else
	  {
	     t = ecore_time_get();
	     if (interval != poll_cur_interval)
	       {
		  t -= last_tick; /* time since we last ticked */
		  /* delete the timer and reset it to tick off in the new
		   * time interval. at the tick this will be adjusted */
		  ecore_timer_del(timer);
		  timer = ecore_timer_add(interval - t, 
					  _ecore_poller_cb_timer, NULL);
	       }
	  }
     }
   poll_cur_interval = interval;
}

static int
_ecore_poller_cb_timer(void *data __UNUSED__)
{
   int i;
   Ecore_List2 *l;
   Ecore_Poller *poller;
   int changes = 0;

   at_tick++;
   last_tick = ecore_time_get();
   /* we have 16 counters - each incriments every time the poller counter
    * "ticks". it incriments by the minimum interval (which can be 1, 2, 4,
    * 7, 16 etc. up to 32768) */
   for (i = 0; i < 15; i++)
     {
	poller_counters[i] += interval_incr;
	/* wrap back to 0 if we exceed out loop count for the counter */
	if (poller_counters[i] >= (1 << i)) poller_counters[i] = 0;
     }
   
   just_added_poller = 0;
   /* walk the pollers now */
   poller_walking++;
   for (i = 0; i < 15; i++)
     {
	/* if the counter is @ 0 - this means that counter "went off" this
	 * tick interval, so run all pollers hooked to that counter */
	if (poller_counters[i] == 0)
	  {
	     for (l = (Ecore_List2 *)pollers[i]; l; l = l->next)
	       {
		  poller = (Ecore_Poller *)l;
		  if (!poller->delete_me)
		    {
		       if (!poller->func(poller->data))
			 {
			    if (!poller->delete_me)
			      {
				 poller->delete_me = 1;
				 poller_delete_count++;
			      }
			 }
		    }
	       }
	  }
     }
   poller_walking--;
	
   /* handle deletes afterwards */
   if (poller_delete_count > 0)
     {
	/* FIXME: walk all pollers and remove deleted ones */
	for (i = 0; i < 15; i++)
	  {
             for (l = (Ecore_List2 *)pollers[i]; l;)
	       {
                  poller = (Ecore_Poller *)l;
		  l = l->next;
		  if (poller->delete_me)
		    {
		       pollers[poller->ibit] = _ecore_list2_remove(pollers[poller->ibit], poller);
		       free(poller);
		       poller_delete_count--;
		       changes++;
		       if (poller_delete_count <= 0) break;
		    }
	       }
	     if (poller_delete_count <= 0) break;
	  }
     }
   /* if we deleted or added any pollers, then we need to re-evaluate our
    * minimum poll interval */
   if ((changes > 0) || (just_added_poller > 0))
     _ecore_poller_next_tick_eval();

   just_added_poller = 0;
   poller_delete_count = 0;
   
   at_tick--;
   
   /* if the timer was deleted then there is no point returning 1 - ambiguous
    * if we do as it im plies "keep running me" but we have been deleted 
    * anyway */
   if (!timer) return 0;

   /* adjust interval */
   ecore_timer_interval_set(timer, poll_cur_interval);
   return 1;
}

/**
 * @defgroup Ecore_Poll_Group Ecore Poll Functions
 *
 * These functions are for the need to poll information, but provide a shared
 * abstracted API to pool such polling to minimise wakeup and ensure all the
 * polling happens in as few spots as possible areound a core poll interval.
 * For now only 1 core poller type is supprted: ECORE_POLLER_CORE
 */


/**
 * Sets the time between ticks (in seconds) for the given ticker clock.
 * @param   type The ticker type to adjust
 * @param   poll_time The time (in seconds) between ticks of the clock
 * @ingroup Ecore_Poller_Group
 * 
 * This will adjust the time between ticks of the given ticker type defined
 * by @p type to the time period defined by @p poll_time.
 */
EAPI void
ecore_poller_poll_interval_set(Ecore_Poller_Type type __UNUSED__, double poll_time)
{
   poll_interval = poll_time;
   _ecore_poller_next_tick_eval();
}

/**
 * Gets the time between ticks (in seconds) for the fiven ticker clock.
 * @param   type The ticker type to query
 * @return  The time in seconds between ticks of the ticker clock
 * @ingroup Ecore_Poller_Group
 * 
 * This will get the time between ticks of the specifider ticker clock.
 */
EAPI double
ecore_poller_poll_interval_get(Ecore_Poller_Type type __UNUSED__)
{
   return poll_interval;
}

/**
 * Creates a poller to call the given function at a particular tick interval.
 * @param   type The ticker type to attach the poller to
 * @param   interval The poll interval
 * @param   func The given function.  If @p func returns 1, the poller is
 *               rescheduled for the next tick interval.
 * @param   data Data to pass to @p func when it is called.
 * @return  A poller object on success.  @c NULL on failure.
 * @ingroup Ecore_Poller_Group
 *
 * This function adds a poller callback that is to be called regularly
 * along with all other poller callbacks so the pollers are synchronized with
 * all other pollers running off the same poller type and at the same tick
 * interval. This should be used for polling things when polling is desired
 * or required, and you do not have specific requirements on the exact times
 * to poll and want to avoid extra process wakeups for polling. This will
 * save power as the CPU has more of a chance to go into a low power state
 * the longer it is asleep for, so this should be used if you are at all
 * power conscious.
 * 
 * The @p type parameter defines the poller tick type (there is a virtual
 * clock ticking all the time - though ecore avoids making it tick when
 * there will not be any work to do at that tick point). There is only one
 * ticker at the moment - that is ECORE_POLLER_CORE. This is here for future
 * expansion if multiple clocks with different frequencies are really required.
 * The default time between ticks for the ECORE_POLLER_CORE ticker is 0.125
 * seconds.
 * 
 * The @p interval is the number of ticker ticks that will pass by in between
 * invocations of the @p func callback. This must be between 1 and 32768
 * inclusive, and must be a power of 2 (i.e. 1, 2, 4, 8, 16, ... 16384, 32768).
 * If it is 1, then the function will be called every tick. if it is 2, then it
 * will be called every 2nd tick, if it is 8, then every 8th tick etc. Exactly
 * which tick is undefined, as only the interval between calls can be defined.
 * Ecore will endeavour to keep pollers synchronised and to call as many in
 * 1 wakeup event as possible.
 * 
 * This function adds a poller and returns its handle on success and NULL on
 * failure. The function @p func will be called at tick intervals described
 * above. The function will be passed the @p data pointer as its parameter.
 * 
 * When the poller @p func is called, it must return a value of either 
 * 1 (or ECORE_CALLBACK_RENEW) or 0 (or ECORE_CALLBACK_CANCEL). If it 
 * returns 1, it will be called again at the next tick, or if it returns
 * 0 it will be deleted automatically making any references/handles for it
 * invalid.
 */
EAPI Ecore_Poller *
ecore_poller_add(Ecore_Poller_Type type __UNUSED__, int interval, int (*func) (void *data), const void *data)
{
   Ecore_Poller *poller;
   int ibit;
   
   if (!func) return NULL;
   if (interval < 1) interval = 1;
   
   poller = calloc(1, sizeof(Ecore_Poller));
   if (!poller) return NULL;
   ECORE_MAGIC_SET(poller, ECORE_MAGIC_POLLER);
   /* interval MUST be a power of 2, so enforce it */
   if (interval < 1) interval = 1;
   ibit = -1;
   while (interval != 0)
     {
	ibit++;
	interval >>= 1;
     }
   /* only allow up to 32768 - i.e. ibit == 15, so limit it */
   if (ibit > 15) ibit = 15;
   
   poller->ibit = ibit;
   poller->func = func;
   poller->data = (void *)data;
   pollers[poller->ibit] = _ecore_list2_prepend(pollers[poller->ibit], poller);
   if (poller_walking)
     just_added_poller++;
   else
     _ecore_poller_next_tick_eval();
   return poller;
}

/**
 * Delete the specified poller from the timer list.
 * @param   poller The poller to delete.
 * @return  The data pointer set for the timer when @ref ecore_poller_add was
 *          called.  @c NULL is returned if the function is unsuccessful.
 * @ingroup Ecore_Poller_Group
 *
 * Note: @p poller must be a valid handle. If the poller function has already
 * returned 0, the handle is no longer valid (and does not need to be delete).
 */
EAPI void *
ecore_poller_del(Ecore_Poller *poller)
{
   void *data;
   
   if (!ECORE_MAGIC_CHECK(poller, ECORE_MAGIC_POLLER))
     {
	ECORE_MAGIC_FAIL(poller, ECORE_MAGIC_POLLER,
			 "ecore_poller_del");
	return NULL;
     }
   /* we are walking the poller list - a bad idea to remove from it while
    * walking it, so just flag it as delete_me and come back to it after
    * the loop has finished */
   if (poller_walking > 0)
     {
	poller_delete_count++;
	poller->delete_me = 1;
	return poller->data;
     }
   /* not in loop so safe - delete immediately */
   data = poller->data;
   pollers[poller->ibit] = _ecore_list2_remove(pollers[poller->ibit], poller);
   free(poller);
   _ecore_poller_next_tick_eval();
   return data;
}

void
_ecore_poller_shutdown(void)
{
   int i;
   Ecore_List2 *l;
   Ecore_Poller *poller;
   
   for (i = 0; i < 15; i++)
     {
	for (l = (Ecore_List2 *)pollers[i]; l;)
	  {
	     poller = (Ecore_Poller *)l;
	     l = l->next;
	     pollers[poller->ibit] = _ecore_list2_remove(pollers[poller->ibit], poller);
	     free(poller);
	  }
     }
}

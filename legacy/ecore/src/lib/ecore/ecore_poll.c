#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include "Ecore.h"
#include "ecore_private.h"

struct _Ecore_Poller
{
   EINA_INLIST;
   ECORE_MAGIC;
   int           ibit;
   unsigned char delete_me : 1;
   Ecore_Task_Cb func;
   void         *data;
};
GENERIC_ALLOC_SIZE_DECLARE(Ecore_Poller);

static Ecore_Timer *timer = NULL;
static int min_interval = -1;
static int interval_incr = 0;
static int at_tick = 0;
static int just_added_poller = 0;
static int poller_delete_count = 0;
static int poller_walking = 0;
static double poll_interval = 0.125;
static double poll_cur_interval = 0.0;
static double last_tick = 0.0;
static Ecore_Poller *pollers[16] =
{
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
static unsigned short poller_counters[16] =
{
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0
};

static void      _ecore_poller_next_tick_eval(void);
static Eina_Bool _ecore_poller_cb_timer(void *data);

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

static Eina_Bool
_ecore_poller_cb_timer(void *data __UNUSED__)
{
   int i;
   Ecore_Poller *poller, *l;
   int changes = 0;

   at_tick++;
   last_tick = ecore_time_get();
   /* we have 16 counters - each increments every time the poller counter
    * "ticks". it increments by the minimum interval (which can be 1, 2, 4,
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
               EINA_INLIST_FOREACH(pollers[i], poller)
                 {
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
              for (l = pollers[i]; l; )
                {
                   poller = l;
                   l = (Ecore_Poller *)EINA_INLIST_GET(l)->next;
                   if (poller->delete_me)
                     {
                        pollers[i] = (Ecore_Poller *)eina_inlist_remove(EINA_INLIST_GET(pollers[i]), EINA_INLIST_GET(poller));
                        ecore_poller_mp_free(poller);
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
    * if we do as it implies keep running me" but we have been deleted
    * anyway */
   if (!timer) return ECORE_CALLBACK_CANCEL;

   /* adjust interval */
   ecore_timer_interval_set(timer, poll_cur_interval);
   return ECORE_CALLBACK_RENEW;
}

EAPI void
ecore_poller_poll_interval_set(Ecore_Poller_Type type __UNUSED__,
                               double            poll_time)
{
   EINA_MAIN_LOOP_CHECK_RETURN;

   if (poll_time < 0.0)
     {
        ERR("Poll time %f less than zero, ignored", poll_time);
        return;
     }

   poll_interval = poll_time;
   _ecore_poller_next_tick_eval();
}

EAPI double
ecore_poller_poll_interval_get(Ecore_Poller_Type type __UNUSED__)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0.0);
   return poll_interval;
}

EAPI Ecore_Poller *
ecore_poller_add(Ecore_Poller_Type type __UNUSED__,
                 int               interval,
                 Ecore_Task_Cb     func,
                 const void       *data)
{
   Ecore_Poller *poller;
   int ibit;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   if (!func) return NULL;
   if (interval < 1) interval = 1;

   poller = ecore_poller_calloc(1);
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
   pollers[poller->ibit] = (Ecore_Poller *)eina_inlist_prepend(EINA_INLIST_GET(pollers[poller->ibit]), EINA_INLIST_GET(poller));
   if (poller_walking)
     just_added_poller++;
   else
     _ecore_poller_next_tick_eval();
   return poller;
}

EAPI Eina_Bool
ecore_poller_poller_interval_set(Ecore_Poller *poller,
                                 int           interval)
{
   int ibit;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(EINA_FALSE);
   if (!ECORE_MAGIC_CHECK(poller, ECORE_MAGIC_POLLER))
     {
        ECORE_MAGIC_FAIL(poller, ECORE_MAGIC_POLLER,
                         "ecore_poller_poller_interval_set");
        return EINA_FALSE;
     }

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
   /* if interval specified is the same as interval set, return true without wasting time */
   if (poller->ibit == ibit)
     return EINA_TRUE;
   pollers[poller->ibit] = (Ecore_Poller *)eina_inlist_remove(EINA_INLIST_GET(pollers[poller->ibit]), EINA_INLIST_GET(poller));
   poller->ibit = ibit;
   pollers[poller->ibit] = (Ecore_Poller *)eina_inlist_prepend(EINA_INLIST_GET(pollers[poller->ibit]), EINA_INLIST_GET(poller));
   if (poller_walking)
     just_added_poller++;
   else
     _ecore_poller_next_tick_eval();
   return EINA_TRUE;
}

EAPI int
ecore_poller_poller_interval_get(Ecore_Poller *poller)
{
   int ibit, interval = 1;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0);
   if (!ECORE_MAGIC_CHECK(poller, ECORE_MAGIC_POLLER))
     {
        ECORE_MAGIC_FAIL(poller, ECORE_MAGIC_POLLER,
                         "ecore_poller_poller_interval_get");
        return 0;
     }

   ibit = poller->ibit;
   while (ibit != 0)
     {
        ibit--;
        interval <<= 1;
     }
   return interval;
}

EAPI void *
ecore_poller_del(Ecore_Poller *poller)
{
   void *data;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
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
   pollers[poller->ibit] = (Ecore_Poller *)eina_inlist_remove(EINA_INLIST_GET(pollers[poller->ibit]), EINA_INLIST_GET(poller));
   ecore_poller_mp_free(poller);
   _ecore_poller_next_tick_eval();
   return data;
}

/**
 * @}
 */

void
_ecore_poller_shutdown(void)
{
   int i;
   Ecore_Poller *poller;

   for (i = 0; i < 15; i++)
     {
        while ((poller = pollers[i]))
          {
             pollers[i] = (Ecore_Poller *)eina_inlist_remove(EINA_INLIST_GET(pollers[i]), EINA_INLIST_GET(pollers[i]));
             ecore_poller_mp_free(poller);
          }
     }
}


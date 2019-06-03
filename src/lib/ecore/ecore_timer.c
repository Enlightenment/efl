#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include <Eo.h>

#include "Ecore.h"
#include "ecore_private.h"

#define MY_CLASS EFL_LOOP_TIMER_CLASS
#define MY_CLASS_NAME "Efl_Loop_Timer"

#define ECORE_TIMER_CHECK(obj) if (!efl_isa((obj), MY_CLASS)) return

struct _Ecore_Timer_Legacy
{
   Ecore_Task_Cb func;

   const void *data;

   Eina_Bool inside_call : 1;
   Eina_Bool delete_me   : 1;
};

struct _Efl_Loop_Timer_Data
{
   EINA_INLIST;

   Eo            *object;
   Eo            *loop;
   Efl_Loop_Data *loop_data;

   double     in;
   double     at;
   double     pending;

   int        listening;

   Eina_Bool  just_added  : 1;
   Eina_Bool  frozen      : 1;
   Eina_Bool  initialized : 1;
   Eina_Bool  noparent    : 1;
   Eina_Bool  constructed : 1;
   Eina_Bool  finalized   : 1;
};

typedef struct _Ecore_Timer_Legacy Ecore_Timer_Legacy;

static void _efl_loop_timer_util_delay(Efl_Loop_Timer_Data *timer, double add);
static void _efl_loop_timer_util_instanciate(Efl_Loop_Data *loop, Efl_Loop_Timer_Data *timer);
static void _efl_loop_timer_set(Efl_Loop_Timer_Data *timer, double at, double in);

static double precision = 10.0 / 1000000.0;

EAPI double
ecore_timer_precision_get(void)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0.0);
   return precision;
}

EAPI void
ecore_timer_precision_set(double value)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   if (value < 0.0) value = 0.0;
   precision = value;
}

static void
_check_timer_event_catcher_add(void *data, const Efl_Event *event)
{
   Efl_Loop_Timer_Data *timer = data;
   const Efl_Callback_Array_Item_Full *array = event->info;
   int i;

   for (i = 0; array[i].desc != NULL; i++)
     {
        if (array[i].desc == EFL_LOOP_TIMER_EVENT_TIMER_TICK)
          {
             if (timer->listening++ > 0) return;
             _efl_loop_timer_util_instanciate(timer->loop_data, timer);
             // No need to walk more than once per array as you can not del
             // a partial array
             return;
          }
     }
}

static void
_check_timer_event_catcher_del(void *data, const Efl_Event *event)
{
   Efl_Loop_Timer_Data *timer = data;
   const Efl_Callback_Array_Item_Full *array = event->info;
   int i;

   for (i = 0; array[i].desc != NULL; i++)
     {
        if (array[i].desc == EFL_LOOP_TIMER_EVENT_TIMER_TICK)
          {
             if ((--timer->listening) > 0) return;
             _efl_loop_timer_util_instanciate(timer->loop_data, timer);
             return;
          }
     }
}

EFL_CALLBACKS_ARRAY_DEFINE(timer_watch,
                          { EFL_EVENT_CALLBACK_ADD, _check_timer_event_catcher_add },
                          { EFL_EVENT_CALLBACK_DEL, _check_timer_event_catcher_del });

EOLIAN static Eo *
_efl_loop_timer_efl_object_constructor(Eo *obj, Efl_Loop_Timer_Data *timer)
{
   efl_constructor(efl_super(obj, MY_CLASS));
   efl_event_callback_array_add(obj, timer_watch(), timer);
   efl_wref_add(obj, &timer->object);

   timer->in = -1.0;
   timer->constructed = EINA_TRUE;
   return obj;
}

EOLIAN static Eo *
_efl_loop_timer_efl_object_finalize(Eo *obj, Efl_Loop_Timer_Data *pd)
{
   pd->loop = efl_provider_find(obj, EFL_LOOP_CLASS);
   pd->loop_data = efl_data_scope_get(pd->loop, EFL_LOOP_CLASS);

   if (pd->at < efl_loop_time_get(pd->loop))
     pd->at = ecore_time_get() + pd->in;
   else pd->at += pd->in;

   if (pd->in < 0.0)
     {
        ERR("You need to specify the interval of a timer to create a valid timer.");
        return NULL;
     }
   pd->initialized = EINA_TRUE;
   pd->finalized = EINA_TRUE;
   _efl_loop_timer_set(pd, pd->at, pd->in);
   return efl_finalize(efl_super(obj, MY_CLASS));
}

static void
_ecore_timer_legacy_del(void *data, const Efl_Event *event EINA_UNUSED)
{
   free(data);
}

static void
_ecore_timer_legacy_tick(void *data, const Efl_Event *event)
{
   Ecore_Timer_Legacy *legacy = data;

   legacy->inside_call = 1;
   if (!_ecore_call_task_cb(legacy->func, (void *)legacy->data) ||
       legacy->delete_me)
     efl_del(event->object);
   else legacy->inside_call = 0;
}

EFL_CALLBACKS_ARRAY_DEFINE(legacy_timer,
                          { EFL_LOOP_TIMER_EVENT_TIMER_TICK, _ecore_timer_legacy_tick },
                          { EFL_EVENT_DEL, _ecore_timer_legacy_del });

EAPI Ecore_Timer *
ecore_timer_add(double in, Ecore_Task_Cb func, const void *data)
{
   Ecore_Timer_Legacy *legacy;
   Eo *timer;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   if (!func)
     {
        ERR("Callback function must be set up for the class.");
        return NULL;
     }
   legacy = calloc(1, sizeof (Ecore_Timer_Legacy));
   if (!legacy) return NULL;
   legacy->func = func;
   legacy->data = data;
   timer = efl_add(MY_CLASS, efl_main_loop_get(),
                  efl_event_callback_array_add(efl_added, legacy_timer(), legacy),
                  efl_key_data_set(efl_added, "_legacy", legacy),
                  efl_loop_timer_interval_set(efl_added, in));
   return timer;
}

EAPI Ecore_Timer *
ecore_timer_loop_add(double in, Ecore_Task_Cb func, const void  *data)
{
   Ecore_Timer_Legacy *legacy;
   Eo *timer;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   if (!func)
     {
        ERR("Callback function must be set up for the class.");
        return NULL;
     }
   legacy = calloc(1, sizeof (Ecore_Timer_Legacy));
   if (!legacy) return NULL;
   legacy->func = func;
   legacy->data = data;
   timer = efl_add(MY_CLASS, efl_main_loop_get(),
                  efl_event_callback_array_add(efl_added, legacy_timer(), legacy),
                  efl_key_data_set(efl_added, "_legacy", legacy),
                  efl_loop_timer_loop_reset(efl_added),
                  efl_loop_timer_interval_set(efl_added, in));
   return timer;
}

EAPI void *
ecore_timer_del(Ecore_Timer *timer)
{
   Ecore_Timer_Legacy *legacy;
   void *data;

   if (!timer) return NULL;
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);

   legacy = efl_key_data_get(timer, "_legacy");
   // If legacy == NULL, this means double free or something
   if (legacy == NULL)
     {
        // Just in case it is an Eo timer, but not a legacy one.
        ERR("You are trying to destroy a timer which seems dead already.");
        efl_unref(timer);
        return NULL;
     }

   data = (void *)legacy->data;
   if (legacy->inside_call) legacy->delete_me = EINA_TRUE;
   else efl_del(timer);
   return data;
}

EOLIAN static void
_efl_loop_timer_timer_interval_set(Eo *obj EINA_UNUSED, Efl_Loop_Timer_Data *timer, double in)
{
   if (in < 0.0) in = 0.0;
   timer->in = in;
}

EOLIAN static double
_efl_loop_timer_timer_interval_get(const Eo *obj EINA_UNUSED, Efl_Loop_Timer_Data *timer)
{
   return timer->in;
}

EOLIAN static void
_efl_loop_timer_timer_delay(Eo *obj EINA_UNUSED, Efl_Loop_Timer_Data *pd, double add)
{
   _efl_loop_timer_util_delay(pd, add);
}

EOLIAN static void
_efl_loop_timer_timer_reset(Eo *obj EINA_UNUSED, Efl_Loop_Timer_Data *timer)
{
   double now, add;

   if (!timer->loop_data) return;
   // Do not reset the current timer while inside the callback
   if (timer->loop_data->timer_current == timer) return;

   now = ecore_time_get();
   if (!timer->initialized)
     {
        timer->at = now;
        return;
     }

   if (timer->frozen) add = timer->pending;
   else add = timer->at - now;
   _efl_loop_timer_util_delay(timer, timer->in - add);
}

EOLIAN static void
_efl_loop_timer_timer_loop_reset(Eo *obj EINA_UNUSED, Efl_Loop_Timer_Data *timer)
{
   double now, add;

   if (!timer->loop_data) return;
   // Do not reset the current timer while inside the callback
   if (timer->loop_data->timer_current == timer) return;

   now = efl_loop_time_get(timer->loop);
   if (!timer->initialized)
     {
        timer->at = now;
        return;
     }

   if (timer->frozen) add = timer->pending;
   else add = timer->at - now;
   _efl_loop_timer_util_delay(timer, timer->in - add);
}

EOLIAN static double
_efl_loop_timer_time_pending_get(const Eo *obj EINA_UNUSED, Efl_Loop_Timer_Data *timer)
{
   double now, ret = 0.0;

   now = ecore_time_get();
   if (timer->frozen) ret = timer->pending;
   else ret = timer->at - now;
   return ret;
}

EAPI void
ecore_timer_freeze(Ecore_Timer *timer)
{
   ECORE_TIMER_CHECK(timer);
   efl_event_freeze(timer);
}

EOLIAN static void
_efl_loop_timer_efl_object_event_freeze(Eo *obj, Efl_Loop_Timer_Data *timer)
{
   double now = 0.0;

   efl_event_freeze(efl_super(obj, MY_CLASS));
   // Timer already frozen
   if (timer->frozen) return;

   /* not set if timer is not finalized */
   if (timer->loop)
     now = efl_loop_time_get(timer->loop);
   /* only if timer interval has been set */
   if (timer->initialized)
     timer->pending = timer->at - now;
   else
     timer->pending = 0.0;
   timer->at = 0.0;
   timer->frozen = 1;

   _efl_loop_timer_util_instanciate(timer->loop_data, timer);
}

EAPI Eina_Bool
ecore_timer_freeze_get(Ecore_Timer *timer)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(EINA_FALSE);
   return !!efl_event_freeze_count_get(timer);
}

EOLIAN static int
_efl_loop_timer_efl_object_event_freeze_count_get(const Eo *obj EINA_UNUSED, Efl_Loop_Timer_Data *timer)
{
   return timer->frozen;
}

EAPI void
ecore_timer_thaw(Ecore_Timer *timer)
{
   ECORE_TIMER_CHECK(timer);
   efl_event_thaw(timer);
}

EOLIAN static void
_efl_loop_timer_efl_object_event_thaw(Eo *obj, Efl_Loop_Timer_Data *timer)
{
   double now;

   efl_event_thaw(efl_super(obj, MY_CLASS));

   if (!timer->frozen) return; // Timer not frozen
   timer->frozen = 0;

   if (timer->loop_data)
     {
        timer->loop_data->suspended = eina_inlist_remove
          (timer->loop_data->suspended, EINA_INLIST_GET(timer));
     }
   now = ecore_time_get();
   _efl_loop_timer_set(timer, timer->pending + now, timer->in);
}

EAPI char *
ecore_timer_dump(void)
{
   return NULL;
}

static void
_efl_loop_timer_util_loop_clear(Efl_Loop_Timer_Data *pd)
{
   Eina_Inlist *first;

   if (!pd->loop_data) return;
   // Check if we are the current timer, if so move along
   if (pd->loop_data->timer_current == pd)
     pd->loop_data->timer_current = (Efl_Loop_Timer_Data *)
       EINA_INLIST_GET(pd)->next;

   // Remove the timer from all possible pending list
   first = eina_inlist_first(EINA_INLIST_GET(pd));
   if (first == pd->loop_data->timers)
     pd->loop_data->timers = eina_inlist_remove
       (pd->loop_data->timers, EINA_INLIST_GET(pd));
   else if (first == pd->loop_data->suspended)
     pd->loop_data->suspended = eina_inlist_remove
       (pd->loop_data->suspended, EINA_INLIST_GET(pd));
}

static void
_efl_loop_timer_util_instanciate(Efl_Loop_Data *loop, Efl_Loop_Timer_Data *timer)
{
   Efl_Loop_Timer_Data *t2;

   if (!loop) return;
   _efl_loop_timer_util_loop_clear(timer);

   // And start putting it back where it belong
   if ((!timer->listening) || (timer->frozen) ||
       (timer->at <= 0.0) || (timer->in < 0.0))
     {
        loop->suspended = eina_inlist_prepend(loop->suspended,
                                              EINA_INLIST_GET(timer));
        return;
     }

   if (!timer->initialized)
     {
        ERR("Trying to instantiate an uninitialized timer is impossible.");
        return;
     }

   EINA_INLIST_REVERSE_FOREACH(loop->timers, t2)
     {
        if (timer->at > t2->at)
          {
             loop->timers = eina_inlist_append_relative(loop->timers,
                                                        EINA_INLIST_GET(timer),
                                                        EINA_INLIST_GET(t2));
             return;
          }
     }
   loop->timers = eina_inlist_prepend(loop->timers, EINA_INLIST_GET(timer));
}

static void
_efl_loop_timer_util_delay(Efl_Loop_Timer_Data *timer, double add)
{
   if (!timer->initialized)
     {
        ERR("Impossible to delay an uninitialized timer.");
        return;
     }
   if (timer->frozen)
     {
        timer->pending += add;
        return;
     }
   _efl_loop_timer_set(timer, timer->at + add, timer->in);
}

EOLIAN static void
_efl_loop_timer_efl_object_parent_set(Eo *obj, Efl_Loop_Timer_Data *pd, Efl_Object *parent)
{
   Eina_Inlist *first;

   efl_parent_set(efl_super(obj, EFL_LOOP_TIMER_CLASS), parent);

   if ((!pd->constructed) || (!pd->finalized)) return;

   // Remove the timer from all possible pending list
   first = eina_inlist_first(EINA_INLIST_GET(pd));
   if (first == pd->loop_data->timers)
     pd->loop_data->timers = eina_inlist_remove
       (pd->loop_data->timers, EINA_INLIST_GET(pd));
   else if (first == pd->loop_data->suspended)
     pd->loop_data->suspended = eina_inlist_remove
       (pd->loop_data->suspended, EINA_INLIST_GET(pd));

   if (efl_invalidated_get(obj)) return;

   pd->loop = efl_provider_find(obj, EFL_LOOP_CLASS);
   if (pd->loop)
     pd->loop_data = efl_data_scope_get(pd->loop, EFL_LOOP_CLASS);
   else
     pd->loop_data = NULL;

   if (efl_parent_get(obj) != parent) return;

   _efl_loop_timer_util_instanciate(pd->loop_data, pd);

   if (parent != NULL) pd->noparent = EINA_FALSE;
   else pd->noparent = EINA_TRUE;
}

EOLIAN static void
_efl_loop_timer_efl_object_destructor(Eo *obj, Efl_Loop_Timer_Data *pd)
{
   _efl_loop_timer_util_loop_clear(pd);
   efl_destructor(efl_super(obj, MY_CLASS));
}

void
_efl_loop_timer_enable_new(Eo *obj EINA_UNUSED, Efl_Loop_Data *pd)
{
   Efl_Loop_Timer_Data *timer;

   if (!pd->timers_added) return;
   pd->timers_added = 0;
   EINA_INLIST_FOREACH(pd->timers, timer) timer->just_added = 0;
}

int
_efl_loop_timers_exists(Eo *obj EINA_UNUSED, Efl_Loop_Data *pd)
{
   return !!pd->timers;
}

static inline Ecore_Timer *
_efl_loop_timer_first_get(Eo *ob EINA_UNUSED, Efl_Loop_Data *pd)
{
   Efl_Loop_Timer_Data *timer;

   EINA_INLIST_FOREACH(pd->timers, timer)
     {
        if (!timer->just_added) return timer->object;
     }
   return NULL;
}

static inline Efl_Loop_Timer_Data *
_efl_loop_timer_after_get(Efl_Loop_Timer_Data *base)
{
   Efl_Loop_Timer_Data *timer;
   Efl_Loop_Timer_Data *valid_timer = base;
   double maxtime = base->at + precision;

   EINA_INLIST_FOREACH(EINA_INLIST_GET(base)->next, timer)
     {
        if (EINA_UNLIKELY(!timer->initialized)) continue; // This shouldn't happen
        if (timer->at >= maxtime) break;
        if (!timer->just_added) valid_timer = timer;
     }
   return valid_timer;
}

double
_efl_loop_timer_next_get(Eo *obj, Efl_Loop_Data *pd)
{
   Ecore_Timer *object;
   Efl_Loop_Timer_Data *first;
   double now;
   double in;

   object = _efl_loop_timer_first_get(obj, pd);
   if (!object) return -1;

   first = _efl_loop_timer_after_get(efl_data_scope_get(object, MY_CLASS));
   now = efl_loop_time_get(obj);
   in = first->at - now;
   if (in < 0) in = 0;
   return in;
}

static inline void
_efl_loop_timer_reschedule(Efl_Loop_Timer_Data *timer, double when)
{
   if (timer->frozen || efl_invalidated_get(timer->object)) return;

   if (timer->loop_data &&
       (EINA_INLIST_GET(timer)->next || EINA_INLIST_GET(timer)->prev))
     {
        if (timer->loop_data->timers && (!timer->noparent))
          timer->loop_data->timers = eina_inlist_remove
            (timer->loop_data->timers, EINA_INLIST_GET(timer));
     }

   /* if the timer would have gone off more than 15 seconds ago,
    * assume that the system hung and set the timer to go off
    * timer->in from now. this handles system hangs, suspends
    * and more, so ecore will only "replay" the timers while
    * the system is suspended if it is suspended for less than
    * 15 seconds (basically). this also handles if the process
    * is stopped in a debugger or IO and other handling gets
    * really slow within the main loop.
    */
   if ((timer->at + timer->in) < (when - 15.0))
     _efl_loop_timer_set(timer, when + timer->in, timer->in);
   else
     _efl_loop_timer_set(timer, timer->at + timer->in, timer->in);
}

void
_efl_loop_timer_expired_timers_call(Eo *obj, Efl_Loop_Data *pd, double when)
{
   // call the first expired timer until no expired timers exist
   while (_efl_loop_timer_expired_call(obj, pd, when));
}

int
_efl_loop_timer_expired_call(Eo *obj EINA_UNUSED, Efl_Loop_Data *pd, double when)
{
   if (!pd->timers) return 0;
   if (pd->last_check > when)
     {
        Efl_Loop_Timer_Data *timer;
        // User set time backwards
        EINA_INLIST_FOREACH(pd->timers, timer)
          timer->at -= (pd->last_check - when);
     }
   pd->last_check = when;

   if (!pd->timer_current) // regular main loop, start from head
     pd->timer_current = (Efl_Loop_Timer_Data *)pd->timers;
   else
     {
        // recursive main loop, continue from where we were
        Efl_Loop_Timer_Data *timer_old = pd->timer_current;

        pd->timer_current = (Efl_Loop_Timer_Data *)
          EINA_INLIST_GET(pd->timer_current)->next;
        _efl_loop_timer_reschedule(timer_old, when);
     }

   while (pd->timer_current)
     {
        Efl_Loop_Timer_Data *timer = pd->timer_current;

        if (timer->at > when)
          {
             pd->timer_current = NULL; // ended walk, next should restart.
             return 0;
          }

        if (timer->just_added)
          {
             pd->timer_current = (Efl_Loop_Timer_Data *)
               EINA_INLIST_GET(pd->timer_current)->next;
             continue;
          }

        efl_ref(timer->object);
        eina_evlog("+timer", timer, 0.0, NULL);
        efl_event_callback_call(timer->object, EFL_LOOP_TIMER_EVENT_TIMER_TICK, NULL);
        eina_evlog("-timer", timer, 0.0, NULL);

        // may have changed in recursive main loops
        // this current timer can not die yet as we hold a reference on it
        if (pd->timer_current)
          pd->timer_current = (Efl_Loop_Timer_Data *)
            EINA_INLIST_GET(pd->timer_current)->next;
        _efl_loop_timer_reschedule(timer, when);
        efl_unref(timer->object);
     }
   return 0;
}

static void
_efl_loop_timer_set(Efl_Loop_Timer_Data *timer, double at, double in)
{
   if (!timer->loop_data) return;
   timer->loop_data->timers_added = 1;
   timer->in = in;
   timer->just_added = 1;
   timer->initialized = 1;
   if (!timer->frozen)
     {
        timer->at = at;
        timer->pending = 0.0;
     }
   _efl_loop_timer_util_instanciate(timer->loop_data, timer);
}

#include "efl_loop_timer.eo.c"
#include "efl_loop_timer_eo.legacy.c"

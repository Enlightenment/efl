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

#define ECORE_TIMER_CHECK(obj)                       \
  if (!eo_isa((obj), MY_CLASS)) \
    return

struct _Efl_Loop_Timer_Data
{
   EINA_INLIST;

   Eo            *object;

   double         in;
   double         at;
   double         pending;

   int            listening;

   unsigned char  just_added : 1;
   unsigned char  frozen : 1;
   unsigned char  initialized : 1;
   unsigned char  noparent : 1;
};

typedef struct _Efl_Loop_Timer_Data Efl_Loop_Timer_Data;

static void _efl_loop_timer_util_delay(Efl_Loop_Timer_Data *timer,
                                       double add);
static void _efl_loop_timer_util_instanciate(Efl_Loop_Timer_Data *timer);
static void _efl_loop_timer_set(Efl_Loop_Timer_Data *timer,
                                double        at,
                                double        in);

static Eina_Inlist *timers = NULL;
static Eina_Inlist *suspended = NULL;

static Efl_Loop_Timer_Data *timer_current = NULL;

static int timers_added = 0;

static double last_check = 0.0;
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

   if (value < 0.0)
     {
        ERR("Precision %f less than zero, ignored", value);
        return ;
     }
   precision = value;
}

static Eina_Bool
_check_timer_event_catcher_add(void *data, const Eo_Event *event)
{
   const Eo_Callback_Array_Item *array = event->info;
   Efl_Loop_Timer_Data *timer = data;
   int i;

   for (i = 0; array[i].desc != NULL; i++)
     {
        if (array[i].desc == EFL_LOOP_TIMER_EVENT_TICK)
          {
             if (timer->listening++ > 0) return EO_CALLBACK_CONTINUE;
             _efl_loop_timer_util_instanciate(timer);
             // No need to walk more than once per array as you can not del
             // a partial array
             return EO_CALLBACK_CONTINUE;
          }
     }

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_check_timer_event_catcher_del(void *data, const Eo_Event *event)
{
   const Eo_Callback_Array_Item *array = event->info;
   Efl_Loop_Timer_Data *timer = data;
   int i;

   for (i = 0; array[i].desc != NULL; i++)
     {
        if (array[i].desc == EFL_LOOP_TIMER_EVENT_TICK)
          {
             if ((--timer->listening) > 0) return EO_CALLBACK_CONTINUE;
             _efl_loop_timer_util_instanciate(timer);
             return EO_CALLBACK_CONTINUE;
          }
     }

   return EO_CALLBACK_CONTINUE;
}

EO_CALLBACKS_ARRAY_DEFINE(timer_watch,
                          { EO_EVENT_CALLBACK_ADD, _check_timer_event_catcher_add },
                          { EO_EVENT_CALLBACK_DEL, _check_timer_event_catcher_del });

EOLIAN static Eo *
_efl_loop_timer_eo_base_constructor(Eo *obj, Efl_Loop_Timer_Data *timer)
{
   eo_constructor(eo_super(obj, MY_CLASS));

   eo_event_callback_array_add(obj, timer_watch(), timer);

   eo_wref_add(obj, &timer->object);

   timer->at = ecore_time_get();
   timer->initialized = 0;

   return obj;
}


EOLIAN static Eo *
_efl_loop_timer_eo_base_finalize(Eo *obj, Efl_Loop_Timer_Data *pd)
{
   _efl_loop_timer_util_instanciate(pd);

   return eo_finalize(eo_super(obj, MY_CLASS));
}

typedef struct _Ecore_Timer_Legacy Ecore_Timer_Legacy;
struct _Ecore_Timer_Legacy
{
   Ecore_Task_Cb func;

   const void *data;

   Eina_Bool inside_call : 1;
   Eina_Bool delete_me : 1;
};

static Eina_Bool
_ecore_timer_legacy_del(void *data, const Eo_Event *event EINA_UNUSED)
{
   Ecore_Timer_Legacy *legacy = data;

   free(legacy);

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_ecore_timer_legacy_tick(void *data, const Eo_Event *event)
{
   Ecore_Timer_Legacy *legacy = data;

   legacy->inside_call = 1;
   if (!_ecore_call_task_cb(legacy->func, (void*)legacy->data) ||
       legacy->delete_me)
     eo_del(event->object);

   return EO_CALLBACK_CONTINUE;
}

EO_CALLBACKS_ARRAY_DEFINE(legacy_timer,
                          { EFL_LOOP_TIMER_EVENT_TICK, _ecore_timer_legacy_tick },
                          { EO_EVENT_DEL, _ecore_timer_legacy_del });

EAPI Ecore_Timer *
ecore_timer_add(double        in,
                Ecore_Task_Cb func,
                const void   *data)
{
   Ecore_Timer_Legacy *legacy;
   Eo *timer;

   legacy = calloc(1, sizeof (Ecore_Timer_Legacy));
   if (!legacy) return NULL;

   legacy->func = func;
   legacy->data = data;
   timer = eo_add(MY_CLASS, ecore_main_loop_get(),
                  eo_event_callback_array_add(eo_self, legacy_timer(), legacy),
                  eo_key_data_set(eo_self, "_legacy", legacy),
                  efl_loop_timer_interval_set(eo_self, in));

   return timer;
}

EAPI Ecore_Timer *
ecore_timer_loop_add(double        in,
                     Ecore_Task_Cb func,
                     const void   *data)
{
   Ecore_Timer_Legacy *legacy;
   Eo *timer;

   legacy = calloc(1, sizeof (Ecore_Timer_Legacy));
   if (!legacy) return NULL;

   legacy->func = func;
   legacy->data = data;
   timer = eo_add(MY_CLASS, ecore_main_loop_get(),
                  eo_event_callback_array_add(eo_self, legacy_timer(), legacy),
                  eo_key_data_set(eo_self, "_legacy", legacy),
                  efl_loop_timer_loop_reset(eo_self),
                  efl_loop_timer_interval_set(eo_self, in));

   return timer;
}

EAPI void *
ecore_timer_del(Ecore_Timer *timer)
{
   Ecore_Timer_Legacy *legacy;
   void *data;

   if (!timer) return NULL;
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);

   legacy = eo_key_data_get(timer, "_legacy");
   // If legacy == NULL, this means double free or something
   if (legacy == NULL)
     {
        // Just in case it is an Eo timer, but not a legacy one.
        ERR("You are trying to destroy a timer which seems dead already.");
        eo_unref(timer);
        return NULL;
     }

   data = (void*) legacy->data;

   if (legacy->inside_call)
     legacy->delete_me = EINA_TRUE;
   else
     eo_del(timer);

   return data;
}

EOLIAN static void
_efl_loop_timer_interval_set(Eo *obj EINA_UNUSED, Efl_Loop_Timer_Data *timer, double in)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   if (in < 0.0) in = 0.0;

   timer->in = in;

   if (!timer->initialized)
     _efl_loop_timer_set(timer, timer->at + in, in);
}

EOLIAN static double
_efl_loop_timer_interval_get(Eo *obj EINA_UNUSED, Efl_Loop_Timer_Data *timer)
{
   double ret = -1.0;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(ret);
   ret = timer->in;

   return ret;
}

EOLIAN static void
_efl_loop_timer_delay(Eo *obj EINA_UNUSED, Efl_Loop_Timer_Data *pd, double add)
{
   EINA_MAIN_LOOP_CHECK_RETURN;

   _efl_loop_timer_util_delay(pd, add);
}

EOLIAN static void
_efl_loop_timer_reset(Eo *obj EINA_UNUSED, Efl_Loop_Timer_Data *timer)
{
   double now, add;
   EINA_MAIN_LOOP_CHECK_RETURN;

   now = ecore_time_get();

   if (!timer->initialized)
     {
        timer->at = now;
        return;
     }

   if (timer->frozen)
     add = timer->pending;
   else
     add = timer->at - now;
   _efl_loop_timer_util_delay(timer, timer->in - add);
}

EOLIAN static void
_efl_loop_timer_loop_reset(Eo *obj EINA_UNUSED, Efl_Loop_Timer_Data *timer)
{
   double now, add;
   EINA_MAIN_LOOP_CHECK_RETURN;

   now = ecore_loop_time_get();

   if (!timer->initialized)
     {
        timer->at = now;
        return ;
     }

   if (timer->frozen)
     add = timer->pending;
   else
     add = timer->at - now;
   _efl_loop_timer_util_delay(timer, timer->in - add);
}

EOLIAN static double
_efl_loop_timer_pending_get(Eo *obj EINA_UNUSED, Efl_Loop_Timer_Data *timer)
{
   double now;
   double ret = 0.0;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(ret);

   now = ecore_time_get();

   if (timer->frozen)
     ret = timer->pending;
   else
     ret = timer->at - now;

   return ret;
}

EAPI void
ecore_timer_freeze(Ecore_Timer *timer)
{
   ECORE_TIMER_CHECK(timer);
   eo_event_freeze(timer);
}

EOLIAN static void
_efl_loop_timer_eo_base_event_freeze(Eo *obj EINA_UNUSED, Efl_Loop_Timer_Data *timer)
{
   double now;

   EINA_MAIN_LOOP_CHECK_RETURN;

   eo_event_freeze(eo_super(obj, MY_CLASS));

   /* Timer already frozen */
   if (timer->frozen)
     return;

   now = ecore_time_get();

   timer->pending = timer->at - now;
   timer->at = 0.0;
   timer->frozen = 1;

   _efl_loop_timer_util_instanciate(timer);
}

EAPI Eina_Bool
ecore_timer_freeze_get(Ecore_Timer *timer)
{
   int r = 0;

   r = eo_event_freeze_count_get(timer);
   return !!r;
}

EOLIAN static int
_efl_loop_timer_eo_base_event_freeze_count_get(Eo *obj EINA_UNUSED, Efl_Loop_Timer_Data *timer)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0);

   return timer->frozen;
}

EAPI void
ecore_timer_thaw(Ecore_Timer *timer)
{
   ECORE_TIMER_CHECK(timer);
   eo_event_thaw(timer);
}

EOLIAN static void
_efl_loop_timer_eo_base_event_thaw(Eo *obj, Efl_Loop_Timer_Data *timer)
{
   double now;

   EINA_MAIN_LOOP_CHECK_RETURN;

   eo_event_thaw(eo_super(obj, MY_CLASS));

   /* Timer not frozen */
   if (!timer->frozen)
     return ;

   suspended = eina_inlist_remove(suspended, EINA_INLIST_GET(timer));
   now = ecore_time_get();

   _efl_loop_timer_set(timer, timer->pending + now, timer->in);
}

EAPI char *
ecore_timer_dump(void)
{
   return NULL;
}

static void
_efl_loop_timer_util_instanciate(Efl_Loop_Timer_Data *timer)
{
   Efl_Loop_Timer_Data *t2;
   Eina_Inlist *first;

   // Remove the timer from all possible pending list
   first = eina_inlist_first(EINA_INLIST_GET(timer));
   if (first == timers)
     timers = eina_inlist_remove(timers, EINA_INLIST_GET(timer));
   else if (first == suspended)
     suspended = eina_inlist_remove(suspended, EINA_INLIST_GET(timer));

   // And start putting it back where it belong
   if (!timer->listening || timer->frozen || timer->at <= 0.0 || timer->in < 0.0)
     {
        suspended = eina_inlist_prepend(suspended, EINA_INLIST_GET(timer));
        return ;
     }

   EINA_INLIST_REVERSE_FOREACH(timers, t2)
     {
        if (timer->at > t2->at)
          {
             timers = eina_inlist_append_relative(timers,
                                                  EINA_INLIST_GET(timer),
                                                  EINA_INLIST_GET(t2));
             return;
          }
     }
   timers = eina_inlist_prepend(timers, EINA_INLIST_GET(timer));
}

static void
_efl_loop_timer_util_delay(Efl_Loop_Timer_Data *timer,
                           double add)
{
   if (timer->frozen)
     {
        timer->pending += add;
        return ;
     }

   _efl_loop_timer_set(timer, timer->at + add, timer->in);
}

EOLIAN static void
_efl_loop_timer_eo_base_parent_set(Eo *obj EINA_UNUSED, Efl_Loop_Timer_Data *pd, Eo_Base *parent)
{
   Eina_Inlist *first;

   first = eina_inlist_first(EINA_INLIST_GET(pd));
   if (first == timers)
     timers = eina_inlist_remove(timers, EINA_INLIST_GET(pd));
   else if (first == suspended)
     suspended = eina_inlist_remove(suspended, EINA_INLIST_GET(pd));

   eo_parent_set(eo_super(obj, EFL_LOOP_USER_CLASS), parent);

   if (eo_parent_get(obj) != parent)
     return ;

   if (parent != NULL)
     {
        _efl_loop_timer_util_instanciate(pd);
        pd->noparent = EINA_FALSE;
     }
   else
     {
        pd->noparent = EINA_TRUE;
     }
}

EOLIAN static void
_efl_loop_timer_eo_base_destructor(Eo *obj, Efl_Loop_Timer_Data *pd)
{
   Eina_Inlist *first;

   // Check if we are the current timer, if so move along
   if (timer_current == pd)
     timer_current = (Efl_Loop_Timer_Data *)EINA_INLIST_GET(pd)->next;

   // Remove the timer from all possible pending list
   first = eina_inlist_first(EINA_INLIST_GET(pd));
   if (first == timers)
     timers = eina_inlist_remove(timers, EINA_INLIST_GET(pd));
   else if (first == suspended)
     suspended = eina_inlist_remove(suspended, EINA_INLIST_GET(pd));

   eo_destructor(eo_super(obj, MY_CLASS));
}

void
_efl_loop_timer_shutdown(void)
{
   timer_current = NULL;
}

void
_efl_loop_timer_enable_new(void)
{
   Efl_Loop_Timer_Data *timer;

   if (!timers_added) return;
   timers_added = 0;
   EINA_INLIST_FOREACH(timers, timer) timer->just_added = 0;
}

int
_efl_loop_timers_exists(void)
{
   return !!timers;
}

static inline Ecore_Timer *
_efl_loop_timer_first_get(void)
{
   Efl_Loop_Timer_Data *timer;

   EINA_INLIST_FOREACH(timers, timer)
     if (!timer->just_added) return timer->object;

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
        if (timer->at >= maxtime) break;
        if (!timer->just_added)
          valid_timer = timer;
     }

   return valid_timer;
}

double
_efl_loop_timer_next_get(void)
{
   Ecore_Timer *object;
   Efl_Loop_Timer_Data *first;
   double now;
   double in;

   object = _efl_loop_timer_first_get();
   if (!object) return -1;

   first = _efl_loop_timer_after_get(eo_data_scope_get(object, MY_CLASS));

   now = ecore_loop_time_get();
   in = first->at - now;
   if (in < 0) in = 0;
   return in;
}

static inline void
_efl_loop_timer_reschedule(Efl_Loop_Timer_Data *timer,
                           double when)
{
   if (timer->frozen) return;

   if (timers && !timer->noparent)
     timers = eina_inlist_remove(timers, EINA_INLIST_GET(timer));

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
_efl_loop_timer_expired_timers_call(double when)
{
   /* call the first expired timer until no expired timers exist */
    while (_efl_loop_timer_expired_call(when)) ;
}

int
_efl_loop_timer_expired_call(double when)
{
   if (!timers) return 0;
   if (last_check > when)
     {
        Efl_Loop_Timer_Data *timer;
        /* User set time backwards */
        EINA_INLIST_FOREACH(timers, timer) timer->at -= (last_check - when);
     }
   last_check = when;

   if (!timer_current)
     {
        /* regular main loop, start from head */
        timer_current = (Efl_Loop_Timer_Data *)timers;
     }
   else
     {
        /* recursive main loop, continue from where we were */
        Efl_Loop_Timer_Data *timer_old = timer_current;

        timer_current = (Efl_Loop_Timer_Data *)EINA_INLIST_GET(timer_current)->next;
        _efl_loop_timer_reschedule(timer_old, when);
     }

   while (timer_current)
     {
        Efl_Loop_Timer_Data *timer = timer_current;

        if (timer->at > when)
          {
             timer_current = NULL; /* ended walk, next should restart. */
             return 0;
          }

        if (timer->just_added)
          {
             timer_current = (Efl_Loop_Timer_Data *)EINA_INLIST_GET(timer_current)->next;
             continue;
          }

        eo_ref(timer->object);
        eina_evlog("+timer", timer, 0.0, NULL);
        eo_event_callback_call(timer->object, EFL_LOOP_TIMER_EVENT_TICK, NULL);
        eina_evlog("-timer", timer, 0.0, NULL);

        /* may have changed in recursive main loops */
        /* this current timer can not die yet as we hold a reference on it */
        if (timer_current)
          timer_current = (Efl_Loop_Timer_Data *)EINA_INLIST_GET(timer_current)->next;

        _efl_loop_timer_reschedule(timer, when);
        eo_unref(timer->object);
     }
   return 0;
}

static void
_efl_loop_timer_set(Efl_Loop_Timer_Data *timer,
                    double at,
                    double in)
{
   timers_added = 1;
   timer->at = at;
   timer->in = in;
   timer->just_added = 1;
   timer->initialized = 1;
   timer->frozen = 0;
   timer->pending = 0.0;

   _efl_loop_timer_util_instanciate(timer);
}

#include "efl_loop_timer.eo.c"

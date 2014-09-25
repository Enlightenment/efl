#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include <Eo.h>

#include "Ecore.h"
#include "ecore_private.h"

#define MY_CLASS ECORE_TIMER_CLASS
#define MY_CLASS_NAME "Ecore_Timer"

#define ECORE_TIMER_CHECK(obj)                       \
  if (!eo_isa((obj), ECORE_TIMER_CLASS)) \
    return

#ifdef WANT_ECORE_TIMER_DUMP
# include <string.h>
# include <execinfo.h>
# define ECORE_TIMER_DEBUG_BT_NUM 64
typedef void (*Ecore_Timer_Bt_Func)();
#endif

struct _Ecore_Timer_Data
{
   EINA_INLIST;
   Ecore_Timer         *obj;
   double              in;
   double              at;
   double              pending;
   Ecore_Task_Cb       func;
   void               *data;

#ifdef WANT_ECORE_TIMER_DUMP
   Ecore_Timer_Bt_Func timer_bt[ECORE_TIMER_DEBUG_BT_NUM];
   int                 timer_bt_num;
#endif

   int                 references;
   unsigned char       delete_me : 1;
   unsigned char       just_added : 1;
   unsigned char       frozen : 1;
};

typedef struct _Ecore_Timer_Data Ecore_Timer_Data;

static void _ecore_timer_set(Ecore_Timer *timer,
                             double        at,
                             double        in,
                             Ecore_Task_Cb func,
                             void         *data);
#ifdef WANT_ECORE_TIMER_DUMP
static int _ecore_timer_cmp(const void *d1,
                            const void *d2);
#endif

static int timers_added = 0;
static int timers_delete_me = 0;
static Ecore_Timer_Data *timers = NULL;
static Ecore_Timer_Data *timer_current = NULL;
static Ecore_Timer_Data *suspended = NULL;
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
   _ecore_lock();

   if (value < 0.0)
     {
        ERR("Precision %f less than zero, ignored", value);
        goto unlock;
     }
   precision = value;

unlock:
   _ecore_unlock();
}

EAPI Ecore_Timer *
ecore_timer_add(double        in,
                Ecore_Task_Cb func,
                const void   *data)
{
   Ecore_Timer *timer = NULL;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   timer = eo_add(MY_CLASS, _ecore_parent, ecore_obj_timer_constructor(in, func, data));
   return timer;
}

static Eina_Bool
_ecore_timer_add(Ecore_Timer *obj,
                 Ecore_Timer_Data *timer,
                 double now,
                 double in,
                 Ecore_Task_Cb func,
                 const void *data)
{

   if (EINA_UNLIKELY(!eina_main_loop_is()))
     {
        eo_error_set(obj);
        EINA_MAIN_LOOP_CHECK_RETURN_VAL(EINA_FALSE);
     }

   timer->obj = obj;
   eo_manual_free_set(obj, EINA_TRUE);

   if (!func)
     {
        eo_error_set(obj);
        ERR("callback function must be set up for an object of class: '%s'", MY_CLASS_NAME);
        return EINA_FALSE;
     }

   if (in < 0.0) in = 0.0;

#ifdef WANT_ECORE_TIMER_DUMP
   timer->timer_bt_num = backtrace((void **)(timer->timer_bt),
                                   ECORE_TIMER_DEBUG_BT_NUM);
#endif
   _ecore_timer_set(obj, now + in, in, func, (void *)data);
   return EINA_TRUE;
}

EOLIAN static void
_ecore_timer_constructor(Eo *obj, Ecore_Timer_Data *timer, double in, Ecore_Task_Cb func, const void *data)
{
   double now;

   _ecore_lock();
   now = ecore_time_get();

   _ecore_timer_add(obj, timer, now, in, func, data);
   _ecore_unlock();
}

EOLIAN static void
_ecore_timer_loop_constructor(Eo *obj, Ecore_Timer_Data *timer, double in, Ecore_Task_Cb func, const void *data)
{
   double now;

   now = ecore_loop_time_get();

   _ecore_timer_add(obj, timer, now, in, func, data);
}

EAPI Ecore_Timer *
ecore_timer_loop_add(double        in,
                     Ecore_Task_Cb func,
                     const void   *data)
{
   Ecore_Timer *timer;

   _ecore_lock();
   timer = _ecore_timer_loop_add(in, func, data);
   _ecore_unlock();

   return timer;
}

EAPI void *
ecore_timer_del(Ecore_Timer *timer)
{
   void *data = NULL;

   if (!timer) return NULL;
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   _ecore_lock();

   data = _ecore_timer_del(timer);

   _ecore_unlock();
   return data;
}

EOLIAN static void
_ecore_timer_interval_set(Eo *obj EINA_UNUSED, Ecore_Timer_Data *timer, double in)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   if (in < 0.0) in = 0.0;

   _ecore_lock();
   timer->in = in;
   _ecore_unlock();
}

EOLIAN static double
_ecore_timer_interval_get(Eo *obj EINA_UNUSED, Ecore_Timer_Data *timer)
{
   double ret = -1.0;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(ret);
   _ecore_lock();
   ret = timer->in;
   _ecore_unlock();

   return ret;
}

EOLIAN static void
_ecore_timer_delay(Eo *obj, Ecore_Timer_Data *_pd EINA_UNUSED, double add)
{
   EINA_MAIN_LOOP_CHECK_RETURN;

   _ecore_lock();
   _ecore_timer_util_delay(obj, add);
   _ecore_unlock();
}

EOLIAN static void
_ecore_timer_reset(Eo *obj, Ecore_Timer_Data *timer)
{
   double now, add;
   EINA_MAIN_LOOP_CHECK_RETURN;

   _ecore_lock();
   now = ecore_time_get();

   if (timer->frozen)
     add = timer->pending;
   else
     add = timer->at - now;
   _ecore_timer_util_delay(obj, timer->in - add);
   _ecore_unlock();
}

EOLIAN static double
_ecore_timer_pending_get(Eo *obj EINA_UNUSED, Ecore_Timer_Data *timer)
{
   double now;
   double ret = 0.0;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(ret);

   _ecore_lock();

   now = ecore_time_get();

   if (timer->frozen)
     ret = timer->pending;
   else
     ret = timer->at - now;
   _ecore_unlock();

   return ret;
}

EAPI void
ecore_timer_freeze(Ecore_Timer *timer)
{
   ECORE_TIMER_CHECK(timer);
   eo_do(timer, eo_event_freeze());
}

EOLIAN static void
_ecore_timer_eo_base_event_freeze(Eo *obj EINA_UNUSED, Ecore_Timer_Data *timer)
{
   double now;

   EINA_MAIN_LOOP_CHECK_RETURN;

   _ecore_lock();

   /* Timer already frozen */
   if (timer->frozen)
     goto unlock;

   timers = (Ecore_Timer_Data *)eina_inlist_remove(EINA_INLIST_GET(timers), EINA_INLIST_GET(timer));
   suspended = (Ecore_Timer_Data *)eina_inlist_prepend(EINA_INLIST_GET(suspended), EINA_INLIST_GET(timer));

   now = ecore_time_get();

   timer->pending = timer->at - now;
   timer->at = 0.0;
   timer->frozen = 1;
unlock:
   _ecore_unlock();
}

EAPI Eina_Bool
ecore_timer_freeze_get(Ecore_Timer *timer)
{
   int r = 0;

   eo_do(timer, r = eo_event_freeze_count_get());
   return !!r;
}

EOLIAN static int
_ecore_timer_eo_base_event_freeze_count_get(Eo *obj EINA_UNUSED, Ecore_Timer_Data *timer)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0);

   return timer->frozen;
}

EAPI void
ecore_timer_thaw(Ecore_Timer *timer)
{
   ECORE_TIMER_CHECK(timer);
   eo_do(timer, eo_event_thaw());
}

EOLIAN static void
_ecore_timer_eo_base_event_thaw(Eo *obj, Ecore_Timer_Data *timer)
{
   double now;

   EINA_MAIN_LOOP_CHECK_RETURN;

   _ecore_lock();

   /* Timer not frozen */
   if (!timer->frozen)
     goto unlock;

   suspended = (Ecore_Timer_Data *)eina_inlist_remove(EINA_INLIST_GET(suspended), EINA_INLIST_GET(timer));
   now = ecore_time_get();

   _ecore_timer_set(obj, timer->pending + now, timer->in, timer->func, timer->data);
unlock:
   _ecore_unlock();
}

EAPI char *
ecore_timer_dump(void)
{
#ifdef WANT_ECORE_TIMER_DUMP
   Eina_Strbuf *result;
   char *out;
   Ecore_Timer_Data *tm;
   Eina_List *tmp = NULL;
   int living_timer = 0;
   int unknow_timer = 0;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   _ecore_lock();
   result = eina_strbuf_new();

   EINA_INLIST_FOREACH(timers, tm)
     tmp = eina_list_sorted_insert(tmp, _ecore_timer_cmp, tm);

   EINA_LIST_FREE(tmp, tm)
     {
        char **strings;
        int j;

        if (!tm->frozen && !tm->delete_me)
          living_timer++;

        strings = backtrace_symbols((void **)tm->timer_bt, tm->timer_bt_num);
        if (tm->timer_bt_num <= 0 || strings == NULL)
          {
             unknow_timer++;
             continue;
          }

        eina_strbuf_append_printf(result, "*** timer: %f ***\n", tm->in);
        if (tm->frozen)
          eina_strbuf_append(result, "FROZEN\n");
        if (tm->delete_me)
          eina_strbuf_append(result, "DELETED\n");
        for (j = 0; j < tm->timer_bt_num; j++)
          eina_strbuf_append_printf(result, "%s\n", strings[j]);

        free(strings);
     }

   eina_strbuf_append_printf(result, "\n***\nThere is %i living timer.\nWe did lost track of %i timers.\n", living_timer, unknow_timer);

   out = eina_strbuf_string_steal(result);
   eina_strbuf_free(result);
   _ecore_unlock();

   return out;
#else
   return NULL;
#endif
}

Ecore_Timer *
_ecore_timer_loop_add(double        in,
                      Ecore_Task_Cb func,
                      const void   *data)
{
   Ecore_Timer *timer = NULL;
   timer = eo_add(MY_CLASS, _ecore_parent, ecore_obj_timer_loop_constructor(in, func, data));

   return timer;
}

EAPI void
_ecore_timer_util_delay(Ecore_Timer *obj,
                   double       add)
{
   Ecore_Timer_Data *timer = eo_data_scope_get(obj, MY_CLASS);

   if (timer->frozen)
     {
        timer->pending += add;
     }
   else
     {
        timers = (Ecore_Timer_Data *)eina_inlist_remove(EINA_INLIST_GET(timers), EINA_INLIST_GET(timer));
        eo_data_unref(obj, timer);
        _ecore_timer_set(obj, timer->at + add, timer->in, timer->func, timer->data);
     }
}

void *
_ecore_timer_del(Ecore_Timer *obj)
{
   Ecore_Timer_Data *timer = eo_data_scope_get(obj, MY_CLASS);

   EINA_SAFETY_ON_NULL_RETURN_VAL(timer, NULL);

   if (timer->frozen && !timer->references)
     {
        void *data = timer->data;

        eo_data_unref(obj, timer);
        suspended = (Ecore_Timer_Data *)eina_inlist_remove(EINA_INLIST_GET(suspended), EINA_INLIST_GET(timer));

        if (timer->delete_me)
          timers_delete_me--;

        eo_do(obj, eo_parent_set(NULL));

        if (eo_destructed_is(obj))
          eo_manual_free(obj);
        else
          eo_manual_free_set(obj, EINA_FALSE);
        return data;
     }

   EINA_SAFETY_ON_TRUE_RETURN_VAL(timer->delete_me, NULL);
   timer->delete_me = 1;
   timers_delete_me++;
   return timer->data;
}

EOLIAN static void
_ecore_timer_eo_base_destructor(Eo *obj, Ecore_Timer_Data *pd)
{
   if (!pd->delete_me)
   {
     pd->delete_me = 1;
     timers_delete_me++;
   }

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

void
_ecore_timer_shutdown(void)
{
   Ecore_Timer_Data *timer;

   while ((timer = timers))
     {
        timers = (Ecore_Timer_Data *)eina_inlist_remove(EINA_INLIST_GET(timers), EINA_INLIST_GET(timers));

        eo_data_unref(timer->obj, timer);
        eo_do(timer->obj, eo_parent_set(NULL));
        if (eo_destructed_is(timer->obj))
          eo_manual_free(timer->obj);
        else
          eo_manual_free_set(timer->obj, EINA_FALSE);
     }

   while ((timer = suspended))
     {
        suspended = (Ecore_Timer_Data *)eina_inlist_remove(EINA_INLIST_GET(suspended), EINA_INLIST_GET(suspended));

        eo_data_unref(timer->obj, timer);
        eo_do(timer->obj, eo_parent_set(NULL));
        if (eo_destructed_is(timer->obj))
          eo_manual_free(timer->obj);
        else
          eo_manual_free_set(timer->obj, EINA_FALSE);
     }

   timer_current = NULL;
}

void
_ecore_timer_cleanup(void)
{
   Ecore_Timer_Data *l;
   int in_use = 0, todo = timers_delete_me, done = 0;

   if (!timers_delete_me) return;
   for (l = timers; l; )
     {
        Ecore_Timer_Data *timer = l;

        l = (Ecore_Timer_Data *)EINA_INLIST_GET(l)->next;
        if (timer->delete_me)
          {
             if (timer->references)
               {
                  in_use++;
                  continue;
               }
             timers = (Ecore_Timer_Data *)eina_inlist_remove(EINA_INLIST_GET(timers), EINA_INLIST_GET(timer));

             eo_data_unref(timer->obj, timer);
             eo_do(timer->obj, eo_parent_set(NULL));
             if (eo_destructed_is(timer->obj))
               eo_manual_free(timer->obj);
             else
               eo_manual_free_set(timer->obj, EINA_FALSE);
             timers_delete_me--;
             done++;
             if (timers_delete_me == 0) return;
          }
     }
   for (l = suspended; l; )
     {
        Ecore_Timer_Data *timer = l;

        l = (Ecore_Timer_Data *)EINA_INLIST_GET(l)->next;
        if (timer->delete_me)
          {
             if (timer->references)
               {
                  in_use++;
                  continue;
               }
             suspended = (Ecore_Timer_Data *)eina_inlist_remove(EINA_INLIST_GET(suspended), EINA_INLIST_GET(timer));

             eo_data_unref(timer->obj, timer);
             eo_do(timer->obj, eo_parent_set(NULL));
             if (eo_destructed_is(timer->obj))
                eo_manual_free(timer->obj);
             else
                eo_manual_free_set(timer->obj, EINA_FALSE);
             timers_delete_me--;
             done++;
             if (timers_delete_me == 0) return;
          }
     }

   if ((!in_use) && (timers_delete_me))
     {
        ERR("%d timers to delete, but they were not found!"
            "Stats: todo=%d, done=%d, pending=%d, in_use=%d. "
            "reset counter.",
            timers_delete_me, todo, done, todo - done, in_use);
        timers_delete_me = 0;
     }
}

void
_ecore_timer_enable_new(void)
{
   Ecore_Timer_Data *timer;

   if (!timers_added) return;
   timers_added = 0;
   EINA_INLIST_FOREACH(timers, timer) timer->just_added = 0;
}

int
_ecore_timers_exists(void)
{
   Ecore_Timer_Data *timer = timers;

   while ((timer) && (timer->delete_me))
     timer = (Ecore_Timer_Data *)EINA_INLIST_GET(timer)->next;

   return !!timer;
}

static inline Ecore_Timer *
_ecore_timer_first_get(void)
{
   Ecore_Timer *ret = NULL;
   Ecore_Timer_Data *timer = timers;

   while ((timer) && ((timer->delete_me) || (timer->just_added)))
     timer = (Ecore_Timer_Data *)EINA_INLIST_GET(timer)->next;

   if (timer)
     ret = timer->obj;
   return ret;
}

static inline Ecore_Timer *
_ecore_timer_after_get(Ecore_Timer *obj)
{
   Ecore_Timer *ret = NULL;
   Ecore_Timer_Data *base = eo_data_scope_get(obj, MY_CLASS);

   Ecore_Timer_Data *timer = (Ecore_Timer_Data *)EINA_INLIST_GET(base)->next;
   Ecore_Timer_Data *valid_timer = NULL;
   double maxtime = base->at + precision;

   while ((timer) && (timer->at < maxtime))
     {
        if (!((timer->delete_me) || (timer->just_added)))
          valid_timer = timer;
        timer = (Ecore_Timer_Data *)EINA_INLIST_GET(timer)->next;
     }

   if (valid_timer)
     ret = valid_timer->obj;
   return ret;
}

double
_ecore_timer_next_get(void)
{
   double now;
   double in;
   Ecore_Timer *first_obj, *second_obj;
   Ecore_Timer_Data *first;

   first_obj = _ecore_timer_first_get();
   if (!first_obj) return -1;

   second_obj = _ecore_timer_after_get(first_obj);
   if (second_obj) first_obj = second_obj;

   first = eo_data_scope_get(first_obj, MY_CLASS);

   now = ecore_loop_time_get();
   in = first->at - now;
   if (in < 0) in = 0;
   return in;
}

static inline void
_ecore_timer_reschedule(Ecore_Timer *obj,
                        double       when)
{
   Ecore_Timer_Data *timer = eo_data_scope_get(obj, MY_CLASS);
   if ((timer->delete_me) || (timer->frozen)) return;

   timers = (Ecore_Timer_Data *)eina_inlist_remove(EINA_INLIST_GET(timers), EINA_INLIST_GET(timer));
   eo_data_unref(obj, timer);

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
     _ecore_timer_set(obj, when + timer->in, timer->in, timer->func, timer->data);
   else
     _ecore_timer_set(obj, timer->at + timer->in, timer->in, timer->func, timer->data);
}

/* assume that we hold the ecore lock when entering this function */
void
_ecore_timer_expired_timers_call(double when)
{
   /* call the first expired timer until no expired timers exist */
    while (_ecore_timer_expired_call(when)) ;
}

/* assume that we hold the ecore lock when entering this function */
int
_ecore_timer_expired_call(double when)
{
   if (!timers) return 0;
   if (last_check > when)
     {
        Ecore_Timer_Data *timer;
        /* User set time backwards */
        EINA_INLIST_FOREACH(timers, timer) timer->at -= (last_check - when);
     }
   last_check = when;

   if (!timer_current)
     {
        /* regular main loop, start from head */
        timer_current = timers;
     }
   else
     {
        /* recursive main loop, continue from where we were */
        Ecore_Timer_Data *timer_old = timer_current;
        timer_current = (Ecore_Timer_Data *)EINA_INLIST_GET(timer_current)->next;
        _ecore_timer_reschedule(timer_old->obj, when);
     }

   while (timer_current)
     {
        Ecore_Timer_Data *timer = timer_current;

        if (timer->at > when)
          {
             timer_current = NULL; /* ended walk, next should restart. */
             return 0;
          }

        if ((timer->just_added) || (timer->delete_me))
          {
             timer_current = (Ecore_Timer_Data *)EINA_INLIST_GET(timer_current)->next;
             continue;
          }

        timer->references++;
        if (!_ecore_call_task_cb(timer->func, timer->data))
          {
             if (!timer->delete_me) _ecore_timer_del(timer->obj);
          }
        timer->references--;

        if (timer_current) /* may have changed in recursive main loops */
          timer_current = (Ecore_Timer_Data *)EINA_INLIST_GET(timer_current)->next;

        _ecore_timer_reschedule(timer->obj, when);
     }
   return 0;
}

static void
_ecore_timer_set(Ecore_Timer  *obj,
                 double        at,
                 double        in,
                 Ecore_Task_Cb func,
                 void         *data)
{
   Ecore_Timer_Data *t2;

   Ecore_Timer_Data *timer = eo_data_ref(obj, MY_CLASS);

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
        EINA_INLIST_REVERSE_FOREACH(EINA_INLIST_GET(timers), t2)
          {
             if (timer->at > t2->at)
               {
                  timers = (Ecore_Timer_Data *)eina_inlist_append_relative(EINA_INLIST_GET(timers), EINA_INLIST_GET(timer), EINA_INLIST_GET(t2));
                  return;
               }
          }
     }
   timers = (Ecore_Timer_Data *)eina_inlist_prepend(EINA_INLIST_GET(timers), EINA_INLIST_GET(timer));
}

#ifdef WANT_ECORE_TIMER_DUMP
static int
_ecore_timer_cmp(const void *d1,
                 const void *d2)
{
   const Ecore_Timer_Data *t1 = d1;
   const Ecore_Timer_Data *t2 = d2;

   return (int)((t1->in - t2->in) * 100);
}
#endif

#include "ecore_timer.eo.c"

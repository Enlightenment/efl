#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include "Ecore.h"
#include "ecore_private.h"

static int inpurge = 0;

struct _Ecore_Event_Handler
{
   EINA_INLIST;
                          ECORE_MAGIC;
   int                    type;
   Ecore_Event_Handler_Cb func;
   void                  *data;
   int                    references;
   Eina_Bool              delete_me : 1;
};
GENERIC_ALLOC_SIZE_DECLARE(Ecore_Event_Handler);

struct _Ecore_Event_Filter
{
   EINA_INLIST;
                   ECORE_MAGIC;
   Ecore_Data_Cb   func_start;
   Ecore_Filter_Cb func_filter;
   Ecore_End_Cb    func_end;
   void           *loop_data;
   void           *data;
   int             references;
   Eina_Bool       delete_me : 1;
};
GENERIC_ALLOC_SIZE_DECLARE(Ecore_Event_Filter);

struct _Ecore_Event
{
   EINA_INLIST;
                ECORE_MAGIC;
   int          type;
   void        *event;
   Ecore_End_Cb func_free;
   void        *data;
   int          references;
   Eina_Bool    delete_me : 1;
};
GENERIC_ALLOC_SIZE_DECLARE(Ecore_Event);

typedef struct _Ecore_Future_Schedule_Entry
{
   Eina_Future_Schedule_Entry base;
   Eina_Future_Scheduler_Cb cb;
   Eina_Future *future;
   Ecore_Event *event;
   Eina_Value value;
} Ecore_Future_Schedule_Entry;

static int events_num = 0;
static Ecore_Event *events = NULL;
static Ecore_Event *event_current = NULL;
static Ecore_Event *purge_events = NULL;

static Ecore_Event_Handler **event_handlers = NULL;
static Ecore_Event_Handler *event_handler_current = NULL;
static int event_handlers_num = 0;
static int event_handlers_alloc_num = 0;
static Eina_List *event_handlers_delete_list = NULL;

static Ecore_Event_Handler *event_handlers_add_list = NULL;

static Ecore_Event_Filter *event_filters = NULL;
static Ecore_Event_Filter *event_filter_current = NULL;
static Ecore_Event *event_filter_event_current = NULL;
static int event_filters_delete_me = 0;
static int event_id_max = ECORE_EVENT_COUNT;
static int ecore_raw_event_type = ECORE_EVENT_NONE;
static void *ecore_raw_event_event = NULL;
static Ecore_Event_Handler *future_handler = NULL;
static int ECORE_EV_FUTURE_ID = -1;
static Eina_Mempool *mp_future_schedule_entry = NULL;
static Eina_Bool shutting_down = EINA_FALSE;

static void  _ecore_event_purge_deleted(void);
static void *_ecore_event_del(Ecore_Event *event);

EAPI Ecore_Event_Handler *
ecore_event_handler_add(int                    type,
                        Ecore_Event_Handler_Cb func,
                        const void            *data)
{
   Ecore_Event_Handler *eh = NULL;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);

   if (!func) return NULL;
   if ((type <= ECORE_EVENT_NONE) || (type >= event_id_max)) return NULL;
   eh = ecore_event_handler_calloc(1);
   if (!eh) return NULL;
   ECORE_MAGIC_SET(eh, ECORE_MAGIC_EVENT_HANDLER);
   eh->type = type;
   eh->func = func;
   eh->data = (void *)data;
   if (type >= (event_handlers_num - 1))
     {
        int p_alloc_num;

        p_alloc_num = event_handlers_alloc_num;
        event_handlers_num = type + 1;
        if (event_handlers_num > event_handlers_alloc_num)
          {
             Ecore_Event_Handler **new_handlers;
             int i;

             event_handlers_alloc_num = ((event_handlers_num + 16) / 16) * 16;
             new_handlers = realloc(event_handlers, event_handlers_alloc_num * sizeof(Ecore_Event_Handler *));
             if (!new_handlers)
               {
                  ecore_event_handler_mp_free(eh);
                  return NULL;
               }
             event_handlers = new_handlers;
             for (i = p_alloc_num; i < event_handlers_alloc_num; i++)
               event_handlers[i] = NULL;
          }
     }
   if (ecore_raw_event_type == type)
     event_handlers_add_list = (Ecore_Event_Handler *)eina_inlist_append(EINA_INLIST_GET(event_handlers_add_list), EINA_INLIST_GET(eh));
   else if (type < event_handlers_alloc_num)
     event_handlers[type] = (Ecore_Event_Handler *)eina_inlist_append(EINA_INLIST_GET(event_handlers[type]), EINA_INLIST_GET(eh));

   return eh;
}

EAPI void *
ecore_event_handler_del(Ecore_Event_Handler *event_handler)
{
   if (!event_handler) return NULL;
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   if (!ECORE_MAGIC_CHECK(event_handler, ECORE_MAGIC_EVENT_HANDLER))
     {
        ECORE_MAGIC_FAIL(event_handler, ECORE_MAGIC_EVENT_HANDLER,
                         "ecore_event_handler_del");
        return NULL;
     }
   return _ecore_event_handler_del(event_handler);
}

EAPI void *
ecore_event_handler_data_get(Ecore_Event_Handler *eh)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   if (!ECORE_MAGIC_CHECK(eh, ECORE_MAGIC_EVENT_HANDLER))
     {
        ECORE_MAGIC_FAIL(eh, ECORE_MAGIC_EVENT_HANDLER, "ecore_event_handler_data_get");
        return NULL;
     }
   return eh->data;
}

EAPI void *
ecore_event_handler_data_set(Ecore_Event_Handler *eh,
                             const void          *data)
{
   void *old = NULL;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   if (!ECORE_MAGIC_CHECK(eh, ECORE_MAGIC_EVENT_HANDLER))
     {
        ECORE_MAGIC_FAIL(eh, ECORE_MAGIC_EVENT_HANDLER, "ecore_event_handler_data_set");
        return NULL;
     }
   old = eh->data;
   eh->data = (void *)data;

   return old;
}

static void
_ecore_event_generic_free(void *data EINA_UNUSED,
                          void *event)
{ /* DO NOT MEMPOOL FREE THIS */
   free(event);
}

EAPI Ecore_Event *
ecore_event_add(int          type,
                void        *ev,
                Ecore_End_Cb func_free,
                void        *data)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);

   if (type <= ECORE_EVENT_NONE) return NULL;
   if (type >= event_id_max) return NULL;
   if ((ev) && (!func_free)) func_free = _ecore_event_generic_free;
   return _ecore_event_add(type, ev, func_free, data);
}

EAPI void *
ecore_event_del(Ecore_Event *event)
{
   if (!event) return NULL;
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   if (!ECORE_MAGIC_CHECK(event, ECORE_MAGIC_EVENT))
     {
        ECORE_MAGIC_FAIL(event, ECORE_MAGIC_EVENT, "ecore_event_del");
        return NULL;
     }
   EINA_SAFETY_ON_TRUE_RETURN_VAL(event->delete_me, NULL);
   event->delete_me = 1;
   return event->data;
}

EAPI int
ecore_event_type_new(void)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0);
   return event_id_max++;
}

EAPI Ecore_Event_Filter *
ecore_event_filter_add(Ecore_Data_Cb   func_start,
                       Ecore_Filter_Cb func_filter,
                       Ecore_End_Cb    func_end,
                       const void     *data)
{
   Ecore_Event_Filter *ef = NULL;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   if (!func_filter) return NULL;
   ef = ecore_event_filter_calloc(1);
   if (!ef) return NULL;
   ECORE_MAGIC_SET(ef, ECORE_MAGIC_EVENT_FILTER);
   ef->func_start = func_start;
   ef->func_filter = func_filter;
   ef->func_end = func_end;
   ef->data = (void *)data;
   event_filters = (Ecore_Event_Filter *)eina_inlist_append(EINA_INLIST_GET(event_filters), EINA_INLIST_GET(ef));

   return ef;
}

EAPI void *
ecore_event_filter_del(Ecore_Event_Filter *ef)
{
   if (!ef) return NULL;
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   if (!ECORE_MAGIC_CHECK(ef, ECORE_MAGIC_EVENT_FILTER))
     {
        ECORE_MAGIC_FAIL(ef, ECORE_MAGIC_EVENT_FILTER, "ecore_event_filter_del");
        return NULL;
     }
   EINA_SAFETY_ON_TRUE_RETURN_VAL(ef->delete_me, NULL);
   ef->delete_me = 1;
   event_filters_delete_me = 1;
   return ef->data;
}

EAPI int
ecore_event_current_type_get(void)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0);
   return ecore_raw_event_type;
}

EAPI void *
ecore_event_current_event_get(void)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   return ecore_raw_event_event;
}

EAPI void *
_ecore_event_handler_del(Ecore_Event_Handler *event_handler)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(event_handler->delete_me, NULL);
   event_handler->delete_me = 1;
   event_handlers_delete_list = eina_list_append(event_handlers_delete_list, event_handler);
   return event_handler->data;
}

static Eina_Bool
ecore_future_dispatched(void *data EINA_UNUSED, int type  EINA_UNUSED, void *event)
{
   Ecore_Future_Schedule_Entry *entry = event;
   entry->event = NULL;
   entry->cb(entry->future, entry->value);
   return EINA_FALSE;
}

static void
ecore_future_free(void *user_data, void *func_data EINA_UNUSED)
{
   Ecore_Future_Schedule_Entry *entry = user_data;
   /*
     In case entry->event is not NULL, it means
     that ecore is shutting down. In this case,
     we must cancel the future otherwise Eina may
     try to use it and lead to crashes.
    */
   if (entry->event)
     {
        eina_future_cancel(entry->future);
        eina_value_flush(&entry->value);
     }
   eina_mempool_free(mp_future_schedule_entry, entry);
}

static Eina_Future_Schedule_Entry *
ecore_future_schedule(Eina_Future_Scheduler *sched, Eina_Future_Scheduler_Cb cb, Eina_Future *future, Eina_Value value)
{
   Ecore_Future_Schedule_Entry *entry = eina_mempool_malloc(mp_future_schedule_entry,
                                                            sizeof(Ecore_Future_Schedule_Entry));
   EINA_SAFETY_ON_NULL_RETURN_VAL(entry, NULL);
   entry->base.scheduler = sched;
   entry->cb = cb;
   entry->future = future;
   entry->value = value;
   entry->event = ecore_event_add(ECORE_EV_FUTURE_ID, entry, ecore_future_free, entry);
   EINA_SAFETY_ON_NULL_GOTO(entry->event, err);
   return &entry->base;

  err:
   eina_mempool_free(mp_future_schedule_entry, entry);
   return NULL;
}

static void
ecore_future_recall(Eina_Future_Schedule_Entry *s_entry)
{
   if (shutting_down) return;
   Ecore_Future_Schedule_Entry *entry = (Ecore_Future_Schedule_Entry *)s_entry;
   EINA_SAFETY_ON_NULL_RETURN(entry->event);
   ecore_event_del(entry->event);
   eina_value_flush(&entry->value);
   entry->event = NULL;
}

static Eina_Future_Scheduler ecore_future_scheduler = {
   .schedule = ecore_future_schedule,
   .recall = ecore_future_recall,
};

Eina_Future_Scheduler *
_ecore_event_future_scheduler_get(void)
{
   return &ecore_future_scheduler;
}

Eina_Bool
_ecore_event_init(void)
{
   const char *choice = getenv("EINA_MEMPOOL");
   if ((!choice) || (!choice[0])) choice = "chained_mempool";

   shutting_down = EINA_FALSE;
   ECORE_EV_FUTURE_ID = ecore_event_type_new();
   future_handler = ecore_event_handler_add(ECORE_EV_FUTURE_ID, ecore_future_dispatched, NULL);
   EINA_SAFETY_ON_NULL_GOTO(future_handler, err_handler);
   //FIXME: Is 512 too high?
   mp_future_schedule_entry = eina_mempool_add(choice, "Ecore_Future_Event",
                                               NULL, sizeof(Ecore_Future_Schedule_Entry),
                                               512);
   EINA_SAFETY_ON_NULL_GOTO(mp_future_schedule_entry, err_pool);

   return EINA_TRUE;

 err_pool:
   ecore_event_handler_del(future_handler);
   future_handler = NULL;
 err_handler:
   ECORE_EV_FUTURE_ID = -1;
   return EINA_FALSE;
}

void
_ecore_event_shutdown(void)
{
   int i;
   Ecore_Event_Handler *eh;
   Ecore_Event_Filter *ef;

   shutting_down = EINA_TRUE;
   ecore_event_handler_del(future_handler);
   future_handler = NULL;
   while (events) _ecore_event_del(events);
   event_current = NULL;
   for (i = 0; i < event_handlers_num; i++)
     {
        while ((eh = event_handlers[i]))
          {
             event_handlers[i] = (Ecore_Event_Handler *)eina_inlist_remove(EINA_INLIST_GET(event_handlers[i]), EINA_INLIST_GET(event_handlers[i]));
             ECORE_MAGIC_SET(eh, ECORE_MAGIC_NONE);
             if (!eh->delete_me) ecore_event_handler_mp_free(eh);
          }
     }
   EINA_LIST_FREE(event_handlers_delete_list, eh)
     ecore_event_handler_mp_free(eh);
   if (event_handlers) free(event_handlers);
   event_handlers = NULL;
   event_handlers_num = 0;
   event_handlers_alloc_num = 0;
   while ((ef = event_filters))
     {
        event_filters = (Ecore_Event_Filter *)eina_inlist_remove(EINA_INLIST_GET(event_filters), EINA_INLIST_GET(event_filters));
        ECORE_MAGIC_SET(ef, ECORE_MAGIC_NONE);
        ecore_event_filter_mp_free(ef);
     }
   event_filters_delete_me = 0;
   event_filter_current = NULL;
   event_filter_event_current = NULL;
   ECORE_EV_FUTURE_ID = -1;
}

int
_ecore_event_exist(void)
{
   Ecore_Event *e;
   EINA_INLIST_FOREACH(events, e)
     if (!e->delete_me) return 1;
   return 0;
}

Ecore_Event *
_ecore_event_add(int          type,
                 void        *ev,
                 Ecore_End_Cb func_free,
                 void        *data)
{
   Ecore_Event *e;

   e = ecore_event_calloc(1);
   if (!e) return NULL;
   ECORE_MAGIC_SET(e, ECORE_MAGIC_EVENT);
   e->type = type;
   e->event = ev;
   e->func_free = func_free;
   e->data = data;
   if (inpurge > 0)
     {
        purge_events = (Ecore_Event *)eina_inlist_append(EINA_INLIST_GET(purge_events), EINA_INLIST_GET(e));
        events_num++;
     }
   else
     {
        events = (Ecore_Event *)eina_inlist_append(EINA_INLIST_GET(events), EINA_INLIST_GET(e));
        events_num++;
     }
   return e;
}

void *
_ecore_event_del(Ecore_Event *event)
{
   void *data;

   data = event->data;
   if (event->func_free) _ecore_call_end_cb(event->func_free, event->data, event->event);
   events = (Ecore_Event *)eina_inlist_remove(EINA_INLIST_GET(events), EINA_INLIST_GET(event));
   ECORE_MAGIC_SET(event, ECORE_MAGIC_NONE);
   ecore_event_mp_free(event);
   events_num--;
   return data;
}

static void
_ecore_event_purge_deleted(void)
{
   Ecore_Event *itr = events;

   inpurge++;
   while (itr)
     {
        Ecore_Event *next = (Ecore_Event *)EINA_INLIST_GET(itr)->next;
        if ((!itr->references) && (itr->delete_me))
          _ecore_event_del(itr);
        itr = next;
     }
   inpurge--;
   while (purge_events)
     {
        Ecore_Event *e = purge_events;
        purge_events = (Ecore_Event *)eina_inlist_remove(EINA_INLIST_GET(purge_events), EINA_INLIST_GET(purge_events));
        events = (Ecore_Event *)eina_inlist_append(EINA_INLIST_GET(events), EINA_INLIST_GET(e));
     }
}

static inline void
_ecore_event_filters_apply(void)
{
   if (!event_filter_current)
     {
        /* regular main loop, start from head */
         event_filter_current = event_filters;
     }
   else
     {
        /* recursive main loop, continue from where we were */
         event_filter_current = (Ecore_Event_Filter *)EINA_INLIST_GET(event_filter_current)->next;
     }
   if ((!event_filter_current) && (!event_filters_delete_me) && (!purge_events)) return;
   eina_evlog("+event_filter", NULL, 0.0, NULL);
   while (event_filter_current)
     {
        Ecore_Event_Filter *ef = event_filter_current;

        if (!ef->delete_me)
          {
             ef->references++;

             if (ef->func_start)
               ef->loop_data = _ecore_call_data_cb(ef->func_start, ef->data);

             if (!event_filter_event_current)
               {
     /* regular main loop, start from head */
                   event_filter_event_current = events;
               }
             else
               {
     /* recursive main loop, continue from where we were */
                   event_filter_event_current = (Ecore_Event *)EINA_INLIST_GET(event_filter_event_current)->next;
               }

             while (event_filter_event_current)
               {
                  Ecore_Event *e = event_filter_event_current;

                  if (!_ecore_call_filter_cb(ef->func_filter, ef->data,
                                             ef->loop_data, e->type, e->event))
                    {
                       ecore_event_del(e);
                    }

                  if (event_filter_event_current) /* may have changed in recursive main loops */
                    event_filter_event_current = (Ecore_Event *)EINA_INLIST_GET(event_filter_event_current)->next;
               }
             if (ef->func_end)
               _ecore_call_end_cb(ef->func_end, ef->data, ef->loop_data);

             ef->references--;
          }

        if (event_filter_current) /* may have changed in recursive main loops */
          event_filter_current = (Ecore_Event_Filter *)EINA_INLIST_GET(event_filter_current)->next;
     }
   eina_evlog("-event_filter", NULL, 0.0, NULL);
   if (event_filters_delete_me)
     {
        int deleted_in_use = 0;
        Ecore_Event_Filter *l;
        for (l = event_filters; l; )
          {
             Ecore_Event_Filter *ef = l;
             l = (Ecore_Event_Filter *)EINA_INLIST_GET(l)->next;
             if (ef->delete_me)
               {
                  if (ef->references)
                    {
                       deleted_in_use++;
                       continue;
                    }

                  event_filters = (Ecore_Event_Filter *)eina_inlist_remove(EINA_INLIST_GET(event_filters), EINA_INLIST_GET(ef));
                  ECORE_MAGIC_SET(ef, ECORE_MAGIC_NONE);
                  ecore_event_filter_mp_free(ef);
               }
          }
        if (!deleted_in_use)
          event_filters_delete_me = 0;
     }
}

void
_ecore_event_call(void)
{
   Eina_List *l, *l_next;
   Ecore_Event_Handler *eh;

   _ecore_event_filters_apply();

   if (!event_current)
     {
        /* regular main loop, start from head */
         event_current = events;
         event_handler_current = NULL;
     }
   if ((!event_current) && (!event_handlers_delete_list)) return;
   eina_evlog("+events", NULL, 0.0, NULL);
   while (event_current)
     {
        Ecore_Event *e = event_current;
        int handle_count = 0;

        if (e->delete_me)
          {
             event_current = (Ecore_Event *)EINA_INLIST_GET(event_current)->next;
             continue;
          }

        ecore_raw_event_type = e->type;
        ecore_raw_event_event = e->event;
        e->references++;
        if ((e->type >= 0) && (e->type < event_handlers_num))
          {
             if (!event_handler_current)
               {
     /* regular main loop, start from head */
                   event_handler_current = event_handlers[e->type];
               }
             else
               {
     /* recursive main loop, continue from where we were */
                   event_handler_current = (Ecore_Event_Handler *)EINA_INLIST_GET(event_handler_current)->next;
               }

             while ((event_handler_current) && (!e->delete_me))
               {
                  eh = event_handler_current;
                  if (!eh->delete_me)
                    {
                       Eina_Bool ret;

                       handle_count++;

                       eh->references++;
                       ret = _ecore_call_handler_cb(eh->func, eh->data, e->type, e->event);
                       eh->references--;

                       if (!ret)
                         {
                            event_handler_current = NULL;
                            break;  /* 0 == "call no further handlers" */
                         }
                    }

                  if (event_handler_current) /* may have changed in recursive main loops */
                    event_handler_current = (Ecore_Event_Handler *)EINA_INLIST_GET(event_handler_current)->next;
               }
          }
        while (event_handlers_add_list)
          {
             eh = event_handlers_add_list;
             event_handlers_add_list = (Ecore_Event_Handler *)eina_inlist_remove(EINA_INLIST_GET(event_handlers_add_list), EINA_INLIST_GET(eh));
             event_handlers[eh->type] = (Ecore_Event_Handler *)eina_inlist_append(EINA_INLIST_GET(event_handlers[eh->type]), EINA_INLIST_GET(eh));
          }
        /* if no handlers were set for EXIT signal - then default is */
        /* to quit the main loop */
        if ((e->type == ECORE_EVENT_SIGNAL_EXIT) && (handle_count == 0))
          ecore_main_loop_quit();
        e->references--;
        e->delete_me = 1;

        if (event_current) /* may have changed in recursive main loops */
          event_current = (Ecore_Event *)EINA_INLIST_GET(event_current)->next;
     }
   eina_evlog("-events", NULL, 0.0, NULL);

   ecore_raw_event_type = ECORE_EVENT_NONE;
   ecore_raw_event_event = NULL;

   _ecore_event_purge_deleted();

   EINA_LIST_FOREACH_SAFE(event_handlers_delete_list, l, l_next, eh)
     {
        if (eh->references) continue;

        event_handlers_delete_list = eina_list_remove_list(event_handlers_delete_list, l);

        event_handlers[eh->type] = (Ecore_Event_Handler *)eina_inlist_remove(EINA_INLIST_GET(event_handlers[eh->type]), EINA_INLIST_GET(eh));
        ECORE_MAGIC_SET(eh, ECORE_MAGIC_NONE);
        ecore_event_handler_mp_free(eh);
     }
}

void *
_ecore_event_signal_user_new(void)
{
   Ecore_Event_Signal_User *e;

   e = calloc(1, sizeof(Ecore_Event_Signal_User));
   return e;
}

void *
_ecore_event_signal_hup_new(void)
{
   Ecore_Event_Signal_Hup *e;

   e = calloc(1, sizeof(Ecore_Event_Signal_Hup));
   return e;
}

void *
_ecore_event_signal_exit_new(void)
{
   Ecore_Event_Signal_Exit *e;

   e = calloc(1, sizeof(Ecore_Event_Signal_Exit));
   return e;
}

void *
_ecore_event_signal_power_new(void)
{
   Ecore_Event_Signal_Power *e;

   e = calloc(1, sizeof(Ecore_Event_Signal_Power));
   return e;
}

void *
_ecore_event_signal_realtime_new(void)
{
   return calloc(1, sizeof(Ecore_Event_Signal_Realtime));
}

EAPI void
ecore_event_type_flush_internal(int type, ...)
{
   Eina_Inarray types;
   Ecore_Event *event;
   Eina_Inlist *l;
   int *itr;
   va_list args;
   Eina_Bool wrong_type = EINA_FALSE;

   // In case of an empty list of event
   if (type == ECORE_EVENT_NONE) return;

   eina_inarray_step_set(&types, sizeof (Eina_Inarray), sizeof (int), 4);

   eina_inarray_push(&types, &type);

   va_start(args, type);
   do
     {
        type = va_arg(args, int);
        if (type == ECORE_EVENT_NONE) break ;
        eina_inarray_push(&types, &type);
     }
   while (1);
   va_end(args);

   EINA_INARRAY_FOREACH(&types, itr)
     {
        if (*itr >= 0 && *itr < event_id_max) continue;

        ERR("Invalid event flush requested: %i", *itr);
        wrong_type = EINA_TRUE;
     }

   if (wrong_type)
     {
        eina_inarray_flush(&types);
        return ;
     }

   EINA_INLIST_FOREACH_SAFE((Eina_Inlist *) events, l, event)
     {
        Eina_Bool found = EINA_FALSE;

        EINA_INARRAY_FOREACH(&types, itr)
          if (event->type == *itr)
            found = EINA_TRUE;
        if (!found) continue ;

        if (event->delete_me) continue ;
        event->delete_me = 1;
     }

   _ecore_event_purge_deleted();

   eina_inarray_flush(&types);
}

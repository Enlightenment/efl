#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include "Ecore.h"
#include "ecore_private.h"

static Ecore_Event_Message_Handler *_event_msg_handler = NULL;

EAPI Ecore_Event_Handler *
ecore_event_handler_add(int                    type,
                        Ecore_Event_Handler_Cb func,
                        const void            *data)
{
   return ecore_event_message_handler_add(_event_msg_handler,
                                          type, func, (void *)data);
}

EAPI Ecore_Event_Handler *
ecore_event_handler_prepend(int                    type,
                        Ecore_Event_Handler_Cb func,
                        const void            *data)
{
   return ecore_event_message_handler_prepend(_event_msg_handler,
                                          type, func, (void *)data);
}

EAPI void *
ecore_event_handler_del(Ecore_Event_Handler *event_handler)
{
   return ecore_event_message_handler_del(_event_msg_handler,
                                          event_handler);
}

EAPI void *
ecore_event_handler_data_get(Ecore_Event_Handler *eh)
{
   return ecore_event_message_handler_data_get(_event_msg_handler, eh);
}

EAPI void *
ecore_event_handler_data_set(Ecore_Event_Handler *eh,
                             const void          *data)
{
   return ecore_event_message_handler_data_set(_event_msg_handler, eh,
                                               (void *)data);
}

EAPI Ecore_Event *
ecore_event_add(int          type,
                void        *ev,
                Ecore_End_Cb func_free,
                void        *data)
{
   Ecore_Event_Message *msg;
   if (type <= ECORE_EVENT_NONE) return NULL;

   msg = ecore_event_message_handler_message_type_add(_event_msg_handler);
   if (msg)
     {
        ecore_event_message_data_set(msg, type, ev, func_free, data);
        efl_loop_message_handler_message_send(_event_msg_handler, msg);
     }
   return (Ecore_Event *)msg;
}

EAPI void *
ecore_event_del(Ecore_Event *event)
{
   void *data = NULL;
   if (!event) return data;
   ecore_event_message_data_get((Eo *)event, NULL, NULL, NULL, &data);
   _efl_loop_message_unsend((Eo *)event);
   return data;
}

EAPI int
ecore_event_type_new(void)
{
   return ecore_event_message_handler_type_new(_event_msg_handler);
}

EAPI Ecore_Event_Filter *
ecore_event_filter_add(Ecore_Data_Cb   func_start,
                       Ecore_Filter_Cb func_filter,
                       Ecore_End_Cb    func_end,
                       const void     *data)
{
   return ecore_event_message_handler_filter_add(_event_msg_handler,
                                                 func_start, func_filter,
                                                 func_end, (void *)data);
}

EAPI void *
ecore_event_filter_del(Ecore_Event_Filter *ef)
{
   return ecore_event_message_handler_filter_del(_event_msg_handler, ef);
}

EAPI int
ecore_event_current_type_get(void)
{
   return ecore_event_message_handler_current_type_get(_event_msg_handler);
}

EAPI void *
ecore_event_current_event_get(void)
{
   return ecore_event_message_handler_current_event_get(_event_msg_handler);
}

Eina_Bool
_ecore_event_init(void)
{
   const char *choice = getenv("EINA_MEMPOOL");
   if ((!choice) || (!choice[0])) choice = "chained_mempool";

   _event_msg_handler = efl_add(ECORE_EVENT_MESSAGE_HANDLER_CLASS, _mainloop_singleton);
   efl_provider_register(_mainloop_singleton, ECORE_EVENT_MESSAGE_HANDLER_CLASS, _event_msg_handler);

   if (!_event_msg_handler)
     {
        ERR("Cannot create legacy ecore event message handler");
        return EINA_FALSE;
     }
   // init some core legacy event types in t he same order and numbering as before
   // ECORE_EVENT_NONE                     0
   // no need to do as ev types start at 1

   // ECORE_EVENT_SIGNAL_USER              1
   ecore_event_message_handler_type_new(_event_msg_handler);
   // ECORE_EVENT_SIGNAL_HUP               2
   ecore_event_message_handler_type_new(_event_msg_handler);
   // ECORE_EVENT_SIGNAL_EXIT              3
   ecore_event_message_handler_type_new(_event_msg_handler);
   // ECORE_EVENT_SIGNAL_POWER             4
   ecore_event_message_handler_type_new(_event_msg_handler);
   // ECORE_EVENT_SIGNAL_REALTIME          5
   ecore_event_message_handler_type_new(_event_msg_handler);
   // ECORE_EVENT_MEMORY_STATE             6
   ecore_event_message_handler_type_new(_event_msg_handler);
   // ECORE_EVENT_POWER_STATE              7
   ecore_event_message_handler_type_new(_event_msg_handler);
   // ECORE_EVENT_LOCALE_CHANGED           8
   ecore_event_message_handler_type_new(_event_msg_handler);
   // ECORE_EVENT_HOSTNAME_CHANGED         9
   ecore_event_message_handler_type_new(_event_msg_handler);
   // ECORE_EVENT_SYSTEM_TIMEDATE_CHANGED  10
   ecore_event_message_handler_type_new(_event_msg_handler);
   // ECORE_EVENT_COUNT                    11
   // no need to do as it was a count, nto an event

   return EINA_TRUE;
}

void
_ecore_event_shutdown(void)
{
   efl_loop_message_handler_message_clear(_event_msg_handler);
   _event_msg_handler = NULL;
}

void *
_ecore_event_signal_user_new(void)
{
   return calloc(1, sizeof(Ecore_Event_Signal_User));
}

void *
_ecore_event_signal_hup_new(void)
{
   return calloc(1, sizeof(Ecore_Event_Signal_Hup));
}

void *
_ecore_event_signal_exit_new(void)
{
   return calloc(1, sizeof(Ecore_Event_Signal_Exit));
}

void *
_ecore_event_signal_power_new(void)
{
   return calloc(1, sizeof(Ecore_Event_Signal_Power));
}

void *
_ecore_event_signal_realtime_new(void)
{
   return calloc(1, sizeof(Ecore_Event_Signal_Realtime));
}

EAPI void
ecore_event_type_flush_internal(int type, ...)
{
   va_list args;

   if (type == ECORE_EVENT_NONE) return;
   ecore_event_message_handler_type_flush(_event_msg_handler, type);

   va_start(args, type);
   for (;;)
     {
        type = va_arg(args, int);
        if (type == ECORE_EVENT_NONE) break;
        ecore_event_message_handler_type_flush(_event_msg_handler, type);
     }
   va_end(args);
}

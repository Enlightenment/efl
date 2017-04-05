#ifndef ELM_INTERFACE_ATSPI_WINDOW_H
#define ELM_INTERFACE_ATSPI_WINDOW_H

#ifdef EFL_BETA_API_SUPPORT
#ifdef EFL_EO_API_SUPPORT

/**
 * Emits ATSPI 'Window:Activated' event.
 */
#define elm_interface_atspi_window_activated_signal_emit(obj) \
   efl_event_callback_call(obj, ELM_INTERFACE_ATSPI_WINDOW_EVENT_WINDOW_ACTIVATED, NULL);

/**
 * Emits ATSPI 'Window:Deactivated' event.
 */
#define elm_interface_atspi_window_deactivated_signal_emit(obj) \
   efl_event_callback_call(obj, ELM_INTERFACE_ATSPI_WINDOW_EVENT_WINDOW_DEACTIVATED, NULL);

/**
 * Emits ATSPI 'Window:Created' event.
 */
#define elm_interface_atspi_window_created_signal_emit(obj) \
   efl_event_callback_call(obj, ELM_INTERFACE_ATSPI_WINDOW_EVENT_WINDOW_CREATED, NULL);

/**
 * Emits ATSPI 'Window:Destroyed' event.
 */
#define elm_interface_atspi_window_destroyed_signal_emit(obj) \
   efl_event_callback_call(obj, ELM_INTERFACE_ATSPI_WINDOW_EVENT_WINDOW_DESTROYED, NULL);

/**
 * Emits ATSPI 'Window:Maximized' event.
 */
#define elm_interface_atspi_window_maximized_signal_emit(obj) \
   efl_event_callback_call(obj, ELM_INTERFACE_ATSPI_WINDOW_EVENT_WINDOW_MAXIMIZED, NULL);

/**
 * Emits ATSPI 'Window:Minimized' event.
 */
#define elm_interface_atspi_window_minimized_signal_emit(obj) \
   efl_event_callback_call(obj, ELM_INTERFACE_ATSPI_WINDOW_EVENT_WINDOW_MINIMIZED, NULL);

/**
 * Emits ATSPI 'Window:Restored' event.
 */
#define elm_interface_atspi_window_restored_signal_emit(obj) \
   efl_event_callback_call(obj, ELM_INTERFACE_ATSPI_WINDOW_EVENT_WINDOW_RESTORED, NULL);

#include "elm_interface_atspi_window.eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_interface_atspi_window.eo.legacy.h"
#endif

#endif
#endif

#ifndef ELM_INTERFACE_ATSPI_WINDOW_H
#define ELM_INTERFACE_ATSPI_WINDOW_H

#ifdef EFL_BETA_API_SUPPORT
#ifdef EFL_EO_API_SUPPORT

/**
 * Emits ATSPI 'Window:Activated' dbus signal.
 */
#define elm_interface_atspi_window_activated_signal_emit(obj) \
   eo_do(ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN, elm_interface_atspi_accessible_event_emit(obj, ELM_INTERFACE_ATSPI_WINDOW_EVENT_WINDOW_ACTIVATED, NULL));

/**
 * Emits ATSPI 'Window:Deactivated' dbus signal.
 */
#define elm_interface_atspi_window_deactivated_signal_emit(obj) \
   eo_do(ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN, elm_interface_atspi_accessible_event_emit(obj, ELM_INTERFACE_ATSPI_WINDOW_EVENT_WINDOW_DEACTIVATED, NULL));

/**
 * Emits ATSPI 'Window:Created' dbus signal.
 */
#define elm_interface_atspi_window_created_signal_emit(obj) \
   eo_do(ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN, elm_interface_atspi_accessible_event_emit(obj, ELM_INTERFACE_ATSPI_WINDOW_EVENT_WINDOW_CREATED, NULL));

/**
 * Emits ATSPI 'Window:Destroyed' dbus signal.
 */
#define elm_interface_atspi_window_destroyed_signal_emit(obj) \
   eo_do(ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN, elm_interface_atspi_accessible_event_emit(obj, ELM_INTERFACE_ATSPI_WINDOW_EVENT_WINDOW_DESTROYED, NULL));

/**
 * Emits ATSPI 'Window:Maximized' dbus signal.
 */
#define elm_interface_atspi_window_maximized_signal_emit(obj) \
   eo_do(ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN, elm_interface_atspi_accessible_event_emit(obj, ELM_INTERFACE_ATSPI_WINDOW_EVENT_WINDOW_MAXIMIZED, NULL));

/**
 * Emits ATSPI 'Window:Minimized' dbus signal.
 */
#define elm_interface_atspi_window_minimized_signal_emit(obj) \
   eo_do(ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN, elm_interface_atspi_accessible_event_emit(obj, ELM_INTERFACE_ATSPI_WINDOW_EVENT_WINDOW_MINIMIZED, NULL));

/**
 * Emits ATSPI 'Window:Restored' dbus signal.
 */
#define elm_interface_atspi_window_restored_signal_emit(obj) \
   eo_do(ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN, elm_interface_atspi_accessible_event_emit(obj, ELM_INTERFACE_ATSPI_WINDOW_EVENT_WINDOW_RESTORED, NULL));

#include "elm_interface_atspi_window.eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_interface_atspi_window.eo.legacy.h"
#endif

#endif
#endif

#ifndef ELM_SYS_NOTIFY_H
#define ELM_SYS_NOTIFY_H

typedef void (*Elm_Sys_Notify_Send_Cb)(void *data, unsigned int id);

#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_sys_notify_interface_eo.legacy.h"
#include "elm_sys_notify_eo.legacy.h"
#endif

/**
 * Emitted when the signal NotificationClosed is received.
 * @since 1.8
 */
EAPI extern int ELM_EVENT_SYS_NOTIFY_NOTIFICATION_CLOSED;

/**
 * Emitted when the signal ActionInvoked is received.
 * @since 1.8
 */
EAPI extern int ELM_EVENT_SYS_NOTIFY_ACTION_INVOKED; /**< A Action has been invoked. */


/**
 * @def elm_sys_notify_simple_send
 *
 * Create a new notification just with Icon, Body and Summary.
 *
 * @param[in] icon
 * @param[in] summary
 * @param[in] body
 *
 * @see elm_sys_notify_send()
 *
 * @since 1.8
 */
#define elm_sys_notify_simple_send(icon, summary, body)    \
        elm_sys_notify_send(0, icon, summary, body,        \
                            ELM_SYS_NOTIFY_URGENCY_NORMAL, \
                            -1, NULL, NULL)

/**
 * Sends a notification to the notification servers that have
 * been registered by elm_need_sys_notify() or
 * elm_sys_notify_servers_set().
 *
 * @param replaces_id Notification ID that this notification replaces.
 * The value 0 means a new notification.
 * @param icon The optional program icon of the calling application.
 * @param summary The summary text briefly describing the notification.
 * @param body The optional detailed body text. Can be empty.
 * @param urgency The urgency level.
 * @param timeout Timeout display in milliseconds.
 * @param cb Callback used to retrieve the notification id
 * return by the Notification Server.
 * @param cb_data Optional context data
 *
 * @since 1.8
 */
EAPI void      elm_sys_notify_send(unsigned int replaces_id,
                                   const char *icon,
                                   const char *summary,
                                   const char *body,
                                   Elm_Sys_Notify_Urgency urgency,
                                   int timeout,
                                   Elm_Sys_Notify_Send_Cb cb,
                                   const void *cb_data);

/**
 * Causes a notification to be forcefully closed and removed from the user's
 * view. It can be used, for example, in the event that what the notification
 * pertains to is no longer relevant, or to cancel a notification * with no
 * expiration time.
 *
 * @param id Notification id
 *
 * @note If the notification no longer exists,
 * an empty D-BUS Error message is sent back.
 *
 * @since 1.8
 */
EAPI void      elm_sys_notify_close(unsigned int id);

#endif


#ifndef ELM_SYS_NOTIFY_H
#define ELM_SYS_NOTIFY_H

/**
 * The reason the notification was closed
 *
 * @since 1.8
 */
typedef enum _Elm_Sys_Notify_Closed_Reason
{
  ELM_SYS_NOTIFY_CLOSED_EXPIRED, /** The notification expired. */
  ELM_SYS_NOTIFY_CLOSED_DISMISSED, /** The notification was dismissed by the user. */
  ELM_SYS_NOTIFY_CLOSED_REQUESTED, /** The notification was closed by a call to CloseNotification method. */
  ELM_SYS_NOTIFY_CLOSED_UNDEFINED /** Undefined/reserved reasons. */
} Elm_Sys_Notify_Closed_Reason;

/**
 * Urgency levels of a notification
 *
 * @see elm_sys_notify_send()
 *
 * @since 1.8
 */
typedef enum _Elm_Sys_Notify_Urgency
{
  ELM_SYS_NOTIFY_URGENCY_LOW,
  ELM_SYS_NOTIFY_URGENCY_NORMAL,
  ELM_SYS_NOTIFY_URGENCY_CRITICAL
} Elm_Sys_Notify_Urgency;

typedef void (*Elm_Sys_Notify_Send_Cb)(void *data, unsigned int id);

/**
 * Emitted when the signal NotificationClosed is received.
 */
EAPI extern int ELM_EVENT_SYS_NOTIFY_NOTIFICATION_CLOSED;

/**
 * Emitted when the signal ActionInvoked is received.
 */
EAPI extern int ELM_EVENT_SYS_NOTIFY_ACTION_INVOKED; /**< A Action has been invoked. */

/**
 * Data on event when Notification Closed is emitted.
 *
 * @since 1.8
 */
typedef struct _Elm_Sys_Notify_Notification_Closed
{
   unsigned int id; /**< ID of the notification. */
   Elm_Sys_Notify_Closed_Reason reason; /**< The Reason the notification was closed. */
} Elm_Sys_Notify_Notification_Closed;


/**
 * Data on event when Action Invoked is emitted.
 *
 * @since 1.8
 */
typedef struct _Elm_Sys_Notify_Action_Invoked
{
   unsigned int id; /**< ID of the notification. */
   char *action_key; /**< The key of the action invoked. These match the keys sent over in the list of actions. */
} Elm_Sys_Notify_Action_Invoked;

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

/**
 * Sends a notification to the notification server.
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
#endif

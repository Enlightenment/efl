#ifndef _ELM_SYS_NOTIFY_INTERFACE_EO_LEGACY_H_
#define _ELM_SYS_NOTIFY_INTERFACE_EO_LEGACY_H_

#ifndef _ELM_SYS_NOTIFY_INTERFACE_EO_CLASS_TYPE
#define _ELM_SYS_NOTIFY_INTERFACE_EO_CLASS_TYPE

typedef Eo Elm_Sys_Notify_Interface;

#endif

#ifndef _ELM_SYS_NOTIFY_INTERFACE_EO_TYPES
#define _ELM_SYS_NOTIFY_INTERFACE_EO_TYPES

/** Urgency levels of a notification
 *
 * @since 1.8
 *
 * @ingroup Elm_Sys_Notify
 */
typedef enum
{
  ELM_SYS_NOTIFY_URGENCY_LOW = 0, /**< Low urgency */
  ELM_SYS_NOTIFY_URGENCY_NORMAL, /**< Normal urgency */
  ELM_SYS_NOTIFY_URGENCY_CRITICAL /**< Critical urgency */
} Elm_Sys_Notify_Urgency;


#endif

/**
 * @brief Causes a notification to be forcefully closed and removed from the
 * user's view. It can be used, for example, in the event that what the
 * notification pertains to is no longer relevant, or to cancel a notification
 * with no expiration time.
 *
 * @param[in] obj The object.
 * @param[in] replaces_id Notification ID that this notification replaces. The
 * value 0 means a new notification.
 * @param[in] icon The optional program icon of the calling application
 * @param[in] summary The summary text briefly describing the notification
 * @param[in] body The optional detailed body text. Can be empty
 * @param[in] urgency The urgency level
 * @param[in] timeout Timeout display in milliseconds
 * @param[in] cb Callback used to retrieve the notification id returned by the
 * Notification Server
 * @param[in] cb_data Optional context data
 *
 * @since 1.8
 *
 * @ingroup Elm_Sys_Notify_Interface_Group
 */
EAPI void elm_sys_notify_interface_send(const Elm_Sys_Notify_Interface *obj, unsigned int replaces_id, const char *icon, const char *summary, const char *body, Elm_Sys_Notify_Urgency urgency, int timeout, Elm_Sys_Notify_Send_Cb cb, const void *cb_data);

/**
 * @brief Create a new notification just with Icon, Body and Summary. It is a
 * helper that wraps the send method
 *
 * @param[in] obj The object.
 * @param[in] icon The optional program icon of the calling application
 * @param[in] summary The summary text briefly describing the notification
 * @param[in] body The optional detailed body text. Can be empty
 *
 * @since 1.16
 *
 * @ingroup Elm_Sys_Notify_Interface_Group
 */
EAPI void elm_sys_notify_interface_simple_send(const Elm_Sys_Notify_Interface *obj, const char *icon, const char *summary, const char *body);

/**
 * @brief Causes a notification to be forcefully closed and removed from the
 * user's view. It can be used, for example, in the event that what the
 * notification  pertains to is no longer relevant, or to cancel a notification
 * with no expiration time.
 *
 * @param[in] obj The object.
 * @param[in] id Notification ID
 *
 * @since 1.8
 *
 * @ingroup Elm_Sys_Notify_Interface_Group
 */
EAPI void elm_sys_notify_interface_close(const Elm_Sys_Notify_Interface *obj, unsigned int id);

#endif

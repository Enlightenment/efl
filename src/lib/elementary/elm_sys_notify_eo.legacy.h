#ifndef _ELM_SYS_NOTIFY_EO_LEGACY_H_
#define _ELM_SYS_NOTIFY_EO_LEGACY_H_

#ifndef _ELM_SYS_NOTIFY_EO_CLASS_TYPE
#define _ELM_SYS_NOTIFY_EO_CLASS_TYPE

typedef Eo Elm_Sys_Notify;

#endif

#ifndef _ELM_SYS_NOTIFY_EO_TYPES
#define _ELM_SYS_NOTIFY_EO_TYPES

/** System notification server types
 *
 * @ingroup Elm_Sys_Notify
 */
typedef enum
{
  ELM_SYS_NOTIFY_SERVER_NONE = 0, /**< No notificationserver (disables
                                   * notifications) */
  ELM_SYS_NOTIFY_SERVER_DBUS = 1 /* 1 >> 0 */ /**< Use DBus as a notification
                                               * server */
} Elm_Sys_Notify_Server;

/** The reason the notification was closed
 *
 * @since 1.8
 *
 * @ingroup Elm_Sys_Notify
 */
typedef enum
{
  ELM_SYS_NOTIFY_CLOSED_EXPIRED = 0, /**< The notification expired */
  ELM_SYS_NOTIFY_CLOSED_DISMISSED, /**< The notification was dismissed by the
                                    * user */
  ELM_SYS_NOTIFY_CLOSED_REQUESTED, /**< The notification was closed by a call to
                                    * CloseNotification method */
  ELM_SYS_NOTIFY_CLOSED_UNDEFINED /**< Undefined/reserved reasons */
} Elm_Sys_Notify_Closed_Reason;

/** Data on event when notification closed is emitted
 *
 * @since 1.8
 *
 * @ingroup Elm_Sys_Notify
 */
typedef struct _Elm_Sys_Notify_Notification_Closed
{
  unsigned int id; /**< ID of the notification */
  Elm_Sys_Notify_Closed_Reason reason; /**< The reason the notification was
                                        * closed */
} Elm_Sys_Notify_Notification_Closed;

/** Data on event when the action invoked is emitted
 *
 * @since 1.8
 *
 * @ingroup Elm_Sys_Notify
 */
typedef struct _Elm_Sys_Notify_Action_Invoked
{
  unsigned int id; /**< ID of the notification */
  char *action_key; /**< The key of the action invoked. These match the keys
                     * sent over in the list of actions */
} Elm_Sys_Notify_Action_Invoked;


#endif

/**
 * @brief Set the notifications server to be used.
 *
 * @note This is an advanced function that should be used only to fullfill very
 * specific purposes. Use elm_need_sys_notify() which activates the default
 * available notification servers.
 *
 * @param[in] obj The object.
 * @param[in] servers Binary mask of servers to enable. If a server is not
 * present in the binary mask but was previously registered, it will be
 * unregistered.
 *
 * @return @c true on success, @c false on failure
 *
 * @since 1.17
 *
 * @ingroup Elm_Sys_Notify_Group
 */
EAPI Eina_Bool elm_sys_notify_servers_set(Elm_Sys_Notify *obj, Elm_Sys_Notify_Server servers);

/**
 * @brief Get the notification servers that have been registered
 *
 * @param[in] obj The object.
 *
 * @return Binary mask of servers to enable. If a server is not present in the
 * binary mask but was previously registered, it will be unregistered.
 *
 * @since 1.17
 *
 * @ingroup Elm_Sys_Notify_Group
 */
EAPI Elm_Sys_Notify_Server elm_sys_notify_servers_get(const Elm_Sys_Notify *obj);

/**
 * @brief Returns the singleton instance of the notification manager
 * Elm.Sys_Notify. It is initialized upon the first call of this function
 *
 * @return The unique notification manager
 *
 * @since 1.17
 *
 * @ingroup Elm_Sys_Notify_Group
 */
EAPI Elm_Sys_Notify *elm_sys_notify_singleton_get(void);

#endif

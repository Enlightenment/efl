#ifndef ELDBUS_CONNECTION_H
#define ELDBUS_CONNECTION_H 1

/**
 * @defgroup Eldbus_Connection Connection
 * @ingroup Eldbus
 *
 * @{
 */

/**
 * @typedef Eldbus_Connection_Type
 * Enum of several Eldbus connection types.
 */
typedef enum
{
   ELDBUS_CONNECTION_TYPE_UNKNOWN = 0,       /**< sentinel, not a real type */
   ELDBUS_CONNECTION_TYPE_SESSION,
   ELDBUS_CONNECTION_TYPE_SYSTEM,
   ELDBUS_CONNECTION_TYPE_STARTER,
   ELDBUS_CONNECTION_TYPE_ADDRESS,
   ELDBUS_CONNECTION_TYPE_LAST       /**< sentinel, not a real type */
} Eldbus_Connection_Type;

#define ELDBUS_TIMEOUT_INFINITE ((int) 0x7fffffff) /**< Infinite timeout definition */

/**
 * Establish a connection to bus and integrate it with the ecore main
 * loop. If a connection of given type was already created before, its
 * reference counter is incremented and the connection is returned.
 *
 * @param type type of connection e.g ELDBUS_CONNECTION_TYPE_SESSION,
 * ELDBUS_CONNECTION_TYPE_SYSTEM or ELDBUS_CONNECTION_TYPE_STARTER
 *
 * @return connection with bus
 */
EAPI Eldbus_Connection *eldbus_connection_get(Eldbus_Connection_Type type);

/**
 * Always create and establish a new connection to bus and integrate it with
 * the ecore main loop. Differently from eldbus_connection_get(), this function
 * guarantees to create a new connection to the D-Bus daemon and the connection
 * is not shared by any means.
 *
 * @param type type of connection e.g ELDBUS_CONNECTION_TYPE_SESSION,
 * ELDBUS_CONNECTION_TYPE_SYSTEM or ELDBUS_CONNECTION_TYPE_STARTER
 *
 * @return connection with bus
 */
EAPI Eldbus_Connection *eldbus_private_connection_get(Eldbus_Connection_Type type);

/**
 * Establish a connection to bus and integrate it with the ecore main
 * loop. If a connection of given type was already created before, its
 * reference counter is incremented and the connection is returned.
 *
 * @param address the address which will be passed to dbus_connection_open()
 *
 * @return connection with bus
 */
EAPI Eldbus_Connection *eldbus_address_connection_get(const char *address) EINA_ARG_NONNULL(1);

/**
 * Always create and establish a new connection to bus and integrate it with
 * the ecore main loop. Differently from eldbus_connection_get(), this function
 * guarantees to create a new connection to the D-Bus daemon and the connection
 * is not shared by any means.
 *
 * @param address the address which will be passed to dbus_connection_open_private()
 *
 * @return connection with bus
 */
EAPI Eldbus_Connection *eldbus_private_address_connection_get(const char *address) EINA_ARG_NONNULL(1);

/**
 * @brief Increment connection reference count.
 *
 * @param conn The given Eldbus_Connection object to reference.
 * @return The Eldbus_Connection object given as parameter.
 */
EAPI Eldbus_Connection *eldbus_connection_ref(Eldbus_Connection *conn) EINA_ARG_NONNULL(1);

/**
 * @brief Decrement connection reference count.
 *
 * If reference count reaches 0, the connection to bus will be dropped and all
 * its children will be invalidated.
 *
 * @param conn The given Eldbus_Connection object to unreference.
 */
EAPI void              eldbus_connection_unref(Eldbus_Connection *conn) EINA_ARG_NONNULL(1);

/**
 * @brief Add a callback function to be called when connection is freed
 *
 * @param conn The connection object to add the callback to.
 * @param cb The callback to be called when the connection is free.
 * @param data The data passed to the callback.
 */
EAPI void              eldbus_connection_free_cb_add(Eldbus_Connection *conn, Eldbus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Remove callback registered in eldbus_connection_free_cb_add().
 *
 * @param conn The connection object to delete the callback from.
 * @param cb The callback that was called when the connection was free.
 * @param data The data that was passed to the callback.
 */
EAPI void              eldbus_connection_free_cb_del(Eldbus_Connection *conn, Eldbus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Set an attached data pointer to an object with a given string key.
 *
 * @param conn The connection object to store data to
 * @param key to identify data
 * @param data data that will be stored
 */
EAPI void              eldbus_connection_data_set(Eldbus_Connection *conn, const char *key, const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Get data stored in connection.
 *
 * @param conn connection where data is stored
 * @param key key that identifies data
 *
 * @return pointer to data if found otherwise NULL
 */
EAPI void             *eldbus_connection_data_get(const Eldbus_Connection *conn, const char *key) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Del data stored in connection.
 *
 * @param conn connection where data is stored
 * @param key that identifies data
 *
 * @return pointer to data if found otherwise NULL
 */
EAPI void             *eldbus_connection_data_del(Eldbus_Connection *conn, const char *key) EINA_ARG_NONNULL(1, 2);

/**
 * @typedef Eldbus_Connection_Event_Type
 * Enum of several Eldbus connection events.
 */
typedef enum
{
   ELDBUS_CONNECTION_EVENT_DEL,
   ELDBUS_CONNECTION_EVENT_DISCONNECTED,
   ELDBUS_CONNECTION_EVENT_LAST    /**< sentinel, not a real event type */
} Eldbus_Connection_Event_Type;

/**
 * @typedef Eldbus_Connection_Event_Cb
 *
 * Callback called when we receive an event.
 */
typedef void (*Eldbus_Connection_Event_Cb)(void *data, Eldbus_Connection *conn, void *event_info);

/**
 * @brief Add a callback function to be called when an event occurs of the
 * type passed.
 *
 * @param conn The connection object to add the callback to.
 * @param type The type of event that will trigger the callback.
 * @param cb The callback to be called when the event is triggered.
 * @param cb_data The data passed to the callback.
 */
EAPI void                  eldbus_connection_event_callback_add(Eldbus_Connection *conn, Eldbus_Connection_Event_Type type, Eldbus_Connection_Event_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Remove callback registered in eldbus_connection_event_callback_add().
 *
 * @param conn The connection object to delete the callback from.
 * @param type The type of event that was triggered the callback.
 * @param cb The callback that was called when the event was triggered.
 * @param cb_data The data that was passed to the callback.
 */
EAPI void                  eldbus_connection_event_callback_del(Eldbus_Connection *conn, Eldbus_Connection_Event_Type type, Eldbus_Connection_Event_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Send a message.
 *
 * @param conn the connection where the message will be sent
 * @param msg message that will be sent
 * @param cb if msg is a method call a callback should be passed
 * to be executed when a response arrives
 * @param cb_data data passed to callback
 * @param timeout timeout in milliseconds, -1 to use default internal value or
 * ELDBUS_TIMEOUT_INFINITE for no timeout
 * @return A Eldbus_Pending struct.
 */
EAPI Eldbus_Pending *eldbus_connection_send(Eldbus_Connection *conn, Eldbus_Message *msg, Eldbus_Message_Cb cb, const void *cb_data, double timeout) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Gets unique name assigned by the message bus.
 *
 * @param conn connection object to get unique name from.
 *
 * @return pointer to unique name string or NULL or error. Returned value
 * remains valid until connection is free.
 */
EAPI const char *eldbus_connection_unique_name_get(Eldbus_Connection *conn) EINA_ARG_NONNULL(1);
/**
 * @}
 */
#endif

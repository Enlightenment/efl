#ifndef ELDBUS_PROXY_H
#define ELDBUS_PROXY_H 1

/**
 * @defgroup Eldbus_Proxy Proxy
 * @ingroup Eldbus
 *
 * @{
 */

/**
 * @brief Get a proxy of the following interface name in a Eldbus_Object.
 *
 * @param obj The Eldbus_Object on which to do the query.
 * @param interface The interface name of the proxy.
 * @return The corresponding Eldbus_Proxy object.
 */
EAPI Eldbus_Proxy          *eldbus_proxy_get(Eldbus_Object *obj, const char *interface) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Increase proxy reference.
 *
 * @param proxy The Eldbus_Proxy on which to increase the refcount.
 * @return The Eldbus_Proxy with an increased refcount.
 */
EAPI Eldbus_Proxy          *eldbus_proxy_ref(Eldbus_Proxy *proxy) EINA_ARG_NONNULL(1);

/**
 * @brief Decrease proxy reference.
 * If reference == 0 proxy will be freed and all your children.
 *
 * @param proxy The Eldbus_Proxy on which to decrease the refcount.
 */
EAPI void                  eldbus_proxy_unref(Eldbus_Proxy *proxy) EINA_ARG_NONNULL(1);

/**
 * @brief Get the Eldbus_Object associated with a proxy object.
 *
 * @param proxy The Eldbus_Proxy on which to do the query.
 * @return The corresponding Eldbus_Object.
 */
EAPI Eldbus_Object         *eldbus_proxy_object_get(const Eldbus_Proxy *proxy) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Get the interface name associated with a proxy object.
 *
 * @param proxy The Eldbus_Proxy on which to do the query.
 * @return The string corresponding to the interface name.
 */
EAPI const char           *eldbus_proxy_interface_get(const Eldbus_Proxy *proxy) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Set data to an Eldbus_Proxy object.
 *
 * @param proxy The Eldbus_Proxy on which to set the data.
 * @param key A string to which the data will be associated.
 * @param data The data to set to the proxy object.
 */
EAPI void                  eldbus_proxy_data_set(Eldbus_Proxy *proxy, const char *key, const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Get data of an Eldbus_Proxy object.
 *
 * @param proxy The Eldbus_Proxy on which to get the data.
 * @param key The string to which the data is associated.
 * @return The data set to the proxy object associated with the provided key.
 */
EAPI void                 *eldbus_proxy_data_get(const Eldbus_Proxy *proxy, const char *key) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Delete data of an Eldbus_Proxy object.
 *
 * @param proxy The Eldbus_Proxy on which to delete the data.
 * @param key The string to which the data is associated.
 */
EAPI void                 *eldbus_proxy_data_del(Eldbus_Proxy *proxy, const char *key) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Add a callback function to be called when an event of the
 * type passed occurs.
 *
 * @param proxy The Eldbus_Proxy on which to add the callback.
 * @param cb The callback to add.
 * @param data The data to pass to the callback.
 */
EAPI void                  eldbus_proxy_free_cb_add(Eldbus_Proxy *proxy, Eldbus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Remove callback registered in eldbus_proxy_free_cb_add().
 *
 * @param proxy The Eldbus_Proxy on which to delete the callback.
 * @param cb The callback to delete.
 * @param data The data passed to the callback.
 */
EAPI void                  eldbus_proxy_free_cb_del(Eldbus_Proxy *proxy, Eldbus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Constructs a new message to invoke a method on a remote interface.
 *
 * @param proxy The Eldbus_Proxy on which to call the method.
 * @param member The name of the method to invoke.
 * @return An Eldbus_Message object.
 */
EAPI Eldbus_Message        *eldbus_proxy_method_call_new(Eldbus_Proxy *proxy, const char *member) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Send a message.
 *
 * @param proxy the msg will be send in connection that proxy belongs
 * @param msg message that will be send
 * @param cb if msg is a method call a callback should be passed
 * @param cb_data data passed to callback
 * @param timeout timeout in milliseconds, -1 to default internal value or
 * ELDBUS_TIMEOUT_INFINITE for no timeout
 * @return A Eldbus_Pending object on the sent message.
 */
EAPI Eldbus_Pending        *eldbus_proxy_send(Eldbus_Proxy *proxy, Eldbus_Message *msg, Eldbus_Message_Cb cb, const void *cb_data, double timeout) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Send a message and block while waiting for the reply.
 *
 * @param proxy the msg will be send in connection that proxy belongs
 * @param msg message that will be send
 * @param timeout timeout in milliseconds, -1 to default internal value or
 * ELDBUS_TIMEOUT_INFINITE for no timeout
 *
 * @return The reply message, error message or NULL.
 * The returned Eldbus_Message need to be unref after read.
 * @since 1.13
 */
EAPI Eldbus_Message        *eldbus_proxy_send_and_block(Eldbus_Proxy *proxy, Eldbus_Message *msg, double timeout) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Call a method in proxy.
 * Send a method call to interface that proxy belong with data.
 *
 * @param proxy
 * @param member method name
 * @param cb if msg is a method call a callback should be passed
 * to be execute when response arrive
 * @param cb_data data passed to callback
 * @param timeout timeout in milliseconds, -1 to default internal value or
 * ELDBUS_TIMEOUT_INFINITE for no timeout
 * @param signature of data that will be send
 * @param ... data value
 * @return A Eldbus_Pending object on the sent message.
 *
 * @note This function only support basic type to complex types use
 * eldbus_message_iter_* functions.
 */
EAPI Eldbus_Pending        *eldbus_proxy_call(Eldbus_Proxy *proxy, const char *member, Eldbus_Message_Cb cb, const void *cb_data, double timeout, const char *signature, ...) EINA_ARG_NONNULL(1, 2, 6);

/**
 * @brief Call a method in proxy.
 * Send a method call to interface that proxy belong with data.
 *
 * @param proxy
 * @param member method name
 * @param cb callback that will be called when response arrive.
 * @param cb_data data passed to callback
 * @param timeout timeout in milliseconds, -1 to default internal value or
 * ELDBUS_TIMEOUT_INFINITE for no timeout
 * @param signature of data that will be send
 * @param ap va_list of data value
 * @return A Eldbus_Pending object on the sent message.
 *
 * @note This function only support basic type to complex types use
 * eldbus_message_iter_* functions.
 */
EAPI Eldbus_Pending        *eldbus_proxy_vcall(Eldbus_Proxy *proxy, const char *member, Eldbus_Message_Cb cb, const void *cb_data, double timeout, const char *signature, va_list ap) EINA_ARG_NONNULL(1, 2, 6);

/**
 * @brief Add a signal handler.
 *
 * @param proxy interface where the signal is emitted
 * @param member name of the signal
 * @param cb callback that will be called when this signal is received
 * @param cb_data data that will be passed to callback
 * @return A listener to the desired signal.
 */
EAPI Eldbus_Signal_Handler *eldbus_proxy_signal_handler_add(Eldbus_Proxy *proxy, const char *member, Eldbus_Signal_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 3);

/**
 * @typedef Eldbus_Proxy_Event_Type
 *
 * An enumeration containing proxy event types.
 */
typedef enum
{
   ELDBUS_PROXY_EVENT_PROPERTY_CHANGED = 0, /**< a property has changed */
   ELDBUS_PROXY_EVENT_PROPERTY_REMOVED, /**< a property was removed */
   ELDBUS_PROXY_EVENT_DEL,
   ELDBUS_PROXY_EVENT_PROPERTY_LOADED,
   ELDBUS_PROXY_EVENT_LAST    /**< sentinel, not a real event type */
} Eldbus_Proxy_Event_Type;

/**
 * @typedef Eldbus_Proxy_Event_Property_Changed
 *
 * A structure used when ELDBUS_PROXY_EVENT_PROPERTY_CHANGED event is received.
 */
typedef struct _Eldbus_Proxy_Event_Property_Changed
{
   const char       *name; /**< the name of the property */
   const Eldbus_Proxy *proxy; /**< the proxy object */
   const Eina_Value *value; /**< the value of the property */
} Eldbus_Proxy_Event_Property_Changed;

/**
 * @typedef Eldbus_Proxy_Event_Property_Loaded
 *
 * A structure used when ELDBUS_PROXY_EVENT_PROPERTY_LOADED event is received.
 */
typedef struct _Eldbus_Proxy_Event_Property_Loaded
{
   const Eldbus_Proxy *proxy; /**< the proxy object */
} Eldbus_Proxy_Event_Property_Loaded;

/**
 * @typedef Eldbus_Proxy_Event_Property_Removed
 *
 * A structure used when ELDBUS_PROXY_EVENT_PROPERTY_REMOVED event is received.
 */
typedef struct _Eldbus_Proxy_Event_Property_Removed
{
   const char  *interface; /**< interface name */
   const Eldbus_Proxy *proxy; /**< the proxy object */
   const char  *name; /**< the name of the property */
} Eldbus_Proxy_Event_Property_Removed;

/**
 * @typedef Eldbus_Proxy_Event_Cb
 *
 * A callback called when an event occurs.
 * @param data The data passed by the caller.
 * @param proxy The Eldbus_Proxy object.
 * @param event_info Information on the event that triggered the callback.
 */
typedef void (*Eldbus_Proxy_Event_Cb)(void *data, Eldbus_Proxy *proxy, void *event_info);

/**
 * @brief Add a callback function to be called when occurs a event of the
 * type passed.
 *
 * @param proxy The Eldbus_Proxy object.
 * @param type The type of the event to register an callback on.
 * @param cb The callback function to register.
 * @param cb_data The data to pass to the callback.
 */
EAPI void eldbus_proxy_event_callback_add(Eldbus_Proxy *proxy, Eldbus_Proxy_Event_Type type, Eldbus_Proxy_Event_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Remove callback registered in eldbus_proxy_event_callback_add().
 *
 * @param proxy The Eldbus_Proxy object.
 * @param type The type of the event the callback was registered on.
 * @param cb The callback function to delete.
 * @param cb_data The data passed to the callback.
 */
EAPI void eldbus_proxy_event_callback_del(Eldbus_Proxy *proxy, Eldbus_Proxy_Event_Type type, Eldbus_Proxy_Event_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 3);

/**
 * @}
 */
#endif

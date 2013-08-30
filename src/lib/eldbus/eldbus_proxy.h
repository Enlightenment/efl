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
 */
EAPI Eldbus_Proxy          *eldbus_proxy_get(Eldbus_Object *obj, const char *interface) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Increase proxy reference.
 */
EAPI Eldbus_Proxy          *eldbus_proxy_ref(Eldbus_Proxy *proxy) EINA_ARG_NONNULL(1);

/**
 * @brief Decrease proxy reference.
 * If reference == 0 proxy will be freed and all your children.
 */
EAPI void                  eldbus_proxy_unref(Eldbus_Proxy *proxy) EINA_ARG_NONNULL(1);

EAPI Eldbus_Object         *eldbus_proxy_object_get(const Eldbus_Proxy *proxy) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *eldbus_proxy_interface_get(const Eldbus_Proxy *proxy) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

EAPI void                  eldbus_proxy_data_set(Eldbus_Proxy *proxy, const char *key, const void *data) EINA_ARG_NONNULL(1, 2, 3);
EAPI void                 *eldbus_proxy_data_get(const Eldbus_Proxy *proxy, const char *key) EINA_ARG_NONNULL(1, 2);
EAPI void                 *eldbus_proxy_data_del(Eldbus_Proxy *proxy, const char *key) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Add a callback function to be called when occurs a event of the
 * type passed.
 */
EAPI void                  eldbus_proxy_free_cb_add(Eldbus_Proxy *proxy, Eldbus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Remove callback registered in eldbus_proxy_free_cb_add().
 */
EAPI void                  eldbus_proxy_free_cb_del(Eldbus_Proxy *proxy, Eldbus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Constructs a new message to invoke a method on a remote interface.
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
 */
EAPI Eldbus_Pending        *eldbus_proxy_send(Eldbus_Proxy *proxy, Eldbus_Message *msg, Eldbus_Message_Cb cb, const void *cb_data, double timeout) EINA_ARG_NONNULL(1, 2);

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
 */
EAPI Eldbus_Signal_Handler *eldbus_proxy_signal_handler_add(Eldbus_Proxy *proxy, const char *member, Eldbus_Signal_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 3);

typedef enum
{
   ELDBUS_PROXY_EVENT_PROPERTY_CHANGED = 0,
   ELDBUS_PROXY_EVENT_PROPERTY_REMOVED,
   ELDBUS_PROXY_EVENT_DEL,
   ELDBUS_PROXY_EVENT_PROPERTY_LOADED,
   ELDBUS_PROXY_EVENT_LAST    /**< sentinel, not a real event type */
} Eldbus_Proxy_Event_Type;

typedef struct _Eldbus_Proxy_Event_Property_Changed
{
   const char       *name;
   const Eldbus_Proxy *proxy;
   const Eina_Value *value;
} Eldbus_Proxy_Event_Property_Changed;

typedef struct _Eldbus_Proxy_Event_Property_Loaded
{
   const Eldbus_Proxy *proxy;
} Eldbus_Proxy_Event_Property_Loaded;

typedef struct _Eldbus_Proxy_Event_Property_Removed
{
   const char  *interface;
   const Eldbus_Proxy *proxy;
   const char  *name;
} Eldbus_Proxy_Event_Property_Removed;

typedef void (*Eldbus_Proxy_Event_Cb)(void *data, Eldbus_Proxy *proxy, void *event_info);

/**
 * @brief Add a callback function to be called when occurs a event of the
 * type passed.
 */
EAPI void eldbus_proxy_event_callback_add(Eldbus_Proxy *proxy, Eldbus_Proxy_Event_Type type, Eldbus_Proxy_Event_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Remove callback registered in eldbus_proxy_event_callback_add().
 */
EAPI void eldbus_proxy_event_callback_del(Eldbus_Proxy *proxy, Eldbus_Proxy_Event_Type type, Eldbus_Proxy_Event_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 3);

/**
 * @}
 */
#endif

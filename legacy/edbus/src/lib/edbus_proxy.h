#ifndef EDBUS_PROXY_H
#define EDBUS_PROXY_H 1

/**
 * @defgroup EDBus_Proxy Proxy
 *
 * @{
 */
/**
 * @brief Get a proxy of the following interface name in a EDBus_Object.
 */
EAPI EDBus_Proxy          *edbus_proxy_get(EDBus_Object *obj, const char *interface) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;
/**
 * @brief Increase proxy reference.
 */
EAPI EDBus_Proxy          *edbus_proxy_ref(EDBus_Proxy *proxy) EINA_ARG_NONNULL(1);
/**
 * @brief Decrease proxy reference.
 * If reference == 0 proxy will be freed and all your children.
 */
EAPI void                  edbus_proxy_unref(EDBus_Proxy *proxy) EINA_ARG_NONNULL(1);

EAPI EDBus_Object         *edbus_proxy_object_get(const EDBus_Proxy *proxy) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *edbus_proxy_interface_get(const EDBus_Proxy *proxy) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

EAPI void                  edbus_proxy_data_set(EDBus_Proxy *proxy, const char *key, const void *data) EINA_ARG_NONNULL(1, 2, 3);
EAPI void                 *edbus_proxy_data_get(const EDBus_Proxy *proxy, const char *key) EINA_ARG_NONNULL(1, 2);
EAPI void                 *edbus_proxy_data_del(EDBus_Proxy *proxy, const char *key) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Add a callback function to be called when occurs a event of the
 * type passed.
 */
EAPI void                  edbus_proxy_cb_free_add(EDBus_Proxy *proxy, EDBus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Remove callback registered in edbus_proxy_cb_free_add().
 */
EAPI void                  edbus_proxy_cb_free_del(EDBus_Proxy *proxy, EDBus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Constructs a new message to invoke a method on a remote interface.
 */
EAPI EDBus_Message        *edbus_proxy_method_call_new(EDBus_Proxy *proxy, const char *member);

/**
 * @brief Send a message.
 *
 * @param proxy the msg will be send in connection that proxy belongs
 * @param msg message that will be send
 * @param cb if msg is a method call a callback should be passed
 * @param cb_data data passed to callback
 * @param timeout timeout in milliseconds, -1 to default internal value or
 * EDBUS_TIMEOUT_INFINITE for no timeout
 */
EAPI EDBus_Pending        *edbus_proxy_send(EDBus_Proxy *proxy, EDBus_Message *msg, EDBus_Message_Cb cb, const void *cb_data, double timeout) EINA_ARG_NONNULL(1, 2);
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
 * EDBUS_TIMEOUT_INFINITE for no timeout
 * @param signature of data that will be send
 * @param ... data value
 *
 * @note This function only support basic type to complex types use
 * edbus_message_iter_* functions.
 */
EAPI EDBus_Pending        *edbus_proxy_call(EDBus_Proxy *proxy, const char *member, EDBus_Message_Cb cb, const void *cb_data, double timeout, const char *signature, ...) EINA_ARG_NONNULL(1, 2, 6);
/**
 * @brief Call a method in proxy.
 * Send a method call to interface that proxy belong with data.
 *
 * @param proxy
 * @param member method name
 * @param cb callback that will be called when response arrive.
 * @param cb_data data passed to callback
 * @param timeout timeout in milliseconds, -1 to default internal value or
 * EDBUS_TIMEOUT_INFINITE for no timeout
 * @param signature of data that will be send
 * @param ap va_list of data value
 *
 * @note This function only support basic type to complex types use
 * edbus_message_iter_* functions.
 */
EAPI EDBus_Pending        *edbus_proxy_vcall(EDBus_Proxy *proxy, const char *member, EDBus_Message_Cb cb, const void *cb_data, double timeout, const char *signature, va_list ap) EINA_ARG_NONNULL(1, 2, 6);
/**
 * @brief Add a signal handler.
 *
 * @param proxy interface where the signal is emitted
 * @param member name of the signal
 * @param cb callback that will be called when this signal is received
 * @param cb_data data that will be passed to callback
 */
EAPI EDBus_Signal_Handler *edbus_proxy_signal_handler_add(EDBus_Proxy *proxy, const char *member, EDBus_Signal_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 3);

typedef enum
{
   EDBUS_PROXY_EVENT_PROPERTY_CHANGED = 0,
   EDBUS_PROXY_EVENT_PROPERTY_REMOVED,
   EDBUS_PROXY_EVENT_DEL,
   EDBUS_PROXY_EVENT_LAST    /**< sentinel, not a real event type */
} EDBus_Proxy_Event_Type;

typedef struct _EDBus_Proxy_Event_Property_Changed
{
   const char       *name;
   const Eina_Value *value;
} EDBus_Proxy_Event_Property_Changed;

typedef struct _EDBus_Proxy_Event_Property_Removed
{
   const char  *interface;
   EDBus_Proxy *proxy;
   const char  *name;
} EDBus_Proxy_Event_Property_Removed;

typedef void (*EDBus_Proxy_Event_Cb)(void *data, EDBus_Proxy *proxy, void *event_info);

/**
 * @brief Add a callback function to be called when occurs a event of the
 * type passed.
 */
EAPI void edbus_proxy_event_callback_add(EDBus_Proxy *proxy, EDBus_Proxy_Event_Type type, EDBus_Proxy_Event_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 3);
/**
 * @brief Remove callback registered in edbus_connection_event_callback_add().
 */
EAPI void edbus_proxy_event_callback_del(EDBus_Proxy *proxy, EDBus_Proxy_Event_Type type, EDBus_Proxy_Event_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 3);

/**
 * @}
 */
#endif

#ifndef EDBUS_OBJECT_H
#define EDBUS_OBJECT_H 1

/**
 * @defgroup EDBus_Object_Mapper Object Mapper
 *
 * @{
 */
/**
 * @brief Get an object of the given bus and path.
 *
 * @param conn connection where object belongs
 * @param bus name of bus or unique-id of who listens for calls of this object
 * @param path object path of this object
 */
EAPI EDBus_Object *edbus_object_get(EDBus_Connection *conn, const char *bus, const char *path) EINA_ARG_NONNULL(1, 2, 3) EINA_WARN_UNUSED_RESULT;
/**
 * @brief Increase object reference.
 */
EAPI EDBus_Object *edbus_object_ref(EDBus_Object *obj) EINA_ARG_NONNULL(1);
/**
 * @brief Decrease object reference.
 * If reference == 0 object will be freed and all its children.
 */
EAPI void          edbus_object_unref(EDBus_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Add a callback function to be called when object will be freed.
 *
 * @param obj object that you want to monitor
 * @param cb callback that will be executed
 * @param data passed to callback
 */
EAPI void          edbus_object_cb_free_add(EDBus_Object *obj, EDBus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Remove callback registered in edbus_object_cb_free_add().
 */
EAPI void          edbus_object_cb_free_del(EDBus_Object *obj, EDBus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

typedef enum
{
   EDBUS_OBJECT_EVENT_IFACE_ADDED = 0,
   EDBUS_OBJECT_EVENT_IFACE_REMOVED,
   EDBUS_OBJECT_EVENT_PROPERTY_CHANGED,
   EDBUS_OBJECT_EVENT_PROPERTY_REMOVED,
   EDBUS_OBJECT_EVENT_DEL,
   EDBUS_OBJECT_EVENT_LAST    /**< sentinel, not a real event type */
} EDBus_Object_Event_Type;

typedef struct _EDBus_Object_Event_Interface_Added
{
   const char  *interface;
   EDBus_Proxy *proxy;
} EDBus_Object_Event_Interface_Added;

typedef struct _EDBus_Object_Event_Interface_Removed
{
   const char *interface;
} EDBus_Object_Event_Interface_Removed;

typedef struct _EDBus_Object_Event_Property_Changed
{
   const char       *interface;
   EDBus_Proxy      *proxy;
   const char       *name;
   const Eina_Value *value;
} EDBus_Object_Event_Property_Changed;

typedef struct _EDBus_Object_Event_Property_Removed
{
   const char  *interface;
   EDBus_Proxy *proxy;
   const char  *name;
} EDBus_Object_Event_Property_Removed;

typedef void (*EDBus_Object_Event_Cb)(void *data, EDBus_Object *obj, void *event_info);

/**
 * @brief Add a callback function to be called when an event of the specified
 * type occurs.
 */
EAPI void                  edbus_object_event_callback_add(EDBus_Object *obj, EDBus_Object_Event_Type type, EDBus_Object_Event_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 3);
/**
 * @brief Remove callback registered in edbus_object_event_callback_add().
 */
EAPI void                  edbus_object_event_callback_del(EDBus_Object *obj, EDBus_Object_Event_Type type, EDBus_Object_Event_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 3);

EAPI EDBus_Connection     *edbus_object_connection_get(const EDBus_Object *obj) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *edbus_object_bus_name_get(const EDBus_Object *obj) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *edbus_object_path_get(const EDBus_Object *obj) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Send a message.
 *
 * @param obj the msg will be sent in connection to this object
 * @param msg message that will be sent
 * @param cb if msg is a method call a callback should be passed
 * to be executed when a response arrives
 * @param cb_data data passed to callback
 * @param timeout timeout in milliseconds, -1 to default internal value or
 * EDBUS_TIMEOUT_INFINITE for no timeout
 */
EAPI EDBus_Pending        *edbus_object_send(EDBus_Object *obj, EDBus_Message *msg, EDBus_Message_Cb cb, const void *cb_data, double timeout) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Add a signal handler.
 *
 * @param obj where the signal is emitted
 * @param interface of the signal
 * @param member name of the signal
 * @param cb callback that will be called when this signal is received
 * @param cb_data data that will be passed to callback
 */
EAPI EDBus_Signal_Handler *edbus_object_signal_handler_add(EDBus_Object *obj, const char *interface, const char *member, EDBus_Signal_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 4);
/**
 * @}
 */
#endif

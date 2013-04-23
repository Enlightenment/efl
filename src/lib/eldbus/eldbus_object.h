#ifndef ELDBUS_OBJECT_H
#define ELDBUS_OBJECT_H 1

/**
 * @defgroup Eldbus_Object_Mapper Object Mapper
 * @ingroup Eldbus
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
EAPI Eldbus_Object *eldbus_object_get(Eldbus_Connection *conn, const char *bus, const char *path) EINA_ARG_NONNULL(1, 2, 3) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Increase object reference.
 */
EAPI Eldbus_Object *eldbus_object_ref(Eldbus_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Decrease object reference.
 * If reference == 0 object will be freed and all its children.
 */
EAPI void          eldbus_object_unref(Eldbus_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Add a callback function to be called when object will be freed.
 *
 * @param obj object that you want to monitor
 * @param cb callback that will be executed
 * @param data passed to callback
 */
EAPI void          eldbus_object_free_cb_add(Eldbus_Object *obj, Eldbus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Remove callback registered in eldbus_object_free_cb_add().
 */
EAPI void          eldbus_object_free_cb_del(Eldbus_Object *obj, Eldbus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

typedef enum
{
   ELDBUS_OBJECT_EVENT_IFACE_ADDED = 0, /**< a parent path must have a ObjectManager interface */
   ELDBUS_OBJECT_EVENT_IFACE_REMOVED, /**< a parent path must have a ObjectManager interface */
   ELDBUS_OBJECT_EVENT_PROPERTY_CHANGED,
   ELDBUS_OBJECT_EVENT_PROPERTY_REMOVED,
   ELDBUS_OBJECT_EVENT_DEL,
   ELDBUS_OBJECT_EVENT_LAST    /**< sentinel, not a real event type */
} Eldbus_Object_Event_Type;

typedef struct _Eldbus_Object_Event_Interface_Added
{
   const char  *interface;
   Eldbus_Proxy *proxy;
} Eldbus_Object_Event_Interface_Added;

typedef struct _Eldbus_Object_Event_Interface_Removed
{
   const char *interface;
} Eldbus_Object_Event_Interface_Removed;

typedef struct _Eldbus_Object_Event_Property_Changed
{
   const char       *interface;
   Eldbus_Proxy      *proxy;
   const char       *name;
   const Eina_Value *value;
} Eldbus_Object_Event_Property_Changed;

typedef struct _Eldbus_Object_Event_Property_Removed
{
   const char  *interface;
   Eldbus_Proxy *proxy;
   const char  *name;
} Eldbus_Object_Event_Property_Removed;

typedef void (*Eldbus_Object_Event_Cb)(void *data, Eldbus_Object *obj, void *event_info);

/**
 * @brief Add a callback function to be called when an event of the specified
 * type occurs.
 */
EAPI void                  eldbus_object_event_callback_add(Eldbus_Object *obj, Eldbus_Object_Event_Type type, Eldbus_Object_Event_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Remove callback registered in eldbus_object_event_callback_add().
 */
EAPI void                  eldbus_object_event_callback_del(Eldbus_Object *obj, Eldbus_Object_Event_Type type, Eldbus_Object_Event_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 3);

EAPI Eldbus_Connection     *eldbus_object_connection_get(const Eldbus_Object *obj) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *eldbus_object_bus_name_get(const Eldbus_Object *obj) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *eldbus_object_path_get(const Eldbus_Object *obj) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Send a message.
 *
 * @param obj the msg will be sent in connection to this object
 * @param msg message that will be sent
 * @param cb if msg is a method call a callback should be passed
 * to be executed when a response arrives
 * @param cb_data data passed to callback
 * @param timeout timeout in milliseconds, -1 to default internal value or
 * ELDBUS_TIMEOUT_INFINITE for no timeout
 */
EAPI Eldbus_Pending        *eldbus_object_send(Eldbus_Object *obj, Eldbus_Message *msg, Eldbus_Message_Cb cb, const void *cb_data, double timeout) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Add a signal handler.
 *
 * @param obj where the signal is emitted
 * @param interface of the signal
 * @param member name of the signal
 * @param cb callback that will be called when this signal is received
 * @param cb_data data that will be passed to callback
 */
EAPI Eldbus_Signal_Handler *eldbus_object_signal_handler_add(Eldbus_Object *obj, const char *interface, const char *member, Eldbus_Signal_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 4);

EAPI Eldbus_Message *eldbus_object_method_call_new(Eldbus_Object *obj, const char *interface, const char *member) EINA_ARG_NONNULL(1, 2, 3) EINA_WARN_UNUSED_RESULT;

/**
 * @}
 */
#endif

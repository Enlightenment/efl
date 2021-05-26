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
 * @return The corresponding Eldbus_Object.
 */
EAPI Eldbus_Object *eldbus_object_get(Eldbus_Connection *conn, const char *bus, const char *path) EINA_ARG_NONNULL(1, 2, 3) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Increase object reference.
 *
 * @param obj An Eldbus_Object.
 * @return The same Eldbus_Object with an increased refcount.
 */
EAPI Eldbus_Object *eldbus_object_ref(Eldbus_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Decrease object reference.
 * If reference == 0 object will be freed and all its children.
 *
 * @param obj An Eldbus_Object.
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
 *
 * @param obj Object monitored.
 * @param cb Callback that was registered.
 * @param data Data that was passed to callback.
 */
EAPI void          eldbus_object_free_cb_del(Eldbus_Object *obj, Eldbus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @typedef Eldbus_Object_Event_Type
 *
 * An enumeration containing several Eldbus_Object event types.
 */
typedef enum
{
   ELDBUS_OBJECT_EVENT_IFACE_ADDED = 0, /**< a parent path must have a ObjectManager interface */
   ELDBUS_OBJECT_EVENT_IFACE_REMOVED, /**< a parent path must have a ObjectManager interface */
   ELDBUS_OBJECT_EVENT_PROPERTY_CHANGED, /**< a property has changes */
   ELDBUS_OBJECT_EVENT_PROPERTY_REMOVED, /**< a property was removed */
   ELDBUS_OBJECT_EVENT_DEL,
   ELDBUS_OBJECT_EVENT_LAST    /**< sentinel, not a real event type */
} Eldbus_Object_Event_Type;

/**
 * @typedef Eldbus_Object_Event_Interface_Added
 *
 * Structure used with the ELDBUS_OBJECT_EVENT_IFACE_ADDED event.
 */
typedef struct _Eldbus_Object_Event_Interface_Added
{
   const char  *interface; /**< The interface name */
   Eldbus_Proxy *proxy; /**< The proxy object */
} Eldbus_Object_Event_Interface_Added;

/**
 * @typedef Eldbus_Object_Event_Interface_Removed
 *
 * Structure used with the ELDBUS_OBJECT_EVENT_IFACE_REMOVED event.
 */
typedef struct _Eldbus_Object_Event_Interface_Removed
{
   const char *interface; /**< The interface name */
} Eldbus_Object_Event_Interface_Removed;

/**
 * @typedef Eldbus_Object_Event_Property_Changed
 *
 * Structure used with the ELDBUS_OBJECT_EVENT_PROPERTY_CHANGED event.
 */
typedef struct _Eldbus_Object_Event_Property_Changed
{
   const char       *interface; /**< The interface name */
   Eldbus_Proxy      *proxy; /**< The proxy object */
   const char       *name; /**< The name of the property */
   const Eina_Value *value; /**< The value of the property */
} Eldbus_Object_Event_Property_Changed;

/**
 * @typedef Eldbus_Object_Event_Property_Removed
 *
 * Structure used with the ELDBUS_OBJECT_EVENT_PROPERTY_REMOVED event.
 */
typedef struct _Eldbus_Object_Event_Property_Removed
{
   const char  *interface; /**< The interface name */
   Eldbus_Proxy *proxy; /**< The proxy object */
   const char  *name; /**< The name of the property */
} Eldbus_Object_Event_Property_Removed;

/**
 * @typedef Eldbus_Object_Event_Cb
 *
 * Callback that will be called when an Eldbus_Object event happens.
 *
 * @param data Context data.
 * @param obj The Eldbus_Object.
 * @param event_info Information about the event that triggered the callback.
 */
typedef void (*Eldbus_Object_Event_Cb)(void *data, Eldbus_Object *obj, void *event_info);

/**
 * @brief Add a callback function to be called when an event of the specified
 * type occurs.
 *
 * @param obj The Eldbus_Object on which to register a callback.
 * @param type The type of the event.
 * @param cb The callback to call.
 * @param cb_data The data to pass to the callback.
 */
EAPI void                  eldbus_object_event_callback_add(Eldbus_Object *obj, Eldbus_Object_Event_Type type, Eldbus_Object_Event_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Remove callback registered in eldbus_object_event_callback_add().
 *
 * @param obj The Eldbus_Object.
 * @param type The type of the event.
 * @param cb The callback to call.
 * @param cb_data The data to pass to the callback.
 */
EAPI void                  eldbus_object_event_callback_del(Eldbus_Object *obj, Eldbus_Object_Event_Type type, Eldbus_Object_Event_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 3);

/**
 * Get the Eldbus_Connection object associated with a Eldbus_Object.
 *
 * @param obj The Eldbus_Object.
 * @return The corresponding Eldbus_Connection object.
 */
EAPI Eldbus_Connection     *eldbus_object_connection_get(const Eldbus_Object *obj) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * Get the name associated with a Eldbus_Object.
 *
 * @param obj The Eldbus_Object.
 * @return A string corresponding to the Eldbus_Object name.
 */
EAPI const char           *eldbus_object_bus_name_get(const Eldbus_Object *obj) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * Get the path associated with a Eldbus_Object.
 *
 * @param obj The Eldbus_Object.
 * @return A string corresponding to the Eldbus_Object path.
 */
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
 * @return A Eldbus_Pending object.
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
 * @return A listener to the desired signal.
 */
EAPI Eldbus_Signal_Handler *eldbus_object_signal_handler_add(Eldbus_Object *obj, const char *interface, const char *member, Eldbus_Signal_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 4);

/**
 * @brief Call a dbus method on the Eldbus_Object.
 *
 * @param obj The Eldbus_Object on which to call the method.
 * @param interface Interface name.
 * @param member Name of the method to be called.
 *
 * @return a new Eldbus_Message, free with eldbus_message_unref()
 */
EAPI Eldbus_Message *eldbus_object_method_call_new(Eldbus_Object *obj, const char *interface, const char *member) EINA_ARG_NONNULL(1, 2, 3) EINA_WARN_UNUSED_RESULT;

/**
 * @}
 */
#endif

#ifndef ELDBUS_SERVICE_H
#define ELDBUS_SERVICE_H 1

/**
 * @defgroup Eldbus_Service Service
 * @ingroup Eldbus
 *
 * @{
 */
#define ELDBUS_METHOD_FLAG_DEPRECATED 1
#define ELDBUS_METHOD_FLAG_NOREPLY (1 << 1)

#define ELDBUS_SIGNAL_FLAG_DEPRECATED 1

#define ELDBUS_PROPERTY_FLAG_DEPRECATED 1

typedef struct _Eldbus_Arg_Info
{
   const char *signature;
   const char *name;
} Eldbus_Arg_Info;

/**
 * @brief Used to insert complete types to signature of methods or signals.
 *
 * Example: ELDBUS_ARGS({"s", "interface"}, {"s", "property"})
 * The signature will be "ss" and each string will have a tag name on
 * introspect XML with the respective name.
 */
#define ELDBUS_ARGS(args...) (const Eldbus_Arg_Info[]){ args, { NULL, NULL } }

typedef struct _Eldbus_Service_Interface Eldbus_Service_Interface;
typedef Eldbus_Message * (*Eldbus_Method_Cb)(const Eldbus_Service_Interface *iface, const Eldbus_Message *message);

/**
 * Callback function to append property value to message.
 *
 * @param iface interface of property
 * @param propname name of property
 * @param iter variant iterator in which value must be appended
 * @param request_msg message that request property
 * @param error if a error happen you must set a message error to be send caller
 *
 * @return EINA_TRUE if success
 *
 * @note request_msg and error arguments are only different from NULL when a
 * client request a property with Properties.Get or Properties.GetAll. Upon
 * calls to eldbus_service_property_changed(), this callback will also be called.
 * It's a mistake to return an error in this case because if a property changed,
 * it must have a new value set and it should be able to be read.
 */
typedef Eina_Bool (*Eldbus_Property_Get_Cb)(const Eldbus_Service_Interface *iface, const char *propname, Eldbus_Message_Iter *iter, const Eldbus_Message *request_msg, Eldbus_Message **error);

/**
 * Callback function to set property value from message.
 *
 * @param iface interface of property
 * @param propname name of property
 * @param input_msg message call where you have to get value
 *
 * @return Message of response, could be a simple method_return, error or NULL to send response later.
 */
typedef Eldbus_Message *(*Eldbus_Property_Set_Cb)(const Eldbus_Service_Interface *iface, const char *propname, Eldbus_Message_Iter *iter, const Eldbus_Message *input_msg);

typedef struct _Eldbus_Method
{
   const char *member;
   const Eldbus_Arg_Info *in;
   const Eldbus_Arg_Info *out;
   Eldbus_Method_Cb cb;
   unsigned int flags;
} Eldbus_Method;

typedef struct _Eldbus_Signal
{
   const char *name;
   const Eldbus_Arg_Info *args;
   unsigned int flags;
} Eldbus_Signal;

typedef struct _Eldbus_Property
{
   const char *name;
   const char *type;
   Eldbus_Property_Get_Cb get_func;
   Eldbus_Property_Set_Cb set_func;
   unsigned int flags;
} Eldbus_Property;

typedef struct _Eldbus_Service_Interface_Desc
{
   const char *interface; /**< interface name */
   const Eldbus_Method *methods; /**< array of the methods that should be registered in this interface, the last item of array should be filled with NULL */
   const Eldbus_Signal *signals; /**< array of signal that this interface send, the last item of array should be filled with NULL */
   const Eldbus_Property *properties; /**< array of property that this interface have, the last item of array should be filled with NULL  */
   const Eldbus_Property_Get_Cb default_get; /**< default get function, if a property don't have a get function this will be used */
   const Eldbus_Property_Set_Cb default_set; /**< default set function, if a property don't have a set function this will be used */
} Eldbus_Service_Interface_Desc;

/**
 * @brief Register an interface in the given path and connection.
 *
 * @param conn where the interface should listen
 * @param path object path
 * @param desc description of interface
 *
 * @return Interface
 */
EAPI Eldbus_Service_Interface *eldbus_service_interface_register(Eldbus_Connection *conn, const char *path, const Eldbus_Service_Interface_Desc *desc) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Register a fallback interface handler for a given subsection of the object hierarchy.
 * Note: Use eldbus_service_interface_unregister() to unregister a interface.
 * @param conn where the interface should listen
 * @param path a '/' delimited string of path elements
 * @param desc description of interface
 * @see eldbus_service_interface_unregister()
 *
 * @since 1.9
 *
 * @return Interface
 */
EAPI Eldbus_Service_Interface *
eldbus_service_interface_fallback_register(Eldbus_Connection *conn, const char *path, const Eldbus_Service_Interface_Desc *desc) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Unregister a interface.
 * Note: This doesn't unregister the object path if interface count reaches 0.
 * Use eldbus_service_object_unregister() to unregister the object.
 */
EAPI void eldbus_service_interface_unregister(Eldbus_Service_Interface *iface) EINA_ARG_NONNULL(1);

/**
 * @brief Unregister all interfaces of the object path that this interface belongs
 * and the object path.
 */
EAPI void eldbus_service_object_unregister(Eldbus_Service_Interface *iface) EINA_ARG_NONNULL(1);
EAPI Eldbus_Connection *eldbus_service_connection_get(const Eldbus_Service_Interface *iface) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char *eldbus_service_object_path_get(const Eldbus_Service_Interface *iface) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Emit a signal handler of the interface with non-complex types.
 * Each signal handler have a internal id, the first signal handler of
 * interface is = 0 the second = 1 and go on.
 *
 * @param iface interface of the signal
 * @param signal_id id of signal
 * @param ... values that will be send on signal
 */
EAPI Eina_Bool eldbus_service_signal_emit(const Eldbus_Service_Interface *iface, unsigned int signal_id, ...) EINA_ARG_NONNULL(1);

/**
 * @brief Create signal message.
 * Each signal handler have a internal id, the first signal handler of
 * interface is = 0 the second = 1 and go on.
 * This function is used when the signal has complex types.
 *
 * @param iface interface of the signal
 * @param signal_id id of signal
 */
EAPI Eldbus_Message *eldbus_service_signal_new(const Eldbus_Service_Interface *iface, unsigned int signal_id) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Send a signal message.
 *
 * On success this will call eldbus_message_unref() on the @param signal_msg,
 * which is the intended behavior in 99% of the cases. Remember to increment
 * the refcount if you want to keep it alive.
 */
EAPI Eina_Bool eldbus_service_signal_send(const Eldbus_Service_Interface *iface, Eldbus_Message *signal_msg) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Store data at object path, this data can be obtained from all interfaces
 * of the same object.
 *
 * @param iface interface that belong to the object path where data will
 * be stored
 * @param key to identify data
 * @param data
 */
EAPI void eldbus_service_object_data_set(Eldbus_Service_Interface *iface, const char *key, const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Get data stored in object path.
 *
 * @param iface interface that belongs to the object path where data are stored
 * @param key that identify data
 *
 * @return pointer to data if found otherwise NULL
 */
EAPI void *eldbus_service_object_data_get(const Eldbus_Service_Interface *iface, const char *key) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Del data stored in object path.
 *
 * @param iface interface that belongs to the object path where data are stored
 * @param key that identify data
 *
 * @return pointer to data if found otherwise NULL
 */
EAPI void *eldbus_service_object_data_del(Eldbus_Service_Interface *iface, const char *key) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Add property to list of changed properties
 * A DBus.PropertiesChanged signal will be sent in an idler with all properties
 * that have changed.
 *
 * @param iface Interface containing the changed property
 * @param name Property name
 */
EAPI Eina_Bool eldbus_service_property_changed(const Eldbus_Service_Interface *iface, const char *name) EINA_ARG_NONNULL(1, 2);

EAPI Eina_Bool eldbus_service_property_invalidate_set(const Eldbus_Service_Interface *iface, const char *name, Eina_Bool is_invalidate) EINA_ARG_NONNULL(1, 2);

/**
 * Attach ObjectManager interface.
 *
 * @param iface ObjectManager will be attach in object path of this interface.
 * @return EINA_TRUE if success
 */
EAPI Eina_Bool eldbus_service_object_manager_attach(Eldbus_Service_Interface *iface) EINA_ARG_NONNULL(1);

/**
 * Detach ObjectManager interface.
 *
 * @param iface ObjectManager of object path of this interface will be detach.
 * @return EINA_TRUE if success
 */
EAPI Eina_Bool eldbus_service_object_manager_detach(Eldbus_Service_Interface *iface) EINA_ARG_NONNULL(1);

/**
 * @}
 */
#endif

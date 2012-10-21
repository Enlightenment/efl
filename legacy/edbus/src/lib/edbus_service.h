#ifndef EDBUS_SERVICE_H
#define EDBUS_SERVICE_H 1

/**
 * @defgroup EDBus_Service Service
 *
 * @{
 */
#define EDBUS_METHOD_FLAG_DEPRECATED 1
#define EDBUS_METHOD_FLAG_NOREPLY (1 << 1)

#define EDBUS_SIGNAL_FLAG_DEPRECATED 1

typedef struct _EDBus_Arg_Info
{
   const char *signature;
   const char *name;
} EDBus_Arg_Info;

/**
 * @brief Used to insert complete types to signature of methods or signals.
 *
 * Example: EDBUS_ARGS({"s", "interface"}, {"s", "property"})
 * The signature will be "ss" and each string will have a tag name on
 * introspect XML with the respective name.
 */
#define EDBUS_ARGS(args...) (const EDBus_Arg_Info[]){ args, { NULL, NULL } }

typedef struct _EDBus_Service_Interface EDBus_Service_Interface;
typedef EDBus_Message * (*EDBus_Method_Cb)(const EDBus_Service_Interface *iface, const EDBus_Message *message);

typedef struct _EDBus_Method
{
   const char *member;
   const EDBus_Arg_Info *in;
   const EDBus_Arg_Info *out;
   EDBus_Method_Cb cb;
   unsigned int flags;
} EDBus_Method;

typedef struct _EDBus_Signal
{
   const char *name;
   const EDBus_Arg_Info *args;
   unsigned int flags;
} EDBus_Signal;

/**
 * @brief Register an interface in the given path and connection.
 *
 * @param conn where the interface should listen
 * @param path object path
 * @param iface interface
 * @param methods array of the methods that should be registered in this
 * interface, the last item of array should be filled with NULL
 * @param signals array of signal that this interface send, the last item
 * of array should be filled with NULL
 *
 * @note methods and signals must be static variables
 *
 * @return Interface
 */
EAPI EDBus_Service_Interface *edbus_service_interface_register(EDBus_Connection *conn, const char *path, const char *interface, const EDBus_Method methods[], const EDBus_Signal signals[]);
/**
 * @brief Unregister a interface.
 * If this is the last interface of the object path, the object path will be
 * removed too.
 */
EAPI void edbus_service_interface_unregister(EDBus_Service_Interface *iface);
/**
 * @brief Unregister all interfaces of the object path that this interface belongs
 * and the object path.
 */
EAPI void edbus_service_object_unregister(EDBus_Service_Interface *iface);
EAPI EDBus_Connection *edbus_service_connection_get(const EDBus_Service_Interface *iface);
EAPI const char *edbus_service_object_path_get(const EDBus_Service_Interface *iface);

/**
 * @brief Emit a signal handler of the interface with non-complex types.
 * Each signal handler have a internal id, the first signal handler of
 * interface is = 0 the second = 1 and go on.
 *
 * @param iface interface of the signal
 * @param signal_id id of signal
 * @param ... values that will be send on signal
 */
EAPI Eina_Bool edbus_service_signal_emit(EDBus_Service_Interface *iface, unsigned int signal_id, ...) EINA_ARG_NONNULL(1);
/**
 * @brief Create signal message.
 * Each signal handler have a internal id, the first signal handler of
 * interface is = 0 the second = 1 and go on.
 * This function is used when the signal has complex types.
 *
 * @param iface interface of the signal
 * @param signal_id id of signal
 */
EAPI EDBus_Message *edbus_service_signal_new(EDBus_Service_Interface *iface, unsigned int signal_id) EINA_ARG_NONNULL(1);
/**
 * @brief Send a signal message.
 */
EAPI Eina_Bool edbus_service_signal_send(EDBus_Service_Interface *iface, EDBus_Message *signal_msg) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Store data at object path, this data can be obtained from all interfaces
 * of the same object.
 *
 * @param iface interface that belong to the object path where data will
 * be stored
 * @param key to identify data
 * @param data
 */
EAPI void edbus_service_object_data_set(EDBus_Service_Interface *iface, const char *key, const void *data);
/**
 * @brief Get data stored in object path.
 *
 * @param iface interface that belongs to the object path where data are stored
 * @param key that identify data
 *
 * @return pointer to data if found otherwise NULL
 */
EAPI void *edbus_service_object_data_get(const EDBus_Service_Interface *iface, const char *key);
/**
 * @brief Del data stored in object path.
 *
 * @param iface interface that belongs to the object path where data are stored
 * @param key that identify data
 *
 * @return pointer to data if found otherwise NULL
 */
EAPI void *edbus_service_object_data_del(EDBus_Service_Interface *iface, const char *key);
/**
 * @}
 */
#endif

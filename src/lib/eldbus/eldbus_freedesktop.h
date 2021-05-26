#ifndef ELDBUS_FREEDESKTOP_H
#define ELDBUS_FREEDESKTOP_H 1

/**
 * @defgroup Eldbus_Basic Basic FreeDesktop.Org Methods
 * @ingroup Eldbus
 *
 * @{
 */
#define ELDBUS_NAME_REQUEST_FLAG_ALLOW_REPLACEMENT 0x1 /**< Allow another service to become the primary owner if requested */
#define ELDBUS_NAME_REQUEST_FLAG_REPLACE_EXISTING  0x2 /**< Request to replace the current primary owner */
#define ELDBUS_NAME_REQUEST_FLAG_DO_NOT_QUEUE      0x4 /**< If we can not become the primary owner do not place us in the queue */

/* Replies to request for a name */
#define ELDBUS_NAME_REQUEST_REPLY_PRIMARY_OWNER    1 /**< Service has become the primary owner of the requested name */
#define ELDBUS_NAME_REQUEST_REPLY_IN_QUEUE         2 /**< Service could not become the primary owner and has been placed in the queue */
#define ELDBUS_NAME_REQUEST_REPLY_EXISTS           3 /**< Service is already in the queue */
#define ELDBUS_NAME_REQUEST_REPLY_ALREADY_OWNER    4 /**< Service is already the primary owner */

/**
 * Send a "RequestName" method call in proxy.
 *
 * @param conn Connection object.
 * @param bus Name of the bus
 * @param flags Parameter of the "RequestName" method.
 * @param cb Callback to call when receiving answer.
 * @param cb_data Data passed to callback.
 * @return The Eldbus_Pending corresponding to the message sent.
 */
EAPI Eldbus_Pending *eldbus_name_request(Eldbus_Connection *conn, const char *bus, unsigned int flags, Eldbus_Message_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 2);

/* Replies to releasing a name */
#define ELDBUS_NAME_RELEASE_REPLY_RELEASED     1    /**< Service was released from the given name */
#define ELDBUS_NAME_RELEASE_REPLY_NON_EXISTENT 2    /**< The given name does not exist on the bus */
#define ELDBUS_NAME_RELEASE_REPLY_NOT_OWNER    3    /**< Service is not an owner of the given name */

/**
 * Send a "ReleaseName" method call in proxy.
 *
 * @param conn Connection object.
 * @param bus Name of the bus
 * @param cb Callback to call when receiving answer.
 * @param cb_data Data passed to callback.
 * @return The Eldbus_Pending corresponding to the message sent.
 */
EAPI Eldbus_Pending *eldbus_name_release(Eldbus_Connection *conn, const char *bus, Eldbus_Message_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 2);

/**
 * Send a "GetNameOwner" method call in proxy.
 *
 * @param conn Connection object.
 * @param bus Name of the bus
 * @param cb Callback to call when receiving answer.
 * @param cb_data Data passed to callback.
 * @return The Eldbus_Pending corresponding to the message sent.
 */
EAPI Eldbus_Pending *eldbus_name_owner_get(Eldbus_Connection *conn, const char *bus, Eldbus_Message_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 2);

/**
 * Send a "NameHasOwner" method call in proxy.
 *
 * @param conn Connection object.
 * @param bus Name of the bus
 * @param cb Callback to call when receiving answer.
 * @param cb_data Data passed to callback.
 * @return The Eldbus_Pending corresponding to the message sent.
 */
EAPI Eldbus_Pending *eldbus_name_owner_has(Eldbus_Connection *conn, const char *bus, Eldbus_Message_Cb cb, const void *cb_data);

/**
 * Send a "ListNames" method call in proxy.
 *
 * @param conn Connection object.
 * @param cb Callback to call when receiving answer.
 * @param cb_data Data passed to callback.
 * @return The Eldbus_Pending corresponding to the message sent.
 */
EAPI Eldbus_Pending *eldbus_names_list(Eldbus_Connection *conn, Eldbus_Message_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1);

/**
 * Send a "ListActivatableNames" method call in proxy.
 *
 * @param conn Connection object.
 * @param cb Callback to call when receiving answer.
 * @param cb_data Data passed to callback.
 * @return The Eldbus_Pending corresponding to the message sent.
 */
EAPI Eldbus_Pending *eldbus_names_activatable_list(Eldbus_Connection *conn, Eldbus_Message_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1);

/**
 * Send a "Hello" method call in proxy.
 *
 * @param conn Connection object.
 * @param cb Callback to call when receiving answer.
 * @param cb_data Data passed to callback.
 * @return The Eldbus_Pending corresponding to the message sent.
 */
EAPI Eldbus_Pending *eldbus_hello(Eldbus_Connection *conn, Eldbus_Message_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1);

/* Replies to service starts */
#define ELDBUS_NAME_START_REPLY_SUCCESS         1 /**< Service was auto started */
#define ELDBUS_NAME_START_REPLY_ALREADY_RUNNING 2 /**< Service was already running */

/**
 * Send a "StartServiceByName" method call in proxy.
 *
 * @param conn Connection object.
 * @param bus Name of the bus.
 * @param flags Parameter of the "StartServiceByName" method.
 * @param cb Callback to call when receiving answer.
 * @param cb_data Data passed to callback.
 * @return The Eldbus_Pending corresponding to the message sent.
 */
EAPI Eldbus_Pending        *eldbus_name_start(Eldbus_Connection *conn, const char *bus, unsigned int flags, Eldbus_Message_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 2);

/**
 * @typedef Eldbus_Name_Owner_Changed_Cb
 *
 * Callback called when unique id of a bus name changed.
 */
typedef void (*Eldbus_Name_Owner_Changed_Cb)(void *data, const char *bus, const char *old_id, const char *new_id);

/**
 * Add a callback to be called when unique id of a bus name changed.
 *
 * This function implicitly calls eldbus_name_owner_get() in order to be able to
 * monitor the name. If the only interest is to receive notifications when the
 * name in fact changes, pass EINA_FALSE to \p allow_initial_call so your
 * callback will not be called on first retrieval of name owner. If the
 * initial state is important, pass EINA_TRUE to this parameter.
 *
 * @param conn connection
 * @param bus name of bus
 * @param cb callback
 * @param cb_data context data
 * @param allow_initial_call allow call callback with actual id of the bus
 */
EAPI void                  eldbus_name_owner_changed_callback_add(Eldbus_Connection *conn, const char *bus, Eldbus_Name_Owner_Changed_Cb cb, const void *cb_data, Eina_Bool allow_initial_call) EINA_ARG_NONNULL(1, 2, 3);
/**
 * Remove callback added with eldbus_name_owner_changed_callback_add().
 *
 * @param conn connection
 * @param bus name of bus
 * @param cb callback
 * @param cb_data context data
 */
EAPI void                  eldbus_name_owner_changed_callback_del(Eldbus_Connection *conn, const char *bus, Eldbus_Name_Owner_Changed_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @defgroup Eldbus_FDO_Peer org.freedesktop.DBus.Peer
 *
 * @{
 */

/**
 * Call the method "Ping" on the eldbus object.
 *
 * @param obj Eldbus object.
 * @param cb Callback called when receiving an answer.
 * @param data Data passed to the callback.
 * @return The Eldbus_Pending corresponding to the message sent.
 */
EAPI Eldbus_Pending        *eldbus_object_peer_ping(Eldbus_Object *obj, Eldbus_Message_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Call the method "GetMachineId" on the eldbus object.
 *
 * @param obj Eldbus object.
 * @param cb Callback called when receiving an answer.
 * @param data Data passed to the callback.
 * @return The Eldbus_Pending corresponding to the message sent.
 */
EAPI Eldbus_Pending        *eldbus_object_peer_machine_id_get(Eldbus_Object *obj, Eldbus_Message_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @}
 */

/**
 * @defgroup Eldbus_FDO_Introspectable org.freedesktop.DBus.Introspectable
 *
 * @{
 */

/**
 * Call the method "Introspect" on the eldbus object.
 *
 * @param obj Eldbus object.
 * @param cb Callback called when receiving an answer.
 * @param data Data passed to the callback.
 * @return The Eldbus_Pending corresponding to the message sent.
 */
EAPI Eldbus_Pending        *eldbus_object_introspect(Eldbus_Object *obj, Eldbus_Message_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);
/**
 * @}
 */

/**
 * @defgroup Eldbus_FDO_Properties org.freedesktop.DBus.Properties
 * @{
 */

/**
 * Enable or disable local cache of properties.
 *
 * After enable you can call eldbus_proxy_property_local_get() or
 * eldbus_proxy_property_local_get_all() to get cached properties.
 *
 * @param proxy bus+path+interface that the properties belong
 * @param enable enable or disable properties monitor
 * @return EINA_TRUE if already have cached properties
 * EINA_FALSE if it will asynchrony get the properties.
 * You should listen for a ELDBUS_PROXY_EVENT_PROPERTY_LOADED
 * to know when properties finish to load.
 */
EAPI Eina_Bool eldbus_proxy_properties_monitor(Eldbus_Proxy *proxy, Eina_Bool enable) EINA_ARG_NONNULL(1);

/**
 * Get a property.
 *
 * @param proxy The proxy object on which to do the query.
 * @param name The property name to get.
 * @param cb The callback to be called when receiving an answer.
 * @param data Data to be passed to the callback.
 * @return Eldbus_Pending object corresponding to the message sent.
 */
EAPI Eldbus_Pending        *eldbus_proxy_property_get(Eldbus_Proxy *proxy, const char *name, Eldbus_Message_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * Set a property.
 *
 * @param proxy The proxy object on which to do the query.
 * @param name The property name to get.
 * @param sig
 * @param value The value to set.
 * @param cb The callback to be called when receiving an answer.
 * @param data Data to be passed to the callback.
 * @return Eldbus_Pending object corresponding to the message sent.
 */
EAPI Eldbus_Pending        *eldbus_proxy_property_set(Eldbus_Proxy *proxy, const char *name, const char *sig, const void *value, Eldbus_Message_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2, 3, 4);

/**
 * Set a property with a Eina_Value.
 *
 * @param proxy The proxy object on which to do the query.
 * @param name The property name to get.
 * @param sig
 * @param value The value to set.
 * @param cb The callback to be called when receiving an answer.
 * @param data Data to be passed to the callback.
 * @return Eldbus_Pending object corresponding to the message sent.
 *
 * @since 1.17
 */
EAPI Eldbus_Pending        *eldbus_proxy_property_value_set(Eldbus_Proxy *proxy, const char *name, const char *sig, const Eina_Value *value, Eldbus_Message_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2, 3, 4);

/**
 * Get all properties.
 *
 * @param proxy The proxy object on which to do the query.
 * @param cb The callback to be called when receiving an answer.
 * @param data Data to be passed to the callback.
 * @return Eldbus_Pending object corresponding to the message sent.
 */
EAPI Eldbus_Pending        *eldbus_proxy_property_get_all(Eldbus_Proxy *proxy, Eldbus_Message_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Register a callback on "PropertiesChanged" signal.
 *
 * @param proxy The proxy object on which to register the callback.
 * @param cb The callback to be called when receiving the signal.
 * @param data Data to be passed to the callback.
 * @return Eldbus_Signal_Handler object representing a listener for "PropertiesChanged" signal.
 */
EAPI Eldbus_Signal_Handler *eldbus_proxy_properties_changed_callback_add(Eldbus_Proxy *proxy, Eldbus_Signal_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Return the cached value of property.
 *
 * This only work if you have enable eldbus_proxy_properties_monitor or
 * if you have call eldbus_proxy_event_callback_add of type
 * ELDBUS_PROXY_EVENT_PROPERTY_CHANGED and the property you want had changed.
 *
 * @param proxy The proxy object on which to do the query.
 * @param name The property name to get.
 * @return Cached value of property.
 */
EAPI Eina_Value           *eldbus_proxy_property_local_get(Eldbus_Proxy *proxy, const char *name) EINA_ARG_NONNULL(1, 2);

/**
 * Get all cached properties.
 *
 * This only work if you have enable eldbus_proxy_properties_monitor or
 * if you have call eldbus_proxy_event_callback_add of type
 * ELDBUS_PROXY_EVENT_PROPERTY_CHANGED.
 *
 * @param proxy The proxy object on which to do the query.
 * @return A Eina_Hash with all cached properties
 */
EAPI const Eina_Hash      *eldbus_proxy_property_local_get_all(Eldbus_Proxy *proxy) EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @defgroup Eldbus_FDO_ObjectManager org.freedesktop.DBus.ObjectManager *
 * @{
 */

/**
 * Call the method "GetManagedObjects" on the eldbus object.
 *
 * @param obj A Eldbus object.
 * @param cb The callback to call when receiving an answer.
 * @param data The data to pass to the callback.
 *
 * @return A Eldbus_Pending object corresponding to the message sent.
 */
EAPI Eldbus_Pending        *eldbus_object_managed_objects_get(Eldbus_Object *obj, Eldbus_Message_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Register a callback on "InterfacesAdded" signal.
 *
 * @param obj The Eldbus object on which to register the callback.
 * @param cb The callback to be called when receiving the signal.
 * @param cb_data Data to be passed to the callback.
 * @return Eldbus_Signal_Handler object representing a listener for "InterfacesAdded" signal.
 */
EAPI Eldbus_Signal_Handler *eldbus_object_manager_interfaces_added(Eldbus_Object *obj, Eldbus_Signal_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1);

/**
 * Register a callback on "InterfacesRemoved" signal.
 *
 * @param obj The Eldbus object on which to register the callback.
 * @param cb The callback to be called when receiving the signal.
 * @param cb_data Data to be passed to the callback.
 * @return Eldbus_Signal_Handler object representing a listener for "InterfacesRemoved" signal.
 */
EAPI Eldbus_Signal_Handler *eldbus_object_manager_interfaces_removed(Eldbus_Object *obj, Eldbus_Signal_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @}
 */
#endif

#ifndef _EFL_NET_CONNMAN_H_
#define _EFL_NET_CONNMAN_H_ 1

#include "Ecore.h"
#include "ecore_con_private.h"

#include "Eldbus.h"

#define DEFAULT_TIMEOUT 10000.0

/**
 * @file efl_net-connman.h
 *
 * Common infrastructure to create Efl_Net_Control_Manager and Efl_Net_Session
 * based on ConnMan connection manager.
 *
 * @note Each connection manager that needs shared infra should create
 * their own file!
 *
 * @internal
 */

/**
 * Should be called from inside Efl.Object.constructor before using
 * any of connman functions.
 *
 * @return #EINA_FALSE on errors, all other functions will be useless
 * in this case. #EINA_TRUE on success.
 *
 * @internal
 */
Eina_Bool efl_net_connman_init(void);

/**
 * Should be called from inside Efl.Object.destructor after done using
 * all of connman functions.
 *
 * @internal
 */
void efl_net_connman_shutdown(void);

/**
 * Returns the DBus connection shared by all objects.
 *
 * @internal
 */
Eldbus_Connection *efl_net_connman_connection_get(void);

/**
 * Returns a singleton for ConnMan's Manager object and interface
 * (proxy).
 *
 * @note call efl_net_connman_init() before using. There is no need to
 * eldbus_proxy_ref() it, but if done should have a matching
 * eldbus_proxy_unref(). After done, remember to
 * efl_net_connman_shutdown().
 *
 * @internal
 */
Eldbus_Proxy *efl_net_connman_manager_get(void);

/**
 * Given a Efl.Net.Control find a technology instance given its name.
 *
 * @internal
 */
Efl_Net_Control_Technology *efl_net_connman_control_find_technology_by_type(Efl_Net_Control_Manager *ctl, const char *tech_type);

/**
 * Ask Efl.Net.Control to reload access point list.
 *
 * @internal
 */
void efl_net_connman_control_access_points_reload(Efl_Net_Control_Manager *ctl);


/**
 * Creates a new Efl.Net.Control.Technology child of an
 * Efl.Net.Control with path and properties
 *
 * @internal
 */
Efl_Net_Control_Technology *efl_net_connman_technology_new(Efl_Net_Control_Manager *parent, const char *path, Eldbus_Message_Iter *properties);

/**
 * Get the path of the given technology.
 *
 * @internal
 */
const char *efl_net_connman_technology_path_get(Efl_Net_Control_Technology *tech);

/**
 * Convert connman's type string to enum value.
 *
 * @internal
 */
Efl_Net_Control_Technology_Type efl_net_connman_technology_type_from_str(const char *tech_type);

/**
 * Creates a new Efl.Net.Control.Access_Point child of an
 * Efl.Net.Control with path and properties
 *
 * @internal
 */
Efl_Net_Control_Access_Point *efl_net_connman_access_point_new(Efl_Net_Control_Manager *parent, const char *path, Eldbus_Message_Iter *properties, unsigned int priority);

/**
 * Get the path of the given access_point.
 *
 * @internal
 */
const char *efl_net_connman_access_point_path_get(Efl_Net_Control_Access_Point *ap);

/**
 * Updates Efl.Net.Control.Access_Point properties
 *
 * @internal
 */
void efl_net_connman_access_point_update(Efl_Net_Control_Manager *ap, Eldbus_Message_Iter *properties, unsigned int priority);


#endif /* _EFL_NET_CONNMAN_H_ */

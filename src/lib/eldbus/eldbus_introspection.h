#ifndef _ELDBUS_INTROSPECTION_INTROSPECTION_H
#define _ELDBUS_INTROSPECTION_INTROSPECTION_H

#include <Eina.h>
#include <Eo.h>

#include "eldbus_types.eot.h"

/**
 * @brief Parses the introspection xml abstracting it to an object tree
 *
 * @param xml The introspection xml
 * @return The introspection object tree
 *
 * @since 1.17
 */
EAPI Eldbus_Introspection_Node *eldbus_introspection_parse(const char *xml);

/**
 * @brief Frees the introspection object tree
 *
 * @param node The root node of introspection tree
 *
 * @since 1.17
 */
EAPI void eldbus_introspection_node_free(Eldbus_Introspection_Node *node);

/**
 * @brief Finds an interface by name
 *
 * @param interfaces The list of interfaces of type @c Eldbus_Introspection_Interface
 * @param name The interfaces's name to search for
 * @return Returns the interface found or @c NULL if not
 *
 * @since 1.17
 */
EAPI Eldbus_Introspection_Interface *eldbus_introspection_interface_find(Eina_List *interfaces, const char *name);

/**
 * @brief Finds a property by name
 *
 * @param properties The list of properties of type @c Eldbus_Introspection_Property
 * @param name The properties's name to search for
 * @return Returns the property found or @c NULL if not
 *
 * @since 1.17
 */
EAPI Eldbus_Introspection_Property *eldbus_introspection_property_find(Eina_List *properties, const char *name);

/**
 * @brief Finds an argument by name
 *
 * @param arguments The list of arguments of type @c Eldbus_Introspection_Property
 * @param name The arguments's name to search for
 * @return Returns the argument found or @c NULL if not
 *
 * @since 1.17
 */
EAPI Eldbus_Introspection_Argument *eldbus_introspection_argument_find(Eina_List *arguments, const char *name);

#endif

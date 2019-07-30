#ifndef _ELDBUS_INTROSPECTION_INTROSPECTION_H
#define _ELDBUS_INTROSPECTION_INTROSPECTION_H

#include <Eina.h>
#include <Eo.h>

/* FIXME: these are duplicated as @extern in eldbus_types.eot */

/** Argument direction
 *
 * @ingroup Eldbus_Introspection
 */
typedef enum
{
  ELDBUS_INTROSPECTION_ARGUMENT_DIRECTION_NONE = 0, /**< No direction */
  ELDBUS_INTROSPECTION_ARGUMENT_DIRECTION_IN, /**< Incoming direction */
  ELDBUS_INTROSPECTION_ARGUMENT_DIRECTION_OUT /**< Outgoing direction */
} Eldbus_Introspection_Argument_Direction;

/** Property access rights
 *
 * @ingroup Eldbus_Introspection
 */
typedef enum
{
  ELDBUS_INTROSPECTION_PROPERTY_ACCESS_READ = 0, /**< Property can be read */
  ELDBUS_INTROSPECTION_PROPERTY_ACCESS_WRITE, /**< Property can be written */
  ELDBUS_INTROSPECTION_PROPERTY_ACCESS_READWRITE /**< Property can be read and
                                                  * written */
} Eldbus_Introspection_Property_Access;

/** DBus Node
 *
 * @ingroup Eldbus_Introspection
 */
typedef struct _Eldbus_Introspection_Node
{
  Eina_Stringshare *name; /**< Node name (optional) */
  Eina_List *nodes; /**< List with nodes */
  Eina_List *interfaces; /**< List with interfaces */
} Eldbus_Introspection_Node;

/** DBus Interface
 *
 * @ingroup Eldbus_Introspection
 */
typedef struct _Eldbus_Introspection_Interface
{
  Eina_Stringshare *name; /**< Interface name */
  Eina_List *methods; /**< List with interface methods */
  Eina_List *signals; /**< List with interface signals */
  Eina_List *properties; /**< List with interface properties */
  Eina_List *annotations; /**< List with interface annotations */
} Eldbus_Introspection_Interface;

/** DBus Method
 *
 * @ingroup Eldbus_Introspection
 */
typedef struct _Eldbus_Introspection_Method
{
  Eina_Stringshare *name; /**< Method name */
  Eina_List *arguments; /**< List with method arguments */
  Eina_List *annotations; /**< List with method annotations */
} Eldbus_Introspection_Method;

/** DBus Property
 *
 * @ingroup Eldbus_Introspection
 */
typedef struct _Eldbus_Introspection_Property
{
  Eina_Stringshare *name; /**< Property name */
  Eina_Stringshare *type; /**< Property type */
  Eldbus_Introspection_Property_Access access; /**< Property access rights */
  Eina_List *annotations; /**< List with property annotations */
} Eldbus_Introspection_Property;

/** DBus Annotation
 *
 * @ingroup Eldbus_Introspection
 */
typedef struct _Eldbus_Introspection_Annotation
{
  Eina_Stringshare *name; /**< Annotation name */
  Eina_Stringshare *value; /**< Annotation value */
} Eldbus_Introspection_Annotation;

/** DBus Argument
 *
 * @ingroup Eldbus_Introspection
 */
typedef struct _Eldbus_Introspection_Argument
{
  Eina_Stringshare *name; /**< Argument name (optional) */
  Eina_Stringshare *type; /**< Argument type */
  Eldbus_Introspection_Argument_Direction direction; /**< Argument direction */
} Eldbus_Introspection_Argument;

/** DBus Signal
 *
 * @ingroup Eldbus_Introspection
 */
typedef struct _Eldbus_Introspection_Signal
{
  Eina_Stringshare *name; /**< Signal name */
  Eina_List *arguments; /**< List with signal arguments */
  Eina_List *annotations; /**< List with signal annotations */
} Eldbus_Introspection_Signal;

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

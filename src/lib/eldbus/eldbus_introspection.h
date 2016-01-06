#ifndef _ELDBUS_INTROSPECTION_INTROSPECTION_H
#define _ELDBUS_INTROSPECTION_INTROSPECTION_H

#include <Eina.h>

// DTD conversion form: http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd

typedef struct _Eldbus_Introspection_Node Eldbus_Introspection_Node;
typedef struct _Eldbus_Introspection_Interface Eldbus_Introspection_Interface;
typedef struct _Eldbus_Introspection_Method Eldbus_Introspection_Method;
typedef struct _Eldbus_Introspection_Signal Eldbus_Introspection_Signal;
typedef struct _Eldbus_Introspection_Argument Eldbus_Introspection_Argument;
typedef struct _Eldbus_Introspection_Property Eldbus_Introspection_Property;
typedef struct _Eldbus_Introspection_Annotation Eldbus_Introspection_Annotation;

typedef enum
{
   ELDBUS_INTROSPECTION_ARGUMENT_DIRECTION_NONE = 0,
   ELDBUS_INTROSPECTION_ARGUMENT_DIRECTION_IN,
   ELDBUS_INTROSPECTION_ARGUMENT_DIRECTION_OUT,

} Eldbus_Introspection_Argument_Direction;

typedef enum
{
   ELDBUS_INTROSPECTION_PROPERTY_ACCESS_READ,
   ELDBUS_INTROSPECTION_PROPERTY_ACCESS_WRITE,
   ELDBUS_INTROSPECTION_PROPERTY_ACCESS_READWRITE,
} Eldbus_Introspection_Property_Access;

struct _Eldbus_Introspection_Node
{
   Eina_Stringshare *name; // optional
   Eina_List *nodes;
   Eina_List *interfaces;
};

struct _Eldbus_Introspection_Interface
{
   Eina_Stringshare *name;
   Eina_List *methods;
   Eina_List *signals;
   Eina_List *properties;
   Eina_List *annotations;
};

struct _Eldbus_Introspection_Method
{
   Eina_Stringshare *name;
   Eina_List *arguments;
   Eina_List *annotations;
};

struct _Eldbus_Introspection_Signal
{
   Eina_Stringshare *name;
   Eina_List *arguments;
   Eina_List *annotations;
};

struct _Eldbus_Introspection_Argument
{
   Eina_Stringshare *name; // optional
   Eina_Stringshare *type;
   Eldbus_Introspection_Argument_Direction direction;
};

struct _Eldbus_Introspection_Property
{
   Eina_Stringshare *name;
   Eina_Stringshare *type;
   Eldbus_Introspection_Property_Access access;
   Eina_List *annotations;
};

struct _Eldbus_Introspection_Annotation
{
   Eina_Stringshare *name;
   Eina_Stringshare *value;
};

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

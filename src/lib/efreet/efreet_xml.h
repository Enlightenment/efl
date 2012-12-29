#ifndef EFREET_XML_H
#define EFREET_XML_H

/**
 * @internal
 * @file efreet_xml.h
 * @brief A simple and fast XML parser
 * @addtogroup Efreet_Xml Efreet_Xml: An XML parser (private)
 * @ingroup Efreet
 *
 * Consider using @ref Eina_Simple_XML_Group instead.
 *
 * @private
 *
 * @{
 */

/**
 * Efreet_Xml_Attributes
 */
typedef struct Efreet_Xml_Attribute Efreet_Xml_Attribute;

/**
 * Efreet_Xml_Attributes
 * @brief Contains information about a given XML attribute
 */
struct Efreet_Xml_Attribute
{
    const char *key;        /**< The attribute key */
    const char *value;      /**< The attribute value */
};

/**
 * Efreet_Xml
 */
typedef struct Efreet_Xml Efreet_Xml;

/**
 * Efreet_Xml
 * @brief Contains the XML tree for a given XML document
 */
struct Efreet_Xml
{
    const char *text;                   /**< The XML text for this node */
    const char *tag;                    /**< The tag for this node */

    Efreet_Xml_Attribute **attributes;  /**< The attributes for this node */

    Eina_List *children;               /**< Child nodes */
};

int efreet_xml_init(void);
void efreet_xml_shutdown(void);

Efreet_Xml *efreet_xml_new(const char *file);
void efreet_xml_del(Efreet_Xml *xml);

const char *efreet_xml_attribute_get(Efreet_Xml *xml, const char *key);

/**
 * @}
 */

#endif

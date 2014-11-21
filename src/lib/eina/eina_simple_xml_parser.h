/* EINA - EFL data type library
 * Copyright (C) 2011 Gustavo Sverzut Barbieri
 *                    Cedric Bail
 *
 * This library is a free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EINA_SIMPLE_XML_H_
#define EINA_SIMPLE_XML_H_

#include "eina_config.h"

#include "eina_types.h"
#include "eina_magic.h"
#include "eina_inlist.h"

/**
 * @defgroup Eina_Simple_XML_Group Simple_XML
 * @ingroup Eina_Tools_Group
 *
 * @brief This is a simplistic relaxed SAX-like XML parser.
 *
 * This parser is far from being compliant with XML standards, but
 * works for most XMLs out there. If you know that your format is simple
 * and won't vary in the future with strange corner cases, then you can
 * use it safely.
 *
 * The parser is SAX like, that is, it tokenizes content and calls
 * you back so that you can take some action. No content is allocated
 * during this parser work and it's not recursive, so you can use it
 * with a very large document without any issues.
 *
 * It does not validate the document anyhow, neither does it create a
 * tree hierarchy. That's up to you.
 *
 * Accordingly to XML, open tags may contain attributes. This parser
 * does not tokenize this. If you want you can use
 * eina_simple_xml_tag_attributes_find() and then
 * eina_simple_xml_attributes_parse().
 *
 * @{
 */

typedef struct _Eina_Simple_XML_Node      Eina_Simple_XML_Node;
typedef struct _Eina_Simple_XML_Node_Tag  Eina_Simple_XML_Node_Root;
typedef struct _Eina_Simple_XML_Node_Tag  Eina_Simple_XML_Node_Tag;
typedef struct _Eina_Simple_XML_Node_Data Eina_Simple_XML_Node_Data;
typedef struct _Eina_Simple_XML_Node_Data Eina_Simple_XML_Node_CData;
typedef struct _Eina_Simple_XML_Node_Data Eina_Simple_XML_Node_Processing;
typedef struct _Eina_Simple_XML_Node_Data Eina_Simple_XML_Node_Doctype;
typedef struct _Eina_Simple_XML_Node_Data Eina_Simple_XML_Node_Doctype_Child; /**< @since 1.8 */
typedef struct _Eina_Simple_XML_Node_Data Eina_Simple_XML_Node_Comment;
typedef struct _Eina_Simple_XML_Attribute Eina_Simple_XML_Attribute;

struct _Eina_Simple_XML_Attribute
{
   EINA_INLIST;
   EINA_MAGIC;

   Eina_Simple_XML_Node_Tag *parent;
   const char *key;
   const char *value;
};

/**
 * @typedef _Eina_Simple_XML_Node_Type
 * @brief Enumeration for a simple XML node type.
 */
typedef enum _Eina_Simple_XML_Node_Type
{
  EINA_SIMPLE_XML_NODE_ROOT = 0,
  EINA_SIMPLE_XML_NODE_TAG,
  EINA_SIMPLE_XML_NODE_DATA,
  EINA_SIMPLE_XML_NODE_CDATA,
  EINA_SIMPLE_XML_NODE_PROCESSING,
  EINA_SIMPLE_XML_NODE_DOCTYPE,
  EINA_SIMPLE_XML_NODE_COMMENT,
  EINA_SIMPLE_XML_NODE_DOCTYPE_CHILD, /**< @since 1.8 */
} Eina_Simple_XML_Node_Type;

struct _Eina_Simple_XML_Node
{
   EINA_INLIST;
   EINA_MAGIC;

   Eina_Simple_XML_Node_Tag *parent;
   Eina_Simple_XML_Node_Type type;
};

struct _Eina_Simple_XML_Node_Tag
{
   Eina_Simple_XML_Node base;
   Eina_Inlist *children;
   Eina_Inlist *attributes;
   const char *name;
};

struct _Eina_Simple_XML_Node_Data
{
   Eina_Simple_XML_Node base;
   size_t length;
   char data[];
};

/**
 * @typedef _Eina_Simple_XML_Type
 * @brief Enumeration for a simple XML type.
 */
typedef enum _Eina_Simple_XML_Type
{
  EINA_SIMPLE_XML_OPEN = 0, /**< \<tag attribute="value"\> */
  EINA_SIMPLE_XML_OPEN_EMPTY, /**< \<tag attribute="value" /\> */
  EINA_SIMPLE_XML_CLOSE, /**< \</tag\> */
  EINA_SIMPLE_XML_DATA, /**< tag text data */
  EINA_SIMPLE_XML_CDATA, /**< \<![CDATA[something]]\> */
  EINA_SIMPLE_XML_ERROR, /**< Error contents */
  EINA_SIMPLE_XML_PROCESSING, /**< \<?xml ... ?\> \<?php .. ?\> */
  EINA_SIMPLE_XML_DOCTYPE, /**< \<!DOCTYPE html */
  EINA_SIMPLE_XML_COMMENT, /**< \<!-- something --\> */
  EINA_SIMPLE_XML_IGNORED, /**< Whatever is ignored by the parser, like whitespace */
  EINA_SIMPLE_XML_DOCTYPE_CHILD /**< \<!DOCTYPE_CHILD @since 1.8 */
} Eina_Simple_XML_Type;

typedef Eina_Bool (*Eina_Simple_XML_Cb)(void *data, Eina_Simple_XML_Type type, const char *content, unsigned offset, unsigned length);
typedef Eina_Bool (*Eina_Simple_XML_Attribute_Cb)(void *data, const char *key, const char *value);


/**
 * @brief Parses a section of the XML string text.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The input string \n
 *            May not contain the \0 terminator.
 * @param[in] buflen The input string size
 * @param[in] strip The boolean value that indicates when this parser should do strip leading and whitespace trailing  \n
 *              This whitespace is still issued, but as type
 *              #EINA_SIMPLE_XML_IGNORED.
 * @param[in] func The function to call back while parse does some action \n
 *             The first parameter is the given user @a data, the second is the
 *             token type, the third is the pointer to the content's start (it's
 *             not a NULL terminated string), the fourth is where this
 *             content is located, that is @a buf (does not include tag
 *             start, for instance "<!DOCTYPE value>" the offset points at
 *             "value"), the fifth is the size of the content \n
 *             Whenever this function returns @c EINA_FALSE the parser aborts.
 * @param[in] data The data to give as context to @a func
 *
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE if it is aborted by the user or a
 *         parsing error occurs
 */
EAPI Eina_Bool eina_simple_xml_parse(const char *buf, unsigned buflen,
				     Eina_Bool strip,
				     Eina_Simple_XML_Cb func, const void *data);


/**
 * @brief Finds where the attributes start from given the contents of a tag are provided.
 *
 * @since_tizen 2.3
 *
 * @remarks The tag contents are returned by eina_simple_xml_parse() when
 *          type is #EINA_SIMPLE_XML_OPEN or #EINA_SIMPLE_XML_OPEN_EMPTY.
 *
 * @param[in] buf The input string \n
 *            May not contain \0 terminator.
 * @param[in] buflen The input string size
 * @return A pointer to the start of the attributes, it can be used
 *         to feed eina_simple_xml_attributes_parse() \n
 *         @c NULL is returned if no attributes are found.
 *
 */
EAPI const char * eina_simple_xml_tag_attributes_find(const char *buf, unsigned buflen);

/**
 * @brief Parses a buffer with XML attributes to key=value pairs.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The input string \n
 *            May not contain \0 terminator.
 * @param[in] buflen The input string size
 * @param[in] func The function to call back while parse does some action \n
 *             The first parameter is the given user @a data, the second is the
 *             key (null-terminated) and the last is the value (null
 *             terminated) \n
 *             These strings should not be modified and the
 *             reference is just valid till the function returns
 * @param[in] data The data to pass to the callback function
 *
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE if it is aborted by the user or a
 *         parsing error occurs
 */
EAPI Eina_Bool eina_simple_xml_attributes_parse(const char *buf, unsigned buflen,
						Eina_Simple_XML_Attribute_Cb func, const void *data);

/**
 * @brief Creates (and appends) a new attribute to the tag.
 *
 * @since_tizen 2.3
 *
 * @param[in] parent If provided, this is set in the resulting structure
 *               and the attribute is appended to the attributes list
 * @param[in] key The null-terminated string \n
 *            Must not be @c NULL.
 * @param[in] value The null-terminated string \n
 *              If @c NULL, the empty string is used.
 *
 * @return The newly allocated memory, otherwise @c NULL on error \n
 *         This memory should be released directly with eina_simple_xml_attribute_free() or indirectly
 *         with eina_simple_xml_node_tag_free().
 */
EAPI Eina_Simple_XML_Attribute * eina_simple_xml_attribute_new(Eina_Simple_XML_Node_Tag *parent, const char *key, const char *value);

/**
 * @brief Removes an attribute from the parent and deletes it.
 *
 * @since_tizen 2.3
 *
 * @param[in] attr The attribute to release memory of
 */
EAPI void eina_simple_xml_attribute_free(Eina_Simple_XML_Attribute *attr);


/**
 * @brief Creates a new tag. If @a parent is provided, it is automatically appended.
 *
 * @since_tizen 2.3
 *
 * @param[in] parent If provided, this is set in the resulting structure
 *               and the tag is appended to the children list
 * @param[in] name The null-terminated string \n
 *             Must not be @c NULL.
 *
 * @return The newly allocated memory, otherwise @c NULL on error \n
 *         This memory of the parent should be released directly with eina_simple_xml_node_tag_free() or indirectly
 *         with eina_simple_xml_node_tag_free().
 */
EAPI Eina_Simple_XML_Node_Tag * eina_simple_xml_node_tag_new(Eina_Simple_XML_Node_Tag *parent, const char *name);

/**
 * @brief Removes a tag from the parent and deletes it.
 *
 * @since_tizen 2.3
 *
 * @param[in] tag The tag to release memory of
 */
EAPI void eina_simple_xml_node_tag_free(Eina_Simple_XML_Node_Tag *tag);


/**
 * @brief Creates new data. If @a parent is provided, it is automatically appended.
 *
 * @since_tizen 2.3
 *
 * @param[in] parent If provided, this is set in the resulting structure
 *               and the data is appended to the children list
 * @param[in] contents The string to be used \n
 *                 Must not be @c NULL.
 * @param[in] length size in bytes of @a contents
 *
 * @return The newly allocated memory, otherwise @c NULL on error \n
 *         This memory of the parent should be released directly with eina_simple_xml_node_data_free() or indirectly
 *         with eina_simple_xml_node_tag_free().
 */
EAPI Eina_Simple_XML_Node_Data * eina_simple_xml_node_data_new(Eina_Simple_XML_Node_Tag *parent, const char *contents, size_t length);

/**
 * @brief Removes data from the parent and deletes it.
 *
 * @since_tizen 2.3
 *
 * @param[in] node The data to release memory of
 */
EAPI void eina_simple_xml_node_data_free(Eina_Simple_XML_Node_Data *node);


/**
 * @brief Creates new cdata. If @a parent is provided, it is automatically appended.
 *
 * @since_tizen 2.3
 *
 * @param[in] parent If provided, this is set in the resulting structure
 *               and the cdata is appended to the children list
 * @param[in] contents The string to be used \n
 *                 Must not be @c NULL.
 * @param[in] length size in bytes of @a contents
 *
 * @return The newly allocated memory, otherwise @c NULL on error \n
 *         This memory of the parent should be released directly with eina_simple_xml_node_cdata_free() or indirectly
 *         with eina_simple_xml_node_tag_free().
 */
EAPI Eina_Simple_XML_Node_CData * eina_simple_xml_node_cdata_new(Eina_Simple_XML_Node_Tag *parent, const char *contents, size_t length);

/**
 * @brief Removes cdata from the parent and deletes it.
 *
 * @since_tizen 2.3
 *
 * @param[in] node The cdata to release memory of
 */
EAPI void eina_simple_xml_node_cdata_free(Eina_Simple_XML_Node_Data *node);


/**
 * @brief Creates a new doctype child. If @a parent is provided, it is automatically appended.
 *
 * @since 1.8
 *
 * @since_tizen 2.3
 *
 * @param[in] parent If provided, this is set in the resulting structure
 *               and the doctype child is appended to the children list
 * @param[in] contents The string to be used \n
 *                 Must not be @c NULL.
 * @param[in] length size in bytes of @a contents
 *
 * @return The newly allocated memory, otherwise @c NULL on error \n
 *         This memory of the parent should be released directly with eina_simple_xml_node_doctype_child_free() or indirectly
 *         with eina_simple_xml_node_tag_free().
 *
 */
EAPI Eina_Simple_XML_Node_Doctype_Child * eina_simple_xml_node_doctype_child_new(Eina_Simple_XML_Node_Tag *parent, const char *contents, size_t length);

/**
 * @brief Removes the doctype child from the parent and deletes it.
 *
 * @since 1.8
 *
 * @since_tizen 2.3
 *
 * @param[in] node The doctype child to release memory of
 *
 */
EAPI void eina_simple_xml_node_doctype_child_free(Eina_Simple_XML_Node_Data *node);


/**
 * @brief Creates new processing. If @a parent is provided, it is automatically appended.
 *
 * @since_tizen 2.3
 *
 * @param[in] parent If provided, this is set in the resulting structure
 *               and the processing is appended to the children list
 * @param[in] contents The string to be used \n
 *                 Must not be @c NULL.
 * @param[in] length size in bytes of @a contents
 *
 * @return The newly allocated memory, otherwise @c NULL on error \n
 *         This memory of the parent should be released directly with eina_simple_xml_node_processing_free() or indirectly
 *         with eina_simple_xml_node_tag_free().
 */
EAPI Eina_Simple_XML_Node_Processing * eina_simple_xml_node_processing_new(Eina_Simple_XML_Node_Tag *parent, const char *contents, size_t length);

/**
 * @brief Removes processing from the parent and deletes it.
 *
 * @since_tizen 2.3
 *
 * @param[in] node The processing to release memory of
 */
EAPI void eina_simple_xml_node_processing_free(Eina_Simple_XML_Node_Data *node);


/**
 * @brief Creates a new doctype. If @a parent is provided, it is automatically appended.
 *
 * @since_tizen 2.3
 *
 * @param[in] parent If provided, this is set in the resulting structure
 *               and the doctype is appended to the children list
 * @param[in] contents The string to be used \n
 *                 Must not be @c NULL.
 * @param[in] length size in bytes of @a contents
 *
 * @return The newly allocated memory, otherwise @c NULL on error \n
 *         This memory of the parent should be released directly with eina_simple_xml_node_doctype_free() or indirectly
 *         with eina_simple_xml_node_tag_free().
 */
EAPI Eina_Simple_XML_Node_Doctype * eina_simple_xml_node_doctype_new(Eina_Simple_XML_Node_Tag *parent, const char *contents, size_t length);

/**
 * @brief Removes the doctype from the parent and deletes it.
 *
 * @since_tizen 2.3
 *
 * @param[in] node The doctype to release memory of
 */
EAPI void eina_simple_xml_node_doctype_free(Eina_Simple_XML_Node_Data *node);


/**
 * @brief Creates a new comment. If @a parent is provided, it is automatically appended.
 *
 * @since_tizen 2.3
 *
 * @param[in] parent If provided, this is set in the resulting structure
 *               and the comment is appended to the children list
 * @param[in] contents The string to be used \n
 *                 Must not be @c NULL.
 * @param[in] length size in bytes of @a contents
 *
 * @return The newly allocated memory, otherwise @c NULL on error \n
 *         This memory of the parent should be released directly with eina_simple_xml_node_comment_free() or indirectly
 *         with eina_simple_xml_node_tag_free().
 */
EAPI Eina_Simple_XML_Node_Comment * eina_simple_xml_node_comment_new(Eina_Simple_XML_Node_Tag *parent, const char *contents, size_t length);

/**
 * @brief Removes a comment from the parent and deletes it.
 *
 * @since_tizen 2.3
 *
 * @param[in] node The comment to release memory of
 */
EAPI void eina_simple_xml_node_comment_free(Eina_Simple_XML_Node_Data *node);


/**
 * @brief Loads an XML node tree based on the given string.
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The input string \n
 *            May not contain \0 terminator.
 * @param[in] buflen The input string size
 * @param[in] strip The boolean value that indicates when this parser should do
 *            strip leading and whitespace trailing
 *
 * @return The document root with children tags, otherwise @c NULL on errors \n
 *         The document with errors may return a partial tree instead of @c NULL,
 *         we are going to do our best to avoid returning nothing.
 */
EAPI Eina_Simple_XML_Node_Root * eina_simple_xml_node_load(const char *buf, unsigned buflen, Eina_Bool strip);

/**
 * @brief Frees the node tree built with eina_simple_xml_node_load().
 *
 * @since_tizen 2.3
 *
 * @param[in] root The memory returned by eina_simple_xml_node_load()
 */
EAPI void eina_simple_xml_node_root_free(Eina_Simple_XML_Node_Root *root);

/**
 * @brief Converts the node tree under the given element to an XML string.
 *
 * @since_tizen 2.3
 *
 * @param[in] node The base node to convert
 * @param[in] indent The indentation string, otherwise @c NULL to disable it
 *
 * @return @c NULL on errors, otherwise a newly allocated string on success
 */
EAPI char * eina_simple_xml_node_dump(Eina_Simple_XML_Node *node, const char *indent);

/**
 * @}
 */

#endif /* EINA_SIMPLE_XML_H_ */

/* EINA - EFL data type library
 * Copyright (C) 2011 Gustavo Sverzut Barbieri
 *                    Cedric Bail
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
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
 *
 * Simplistic relaxed SAX-like XML parser.
 *
 * This parser is far from being compliant with XML standard, but will
 * do for most XMLs out there. If you know that your format is simple
 * and will not vary in future with strange corner cases, then you can
 * use it safely.
 *
 * The parser is SAX like, that is, it will tokenize contents and call
 * you back so you can take some action. No contents are allocated
 * during this parser work and it's not recursive, so you can use it
 * with a very large document without worries.
 *
 * It will not validate the document anyhow, neither it will create a
 * tree hierarchy. That's up to you.
 *
 * Accordingly to XML, open tags may contain attributes. This parser
 * will not tokenize this. If you want you can use
 * eina_simple_xml_tag_attributes_find() and then
 * eina_simple_xml_attributes_parse().
 *
 * @{
 */

/**
 * @addtogroup Eina_Tools_Group Tools
 *
 * @{
 */

/**
 * @defgroup Eina_Simple_XML_Group Simple_XML
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

typedef enum _Eina_Simple_XML_Node_Type
{
  EINA_SIMPLE_XML_NODE_ROOT = 0,
  EINA_SIMPLE_XML_NODE_TAG,
  EINA_SIMPLE_XML_NODE_DATA,
  EINA_SIMPLE_XML_NODE_CDATA,
  EINA_SIMPLE_XML_NODE_PROCESSING,
  EINA_SIMPLE_XML_NODE_DOCTYPE,
  EINA_SIMPLE_XML_NODE_COMMENT
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

typedef enum _Eina_Simple_XML_Type
{
  EINA_SIMPLE_XML_OPEN = 0, /* <tag attribute="value"> */
  EINA_SIMPLE_XML_OPEN_EMPTY, /* <tag attribute="value" /> */
  EINA_SIMPLE_XML_CLOSE, /* </tag> */
  EINA_SIMPLE_XML_DATA, /* tag text data */
  EINA_SIMPLE_XML_CDATA, /* <![CDATA[something]]> */
  EINA_SIMPLE_XML_ERROR, /* error contents */
  EINA_SIMPLE_XML_PROCESSING, /* <?xml ... ?> <?php .. ?> */
  EINA_SIMPLE_XML_DOCTYPE, /* <!DOCTYPE html */
  EINA_SIMPLE_XML_COMMENT, /* <!-- something --> */
  EINA_SIMPLE_XML_IGNORED /* whatever is ignored by parser, like whitespace */
} Eina_Simple_XML_Type;

typedef Eina_Bool (*Eina_Simple_XML_Cb)(void *data, Eina_Simple_XML_Type type, const char *content, unsigned offset, unsigned length);
typedef Eina_Bool (*Eina_Simple_XML_Attribute_Cb)(void *data, const char *key, const char *value);


/**
 * Parse a section of XML string text
 * 
 * @param buf the input string. May not contain \0 terminator.
 * @param buflen the input string size.
 * @param strip whenever this parser should strip leading and trailing
 *        whitespace. These whitespace will still be issued, but as type
 *        #EINA_SIMPLE_XML_IGNORED.
 * @param func what to call back while parse to do some action.  The
 *        first parameter is the given user @a data, the second is the
 *        token type, the third is the pointer to content start (it's
 *        not a NULL terminated string!), the forth is where this
 *        content is located inside @a buf (does not include tag
 *        start, for instance "<!DOCTYPE value>" the offset points at
 *        "value"), the fifth is the size of the content. Whenever this
 *        function return EINA_FALSE the parser will abort.  @param
 *        data what to give as context to @a func.
 *
 * @return EINA_TRUE on success or EINA_FALSE if it was aborted by user or
 *         parsing error.
 */
EAPI Eina_Bool eina_simple_xml_parse(const char *buf, unsigned buflen,
				     Eina_Bool strip,
				     Eina_Simple_XML_Cb func, const void *data);


/**
 * Given the contents of a tag, find where the attributes start.
 *
 * The tag contents is returned by eina_simple_xml_parse() when
 * type is #EINA_SIMPLE_XML_OPEN or #EINA_SIMPLE_XML_OPEN_EMPTY.
 *
 * @return pointer to the start of attributes, it can be used
 *         to feed eina_simple_xml_attributes_parse(). NULL is returned
 *         if no attributes were found.
 */
EAPI const char * eina_simple_xml_tag_attributes_find(const char *buf, unsigned buflen);

/**
 * Given a buffer with xml attributes, parse them to key=value pairs.
 *
 * @param buf the input string. May not contain \0 terminator.
 * @param buflen the input string size.
 * @param func what to call back while parse to do some action. The
 *        first parameter is the given user @a data, the second is the
 *        key (null-terminated) and the last is the value (null
 *        terminated). These strings should not be modified and
 *        reference is just valid until the function return.
 *
 * @return EINA_TRUE on success or EINA_FALSE if it was aborted by user or
 *         parsing error.
 */
EAPI Eina_Bool eina_simple_xml_attributes_parse(const char *buf, unsigned buflen,
						Eina_Simple_XML_Attribute_Cb func, const void *data);

/**
 * Create (and append) new attribute to tag.
 *
 * @param parent if provided, will be set in the resulting structure
 *        as well as the attribute will be appended to attributes list.
 * @param key null-terminated string. Must not be NULL.
 * @param value null-terminated string. If NULL, the empty string will be used.
 *
 * @return newly allocated memory or NULL on error. This memory should be
 *         released with eina_simple_xml_attribute_free() or indirectly
 *         with eina_simple_xml_node_tag_free().
 */
EAPI Eina_Simple_XML_Attribute * eina_simple_xml_attribute_new(Eina_Simple_XML_Node_Tag *parent, const char *key, const char *value);

/**
 * Remove attribute from parent and delete it.
 *
 * @param attr attribute to release memory.
 */
EAPI void eina_simple_xml_attribute_free(Eina_Simple_XML_Attribute *attr);


/**
 * Create new tag. If parent is provided, it is automatically appended.
 *
 * @param parent if provided, will be set in the resulting structure
 *        as well as the tag will be appended to children list.
 * @param name null-terminated string. Must not be NULL.
 *
 * @return newly allocated memory or NULL on error. This memory should be
 *         released with eina_simple_xml_node_tag_free() or indirectly
 *         with eina_simple_xml_node_tag_free() of the parent.
 */
EAPI Eina_Simple_XML_Node_Tag * eina_simple_xml_node_tag_new(Eina_Simple_XML_Node_Tag *parent, const char *name);

/**
 * Remove tag from parent and delete it.
 *
 * @param tag to release memory.
 */
EAPI void eina_simple_xml_node_tag_free(Eina_Simple_XML_Node_Tag *tag);


/**
 * Create new data. If parent is provided, it is automatically appended.
 *
 * @param parent if provided, will be set in the resulting structure
 *        as well as the data will be appended to children list.
 * @param content string to be used. Must not be NULL.
 * @param length size in bytes of @a content.
 *
 * @return newly allocated memory or NULL on error. This memory should be
 *         released with eina_simple_xml_node_data_free() or indirectly
 *         with eina_simple_xml_node_tag_free() of the parent.
 */
EAPI Eina_Simple_XML_Node_Data * eina_simple_xml_node_data_new(Eina_Simple_XML_Node_Tag *parent, const char *contents, size_t length);

/**
 * Remove data from parent and delete it.
 *
 * @param data to release memory.
 */
EAPI void eina_simple_xml_node_data_free(Eina_Simple_XML_Node_Data *node);


/**
 * Create new cdata. If parent is provided, it is automatically appended.
 *
 * @param parent if provided, will be set in the resulting structure
 *        as well as the cdata will be appended to children list.
 * @param content string to be used. Must not be NULL.
 * @param length size in bytes of @a content.
 *
 * @return newly allocated memory or NULL on error. This memory should be
 *         released with eina_simple_xml_node_cdata_free() or indirectly
 *         with eina_simple_xml_node_tag_free() of the parent.
 */
EAPI Eina_Simple_XML_Node_CData * eina_simple_xml_node_cdata_new(Eina_Simple_XML_Node_Tag *parent, const char *contents, size_t length);

/**
 * Remove cdata from parent and delete it.
 *
 * @param cdata to release memory.
 */
EAPI void eina_simple_xml_node_cdata_free(Eina_Simple_XML_Node_Data *node);


/**
 * Create new processing. If parent is provided, it is automatically appended.
 *
 * @param parent if provided, will be set in the resulting structure
 *        as well as the processing will be appended to children list.
 * @param content string to be used. Must not be NULL.
 * @param length size in bytes of @a content.
 *
 * @return newly allocated memory or NULL on error. This memory should be
 *         released with eina_simple_xml_node_processing_free() or indirectly
 *         with eina_simple_xml_node_tag_free() of the parent.
 */
EAPI Eina_Simple_XML_Node_Processing * eina_simple_xml_node_processing_new(Eina_Simple_XML_Node_Tag *parent, const char *contents, size_t length);

/**
 * Remove processing from parent and delete it.
 *
 * @param processing to release memory.
 */
EAPI void eina_simple_xml_node_processing_free(Eina_Simple_XML_Node_Data *node);


/**
 * Create new doctype. If parent is provided, it is automatically appended.
 *
 * @param parent if provided, will be set in the resulting structure
 *        as well as the doctype will be appended to children list.
 * @param content string to be used. Must not be NULL.
 * @param length size in bytes of @a content.
 *
 * @return newly allocated memory or NULL on error. This memory should be
 *         released with eina_simple_xml_node_doctype_free() or indirectly
 *         with eina_simple_xml_node_tag_free() of the parent.
 */
EAPI Eina_Simple_XML_Node_Doctype * eina_simple_xml_node_doctype_new(Eina_Simple_XML_Node_Tag *parent, const char *contents, size_t length);

/**
 * Remove doctype from parent and delete it.
 *
 * @param doctype to release memory.
 */
EAPI void eina_simple_xml_node_doctype_free(Eina_Simple_XML_Node_Data *node);


/**
 * Create new comment. If parent is provided, it is automatically appended.
 *
 * @param parent if provided, will be set in the resulting structure
 *        as well as the comment will be appended to children list.
 * @param content string to be used. Must not be NULL.
 * @param length size in bytes of @a content.
 *
 * @return newly allocated memory or NULL on error. This memory should be
 *         released with eina_simple_xml_node_comment_free() or indirectly
 *         with eina_simple_xml_node_tag_free() of the parent.
 */
EAPI Eina_Simple_XML_Node_Comment * eina_simple_xml_node_comment_new(Eina_Simple_XML_Node_Tag *parent, const char *contents, size_t length);

/**
 * Remove comment from parent and delete it.
 *
 * @param comment to release memory.
 */
EAPI void eina_simple_xml_node_comment_free(Eina_Simple_XML_Node_Data *node);


/**
 * Load a XML node tree based on the given string.
 *
 * @param buf the input string. May not contain \0 terminator.
 * @param buflen the input string size.
 * @param strip whenever this parser should strip leading and trailing
 *        whitespace.
 *
 * @return document root with children tags, or NULL on errors.
 *         Document with errors may return partial tree instead of NULL,
 *         we'll do our best to avoid returning nothing.
 */
EAPI Eina_Simple_XML_Node_Root * eina_simple_xml_node_load(const char *buf, unsigned buflen, Eina_Bool strip);

/**
 * Free node tree build with eina_simple_xml_node_load()
 *
 * @param root memory returned by eina_simple_xml_node_load()
 */
EAPI void eina_simple_xml_node_root_free(Eina_Simple_XML_Node_Root *root);

/**
 * Converts the node tree under the given element to a XML string.
 *
 * @param node the base node to convert.
 * @param indent indentation string, or NULL to disable it.
 *
 * @param NULL on errors or a newly allocated string on success.
 */
EAPI char * eina_simple_xml_node_dump(Eina_Simple_XML_Node *node, const char *indent);


/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_SIMPLE_XML_H_ */

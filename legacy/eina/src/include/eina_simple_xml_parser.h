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

EAPI Eina_Bool eina_simple_xml_parse(const char *buf, unsigned buflen,
				     Eina_Bool strip,
				     Eina_Simple_XML_Cb func, const void *data);

EAPI const char * eina_simple_xml_tag_attributes_find(const char *buf, unsigned buflen);
EAPI Eina_Bool eina_simple_xml_attributes_parse(const char *buf, unsigned buflen,
						Eina_Simple_XML_Attribute_Cb func, const void *data);
EAPI Eina_Simple_XML_Attribute * eina_simple_xml_attribute_new(Eina_Simple_XML_Node_Tag *parent, const char *key, const char *value);
EAPI void eina_simple_xml_attribute_free(Eina_Simple_XML_Attribute *attr);

EAPI Eina_Simple_XML_Node_Tag * eina_simple_xml_node_tag_new(Eina_Simple_XML_Node_Tag *parent, const char *name);
EAPI void eina_simple_xml_node_tag_free(Eina_Simple_XML_Node_Tag *tag);

EAPI Eina_Simple_XML_Node_Data * eina_simple_xml_node_data_new(Eina_Simple_XML_Node_Tag *parent, const char *contents, size_t length);
EAPI void eina_simple_xml_node_data_free(Eina_Simple_XML_Node_Data *node);

EAPI Eina_Simple_XML_Node_CData * eina_simple_xml_node_cdata_new(Eina_Simple_XML_Node_Tag *parent, const char *contents, size_t length);
EAPI void eina_simple_xml_node_cdata_free(Eina_Simple_XML_Node_Data *node);

EAPI Eina_Simple_XML_Node_Processing * eina_simple_xml_node_processing_new(Eina_Simple_XML_Node_Tag *parent, const char *contents, size_t length);
EAPI void eina_simple_xml_node_processing_free(Eina_Simple_XML_Node_Data *node);

EAPI Eina_Simple_XML_Node_Doctype * eina_simple_xml_node_doctype_new(Eina_Simple_XML_Node_Tag *parent, const char *contents, size_t length);
EAPI void eina_simple_xml_node_doctype_free(Eina_Simple_XML_Node_Data *node);

EAPI Eina_Simple_XML_Node_Comment * eina_simple_xml_node_comment_new(Eina_Simple_XML_Node_Tag *parent, const char *contents, size_t length);
EAPI void eina_simple_xml_node_comment_free(Eina_Simple_XML_Node_Data *node);

EAPI Eina_Simple_XML_Node_Root * eina_simple_xml_node_load(const char *buf, unsigned buflen, Eina_Bool strip);
EAPI void eina_simple_xml_node_root_free(Eina_Simple_XML_Node_Root *root);
EAPI char * eina_simple_xml_node_dump(Eina_Simple_XML_Node *node, const char *indent);

#endif /* EINA_SIMPLE_XML_H_ */

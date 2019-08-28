#ifndef _EFL_CANVAS_TEXT_INTERNAL_H
#define _EFL_CANVAS_TEXT_INTERNAL_H

#include <Evas.h>

/**
 * @internal
 * @typedef Evas_Object_Textblock_Node_Text
 * A text node.
 */
typedef struct _Evas_Object_Textblock_Node_Text   Evas_Object_Textblock_Node_Text;

struct _Efl2_Text_Cursor_Handle
{
   Evas_Object                     *obj;
   size_t                           pos;
   Evas_Object_Textblock_Node_Text *node;
   Eina_Bool                        changed : 1;
};

struct _Efl2_Text_Attribute_Handle
{
   EINA_INLIST;
   Evas_Object                       *obj;
   Evas_Object_Textblock_Node_Format *start_node, *end_node;
   Eina_Bool                         is_item : 1; /**< indicates it is an item/object placeholder */
};

#endif

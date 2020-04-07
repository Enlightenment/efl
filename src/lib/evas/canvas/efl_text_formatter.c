//#define EFL_BETA_API_SUPPORT
#include "evas_common_private.h"
#include "evas_private.h"
#include "efl_canvas_textblock_internal.h"
#include "efl_text_cursor_object.eo.h"

#define MY_CLASS EFL_TEXT_FORMATTER_CLASS

typedef struct
{

} Efl_Text_Formatter_Data;

EOLIAN static void
_efl_text_formatter_attribute_insert(const Efl_Text_Cursor_Object *start, const Efl_Text_Cursor_Object *end, const char *format)
{
   EINA_SAFETY_ON_TRUE_RETURN(!efl_text_cursor_object_handle_get(start) ||
                              !efl_text_cursor_object_handle_get(end) ||
                               efl_text_cursor_object_handle_get(start)->obj != efl_text_cursor_object_handle_get(end)->obj);

   Eo *eo_obj= efl_text_cursor_object_handle_get(start)->obj;
   evas_textblock_async_block(eo_obj);

   _evas_textblock_annotations_insert(eo_obj, efl_text_cursor_object_handle_get(start), efl_text_cursor_object_handle_get(end), format,
         EINA_FALSE);
   efl_event_callback_legacy_call(eo_obj, EFL_CANVAS_TEXTBLOCK_EVENT_CHANGED, NULL);
}

EOLIAN static unsigned int
_efl_text_formatter_attribute_clear(const Efl_Text_Cursor_Object *start, const Efl_Text_Cursor_Object *end)
{
   unsigned int ret = 0;
   Eina_Iterator *annotations;
   Efl_Text_Attribute_Handle *an;
   annotations = efl_text_formatter_range_attributes_get(start, end);

   if (!annotations) return ret;

   EINA_ITERATOR_FOREACH(annotations, an)
     {
        ret++;
        efl_text_formatter_remove(an);
     }
   eina_iterator_free(annotations);

   return ret;
}

const char *
efl_text_formatter_attribute_get(Efl_Text_Attribute_Handle *annotation)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!annotation || !(annotation->obj), NULL);

   return (annotation->start_node ? annotation->start_node->format : NULL);
}

Eina_Iterator *
efl_text_formatter_range_attributes_get(const Efl_Text_Cursor_Object *start, const Efl_Text_Cursor_Object *end)
{
   Eina_List *lst = NULL;
   Efl_Text_Attribute_Handle *it;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(!efl_text_cursor_object_handle_get(start) ||
                              !efl_text_cursor_object_handle_get(end) ||
                              efl_text_cursor_object_handle_get(start)->obj != efl_text_cursor_object_handle_get(end)->obj, NULL);

   Eina_Inlist *annotations = _evas_textblock_annotations_get(efl_text_cursor_object_handle_get(start)->obj);

   EINA_INLIST_FOREACH(annotations, it)
     {
        Efl_Text_Cursor_Handle start2, end2;
        _evas_textblock_cursor_init(&start2, efl_text_cursor_object_handle_get(start)->obj);
        _evas_textblock_cursor_init(&end2, efl_text_cursor_object_handle_get(start)->obj);

        if (!it->start_node || !it->end_node) continue;
        _textblock_cursor_pos_at_fnode_set(&start2, it->start_node);
        _textblock_cursor_pos_at_fnode_set(&end2, it->end_node);
        evas_textblock_cursor_char_prev(&end2);
        if (!((evas_textblock_cursor_compare(&start2, efl_text_cursor_object_handle_get(end)) > 0) ||
                 (evas_textblock_cursor_compare(&end2, efl_text_cursor_object_handle_get(start)) < 0)))
          {
             lst = eina_list_append(lst, it);
          }
     }
   return _evas_textblock_annotation_iterator_new(lst);
}

void
efl_text_formatter_attribute_cursors_get(const Efl_Text_Attribute_Handle *handle, Efl_Text_Cursor_Object *start, Efl_Text_Cursor_Object *end)
{
   EINA_SAFETY_ON_TRUE_RETURN (!handle || !(handle->obj));

   efl_text_cursor_object_text_object_set(start, handle->obj, handle->obj);
   efl_text_cursor_object_text_object_set(end, handle->obj, handle->obj);
   _textblock_cursor_pos_at_fnode_set(efl_text_cursor_object_handle_get(start), handle->start_node);
   _textblock_cursor_pos_at_fnode_set(efl_text_cursor_object_handle_get(end), handle->end_node);
}

void
efl_text_formatter_remove(Efl_Text_Attribute_Handle *annotation)
{
   EINA_SAFETY_ON_TRUE_RETURN (!annotation || !(annotation->obj));

   evas_textblock_async_block(annotation->obj);
   _evas_textblock_annotation_remove(annotation->obj, NULL, annotation, EINA_TRUE, EINA_TRUE);
}

Eina_Bool
efl_text_formatter_attribute_is_item(Efl_Text_Attribute_Handle *annotation)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!annotation || !(annotation->obj), EINA_FALSE);

   return annotation->is_item;
}

Eina_Bool
efl_text_formatter_item_geometry_get(const Efl_Text_Attribute_Handle *annotation, int *x, int *y, int *w, int *h)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!annotation || !(annotation->obj), EINA_FALSE);

   Efl_Text_Cursor_Handle cur;

   Eo *eo_obj = annotation->obj;
   Evas_Object_Protected_Data *obj_data = efl_data_scope_safe_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj_data, EINA_FALSE);
   evas_object_async_block(obj_data);
   _evas_textblock_relayout_if_needed(eo_obj);

   _evas_textblock_cursor_init(&cur, eo_obj);
   _textblock_cursor_pos_at_fnode_set(&cur, annotation->start_node);
   return evas_textblock_cursor_format_item_geometry_get(&cur, x, y, w, h);
}

#include "efl_text_formatter.eo.c"

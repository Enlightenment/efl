#include "evas_common_private.h"
#include "evas_private.h"
#include "efl_canvas_textblock_internal.h"
#include "eo_internal.h"

#define MY_CLASS EFL_TEXT_CURSOR_OBJECT_CLASS
#define MY_CLASS_NAME "Efl.Text.Cursor"

typedef struct
{
   Efl_Text_Cursor_Handle *handle;
   Efl_Canvas_Object *text_obj;
} Efl_Text_Cursor_Object_Data;

struct _Evas_Textblock_Selection_Iterator
{
   Eina_Iterator                       iterator; /**< Eina Iterator. */
   Eina_List                           *list; /**< Head of list. */
   Eina_List                           *current; /**< Current node in loop. */
};

typedef struct _Evas_Textblock_Selection_Iterator Evas_Textblock_Selection_Iterator;

EFL_CLASS_SIMPLE_CLASS(efl_text_cursor_object, "Efl.Text.Cursor", EFL_TEXT_CURSOR_OBJECT_CLASS)

EOLIAN static void
_efl_text_cursor_object_position_set(Eo *obj EINA_UNUSED, Efl_Text_Cursor_Object_Data *pd, int position)
{
   evas_textblock_cursor_pos_set(pd->handle, position);
}

EOLIAN static int
_efl_text_cursor_object_position_get(const Eo *obj EINA_UNUSED, Efl_Text_Cursor_Object_Data *pd)
{
   return evas_textblock_cursor_pos_get(pd->handle);
}

EOLIAN static Eina_Unicode
_efl_text_cursor_object_content_get(const Eo *obj EINA_UNUSED, Efl_Text_Cursor_Object_Data *pd)
{
   if (pd->handle && pd->handle->node)
    return eina_ustrbuf_string_get(pd->handle->node->unicode)[pd->handle->pos];
   else
    return 0;
}

EOLIAN static Eina_Rect
_efl_text_cursor_object_content_geometry_get(const Eo *obj EINA_UNUSED, Efl_Text_Cursor_Object_Data *pd)
{
   Eina_Rect rect = {0};
   Eina_Bool item_is = evas_textblock_cursor_format_item_geometry_get(pd->handle, &(rect.x), &(rect.y), &(rect.w), &(rect.h));
   if (item_is)
      return rect;

   evas_textblock_cursor_pen_geometry_get(pd->handle, &(rect.x), &(rect.y), &(rect.w), &(rect.h));

   return rect;
}

EOLIAN static void
_efl_text_cursor_object_line_number_set(Eo *obj EINA_UNUSED, Efl_Text_Cursor_Object_Data *pd, int line_number)
{
   evas_textblock_cursor_line_set(pd->handle, line_number);
}

EOLIAN static int
_efl_text_cursor_object_line_number_get(const Eo *obj EINA_UNUSED, Efl_Text_Cursor_Object_Data *pd)
{
   Eina_Rect rect = {0};

   return evas_textblock_cursor_line_geometry_get(pd->handle, &(rect.x), &(rect.y), &(rect.w), &(rect.h));
}

EOLIAN static Eina_Rect
_efl_text_cursor_object_cursor_geometry_get(const Eo *obj EINA_UNUSED, Efl_Text_Cursor_Object_Data *pd, Efl_Text_Cursor_Type ctype)
{
   Eina_Rect rc = {0};
   Evas_Textblock_Cursor_Type cursor_type = (ctype == EFL_TEXT_CURSOR_TYPE_BEFORE) ? EVAS_TEXTBLOCK_CURSOR_BEFORE : EVAS_TEXTBLOCK_CURSOR_UNDER;
   evas_textblock_cursor_geometry_bidi_get(pd->handle, &rc.x, &rc.y, &rc.w, &rc.h, NULL, NULL, NULL, NULL, cursor_type);
   return rc;
}

EOLIAN static Eina_Bool
_efl_text_cursor_object_lower_cursor_geometry_get(const Eo *obj EINA_UNUSED, Efl_Text_Cursor_Object_Data *pd, Eina_Rect *geometry2)
{
   Eina_Rect rc = {0};
   Eina_Bool b_ret = EINA_FALSE;
   b_ret = evas_textblock_cursor_geometry_bidi_get(pd->handle, NULL, NULL, NULL, NULL, &rc.x, &rc.y, &rc.w, &rc.h, EVAS_TEXTBLOCK_CURSOR_BEFORE);
   if (geometry2)
     {
        *geometry2 = rc;
     }
   return b_ret;
}

EOLIAN static Eina_Bool
_efl_text_cursor_object_equal(const Eo *obj EINA_UNUSED, Efl_Text_Cursor_Object_Data *pd, const Efl_Text_Cursor_Object *dst)
{
   return evas_textblock_cursor_equal(pd->handle, efl_text_cursor_object_handle_get(dst));
}

EOLIAN static int
_efl_text_cursor_object_compare(const Eo *obj EINA_UNUSED, Efl_Text_Cursor_Object_Data *pd, const Efl_Text_Cursor_Object *dst)
{
   return evas_textblock_cursor_compare(pd->handle, efl_text_cursor_object_handle_get(dst));
}

static void
_efl_text_cursor_object_copy(const Efl_Text_Cursor_Object *obj, Efl_Text_Cursor_Object *dst)
{
   Efl_Text_Cursor_Object_Data *pd = efl_data_scope_safe_get(obj, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(pd);

   Efl_Text_Cursor_Object_Data *pd_dest = efl_data_scope_safe_get(dst, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(pd_dest);

   if (!pd->handle) return;

   Efl_Text_Cursor_Handle *handle = evas_object_textblock_cursor_new(pd->handle->obj);
   evas_textblock_cursor_copy(pd->handle, handle);
   pd_dest->text_obj = pd->text_obj;
   efl_text_cursor_object_handle_set(dst, handle);
   evas_textblock_cursor_unref(handle, NULL);
}

EOLIAN static Efl_Text_Cursor_Object *
_efl_text_cursor_object_efl_duplicate_duplicate(const Eo *obj, Efl_Text_Cursor_Object_Data *pd EINA_UNUSED)
{
  Efl_Text_Cursor_Object *dup = efl_text_cursor_object_create(efl_parent_get(obj));

  _efl_text_cursor_object_copy(obj, dup);

  return dup;
}

EOLIAN static Eina_Bool
_efl_text_cursor_object_move(Eo *obj EINA_UNUSED, Efl_Text_Cursor_Object_Data *pd, Efl_Text_Cursor_Move_Type type)
{
   Eina_Bool moved = EINA_FALSE;
   int pos = evas_textblock_cursor_pos_get(pd->handle);

   switch (type) {
      case EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_NEXT :
         moved = evas_textblock_cursor_char_next(pd->handle);
         break;
      case EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_PREVIOUS :
         moved = evas_textblock_cursor_char_prev(pd->handle);
         break;
      case EFL_TEXT_CURSOR_MOVE_TYPE_CLUSTER_NEXT :
         moved = evas_textblock_cursor_cluster_next(pd->handle);
         break;
      case EFL_TEXT_CURSOR_MOVE_TYPE_CLUSTER_PREVIOUS :
         moved = evas_textblock_cursor_cluster_prev(pd->handle);
         break;
      case EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_START :
         evas_textblock_cursor_paragraph_char_first(pd->handle);
         if (pos != evas_textblock_cursor_pos_get(pd->handle))
           moved = EINA_TRUE;
         break;
      case EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_END :
         evas_textblock_cursor_paragraph_char_last(pd->handle);
         if (pos != evas_textblock_cursor_pos_get(pd->handle))
           moved = EINA_TRUE;
         break;
      case EFL_TEXT_CURSOR_MOVE_TYPE_WORD_START :
         moved = evas_textblock_cursor_word_start(pd->handle);
         break;
      case EFL_TEXT_CURSOR_MOVE_TYPE_WORD_END :
         moved = evas_textblock_cursor_word_end(pd->handle);
         break;
      case EFL_TEXT_CURSOR_MOVE_TYPE_LINE_START :
         evas_textblock_cursor_line_char_first(pd->handle);
         if (pos != evas_textblock_cursor_pos_get(pd->handle))
           moved = EINA_TRUE;
         break;
      case EFL_TEXT_CURSOR_MOVE_TYPE_LINE_END :
         evas_textblock_cursor_line_char_last(pd->handle);
         if (pos != evas_textblock_cursor_pos_get(pd->handle))
           moved = EINA_TRUE;
         break;
      case EFL_TEXT_CURSOR_MOVE_TYPE_FIRST :
         evas_textblock_cursor_paragraph_first(pd->handle);
         if (pos != evas_textblock_cursor_pos_get(pd->handle))
           moved = EINA_TRUE;
         break;
      case EFL_TEXT_CURSOR_MOVE_TYPE_LAST :
         evas_textblock_cursor_paragraph_last(pd->handle);
         if (pos != evas_textblock_cursor_pos_get(pd->handle))
           moved = EINA_TRUE;
         break;
      case EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_NEXT :
         moved = evas_textblock_cursor_paragraph_next(pd->handle);
         break;
      case EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_PREVIOUS :
         moved = evas_textblock_cursor_paragraph_prev(pd->handle);
         break;
     }

   return moved;
}

EOLIAN static void
_efl_text_cursor_object_char_delete(Eo *obj EINA_UNUSED, Efl_Text_Cursor_Object_Data *pd)
{
   evas_textblock_cursor_char_delete(pd->handle);
}

EOLIAN static Eina_Bool
_efl_text_cursor_object_line_jump_by(Eo *obj EINA_UNUSED, Efl_Text_Cursor_Object_Data *pd, int by)
{
   if (!pd->handle) return EINA_FALSE;

   Eina_Bool moved = EINA_FALSE;
   int pos = evas_textblock_cursor_pos_get(pd->handle);
   evas_textblock_cursor_line_jump_by(pd->handle, by);
   moved = (pos != evas_textblock_cursor_pos_get(pd->handle));
   return moved;
}

EOLIAN static void
_efl_text_cursor_object_char_coord_set(Eo *obj EINA_UNUSED, Efl_Text_Cursor_Object_Data *pd, Eina_Position2D coord)
{
   evas_textblock_cursor_char_coord_set(pd->handle, coord.x, coord.y);
}

EOLIAN static void
_efl_text_cursor_object_cluster_coord_set(Eo *obj EINA_UNUSED, Efl_Text_Cursor_Object_Data *pd, Eina_Position2D coord)
{
   evas_textblock_cursor_cluster_coord_set(pd->handle, coord.x, coord.y);
}

static int
_prepend_text_run2(Efl_Text_Cursor_Handle *cur, const char *s, const char *p)
{
   if ((s) && (p > s))
     {
        char *ts;

        ts = alloca(p - s + 1);
        strncpy(ts, s, p - s);
        ts[p - s] = 0;
        return evas_textblock_cursor_text_prepend(cur, ts);
     }
   return 0;
}

int
_cursor_text_append(Efl_Text_Cursor_Handle *cur,
      const char *text)
{
   if (!text || !cur) return 0;

   const char *off = text;
   int len = 0;

   Evas_Object_Protected_Data *obj = efl_data_scope_safe_get(cur->obj, EFL_CANVAS_OBJECT_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, 0);
   evas_object_async_block(obj);

   while (*off)
     {
        char *format = NULL;
        int n = 1;
        if (!strncmp(_PARAGRAPH_SEPARATOR_UTF8, off,
                    strlen(_PARAGRAPH_SEPARATOR_UTF8)))
          {
             format = "ps";
             n = strlen(_PARAGRAPH_SEPARATOR_UTF8);
          }
        else if (!strncmp(_NEWLINE_UTF8, off, strlen(_NEWLINE_UTF8)))
          {
             format = "br";
             n = strlen(_NEWLINE_UTF8);
          }
        else if (!strncmp(_TAB_UTF8, off, strlen(_TAB_UTF8)))
          {
             format = "tab";
             n = strlen(_TAB_UTF8);
          }

        if (format)
          {
             len += _prepend_text_run2(cur, text, off);
             if (evas_textblock_cursor_format_prepend(cur, format))
               {
                  len++;
               }
             text = off + n; /* sync text with next segment */
          }
          off += n;
     }
   len += _prepend_text_run2(cur, text, off);
   return len;
}

EOLIAN static void
_efl_text_cursor_object_text_insert(Eo *obj EINA_UNUSED, Efl_Text_Cursor_Object_Data *pd, const char *text)
{
   _cursor_text_append(pd->handle, text);
}

EOLIAN static char *
_efl_text_cursor_object_range_text_get(const Eo *obj EINA_UNUSED, Efl_Text_Cursor_Object_Data *pd, Efl_Text_Cursor_Object *cur2)
{
   return evas_textblock_cursor_range_text_get(pd->handle, efl_text_cursor_object_handle_get(cur2), EVAS_TEXTBLOCK_TEXT_PLAIN);
}

EOLIAN static void
_efl_text_cursor_object_markup_insert(Eo *obj EINA_UNUSED, Efl_Text_Cursor_Object_Data *pd, const char *markup)
{
   evas_object_textblock_text_markup_prepend(pd->handle, markup);
}

EOLIAN static char *
_efl_text_cursor_object_range_markup_get(const Eo *obj EINA_UNUSED, Efl_Text_Cursor_Object_Data *pd, Efl_Text_Cursor_Object *cur2)
{
   return evas_textblock_cursor_range_text_get(pd->handle,efl_text_cursor_object_handle_get(cur2), EVAS_TEXTBLOCK_TEXT_MARKUP);
}

EOLIAN static Eina_Iterator *
_efl_text_cursor_object_range_geometry_get(Eo *obj EINA_UNUSED, Efl_Text_Cursor_Object_Data *pd, Efl_Text_Cursor_Object *cur2)
{
   return evas_textblock_cursor_range_simple_geometry_get(pd->handle, efl_text_cursor_object_handle_get(cur2));
}

/** selection iterator */
/**
  * @internal
  * Returns the value of the current data of list node,
  * and goes to the next list node.
  *
  * @param it the iterator.
  * @param data the data of the current list node.
  * @return EINA_FALSE if the current list node does not exists.
  * Otherwise, returns EINA_TRUE.
  */
static Eina_Bool
_evas_textblock_selection_iterator_next(Evas_Textblock_Selection_Iterator *it, void **data)
{
   if (!it->current)
     return EINA_FALSE;

   *data = eina_list_data_get(it->current);
   it->current = eina_list_next(it->current);

   return EINA_TRUE;
}

/**
  * @internal
  * Gets the iterator container (Eina_List) which created the iterator.
  * @param it the iterator.
  * @return A pointer to Eina_List.
  */
static Eina_List *
_evas_textblock_selection_iterator_get_container(Evas_Textblock_Selection_Iterator *it)
{
   return it->list;
}

/**
  * @internal
  * Frees the iterator container (Eina_List).
  * @param it the iterator.
  */
static void
_evas_textblock_selection_iterator_free(Evas_Textblock_Selection_Iterator *it)
{
   Eina_Rectangle *tr;

   EINA_LIST_FREE(it->list, tr)
     free(tr);
   EINA_MAGIC_SET(&it->iterator, 0);
   free(it);
}

/**
  * @internal
  * Creates newly allocated  iterator associated to a list.
  * @param list The list.
  * @return If the memory cannot be allocated, NULL is returned.
  * Otherwise, a valid iterator is returned.
  */
static Eina_Iterator *
_evas_textblock_selection_iterator_new(Eina_List *list)
{
   Evas_Textblock_Selection_Iterator *it;

   it = calloc(1, sizeof(Evas_Textblock_Selection_Iterator));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);
   it->list = list;
   it->current = list;

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(
                                     _evas_textblock_selection_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(
                            _evas_textblock_selection_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(
                                     _evas_textblock_selection_iterator_free);

   return &it->iterator;
}

EOLIAN static Eina_Iterator *
_efl_text_cursor_object_range_precise_geometry_get(Eo *obj EINA_UNUSED, Efl_Text_Cursor_Object_Data *pd, Efl_Text_Cursor_Object *cur2)
{
   Eina_List *rects = evas_textblock_cursor_range_geometry_get(pd->handle, efl_text_cursor_object_handle_get(cur2));
   return _evas_textblock_selection_iterator_new(rects);
}

EOLIAN static void
_efl_text_cursor_object_range_delete(Eo *obj EINA_UNUSED, Efl_Text_Cursor_Object_Data *pd, Efl_Text_Cursor_Object *cur2)
{
   evas_textblock_cursor_range_delete(pd->handle, efl_text_cursor_object_handle_get(cur2));
}

EVAS_API void
efl_text_cursor_object_handle_set(Eo *obj, Efl_Text_Cursor_Handle *handle)
{
   Efl_Text_Cursor_Object_Data *pd = efl_data_scope_safe_get(obj, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(pd);
   if (handle == pd->handle)
     return;

   Efl_Text_Cursor_Handle *old_handle = pd->handle;

   pd->handle = evas_textblock_cursor_ref(handle, obj);

   if (old_handle)
     {
        evas_textblock_cursor_unref(old_handle, obj);
     }
}

EVAS_API Efl_Text_Cursor_Handle *
efl_text_cursor_object_handle_get(const Eo *obj)
{
   Efl_Text_Cursor_Object_Data *pd = efl_data_scope_safe_get(obj, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, NULL);
   return pd->handle;
}

Eo* efl_text_cursor_object_create(Eo *parent)
{
   return efl_add(efl_text_cursor_object_realized_class_get(), parent);
}

void efl_text_cursor_object_text_object_set(Eo *cursor, Eo *canvas_text_obj, Eo *text_obj)
{
   Efl_Text_Cursor_Object_Data *pd = efl_data_scope_safe_get(cursor, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(pd);
   Efl_Text_Cursor_Handle *handle = NULL;
   if (efl_isa(canvas_text_obj, EFL_CANVAS_TEXTBLOCK_CLASS))
     {
        pd->text_obj = text_obj;
        handle = evas_object_textblock_cursor_new(canvas_text_obj);
     }
   else
     {
        ERR("Expect Canvas Text Object");
     }

   if (handle)
     {
        efl_text_cursor_object_handle_set(cursor, handle);
        evas_textblock_cursor_unref(handle, NULL);
     }
}

EOLIAN static Efl_Canvas_Object *
_efl_text_cursor_object_text_object_get(const Eo *obj EINA_UNUSED, Efl_Text_Cursor_Object_Data *pd)
{
   return pd->text_obj;
}

EOLIAN static void
_efl_text_cursor_object_efl_object_destructor(Eo *obj, Efl_Text_Cursor_Object_Data *pd)
{
   if (pd->handle)
     {
        evas_textblock_cursor_unref(pd->handle, obj);
        pd->handle = NULL;
     }

   if (pd->text_obj)
     {
        pd->text_obj = NULL;
     }

   efl_destructor(efl_super(obj, MY_CLASS));

}

#include "efl_text_cursor_object.eo.c"

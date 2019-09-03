#include "evas_common_private.h"
#include "evas_private.h"

#include "efl_canvas_text_internal.h"

#define MY_CLASS EFL2_TEXT_CURSOR

struct _Efl2_Text_Cursor_Data
{
   Efl2_Canvas_Text *obj;
   Efl2_Text_Cursor_Handle *cur;
};

typedef struct _Efl2_Text_Cursor_Data Efl2_Text_Cursor_Data;

EOLIAN static void
_efl2_text_cursor_position_set(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd, int position)
{
}

EOLIAN static int
_efl2_text_cursor_position_get(const Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd)
{
}

EOLIAN static Eina_Unicode
_efl2_text_cursor_content_get(const Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd)
{
}

EOLIAN static Eina_Bool
_efl2_text_cursor_geometry_get(const Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd, Efl2_Text_Cursor_Type ctype, int *cx, int *cy, int *cw, int *ch, int *cx2, int *cy2, int *cw2, int *ch2)
{
}

EOLIAN static Eina_Bool
_efl2_text_cursor_equal(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd, Efl2_Text_Cursor *cur2)
{
}

EOLIAN static int
_efl2_text_cursor_compare(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd, Efl2_Text_Cursor *cur2)
{
}

EOLIAN static void
_efl2_text_cursor_copy(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd)
{
}

EOLIAN static void
_efl2_text_cursor_char_next(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd)
{
}

EOLIAN static void
_efl2_text_cursor_char_prev(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd)
{
}

EOLIAN static void
_efl2_text_cursor_char_delete(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd)
{
}

EOLIAN static void
_efl2_text_cursor_cluster_start(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd)
{
}

EOLIAN static void
_efl2_text_cursor_cluster_end(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd)
{
}

EOLIAN static void
_efl2_text_cursor_cluster_delete(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd)
{
}

EOLIAN static void
_efl2_text_cursor_paragraph_start(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd)
{
}

EOLIAN static void
_efl2_text_cursor_paragraph_end(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd)
{
}

EOLIAN static void
_efl2_text_cursor_word_start(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd)
{
}

EOLIAN static void
_efl2_text_cursor_word_end(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd)
{
}

EOLIAN static void
_efl2_text_cursor_line_start(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd)
{
}

EOLIAN static void
_efl2_text_cursor_line_end(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd)
{
}

EOLIAN static void
_efl2_text_cursor_paragraph_first(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd)
{
}

EOLIAN static void
_efl2_text_cursor_paragraph_last(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd)
{
}

EOLIAN static void
_efl2_text_cursor_paragraph_next(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd)
{
}

EOLIAN static void
_efl2_text_cursor_paragraph_prev(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd)
{
}

EOLIAN static void
_efl2_text_cursor_line_jump_by(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd, int by)
{
}

EOLIAN static void
_efl2_text_cursor_char_coord_set(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd, int x, int y)
{
}

EOLIAN static void
_efl2_text_cursor_cluster_coord_set(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd, int x, int y)
{
}

EOLIAN static int
_efl2_text_cursor_text_insert(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd, const char *text)
{
}

EOLIAN static char *
_efl2_text_cursor_range_text_get(const Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd, Efl2_Text_Cursor *cur2)
{
}

EOLIAN static Eina_Iterator *
_efl2_text_cursor_range_geometry_get(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd, Efl2_Text_Cursor *cur2)
{
}

EOLIAN static Eina_Iterator *
_efl2_text_cursor_range_precise_geometry_get(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd, Efl2_Text_Cursor *cur2)
{
}

EOLIAN static void
_efl2_text_cursor_range_delete(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd, Efl2_Text_Cursor *cur2)
{
}

EOLIAN static void
_efl2_text_cursor_handle_set(Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd, Efl2_Text_Cursor_Handle *handle)
{
   // FIXME: What about refcouting?
   pd->cur = handle;
}

EOLIAN static Efl2_Text_Cursor_Handle *
_efl2_text_cursor_handle_get(const Eo *obj EINA_UNUSED, Efl2_Text_Cursor_Data *pd)
{
   return pd->cur;
}

EOLIAN static void
_efl2_text_cursor_handle_ref(Efl2_Text_Cursor_Handle *handle)
{
   handle->ref++;
}

EOLIAN static void
_efl2_text_cursor_handle_unref(Efl2_Text_Cursor_Handle *handle)
{
   handle->ref--;
   // FIXME: Actually do something
}

#include "canvas/efl2_text_cursor.eo.c"


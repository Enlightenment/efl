#include "edje_private.h"
#include "edje_part_helper.h"
#include "efl_canvas_layout_internal_text.eo.h"
#define MY_CLASS EFL_CANVAS_LAYOUT_INTERNAL_TEXT_CLASS

PROXY_IMPLEMENTATION(text, MY_CLASS, EINA_FALSE)
#undef PROXY_IMPLEMENTATION

EOLIAN static void
_efl_canvas_layout_internal_text_efl_text_text_set(Eo *obj,
      void *_pd EINA_UNUSED, const char *text)
{
   PROXY_DATA_GET(obj, pd);
   _edje_efl_text_set(obj, pd->ed, pd->part, text);
   RETURN_VOID;
}

EOLIAN static const char *
_efl_canvas_layout_internal_text_efl_text_text_get(Eo *obj,
      void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   RETURN_VAL(_edje_efl_text_get(obj, pd->ed, pd->part));
}

EOLIAN static Efl_Text_Cursor_Cursor_Data *
_efl_canvas_layout_internal_text_efl_text_cursor_cursor_get(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor_Get_Type get_type)
{
   PROXY_DATA_GET(obj, pd);
   RETURN_VAL(_edje_text_cursor_get(pd->rp, (int) get_type));
}

EOLIAN static void
_efl_canvas_layout_internal_text_efl_text_cursor_cursor_paragraph_first(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor_Data *cur)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_begin(pd->rp, cur);
   RETURN_VOID;
}

EOLIAN static void
_efl_canvas_layout_internal_text_efl_text_cursor_cursor_paragraph_last(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor_Data *cur)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_end(pd->rp, cur);
   RETURN_VOID;
}

EOLIAN static void
_efl_canvas_layout_internal_text_efl_text_cursor_cursor_position_set(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor_Data *cur, int pos)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_pos_set(pd->rp, cur, pos);
   RETURN_VOID;
}

EOLIAN static int
_efl_canvas_layout_internal_text_efl_text_cursor_cursor_position_get(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor_Data *cur)
{
   PROXY_DATA_GET(obj, pd);
   RETURN_VAL(_edje_text_cursor_pos_get(pd->rp, cur));
}

EOLIAN static void
_efl_canvas_layout_internal_text_efl_text_cursor_cursor_coord_set(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor_Data *cur,
      int x, int y)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_coord_set(pd->rp, cur, x, y);
   RETURN_VOID;
}

EOLIAN static void
_efl_canvas_layout_internal_text_efl_text_cursor_cursor_line_char_first(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor_Data *cur)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_line_begin(pd->rp, cur);
   RETURN_VOID;
}

EOLIAN static void
_efl_canvas_layout_internal_text_efl_text_cursor_cursor_line_char_last(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor_Data *cur)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_line_end(pd->rp, cur);
   RETURN_VOID;
}

EOLIAN static void
_efl_canvas_layout_internal_text_efl_text_cursor_cursor_char_prev(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor_Data *cur)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_prev(pd->rp, cur);
   RETURN_VOID;
}

EOLIAN static void
_efl_canvas_layout_internal_text_efl_text_cursor_cursor_char_next(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor_Data *cur)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_next(pd->rp, cur);
   RETURN_VOID;
}

EOLIAN static void
_efl_canvas_layout_internal_text_efl_text_cursor_cursor_line_jump_by(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor_Data *cur, int by)
{
   PROXY_DATA_GET(obj, pd);
   if (by == 1)
     {
        _edje_text_cursor_down(pd->rp, cur);
     }
   else if (by == -1)
     {
        _edje_text_cursor_up(pd->rp, cur);
     }
   RETURN_VOID;
}

EOLIAN static void
_efl_canvas_layout_internal_text_efl_text_cursor_cursor_copy(Eo *obj,
      void *_pd EINA_UNUSED,
      Efl_Text_Cursor_Cursor_Data *dst,
      const Efl_Text_Cursor_Cursor_Data *cur)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_copy(pd->rp, (Efl_Text_Cursor_Cursor_Data *) cur, dst);
   RETURN_VOID;
}

EOLIAN static Eina_Unicode
_efl_canvas_layout_internal_text_efl_text_cursor_cursor_content_get(Eo *obj,
      void *_pd EINA_UNUSED,
      const Efl_Text_Cursor_Cursor_Data *cur)
{
   Eina_Unicode *ustr;
   Eina_Unicode uc = { 0 };
   char *c;
   int len;

   PROXY_DATA_GET(obj, pd);
   c = _edje_text_cursor_content_get(pd->rp, (Efl_Text_Cursor_Cursor_Data *) cur);
   ustr = eina_unicode_utf8_to_unicode(c, &len);
   free(c);
   if (ustr)
     {
        uc = *ustr;
        free(ustr);
     }
   RETURN_VAL(uc);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_text_efl_text_cursor_cursor_geometry_get(Eo *obj,
      void *_pd EINA_UNUSED,
      const Efl_Text_Cursor_Cursor_Data *cur EINA_UNUSED,
      Efl_Text_Cursor_Cursor_Type ctype EINA_UNUSED,
      Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch,
      Evas_Coord *cx2 EINA_UNUSED, Evas_Coord *cy2 EINA_UNUSED,
      Evas_Coord *cw2 EINA_UNUSED, Evas_Coord *ch2 EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   if (pd->rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_cursor_geometry_get(pd->rp, cx, cy, cw, ch, NULL);
        if (cx) *cx -= pd->ed->x;
        if (cy) *cy -= pd->ed->y;
     }
   RETURN_VAL(EINA_FALSE);
}

EOLIAN static int
_efl_canvas_layout_internal_text_efl_text_cursor_cursor_text_insert(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor_Data *cur EINA_UNUSED,
      const char *text)
{
   PROXY_DATA_GET(obj, pd);
   // Ignoring cursor type and always inserting with main cursor
   _edje_object_part_text_insert(pd->ed, pd->rp, text);
   // FIXME: entry should report the length of inserted text (after filtering)
   RETURN_VAL(0);
}

#include "efl_canvas_layout_internal_text.eo.c"


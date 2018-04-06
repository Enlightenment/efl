#include "edje_private.h"
#include "edje_part_helper.h"
#include "efl_canvas_layout_part_text.eo.h"
#define MY_CLASS EFL_CANVAS_LAYOUT_PART_TEXT_CLASS

PROXY_IMPLEMENTATION(text, MY_CLASS, EINA_FALSE)
#undef PROXY_IMPLEMENTATION

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_text_set(Eo *obj,
      void *_pd EINA_UNUSED, const char *text)
{
   PROXY_DATA_GET(obj, pd);
   _edje_efl_text_text_set(obj, pd->ed, pd->part, text, EINA_FALSE, EINA_FALSE);
}

EOLIAN static const char *
_efl_canvas_layout_part_text_efl_text_text_get(const Eo *obj,
      void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   return _edje_efl_text_text_get(obj, pd->ed, pd->part, EINA_FALSE, EINA_FALSE);
}

EOLIAN static const char *
_efl_canvas_layout_part_text_efl_text_markup_markup_get(const Eo *obj,
      void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   return _edje_efl_text_text_get(obj, pd->ed, pd->part, EINA_FALSE, EINA_TRUE);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_markup_markup_set(Eo *obj,
      void *_pd EINA_UNUSED, const char *text)
{
   PROXY_DATA_GET(obj, pd);
   _edje_efl_text_text_set(obj, pd->ed, pd->part, text, EINA_FALSE, EINA_TRUE);
}


EOLIAN static Efl_Text_Cursor_Cursor *
_efl_canvas_layout_part_text_efl_text_cursor_text_cursor_get(const Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Get_Type get_type)
{
   PROXY_DATA_GET(obj, pd);
   return _edje_text_cursor_get(pd->rp, (int) get_type);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_cursor_cursor_paragraph_first(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor *cur)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_begin(pd->rp, cur);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_cursor_cursor_paragraph_last(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor *cur)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_end(pd->rp, cur);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_cursor_cursor_position_set(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor *cur, int pos)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_pos_set(pd->rp, cur, pos);
}

EOLIAN static int
_efl_canvas_layout_part_text_efl_text_cursor_cursor_position_get(const Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor *cur)
{
   PROXY_DATA_GET(obj, pd);
   return _edje_text_cursor_pos_get(pd->rp, cur);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_cursor_cursor_coord_set(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor *cur,
      int x, int y)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_coord_set(pd->rp, cur, x, y);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_cursor_cursor_line_char_first(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor *cur)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_line_begin(pd->rp, cur);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_cursor_cursor_line_char_last(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor *cur)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_line_end(pd->rp, cur);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_cursor_cursor_char_prev(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor *cur)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_prev(pd->rp, cur);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_cursor_cursor_char_next(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor *cur)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_next(pd->rp, cur);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_cursor_cursor_line_jump_by(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor *cur, int by)
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
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_cursor_cursor_copy(Eo *obj,
      void *_pd EINA_UNUSED,
      Efl_Text_Cursor_Cursor *dst,
      const Efl_Text_Cursor_Cursor *cur)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_copy(pd->rp, (Efl_Text_Cursor_Cursor *) cur, dst);
}

EOLIAN static Eina_Unicode
_efl_canvas_layout_part_text_efl_text_cursor_cursor_content_get(const Eo *obj,
      void *_pd EINA_UNUSED,
      const Efl_Text_Cursor_Cursor *cur)
{
   Eina_Unicode *ustr;
   Eina_Unicode uc = { 0 };
   char *c;
   int len;

   PROXY_DATA_GET(obj, pd);
   c = _edje_text_cursor_content_get(pd->rp, (Efl_Text_Cursor_Cursor *) cur);
   ustr = eina_unicode_utf8_to_unicode(c, &len);
   free(c);
   if (ustr)
     {
        uc = *ustr;
        free(ustr);
     }
   return uc;
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_text_efl_text_cursor_cursor_geometry_get(const Eo *obj,
      void *_pd EINA_UNUSED,
      const Efl_Text_Cursor_Cursor *cur EINA_UNUSED,
      Efl_Text_Cursor_Type ctype EINA_UNUSED,
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
   return EINA_FALSE;
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_markup_cursor_markup_insert(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor *cur EINA_UNUSED,
      const char *text)
{
   PROXY_DATA_GET(obj, pd);
   // Ignoring cursor type and always inserting with main cursor
   _edje_object_part_text_insert(pd->ed, pd->rp, text);
   // FIXME: entry should report the length of inserted text (after filtering)
}

#include "efl_canvas_layout_part_text.eo.c"


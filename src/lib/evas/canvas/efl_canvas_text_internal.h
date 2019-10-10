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
   Efl2_Canvas_Text                *obj;
   Efl2_Text_Cursor                *cur_obj;
   size_t                           pos;
   Evas_Object_Textblock_Node_Text *node;
   Eina_Bool                        changed : 1;
};

struct _Efl2_Text_Attribute_Handle
{
   EINA_INLIST;
   Eina_Stringshare *tag;
   Eina_Stringshare *properties;
   Eina_Bool ephemeral;
   // FIXME: Just add cursor handles for the start/end so they are updated automatically?
   int ref;  /**< Value of the ref. */
};

Eina_Iterator *_canvas_text_selection_iterator_new(Eina_List *list);

Eina_Unicode _canvas_text_cursor_content_get(const Efl2_Text_Cursor_Handle *cur);
void _canvas_text_cursor_range_delete(Efl2_Text_Cursor_Handle *cur1, Efl2_Text_Cursor_Handle *cur2);
void _canvas_text_cursor_char_delete(Efl2_Text_Cursor_Handle *cur);
unsigned int _canvas_text_cursor_text_insert(Efl2_Text_Cursor_Handle *cur, const char *_text);
void _canvas_text_cursor_copy(Efl2_Text_Cursor_Handle *dst, const Efl2_Text_Cursor_Handle *src);
Eina_Bool _canvas_text_cursor_equal(const Efl2_Text_Cursor_Handle *cur, const Efl2_Text_Cursor_Handle *cur2);
int _canvas_text_cursor_compare(const Efl2_Text_Cursor_Handle *cur1, const Efl2_Text_Cursor_Handle *cur2);
Eina_Bool _canvas_text_cursor_line_jump_by(Efl2_Text_Cursor_Handle *cur, int by);
int _canvas_text_cursor_line_number_get(const Efl2_Text_Cursor_Handle *cur);
Eina_Bool _canvas_text_cursor_line_number_set(Efl2_Text_Cursor_Handle *cur, int line);
void _canvas_text_cursor_position_set(Efl2_Text_Cursor_Handle *cur, int _pos);
int _canvas_text_cursor_position_get(const Efl2_Text_Cursor_Handle *cur);
Eina_Bool _canvas_text_cursor_line_end(Efl2_Text_Cursor_Handle *cur);
Eina_Bool _canvas_text_cursor_line_start(Efl2_Text_Cursor_Handle *cur);
Eina_Bool _canvas_text_cursor_paragraph_end(Efl2_Text_Cursor_Handle *cur);
Eina_Bool _canvas_text_cursor_paragraph_start(Efl2_Text_Cursor_Handle *cur);
Eina_Bool _canvas_text_cursor_prev(Efl2_Text_Cursor_Handle *cur);
Eina_Bool _canvas_text_cursor_next(Efl2_Text_Cursor_Handle *cur);
size_t _canvas_text_cursor_cluster_post_get(Efl2_Text_Cursor_Handle *cur, Eina_Bool inc);
Eina_Bool _canvas_text_cursor_word_end(Efl2_Text_Cursor_Handle *cur);
Eina_Bool _canvas_text_cursor_word_start(Efl2_Text_Cursor_Handle *cur);
Eina_Bool _canvas_text_cursor_grapheme_end(Efl2_Text_Cursor_Handle *cur);
Eina_Bool _canvas_text_cursor_grapheme_start(Efl2_Text_Cursor_Handle *cur);
Eina_Bool _canvas_text_cursor_paragraph_prev(Efl2_Text_Cursor_Handle *cur);
Eina_Bool _canvas_text_cursor_paragraph_next(Efl2_Text_Cursor_Handle *cur);
Eina_Bool _canvas_text_cursor_paragraph_first(Efl2_Text_Cursor_Handle *cur);
Eina_Bool _canvas_text_cursor_paragraph_last(Efl2_Text_Cursor_Handle *cur);
Efl2_Text_Cursor_Handle * _canvas_text_cursor_new(Efl2_Canvas_Text *eo_obj);
void _canvas_text_cursor_free(Efl2_Text_Cursor_Handle *cur);
void _canvas_text_cursor_init(Efl2_Text_Cursor_Handle *cur, const Evas_Object *tb);
void _canvas_text_cursor_emit_if_changed(Efl2_Text_Cursor_Handle *cur);
char *_canvas_text_cursor_text_plain_get(const Efl2_Text_Cursor_Handle *cur1, const Efl2_Text_Cursor_Handle *_cur2);
Eina_Bool _canvas_text_cursor_geometry_get(const Efl2_Text_Cursor_Handle *cur, Efl2_Text_Cursor_Type ctype, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch, Evas_Coord *cx2, Evas_Coord *cy2, Evas_Coord *cw2, Evas_Coord *ch2);
void _canvas_text_cursor_content_geometry_get(const Efl2_Text_Cursor_Handle *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch);
Eina_List *_canvas_text_cursor_range_precise_geometry_get(const Efl2_Text_Cursor_Handle *cur1, const Efl2_Text_Cursor_Handle *cur2);
Eina_Iterator *_canvas_text_cursor_range_simple_geometry_get(const Efl2_Text_Cursor_Handle *cur1, const Efl2_Text_Cursor_Handle *cur2);
Eina_Bool _canvas_text_cursor_coord_set(Efl2_Text_Cursor_Handle *cur, Evas_Coord x, Evas_Coord y);
Eina_Bool _canvas_text_cursor_eol_get(const Efl2_Text_Cursor_Handle *cur);
#endif

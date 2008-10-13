/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "edje_private.h"

typedef struct _Entry Entry;

struct _Entry
{
   Evas_Coord   cx, cy;
   Evas_Object *cursor_bg;
   Evas_Object *cursor_fg;
   Evas_Textblock_Cursor *cursor;
   Evas_Textblock_Cursor *sel_start, *sel_end;
   Evas_List *sel;
   Evas_Bool have_selection : 1;
};

// FIXME: this has to emit signals for "request selection", "set selection"
// so copy & paste work, need api calls to insert text, insert format,
// get text (with markup) delete text, etc. etc. etc.
// 
// FIXME: cursor when at end of text doesnt display right

static void
_edje_focus_in_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Edje *ed = data;
   _edje_emit(ed, "focus,in", "");
}
    
static void
_edje_focus_out_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Edje *ed = data;
   _edje_emit(ed, "focus,out", "");
}

static void
_edje_key_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Edje *ed = data;
   Evas_Event_Key_Down *ev = event_info;
   Edje_Real_Part *rp = ed->focused_part;
   Entry *en = rp->entry_data;
   Evas_Bool control, alt, shift;
   if ((!rp) || (!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_EDITABLE))
     return;
   if (!ev->key) return;
   
   printf("editable: '%s' '%s' '%s'\n", ev->key, ev->string, ev->compose);
   control = evas_key_modifier_is_set(ev->modifiers, "Control");
   alt = evas_key_modifier_is_set(ev->modifiers, "Alt");
   shift = evas_key_modifier_is_set(ev->modifiers, "Shift");
   if (!strcmp(ev->key, "Escape"))
     {
	// dead keys here. Escape for now (shoudl emit these)
     }
   else if (!strcmp(ev->key, "Up"))
     {
	Evas_Coord lx, ly, lw, lh, cx, cy, cw, ch;
	int line_num;
	
	if (shift)
	  {
	     // if no selection, start one 
	  }
	line_num = evas_textblock_cursor_line_geometry_get(en->cursor, NULL, NULL, NULL, NULL);
	line_num--;
	if (evas_object_textblock_line_number_geometry_get(rp->object, line_num, &lx, &ly, &lw, &lh))
	  {
	     evas_textblock_cursor_char_geometry_get(en->cursor, &cx, &cy, &cw, &ch);
	     evas_textblock_cursor_char_coord_set(en->cursor, cx + (cw / 2), ly + (lh / 2));
	  }
	if (shift)
	  {
	     // also extend selection
	  }
     }
   else if (!strcmp(ev->key, "Down"))
     {
	Evas_Coord lx, ly, lw, lh, cx, cy, cw, ch;
	int line_num;
	
	if (shift)
	  {
	     // if no selection, start one 
	  }
	line_num = evas_textblock_cursor_line_geometry_get(en->cursor, NULL, NULL, NULL, NULL);
	line_num++;
	if (evas_object_textblock_line_number_geometry_get(rp->object, line_num, &lx, &ly, &lw, &lh))
	  {
	     evas_textblock_cursor_char_geometry_get(en->cursor, &cx, &cy, &cw, &ch);
	     evas_textblock_cursor_char_coord_set(en->cursor, cx + (cw / 2), ly + (lh / 2));
	  }
	if (shift)
	  {
	     // also extend selection
	  }
     }
   else if (!strcmp(ev->key, "Left"))
     {
	if (shift)
	  {
	     // if no selection, start one 
	  }
	if (!evas_textblock_cursor_char_prev(en->cursor))
	  {
	     evas_textblock_cursor_node_prev(en->cursor);
	  }
	if (shift)
	  {
	     // also extend selection
	  }
     }
   else if (!strcmp(ev->key, "Right"))
     {
	if (shift)
	  {
	     // if no selection, start one 
	  }
	if (!evas_textblock_cursor_char_next(en->cursor))
	  {
	     evas_textblock_cursor_node_next(en->cursor);
	  }
	if (shift)
	  {
	     // also extend selection
	  }
     }
   else if (!strcmp(ev->key, "BackSpace"))
     {
	if (control)
	  {
	     // if ctrl pressed del to start of previois word
	  }
	else if ((alt) && (shift))
	  {
	     // FIXME: undo last action
	  }
	else
	  {
	     if ((en->sel_start) && (en->sel_end) && (en->have_selection))
	       {
		  evas_textblock_cursor_range_delete(en->sel_start, en->sel_end);
	       }
	     else
	       {
		  evas_textblock_cursor_node_prev(en->cursor);
		  evas_textblock_cursor_char_delete(en->cursor);
	       }
	  }
	if ((en->sel_start) && (en->sel_end))
	  {
	     // FIXME: func
	     evas_textblock_cursor_free(en->sel_start);
	     en->sel_start = NULL;
	     evas_textblock_cursor_free(en->sel_end);
	     en->sel_end = NULL;
	  }
     }
   else if (!strcmp(ev->key, "Delete"))
     {
	if (control)
	  {
	     // if ctrl pressed del to end of next word
	  }
	else if (shift)
	  {
	     // cut
	  }
	else
	  {	
	     if ((en->sel_start) && (en->sel_end) && (en->have_selection))
	       {
		  evas_textblock_cursor_range_delete(en->sel_start, en->sel_end);
	       }
	     else
	       {
		  evas_textblock_cursor_char_delete(en->cursor);
	       }
	  }
	if ((en->sel_start) && (en->sel_end))
	  {
	     // FIXME: func
	     evas_textblock_cursor_free(en->sel_start);
	     en->sel_start = NULL;
	     evas_textblock_cursor_free(en->sel_end);
	     en->sel_end = NULL;
	  }
     }
   else if (!strcmp(ev->key, "Home"))
     {
	if (shift)
	  {
	     // if no selection, start one 
	  }
	if (control)
	  {
	     // goto start of text
	  }
	else
	  {
	     evas_textblock_cursor_line_first(en->cursor);
	  }
	if (shift)
	  {
	     // also extend selection
	  }
     }
   else if (!strcmp(ev->key, "End"))
     {
	if (shift)
	  {
	     // if no selection, start one 
	  }
	if (control)
	  {
	     // goto end of text
	  }
	else
	  {
	     evas_textblock_cursor_line_last(en->cursor);
	  }
	if (shift)
	  {
	     // also extend selection
	  }
     }
   else if ((control) && (!strcmp(ev->key, "v")))
     {
	// paste
     }
   else if ((control) && ((!strcmp(ev->key, "c") || (!strcmp(ev->key, "Insert")))))
     {
	// copy
     }
   else if ((control) && ((!strcmp(ev->key, "x") || (!strcmp(ev->key, "m")))))
     {
	// cut
     }
   else if ((control) && (!strcmp(ev->key, "z")))
     {
	if (shift)
	  {
	     // FIXME: redo
	  }
	else
	  {
	     // FIXME: undo
	  }
     }
   else if ((control) && (!strcmp(ev->key, "y")))
     {
	// FIXME: redo
     }
   else if ((control) && (!strcmp(ev->key, "w")))
     {
	// select current word
     }
   else if (!strcmp(ev->key, "Tab"))
     {
	if (shift)
	  {
	     // remove a tab
	  }
	else
	  {
	     // add a tab
	  }
     }
   else if (!strcmp(ev->key, "ISO_Left_Tab"))
     {
	// remove a tab
     }
   else if (!strcmp(ev->key, "Prior"))
     {
	// pgup
     }
   else if (!strcmp(ev->key, "Next"))
     {
	// pgdn
     }
   else if ((!strcmp(ev->key, "Return")) || (!strcmp(ev->key, "KP_Enter")))
     {
	// newline
     }
   else if ((!strcmp(ev->key, "Multi_key")))
     {
	// FIXME: compose next 2 keystrokes (Examples):
	// a " -> ä
	// o / -> ø
	// a e -> æ
	// e ' -> é
	// s s -> ß
	// etc.
     }
   // FIXME: other input methods? (xim, scim, uim etc.)...
   else
     {
	// FIXME: if composing.. store 2 keys
	if (ev->string)
	  evas_textblock_cursor_text_prepend(en->cursor, ev->string);
     }
   _edje_entry_real_part_configure(rp);
}

static void
_edje_key_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Edje *ed = data;
   Evas_Event_Key_Up *ev = event_info;
   Edje_Real_Part *rp = ed->focused_part;
   Entry *en = rp->entry_data;
   if ((!rp) || (!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_EDITABLE))
     return;
}

static void
_edje_part_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Edje_Real_Part *rp = data;
   Evas_Event_Mouse_Down *ev = event_info;
   Entry *en = rp->entry_data;
   Evas_Coord x, y, w, h;
   if ((!rp) || (!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_EDITABLE))
     return;
   evas_object_geometry_get(rp->object, &x, &y, &w, &h);
   en->cx = ev->canvas.x - x;
   en->cy = ev->canvas.y - y;
   if (!evas_textblock_cursor_char_coord_set(en->cursor, en->cx, en->cy))
     {
	Evas_Coord lx, ly, lw, lh;
	
	evas_textblock_cursor_line_coord_set(en->cursor, en->cy);
	evas_textblock_cursor_line_geometry_get(en->cursor, &lx, &ly, &lw, &lh);
	if (en->cx <= lx)
	  evas_textblock_cursor_line_first(en->cursor);
	else
	  evas_textblock_cursor_line_last(en->cursor);
     }
   en->sel_start = evas_object_textblock_cursor_new(rp->object);
   evas_textblock_cursor_copy(en->cursor, en->sel_start);
   en->sel_end = evas_object_textblock_cursor_new(rp->object);
   evas_textblock_cursor_copy(en->cursor, en->sel_start);
   en->have_selection = 0;
   while (en->sel)
     {
	evas_object_del(en->sel->data);
	en->sel = evas_list_remove_list(en->sel, en->sel);
     }
   // FIXME: emit "selection cleared"
   _edje_entry_real_part_configure(rp);
}

static void
_edje_part_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Edje_Real_Part *rp = data;
   Evas_Event_Mouse_Up *ev = event_info;
   Entry *en = rp->entry_data;
   Evas_Coord x, y, w, h;
   if ((!rp) || (!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_EDITABLE))
     return;
   evas_object_geometry_get(rp->object, &x, &y, &w, &h);
   en->cx = ev->canvas.x - x;
   en->cy = ev->canvas.y - y;
   if (!evas_textblock_cursor_char_coord_set(en->cursor, en->cx, en->cy))
     {
	Evas_Coord lx, ly, lw, lh;
	
	evas_textblock_cursor_line_coord_set(en->cursor, en->cy);
	evas_textblock_cursor_line_geometry_get(en->cursor, &lx, &ly, &lw, &lh);
	if (en->cx <= lx)
	  evas_textblock_cursor_line_first(en->cursor);
	else
	  evas_textblock_cursor_line_last(en->cursor);
     }
   evas_textblock_cursor_copy(en->cursor, en->sel_end);
   
   // FIXME: emit "selection ended"
   
   evas_textblock_cursor_free(en->sel_start);
   en->sel_start = NULL;
   evas_textblock_cursor_free(en->sel_end);
   en->sel_end = NULL;
   _edje_entry_real_part_configure(rp);
}

static void
_edje_part_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Edje_Real_Part *rp = data;
   Evas_Event_Mouse_Move *ev = event_info;
   Entry *en = rp->entry_data;
   Evas_Coord x, y, w, h;
   if ((!rp) || (!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_EDITABLE))
     return;
   if (!en->sel_start) return;
   evas_object_geometry_get(rp->object, &x, &y, &w, &h);
   en->cx = ev->cur.canvas.x - x;
   en->cy = ev->cur.canvas.y - y;
   if (!evas_textblock_cursor_char_coord_set(en->cursor, en->cx, en->cy))
     {
	Evas_Coord lx, ly, lw, lh;
	
	evas_textblock_cursor_line_coord_set(en->cursor, en->cy);
	evas_textblock_cursor_line_geometry_get(en->cursor, &lx, &ly, &lw, &lh);
	if (en->cx <= lx)
	  evas_textblock_cursor_line_first(en->cursor);
	else
	  evas_textblock_cursor_line_last(en->cursor);
     }
   evas_textblock_cursor_copy(en->cursor, en->sel_end);
   if (evas_textblock_cursor_compare(en->sel_start, en->sel_end) != 0)
     {
	en->have_selection = 1;
	// FIXME: emit "selection started"
     }
   if (en->have_selection)
     {
	Evas_List *range;
	
	printf("update sel\n");
	range = evas_textblock_cursor_range_geometry_get(en->sel_start, en->sel_end);
	while (en->sel)
	  {
	     evas_object_del(en->sel->data);
	     en->sel = evas_list_remove_list(en->sel, en->sel);
	  }
	while (range)
	  {
	     Evas_Textblock_Rectangle *r;
	     Evas_Object *o;
	     
	     r = range->data;
	     
	     o = evas_object_rectangle_add(evas_object_evas_get(rp->object));
	     evas_object_smart_member_add(o, rp->edje->obj);
	     evas_object_stack_below(o, rp->object);
	     evas_object_clip_set(o, evas_object_clip_get(rp->object));
	     evas_object_color_set(o, 20, 20, 255, 150);
	     evas_object_pass_events_set(o, 1);
	     evas_object_show(o);
	     evas_object_move(o, x + r->x, y + r->y);
	     evas_object_resize(o, r->w, r->h);
	     en->sel = evas_list_append(en->sel, o);
	     
	     range = evas_list_remove_list(range, range);
	     free(r);
	  }
	// FIXME: emit "selection changed"
     }
   _edje_entry_real_part_configure(rp);
}

/***************************************************************/
void
_edje_entry_init(Edje *ed)
{
   evas_object_event_callback_add(ed->obj, EVAS_CALLBACK_FOCUS_IN, _edje_focus_in_cb, ed);
   evas_object_event_callback_add(ed->obj, EVAS_CALLBACK_FOCUS_OUT, _edje_focus_out_cb, ed);
   evas_object_event_callback_add(ed->obj, EVAS_CALLBACK_KEY_DOWN, _edje_key_down_cb, ed);
   evas_object_event_callback_add(ed->obj, EVAS_CALLBACK_KEY_UP, _edje_key_up_cb, ed);
}

void
_edje_entry_shutdown(Edje *ed)
{
}

void
_edje_entry_real_part_init(Edje_Real_Part *rp)
{
   Entry *en;
   
   en = calloc(1, sizeof(Entry));
   if (!en) return;
   rp->entry_data = en;

   evas_object_event_callback_add(rp->object, EVAS_CALLBACK_MOUSE_DOWN, _edje_part_mouse_down_cb, rp);
   evas_object_event_callback_add(rp->object, EVAS_CALLBACK_MOUSE_UP, _edje_part_mouse_up_cb, rp);
   evas_object_event_callback_add(rp->object, EVAS_CALLBACK_MOUSE_MOVE, _edje_part_mouse_move_cb, rp);

   // FIXME: make cursor object correct
   en->cursor_bg = evas_object_rectangle_add(evas_object_evas_get(rp->object));
   evas_object_smart_member_add(en->cursor_bg, rp->edje->obj);
   evas_object_stack_below(en->cursor_bg, rp->object);
   evas_object_clip_set(en->cursor_bg, evas_object_clip_get(rp->object));
   evas_object_color_set(en->cursor_bg, 255, 20, 20, 150);
   evas_object_pass_events_set(en->cursor_bg, 1);
   evas_object_show(en->cursor_bg);
   
   en->cursor = evas_object_textblock_cursor_get(rp->object);
}

void
_edje_entry_real_part_shutdown(Edje_Real_Part *rp)
{
   Entry *en = rp->entry_data;
   if (!en) return;
   rp->entry_data = NULL;
   // FIXME: delete cursor objects, sel cursors and en->sel's undo buffer copy/cut buffer
   free(en);
}

void
_edje_entry_real_part_configure(Edje_Real_Part *rp)
{
   Evas_Coord x, y, w, h, xx, yy, ww, hh;
   Entry *en = rp->entry_data;
   if (!en) return;
   
   evas_object_geometry_get(rp->object, &x, &y, &w, &h);
   // move cursor/selections etc.
   evas_textblock_cursor_char_geometry_get(en->cursor, &xx, &yy, &ww, &hh);
   evas_object_move(en->cursor_bg, x + xx, y + yy);
   if (ww < 1) ww = 1;
   if (hh < 1) ww = 1;
   evas_object_resize(en->cursor_bg, ww, hh);
   // FIXME: move/resize en->sel (record the sels and intended geom)
}

// FIXME: need to implement
// get text
// insert text
// delete selected text
// get selection
// 

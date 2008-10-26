/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
void *alloca (size_t);
#endif

#include "edje_private.h"

// FIXME: need a way to propagate emits to selections and cursors (eg for disabled etc.)
// FIXME: look for anchors <+ a href=X>...</> in nodes - make matching anchor
//        ranges (2 cursors) AND then make matching acnhor event objects,
//        capture events on those objects, emit achor mouse,in, mouse,out,
//        mouse,down and mouse,up 
// FIXME: apis 's query anchor stuff???, anchor revents

typedef struct _Entry Entry;
typedef struct _Sel Sel;
typedef struct _Anchor Anchor;

struct _Entry
{
   Edje_Real_Part *rp;
   Evas_Coord cx, cy;
   Evas_Object *cursor_bg;
   Evas_Object *cursor_fg;
   Evas_Textblock_Cursor *cursor;
   Evas_Textblock_Cursor *sel_start, *sel_end;
   Eina_List *sel;
   Eina_List *anchors;
   Eina_List *anchorlist;
   char *selection;
   Evas_Bool selecting : 1;
   Evas_Bool have_selection : 1;
};

struct _Sel
{
   Evas_Textblock_Rectangle rect;
   Evas_Object *obj_fg, *obj_bg, *obj, *sobj;
};

struct _Anchor
{
   Entry *en;
   char *name;
   Evas_Textblock_Cursor *start, *end;
   Eina_List *sel;
};

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
_curs_update_from_curs(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   Evas_Coord cx, cy, cw, ch;
   evas_textblock_cursor_char_geometry_get(c, &cx, &cy, &cw, &ch);
   en->cx = cx + (cw / 2);
   en->cy = cy + (ch / 2);
}

static void
_curs_back(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   if (!evas_textblock_cursor_char_prev(c))
     {
	if (evas_textblock_cursor_node_prev(c))
	  {
	     while (evas_textblock_cursor_node_format_get(c))
	       {
		  if (evas_textblock_cursor_node_format_is_visible_get(c)) break;
		  if (!evas_textblock_cursor_node_prev(c)) break;
	       }
	  }
     }
   _curs_update_from_curs(c, o, en);
}

static void
_curs_next(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   if (!evas_textblock_cursor_char_next(c))
     {
	if (evas_textblock_cursor_node_next(c))
	  {
	     while (evas_textblock_cursor_node_format_get(c))
	       {
		  if (evas_textblock_cursor_node_format_is_visible_get(c))
		    break;
		  if (!evas_textblock_cursor_node_next(c))
		    break;
	       }
	  }
     }
   else
     {
	int len, pos;
	
	len = evas_textblock_cursor_node_text_length_get(c);
	pos = evas_textblock_cursor_pos_get(c);
	if (pos == len)
	  evas_textblock_cursor_node_next(c);
     }
   _curs_update_from_curs(c, o, en);
}

static int
_curs_line_last_get(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   Evas_Textblock_Cursor *cc;
   int ln;
   
   cc = evas_object_textblock_cursor_new(o);
   evas_textblock_cursor_node_last(cc);
   ln = evas_textblock_cursor_line_geometry_get(cc, NULL, NULL, NULL, NULL);
   evas_textblock_cursor_free(cc);
   return ln;
}

static void
_curs_jump_line(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en, int ln)
{
   Evas_Coord lx, ly, lw, lh;

   if (ln < 0) ln = 0;
   else
     {
	int last = _curs_line_last_get(c, o, en);
	if (ln > last) ln = last;
     }
   if (!evas_object_textblock_line_number_geometry_get(o, ln, &lx, &ly, &lw, &lh))
     return;
   if (evas_textblock_cursor_char_coord_set(c, en->cx, ly + (lh / 2)))
     return;
   evas_textblock_cursor_line_coord_set(c, ly + (lh / 2));
   if (en->cx < (lx + (lw / 2))) evas_textblock_cursor_line_first(c);
   else evas_textblock_cursor_line_last(c);
}

static void
_curs_jump_line_by(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en, int by)
{
   int ln;
   
   ln = evas_textblock_cursor_line_geometry_get(c, NULL, NULL, NULL, NULL) + by;
   _curs_jump_line(c, o, en, ln);
}

static void
_curs_up(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   _curs_jump_line_by(c, o, en, -1);
}

static void
_curs_down(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   _curs_jump_line_by(c, o, en, 1);
}

static void
_curs_lin_start(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   evas_textblock_cursor_line_first(c);
   _curs_update_from_curs(c, o, en);
}

static void
_curs_lin_end(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   evas_textblock_cursor_line_last(c);
   if (!evas_textblock_cursor_node_format_get(c))
     _curs_next(c, o, en);
   _curs_update_from_curs(c, o, en);
}

static void
_curs_start(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   evas_textblock_cursor_line_set(c, 0);
   evas_textblock_cursor_line_first(c);
   _curs_update_from_curs(c, o, en);
}

static void
_curs_end(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   evas_textblock_cursor_line_set(c, _curs_line_last_get(c, o, en));
   _curs_lin_end(c, o, en);
   _curs_update_from_curs(c, o, en);
}



static void
_sel_start(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   if (en->sel_start) return;
   en->sel_start = evas_object_textblock_cursor_new(o);
   evas_textblock_cursor_copy(c, en->sel_start);
   en->sel_end = evas_object_textblock_cursor_new(o);
   evas_textblock_cursor_copy(c, en->sel_start);
   en->have_selection = 0;
}

static void
_sel_enable(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   if (en->have_selection) return;
   en->have_selection = 1;
   _edje_emit(en->rp->edje, "selection,start", en->rp->part->name);
}

static void
_sel_extend(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   if (!en->sel_end) return;
   _sel_enable(c, o, en);
   if (!evas_textblock_cursor_compare(c, en->sel_end)) return;
   evas_textblock_cursor_copy(c, en->sel_end);
   _edje_emit(en->rp->edje, "selection,changed", en->rp->part->name);
}

static void
_sel_clear(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   if (en->sel_start)
     {
	evas_textblock_cursor_free(en->sel_start);
	evas_textblock_cursor_free(en->sel_end);
	en->sel_start = NULL;
	en->sel_end = NULL;
     }
   if (en->selection)
     {
	free(en->selection);
	en->selection = NULL;
     }
   while (en->sel)
     {
	Sel *sel;
	
	sel = en->sel->data;
	if (sel->obj_bg) evas_object_del(sel->obj_bg);
	if (sel->obj_fg) evas_object_del(sel->obj_fg);
	free(sel);
	en->sel = eina_list_remove_list(en->sel, en->sel);
     }
   if (en->have_selection)
     {
        en->have_selection = 0;
	_edje_emit(en->rp->edje, "selection,cleared", en->rp->part->name);
     }
}

static void
_sel_update(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   Eina_List *range = NULL, *l;
   Sel *sel;
   Evas_Coord x, y, w, h;
   Evas_Object *smart, *clip;
   
   smart = evas_object_smart_parent_get(o);
   clip = evas_object_clip_get(o);
   if (en->sel_start)
     range = evas_textblock_cursor_range_geometry_get(en->sel_start, en->sel_end);
   if (eina_list_count(range) != eina_list_count(en->sel))
     {
	while (en->sel)
	  {
	     sel = en->sel->data;
	     if (sel->obj_bg) evas_object_del(sel->obj_bg);
	     if (sel->obj_fg) evas_object_del(sel->obj_fg);
	     free(sel);
	     en->sel = eina_list_remove_list(en->sel, en->sel);
	  }
	if (en->have_selection)
	  {
	     for (l = range; l; l = eina_list_next(l))
	       {
		  Evas_Object *ob;
		  
		  sel = calloc(1, sizeof(Sel));
		  en->sel = eina_list_append(en->sel, sel);
		  ob = edje_object_add(en->rp->edje->evas);
		  edje_object_file_set(ob, en->rp->edje->path, en->rp->part->source);
		  evas_object_smart_member_add(ob, smart);
		  evas_object_stack_below(ob, o);
		  evas_object_clip_set(ob, clip);
		  evas_object_pass_events_set(ob, 1);
		  evas_object_show(ob);
		  sel->obj_bg = ob;
		  ob = edje_object_add(en->rp->edje->evas);
		  edje_object_file_set(ob, en->rp->edje->path, en->rp->part->source2);
		  evas_object_smart_member_add(ob, smart);
		  evas_object_stack_above(ob, o);
		  evas_object_clip_set(ob, clip);
		  evas_object_pass_events_set(ob, 1);
		  evas_object_show(ob);
		  sel->obj_fg = ob;
	       }
	  }
     }
   x = y = w = h = -1;
   evas_object_geometry_get(o, &x, &y, &w, &h);
   EINA_LIST_FOREACH(en->sel, l, sel)
     {
	Evas_Textblock_Rectangle *r;

	r = range->data;
	if (sel->obj_bg)
	  {
	     evas_object_move(sel->obj_bg, x + r->x, y + r->y);
	     evas_object_resize(sel->obj_bg, r->w, r->h);
	  }
	if (sel->obj_fg)
	  {
	     evas_object_move(sel->obj_fg, x + r->x, y + r->y);
	     evas_object_resize(sel->obj_fg, r->w, r->h);
	  }
	range = eina_list_remove_list(range, range);
	free(r);
     }
}

static void
_edje_anchor_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Anchor *an = data;
   Evas_Event_Mouse_Down *ev = event_info;
   Edje_Real_Part *rp = an->en->rp;
   char *buf, *n;
   int len;
   int ignored;

   ignored = rp->part->ignore_flags & ev->event_flags;
   if ((!ev->event_flags) || (!ignored))
     {
	n = an->name;
	if (!n) n = "";
	len = 200 + strlen(n);
	buf = alloca(len);
        if (ev->flags & EVAS_BUTTON_TRIPLE_CLICK)
          snprintf(buf, len, "anchor,mouse,down,%i,%s,triple", ev->button, an->name);
        else if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
	  snprintf(buf, len, "anchor,mouse,down,%i,%s,double", ev->button, an->name);
	else
	  snprintf(buf, len, "anchor,mouse,down,%i,%s", ev->button, an->name);
	_edje_emit(rp->edje, buf, rp->part->name);
     }
}

static void
_edje_anchor_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Anchor *an = data;
   Evas_Event_Mouse_Up *ev = event_info;
   Edje_Real_Part *rp = an->en->rp;
   char *buf, *n;
   int len;
   int ignored;

   ignored = rp->part->ignore_flags & ev->event_flags;
   if ((!ev->event_flags) || (!ignored))
     {
	n = an->name;
	if (!n) n = "";
	len = 200 + strlen(n);
	buf = alloca(len);
	snprintf(buf, len, "anchor,mouse,up,%i,%s", ev->button, an->name);
	_edje_emit(rp->edje, buf, rp->part->name);
     }
}

static void
_edje_anchor_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Anchor *an = data;
   Evas_Event_Mouse_Move *ev = event_info;
   Edje_Real_Part *rp = an->en->rp;
   char *buf, *n;
   int len;
   int ignored;

   ignored = rp->part->ignore_flags & ev->event_flags;
   if ((!ev->event_flags) || (!ignored))
     {
	n = an->name;
	if (!n) n = "";
	len = 200 + strlen(n);
	buf = alloca(len);
	snprintf(buf, len, "anchor,mouse,move,%s", an->name);
	_edje_emit(rp->edje, buf, rp->part->name);
     }
}

static void
_edje_anchor_mouse_in_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Anchor *an = data;
   Evas_Event_Mouse_In *ev = event_info;
   Edje_Real_Part *rp = an->en->rp;
   char *buf, *n;
   int len;
   int ignored;

   ignored = rp->part->ignore_flags & ev->event_flags;
   if ((!ev->event_flags) || (!ignored))
     {
	n = an->name;
	if (!n) n = "";
	len = 200 + strlen(n);
	buf = alloca(len);
	snprintf(buf, len, "anchor,mouse,in,%s", an->name);
	_edje_emit(rp->edje, buf, rp->part->name);
     }
}

static void
_edje_anchor_mouse_out_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Anchor *an = data;
   Evas_Event_Mouse_Out *ev = event_info;
   Edje_Real_Part *rp = an->en->rp;
   char *buf, *n;
   int len;
   int ignored;

   ignored = rp->part->ignore_flags & ev->event_flags;
   if ((!ev->event_flags) || (!ignored))
     {
	n = an->name;
	if (!n) n = "";
	len = 200 + strlen(n);
	buf = alloca(len);
	snprintf(buf, len, "anchor,mouse,out,%s", an->name);
	_edje_emit(rp->edje, buf, rp->part->name);
     }
}

static void
_anchors_update(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   Eina_List *l, *ll, *range;
   Evas_Coord x, y, w, h;
   Evas_Object *smart, *clip;
   Sel *sel;
   Anchor *an;

   smart = evas_object_smart_parent_get(o);
   clip = evas_object_clip_get(o);
   x = y = w = h = -1;
   evas_object_geometry_get(o, &x, &y, &w, &h);
   EINA_LIST_FOREACH(en->anchors, l, an)
     {
	range = evas_textblock_cursor_range_geometry_get(an->start, an->end);
	if (eina_list_count(range) != eina_list_count(an->sel))
	  {
	     while (an->sel)
	       {
		  sel = an->sel->data;
		  if (sel->obj_bg) evas_object_del(sel->obj_bg);
		  if (sel->obj_fg) evas_object_del(sel->obj_fg);
		  if (sel->obj) evas_object_del(sel->obj);
		  free(sel);
		  an->sel = eina_list_remove_list(an->sel, an->sel);
	       }
	     for (ll = range; ll; ll = eina_list_next(ll))
	       {
		  Evas_Object *ob;

		  sel = calloc(1, sizeof(Sel));
		  an->sel = eina_list_append(an->sel, sel);
		  ob = edje_object_add(en->rp->edje->evas);
		  edje_object_file_set(ob, en->rp->edje->path, en->rp->part->source5);
		  evas_object_smart_member_add(ob, smart);
		  evas_object_stack_below(ob, o);
		  evas_object_clip_set(ob, clip);
		  evas_object_pass_events_set(ob, 1);
		  evas_object_show(ob);
		  sel->obj_bg = ob;
		  ob = edje_object_add(en->rp->edje->evas);
		  edje_object_file_set(ob, en->rp->edje->path, en->rp->part->source6);
		  evas_object_smart_member_add(ob, smart);
		  evas_object_stack_above(ob, o);
		  evas_object_clip_set(ob, clip);
		  evas_object_pass_events_set(ob, 1);
		  evas_object_show(ob);
		  sel->obj_fg = ob;
		  ob = evas_object_rectangle_add(en->rp->edje->evas);
		  evas_object_color_set(ob, 0, 0, 0, 0);
		  evas_object_smart_member_add(ob, smart);
		  evas_object_stack_above(ob, o);
		  evas_object_clip_set(ob, clip);
		  evas_object_repeat_events_set(ob, 1);
		  evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_DOWN, _edje_anchor_mouse_down_cb, an);
		  evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_UP, _edje_anchor_mouse_up_cb, an);
		  evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_MOVE, _edje_anchor_mouse_move_cb, an);
		  evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_IN, _edje_anchor_mouse_in_cb, an);
		  evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_OUT, _edje_anchor_mouse_out_cb, an);
		  evas_object_show(ob);
		  sel->obj = ob;
	       }
	  }
	EINA_LIST_FOREACH(an->sel, ll, sel)
          {
	     Evas_Textblock_Rectangle *r;

	     r = range->data;
	     *(&(sel->rect)) = *r;
	     if (sel->obj_bg)
	       {
		  evas_object_move(sel->obj_bg, x + r->x, y + r->y);
		  evas_object_resize(sel->obj_bg, r->w, r->h);
	       }
	     if (sel->obj_fg)
	       {
		  evas_object_move(sel->obj_fg, x + r->x, y + r->y);
		  evas_object_resize(sel->obj_fg, r->w, r->h);
	       }
	     if (sel->obj)
	       {
		  evas_object_move(sel->obj, x + r->x, y + r->y);
		  evas_object_resize(sel->obj, r->w, r->h);
	       }
	     range = eina_list_remove_list(range, range);
	     free(r);
	  }
     }
}

static void
_anchors_clear(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   while (en->anchorlist)
     {
	free(en->anchorlist->data);
	en->anchorlist = eina_list_remove_list(en->anchorlist, en->anchorlist);
     }
   while (en->anchors)
     {
	Anchor *an = en->anchors->data;
	
	evas_textblock_cursor_free(an->start);
	evas_textblock_cursor_free(an->end);
	while (an->sel)
	  {
	     Sel *sel = an->sel->data;
	     if (sel->obj_bg) evas_object_del(sel->obj_bg);
	     if (sel->obj_fg) evas_object_del(sel->obj_fg);
	     if (sel->obj) evas_object_del(sel->obj);
	     free(sel);
	     an->sel = eina_list_remove_list(an->sel, an->sel);
	  }
	free(an);
	en->anchors = eina_list_remove_list(en->anchors, en->anchors);
     }
}

static void
_anchors_get(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   Evas_Textblock_Cursor *c1;
   Anchor *an = NULL;
   int firsttext = 0;

   _anchors_clear(c, o, en);
   c1 = evas_object_textblock_cursor_new(o);
   evas_textblock_cursor_node_first(c1);
   do 
     {
	char *s;
	
	s = evas_textblock_cursor_node_format_get(c1);
	if (s)
	  {
	     if ((!strncmp(s, "+ a ", 4)) || (!strncmp(s, "+a ", 3)))
	       {
		  an = calloc(1, sizeof(Anchor));
		  if (an)
		    {
		       char *p;
		       
		       an->en = en;
		       p = strstr(s, "href=");
		       if (p)
			 {
			    an->name = strdup(p + 5);
			 }
		       en->anchors = eina_list_append(en->anchors, an);
		       an->start = evas_object_textblock_cursor_new(o);
		       an->end = evas_object_textblock_cursor_new(o);
		       evas_textblock_cursor_copy(c1, an->start);
		       evas_textblock_cursor_copy(c1, an->end);
		    }
	       }
	     else if ((!strcmp(s, "- a")) || (!strcmp(s, "-a")))
	       {
		  if (an)
		    {
		       if (!firsttext)
			 {
			    if (an->name) free(an->name);
			    evas_textblock_cursor_free(an->start);
			    evas_textblock_cursor_free(an->end);
			    en->anchors = eina_list_remove(en->anchors, an);
			    free(an);
			 }
		       firsttext = 0;
		       an = NULL;
		    }
	       }
	  }
	else
	  {
	     s = evas_textblock_cursor_node_text_get(c1);
	     if (an)
	       {
		  if (!firsttext)
		    {
		       evas_textblock_cursor_copy(c1, an->start);
		       firsttext = 1;
		    }
		  evas_textblock_cursor_char_last(c1);
		  evas_textblock_cursor_copy(c1, an->end);
	       }
	  }
     } 
   while (evas_textblock_cursor_node_next(c1));
   evas_textblock_cursor_free(c1);
}


static void
_range_del(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   Evas_Textblock_Cursor *c1;
   
   c1 = evas_object_textblock_cursor_new(o);
   evas_textblock_cursor_node_last(c1);
   if (!evas_textblock_cursor_compare(en->sel_end, c1))
     evas_textblock_cursor_node_prev(en->sel_end);
   if (!evas_textblock_cursor_compare(en->sel_start, c1))
     evas_textblock_cursor_node_prev(en->sel_start);
   evas_textblock_cursor_free(c1);
   evas_textblock_cursor_range_delete(en->sel_start, en->sel_end);
}

static void
_backspace(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   Evas_Textblock_Cursor *c1, *c2;
   int nodel = 0;
   
   c1 = evas_object_textblock_cursor_new(o);
   if (!evas_textblock_cursor_char_prev(c))
     {
	if (!evas_textblock_cursor_node_prev(c))
	  nodel = 1;
	else
	  {
	     evas_textblock_cursor_copy(c, c1);
	     if (evas_textblock_cursor_node_format_get(c) &&
		 (!evas_textblock_cursor_node_format_is_visible_get(c)))
	       _curs_back(c, o, en);
	  }
     }
   else
     evas_textblock_cursor_copy(c, c1);
   c2 = evas_object_textblock_cursor_new(o);
   evas_textblock_cursor_copy(c, c2);
   if (!nodel) evas_textblock_cursor_range_delete(c1, c2);
   evas_textblock_cursor_copy(c, c1);
   _curs_back(c, o, en);
   if (evas_textblock_cursor_compare(c, c1))
     _curs_next(c, o, en);
   
   evas_textblock_cursor_free(c1);
   evas_textblock_cursor_free(c2);
}

static void
_edje_key_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Edje *ed = data;
   Evas_Event_Key_Down *ev = event_info;
   Edje_Real_Part *rp = ed->focused_part;
   Entry *en;
   Evas_Bool control, alt, shift;
   Evas_Bool multiline;
   Evas_Bool cursor_changed;
   Evas_Textblock_Cursor *tc;
   if (!rp) return;
   en = rp->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_EDITABLE))
     return;
   if (!ev->key) return;
   
   tc = evas_object_textblock_cursor_new(rp->object);
   evas_textblock_cursor_copy(en->cursor, tc);
   
   control = evas_key_modifier_is_set(ev->modifiers, "Control");
   alt = evas_key_modifier_is_set(ev->modifiers, "Alt");
   shift = evas_key_modifier_is_set(ev->modifiers, "Shift");
   multiline = rp->part->multiline;
   cursor_changed = 0;
   if (!strcmp(ev->key, "Escape"))
     {
	// dead keys here. Escape for now (should emit these)
        _edje_emit(ed, "entry,key,escape", rp->part->name);
	ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if (!strcmp(ev->key, "Up"))
     {
	if (multiline)
	  {
	     if (shift) _sel_start(en->cursor, rp->object, en);
	     else _sel_clear(en->cursor, rp->object, en);
	     _curs_up(en->cursor, rp->object, en);
	     if (shift) _sel_extend(en->cursor, rp->object, en);
	     ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
	  }
        _edje_emit(ed, "entry,key,up", rp->part->name);
     }
   else if (!strcmp(ev->key, "Down"))
     {
	if (multiline)
	  {
	     if (shift) _sel_start(en->cursor, rp->object, en);
	     else _sel_clear(en->cursor, rp->object, en);
	     _curs_down(en->cursor, rp->object, en);
	     if (shift) _sel_extend(en->cursor, rp->object, en);
	     ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
	  }
        _edje_emit(ed, "entry,key,down", rp->part->name);
     }
   else if (!strcmp(ev->key, "Left"))
     {
	if (shift) _sel_start(en->cursor, rp->object, en);
	else _sel_clear(en->cursor, rp->object, en);
	_curs_back(en->cursor, rp->object, en);
	if (shift) _sel_extend(en->cursor, rp->object, en);
        _edje_emit(ed, "entry,key,left", rp->part->name);
	ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if (!strcmp(ev->key, "Right"))
     {
	if (shift) _sel_start(en->cursor, rp->object, en);
	else _sel_clear(en->cursor, rp->object, en);
	_curs_next(en->cursor, rp->object, en);
	if (shift) _sel_extend(en->cursor, rp->object, en);
        _edje_emit(ed, "entry,key,right", rp->part->name);
	ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if (!strcmp(ev->key, "BackSpace"))
     {
	if (control)
	  {
	     // del to start of previous word
	  }
	else if ((alt) && (shift))
	  {
	     // undo last action
	  }
	else
	  {
	     if (en->have_selection)
	       _range_del(en->cursor, rp->object, en);
	     else
	       _backspace(en->cursor, rp->object, en);
	  }
	_sel_clear(en->cursor, rp->object, en);
	_curs_update_from_curs(en->cursor, rp->object, en);
        _edje_emit(ed, "entry,key,backspace", rp->part->name);
	ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if (!strcmp(ev->key, "Delete"))
     {
	if (control)
	  {
	     // del to end of next word
	  }
	else if (shift)
	  {
	     // cut
	  }
	else
	  {	
	     if (en->have_selection)
	       _range_del(en->cursor, rp->object, en);
	     else
	       {
		  Evas_Textblock_Cursor *c1;
		  
		  c1 = evas_object_textblock_cursor_new(rp->object);
		  evas_textblock_cursor_copy(en->cursor, c1);
		  _curs_next(en->cursor, rp->object, en);
		  if (evas_textblock_cursor_compare(en->cursor, c1))
		    _backspace(en->cursor, rp->object, en);
		  evas_textblock_cursor_free(c1);
	       }
	  }
	_sel_clear(en->cursor, rp->object, en);
	_curs_update_from_curs(en->cursor, rp->object, en);
	_anchors_get(en->cursor, rp->object, en);
	_edje_emit(ed, "entry,changed", rp->part->name);
        _edje_emit(ed, "entry,key,delete", rp->part->name);
	ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if (!strcmp(ev->key, "Home"))
     {
	if (shift) _sel_start(en->cursor, rp->object, en);
	else _sel_clear(en->cursor, rp->object, en);
	if ((control) && (multiline))
	  _curs_start(en->cursor, rp->object, en);
	else
	  _curs_lin_start(en->cursor, rp->object, en);
	if (shift) _sel_extend(en->cursor, rp->object, en);
        _edje_emit(ed, "entry,key,home", rp->part->name);
	ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if (!strcmp(ev->key, "End"))
     {
	if (shift) _sel_start(en->cursor, rp->object, en);
	else _sel_clear(en->cursor, rp->object, en);
	if ((control) && (multiline))
	  _curs_end(en->cursor, rp->object, en);
	else
	  _curs_lin_end(en->cursor, rp->object, en);
	if (shift) _sel_extend(en->cursor, rp->object, en);
        _edje_emit(ed, "entry,key,end", rp->part->name);
	ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if ((control) && (!strcmp(ev->key, "v")))
     {
	_edje_emit(ed, "entry,paste,request", rp->part->name);
	ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if ((control) && ((!strcmp(ev->key, "c") || (!strcmp(ev->key, "Insert")))))
     {
	// FIXME: copy - save selection
	_edje_emit(ed, "entry,copy,notify", rp->part->name);
	ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if ((control) && ((!strcmp(ev->key, "x") || (!strcmp(ev->key, "m")))))
     {
	// FIXME: cut - save selection, delete seletion
	_curs_update_from_curs(en->cursor, rp->object, en);
	_edje_emit(ed, "entry,cut,notify", rp->part->name);
	ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if ((control) && (!strcmp(ev->key, "z")))
     {
	if (shift)
	  {
	     // redo
	  }
	else
	  {
	     // undo
	  }
	ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if ((control) && (!strcmp(ev->key, "y")))
     {
	// redo
	ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if ((control) && (!strcmp(ev->key, "w")))
     {
	_sel_clear(en->cursor, rp->object, en);
	// select current word
	ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if (!strcmp(ev->key, "Tab"))
     {
	if (multiline)
	  {
	     if (shift)
	       {
		  // remove a tab
	       }
	     else
	       {
		  evas_textblock_cursor_format_prepend(en->cursor, "\t");
		  _curs_update_from_curs(en->cursor, rp->object, en);
		  _anchors_get(en->cursor, rp->object, en);
		  _edje_emit(ed, "entry,changed", rp->part->name);
	       }
	     ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
	  }
        _edje_emit(ed, "entry,key,tab", rp->part->name);
     }
   else if ((!strcmp(ev->key, "ISO_Left_Tab")) && (multiline))
     { 
	// remove a tab
	ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if (!strcmp(ev->key, "Prior"))
     {
	if (shift) _sel_start(en->cursor, rp->object, en);
	else _sel_clear(en->cursor, rp->object, en);
	_curs_jump_line_by(en->cursor, rp->object, en, -10);
	if (shift) _sel_extend(en->cursor, rp->object, en);
        _edje_emit(ed, "entry,key,pgup", rp->part->name);
	ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if (!strcmp(ev->key, "Next"))
     {
	if (shift) _sel_start(en->cursor, rp->object, en);
	else _sel_clear(en->cursor, rp->object, en);
	_curs_jump_line_by(en->cursor, rp->object, en, 10);
	if (shift) _sel_extend(en->cursor, rp->object, en);
        _edje_emit(ed, "entry,key,pgdn", rp->part->name);
	ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if ((!strcmp(ev->key, "Return")) || (!strcmp(ev->key, "KP_Enter")))
     {
	if (multiline)
	  {
	     evas_textblock_cursor_format_prepend(en->cursor, "\n");
	     _curs_update_from_curs(en->cursor, rp->object, en);
	     _anchors_get(en->cursor, rp->object, en);
	     _edje_emit(ed, "entry,changed", rp->part->name);
	     ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
	  }
        _edje_emit(ed, "entry,key,enter", rp->part->name);
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
	  {
            if (en->have_selection)
	       _range_del(en->cursor, rp->object, en);
	     evas_textblock_cursor_text_prepend(en->cursor, ev->string);
	     _sel_clear(en->cursor, rp->object, en);
	     _curs_update_from_curs(en->cursor, rp->object, en);
	     _anchors_get(en->cursor, rp->object, en);
	     _edje_emit(ed, "entry,changed", rp->part->name);
	     _edje_emit(ed, "cursor,changed", rp->part->name);
	     cursor_changed = 1;
	     ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
	  }
     }
   if ((evas_textblock_cursor_compare(tc, en->cursor)) && (!cursor_changed))
     _edje_emit(ed, "cursor,changed", rp->part->name);
   evas_textblock_cursor_free(tc);
   _edje_entry_real_part_configure(rp);
}

static void
_edje_key_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Edje *ed = data;
   Evas_Event_Key_Up *ev = event_info;
   Edje_Real_Part *rp = ed->focused_part;
   Entry *en;
   if (!rp) return;
   en = rp->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_EDITABLE))
     return;
}

static void
_edje_part_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Edje_Real_Part *rp = data;
   Evas_Event_Mouse_Down *ev = event_info;
   Entry *en;
   Evas_Coord x, y, w, h;
   Evas_Bool multiline;
   Evas_Textblock_Cursor *tc;
   if (ev->button != 1) return;
   if (!rp) return;
   en = rp->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_SELECTABLE))
     return;
   // double click -> select word
   // triple click -> select line
   tc = evas_object_textblock_cursor_new(rp->object);
   evas_textblock_cursor_copy(en->cursor, tc);
   multiline = rp->part->multiline;
   evas_object_geometry_get(rp->object, &x, &y, &w, &h);
   en->cx = ev->canvas.x - x;
   en->cy = ev->canvas.y - y;
   if (!evas_textblock_cursor_char_coord_set(en->cursor, en->cx, en->cy))
     {
	Evas_Coord lx, ly, lw, lh;
	
	evas_textblock_cursor_line_coord_set(en->cursor, en->cy);
	evas_textblock_cursor_line_geometry_get(en->cursor, &lx, &ly, &lw, &lh);
	if (en->cx <= lx)
	  _curs_lin_start(en->cursor, rp->object, en);
	else
	  _curs_lin_end(en->cursor, rp->object, en);
     }
   en->selecting = 1;
   _sel_clear(en->cursor, rp->object, en);
   _sel_start(en->cursor, rp->object, en);
   if (evas_textblock_cursor_compare(tc, en->cursor))
     _edje_emit(rp->edje, "cursor,changed", rp->part->name);
   evas_textblock_cursor_free(tc);
   _edje_entry_real_part_configure(rp);
}

static void
_edje_part_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Edje_Real_Part *rp = data;
   Evas_Event_Mouse_Up *ev = event_info;
   Entry *en;
   Evas_Coord x, y, w, h;
   Evas_Bool multiline;
   Evas_Textblock_Cursor *tc;
   if (ev->button != 1) return;
   if (!rp) return;
   en = rp->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_SELECTABLE))
     return;
   tc = evas_object_textblock_cursor_new(rp->object);
   evas_textblock_cursor_copy(en->cursor, tc);
   multiline = rp->part->multiline;
   evas_object_geometry_get(rp->object, &x, &y, &w, &h);
   en->cx = ev->canvas.x - x;
   en->cy = ev->canvas.y - y;
   if (!evas_textblock_cursor_char_coord_set(en->cursor, en->cx, en->cy))
     {
	Evas_Coord lx, ly, lw, lh;
	
	evas_textblock_cursor_line_coord_set(en->cursor, en->cy);
	evas_textblock_cursor_line_geometry_get(en->cursor, &lx, &ly, &lw, &lh);
	if (en->cx <= lx)
	  _curs_lin_start(en->cursor, rp->object, en);
	else
	  _curs_lin_end(en->cursor, rp->object, en);
     }
   evas_textblock_cursor_copy(en->cursor, en->sel_end);
   en->selecting = 0;
   if (evas_textblock_cursor_compare(tc, en->cursor))
     _edje_emit(rp->edje, "cursor,changed", rp->part->name);
   evas_textblock_cursor_free(tc);
   _edje_entry_real_part_configure(rp);
}

static void
_edje_part_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Edje_Real_Part *rp = data;
   Evas_Event_Mouse_Move *ev = event_info;
   Entry *en;
   Evas_Coord x, y, w, h;
   Evas_Textblock_Cursor *tc;
   if (!rp) return;
   Evas_Bool multiline;
   en = rp->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_SELECTABLE))
     return;
   tc = evas_object_textblock_cursor_new(rp->object);
   evas_textblock_cursor_copy(en->cursor, tc);
   multiline = rp->part->multiline;
   if (!en->selecting) return;
   evas_object_geometry_get(rp->object, &x, &y, &w, &h);
   en->cx = ev->cur.canvas.x - x;
   en->cy = ev->cur.canvas.y - y;
   if (!evas_textblock_cursor_char_coord_set(en->cursor, en->cx, en->cy))
     {
	Evas_Coord lx, ly, lw, lh;
	
	evas_textblock_cursor_line_coord_set(en->cursor, en->cy);
	evas_textblock_cursor_line_geometry_get(en->cursor, &lx, &ly, &lw, &lh);
	if (en->cx <= lx)
	  _curs_lin_start(en->cursor, rp->object, en);
	else
	  _curs_lin_end(en->cursor, rp->object, en);
     }
   _sel_extend(en->cursor, rp->object, en);
   if (evas_textblock_cursor_compare(en->sel_start, en->sel_end) != 0)
     _sel_enable(en->cursor, rp->object, en);
   if (en->have_selection)
     _sel_update(en->cursor, rp->object, en);
   if (evas_textblock_cursor_compare(tc, en->cursor))
     _edje_emit(rp->edje, "cursor,changed", rp->part->name);
   evas_textblock_cursor_free(tc);
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
   en->rp = rp;

   evas_object_event_callback_add(rp->object, EVAS_CALLBACK_MOUSE_DOWN, _edje_part_mouse_down_cb, rp);
   evas_object_event_callback_add(rp->object, EVAS_CALLBACK_MOUSE_UP, _edje_part_mouse_up_cb, rp);
   evas_object_event_callback_add(rp->object, EVAS_CALLBACK_MOUSE_MOVE, _edje_part_mouse_move_cb, rp);
   
   if (rp->part->entry_mode == EDJE_ENTRY_EDIT_MODE_PASSWORD)
     {
	if ((rp->chosen_description) &&
	    (rp->chosen_description->text.repch))
	  evas_object_textblock_replace_char_set(rp->object, rp->chosen_description->text.repch);
	else
	  evas_object_textblock_replace_char_set(rp->object, "*");
     }

   en->cursor_bg = edje_object_add(rp->edje->evas);
   edje_object_file_set(en->cursor_bg, rp->edje->path, rp->part->source3);
   evas_object_smart_member_add(en->cursor_bg, rp->edje->obj);
   evas_object_stack_below(en->cursor_bg, rp->object);
   evas_object_clip_set(en->cursor_bg, evas_object_clip_get(rp->object));
   evas_object_pass_events_set(en->cursor_bg, 1);
   
   en->cursor_fg = edje_object_add(rp->edje->evas);
   edje_object_file_set(en->cursor_fg, rp->edje->path, rp->part->source4);
   evas_object_smart_member_add(en->cursor_fg, rp->edje->obj);
   evas_object_stack_above(en->cursor_fg, rp->object);
   evas_object_clip_set(en->cursor_fg, evas_object_clip_get(rp->object));
   evas_object_pass_events_set(en->cursor_fg, 1);
   if (rp->part->entry_mode >= EDJE_ENTRY_EDIT_MODE_EDITABLE)
     {
	evas_object_show(en->cursor_bg);
	evas_object_show(en->cursor_fg);
     }
   
   en->cursor = evas_object_textblock_cursor_get(rp->object);
}

void
_edje_entry_real_part_shutdown(Edje_Real_Part *rp)
{
   Entry *en = rp->entry_data;
   if (!en) return;
   rp->entry_data = NULL;
   _sel_clear(en->cursor, rp->object, en);
   _anchors_clear(en->cursor, rp->object, en);
   evas_object_del(en->cursor_bg);
   evas_object_del(en->cursor_fg);
   free(en);
}

void
_edje_entry_real_part_configure(Edje_Real_Part *rp)
{
   Evas_Coord x, y, w, h, xx, yy, ww, hh;
   Entry *en = rp->entry_data;
   if (!en) return;

   _sel_update(en->cursor, rp->object, en);
   _anchors_update(en->cursor, rp->object, en);
   x = y = w = h = -1;
   xx = yy = ww = hh = -1;
   evas_object_geometry_get(rp->object, &x, &y, &w, &h);
   evas_textblock_cursor_char_geometry_get(en->cursor, &xx, &yy, &ww, &hh);
   if (ww < 1) ww = 1;
   if (hh < 1) ww = 1;
   if (en->cursor_bg)
     {
	evas_object_move(en->cursor_bg, x + xx, y + yy);
	evas_object_resize(en->cursor_bg, ww, hh);
     }
   if (en->cursor_fg)
     {
	evas_object_move(en->cursor_fg, x + xx, y + yy);
	evas_object_resize(en->cursor_fg, ww, hh);
     }
}

const char *
_edje_entry_selection_get(Edje_Real_Part *rp)
{
   Entry *en = rp->entry_data;
   if (!en) return NULL;
   // get selection - convert to markup
   if ((!en->selection) && (en->have_selection))
     en->selection = evas_textblock_cursor_range_text_get
     (en->sel_start, en->sel_end, EVAS_TEXTBLOCK_TEXT_MARKUP);
   return en->selection;
}

const char *
_edje_entry_text_get(Edje_Real_Part *rp)
{
   Entry *en = rp->entry_data;
   if (!en) return NULL;
   // get text - convert to markup
   return evas_object_textblock_text_markup_get(rp->object);
}

void
_edje_entry_text_markup_set(Edje_Real_Part *rp, const char *text)
{
   Entry *en = rp->entry_data;
   if (!en) return;
   // set text as markup
   _sel_clear(en->cursor, rp->object, en);
   evas_object_textblock_text_markup_set(rp->object, text); 
   evas_textblock_cursor_node_last(en->cursor);
   if (!evas_textblock_cursor_node_format_get(en->cursor))
     {
	evas_textblock_cursor_format_append(en->cursor, "\n");
     }
   else if (!((!strcmp(evas_textblock_cursor_node_format_get(en->cursor), "\n")) ||
	      (!strcmp(evas_textblock_cursor_node_format_get(en->cursor), "\\n"))))
     {
	evas_textblock_cursor_format_append(en->cursor, "\n");
     }
   _anchors_get(en->cursor, rp->object, en);
   _edje_emit(rp->edje, "entry,changed", rp->part->name);
   _edje_entry_set_cursor_start(rp);
}

void
_edje_entry_text_markup_insert(Edje_Real_Part *rp, const char *text)
{
   Entry *en = rp->entry_data;
   if (!en) return;
   // prepend markup @ cursor pos
   if (en->have_selection)
     evas_textblock_cursor_range_delete(en->sel_start, en->sel_end);
   evas_object_textblock_text_markup_prepend(en->cursor, text);
   _sel_clear(en->cursor, rp->object, en);
   _curs_update_from_curs(en->cursor, rp->object, en);
   _anchors_get(en->cursor, rp->object, en);
   _edje_emit(rp->edje, "entry,changed", rp->part->name);
}

void
_edje_entry_set_cursor_start(Edje_Real_Part *rp)
{
   Entry *en = rp->entry_data;
   if (!en) return;
   _curs_start(en->cursor, rp->object, en);
}

void
_edje_entry_set_cursor_end(Edje_Real_Part *rp)
{
   Entry *en = rp->entry_data;
   if (!en) return;
   _curs_end(en->cursor, rp->object, en);
}

const Eina_List *
_edje_entry_anchor_geometry_get(Edje_Real_Part *rp, const char *anchor)
{
   Entry *en = rp->entry_data;
   Eina_List *l;
   Anchor *an;
   
   if (!en) return NULL;
   EINA_LIST_FOREACH(en->anchors, l, an)
     {
	if (!strcmp(anchor, an->name))
	  return an->sel;
     }
   return NULL;
}

const Eina_List *
_edje_entry_anchors_list(Edje_Real_Part *rp)
{
   Entry *en = rp->entry_data;
   Eina_List *l, *anchors = NULL;
   Anchor *an;

   if (!en) return NULL;
   if (!en->anchorlist)
     {
        EINA_LIST_FOREACH(en->anchors, l, an)
	  {
	     const char *n = an->name;
	     if (!n) n = "";
	     anchors = eina_list_append(anchors, strdup(n));
	  }
	en->anchorlist = anchors;
     }
   return en->anchorlist;
}

void
_edje_entry_cursor_geometry_get(Edje_Real_Part *rp, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   Evas_Coord x, y, w, h, xx, yy, ww, hh;
   Entry *en = rp->entry_data;
   if (!en) return;

   x = y = w = h = -1;
   xx = yy = ww = hh = -1;
   evas_object_geometry_get(rp->object, &x, &y, &w, &h);
   evas_textblock_cursor_char_geometry_get(en->cursor, &xx, &yy, &ww, &hh);
   if (ww < 1) ww = 1;
   if (hh < 1) ww = 1;
   if (cx) *cx = x + xx;
   if (cy) *cy = y + yy;
   if (cw) *cw = ww;
   if (ch) *ch = hh;
}

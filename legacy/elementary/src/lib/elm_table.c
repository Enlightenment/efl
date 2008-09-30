#include <Elementary.h>
#include "elm_priv.h"

static void _elm_table_layout_update(Elm_Table *tb);
static void _elm_table_pack(Elm_Table *tb, Elm_Widget *wid, int x, int y, int w, int h);

Elm_Table_Class _elm_table_class =
{
   &_elm_widget_class,
     ELM_OBJ_TABLE,
     _elm_table_layout_update,
     _elm_table_pack,
};

static void
_elm_table_layout_update(Elm_Table *tb)
{
   _els_smart_table_homogenous_set(tb->base, tb->homogenous);
}

static void
_elm_table_pack(Elm_Table *tb, Elm_Widget *wid, int x, int y, int w, int h)
{
   tb->child_add(tb, wid);
   _els_smart_table_pack(tb->base, wid->base, x, y, w, h);
}

static void
_elm_table_size_alloc(Elm_Table *tb, int w, int h)
{
   Evas_Coord mw, mh;
   
   _els_smart_table_min_size_get(tb->base, &mw, &mh);
   if (w < mw) w = mw;
   if (h < mh) h = mh;
   tb->req.w = w;
   tb->req.h = h;
}
    
static void
_elm_table_size_req(Elm_Table *tb, Elm_Widget *child, int w, int h)
{
   Evas_Coord mw, mh;
   
   if (child)
     {
	Evas_Coord maxx, maxy;
	
	child->size_alloc(child, 0, 0);
	maxx = child->req.w;
	maxy = child->req.h;
	if (child->expand_x) maxx = 32767;
	if (child->expand_y) maxy = 32767;
	_els_smart_table_pack_options_set(child->base,
					  child->fill_x, child->fill_y,
					  child->expand_x, child->expand_y,
					  child->align_x, child->align_y,
					  child->req.w, child->req.h,
					  maxx, maxy);
     }
   else
     {
	// FIXME: handle.
     }
   _els_smart_table_min_size_get(tb->base, &mw, &mh);
   ((Elm_Widget *)(tb->parent))->size_req(tb->parent, tb, mw, mh);
   tb->geom_set(tb, tb->x, tb->y, mw, mh);
}

static void
_elm_on_child_del(void *data, Elm_Table *tb, Elm_Cb_Type type, Elm_Obj *obj)
{
   Evas_Coord mw, mh;
   
   if (!(obj->hastype(obj, ELM_OBJ_WIDGET))) return;
   _els_smart_table_unpack(((Elm_Widget *)(obj))->base);
   ((Elm_Widget *)(tb->parent))->size_req(tb->parent, tb, mw, mh);
   tb->geom_set(tb, tb->x, tb->y, mw, mh);
}

EAPI Elm_Table *
elm_table_new(Elm_Win *win)
{
   Elm_Table *tb;
   
   tb = ELM_NEW(Elm_Table);
   
   _elm_widget_init(tb);
   tb->clas = &_elm_table_class;
   tb->type = ELM_OBJ_TABLE;
   
   tb->layout_update = _elm_table_layout_update;
   tb->pack = _elm_table_pack;

   tb->size_alloc = _elm_table_size_alloc;
   tb->size_req = _elm_table_size_req;
   
   tb->base = _els_smart_table_add(win->evas);
   
   tb->cb_add(tb, ELM_CB_CHILD_DEL, _elm_on_child_del, NULL);
   _elm_widget_post_init(tb);
   win->child_add(win, tb);
   return tb;
}

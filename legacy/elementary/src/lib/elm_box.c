#include <Elementary.h>
#include "elm_priv.h"

static void _elm_box_layout_update(Elm_Box *bx);
static void _elm_box_pack_start(Elm_Box *bx, Elm_Widget *wid);
static void _elm_box_pack_end(Elm_Box *bx, Elm_Widget *wid);
static void _elm_box_pack_before(Elm_Box *bx, Elm_Widget *wid, Elm_Widget *wid_before);
static void _elm_box_pack_after(Elm_Box *bx, Elm_Widget *wid, Elm_Widget *wid_after);

Elm_Box_Class _elm_box_class =
{
   &_elm_widget_class,
     ELM_OBJ_SCROLLER,
     _elm_box_layout_update,
     _elm_box_pack_start,
     _elm_box_pack_end,
     _elm_box_pack_before,
     _elm_box_pack_after
};

static void
_elm_box_layout_update(Elm_Box *bx)
{
   _els_smart_box_orientation_set(bx->base, bx->horizontal);
   _els_smart_box_homogenous_set(bx->base, bx->homogenous);
}

static void
_elm_box_pack_start(Elm_Box *bx, Elm_Widget *wid)
{
   bx->child_add(bx, wid);
   _els_smart_box_pack_start(bx->base, wid->base);
   elm_widget_sizing_update(wid);
}

static void
_elm_box_pack_end(Elm_Box *bx, Elm_Widget *wid)
{
   bx->child_add(bx, wid);
   _els_smart_box_pack_end(bx->base, wid->base);
   elm_widget_sizing_update(wid);
}

static void
_elm_box_pack_before(Elm_Box *bx, Elm_Widget *wid, Elm_Widget *wid_before)
{
   bx->child_add(bx, wid);
   _els_smart_box_pack_before(bx->base, wid->base, wid_before->base);
   elm_widget_sizing_update(wid);
}

static void
_elm_box_pack_after(Elm_Box *bx, Elm_Widget *wid, Elm_Widget *wid_after)
{
   bx->child_add(bx, wid);
   _els_smart_box_pack_after(bx->base, wid->base, wid_after->base);
   elm_widget_sizing_update(wid);
}

static void
_elm_box_size_alloc(Elm_Box *bx, int w, int h)
{
   Evas_Coord mw, mh;
   
   _els_smart_box_min_size_get(bx->base, &mw, &mh);
   if (w < mw) w = mw;
   if (h < mh) h = mh;
   printf("box %p size alloc to %ix%i\n", bx, w, h);
   bx->req.w = w;
   bx->req.h = h;
}
    
static void
_elm_box_size_req(Elm_Box *bx, Elm_Widget *child, int w, int h)
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
	_els_smart_box_pack_options_set(child->base,
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
   _els_smart_box_min_size_get(bx->base, &mw, &mh);
   ((Elm_Widget *)(bx->parent))->size_req(bx->parent, bx, mw, mh);
   bx->geom_set(bx, bx->x, bx->y, mw, mh);
}

static void
_elm_on_child_del(void *data, Elm_Box *bx, Elm_Cb_Type type, Elm_Obj *obj)
{
   if (!(obj->hastype(obj, ELM_OBJ_WIDGET))) return;
   _els_smart_box_unpack(((Elm_Widget *)(obj))->base);
}

EAPI Elm_Box *
elm_box_new(Elm_Win *win)
{
   Elm_Box *bx;
   
   bx = ELM_NEW(Elm_Box);
   _elm_widget_init(bx);
   
   bx->clas = &_elm_box_class;
   bx->type = ELM_OBJ_BOX;
   
   bx->layout_update = _elm_box_layout_update;
   bx->pack_start = _elm_box_pack_start;
   bx->pack_end = _elm_box_pack_end;
   bx->pack_before = _elm_box_pack_before;
   bx->pack_after = _elm_box_pack_after;

   bx->size_alloc = _elm_box_size_alloc;
   bx->size_req = _elm_box_size_req;
   
   bx->base = _els_smart_box_add(win->evas);
   
   bx->cb_add(bx, ELM_CB_CHILD_DEL, _elm_on_child_del, NULL);
   _elm_widget_post_init(bx);
   win->child_add(win, bx);
   return bx;
}

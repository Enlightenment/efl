#include <Elementary.h>
#include "elm_priv.h"

static void _elm_widget_geom_set(Elm_Widget *wid, int x, int y, int w, int h);
static void _elm_widget_show(Elm_Widget *wid);
static void _elm_widget_hide(Elm_Widget *wid);
static void _elm_widget_size_alloc(Elm_Widget *wid, int w, int h);
static void _elm_widget_size_req(Elm_Widget *wid, Elm_Widget *child, int w, int h);
static void _elm_widget_del(Elm_Widget *wid);
static void _elm_widget_above(Elm_Widget *wid, Elm_Widget *above);
static void _elm_widget_below(Elm_Widget *wid, Elm_Widget *below);
    
Elm_Widget_Class _elm_widget_class =
{
   &_elm_obj_class, /* parent */
     ELM_OBJ_WIDGET,
     _elm_widget_geom_set,
     _elm_widget_show,
     _elm_widget_hide,
     _elm_widget_size_alloc,
     _elm_widget_size_req,
     _elm_widget_above,
     _elm_widget_below
};

static void
_elm_widget_geom_set(Elm_Widget *wid, int x, int y, int w, int h)
{
   if ((wid->x != x) || (wid->y != y))
     {
	wid->x = x;
	wid->y = y; 
	evas_object_move(wid->base, wid->x, wid->y);
     }
   if ((wid->w != w) || (wid->h != h))
     {
	wid->w = w;
	wid->h = h;
	evas_object_resize(wid->base, wid->w, wid->h);
	_elm_obj_nest_push();
	_elm_cb_call(ELM_OBJ(wid), ELM_CB_RESIZE, NULL);
	_elm_obj_nest_pop();
     }
}

static void
_elm_widget_show(Elm_Widget *wid)
{
   evas_object_show(wid->base);
}

static void
_elm_widget_hide(Elm_Widget *wid)
{
   evas_object_hide(wid->base);
}

static void
_elm_widget_size_alloc(Elm_Widget *wid, int w, int h)
{
   // FIXME: widget gets to implement min/max etc. size here (indicating
   // given the input w, h - what size it is willing to accept, then the
   // parent checks and geometry_set()'s the child
   wid->req.w = w;
   wid->req.h = h;
}

static void
_elm_widget_size_req(Elm_Widget *wid, Elm_Widget *child, int w, int h)
{
   // FIXME: a child will ask its parent for a requested size if something
   // with its sizing setup. this is the call in the parent that will be
   // called
}

static void
_elm_widget_above(Elm_Widget *wid, Elm_Widget *above)
{
   if (above) evas_object_stack_above(wid->base, above->base);
   else evas_object_raise(wid->base);
}

static void
_elm_widget_below(Elm_Widget *wid, Elm_Widget *below)
{
   if (below) evas_object_stack_below(wid->base, below->base);
   else evas_object_lower(wid->base);
}

static void
_elm_widget_del(Elm_Widget *wid)
{    
   if (_elm_obj_del_defer(ELM_OBJ(wid))) return;
   
   evas_object_del(wid->base);
   
   ((Elm_Obj_Class *)(((Elm_Widget_Class *)(wid->clas))->parent))->del(ELM_OBJ(wid));
}

void
_elm_widget_init(Elm_Widget *wid)
{
   _elm_obj_init(ELM_OBJ(wid));
   wid->clas = &_elm_widget_class;
   wid->type = ELM_OBJ_WIDGET;

   wid->del = _elm_widget_del;
   
   wid->geom_set = _elm_widget_geom_set;
   wid->show = _elm_widget_show;
   wid->hide = _elm_widget_hide;
   wid->size_alloc = _elm_widget_size_alloc;
   wid->size_req = _elm_widget_size_req;
   wid->above = _elm_widget_above;
   wid->below = _elm_widget_below;

   wid->align_x = 0.5;
   wid->align_y = 0.5;
   wid->expand_x = 1;
   wid->expand_y = 1;
   wid->fill_x = 1;
   wid->fill_y = 1;
}

void
_elm_widget_post_init(Elm_Widget *wid)
{
   if (wid->base) evas_object_data_set(wid->base, "__Elm", wid);
}

EAPI void
elm_widget_sizing_update(Elm_Widget *wid)
{
   wid->size_alloc(wid, 0, 0);
   if (!wid->parent) return;
   ((Elm_Widget *)(wid->parent))->size_req(wid->parent, wid, wid->w, wid->h);
}

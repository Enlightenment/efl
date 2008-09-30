#include <Elementary.h>
#include "elm_priv.h"

Elm_Pad_Class _elm_pad_class =
{
   &_elm_widget_class,
     ELM_OBJ_PAD
};

static void
_elm_pad_size_alloc(Elm_Pad *pd, int w, int h)
{
   if (w < pd->minw) w = pd->minw;
   if (h < pd->minh) h = pd->minh;
   pd->req.w = w;
   pd->req.h = h;
}

static void
_elm_pad_size_req(Elm_Pad *pd, Elm_Widget *child, int w, int h)
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
	edje_extern_object_min_size_set(child->base,
					child->req.w,
					child->req.h);
        edje_object_part_swallow(pd->base , "elm.swallow.contents",
				 child->base);
	edje_object_size_min_calc(pd->base, &mw, &mh);
	if ((pd->minw != mw) || (pd->minh != mh))
	  {
	     pd->minw = mw;
	     pd->minh = mh;
	     ((Elm_Widget *)(pd->parent))->size_req(pd->parent, pd, mw, mh);
	     pd->geom_set(pd, pd->x, pd->y, mw, mh);
	  }
     }
   else
     {
	// FIXME: handle
     }
}

static void
_elm_on_child_add(void *data, Elm_Pad *pd, Elm_Cb_Type type, Elm_Obj *obj)
{
   Evas_Coord mw, mh;
   
   if (!(obj->hastype(obj, ELM_OBJ_WIDGET))) return;
   ((Elm_Widget *)(obj))->size_alloc(obj, 0, 0);
   ((Elm_Widget *)(obj))->geom_set(obj,
				   ((Elm_Widget *)(obj))->x,
				   ((Elm_Widget *)(obj))->y,
				   ((Elm_Widget *)(obj))->req.w,
				   ((Elm_Widget *)(obj))->req.h);
   edje_extern_object_min_size_set(((Elm_Widget *)(obj))->base,
				   ((Elm_Widget *)(obj))->req.w,
				   ((Elm_Widget *)(obj))->req.h);
   edje_object_part_swallow(pd->base , "elm.swallow.content",
			    ((Elm_Widget *)(obj))->base);
   edje_object_size_min_calc(pd->base, &mw, &mh);
   if ((pd->minw != mw) || (pd->minh != mh))
     {
	pd->minw = mw;
	pd->minh = mh;
	((Elm_Widget *)(pd->parent))->size_req(pd->parent, pd, mw, mh);
        pd->geom_set(pd, pd->x, pd->y, mw, mh);
     }
}

static void
_elm_on_child_del(void *data, Elm_Pad *pd, Elm_Cb_Type type, Elm_Obj *obj)
{
   if (!(obj->hastype(obj, ELM_OBJ_WIDGET))) return;
   // FIXME: allow for removal of child - size down
}

static void
_elm_pad_del(Elm_Pad *pd)
{
   ((Elm_Obj_Class *)(((Elm_Pad_Class *)(pd->clas))->parent))->del(ELM_OBJ(pd));
}

EAPI Elm_Pad *
elm_pad_new(Elm_Win *win)
{
   Elm_Pad *pd;
   
   pd = ELM_NEW(Elm_Pad);
   
   _elm_widget_init(pd);
   pd->clas = &_elm_pad_class;
   pd->type = ELM_OBJ_PAD;
   
   pd->del = _elm_pad_del;
   
   pd->size_alloc = _elm_pad_size_alloc;
   pd->size_req = _elm_pad_size_req;
   
   pd->base = edje_object_add(win->evas);
   _elm_theme_set(pd->base, "pad", "pad");
   pd->cb_add(pd, ELM_CB_CHILD_ADD, _elm_on_child_add, NULL);
   pd->cb_add(pd, ELM_CB_CHILD_DEL, _elm_on_child_del, NULL);
   _elm_widget_post_init(pd);
   win->child_add(win, pd);
   return pd;
}

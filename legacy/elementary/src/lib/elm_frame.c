#include <Elementary.h>
#include "elm_priv.h"

static void _elm_frame_text_set(Elm_Frame *fr, const char *text);

Elm_Frame_Class _elm_frame_class =
{
   &_elm_widget_class,
     ELM_OBJ_FRAME,
     _elm_frame_text_set
};

static void
_elm_frame_text_set(Elm_Frame *fr, const char *text)
{
   Evas_Coord mw, mh;

   edje_object_part_text_set(fr->base, "elm.text", text);
   edje_object_size_min_calc(fr->base, &mw, &mh);
   if ((fr->minw != mw) || (fr->minh != mh))
     {
	fr->minw = mw;
	fr->minh = mh;
	((Elm_Widget *)(fr->parent))->size_req(fr->parent, fr, fr->minw, fr->minh);
	fr->geom_set(fr, fr->x, fr->y, fr->minw, fr->minh);
     }
}

static void
_elm_frame_size_alloc(Elm_Frame *fr, int w, int h)
{
   if (w < fr->minw) w = fr->minw;
   if (h < fr->minh) h = fr->minh;
   fr->req.w = w;
   fr->req.h = h;
}

static void
_elm_frame_size_req(Elm_Frame *fr, Elm_Widget *child, int w, int h)
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
        edje_object_part_swallow(fr->base , "elm.swallow.content",
				 child->base);
	edje_object_size_min_calc(fr->base, &mw, &mh);
	if ((fr->minw != mw) || (fr->minh != mh))
	  {
	     fr->minw = mw;
	     fr->minh = mh;
	     ((Elm_Widget *)(fr->parent))->size_req(fr->parent, fr, mw, mh);
	     fr->geom_set(fr, fr->x, fr->y, mw, mh);
	  }
     }
   else
     {
	// FIXME: handle
     }
}

static void
_elm_on_child_add(void *data, Elm_Frame *fr, Elm_Cb_Type type, Elm_Obj *obj)
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
   edje_object_part_swallow(fr->base , "elm.swallow.content",
			    ((Elm_Widget *)(obj))->base);
   edje_object_size_min_calc(fr->base, &mw, &mh);
   if ((fr->minw != mw) || (fr->minh != mh))
     {
	fr->minw = mw;
	fr->minh = mh;
	((Elm_Widget *)(fr->parent))->size_req(fr->parent, fr, mw, mh);
        fr->geom_set(fr, fr->x, fr->y, mw, mh);
     }
}

static void
_elm_on_child_del(void *data, Elm_Frame *fr, Elm_Cb_Type type, Elm_Obj *obj)
{
   if (!(obj->hastype(obj, ELM_OBJ_WIDGET))) return;
   // FIXME: allow for removal of child - size down
}

static void
_elm_frame_del(Elm_Frame *fr)
{
   if (fr->text) evas_stringshare_del(fr->text);
   ((Elm_Obj_Class *)(((Elm_Frame_Class *)(fr->clas))->parent))->del(ELM_OBJ(fr));
}

EAPI Elm_Frame *
elm_frame_new(Elm_Win *win)
{
   Elm_Frame *fr;
   
   fr = ELM_NEW(Elm_Frame);
   
   _elm_widget_init(fr);
   fr->clas = &_elm_frame_class;
   fr->type = ELM_OBJ_FRAME;
   
   fr->del = _elm_frame_del;
   
   fr->size_alloc = _elm_frame_size_alloc;
   fr->size_req = _elm_frame_size_req;
   
   fr->text_set = _elm_frame_text_set;
   
   fr->base = edje_object_add(win->evas);
   _elm_theme_set(fr->base, "frame", "frame");
   fr->cb_add(fr, ELM_CB_CHILD_ADD, _elm_on_child_add, NULL);
   fr->cb_add(fr, ELM_CB_CHILD_DEL, _elm_on_child_del, NULL);
   _elm_widget_post_init(fr);
   win->child_add(win, fr);
   return fr;
}

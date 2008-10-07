#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *frm;
};

static void _del_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   free(wd);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   
   edje_object_size_min_calc(wd->frm, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   edje_object_part_swallow(wd->frm, "elm.swallow.content", obj);
   _sizing_eval(data);
}

EAPI Evas_Object *
elm_frame_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   
   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   
   wd->frm = edje_object_add(e);
   _elm_theme_set(wd->frm, "frame", "frame");
   elm_widget_resize_object_set(obj, wd->frm);
   
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_frame_label_set(Evas_Object *obj, const char *label)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   edje_object_part_text_set(wd->frm, "elm.text", label);
   _sizing_eval(obj);
}

EAPI void
elm_frame_content_set(Evas_Object *obj, Evas_Object *content)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   edje_object_part_swallow(wd->frm, "elm.swallow.content", content);
   elm_widget_sub_object_add(obj, content);
   evas_object_event_callback_add(content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				  _changed_size_hints, obj);
   // FIXME: track new sub obj...
   _sizing_eval(obj);
}

/*
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
*/

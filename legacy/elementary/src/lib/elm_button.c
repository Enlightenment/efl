#include <Elementary.h>
#include "elm_priv.h"

static void _elm_button_text_set(Elm_Button *bt, const char *text);

Elm_Button_Class _elm_button_class =
{
   &_elm_widget_class,
     ELM_OBJ_BUTTON,
     _elm_button_text_set
};

static void
_elm_button_text_set(Elm_Button *bt, const char *text)
{
   Evas_Coord mw, mh;

   if (text)
     {
	edje_object_signal_emit(bt->base, "elm,state,text,visible", "elm");
	edje_object_message_signal_process(bt->base);
     }
   else
     {
	edje_object_signal_emit(bt->base, "elm,state,text,hidden", "elm");
	edje_object_message_signal_process(bt->base);
     }
   edje_object_part_text_set(bt->base, "elm.text", text);
   edje_object_size_min_calc(bt->base, &mw, &mh);
   if ((bt->minw != mw) || (bt->minh != mh))
     {
	bt->minw = mw;
	bt->minh = mh;
	((Elm_Widget *)(bt->parent))->size_req(bt->parent, bt, bt->minw, bt->minh);
	bt->geom_set(bt, bt->x, bt->y, bt->minw, bt->minh);
     }
}

static void
_elm_button_size_alloc(Elm_Button *bt, int w, int h)
{
   if (w < bt->minw) w = bt->minw;
   if (h < bt->minh) h = bt->minh;
   bt->req.w = w;
   bt->req.h = h;
}

static void
_elm_button_size_req(Elm_Button *bt, Elm_Widget *child, int w, int h)
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
        edje_object_part_swallow(bt->base , "elm.swallow.contents",
				 child->base);
	edje_object_size_min_calc(bt->base, &mw, &mh);
	if ((bt->minw != mw) || (bt->minh != mh))
	  {
	     bt->minw = mw;
	     bt->minh = mh;
	     ((Elm_Widget *)(bt->parent))->size_req(bt->parent, bt, mw, mh);
	     bt->geom_set(bt, bt->x, bt->y, mw, mh);
	  }
     }
   else
     {
	// FIXME: handle
     }
}

static void
_elm_on_child_add(void *data, Elm_Button *bt, Elm_Cb_Type type, Elm_Obj *obj)
{
   Evas_Coord mw, mh;
   
   if (!(obj->hastype(obj, ELM_OBJ_WIDGET))) return;
   edje_object_signal_emit(bt->base, "elm,state,icon,visible", "elm");
   edje_object_message_signal_process(bt->base);
   ((Elm_Widget *)(obj))->size_alloc(obj, 0, 0);
   ((Elm_Widget *)(obj))->geom_set(obj,
				   ((Elm_Widget *)(obj))->x,
				   ((Elm_Widget *)(obj))->y,
				   ((Elm_Widget *)(obj))->req.w,
				   ((Elm_Widget *)(obj))->req.h);
   edje_extern_object_min_size_set(((Elm_Widget *)(obj))->base,
				   ((Elm_Widget *)(obj))->req.w,
				   ((Elm_Widget *)(obj))->req.h);
   edje_object_part_swallow(bt->base , "elm.swallow.contents",
			    ((Elm_Widget *)(obj))->base);
   edje_object_size_min_calc(bt->base, &mw, &mh);
   if ((bt->minw != mw) || (bt->minh != mh))
     {
	bt->minw = mw;
	bt->minh = mh;
	((Elm_Widget *)(bt->parent))->size_req(bt->parent, bt, mw, mh);
        bt->geom_set(bt, bt->x, bt->y, mw, mh);
     }
}

static void
_elm_on_child_del(void *data, Elm_Button *bt, Elm_Cb_Type type, Elm_Obj *obj)
{
   if (!(obj->hastype(obj, ELM_OBJ_WIDGET))) return;
   // FIXME: allow for removal of child - size down
   edje_object_signal_emit(bt->base, "elm,state,icon,hidden", "elm");
   edje_object_message_signal_process(bt->base);
}

static void
_elm_button_activate(Elm_Button *bt)
{
   _elm_obj_nest_push();
   _elm_cb_call(ELM_OBJ(bt), ELM_CB_ACTIVATED, NULL);
   _elm_obj_nest_pop();
}

static void
_elm_signal_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Elm_Button *bt = data;
   
   _elm_button_activate(bt);
}

static void
_elm_button_del(Elm_Button *bt)
{
   if (bt->text) evas_stringshare_del(bt->text);
   ((Elm_Obj_Class *)(((Elm_Button_Class *)(bt->clas))->parent))->del(ELM_OBJ(bt));
}

EAPI Elm_Button *
elm_button_new(Elm_Win *win)
{
   Elm_Button *bt;
   
   bt = ELM_NEW(Elm_Button);
   
   _elm_widget_init(bt);
   bt->clas = &_elm_button_class;
   bt->type = ELM_OBJ_BUTTON;
   
   bt->del = _elm_button_del;
   
   bt->size_alloc = _elm_button_size_alloc;
   bt->size_req = _elm_button_size_req;
   
   bt->text_set = _elm_button_text_set;
   
   bt->base = edje_object_add(win->evas);
   _elm_theme_set(bt->base, "button", "button");
   edje_object_signal_callback_add(bt->base, "elm,action,click", "",
				   _elm_signal_clicked, bt);
   bt->cb_add(bt, ELM_CB_CHILD_ADD, _elm_on_child_add, NULL);
   bt->cb_add(bt, ELM_CB_CHILD_DEL, _elm_on_child_del, NULL);
   _elm_widget_post_init(bt);
   win->child_add(win, bt);
   return bt;
}

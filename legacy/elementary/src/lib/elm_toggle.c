#include <Elementary.h>
#include "elm_priv.h"

static void _elm_toggle_text_set(Elm_Toggle *tg, const char *text);
static void _elm_toggle_layout_update(Elm_Toggle *tg);
static void _elm_toggle_states_text_set(Elm_Toggle *tg, const char *ontext, const char *offtext);

Elm_Toggle_Class _elm_toggle_class =
{
   &_elm_widget_class,
     ELM_OBJ_TOGGLE,
     _elm_toggle_text_set,
     _elm_toggle_layout_update,
     _elm_toggle_states_text_set
};

static void
_elm_toggle_text_set(Elm_Toggle *tg, const char *text)
{
   Evas_Coord mw, mh;

   if (text)
     {
	edje_object_signal_emit(tg->base, "elm,state,text,visible", "elm");
	edje_object_message_signal_process(tg->base);
     }
   else
     {
	edje_object_signal_emit(tg->base, "elm,state,text,hidden", "elm");
	edje_object_message_signal_process(tg->base);
     }
   edje_object_part_text_set(tg->base, "elm.text", text);
   edje_object_size_min_calc(tg->base, &mw, &mh);
   if ((tg->minw != mw) || (tg->minh != mh))
     {
	tg->minw = mw;
	tg->minh = mh;
	((Elm_Widget *)(tg->parent))->size_req(tg->parent, tg, tg->minw, tg->minh);
	tg->geom_set(tg, tg->x, tg->y, tg->minw, tg->minh);
     }
}

static void
_elm_toggle_states_text_set(Elm_Toggle *tg, const char *ontext, const char *offtext)
{
   edje_object_part_text_set(tg->base, "elm.ontext", ontext);
   edje_object_part_text_set(tg->base, "elm.offtext", offtext);
}

static void
_elm_toggle_layout_update(Elm_Toggle *tg)
{
   if (tg->state_ptr) tg->state = *(tg->state_ptr);
   if (tg->state)
     edje_object_signal_emit(tg->base, "elm,state,toggle,on", "elm");
   else
     edje_object_signal_emit(tg->base, "elm,state,toggle,off", "elm");
}

static void
_elm_toggle_size_alloc(Elm_Toggle *tg, int w, int h)
{
   if (w < tg->minw) w = tg->minw;
   if (h < tg->minh) h = tg->minh;
   tg->req.w = w;
   tg->req.h = h;
}

static void
_elm_toggle_size_req(Elm_Toggle *tg, Elm_Widget *child, int w, int h)
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
        edje_object_part_swallow(tg->base , "elm.swallow.contents",
				 child->base);
	edje_object_size_min_calc(tg->base, &mw, &mh);
	if ((tg->minw != mw) || (tg->minh != mh))
	  {
	     tg->minw = mw;
	     tg->minh = mh;
	     ((Elm_Widget *)(tg->parent))->size_req(tg->parent, tg, mw, mh);
	     tg->geom_set(tg, tg->x, tg->y, mw, mh);
	  }
     }
   else
     {
	// FIXME: handle
     }
}

static void
_elm_on_child_add(void *data, Elm_Toggle *tg, Elm_Cb_Type type, Elm_Obj *obj)
{
   Evas_Coord mw, mh;
   
   if (!(obj->hastype(obj, ELM_OBJ_WIDGET))) return;
   edje_object_signal_emit(tg->base, "elm,state,icon,visible", "elm");
   edje_object_message_signal_process(tg->base);
   ((Elm_Widget *)(obj))->size_alloc(obj, 0, 0);
   ((Elm_Widget *)(obj))->geom_set(obj,
				   ((Elm_Widget *)(obj))->x,
				   ((Elm_Widget *)(obj))->y,
				   ((Elm_Widget *)(obj))->req.w,
				   ((Elm_Widget *)(obj))->req.h);
   edje_extern_object_min_size_set(((Elm_Widget *)(obj))->base,
				   ((Elm_Widget *)(obj))->req.w,
				   ((Elm_Widget *)(obj))->req.h);
   edje_object_part_swallow(tg->base , "elm.swallow.contents",
			    ((Elm_Widget *)(obj))->base);
   edje_object_size_min_calc(tg->base, &mw, &mh);
   if ((tg->minw != mw) || (tg->minh != mh))
     {
	tg->minw = mw;
	tg->minh = mh;
	((Elm_Widget *)(tg->parent))->size_req(tg->parent, tg, mw, mh);
        tg->geom_set(tg, tg->x, tg->y, mw, mh);
     }
}

static void
_elm_on_child_del(void *data, Elm_Toggle *tg, Elm_Cb_Type type, Elm_Obj *obj)
{
   if (!(obj->hastype(obj, ELM_OBJ_WIDGET))) return;
   // FIXME: allow for removal of child - size down
   edje_object_signal_emit(tg->base, "elm,state,icon,hidden", "elm");
   edje_object_message_signal_process(tg->base);
}

static void
_elm_toggle_activate(Elm_Toggle *tg)
{
   if (tg->state_ptr) *(tg->state_ptr) = tg->state;
   _elm_obj_nest_push();
   _elm_cb_call(ELM_OBJ(tg), ELM_CB_CHANGED, NULL);
   _elm_obj_nest_pop();
}

static void
_elm_signal_toggle_on(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Elm_Toggle *tg = data;

   if (tg->state) return;
   tg->state = 1;
   _elm_toggle_activate(tg);
}

static void
_elm_signal_toggle_off(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Elm_Toggle *tg = data;
   
   if (!tg->state) return;
   tg->state = 0;
   _elm_toggle_activate(tg);
}

static void
_elm_toggle_del(Elm_Toggle *tg)
{
   if (tg->text) evas_stringshare_del(tg->text);
   ((Elm_Obj_Class *)(((Elm_Toggle_Class *)(tg->clas))->parent))->del(ELM_OBJ(tg));
}

EAPI Elm_Toggle *
elm_toggle_new(Elm_Win *win)
{
   Elm_Toggle *tg;
   
   tg = ELM_NEW(Elm_Toggle);
   
   _elm_widget_init(tg);
   tg->clas = &_elm_toggle_class;
   tg->type = ELM_OBJ_TOGGLE;
   
   tg->del = _elm_toggle_del;
   
   tg->size_alloc = _elm_toggle_size_alloc;
   tg->size_req = _elm_toggle_size_req;
   
   tg->text_set = _elm_toggle_text_set;
   tg->layout_update = _elm_toggle_layout_update;
   tg->states_text_set = _elm_toggle_states_text_set;
   
   tg->base = edje_object_add(win->evas);
   _elm_theme_set(tg->base, "toggle", "toggle");
   edje_object_part_text_set(tg->base, "elm.ontext", "ON");
   edje_object_part_text_set(tg->base, "elm.offtext", "OFF");
   edje_object_signal_callback_add(tg->base, "elm,action,toggle,on", "",
				   _elm_signal_toggle_on, tg);
   edje_object_signal_callback_add(tg->base, "elm,action,toggle,off", "",
				   _elm_signal_toggle_off, tg);
   tg->cb_add(tg, ELM_CB_CHILD_ADD, _elm_on_child_add, NULL);
   tg->cb_add(tg, ELM_CB_CHILD_DEL, _elm_on_child_del, NULL);
   _elm_widget_post_init(tg);
   win->child_add(win, tg);
   return tg;
}

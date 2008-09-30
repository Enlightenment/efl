#include <Elementary.h>
#include "elm_priv.h"

Elm_Cb_Class _elm_cb_class =
{
   &_elm_obj_class,
     ELM_OBJ_CB
};



static void
_elm_cb_del(Elm_Cb *cb)
{
   if (_elm_obj_del_defer(ELM_OBJ(cb))) return;
   if (cb->parent) /* callbacks are special children */
     {
	cb->parent->cbs = evas_list_remove(cb->parent->cbs, cb);
	cb->parent = NULL;
     }
   ((Elm_Obj_Class *)(((Elm_Cb_Class *)(cb->clas))->parent))->del(ELM_OBJ(cb));
}
    
Elm_Cb *
_elm_cb_new(void)
{
   Elm_Cb *cb;
   
   cb = ELM_NEW(Elm_Cb);
   _elm_obj_init(ELM_OBJ(cb));
   cb->clas = &_elm_cb_class;
   cb->type = ELM_OBJ_CB;

   cb->del = _elm_cb_del;

   return cb;
}

void
_elm_cb_call(Elm_Obj *obj, Elm_Cb_Type type, void *info)
{
   Evas_List *l;
   
   _elm_obj_nest_push();
   for (l = obj->cbs; l; l = l->next)
     {
	Elm_Cb *cb;
	
	cb = l->data;
	if (cb->delete_me) continue;
	if (cb->cb_type == type)
	  {
	     if (cb->func) cb->func(cb->data, obj, type, info);
	     if (cb->cbs) _elm_cb_call(cb, type, info);
	  }
     }
   _elm_obj_nest_pop();
}

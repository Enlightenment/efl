#include <Elementary.h>
#include "elm_priv.h"

static void _elm_obj_del(Elm_Obj *obj);
static void _elm_obj_ref(Elm_Obj *obj);
static void _elm_obj_unref(Elm_Obj *obj);
static Elm_Cb *_elm_obj_cb_add(Elm_Obj *obj, Elm_Cb_Type type, Elm_Cb_Func func, void *data);
static void _elm_obj_child_add(Elm_Obj *obj, Elm_Obj *child);
static int _elm_obj_hastype(Elm_Obj *obj, Elm_Obj_Type type);

Elm_Obj_Class _elm_obj_class =
{
   NULL, /* parent */ 
     ELM_OBJ_OBJ,
     _elm_obj_del,
     _elm_obj_ref,
     _elm_obj_unref,
     _elm_obj_cb_add,
     _elm_obj_child_add,
     
};

static int        deferred_nest = 0;
static Evas_List *deferred_deletions = NULL;

static void
_elm_obj_del(Elm_Obj *obj)
{
   if (_elm_obj_del_defer(obj)) return;
   _elm_obj_unref(obj);
}

static void
_elm_obj_ref(Elm_Obj *obj)
{
   obj->ref++;
}

static void
_elm_obj_unref(Elm_Obj *obj)
{
   obj->ref--;
   if (obj->ref > 0) return;
   if (!obj->delete_me)
     {
	obj->del(obj);
	return;
     }
   if (obj->parent)
     obj->parent->children = evas_list_remove(obj->parent->children, obj);
   while (obj->cbs)
     {
	((Elm_Obj *)obj->cbs->data)->parent = NULL;
	((Elm_Obj *)obj->cbs->data)->del(obj->cbs->data);
	obj->cbs = evas_list_remove_list(obj->cbs, obj->cbs);
     }
   while (obj->children)
     {
	_elm_cb_call(obj, ELM_CB_CHILD_DEL, obj->children->data);
	((Elm_Obj *)obj->children->data)->parent = NULL;
	((Elm_Obj *)obj->children->data)->del(obj->children->data);
	obj->children = evas_list_remove_list(obj->children, obj->children);
     }
   free(obj);
}

static Elm_Cb *
_elm_obj_cb_add(Elm_Obj *obj, Elm_Cb_Type type, Elm_Cb_Func func, void *data)
{
   Elm_Cb *cb;
   
   cb = _elm_cb_new();
   cb->cb_type = type;
   cb->func = func;
   cb->data = data;
   cb->parent = obj;
   obj->cbs = evas_list_append(obj->cbs, cb);
}

static void
_elm_obj_child_add(Elm_Obj *obj, Elm_Obj *child)
{
   if (child->parent) child->unparent(child);
   obj->children = evas_list_prepend(obj->children, child);
   child->parent = obj;
   _elm_obj_nest_push();
   _elm_cb_call(obj, ELM_CB_CHILD_ADD, child);
   _elm_cb_call(child, ELM_CB_PARENT, NULL);
   _elm_obj_nest_pop();
}

static void
_elm_obj_unparent(Elm_Obj *obj)
{
   Elm_Obj *parent;
   
   parent = obj->parent;
   obj->parent = NULL;
   // FIXME: what if we are walking the children when we unparent?
   parent->children = evas_list_remove(parent->children, obj);
   _elm_obj_nest_push();
   _elm_cb_call(parent, ELM_CB_CHILD_DEL, obj);
   _elm_cb_call(obj, ELM_CB_UNPARENT, NULL);
   _elm_obj_nest_pop();
}

static int
_elm_obj_class_hastype(Elm_Obj_Class *clas, Elm_Obj_Type type)
{
   if (clas->type == type) return 1;
   if (!clas->parent) return 0;
   return _elm_obj_class_hastype(clas->parent, type);
}

static int
_elm_obj_hastype(Elm_Obj *obj, Elm_Obj_Type type)
{
   if (obj->type == type) return 1;
   if (obj->clas) return _elm_obj_class_hastype(obj->clas, type);
   return 0;
}

void
_elm_obj_init(Elm_Obj *obj)
{
   obj->del = _elm_obj_del;
   obj->ref = _elm_obj_ref;
   obj->unref = _elm_obj_unref;
   obj->cb_add = _elm_obj_cb_add;
   obj->child_add = _elm_obj_child_add;
   obj->unparent = _elm_obj_unparent;
   obj->hastype = _elm_obj_hastype;
   obj->type = ELM_OBJ_OBJ;
   obj->clas = &_elm_obj_class;
   obj->refs = 1;
}

void
_elm_obj_nest_push(void)
{
   deferred_nest++;
}

void
_elm_obj_nest_pop(void)
{
   deferred_nest--;
   if (deferred_nest > 0) return;
   while (deferred_deletions)
     {
	((Elm_Obj *)(deferred_deletions->data))->delete_deferred = 0;
	((Elm_Obj *)(deferred_deletions->data))->del(ELM_OBJ(deferred_deletions->data));
	deferred_deletions = evas_list_remove_list(deferred_deletions, deferred_deletions);
     }
}

int
_elm_obj_del_defer(Elm_Obj *obj)
{
   if (obj->delete_deferred) return 1;
   if (!obj->delete_me)
     {
	/* will never be called during a deferred delete */
	obj->delete_me = 1;
	_elm_obj_nest_push();
	_elm_cb_call(obj, ELM_CB_DEL, NULL);
	_elm_obj_nest_pop();
     }
   if (deferred_nest > 0)
     {
	/* mark to be deleted later */
	obj->delete_deferred = 1;
	deferred_deletions = evas_list_append(deferred_deletions, obj);
	return 1;
     }
   return 0;
}

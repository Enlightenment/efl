#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

typedef struct _Evas_Inform_Func_Node    Evas_Inform_Func_Node;

struct _Evas_Inform_Func_Node
{
   void (*func) (void *data, Evas_Object *obj);
   void *data;
};

/* local calls */

static void evas_object_inform_init(Evas_Object *obj);
static void evas_object_inform_deinit(Evas_Object *obj);
static void evas_object_inform_list_add(Evas_Object *obj, Evas_List **l, void (*func) (void *data, Evas_Object *obj), void *data);
static void *evas_object_inform_list_del(Evas_Object *obj, Evas_List **list, void (*func) (void *data, Evas_Object *obj));
static void evas_object_inform_call(Evas_Object *obj, Evas_List *list);
    
static void
evas_object_inform_init(Evas_Object *obj)
{
   if (!obj->informers)
     obj->informers = evas_mem_calloc(sizeof(Evas_Inform_Func));
}

static void
evas_object_inform_deinit(Evas_Object *obj)
{
   if (!obj->informers) return;
   if ((obj->informers->show) ||
       (obj->informers->hide) ||
       (obj->informers->move) ||
       (obj->informers->resize) ||
       (obj->informers->restack)) return;
   free(obj->informers);
   obj->informers = NULL;
}

static void
evas_object_inform_list_add(Evas_Object *obj, Evas_List **l, void (*func) (void *data, Evas_Object *obj), void *data)
{
   Evas_Inform_Func_Node *fn;
   
   fn = evas_mem_calloc(sizeof(Evas_Inform_Func_Node));
   if (!fn) return;
   fn->func = func;
   fn->data = data;
   evas_object_inform_init(obj);
   if (!obj->informers)
     {
	free(fn);
	return;
     }
   do
     {
	*l = evas_list_append(*l, fn);
	if (!evas_list_alloc_error()) return;
	MERR_BAD();	
	if (!evas_mem_free(sizeof(Evas_List)))
	  {
	     if (!evas_mem_degrade(sizeof(Evas_List)))
	       {
		  evas_object_inform_deinit(obj);		  
		  MERR_FATAL();
		  free(fn);
		  return;
	       }
	  }
     }
   while (evas_list_alloc_error());   
}

static void *
evas_object_inform_list_del(Evas_Object *obj, Evas_List **list, void (*func) (void *data, Evas_Object *obj))
{
   if (!obj->informers) return NULL;
     {
	Evas_List *l;
	
	for (l = (*list); l; l = l->next)
	  {
	     Evas_Inform_Func_Node *fn;
	     
	     fn = l->data;
	     if (fn->func == func)
	       {
		  void *data;
		  
		  data = fn->data;
		  free(fn);
		  (*list) = evas_list_remove_list((*list), l);
		  evas_object_inform_deinit(obj);
		  return data;
	       }
	  }
     }
   return NULL;
}

static void
evas_object_inform_call(Evas_Object *obj, Evas_List *list)
{
   Evas_List *l;
   
   for (l = list; l; l = l->next)
     {
	Evas_Inform_Func_Node *fn;
	
	fn = l->data;
	fn->func(fn->data, obj);
     }
}
  
/* private calls */

void
evas_object_inform_cleanup(Evas_Object *obj)
{
   if (!obj->informers) return;
   while (obj->informers->show)
     {
	free(obj->informers->show->data);
	obj->informers->show = evas_list_remove(obj->informers->show, obj->informers->show->data);
     }
   while (obj->informers->hide)
     {
	free(obj->informers->hide->data);
	obj->informers->hide = evas_list_remove(obj->informers->hide, obj->informers->hide->data);
     }
   while (obj->informers->move)
     {
	free(obj->informers->move->data);
	obj->informers->move = evas_list_remove(obj->informers->move, obj->informers->move->data);
     }
   while (obj->informers->resize)
     {
	free(obj->informers->resize->data);
	obj->informers->resize = evas_list_remove(obj->informers->resize, obj->informers->resize->data);
     }
   while (obj->informers->restack)
     {
	free(obj->informers->restack->data);
	obj->informers->restack = evas_list_remove(obj->informers->restack, obj->informers->restack->data);
     }
   free(obj->informers);
}

void
evas_object_inform_call_show(Evas_Object *obj)
{
   if (!obj->informers) return;
   evas_object_inform_call(obj, obj->informers->show);
}

void
evas_object_inform_call_hide(Evas_Object *obj)
{
   if (!obj->informers) return;
   evas_object_inform_call(obj, obj->informers->hide);
}

void
evas_object_inform_call_move(Evas_Object *obj)
{
   if (!obj->informers) return;
   evas_object_inform_call(obj, obj->informers->move);
}

void
evas_object_inform_call_resize(Evas_Object *obj)
{
   if (!obj->informers) return;
   evas_object_inform_call(obj, obj->informers->resize);
}

void
evas_object_inform_call_restack(Evas_Object *obj)
{
   if (!obj->informers) return;
   evas_object_inform_call(obj, obj->informers->restack);
}

/* public calls */

void
evas_object_inform_show_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), void *data)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!func) return;
   evas_object_inform_list_add(obj, &(obj->informers->show), func, data);
}

void *
evas_object_inform_show_callback_del(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj))
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   return evas_object_inform_list_del(obj, &(obj->informers->show), func);
}

void
evas_object_inform_hide_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), void *data)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();   
   if (!func) return;
   evas_object_inform_list_add(obj, &(obj->informers->hide), func, data);
}

void *
evas_object_inform_hide_callback_del(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj))
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();   
   if (!func) return;
   return evas_object_inform_list_del(obj, &(obj->informers->hide), func);
}

void
evas_object_inform_move_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), void *data)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();   
   if (!func) return;
   evas_object_inform_list_add(obj, &(obj->informers->move), func, data);
}

void *
evas_object_inform_move_callback_del(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj))
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();   
   if (!func) return NULL;
   return evas_object_inform_list_del(obj, &(obj->informers->move), func);
}

void
evas_object_inform_resize_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), void *data)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();   
   if (!func) return;
   evas_object_inform_list_add(obj, &(obj->informers->resize), func, data);
}

void *
evas_object_inform_resize_callback_del(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj))
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();   
   if (!func) return NULL;
   return evas_object_inform_list_del(obj, &(obj->informers->resize), func);
}

void
evas_object_inform_restack_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), void *data)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();   
   if (!func) return;
   evas_object_inform_list_add(obj, &(obj->informers->restack), func, data);
}

void *
evas_object_inform_restack_callback_del(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj))
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   return evas_object_inform_list_del(obj, &(obj->informers->restack), func);
}

#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

void
evas_object_inject(Evas_Object *obj, Evas *e)
{
   Evas_Layer *lay;
   
   lay = evas_layer_find(e, obj->cur.layer);
   if (!lay)
     {
	lay = evas_layer_new(e);
	lay->layer = obj->cur.layer;
	evas_layer_add(lay);
     }
   lay->objects = evas_object_list_append(lay->objects, obj);   
   obj->layer = lay;
}

void
evas_object_release(Evas_Object *obj, int clean_layer)
{
   obj->layer->objects = evas_object_list_remove(obj->layer->objects, obj);
   if (clean_layer)
     {
	if (!obj->layer->objects)
	  {
	     evas_layer_del(obj->layer);
	     evas_layer_free(obj->layer);
	  }
     }
   obj->layer = NULL;
}

Evas_Layer *
evas_layer_new(Evas *e)
{
   Evas_Layer *lay;
   
   lay = calloc(1, sizeof(Evas_Layer));
   if (!lay) return NULL;
   lay->evas = e;
   return lay;
}

void
evas_layer_free(Evas_Layer *lay)
{
   while (lay->objects)
     {
	Evas_Object *obj;
	
	obj = (Evas_Object *)lay->objects;
	if (obj->smart.smart) evas_object_smart_del(obj);	
	evas_object_free(obj, 0);
     }
   free(lay);
}

Evas_Layer *
evas_layer_find(Evas *e, int layer_num)
{
   Evas_Object_List *list;
   
   for (list = (Evas_Object_List *)e->layers; list; list = list->next)
     {
	Evas_Layer *layer;
	
	layer = (Evas_Layer *)list;
	if (layer->layer == layer_num) return layer;
     }
   return NULL;
}

void
evas_layer_add(Evas_Layer *lay)
{
   Evas_Object_List *list;
   
   for (list = (Evas_Object_List *)lay->evas->layers; list; list = list->next)
     {
	Evas_Layer *layer;
	
	layer = (Evas_Layer *)list;
	if (layer->layer > lay->layer)
	  {
	     lay->evas->layers = evas_object_list_prepend_relative(lay->evas->layers, lay, layer);
	     return;
	  }
     }
   lay->evas->layers = evas_object_list_append(lay->evas->layers, lay);
}

void
evas_layer_del(Evas_Layer *lay)
{
   Evas_Object_List *ol;
   Evas *e;
   
   ol = (Evas_Object_List *)lay;
   e = lay->evas;
   e->layers = evas_object_list_remove(e->layers, lay);
}

/* public functions */

void
evas_object_layer_set(Evas_Object *obj, int l)
{
   Evas *e;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (evas_object_intercept_call_layer_set(obj, l)) return;
   if (obj->cur.layer == l) 
     {
	evas_object_raise(obj);
	return;
     }
   if (obj->smart.smart)
     {
       if (obj->smart.smart->func_layer_set)
	  obj->smart.smart->func_layer_set(obj, l);
     }
   e = obj->layer->evas;
   evas_object_release(obj, 1);
   obj->cur.layer = l;
   evas_object_inject(obj, e);
   obj->restack = 1;
   if (obj->clip.clipees)
     {
	evas_object_inform_call_restack(obj);
	return;
     }
   if (!obj->smart.smart)
     {
	evas_object_change(obj);
	if (evas_object_is_in_output_rect(obj, 
					  obj->layer->evas->pointer.x, 
					  obj->layer->evas->pointer.y, 1, 1) &&
	    obj->cur.visible)
	  if (evas_list_find(obj->layer->evas->pointer.object.in, obj))
	    evas_event_feed_mouse_move(obj->layer->evas, obj->layer->evas->pointer.x, obj->layer->evas->pointer.y);   
     }
   evas_object_inform_call_restack(obj);
}

int
evas_object_layer_get(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return obj->cur.layer;
}

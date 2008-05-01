#include "evas_common.h"
#include "evas_private.h"

void
evas_object_inject(Evas_Object *obj, Evas *e)
{
   Evas_Layer *lay;

   if (obj->in_layer) return;
   lay = evas_layer_find(e, obj->cur.layer);
   if (!lay)
     {
	lay = evas_layer_new(e);
	lay->layer = obj->cur.layer;
	evas_layer_add(lay);
     }
   lay->objects = evas_object_list_append(lay->objects, obj);
   lay->usage++;
   obj->layer = lay;
   obj->in_layer = 1;
}

void
evas_object_release(Evas_Object *obj, int clean_layer)
{
   if (!obj->in_layer) return;
   obj->layer->objects = evas_object_list_remove(obj->layer->objects, obj);
   obj->layer->usage--;
   if (clean_layer)
     {
	if (obj->layer->usage <= 0)
	  {
	     evas_layer_del(obj->layer);
	     evas_layer_free(obj->layer);
	  }
     }
   obj->layer = NULL;
   obj->in_layer = 0;
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
evas_layer_pre_free(Evas_Layer *lay)
{
   Evas_Object_List *l;

   for (l = (Evas_Object_List *)lay->objects; l; l = l->next)
     {
	Evas_Object *obj;

	obj = (Evas_Object *)l;
	if ((!obj->smart.parent) && (!obj->delete_me))
	  evas_object_del(obj);
     }
}

void
evas_layer_free(Evas_Layer *lay)
{
   while (lay->objects)
     {
	Evas_Object *obj;

	obj = (Evas_Object *)lay->objects;
	evas_object_free(obj, 0);
     }
   free(lay);
}

Evas_Layer *
evas_layer_find(Evas *e, short layer_num)
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

/**
 * @defgroup Evas_Object_Layer_Group Object Layer Functions
 *
 * Functions that retrieve and set the layer that an evas object is on.
 *
 * @todo Document which way layers go.
 */

/**
 * Sets the layer of the evas that the given object will be part of.
 * @param   obj The given evas object.
 * @param   l   The number of the layer to place the object on.
 * @ingroup Evas_Object_Layer_Group
 */
EAPI void
evas_object_layer_set(Evas_Object *obj, short l)
{
   Evas *e;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (evas_object_intercept_call_layer_set(obj, l)) return;
   if (obj->smart.parent) return;
   if (obj->cur.layer == l)
     {
	evas_object_raise(obj);
	return;
     }
   e = obj->layer->evas;
   evas_object_release(obj, 1);
   obj->cur.layer = l;
   evas_object_inject(obj, e);
   obj->restack = 1;
   evas_object_change(obj);
   if (obj->clip.clipees)
     {
	evas_object_inform_call_restack(obj);
	return;
     }
   evas_object_change(obj);
   if (!obj->smart.smart)
     {
	if (evas_object_is_in_output_rect(obj,
					  obj->layer->evas->pointer.x,
					  obj->layer->evas->pointer.y, 1, 1) &&
	    obj->cur.visible)
	  if (evas_list_find(obj->layer->evas->pointer.object.in, obj))
	    evas_event_feed_mouse_move(obj->layer->evas,
				       obj->layer->evas->pointer.x,
				       obj->layer->evas->pointer.y,
				       obj->layer->evas->last_timestamp,
				       NULL);
     }
   evas_object_inform_call_restack(obj);
}

/**
 * Retrieves the layer of the evas that the given object is part of.
 * @param   obj The given evas object.
 * @return  Number of the layer.
 * @ingroup Evas_Object_Layer_Group
 */
EAPI short
evas_object_layer_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   if (obj->smart.parent)
     {
        return obj->smart.parent->cur.layer;
     }
   return obj->cur.layer;
}

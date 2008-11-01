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
   lay->objects = (Evas_Object *)eina_inlist_append(EINA_INLIST_GET(lay->objects), EINA_INLIST_GET(obj));
   lay->usage++;
   obj->layer = lay;
   obj->in_layer = 1;
}

void
evas_object_release(Evas_Object *obj, int clean_layer)
{
   if (!obj->in_layer) return;
   obj->layer->objects = (Evas_Object *)eina_inlist_remove(EINA_INLIST_GET(obj->layer->objects), EINA_INLIST_GET(obj));
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
   Evas_Object *obj;

   EINA_INLIST_FOREACH(lay->objects, obj)
     {
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
   Evas_Layer *layer;

   EINA_INLIST_FOREACH(e->layers, layer)
     {
	if (layer->layer == layer_num) return layer;
     }
   return NULL;
}

void
evas_layer_add(Evas_Layer *lay)
{
   Evas_Layer *layer;

   EINA_INLIST_FOREACH(lay->evas->layers, layer)
     {
	if (layer->layer > lay->layer)
	  {
	     lay->evas->layers = (Evas_Layer *)eina_inlist_prepend_relative(EINA_INLIST_GET(lay->evas->layers),
									    EINA_INLIST_GET(lay),
									    EINA_INLIST_GET(layer));
	     return;
	  }
     }
   lay->evas->layers = (Evas_Layer *)eina_inlist_append(EINA_INLIST_GET(lay->evas->layers), EINA_INLIST_GET(lay));
}

void
evas_layer_del(Evas_Layer *lay)
{
   Evas *e;

   e = lay->evas;
   e->layers = (Evas_Layer *)eina_inlist_remove(EINA_INLIST_GET(e->layers), EINA_INLIST_GET(lay));
}

/* public functions */

/**
 * @addtogroup Evas_Object_Group
 * @{
 */

/**
 * Sets the layer of the evas that the given object will be part of.
 * @param   obj The given evas object.
 * @param   l   The number of the layer to place the object on.
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
	  if (eina_list_data_find(obj->layer->evas->pointer.object.in, obj))
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

/**
 * @}
 */

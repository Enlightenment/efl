#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define SPANS_COMMON(x1, w1, x2, w2) \
(!((((x2) + (w2)) <= (x1)) || ((x2) >= ((x1) + (w1)))))
#define RECTS_INTERSECT(x, y, w, h, xx, yy, ww, hh) \
((SPANS_COMMON((x), (w), (xx), (ww))) && (SPANS_COMMON((y), (h), (yy), (hh))))

Evas_Layer
_evas_get_object_layer(Evas e, Evas_Object o)
{
   Evas_List l;
   
   for (l = e->layers; l; l = l->next)
     {
        Evas_Layer layer;
	
	layer = l->data;
	if (layer->layer == o->current.layer) return layer;
     }
   return NULL;
}

void
_evas_remove_data(Evas e, Evas_Object o)
{
   Evas_List l;

   if (o->data)
     {
	for (l = o->data; l; l = l->next)
	  {
	     Evas_Data d;
	     
	     d = l->data;
	     if (d->key)
		free(d->key);
	     free(d);
	  }
	evas_list_free(o->data);   
     }
}

void
_evas_remove_callbacks(Evas e, Evas_Object o)
{
   Evas_List l;

   if (o->callbacks)
     {
	for (l = o->callbacks; l; l = l->next)
	  {
	     Evas_Callback cb;
	     
	     cb = l->data;
	     free(cb);
	  }
	evas_list_free(o->callbacks);
     }
}

void
_evas_real_del_object(Evas e, Evas_Object o)
{
   Evas_List l;
   
   for (l = e->layers; l; l = l->next)
     {
	Evas_Layer layer;
	
	layer = l->data;
	if (layer->layer == o->current.layer)
	  {
	     layer->objects = evas_list_remove(layer->objects, o);
	     _evas_callback_call(e, o, CALLBACK_FREE, 0, 0, 0);
	     _evas_remove_callbacks(e, o);
	     _evas_remove_data(e, o);
	     o->object_renderer_data_free(e, o);
	     o->object_free(o);
	     return;
	  }
     }
}

void
_evas_layer_free(Evas e, Evas_Layer layer)
{
   while (layer->objects)
     {
	Evas_Object o;
	
	o = layer->objects->data;
	layer->objects = evas_list_remove(layer->objects, o);
	_evas_callback_call(e, o, CALLBACK_FREE, 0, 0, 0);
	_evas_remove_callbacks(e, o);
	_evas_remove_data(e, o);
	if (o->object_renderer_data_free)
	   o->object_renderer_data_free(e, o);
	if (o->object_free)
	   o->object_free(o);
     }
   free(layer);
}

/* deleting objects */
void
evas_del_object(Evas e, Evas_Object o)
{
   o->delete_me = 1;
   evas_hide(e, o);
}

/* layer stacking for object */
void
evas_set_layer(Evas e, Evas_Object o, int layer_num)
{
   Evas_Layer        layer;
   Evas_List         l;
   int removed;

   if (layer_num == o->current.layer) return;
   removed = 0;
   for (l = e->layers; l; l = l->next)
     {
	layer = l->data;
	if (layer->layer == o->current.layer)
	  {
	     layer->objects = evas_list_remove(layer->objects, o);
	     removed = 1;
	     if (!layer->objects)
	       {
		  e->layers = evas_list_remove(e->layers, layer);
		  _evas_layer_free(e, layer);
	       }
	     break;
	  }
     }
   if (!removed) return;
   o->changed = 1;
   e->changed = 1;
   o->current.layer = layer_num;
   for (l = e->layers; l; l = l->next)
     {
	layer = l->data;
	if (layer->layer == o->current.layer)
	  {
	     layer->objects = evas_list_append(layer->objects, o);
	     return;
	  }
	if (layer->layer > o->current.layer)
	  {
	     Evas_Layer        layer_new;
	     
	     layer_new = malloc(sizeof(struct _Evas_Layer));
	     memset(layer_new, 0, sizeof(struct _Evas_Layer));
	     e->layers = evas_list_prepend_relative(e->layers, layer_new, layer);
	     layer_new->objects = evas_list_append(layer_new->objects, o);
	     layer_new->layer = o->current.layer;
	     if ((o->current.visible) && 
		 (_evas_point_in_object(e, o, e->mouse.x, e->mouse.y)))
		evas_event_move(e, e->mouse.x, e->mouse.y);
	     return;
	  }
     }

   layer = malloc(sizeof(struct _Evas_Layer));
   memset(layer, 0, sizeof(struct _Evas_Layer));
   e->layers = evas_list_append(e->layers, layer);
   layer->objects = evas_list_append(layer->objects, o);
   layer->layer = o->current.layer;
   if ((o->current.visible) && 
       (_evas_point_in_object(e, o, e->mouse.x, e->mouse.y)))
      evas_event_move(e, e->mouse.x, e->mouse.y);
}

void
evas_set_layer_store(Evas e, int l, int store)
{
/* FIXME: find layer and set store flag */
/*   e->changed = 1; */
}

Evas_List
evas_objects_in_rect(Evas e, double x, double y, double w, double h)
{
   Evas_List l, ll, objs;
   Evas_Layer layer;
   
   objs = NULL;
   for (l = e->layers; l ; l = l->next)
     {
	layer = l->data;
	
	for (ll = layer->objects; ll; ll = ll->next)
	  {
	     Evas_Object ob;
	     
	     ob = ll->data;
	     if ((ob->current.visible) && (!ob->pass_events))
	       {
		  if (RECTS_INTERSECT(x, y, w, h, 
				     ob->current.x, ob->current.y, 
				     ob->current.w, ob->current.h))
		     objs = evas_list_prepend(objs, ll->data);
	       }
	  }
     }
   return objs;
}

Evas_List
evas_objects_at_position(Evas e, double x, double y)
{
   Evas_List l, ll, objs;
   Evas_Layer layer;
   
   objs = NULL;
   for (l = e->layers; l ; l = l->next)
     {
	layer = l->data;
	
	for (ll = layer->objects; ll; ll = ll->next)
	  {
	     Evas_Object ob;
	     
	     ob = ll->data;
	     if ((ob->current.visible) && (!ob->pass_events))
	       {
		  if (RECTS_INTERSECT(x, y, 1, 1, 
				     ob->current.x, ob->current.y, 
				     ob->current.w, ob->current.h))
		     objs = evas_list_prepend(objs, ll->data);
	       }
	  }
     }
   return objs;
}

Evas_Object
evas_object_in_rect(Evas e, double x, double y, double w, double h)
{
   Evas_List l, ll;
   Evas_Layer layer;
   Evas_Object o;
   
   o = NULL;
   for (l = e->layers; l ; l = l->next)
     {
	layer = l->data;
	
	for (ll = layer->objects; ll; ll = ll->next)
	  {
	     Evas_Object ob;
	     
	     ob = ll->data;
	     if ((ob->current.visible) && (!ob->pass_events))
	       {
		  if (RECTS_INTERSECT(x, y, w, h, 
				      ob->current.x, ob->current.y, 
				      ob->current.w, ob->current.h))
		     o = ob;
	       }
	  }
     }
   return o;
}

Evas_Object
evas_object_at_position(Evas e, double x, double y)
{
   Evas_List l, ll;
   Evas_Layer layer;
   Evas_Object o;
   
   o = NULL;
   for (l = e->layers; l ; l = l->next)
     {
	layer = l->data;
	
	for (ll = layer->objects; ll; ll = ll->next)
	  {
	     Evas_Object ob;
	     
	     ob = ll->data;
	     if ((ob->current.visible) && (!ob->pass_events))
	       {
		  if (RECTS_INTERSECT(x, y, 1, 1, 
				     ob->current.x, ob->current.y, 
				     ob->current.w, ob->current.h))
		      o = ob;
	       }
	  }
     }
   return o;
}

/* stacking within a layer */
void
evas_raise(Evas e, Evas_Object o)
{
   Evas_Layer layer;
   
   layer = _evas_get_object_layer(e, o);
   if (layer)
     {
	o->current.stacking = 1;
	layer->objects = evas_list_remove(layer->objects, o);
	layer->objects = evas_list_append(layer->objects, o);
	o->changed = 1;
	e->changed = 1;
	if ((o->current.visible) && 
	    (_evas_point_in_object(e, o, e->mouse.x, e->mouse.y)))
	   evas_event_move(e, e->mouse.x, e->mouse.y);
     }
}

void
evas_lower(Evas e, Evas_Object o)
{
   Evas_Layer layer;
   
   layer = _evas_get_object_layer(e, o);
   if (layer)
     {
	o->current.stacking = 1;
	layer->objects = evas_list_remove(layer->objects, o);
	layer->objects = evas_list_prepend(layer->objects, o);
	o->changed = 1;
	e->changed = 1;
	if ((o->current.visible) && 
	    (_evas_point_in_object(e, o, e->mouse.x, e->mouse.y)))
	   evas_event_move(e, e->mouse.x, e->mouse.y);
     }
}

void
evas_stack_above(Evas e, Evas_Object o, Evas_Object above)
{
   Evas_Layer layer;
   
   layer = _evas_get_object_layer(e, o);
   if (layer)
     {
	o->current.stacking = 1;
	layer->objects = evas_list_remove(layer->objects, o);
	layer->objects = evas_list_append_relative(layer->objects, o, above);
	o->changed = 1;
	e->changed = 1;
	if ((o->current.visible) && 
	    (_evas_point_in_object(e, o, e->mouse.x, e->mouse.y)))
	   evas_event_move(e, e->mouse.x, e->mouse.y);
     }
}

void
evas_stack_below(Evas e, Evas_Object o, Evas_Object above)
{
   Evas_Layer layer;
   
   layer = _evas_get_object_layer(e, o);
   if (layer)
     {
	o->current.stacking = 1;
	layer->objects = evas_list_remove(layer->objects, o);
	layer->objects = evas_list_prepend_relative(layer->objects, o, above);
	o->changed = 1;
	e->changed = 1;
	if ((o->current.visible) && 
	    (_evas_point_in_object(e, o, e->mouse.x, e->mouse.y)))
	   evas_event_move(e, e->mouse.x, e->mouse.y);
     }
}

/* object geoemtry */
void
evas_move(Evas e, Evas_Object o, double x, double y)
{
   int event_update = 0;
   
   if ((o->type == OBJECT_LINE)) return;
   if ((o->current.visible) && 
       (_evas_point_in_object(e, o, e->mouse.x, e->mouse.y)))
      event_update = 1;
   o->current.x = x;
   o->current.y = y;
   o->changed = 1;
   e->changed = 1;
   if ((o->current.visible) &&
       ((_evas_point_in_object(e, o, e->mouse.x, e->mouse.y)) || 
	(event_update)))
      evas_event_move(e, e->mouse.x, e->mouse.y);
}

void
evas_resize(Evas e, Evas_Object o, double w, double h)
{
   int event_update = 0;
   
   if ((o->type == OBJECT_LINE)) return;
   if ((o->type == OBJECT_TEXT)) return;
   if ((o->current.visible) && 
       (_evas_point_in_object(e, o, e->mouse.x, e->mouse.y)))
      event_update = 1;
   o->current.w = w;
   o->current.h = h;
   o->changed = 1;
   e->changed = 1;
   if ((o->current.visible) &&
       ((_evas_point_in_object(e, o, e->mouse.x, e->mouse.y)) || 
	(event_update)))
      evas_event_move(e, e->mouse.x, e->mouse.y);
}

void
evas_get_geometry(Evas e, Evas_Object o, double *x, double *y, double *w, double *h)
{
   if (x) *x = o->current.x;
   if (y) *y = o->current.y;
   if (w) *w = o->current.w;
   if (h) *h = o->current.h;
   e->changed = 1;
}

/* object visibility */
void
evas_show(Evas e, Evas_Object o)
{
   o->current.visible = 1;
   o->changed = 1;
   e->changed = 1;
   if (_evas_point_in_object(e, o, e->mouse.x, e->mouse.y))
      evas_event_move(e, e->mouse.x, e->mouse.y);
}

void
evas_hide(Evas e, Evas_Object o)
{
   o->current.visible = 0;
   o->changed = 1;
   e->changed = 1;
   if (_evas_point_in_object(e, o, e->mouse.x, e->mouse.y))
      evas_event_move(e, e->mouse.x, e->mouse.y);
}

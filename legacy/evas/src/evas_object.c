#include "Evas_private.h"
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
	     if (e->mouse.object == o) e->mouse.object = NULL;
	     if (e->mouse.button_object == o) e->mouse.button_object = NULL;
	     _evas_remove_callbacks(e, o);
	     _evas_remove_data(e, o);
	     if (o->name) free(o->name);
	     o->object_renderer_data_free(e, o);
	     o->object_free(o);
	     return;
	  }
     }
}

void
_evas_layer_free(Evas e, Evas_Layer layer)
{
   if (layer->objects)
     {
	Evas_List         l;
	
	for (l = layer->objects; l; l = l->next)
	  {
	     Evas_Object o;
	     
	     o = l->data;
	     _evas_callback_call(e, o, CALLBACK_FREE, 0, 0, 0);
	     _evas_cleanup_clip(e, o);
	     if (e->mouse.object == o) e->mouse.object = NULL;
	     if (e->mouse.button_object == o) e->mouse.button_object = NULL;
	     _evas_remove_callbacks(e, o);
	     _evas_remove_data(e, o);
	     o->object_renderer_data_free(e, o);
	     o->object_free(o);
	  }
	evas_list_free(layer->objects);
     }
   free(layer);
}

void
_evas_cleanup_clip(Evas e, Evas_Object o)
{
   if (o->clip.list)
     {
	Evas_List l;
	
	for (l = o->clip.list; l; l = l->next)
	  {
	     Evas_Object o2;
	     
	     o2 = l->data;
	     o2->clip.object = NULL;
	     o2->clip.changed = 1;
	     o2->changed = 1;
	     e->changed = 1;
	  }
	evas_list_free(o->clip.list);
	o->clip.list = NULL;
     }
   if (o->clip.object)
     o->clip.object->clip.list = evas_list_remove(o->clip.object->clip.list, o);
}

void
_evas_get_current_clipped_geometry(Evas e, Evas_Object o, double *x, double *y, double *w, double *h)
{
   if (!o->current.visible)
     {
	*x = 0.0;
	*y = 0.0;
	*w = 0.0;
	*h = 0.0;
	return;
     }
   if (o->clip.object)
     _evas_get_current_clipped_geometry(e, o->clip.object, x, y, w, h);
   CLIP_TO(*x, *y, *w, *h, 
	   o->current.x, o->current.y, o->current.w, o->current.h);
}

void
_evas_get_previous_clipped_geometry(Evas e, Evas_Object o, double *x, double *y, double *w, double *h)
{
   if (!o->previous.visible)
     {
	*x = 0.0;
	*y = 0.0;
	*w = 0.0;
	*h = 0.0;
	return;
     }
   if (o->clip.object)
     _evas_get_current_clipped_geometry(e, o->clip.object, x, y, w, h);
   CLIP_TO(*x, *y, *w, *h, 
	   o->previous.x, o->previous.y, o->previous.w, o->previous.h);
}

int
_evas_point_in_object(Evas e, Evas_Object o, int x, int y)
{
   double cx, cy;
   double ox, oy, ow, oh;
   
   if (o->delete_me) return 0;
   cx = evas_screen_x_to_world(e, x);
   cy = evas_screen_x_to_world(e, y);
   ox = o->current.x; oy = o->current.y;
   ow = o->current.w; oh = o->current.h;
   _evas_get_current_clipped_geometry(e, o, &ox, &oy, &ow, &oh);
   if ((cx >= ox) && (cx < (ox + ow)) && (cy >= oy) && (cy < (oy + oh)))
     return 1;
   return 0;
}

void
evas_set_clip(Evas e, Evas_Object o, Evas_Object clip)
{
   if (!e) return;
   if (!o) return;
   if (!clip) return;

   if (o->clip.object == clip) return;
   e->changed = 1;
   o->changed = 1;
   o->clip.changed = 1;
   if (o->clip.object)
     o->clip.object->clip.list = evas_list_remove(o->clip.object->clip.list, o);
   o->clip.object = clip;
   clip->clip.list = evas_list_prepend(clip->clip.list, o);
}

void
evas_unset_clip(Evas e, Evas_Object o)
{
   if (!e) return;
   if (!o) return;
   
   if (o->clip.object)
     {
	e->changed = 1;
	o->changed = 1;
	o->clip.changed = 1;
	o->clip.object->clip.list = evas_list_remove(o->clip.object->clip.list, o);
	o->clip.object = NULL;
     }
}

Evas_Object
evas_get_clip_object(Evas e, Evas_Object o)
{
   if (!e) return NULL;
   if (!o) return NULL;
   
   return o->clip.object;
}

Evas_List
evas_get_clip_list(Evas e, Evas_Object o)
{
   if (!e) return NULL;
   if (!o) return NULL;

   return o->clip.list;
}

/* deleting objects */
void
evas_del_object(Evas e, Evas_Object o)
{
   if (!e) return;
   if (!o) return;
   _evas_cleanup_clip(e, o);
   e->changed = 1;
   evas_hide(e, o);
   o->delete_me = 1;
}

/* layer stacking for object */
void
evas_set_layer(Evas e, Evas_Object o, int layer_num)
{
   Evas_Layer        layer;
   Evas_List         l;
   int removed;

   if (!e) return;
   if (!o) return;
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

int
evas_get_layer(Evas e, Evas_Object o)
{
   Evas_Layer        layer;
   Evas_List         l;
   int removed;

   if (!e) return 0;
   if (!o) return 0;
   return o->current.layer;
}

void
evas_set_layer_store(Evas e, int l, int store)
{
   if (!e) return;
/* FIXME: find layer and set store flag */
/*   e->changed = 1; */
}

Evas_List
evas_objects_in_rect(Evas e, double x, double y, double w, double h)
{
   Evas_List l, ll, objs;
   Evas_Layer layer;
   
   if (!e) return NULL;
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   objs = NULL;
   for (l = e->layers; l ; l = l->next)
     {
	layer = l->data;
	
	for (ll = layer->objects; ll; ll = ll->next)
	  {
	     Evas_Object ob;
	     
	     ob = ll->data;
	     if ((ob->current.visible) && (!ob->pass_events) && (!ob->delete_me) && (!ob->clip.list))
	       {
		  double ox, oy, ow, oh;
		  
		  ox = ob->current.x; oy = ob->current.y; 
		  ow = ob->current.w; oh = ob->current.h;
		  _evas_get_current_clipped_geometry(e, ob, &ox, &oy, &ow, &oh);
		  if (RECTS_INTERSECT(x, y, w, h, ox, oy, ow, oh))
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
   
   if (!e) return NULL;
   objs = NULL;
   for (l = e->layers; l ; l = l->next)
     {
	layer = l->data;
	
	for (ll = layer->objects; ll; ll = ll->next)
	  {
	     Evas_Object ob;
	     
	     ob = ll->data;
	     if ((ob->current.visible) && (!ob->pass_events) && (!ob->delete_me) && (!ob->clip.list))
	       {
		  double ox, oy, ow, oh;
		  
		  ox = ob->current.x; oy = ob->current.y; 
		  ow = ob->current.w; oh = ob->current.h;
		  _evas_get_current_clipped_geometry(e, ob, &ox, &oy, &ow, &oh);
		  if (RECTS_INTERSECT(x, y, 1, 1, ox, oy, ow, oh))
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
   
   if (!e) return NULL;
   o = NULL;
   for (l = e->layers; l ; l = l->next)
     {
	layer = l->data;
	
	for (ll = layer->objects; ll; ll = ll->next)
	  {
	     Evas_Object ob;
	     
	     ob = ll->data;
	     if ((ob->current.visible) && (!ob->pass_events) && (!ob->delete_me) && (!ob->clip.list))
	       {
		  double ox, oy, ow, oh;
		  
		  ox = ob->current.x; oy = ob->current.y; 
		  ow = ob->current.w; oh = ob->current.h;
		  _evas_get_current_clipped_geometry(e, ob, &ox, &oy, &ow, &oh);
		  if (RECTS_INTERSECT(x, y, w, h, ox, oy, ow, oh))
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
   
   if (!e) return NULL;
   o = NULL;
   for (l = e->layers; l ; l = l->next)
     {
	layer = l->data;
	
	for (ll = layer->objects; ll; ll = ll->next)
	  {
	     Evas_Object ob;
	     
	     ob = ll->data;
	     if ((ob->current.visible) && (!ob->pass_events) && (!ob->delete_me) && (!ob->clip.list))
	       {
		  double ox, oy, ow, oh;
		  
		  ox = ob->current.x; oy = ob->current.y; 
		  ow = ob->current.w; oh = ob->current.h;
		  _evas_get_current_clipped_geometry(e, ob, &ox, &oy, &ow, &oh);
		  if (RECTS_INTERSECT(x, y, 1.0, 1.0, ox, oy, ow, oh))
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
   
   if (!e) return;
   if (!o) return;
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
   
   if (!e) return;
   if (!o) return;
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
   
   if (!e) return;
   if (!o) return;
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
evas_stack_below(Evas e, Evas_Object o, Evas_Object below)
{
   Evas_Layer layer;
   
   if (!e) return;
   if (!o) return;
   layer = _evas_get_object_layer(e, o);
   if (layer)
     {
	o->current.stacking = 1;
	layer->objects = evas_list_remove(layer->objects, o);
	layer->objects = evas_list_prepend_relative(layer->objects, o, below);
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
   
   if (!e) return;
   if (!o) return;
   if ((o->type == OBJECT_LINE)) return;
   if ((x == o->current.x) && (y == o->current.y)) return;
   if ((o->current.visible) && 
       (_evas_point_in_object(e, o, e->mouse.x, e->mouse.y)))
      event_update = 1;
   if ((o->type == OBJECT_POLYGON))
     {
	Evas_Object_Poly oo;
	Evas_List l;
	Evas_Point p;
	double dx, dy;
	
	dx = x - o->current.x;
	dy = y - o->current.y;
        o->current.x = x;
	o->current.y = y;
	oo = o;
	for (l = oo->current.points; l; l = l->next)
	  {
	     p = l->data;
	     p->x += dx;
	     p->y += dy;
	  }
     }
   else
     {
	o->current.x = x;
	o->current.y = y;
     }
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
   
   if (!e) return;
   if (!o) return;
   if ((o->type == OBJECT_LINE)) return;
   if ((o->type == OBJECT_TEXT)) return;
   if ((o->type == OBJECT_POLYGON)) return;
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((w == o->current.w) && (h == o->current.h)) return;
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
   if (!e) return;
   if (!o) return;
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
   if (!e) return;
   if (!o) return;
   o->current.visible = 1;
   o->changed = 1;
   e->changed = 1;
   if (_evas_point_in_object(e, o, e->mouse.x, e->mouse.y))
      evas_event_move(e, e->mouse.x, e->mouse.y);
}

void
evas_hide(Evas e, Evas_Object o)
{
   if (!e) return;
   if (!o) return;
   o->current.visible = 0;
   o->changed = 1;
   e->changed = 1;
   if (_evas_point_in_object(e, o, e->mouse.x, e->mouse.y))
      evas_event_move(e, e->mouse.x, e->mouse.y);
   if (e->mouse.object == o) e->mouse.object = NULL;
   if (e->mouse.button_object == o) e->mouse.button_object = NULL;
}

Evas_Object
evas_object_get_named(Evas e, char *name)
{
   Evas_List l, ll;
   Evas_Layer layer;
   Evas_Object o;
   
   if (!e) return NULL;
   if (!name) return NULL;
   for (l = e->layers; l ; l = l->next)
     {
	layer = l->data;
	
	for (ll = layer->objects; ll; ll = ll->next)
	  {
	     o = ll->data;
	     if (!o->delete_me)
	       {
		  if ((o->name) && (!strcmp(name, o->name))) return o;
	       }
	  }
     }
   return NULL;
}

void
evas_object_set_name(Evas e, Evas_Object o, char *name)
{
   if (!e) return;
   if (!o) return;
   if (o->name) free(o->name);
   o->name = NULL;
   if (name)
     {
	o->name = malloc(strlen(name) + 1);
	strcpy(o->name, name);
     }
}

char *
evas_object_get_name(Evas e, Evas_Object o)
{
   if (!e) return NULL;
   if (!o) return NULL;
   return o->name;
}

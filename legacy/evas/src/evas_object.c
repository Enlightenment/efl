#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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
   if (layer->objects)
     {
	Evas_List         l;
	
	for (l = layer->objects; l; l = l->next)
	  {
	     Evas_Object o;
	     
	     o = l->data;
	     o->object_renderer_data_free(e, o);
	     o->object_free(o);	     
	  }
	evas_list_free(layer->objects);
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

   if (layer_num == o->current.layer) return;
   
   o->changed = 1;
   e->changed = 1;
   for (l = e->layers; l; l = l->next)
     {
	layer = l->data;
	if (layer->layer == o->current.layer)
	  {
	     layer->objects = evas_list_remove(layer->objects, o);
	     if (!layer->objects)
	       {
		  e->layers = evas_list_remove(e->layers, layer);
		  _evas_layer_free(e, layer);
	       }
	     break;
	  }
     }
   o->current.layer = layer_num;
   for (l = e->layers; l; l = l->next)
     {
	layer = l->data;
	if (layer->layer == o->current.layer)
	  {
	     layer->objects = evas_list_append(layer->objects, o);
	     break;
	  }
	if (layer->layer > o->current.layer)
	  {
	     Evas_Layer        layer_new;
	     
	     layer_new = malloc(sizeof(struct _Evas_Layer));
	     memset(layer_new, 0, sizeof(struct _Evas_Layer));
	     e->layers = evas_list_prepend_relative(e->layers, layer_new, layer);
	     layer_new->objects = evas_list_append(layer_new->objects, o);
	     layer_new->layer = o->current.layer;
	     return;
	  }
     }

   layer = malloc(sizeof(struct _Evas_Layer));
   memset(layer, 0, sizeof(struct _Evas_Layer));
   e->layers = evas_list_append(e->layers, layer);
   layer->objects = evas_list_append(layer->objects, o);
   layer->layer = o->current.layer;
}

void
evas_set_layer_store(Evas e, int l, int store)
{
/* FIXME: find layer and set store flag */
/*   e->changed = 1; */
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
     }
}

/* object geoemtry */
void
evas_move(Evas e, Evas_Object o, double x, double y)
{
   if ((o->type == OBJECT_LINE)) return;
   o->current.x = x;
   o->current.y = y;
   o->changed = 1;
   e->changed = 1;
}

void
evas_resize(Evas e, Evas_Object o, double w, double h)
{
   if ((o->type == OBJECT_TEXT) || (o->type == OBJECT_LINE)) return;
   o->current.w = w;
   o->current.h = h;
   o->changed = 1;
   e->changed = 1;
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
}

void
evas_hide(Evas e, Evas_Object o)
{
   o->current.visible = 0;
   o->changed = 1;
   e->changed = 1;
}

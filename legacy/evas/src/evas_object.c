#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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
	     o->object_renderer_data_free(e, o);
	     o->object_free(o);
	     return;
	  }
     }
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
evas_set_layer(Evas e, Evas_Object o, int l)
{
   e->changed = 1;
}

void
evas_set_layer_store(Evas e, int l, int store)
{
   e->changed = 1;
}

/* stacking within a layer */
void
evas_raise(Evas e, Evas_Object o)
{
   e->changed = 1;
}

void
evas_lower(Evas e, Evas_Object o)
{
   e->changed = 1;
}

void
evas_stack_above(Evas e, Evas_Object o, int above)
{
   e->changed = 1;
}

void
evas_stack_below(Evas e, Evas_Object o, int above)
{
   e->changed = 1;
}

/* object geoemtry */
void
evas_move(Evas e, Evas_Object o, double x, double y)
{
   o->current.x = x;
   o->current.y = y;
   o->changed = 1;
   e->changed = 1;
}

void
evas_resize(Evas e, Evas_Object o, double w, double h)
{
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

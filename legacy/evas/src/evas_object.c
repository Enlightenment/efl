#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* deleting objects */
void
evas_del_object(Evas e, Evas_Object o)
{
   Evas_List l;
   
   for (l = e->layers; l; l = l->next)
     {
	Evas_Layer layer;
	
	layer = l->data;
	if (layer->layer == o->current.layer)
	  {
	     layer->objects = evas_list_remove(layer->objects, o);
	     e->object_renderer_data_free(e, o);
	     o->object_free(o);
	     return;
	  }
     }
}

/* adding objects */

Evas_Object
evas_add_evas(Evas e, Evas evas)
{
}

/* layer stacking for object */
void
evas_set_layer(Evas e, Evas_Object o, int l)
{
}

void
evas_set_layer_store(Evas e, int l, int store)
{
}

/* stacking within a layer */
void
evas_raise(Evas e, Evas_Object o)
{
}

void
evas_lower(Evas e, Evas_Object o)
{
}

void
evas_stack_above(Evas e, Evas_Object o, int above)
{
}

void
evas_stack_below(Evas e, Evas_Object o, int above)
{
}

/* object geoemtry */
void
evas_move(Evas e, Evas_Object o, double x, double y)
{
}

void
evas_resize(Evas e, Evas_Object o, double w, double h)
{
}

void
evas_get_geometry(Evas e, Evas_Object o, double *x, double *y, double *w, double *h)
{
}

/* object visibility */
void
evas_show(Evas e, Evas_Object o)
{
}

void
evas_hide(Evas e, Evas_Object o)
{
}

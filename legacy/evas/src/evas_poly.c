#include "Evas_private.h"
#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static void
_evas_free_poly(Evas_Object o)
{
   Evas_Object_Poly oo;
   Evas_List l;
   Evas_Point p;
   
   IF_OBJ(o, OBJECT_POLYGON) return;
   oo = o;
   for (l = oo->current.points; l; l = l->next)
     {
	p = l->data;
	free(p);
     }
   evas_list_free(oo->current.points);
   free(o);
}

static void
_evas_free_poly_renderer_data(Evas e, Evas_Object o)
{
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	break;
     case RENDER_METHOD_3D_HARDWARE:
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     case RENDER_METHOD_IMAGE:
	break;
     default:
	break;
     }
}

/* adding objects */
Evas_Object
evas_add_poly(Evas e)
{
   Evas_Object_Poly oo;
   Evas_Object_Any   o;
   Evas_List         l;
   Evas_Layer        layer;

   if (!e) return NULL;
   o = oo = malloc(sizeof(struct _Evas_Object_Poly));
   memset(o, 0, sizeof(struct _Evas_Object_Poly));
   o->type = OBJECT_POLYGON;
   o->object_free = _evas_free_poly;
   o->object_renderer_data_free = _evas_free_poly_renderer_data;
   
   o->current.x = 0;
   o->current.y = 0;
   o->current.w = 0;
   o->current.h = 0;
	
   for (l = e->layers; l; l = l->next)
     {
	layer = l->data;
	if (layer->layer == o->current.layer)
	  {
	     layer->objects = evas_list_append(layer->objects, o);
	     return o;
	  }
     }
   
   layer = malloc(sizeof(struct _Evas_Layer));
   memset(layer, 0, sizeof(struct _Evas_Layer));
   e->layers = evas_list_append(e->layers, layer);
   layer->objects = evas_list_append(layer->objects, o);
   
   return o;
}

/* modifying object */
evas_clear_points(Evas e, Evas_Object o)
{
   Evas_Point p;
   Evas_Object_Poly oo;
   Evas_List l;
   
   IF_OBJ(o, OBJECT_POLYGON) return;
   oo = o;
   if (oo->current.points)
     {
	for (l = oo->current.points; l; l = l->next)
	  {
	     p = l->data;
	     free(p);
	  }
	evas_list_free(oo->current.points);
	oo->current.points = NULL;
	o->current.w = 0;
	o->current.h = 0;
	o->changed = 1;
	e->changed = 1;   
     }
}

Evas_List
evas_get_points(Evas e, Evas_Object o)
{
   Evas_Object_Poly oo;
   
   IF_OBJ(o, OBJECT_POLYGON) return NULL;
   oo = o;
   return oo->current.points;
}

void
evas_add_point(Evas e, Evas_Object o, double x, double y)
{
   Evas_Point p;
   Evas_Object_Poly oo;
   
   IF_OBJ(o, OBJECT_POLYGON) return;
   oo = o;
   p = malloc(sizeof(struct _Evas_Point));
   p->x = x;
   p->y = y;
   oo->previous.points = NULL;
   if (!oo->current.points)
     {
	oo->current.points = evas_list_append(oo->current.points, p);
	o->current.x = x;
	o->current.y = y;
	o->current.w = 1;
	o->current.h = 1;
     }
   else
     {
	oo->current.points = evas_list_append(oo->current.points, p);
	if (x > (o->current.x + o->current.w)) o->current.w = x - o->current.x + 1;
	else if (x < o->current.x) 
	   {
	      o->current.w += o->current.x - x;
	      o->current.x = x;
	   }
	if (y > (o->current.y + o->current.h)) o->current.h = y - o->current.y + 1;
	else if (y < o->current.y) 
	   {
	      o->current.h += o->current.y - y;
	      o->current.y = y;
	   }
     }
   o->changed = 1;
   e->changed = 1;   
}

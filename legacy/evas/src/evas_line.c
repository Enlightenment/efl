#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static void
_evas_free_line(Evas_Object o)
{
   Evas_Object_Line oo;
   
   oo = o;
   if (o->callbacks) evas_list_free(o->callbacks);
   free(o);
}

static void
_evas_free_line_renderer_data(Evas e, Evas_Object o)
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
     default:
	break;
     }
}

Evas_Object
evas_add_line(Evas e)
{
   Evas_Object_Line oo;
   Evas_Object_Any  o;
   Evas_List        l;
   Evas_Layer       layer;

   o = oo = malloc(sizeof(struct _Evas_Object_Line));
   memset(o, 0, sizeof(struct _Evas_Object_Line));
   o->type = OBJECT_LINE;
   o->object_free = _evas_free_line;
   o->object_renderer_data_free = _evas_free_line_renderer_data;

   oo->current.x1 = 0;
   oo->current.y1 = 0;
   oo->current.x2 = 0;
   oo->current.y2 = 0;
   
   o->current.x = 0;
   o->current.y = 0;
   o->current.w = 1;
   o->current.h = 1;

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

void
evas_set_line_xy(Evas e, Evas_Object o, double x1, double y1, double x2, double y2)
{
   Evas_Object_Line oo;
   
   oo = (Evas_Object_Line)o;
   oo->current.x1 = x1;
   oo->current.y1 = y1;
   oo->current.x2 = x2;
   oo->current.y2 = y2;
   if (x1 < x2)
     {
	o->current.x = x1;
	o->current.w = (x2 - x1) + 1;
     }
   else
     {
	o->current.x = x2;
	o->current.w = (x1 - x2) + 1;
     }
   if (y1 < y2)
     {
	o->current.y = y1;
	o->current.h = (y2 - y1) + 1;
     }
   else
     {
	o->current.y = y2;
	o->current.h = (y1 - y2) + 1;
     }
   o->changed = 1;
   e->changed = 1;
}

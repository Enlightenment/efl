#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* create and destroy */
Evas
evas_new(void)
{
   Evas e;
   
   e = malloc(sizeof(struct _Evas));
   memset(e, 0, sizeof(struct _Evas));
   e->current.viewport.x = 0.0;
   e->current.viewport.y = 0.0;
   e->current.viewport.w = 0.0;
   e->current.viewport.h = 0.0;
   e->current.render_method = RENDER_METHOD_ALPHA_SOFTWARE;
   return e;
}

void
evas_free(Evas e)
{
   Evas_List l;
   
   for (l = e->layers; l; l = l->next)
     {
	/* FIXME: free layer */
     }
   if (e->layers) evas_list_free(e->layers);
   free(e);
}

void
evas_set_color(Evas e, Evas_Object o, int r, int g, int b, int a)
{
   switch (o->type)
     {
     case OBJECT_TEXT:
	  {
	     Evas_Object_Text oo;
	     
	     oo = (Evas_Object_Text)o;
	     oo->current.r = r;
	     oo->current.g = g;
	     oo->current.b = b;
	     oo->current.a = a;
	  }
	o->changed = 1;
	e->changed = 1;
	break;
     case OBJECT_RECTANGLE:
	  {
	     Evas_Object_Rectangle oo;
	     
	     oo = (Evas_Object_Rectangle)o;
	     oo->current.r = r;
	     oo->current.g = g;
	     oo->current.b = b;
	     oo->current.a = a;
	  }
	o->changed = 1;
	e->changed = 1;
	break;
     case OBJECT_LINE:
	  {
	     Evas_Object_Line oo;
	     
	     oo = (Evas_Object_Line)o;
	     oo->current.r = r;
	     oo->current.g = g;
	     oo->current.b = b;
	     oo->current.a = a;
	  }
	o->changed = 1;
	e->changed = 1;
	break;
     default:
	break;
     }
}

void
evas_set_angle(Evas e, Evas_Object o, double angle)
{
   switch (o->type)
     {
     case OBJECT_GRADIENT_BOX:
	  {
	     Evas_Object_Gradient_Box oo;
	     
	     oo = (Evas_Object_Gradient_Box)o;
	     oo->current.angle = angle;
	  }
	o->changed = 1;
	e->changed = 1;
	break;
     default:
	break;
     }
}

void
evas_set_zoom_scale(Evas e, Evas_Object o, int scale)
{
   o->current.zoomscale = scale;
   o->changed = 1;
   e->changed = 1;
}

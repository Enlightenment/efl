#include "Evas_private.h"
#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "evas_gl_routines.h"
#include "evas_imlib_routines.h"
#include "evas_image_routines.h"
#include "evas_x11_routines.h"
#include "evas_render_routines.h"

static void
_evas_free_gradient_box(Evas_Object o)
{
   Evas_Object_Gradient_Box oo;
   
   IF_OBJ(o, OBJECT_GRADIENT_BOX) return;
   oo = o;
   if (oo->current.gradient) evas_gradient_free(oo->current.gradient);
   free(o);
}

static void
_evas_free_gradient_box_renderer_data(Evas e, Evas_Object o)
{
   Evas_Object_Gradient_Box oo;
   
   oo= o;
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	if (o->renderer_data.method[e->current.render_method])
	   __evas_imlib_gradient_free(o->renderer_data.method[e->current.render_method]);
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	if (o->renderer_data.method[e->current.render_method])
	   __evas_x11_gradient_free(o->renderer_data.method[e->current.render_method]);
	break;
     case RENDER_METHOD_3D_HARDWARE:
	if (o->renderer_data.method[e->current.render_method])
	   __evas_gl_gradient_free(o->renderer_data.method[e->current.render_method]);
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	if (o->renderer_data.method[e->current.render_method])
	   __evas_render_gradient_free(o->renderer_data.method[e->current.render_method]);
	break;
     case RENDER_METHOD_IMAGE:
	if (o->renderer_data.method[e->current.render_method])
	   __evas_image_gradient_free(o->renderer_data.method[e->current.render_method]);
	break;
     default:
	break;
     }   
}

Evas_Object
evas_add_gradient_box(Evas e)
{
   Evas_Object_Gradient_Box oo;
   Evas_Object_Any          o;
   Evas_List                l;
   Evas_Layer               layer;

   if (!e) return NULL;
   o = oo = malloc(sizeof(struct _Evas_Object_Gradient_Box));
   memset(o, 0, sizeof(struct _Evas_Object_Gradient_Box));
   o->type = OBJECT_GRADIENT_BOX;
   o->object_free = _evas_free_gradient_box;
   o->object_renderer_data_free = _evas_free_gradient_box_renderer_data;

   o->current.x = 0;
   o->current.y = 0;
   o->current.w = 1;
   o->current.h = 1;
   
   oo->current.angle = 0.0;
   
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
evas_set_gradient(Evas e, Evas_Object o, Evas_Gradient grad)
{
   Evas_Object_Gradient_Box oo;

   if (!e) return;
   if (!o) return;
   if (!grad) return;
   IF_OBJ(o, OBJECT_GRADIENT_BOX) return;
   oo = o;
   grad->references++;
   if (oo->current.gradient) evas_gradient_free(oo->current.gradient);
   oo->current.gradient = grad;
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	if (o->renderer_data.method[e->current.render_method])
	   __evas_imlib_gradient_free(o->renderer_data.method[e->current.render_method]);
	  {
	     Evas_Imlib_Graident *g;
	     Evas_List l;
	     
	     g = __evas_imlib_gradient_new(e->current.display);
	     o->renderer_data.method[e->current.render_method] = g;
	     for (l = grad->color_points; l; l = l->next)
	       {
		  Evas_Color_Point col;
		  
		  col= l->data;
		  __evas_imlib_gradient_color_add(g, col->r, col->g, col->b, 
						  col->a, col->distance);
	       }
	  }
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	if (o->renderer_data.method[e->current.render_method])
	   __evas_x11_gradient_free(o->renderer_data.method[e->current.render_method]);
	  {
	     Evas_X11_Graident *g;
	     Evas_List l;
	     
	     g = __evas_x11_gradient_new(e->current.display);
	     o->renderer_data.method[e->current.render_method] = g;
	     for (l = grad->color_points; l; l = l->next)
	       {
		  Evas_Color_Point col;
		  
		  col= l->data;
		  __evas_x11_gradient_color_add(g, col->r, col->g, col->b, 
						  col->a, col->distance);
	       }
	  }
	break;
     case RENDER_METHOD_3D_HARDWARE:
	if (o->renderer_data.method[e->current.render_method])
	   __evas_gl_gradient_free(o->renderer_data.method[e->current.render_method]);
	  {
	     Evas_GL_Graident *g;
	     Evas_List l;
	     
	     g = __evas_gl_gradient_new(e->current.display);
	     o->renderer_data.method[e->current.render_method] = g;
	     for (l = grad->color_points; l; l = l->next)
	       {
		  Evas_Color_Point col;
		  
		  col= l->data;
		  __evas_gl_gradient_color_add(g, col->r, col->g, col->b, 
					       col->a, col->distance);
	       }
	  }
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	if (o->renderer_data.method[e->current.render_method])
	   __evas_render_gradient_free(o->renderer_data.method[e->current.render_method]);
	  {
	     Evas_Image_Graident *g;
	     Evas_List l;
	     
	     g = __evas_render_gradient_new(e->current.display);
	     o->renderer_data.method[e->current.render_method] = g;
	     for (l = grad->color_points; l; l = l->next)
	       {
		  Evas_Color_Point col;
		  
		  col= l->data;
		  __evas_render_gradient_color_add(g, col->r, col->g, col->b, 
						   col->a, col->distance);
	       }
	  }
	break;
     case RENDER_METHOD_IMAGE:
	if (o->renderer_data.method[e->current.render_method])
	   __evas_image_gradient_free(o->renderer_data.method[e->current.render_method]);
	  {
	     Evas_Image_Graident *g;
	     Evas_List l;
	     
	     g = __evas_image_gradient_new(e->current.display);
	     o->renderer_data.method[e->current.render_method] = g;
	     for (l = grad->color_points; l; l = l->next)
	       {
		  Evas_Color_Point col;
		  
		  col= l->data;
		  __evas_image_gradient_color_add(g, col->r, col->g, col->b, 
						  col->a, col->distance);
	       }
	  }
	break;
     default:
	break;
     }
   oo->current.new_gradient = 1;
   o->changed = 1;
   e->changed = 1;
}

/* gradient creating / deletion / modification */
Evas_Gradient
evas_gradient_new(void)
{
   Evas_Gradient gr;
   
   gr = malloc(sizeof(struct _Evas_Gradient));
   gr->color_points = NULL;
   gr->references = 1;
   return gr;
}

void
evas_gradient_free(Evas_Gradient grad)
{
   Evas_List l;
   
   if (!grad) return;
   grad->references--;
   if (grad->references > 0) return;
   if (grad->color_points)
     {
	for (l = grad->color_points; l; l = l->next)
	   free(l->data);
	evas_list_free(grad->color_points);
     }
   free(grad);
}

void
evas_gradient_add_color(Evas_Gradient grad, int r, int g, int b, int a, int dist)
{
   Evas_Color_Point col;
   
   if (!grad) return;
   col = malloc(sizeof(struct _Evas_Color_Point));
   col->r = r;
   col->g = g;
   col->b = b;
   col->a = a;
   col->distance = dist;
   grad->color_points = evas_list_append(grad->color_points, col);
}

void
evas_set_angle(Evas e, Evas_Object o, double angle)
{
   if (!e) return;
   if (!o) return;
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

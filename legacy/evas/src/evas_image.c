#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static void
_evas_free_image(Evas_Object o)
{
   Evas_Object_Image oo;
   
   oo = o;
   if (oo->current.file) free(oo->current.file);
   free(o);
}

static void
_evas_free_image_renderer_data(Evas e, Evas_Object o)
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

/* adding objects */
Evas_Object
evas_add_image_from_file(Evas e, char *file)
{
   Evas_Object_Image oo;
   Evas_Object_Any   o;
   Evas_List         l;
   Evas_Layer        layer;
   
   o = oo = malloc(sizeof(struct _Evas_Object_Image));
   memset(o, 0, sizeof(struct _Evas_Object_Image));
   o->type = OBJECT_IMAGE;
   o->object_free = _evas_free_image;
   o->object_renderer_data_free = _evas_free_image_renderer_data;

   oo->current.file = strdup(file);
     {
	Imlib_Image im;
	
	im = imlib_load_image(file);
	if (im)
	  {
	     imlib_context_set_image(im);
	     oo->current.image.w = imlib_image_get_width();
	     oo->current.image.h = imlib_image_get_height();
	     imlib_free_image();
	  }
     }
   oo->current.fill.x = 0;
   oo->current.fill.y = 0;
   oo->current.fill.w = (double)oo->current.image.w;
   oo->current.fill.h = (double)oo->current.image.h;
   o->current.x = 0;
   o->current.y = 0;
   o->current.w = (double)oo->current.image.w;
   o->current.h = (double)oo->current.image.h;
	
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

Evas_Object
evas_add_image_from_data(Evas e, void *data, Evas_Image_Format format, int w, int h)
{
   /* FIXME: not implimented */
   return NULL;
}

/* set object settings */
void
evas_set_image_file(Evas e, Evas_Object o, char *file)
{
   Evas_Object_Image oo;
   
   oo = o;
   if (oo->current.file)
      free(oo->current.file);
   oo->previous.file = NULL;
   oo->current.file = strdup(file);
     {
	Imlib_Image im;
	
	im = imlib_load_image(file);
	if (im)
	  {
	     imlib_context_set_image(im);
	     oo->current.image.w = imlib_image_get_width();
	     oo->current.image.h = imlib_image_get_height();
	     imlib_free_image();
	     evas_resize(e, o, 
			 (double)oo->current.image.w,
			 (double)oo->current.image.h);
	     oo->current.fill.x = 0;
	     oo->current.fill.y = 0;
	     oo->current.fill.w = (double)oo->current.image.w;
	     oo->current.fill.h = (double)oo->current.image.h;
	  }
	else
	  {
	     oo->current.image.w = 0;
	     oo->current.image.h = 0;
	     evas_resize(e, o, 
			 (double)oo->current.image.w,
			 (double)oo->current.image.h);
	     oo->current.fill.x = 0;
	     oo->current.fill.y = 0;
	     oo->current.fill.w = (double)oo->current.image.w;
	     oo->current.fill.h = (double)oo->current.image.h;
	  }
     }
   o->changed = 1;
   e->changed = 1;
}

void
evas_set_image_data(Evas e, Evas_Object o, void *data, Evas_Image_Format format, int w, int h)
{
   /* FIXME: not implimented */
}

void
evas_set_image_fill(Evas e, Evas_Object o, double x, double y, double w, double h)
{
   Evas_Object_Image oo;
   
   oo = o;
   oo->current.fill.x = x;
   oo->current.fill.y = y;
   oo->current.fill.w = w;
   oo->current.fill.h = h;
   o->changed = 1;
   e->changed = 1;
}

/* image query ops */
void
evas_get_image_size(Evas e, Evas_Object o, int *w, int *h)
{
   Evas_Object_Image oo;
   
   oo = o;
   if (w) *w = oo->current.image.w;
   if (h) *h = oo->current.image.h;
}

void
evas_set_image_border(Evas e, Evas_Object o, int l, int r, int t, int b)
{
   Evas_Object_Image oo;
   
   oo = o;
   oo->current.border.l = l;
   oo->current.border.r = r;
   oo->current.border.t = t;
   oo->current.border.b = b;
   o->changed = 1;
   e->changed = 1;   
}

void
evas_get_image_border(Evas e, Evas_Object o, int *l, int *r, int *t, int *b)
{
   Evas_Object_Image oo;
   
   oo = o;
   if (l) *l = oo->current.border.l;
   if (r) *r = oo->current.border.r;
   if (t) *t = oo->current.border.t;
   if (b) *b = oo->current.border.b;
}

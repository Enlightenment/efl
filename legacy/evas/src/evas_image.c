#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static void
_evas_free_image(Evas_Object o)
{
}

static void
_evas_free_image_renderer_data(Evas e, Evas_Object o)
{
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
	     o->current.x = 0;
	     o->current.y = 0;
	     o->current.w = (double)oo->current.image.w;
	     o->current.h = (double)oo->current.image.h;
	     imlib_free_image();
	  }
     }
	
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
}

/* set object settings */
void
evas_set_image_file(Evas e, Evas_Object o, char *file)
{
}

void
evas_set_image_data(Evas e, Evas_Object o, void *data, Evas_Image_Format format, int w, int h)
{
}

void
evas_set_image_scale_smoothness(Evas e, Evas_Object o, int smooth)
{
}

void
evas_set_image_fill(Evas e, Evas_Object o, double x, double y, double w, double h)
{
}

/* image query ops */
void
evas_get_image_size(Evas e, Evas_Object o, int *w, int *h)
{
}

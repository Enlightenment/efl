#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "evas_gl_routines.h"
#include "evas_imlib_routines.h"

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
   e->current.render_method = RENDER_METHOD_3D_HARDWARE;
   return e;
}

void
evas_free(Evas e)
{
   Evas_List l;
   
   for (l = e->layers; l; l = l->next)
     {
	Evas_Layer layer;
	
	layer = l->data;
	_evas_layer_free(layer);
     }
   if (e->layers) evas_list_free(e->layers);
   if (e->updates) imlib_updates_free(e->updates);
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
evas_get_color(Evas e, Evas_Object o, int *r, int *g, int *b, int *a)
{
   switch (o->type)
     {
     case OBJECT_TEXT:
	  {
	     Evas_Object_Text oo;
	     
	     oo = (Evas_Object_Text)o;
	     *r = oo->current.r;
	     *g = oo->current.g;
	     *b = oo->current.b;
	     *a = oo->current.a;
	  }
	break;
     case OBJECT_RECTANGLE:
	  {
	     Evas_Object_Rectangle oo;
	     
	     oo = (Evas_Object_Rectangle)o;
	     *r = oo->current.r;
	     *g = oo->current.g;
	     *b = oo->current.b;
	     *a = oo->current.a;
	  }
	break;
     case OBJECT_LINE:
	  {
	     Evas_Object_Line oo;
	     
	     oo = (Evas_Object_Line)o;
	     *r = oo->current.r;
	     *g = oo->current.g;
	     *b = oo->current.b;
	     *a = oo->current.a;
	  }
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

void
evas_set_font_cache(Evas e, int size)
{
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	__evas_imlib_text_cache_set_size(e->current.display, size);
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	break;
     case RENDER_METHOD_3D_HARDWARE:
	__evas_gl_text_cache_set_size(e->current.display, size);
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     default:
	return;
	break;
     }
}

int
evas_get_font_cache(Evas e)
{
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	return __evas_imlib_text_cache_get_size(e->current.display);
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	break;
     case RENDER_METHOD_3D_HARDWARE:
	return __evas_gl_text_cache_get_size(e->current.display);
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     default:
	return;
	break;
     }
   return 0;
}

void
evas_flush_font_cache(Evas e)
{
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	__evas_imlib_text_cache_empty(e->current.display);
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	break;
     case RENDER_METHOD_3D_HARDWARE:
	__evas_gl_text_cache_empty(e->current.display);
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     default:
	return;
	break;
     }
}

void
evas_set_image_cache(Evas e, int size)
{
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	__evas_imlib_image_cache_set_size(e->current.display, size);
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	break;
     case RENDER_METHOD_3D_HARDWARE:
	__evas_gl_image_cache_set_size(e->current.display, size);
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     default:
	return;
	break;
     }
}

int
evas_get_image_cache(Evas e)
{
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	return __evas_imlib_image_cache_get_size(e->current.display);
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	break;
     case RENDER_METHOD_3D_HARDWARE:
	return __evas_gl_image_cache_get_size(e->current.display);
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     default:
	return;
	break;
     }
   return 0;
}

void
evas_flush_image_cache(Evas e)
{
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	__evas_imlib_image_cache_empty(e->current.display);
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	break;
     case RENDER_METHOD_3D_HARDWARE:
	__evas_gl_image_cache_empty(e->current.display);
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     default:
	return;
	break;
     }
}

void
evas_font_add_path(Evas e, char *path)
{
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	__evas_imlib_text_font_add_path(path);
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	break;
     case RENDER_METHOD_3D_HARDWARE:
	__evas_gl_text_font_add_path(path);
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     default:
	return;
	break;
     }
}

void
evas_font_del_path(Evas e, char *path)
{
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	__evas_imlib_text_font_del_path(path);
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	break;
     case RENDER_METHOD_3D_HARDWARE:
	__evas_gl_text_font_del_path(path);
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     default:
	return;
	break;
     }
}

void
evas_put_data(Evas e, Evas_Object o, char *key, void *data)
{
   Evas_Data d;
   
   if (!key) return;
   d = malloc(sizeof(struct _Evas_Data));
   d->key = strdup(key);
   d->data = data;
   o->data = evas_list_append(o->data, d);
}

void *
evas_get_data(Evas e, Evas_Object o, char *key)
{
   Evas_List l;
   
   for (l = o->data; l; l = l->next)
     {
	Evas_Data d;
	
	d = l->data;
	if (!strcmp(d->key, key)) return d->data;
     }
   return NULL;
}

int
evas_world_x_to_screen(Evas e, double x)
{
   return (int)((x - e->current.viewport.x) *
		((double)e->current.drawable_width / e->current.viewport.w));
}

int
evas_world_y_to_screen(Evas e, double y)
{
   return (int)((y - e->current.viewport.y) *
		((double)e->current.drawable_height / e->current.viewport.h));
}

double
evas_screen_x_to_world(Evas e, int x)
{
   return (double)((double)x * (e->current.viewport.w / (double)e->current.drawable_width));
   + e->current.viewport.x;
}

double
evas_screen_y_to_world(Evas e, int y)
{
   return (double)((double)y * (e->current.viewport.h / (double)e->current.drawable_height));
   + e->current.viewport.y;
}

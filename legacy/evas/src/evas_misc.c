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

void _evas_layer_free(Evas e, Evas_Layer layer);

Evas
evas_new_all(Display *display, Window parent_window, 
	     int x, int y, int w, int h, 
	     Evas_Render_Method render_method,
	     int colors, int font_cache, int image_cache,
	     char *font_dir)
{
   Evas e;
   Window window;
   XSetWindowAttributes att;
   Visual *visual;
   Colormap colormap;
   
   e = evas_new();
   e->current.created_window = 1;
   evas_set_output_method(e, render_method);
   evas_set_output_colors(e, colors);
   visual = evas_get_optimal_visual(e, display);
   colormap = evas_get_optimal_colormap(e, display);
   att.background_pixmap = None;
   att.colormap = colormap;
   att.border_pixel = 0;
   att.event_mask = 0;
   window = XCreateWindow(display,
			  parent_window,
			  x, y, w, h, 0,
			  imlib_get_visual_depth(display, visual),
			  InputOutput,
			  visual,
			  CWColormap | CWBorderPixel | CWEventMask | CWBackPixmap,
			  &att);
   if (font_dir) evas_font_add_path(e, font_dir);
   evas_set_output(e, display, window, visual, colormap);
   evas_set_output_size(e, w, h);
   evas_set_output_viewport(e, 0, 0, w, h);
   evas_set_font_cache(e, font_cache);
   evas_set_image_cache(e, image_cache);
   return e;
}

Window
evas_get_window(Evas e)
{
   if (!e) return 0;
   return e->current.drawable;
}

Display *
evas_get_display(Evas e)
{
   if (!e) return NULL;
   return e->current.display;
}

Visual *
evas_get_visual(Evas e)
{
   if (!e) return NULL;
   return e->current.visual;
}

Colormap
evas_get_colormap(Evas e)
{
   if (!e) return 0;
   return e->current.colormap;
}

int
evas_get_colors(Evas e)
{
   if (!e) return 0;
   return e->current.colors;
}

Imlib_Image
evas_get_image(Evas e)
{
   if (!e) return NULL;
   return e->current.image;
}

Evas_Render_Method
evas_get_render_method(Evas e)
{
   if (!e) return RENDER_METHOD_ALPHA_SOFTWARE;
   return e->current.render_method;
}

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
   e->current.colors = 216;
   return e;
}

static void
_evas_x_err(Display *display, XErrorEvent *ev)
{
   display = NULL;
   ev = NULL;
}

void
evas_free(Evas e)
{
   Evas_List l;

   if (!e) return;
   if ((e->current.display) && 
       (e->current.created_window) && 
       (e->current.drawable))
     {
	XErrorHandler prev_handler;
	
	prev_handler = XSetErrorHandler((XErrorHandler)_evas_x_err);
	XDestroyWindow(e->current.display, e->current.drawable);
	XSync(e->current.display, False);
	XSetErrorHandler(prev_handler);
     }
   for (l = e->layers; l; l = l->next)
     {
	Evas_Layer layer;
	
	layer = l->data;
	_evas_layer_free(e, layer);
     }
   if (e->layers) evas_list_free(e->layers);
   if (e->updates) imlib_updates_free(e->updates);
   free(e);
}

void
evas_set_color(Evas e, Evas_Object o, int r, int g, int b, int a)
{
   if (!e) return;
   if (!o) return;
   if (r < 0) r = 0;
   else if (r > 255) r = 255;
   if (g < 0) g = 0;
   else if (g > 255) g = 255;
   if (b < 0) b = 0;
   else if (b > 255) b = 255;
   if (a < 0) a = 0;
   else if (a > 255) a = 255;
   switch (o->type)
     {
     case OBJECT_IMAGE:
	  {
	     Evas_Object_Image oo;
	     
	     oo = (Evas_Object_Image)o;
	     oo->current.color.r = r;
	     oo->current.color.g = g;
	     oo->current.color.b = b;
	     oo->current.color.a = a;
	  }
	o->changed = 1;
	e->changed = 1;
	break;
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
     case OBJECT_POLYGON:
	  {
	     Evas_Object_Poly oo;
	     
	     oo = (Evas_Object_Poly)o;
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
   if (!e) return;
   if (!o) return;
   switch (o->type)
     {
     case OBJECT_IMAGE:
	  {
	     Evas_Object_Image oo;
	     
	     oo = (Evas_Object_Image)o;
	     if (r) *r = oo->current.color.r;
	     if (g) *g = oo->current.color.g;
	     if (b) *b = oo->current.color.b;
	     if (a) *a = oo->current.color.a;
	  }
	break;
     case OBJECT_TEXT:
	  {
	     Evas_Object_Text oo;
	     
	     oo = (Evas_Object_Text)o;
	     if (r) *r = oo->current.r;
	     if (g) *g = oo->current.g;
	     if (b) *b = oo->current.b;
	     if (a) *a = oo->current.a;
	  }
	break;
     case OBJECT_RECTANGLE:
	  {
	     Evas_Object_Rectangle oo;
	     
	     oo = (Evas_Object_Rectangle)o;
	     if (r) *r = oo->current.r;
	     if (g) *g = oo->current.g;
	     if (b) *b = oo->current.b;
	     if (a) *a = oo->current.a;
	  }
	break;
     case OBJECT_LINE:
	  {
	     Evas_Object_Line oo;
	     
	     oo = (Evas_Object_Line)o;
	     if (r) *r = oo->current.r;
	     if (g) *g = oo->current.g;
	     if (b) *b = oo->current.b;
	     if (a) *a = oo->current.a;
	  }
	break;
     default:
	break;
     }
}

void
evas_set_zoom_scale(Evas e, Evas_Object o, int scale)
{
   if (!e) return;
   if (!o) return;
   o->current.zoomscale = scale;
   o->changed = 1;
   e->changed = 1;
}

void
evas_set_pass_events(Evas e, Evas_Object o, int pass_events)
{
   if (!e) return;
   if (!o) return;
   o->pass_events = 1;
}

void
evas_set_font_cache(Evas e, int size)
{
   if (!e) return;
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	__evas_imlib_text_cache_set_size(e->current.display, size);
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	__evas_x11_text_cache_set_size(e->current.display, size);
	break;
     case RENDER_METHOD_3D_HARDWARE:
	__evas_gl_text_cache_set_size(e->current.display, size);
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     case RENDER_METHOD_IMAGE:
	__evas_image_text_cache_set_size(e->current.display, size);
	break;
     default:
	return;
	break;
     }
}

int
evas_get_font_cache(Evas e)
{
   if (!e) return 0;
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	return __evas_imlib_text_cache_get_size(e->current.display);
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	return __evas_x11_text_cache_get_size(e->current.display);
	break;
     case RENDER_METHOD_3D_HARDWARE:
	return __evas_gl_text_cache_get_size(e->current.display);
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     case RENDER_METHOD_IMAGE:
	return __evas_image_text_cache_get_size(e->current.display);
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
   if (!e) return;
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	__evas_imlib_text_cache_empty(e->current.display);
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	__evas_x11_text_cache_empty(e->current.display);
	break;
     case RENDER_METHOD_3D_HARDWARE:
	__evas_gl_text_cache_empty(e->current.display);
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     case RENDER_METHOD_IMAGE:
	__evas_image_text_cache_empty(e->current.display);
	break;
     default:
	return;
	break;
     }
}

void
evas_set_image_cache(Evas e, int size)
{
   if (!e) return;
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	__evas_imlib_image_cache_set_size(e->current.display, size);
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	__evas_x11_image_cache_set_size(e->current.display, size);
	break;
     case RENDER_METHOD_3D_HARDWARE:
	__evas_gl_image_cache_set_size(e->current.display, size);
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     case RENDER_METHOD_IMAGE:
	__evas_image_image_cache_set_size(e->current.display, size);
	break;
     default:
	return;
	break;
     }
}

int
evas_get_image_cache(Evas e)
{
   if (!e) return 0;
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	return __evas_imlib_image_cache_get_size(e->current.display);
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	return __evas_x11_image_cache_get_size(e->current.display);
	break;
     case RENDER_METHOD_3D_HARDWARE:
	return __evas_gl_image_cache_get_size(e->current.display);
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     case RENDER_METHOD_IMAGE:
	return __evas_image_image_cache_get_size(e->current.display);
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
   if (!e) return;
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	__evas_imlib_image_cache_empty(e->current.display);
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	__evas_x11_image_cache_empty(e->current.display);
	break;
     case RENDER_METHOD_3D_HARDWARE:
	__evas_gl_image_cache_empty(e->current.display);
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     case RENDER_METHOD_IMAGE:
	__evas_image_image_cache_empty(e->current.display);
	break;
     default:
	return;
	break;
     }
}

void
evas_font_add_path(Evas e, char *path)
{
   if (!e) return;
   if (!path) return;
   evas_font_del_path(e, path);
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	__evas_imlib_text_font_add_path(path);
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	__evas_x11_text_font_add_path(path);
	break;
     case RENDER_METHOD_3D_HARDWARE:
	__evas_gl_text_font_add_path(path);
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     case RENDER_METHOD_IMAGE:
	__evas_image_text_font_add_path(path);
	break;
     default:
	return;
	break;
     }
}

void
evas_font_del_path(Evas e, char *path)
{
   if (!e) return;
   if (!path) return;
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	__evas_imlib_text_font_del_path(path);
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	__evas_x11_text_font_del_path(path);
	break;
     case RENDER_METHOD_3D_HARDWARE:
	__evas_gl_text_font_del_path(path);
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     case RENDER_METHOD_IMAGE:
	__evas_image_text_font_del_path(path);
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
   Evas_List l;
   
   if (!e) return;
   if (!o) return;
   if (!key) return;
   for (l = o->data; l; l = l->next)
     {
	Evas_Data d;
	
	d = l->data;
	if (!strcmp(d->key, key)) 
	   {
	      d->data = data;
	      return;
	   }
     }
   d = malloc(sizeof(struct _Evas_Data));
   d->key = strdup(key);
   d->data = data;
   o->data = evas_list_prepend(o->data, d);
}

void *
evas_get_data(Evas e, Evas_Object o, char *key)
{
   Evas_List l;
   
   if (!e) return NULL;
   if (!o) return NULL;
   if (!key) return NULL;
   for (l = o->data; l; l = l->next)
     {
	Evas_Data d;
	
	d = l->data;
	if (!strcmp(d->key, key)) return d->data;
     }
   return NULL;
}

void *
evas_remove_data(Evas e, Evas_Object o, char *key)
{
   Evas_List l;
   
   if (!e) return NULL;
   if (!o) return NULL;
   if (!key) return NULL;
   for (l = o->data; l; l = l->next)
     {
	Evas_Data d;
	
	d = l->data;
	if (!strcmp(d->key, key))
	  {
	     void *data;
	     
	     o->data = evas_list_remove(o->data, l->data);
	     data = d->data;
	     free(d->key);
	     free(d);
	     return data;
	  }
     }
   return NULL;
}

int
evas_world_x_to_screen(Evas e, double x)
{
   if (!e) return 0;
   return (int)((x - e->current.viewport.x) *
		((double)e->current.drawable_width / e->current.viewport.w));
}

int
evas_world_y_to_screen(Evas e, double y)
{
   if (!e) return 0;
   return (int)((y - e->current.viewport.y) *
		((double)e->current.drawable_height / e->current.viewport.h));
}

double
evas_screen_x_to_world(Evas e, int x)
{
   if (!e) return 0;
   return (double)((double)x * (e->current.viewport.w / (double)e->current.drawable_width));
   + e->current.viewport.x;
}

double
evas_screen_y_to_world(Evas e, int y)
{
   if (!e) return 0;
   return (double)((double)y * (e->current.viewport.h / (double)e->current.drawable_height));
   + e->current.viewport.y;
}

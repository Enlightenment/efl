#include "Evas_private.h"
#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static void
_evas_free_image(Evas_Object o)
{
   Evas_Object_Image oo;
   
   IF_OBJ(o, OBJECT_IMAGE) return;
   oo = o;
   if (oo->current.file) _evas_remove_fileless_image(oo->current.file);
   if (oo->current.file) free(oo->current.file);
   free(o);
}

static void
_evas_free_image_renderer_data(Evas e, Evas_Object o)
{
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	if (o->renderer_data.method[e->current.render_method])
	  __evas_imlib_image_free((void *)o->renderer_data.method[e->current.render_method]);
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	if (o->renderer_data.method[e->current.render_method])
	  __evas_x11_image_free((void *)o->renderer_data.method[e->current.render_method]);
	break;
     case RENDER_METHOD_3D_HARDWARE:
	if (o->renderer_data.method[e->current.render_method])
	  __evas_gl_image_free((void *)o->renderer_data.method[e->current.render_method]);
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	if (o->renderer_data.method[e->current.render_method])
	  __evas_render_image_free((void *)o->renderer_data.method[e->current.render_method]);
	break;
     case RENDER_METHOD_IMAGE:
	if (o->renderer_data.method[e->current.render_method])
	  __evas_image_image_free((void *)o->renderer_data.method[e->current.render_method]);
	break;
     default:
	break;
     }
   o->renderer_data.method[e->current.render_method] = NULL;
}

/* adding objects */
Evas_Object
evas_add_image_from_file(Evas e, const char *file)
{
   Evas_Object_Image oo;
   Evas_Object_Any   o;
   Evas_List         l;
   Evas_Layer        layer;

   if (!e) return NULL;
   o = oo = malloc(sizeof(struct _Evas_Object_Image));
   memset(o, 0, sizeof(struct _Evas_Object_Image));
   o->type = OBJECT_IMAGE;
   o->object_free = _evas_free_image;
   o->object_renderer_data_free = _evas_free_image_renderer_data;

   oo->load_error = IMLIB_LOAD_ERROR_NONE;
   
   if (file)
     {
	char *tmp_name, *name, *dot;
	
	oo->current.file = malloc(strlen(file) + 1);
	strcpy(oo->current.file, file);
	
	name = strrchr(oo->current.file, '/');
	if (!name) name = oo->current.file;
	else name++;
	
	tmp_name = malloc(strlen(name) + 1);
	strcpy(tmp_name, name);	
	dot = strchr(tmp_name, '.');
	if (dot) *dot = '\0';
	evas_object_set_name(e, o, tmp_name);
	if (dot) *dot = '.';
	free(tmp_name);
	
	  {
	     Imlib_Image im;
	     
	     im = imlib_load_image_with_error_return(file, &(oo->load_error));
	     if (im)
	       {
		  imlib_context_set_image(im);
		  oo->current.image.w = imlib_image_get_width();
		  oo->current.image.h = imlib_image_get_height();
		  oo->current.alpha = imlib_image_has_alpha();
		  imlib_free_image();
	       }
	     else
	       {
		  oo->current.alpha = 1;		  
	       }
	  }
     }
   oo->current.fill.x = 0;
   oo->current.fill.y = 0;
   oo->current.fill.w = (double)oo->current.image.w;
   oo->current.fill.h = (double)oo->current.image.h;
   oo->current.color.r = 255;
   oo->current.color.g = 255;
   oo->current.color.b = 255;
   oo->current.color.a = 255;
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
	if (layer->layer > o->current.layer)
	  {
	     Evas_Layer        layer_new;
	     
	     layer_new = malloc(sizeof(struct _Evas_Layer));
	     memset(layer_new, 0, sizeof(struct _Evas_Layer));
	     e->layers = evas_list_prepend_relative(e->layers, layer_new, layer);
	     layer_new->objects = evas_list_append(layer_new->objects, o);
	     layer_new->layer = o->current.layer;
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
   Evas_Object_Image oo;
   Evas_Object_Any   o;
   Evas_List         l;
   Evas_Layer        layer;

   if (!e) return NULL;
   o = oo = malloc(sizeof(struct _Evas_Object_Image));
   memset(o, 0, sizeof(struct _Evas_Object_Image));
   o->type = OBJECT_IMAGE;
   o->object_free = _evas_free_image;
   o->object_renderer_data_free = _evas_free_image_renderer_data;

   oo->load_error = IMLIB_LOAD_ERROR_NONE;
   
   if (data)
     {
	oo->current.file = _evas_generate_filename();
	  {
	     Imlib_Image im;
	     
	     im = imlib_create_image(w,h);
	     if (im)
	       {
		   DATA32 *p;
		   unsigned char *p1, *p2;
		   int i, j, k;

		   imlib_context_set_image(im);
		   if((format==IMAGE_FORMAT_ARGB)||(format==IMAGE_FORMAT_ARGB))
		       imlib_image_set_has_alpha(1);
		   imlib_image_clear();
		   p = imlib_image_get_data();
		   p2 = (unsigned char*)p;
		   p1 = (unsigned char*)data;
		   
		   if(format==IMAGE_FORMAT_ARGB)memcpy(p, data, w*h*4);
		   
		   for(i=0; i<w*h;i++){
		       switch(format){
			       case IMAGE_FORMAT_BGRA: 
				       	j = i*4;
				       	p2[j+3]=p1[j];
				       	p2[j+2]=p1[j+1];
				       	p2[j+1]=p1[j+2];
				       	p2[j]=p1[j+3];
				       	break;
			       case IMAGE_FORMAT_ARGB: break;
			       case IMAGE_FORMAT_RGB: 
					j = i*4;
					k = i*3;
					p2[j+3] = 255;
					p2[j+2] = p1[k+2];
					p2[j+1] = p1[k+1];
					p2[j] = p1[k];
					break;
			       case IMAGE_FORMAT_GRAY:
					j = i*4;
					p2[j+3] = 255;
					p2[j+2] = p1[i];
					p2[j+1] = p1[i];
					p2[j] = p1[i];
					break;
			       default:;
					       
		       }
		       
		   }
		   imlib_image_put_back_data(p);
		   _evas_add_fileless_image(im,oo->current.file);

		  oo->current.image.w = imlib_image_get_width();
		  oo->current.image.h = imlib_image_get_height();
		  oo->current.alpha = imlib_image_has_alpha();
		  imlib_free_image();
	       }
	     else
	       {
		  oo->current.alpha = 1;		  
	       }
	  }
     }
   oo->current.fill.x = 0;
   oo->current.fill.y = 0;
   oo->current.fill.w = (double)oo->current.image.w;
   oo->current.fill.h = (double)oo->current.image.h;
   oo->current.color.r = 255;
   oo->current.color.g = 255;
   oo->current.color.b = 255;
   oo->current.color.a = 255;
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
	if (layer->layer > o->current.layer)
	  {
	     Evas_Layer        layer_new;
	     
	     layer_new = malloc(sizeof(struct _Evas_Layer));
	     memset(layer_new, 0, sizeof(struct _Evas_Layer));
	     e->layers = evas_list_prepend_relative(e->layers, layer_new, layer);
	     layer_new->objects = evas_list_append(layer_new->objects, o);
	     layer_new->layer = o->current.layer;
	     return o;
	  }
     }
   
   layer = malloc(sizeof(struct _Evas_Layer));
   memset(layer, 0, sizeof(struct _Evas_Layer));
   e->layers = evas_list_append(e->layers, layer);
   layer->objects = evas_list_append(layer->objects, o);
   
   return o; 
}

/* set object settings */
void
evas_set_image_file(Evas e, Evas_Object o, const char *file)
{
   Evas_Object_Image oo;
   
   if (!e) return;
   o = TO_OBJECT(e, o);
   if (!o) return;
   IF_OBJ(o, OBJECT_IMAGE) return;
   oo = o;
   if (((oo->current.file) && (file) && (strcmp(file, oo->current.file))) ||
       ((!oo->current.file) && (file)) ||
       (oo->current.image.w == 0) ||
       (oo->current.image.h == 0)
       )
     {
	char *tmp_name, *name, *dot;
	
	_evas_free_image_renderer_data(e, o);
	if (oo->current.file)
	   free(oo->current.file);
	oo->previous.file = NULL;
	oo->current.file = malloc(strlen(file) + 1);
	strcpy(oo->current.file, file);
	
	name = strrchr(oo->current.file, '/');
	if (!name) name = oo->current.file;
	else name++;
	
	tmp_name = malloc(strlen(name) + 1);
	strcpy(tmp_name, name);	
	dot = strchr(tmp_name, '.');
	if (dot) *dot = '\0';
	evas_object_set_name(e, o, tmp_name);
	if (dot) *dot = '.';
	free(tmp_name);
	
	  {
	     Imlib_Image im;
	     
	     im = imlib_load_image_with_error_return(file, &oo->load_error);
	     if (im)
	       {
		  imlib_context_set_image(im);
		  oo->current.image.w = imlib_image_get_width();
		  oo->current.image.h = imlib_image_get_height();
		  oo->current.alpha = imlib_image_has_alpha();
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
		  oo->current.alpha = 1;		  
	       }
	  }
	o->changed = 1;
	e->changed = 1;
     }
   else if (!file)
     {
        if (oo->current.file)
	  free(oo->current.file);
	oo->previous.file = NULL;
	oo->current.file = NULL;
	oo->current.image.w = 0;
	oo->current.image.h = 0;
	oo->current.alpha = 1;
	evas_resize(e, o, 
		    (double)oo->current.image.w,
		    (double)oo->current.image.h);
	oo->current.fill.x = 0;
	oo->current.fill.y = 0;
	oo->current.fill.w = (double)oo->current.image.w;
	oo->current.fill.h = (double)oo->current.image.h;
	oo->load_error = IMLIB_LOAD_ERROR_NONE;
	o->changed = 1;
	e->changed = 1;
     }
   else
     {
     }
}

void
evas_set_image_data(Evas e, Evas_Object o, void *data, Evas_Image_Format format, int w, int h)
{
   Evas_Object_Image oo;
   
   if (!e) return;
   if (!o) return;
   IF_OBJ(o, OBJECT_IMAGE) return;
   oo = o;
   if (data)
     {
	_evas_free_image_renderer_data(e, o);
	if (oo->current.file) _evas_remove_fileless_image(oo->current.file);
	if (oo->current.file) free(oo->current.file);
	
	oo->previous.file = NULL;
	oo->current.file = _evas_generate_filename();
	  {
	     Imlib_Image im;
	     
	     im = imlib_create_image(w,h);
	     if (im)
	       {
		  DATA32 *p;
		  unsigned char *p1, *p2;
		  int i, j, k;

		  imlib_context_set_image(im);

		  if((format==IMAGE_FORMAT_ARGB)||(format==IMAGE_FORMAT_ARGB))
		      imlib_image_set_has_alpha(1);
		  imlib_image_clear();
		  p = imlib_image_get_data();
		  p2 = (unsigned char*)p;
		  p1 = (unsigned char*)data;
		   
		  if(format==IMAGE_FORMAT_ARGB)memcpy(p, data, w*h*4);
		   
		  for(i=0; i<w*h;i++){
		       switch(format){
			       case IMAGE_FORMAT_BGRA: 
				       	j = i*4;
				       	p2[j+3]=p1[j];
				       	p2[j+2]=p1[j+1];
				       	p2[j+1]=p1[j+2];
				       	p2[j]=p1[j+3];
				       	break;
			       case IMAGE_FORMAT_ARGB: break;
			       case IMAGE_FORMAT_RGB: 
					j = i*4;
					k = i*3;
					p2[j+3] = 255;
					p2[j+2] = p1[k+2];
					p2[j+1] = p1[k+1];
					p2[j] = p1[k];
					break;
			       case IMAGE_FORMAT_GRAY:
					j = i*4;
					p2[j+3] = 255;
					p2[j+2] = p1[i];
					p2[j+1] = p1[i];
					p2[j] = p1[i];
					break;
			       default:;
					       
		      }
		       
		  }
		  imlib_image_put_back_data(p);
		  _evas_add_fileless_image(im,oo->current.file);

		  
		  oo->current.image.w = imlib_image_get_width();
		  oo->current.image.h = imlib_image_get_height();
		  oo->current.alpha = imlib_image_has_alpha();
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
		  oo->current.alpha = 1;		  
	       }
	  }
	o->changed = 1;
	e->changed = 1;
     }
   else if (!data)
     {
        if (oo->current.file)
	  free(oo->current.file);
	oo->previous.file = NULL;
	oo->current.file = NULL;
	oo->current.image.w = 0;
	oo->current.image.h = 0;
	oo->current.alpha = 1;
	evas_resize(e, o, 
		    (double)oo->current.image.w,
		    (double)oo->current.image.h);
	oo->current.fill.x = 0;
	oo->current.fill.y = 0;
	oo->current.fill.w = (double)oo->current.image.w;
	oo->current.fill.h = (double)oo->current.image.h;
	oo->load_error = IMLIB_LOAD_ERROR_NONE;
	o->changed = 1;
	e->changed = 1;
     }
   else
     {
     }
}

void
evas_set_image_fill(Evas e, Evas_Object o, double x, double y, double w, double h)
{
   Evas_Object_Image oo;
   
   if (!e) return;
   o = TO_OBJECT(e, o);
   if (!o) return;
   IF_OBJ(o, OBJECT_IMAGE) return;
   oo = o;
   oo->current.fill.x = x;
   oo->current.fill.y = y;
   oo->current.fill.w = w;
   oo->current.fill.h = h;
   o->changed = 1;
   e->changed = 1;
}

/* image query ops */
int
evas_get_image_alpha(Evas e, Evas_Object o)
{
   Evas_Object_Image oo;
   
   if (!e) return 0;
   if (!o) return 0;
   IF_OBJ(o, OBJECT_IMAGE) return 0;
   oo = o;
   return oo->current.alpha;
}

void
evas_get_image_size(Evas e, Evas_Object o, int *w, int *h)
{
   Evas_Object_Image oo;
   
   if (!e) return;
   o = TO_OBJECT(e, o);
   if (!o) return;
   IF_OBJ(o, OBJECT_IMAGE) return;
   oo = o;
   if (w) *w = oo->current.image.w;
   if (h) *h = oo->current.image.h;
}

void
evas_set_image_border(Evas e, Evas_Object o, int l, int r, int t, int b)
{
   Evas_Object_Image oo;
   
   if (!e) return;
   o = TO_OBJECT(e, o);
   if (!o) return;
   IF_OBJ(o, OBJECT_IMAGE) return;
   oo = o;
   if (l < 0) l = 0;
   if (r < 0) r = 0;
   if (t < 0) t = 0;
   if (b < 0) b = 0;
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
   
   if (!e) return;
   o = TO_OBJECT(e, o);
   if (!o) return;
   IF_OBJ(o, OBJECT_IMAGE) return;
   oo = o;
   if (l) *l = oo->current.border.l;
   if (r) *r = oo->current.border.r;
   if (t) *t = oo->current.border.t;
   if (b) *b = oo->current.border.b;
}

Imlib_Load_Error
evas_get_image_load_error(Evas e, Evas_Object o)
{
   Evas_Object_Image oo;
   
   if (!e) return IMLIB_LOAD_ERROR_NONE;
   if (!o) return IMLIB_LOAD_ERROR_NONE;
   IF_OBJ(o, OBJECT_IMAGE) return IMLIB_LOAD_ERROR_NONE;
   oo = o;
   
   return oo->load_error;
}

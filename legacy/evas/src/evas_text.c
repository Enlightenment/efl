#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "evas_gl_routines.h"
#include "evas_imlib_routines.h"

static void
_evas_free_text(Evas_Object o)
{
   Evas_Object_Text oo;
   
   oo = o;
   if (o->callbacks) evas_list_free(o->callbacks);
   if (oo->current.text) free(oo->current.text);
   free(o);
}

static void
_evas_free_text_renderer_data(Evas e, Evas_Object o)
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
evas_add_text(Evas e, char *font, int size, char *text)
{
   Evas_Object_Text oo;
   Evas_Object_Any   o;
   Evas_List         l;
   Evas_Layer        layer;
   
   o = oo = malloc(sizeof(struct _Evas_Object_Text));
   memset(o, 0, sizeof(struct _Evas_Object_Text));
   o->type = OBJECT_TEXT;
   o->object_free = _evas_free_text;
   o->object_renderer_data_free = _evas_free_text_renderer_data;

   oo->current.text = strdup(text);
   oo->current.font = strdup(font);
   oo->current.size = size;
   
     {
	switch (e->current.render_method)
	  {
	  case RENDER_METHOD_ALPHA_SOFTWARE:
	       {
		  Evas_Imlib_Font *fn;
		  
		  fn = __evas_imlib_text_font_new (e->current.display, 
						   oo->current.font, 
						   oo->current.size);
		  if (fn)
		    {
		       __evas_imlib_text_get_size(fn, oo->current.text, 
						  &oo->current.string.w, 
						  &oo->current.string.h);
		       __evas_imlib_text_font_free(fn);
		    }
	       }
	     break;
	  case RENDER_METHOD_BASIC_HARDWARE:
	     break;
	  case RENDER_METHOD_3D_HARDWARE:
	       {
		  Evas_GL_Font *fn;
		  
		  fn = __evas_gl_text_font_new (e->current.display, 
						oo->current.font, 
						oo->current.size);
		  if (fn)
		    {
		       __evas_gl_text_get_size(fn, oo->current.text, 
					       &oo->current.string.w, 
					       &oo->current.string.h);
		       __evas_gl_text_font_free(fn);
		    }
	       }
	     break;
	  case RENDER_METHOD_ALPHA_HARDWARE:
	     break;
	  default:
	     break;
	  }	
     }
   
   o->current.x = 0;
   o->current.y = 0;
   o->current.w = (double)oo->current.string.w;
   o->current.h = (double)oo->current.string.h;
   
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

char *
evas_get_text_string(Evas e, Evas_Object o)
{
   Evas_Object_Text oo;

   oo = o;
   return oo->current.text;
}

char *
evas_get_text_font(Evas e, Evas_Object o)
{
   Evas_Object_Text oo;

   oo = o;
   return oo->current.font;
}

int
evas_get_text_size(Evas e, Evas_Object o)
{
   Evas_Object_Text oo;

   oo = o;
   return oo->current.size;
}

int
evas_text_at_position(Evas e, Evas_Object o, double x, double y, 
		      int *char_x, int *char_y, int *char_w, int *char_h)
{
   Evas_Object_Text oo;
   
   oo = o;
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	  {
	     int ret;
	     Evas_Imlib_Font *fn;
	     
	     fn = __evas_imlib_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  ret =  __evas_imlib_text_get_character_at_pos(fn, oo->current.text,
								(int)(x - o->current.x),
								(int)(y - o->current.y),
								char_x, char_y, 
								char_w, char_h);
		  __evas_imlib_font_free(fn);
		  return ret;
	       }
	  }
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	break;
     case RENDER_METHOD_3D_HARDWARE:
	  {
	     int ret;
	     Evas_GL_Font *fn;
	     
	     fn = __evas_gl_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  ret =  __evas_gl_text_get_character_at_pos(fn, oo->current.text,
							     (int)(x - o->current.x),
							     (int)(y - o->current.y),
							     char_x, char_y, 
							     char_w, char_h);
		  __evas_gl_font_free(fn);
		  return ret;
	       }
	  }
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     default:
	break;
     }
   return 0;
}

void
evas_text_at(Evas e, Evas_Object o, int index, 
	     int *char_x, int *char_y, int *char_w, int *char_h)
{
   Evas_Object_Text oo;
   
   oo = o;
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	  {
	     Evas_Imlib_Font *fn;
	     
	     fn = __evas_imlib_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  __evas_imlib_text_get_character_number(fn, oo->current.text,
							 index,
							 char_x, char_y, 
							 char_w, char_h);
		  __evas_imlib_font_free(fn);
	       }
	  }
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	break;
     case RENDER_METHOD_3D_HARDWARE:
	  {
	     Evas_GL_Font *fn;
	     
	     fn = __evas_gl_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  __evas_gl_text_get_character_number(fn, oo->current.text,
						      index,
						      char_x, char_y, 
						      char_w, char_h);
		  __evas_gl_font_free(fn);
	       }
	  }
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     default:
	break;
     }
}

#include "Evas.h"
#include "evas_gl_routines.h"
#include "evas_imlib_routines.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static void
_evas_object_get_current_translated_coords(Evas e, Evas_Object o, 
					   int *x, int *y, int *w, int *h)
{
   *x = (int)
      (((o->current.x - e->current.viewport.x) * 
	(double)e->current.drawable_width) / 
       e->current.viewport.w);
   *y = (int)
      (((o->current.y - e->current.viewport.y) * 
	(double)e->current.drawable_height) / 
       e->current.viewport.h);
   *w = (int)
      ((o->current.w * (double)e->current.drawable_width) /
       e->current.viewport.w);
   *h = (int)
      ((o->current.h * (double)e->current.drawable_height) /
       e->current.viewport.h);
}

static void
_evas_object_get_previous_translated_coords(Evas e, Evas_Object o, 
					   int *x, int *y, int *w, int *h)
{
   *x = (int)
      (((o->previous.x - e->previous.viewport.x) * 
	(double)e->previous.drawable_width) / 
       e->previous.viewport.w);
   *y = (int)
      (((o->previous.y - e->previous.viewport.y) * 
	(double)e->previous.drawable_height) / 
       e->previous.viewport.h);
   *w = (int)
      ((o->previous.w * (double)e->previous.drawable_width) /
       e->previous.viewport.w);
   *h = (int)
      ((o->previous.h * (double)e->previous.drawable_height) /
       e->previous.viewport.h);
}

/* for exposes or forced redraws (relative to output drawable) */
void
evas_update_rect(Evas e, int x, int y, int w, int h)
{
   e->updates = imlib_update_append_rect(e->updates, x, y, w, h);
}

#if 0
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
#endif

/* drawing */
void
evas_render(Evas e)
{
   Evas_List delete_objects;
   Evas_List l, ll;
   
   if ((!e->changed) || (!e->current.display) || (!e->current.drawable))
      return;
   e->changed = 0;
   if ((e->current.viewport.x != e->previous.viewport.x) || 
       (e->current.viewport.y != e->previous.viewport.y) || 
       (e->current.viewport.w != e->previous.viewport.w) || 
       (e->current.viewport.h != e->previous.viewport.h))
      evas_update_rect(e, 
		       0, 0, 
		       e->current.drawable_width, 
		       e->current.drawable_height);
   
   delete_objects = 0;
   /* go thru layers & objects and add updates */
   for (l = e->layers; l; l = l->next)
     {
	Evas_Layer layer;
	
	layer = l->data;
	for (ll = layer->objects; ll; ll = ll->next)
	  {
	     Evas_Object_Any o;
	     int real_change;

	     real_change = 0;
	     o = ll->data;
	     if (o->delete_me) 
		delete_objects = evas_list_append(delete_objects, o);
	     if (o->changed)
	       {
		  o->changed = 0;
		  if ((o->current.visible != o->previous.visible) ||
		      ((o->current.visible) && 
		       ((o->current.x != o->previous.x) ||
			(o->current.y != o->previous.y) ||
			(o->current.w != o->previous.w) ||
			(o->current.h != o->previous.h) ||
			(o->current.zoomscale != o->previous.zoomscale) ||
			(o->current.layer != o->previous.layer)))
		      )
		     real_change = 1;
		  if (o->current.visible)
		    {
		       switch (o->type)
			 {
			 case OBJECT_IMAGE:
			      {
				 Evas_Object_Image oo;
				 
				 oo = o;
				 if (((oo->current.file) && (!oo->previous.file)) ||
				     ((!oo->current.file) && (oo->previous.file)) ||
				     (oo->current.new_data) ||
				     (oo->current.scale != oo->previous.scale) ||
				     (oo->current.fill.x != oo->previous.fill.x) ||
				     (oo->current.fill.y != oo->previous.fill.y) ||
				     (oo->current.fill.w != oo->previous.fill.w) ||
				     (oo->current.fill.h != oo->previous.fill.h)
				     )
				    real_change = 1;
				 oo->previous = oo->current;
			      }
			    break;
			 case OBJECT_TEXT:
			      {
				 Evas_Object_Text oo;
				 
				 oo = o;
				 if (1)
				    real_change = 1;
				 oo->previous = oo->current;
			      }
			    break;
			 case OBJECT_RECTANGLE:
			      {
				 Evas_Object_Rectangle oo;
				 
				 oo = o;
				 if (1)
				    real_change = 1;
				 oo->previous = oo->current;
			      }
			    break;
			 case OBJECT_LINE:
			      {
				 Evas_Object_Line oo;
				 
				 oo = o;
				 if (1)
				    real_change = 1;
				 oo->previous = oo->current;
			      }
			    break;
			 case OBJECT_GRADIENT_BOX:
			      {
				 Evas_Object_Gradient_Box oo;
				 
				 oo = o;
				 if (1)
				    real_change = 1;
				 oo->previous = oo->current;
			      }
			    break;
			 case OBJECT_BITS:
			      {
				 Evas_Object_Bits oo;
				 
				 oo = o;
				 if (1)
				    real_change = 1;
				 oo->previous = oo->current;
			      }
			    break;
			 }
		    }
	       }
	     if (real_change)
	       {
		  int x, y, w, h;
		  
		  _evas_object_get_previous_translated_coords(e, o, 
							      &x, &y, 
							      &w, &h);
		  evas_update_rect(e, x, y, w, h);
		  _evas_object_get_current_translated_coords(e, o, 
							     &x, &y, 
							     &w, &h);
		  evas_update_rect(e, x, y, w, h);
	       }
	     o->previous = o->current;
	  }
     }
   
   if (delete_objects)
     {
	for (l = delete_objects; l; l = l->next)
	   _evas_real_del_object(e, l->data);
	evas_list_free(delete_objects);
     }
   
   /* take all the update rects we've produced and render them */
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	  {
	     Imlib_Updates up;
	     
	     __evas_imlib_init(e->current.display, e->current.screen);
	     if (e->updates)
	       {
		  up = imlib_updates_merge_for_rendering(e->updates, 
							  e->current.drawable_width,
							  e->current.drawable_height);
		  e->updates = NULL;
		  if (up)
		    {
		       Imlib_Updates u;
		       
		       u = up;
		       while (u)
			 {
			    int x, y, w, h;
			    
			    imlib_updates_get_coordinates(u, &x, &y, &w, &h);
			    __evas_imlib_draw_add_rect(e->current.display, 
						       e->current.drawable,
						       x, y, w, h);
			    u = imlib_updates_get_next(u);
			 }
		       imlib_updates_free(up);
		       /* draw all objects now */
		       for (l = e->layers; l; l = l->next)
			 {
			    Evas_Layer layer;
			    
			    layer = l->data;
			    for (ll = layer->objects; ll; ll = ll->next)
			      {
				 Evas_Object_Any o;
				 
				 o = ll->data;
				 if (o->current.visible)
				   {
				      int x, y, w, h;
				      _evas_object_get_current_translated_coords(e, o, 
										 &x, &y, 
										 &w, &h);
				      switch (o->type)
					{
					case OBJECT_IMAGE:
					     {
						Evas_Object_Image oo;
						
						oo = o;
						  {
						     Evas_Imlib_Image *im;
						     
						     im = __evas_imlib_image_new_from_file(e->current.display, oo->current.file);
						     if (im)
						       {
							  __evas_imlib_image_draw(im, 
									       e->current.display,
									       e->current.drawable,
									       e->current.drawable_width,
									       e->current.drawable_height,
									       0, 0, 
									       __evas_gl_image_get_width(im),
									       __evas_gl_image_get_height(im),
									       x, y, w, h);
							  __evas_imlib_image_free(im);
						       }
						  }
					     }
					   break;
					case OBJECT_TEXT:
					     {
						Evas_Object_Text oo;
						
						oo = o;
					     }
					   break;
					case OBJECT_RECTANGLE:
					     {
						Evas_Object_Rectangle oo;
						
						oo = o;
					     }
					   break;
					case OBJECT_LINE:
					     {
						Evas_Object_Line oo;
						
						oo = o;
					     }
					   break;
					case OBJECT_GRADIENT_BOX:
					     {
						Evas_Object_Gradient_Box oo;
						
						oo = o;
					     }
					   break;
					case OBJECT_BITS:
					     {
						Evas_Object_Bits oo;
						
						oo = o;
					     }
					   break;
					}
				   }
			      }
			 }
		       __evas_imlib_flush_draw(e->current.display, 
					       e->current.drawable);
		    }
	       }
	  }
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	break;
     case RENDER_METHOD_3D_HARDWARE:
	  {
	     Imlib_Updates up;
	     
	     __evas_gl_init(e->current.display, e->current.screen);
	     if (e->updates)
	       {
		  up = imlib_updates_merge_for_rendering(e->updates, 
							 e->current.drawable_width,
							 e->current.drawable_height);
		  e->updates = NULL;
		  if (up)
		    {
		       Imlib_Updates u;
		       
		       u = up;
		       while (u)
			 {
			    int x, y, w, h;
			    
			    imlib_updates_get_coordinates(u, &x, &y, &w, &h);
			    __evas_gl_draw_add_rect(e->current.display, 
						    e->current.drawable,
						    x, y, w, h);
			    u = imlib_updates_get_next(u);
			 }
		       imlib_updates_free(up);
		       /* draw all objects now */
		       for (l = e->layers; l; l = l->next)
			 {
			    Evas_Layer layer;
			    
			    layer = l->data;
			    for (ll = layer->objects; ll; ll = ll->next)
			      {
				 Evas_Object_Any o;
				 
				 o = ll->data;
				 if (o->current.visible)
				   {
				      int x, y, w, h;
				      _evas_object_get_current_translated_coords(e, o, 
										 &x, &y, 
										 &w, &h);
				      switch (o->type)
					{
					case OBJECT_IMAGE:
					     {
						Evas_Object_Image oo;
						
						oo = o;
						  {
						     Evas_GL_Image *im;
						     
						     im = __evas_gl_image_new_from_file(e->current.display, oo->current.file);
						     if (im)
						       {
							  __evas_gl_image_draw(im, 
									       e->current.display,
									       e->current.drawable,
									       e->current.drawable_width,
									       e->current.drawable_height,
									       0, 0, 
									       __evas_gl_image_get_width(im),
									       __evas_gl_image_get_height(im),
									       x, y, w, h);
							  __evas_gl_image_free(im);
						       }
						  }
					     }
					   break;
					case OBJECT_TEXT:
					     {
						Evas_Object_Text oo;
						
						oo = o;
					     }
					   break;
					case OBJECT_RECTANGLE:
					     {
						Evas_Object_Rectangle oo;
						
						oo = o;
					     }
					   break;
					case OBJECT_LINE:
					     {
						Evas_Object_Line oo;
						
						oo = o;
					     }
					   break;
					case OBJECT_GRADIENT_BOX:
					     {
						Evas_Object_Gradient_Box oo;
						
						oo = o;
					     }
					   break;
					case OBJECT_BITS:
					     {
						Evas_Object_Bits oo;
						
						oo = o;
					     }
					   break;
					}
				   }
			      }
			 }
		       __evas_gl_flush_draw(e->current.display, 
					    e->current.drawable);
		    }
	       }
	  }
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     default:
	break;
     }
   e->previous = e->current;
}

/* query for settings to use */
Visual *
evas_get_optimal_visual(Evas e, Display *disp)
{
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	return __evas_imlib_get_visual(disp, e->current.screen);
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	break;
     case RENDER_METHOD_3D_HARDWARE:
	return __evas_gl_get_visual(disp, e->current.screen);
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     default:
	return NULL;
	break;
     }
}

Colormap
evas_get_optimal_colormap(Evas e, Display *disp)
{
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	return __evas_imlib_get_colormap(disp, e->current.screen);
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	break;
     case RENDER_METHOD_3D_HARDWARE:
	return __evas_gl_get_colormap(disp, e->current.screen);
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     default:
	return 0;
	break;
     }
}

/* the output settings */
void
evas_set_output(Evas e, Display *disp, Drawable d, Visual *v, Colormap c)
{
   e->current.display = disp;
   e->current.drawable = d;
   e->current.visual = v;
   e->current.colormap = c;
   e->changed = 1;
}

void
evas_set_output_size(Evas e, int w, int h)
{
   e->current.drawable_width = w;
   e->current.drawable_height = h;
   e->changed = 1;
}

void
evas_set_output_viewport(Evas e, double x, double y, double w, double h)
{
   e->current.viewport.x = x;
   e->current.viewport.y = y;
   e->current.viewport.w = w;
   e->current.viewport.h = h;
   e->changed = 1;
}

void
evas_set_output_method(Evas e, Evas_Render_Method method)
{
   e->current.render_method = method;
   e->changed = 1;
}

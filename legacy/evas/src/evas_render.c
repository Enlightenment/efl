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
   e->changed = 1;
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
   Imlib_Updates up;
   Evas_List delete_objects;
   Evas_List l, ll;
   void (*func_draw_add_rect) (Display *disp, Window win, int x, int y, int w, int h);
   void * (*func_image_new_from_file) (Display *disp, char *file);   
   void (*func_image_draw) (void *im, Display *disp, Window w, int win_w, int win_h, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h);   
   void (*func_image_free) (void *im);   
   void (*func_flush_draw) (Display *disp, Window w);
   void (*func_init) (Display *disp, Window w);
   int (*func_image_get_width) (void *im);
   int (*func_image_get_height) (void *im);
   
   if ((!e->changed) || 
       (!e->current.display) || 
       (!e->current.drawable) ||
       (e->current.drawable_width <= 0) || 
       (e->current.drawable_height <= 0) ||
       (e->current.viewport.w <= 0) || 
       (e->current.viewport.h <= 0))
      return;
   
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	func_draw_add_rect       = __evas_imlib_draw_add_rect;
	func_image_new_from_file = __evas_imlib_image_new_from_file;
	func_image_draw          = __evas_imlib_image_draw;
	func_image_free          = __evas_imlib_image_free;
	func_flush_draw          = __evas_imlib_flush_draw;
	func_init                = __evas_imlib_init;
	func_image_get_width     = __evas_imlib_image_get_width;
	func_image_get_height    = __evas_imlib_image_get_height;
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	break;
     case RENDER_METHOD_3D_HARDWARE:
	func_draw_add_rect       = __evas_gl_draw_add_rect;
	func_image_new_from_file = __evas_gl_image_new_from_file;
	func_image_draw          = __evas_gl_image_draw;
	func_image_free          = __evas_gl_image_free;
	func_flush_draw          = __evas_gl_flush_draw;
	func_init                = __evas_gl_init;
	func_image_get_width     = __evas_gl_image_get_width;
	func_image_get_height    = __evas_gl_image_get_height;
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     default:
	break;
     }
   if ((e->current.viewport.x != e->previous.viewport.x) || 
       (e->current.viewport.y != e->previous.viewport.y) || 
       (e->current.viewport.w != e->previous.viewport.w) || 
       (e->current.viewport.h != e->previous.viewport.h))
      evas_update_rect(e, 
		       0, 0, 
		       e->current.drawable_width, 
		       e->current.drawable_height);
   
   e->changed = 0;
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
			 default:
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
   
   func_init(e->current.display, e->current.screen);
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
		  func_draw_add_rect(e->current.display, 
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
			    if (RECTS_INTERSECT(0, 0, 
						e->current.drawable_width,
						e->current.drawable_height,
						x, y, w, h))
			      {
				 switch (o->type)
				   {
				   case OBJECT_IMAGE:
					{
					   Evas_Object_Image oo;
					   void *im;
					   
					   oo = o;
					   im = func_image_new_from_file(e->current.display, oo->current.file);
					   if (im)
					     {
						int visx, visy, visw, vish;
						int tilex, tiley, tilew, tileh;
						struct _points
						  {
						     int out1, out2;
						     int in1, in2;
						  } *pointsx, *pointsy;
						int xx, yy, ww, hh;
						
						visx = x;
						visw = w;
						if (x < 0) 
						  {
						     visx = 0;
						     visw += x;
						  }
						if ((visx + visw) > e->current.drawable_width) visw = e->current.drawable_width - visx;
						tilex = (-oo->current.fill.x * (double)e->current.drawable_width) / e->current.viewport.w;
						tilew = (oo->current.fill.w * (double)e->current.drawable_width) / e->current.viewport.w;
						if (tilew < 1) tilew = 1;
						tilex = (((tilex - (x - visx)) % tilew) + tilew) % tilew;
						if (tilex == 0) ww = 1 + (visw - 1) / tilew;
						else ww = 1 + ((visw + tilex) / tilew);
						
						pointsx = malloc(ww * sizeof(struct _points));
						for (xx = 0; xx < ww; xx++)
						  {
						     if (xx == 0)
						       {
							  if (xx == (ww - 1))  /* | * | */
							    {
							       pointsx[xx].out1 = visx + 0;
							       pointsx[xx].out2 = visw;
							       pointsx[xx].in1 = (int)(((double)tilex * (double)oo->current.image.w * e->current.viewport.w) / ((double)e->current.drawable_width * (double)tilew));
							       pointsx[xx].in2 = (int)(((double)func_image_get_width(im) * (double)visw) / (double)tilew);
							    }
							  else  /* | *  |   |   | */
							    {
							       pointsx[xx].out1 = visx + 0;
							       pointsx[xx].out2 = tilew - tilex;
							       pointsx[xx].in1 = (int)(((double)tilex * (double)oo->current.image.w * e->current.viewport.w) / ((double)e->current.drawable_width * (double)tilew));
							       pointsx[xx].in2 = func_image_get_width(im) - pointsx[xx].in1;
							    }
						       }
						     else
						       {
							  if (xx == (ww - 1))  /* |   |   | * | */
							    {
							       pointsx[xx].out1 = visx + (xx * tilew) - tilex;
							       pointsx[xx].out2 = visw - (pointsx[xx].out1 - visx);
							       pointsx[xx].in1 = 0;
							       pointsx[xx].in2 = (int)(((double)pointsx[xx].out2 * (double)oo->current.image.w * e->current.viewport.w) / ((double)e->current.drawable_width * (double)tilew));
							    }
							  else /* |   | * |   | */
							    {
							       pointsx[xx].out1 = visx + (xx * tilew) - tilex;
							       pointsx[xx].out2 = tilew;
							       pointsx[xx].in1 = 0;
							       pointsx[xx].in2 = func_image_get_width(im);
							    }
						       }
						  }
						
						visy = y;
						vish = h;
						if (y < 0) 
						  {
						     visy = 0;
						     vish += y;
						  }
						if ((visy + vish) > e->current.drawable_height) vish = e->current.drawable_height - visy;
						tiley = (-oo->current.fill.y * (double)e->current.drawable_height) / e->current.viewport.h;
						tileh = (oo->current.fill.h * (double)e->current.drawable_height) / e->current.viewport.h;
						if (tileh < 1) tileh = 1;
						tiley = (((tiley - (y - visy)) % tileh) + tileh) % tileh;
						if (tiley == 0) hh = 1 + (vish - 1) / tileh;
						else hh = 1 + ((vish + tiley) / tileh);

						pointsy = malloc(hh * sizeof(struct _points));
						for (yy = 0; yy < hh; yy++)
						  {
						     if (yy == 0)
						       {
							  if (yy == (hh - 1))  /* | * | */
							    {
							       pointsy[yy].out1 = visy + 0;
							       pointsy[yy].out2 = vish;
							       pointsy[yy].in1 = (int)(((double)tiley * (double)oo->current.image.h * e->current.viewport.h) / ((double)e->current.drawable_height * (double)tileh));
							       pointsy[yy].in2 = (int)(((double)func_image_get_height(im)  * (double)vish) / (double)tileh);
							    }
							  else  /* | *  |   |   | */
							    {
							       pointsy[yy].out1 = visy + 0;
							       pointsy[yy].out2 = tileh - tiley;
							       pointsy[yy].in1 = (int)(((double)tiley * (double)oo->current.image.h * e->current.viewport.h) / ((double)e->current.drawable_height * (double)tileh));
							       pointsy[yy].in2 = func_image_get_height(im) - pointsy[yy].in1;
							    }
						       }
						     else
						       {
							  if (yy == (hh - 1))  /* |   |   | * | */
							    {
							       pointsy[yy].out1 = visy + (yy * tileh) - tiley;
							       pointsy[yy].out2 = vish - (pointsy[yy].out1 - visy);
							       pointsy[yy].in1 = 0;
							       pointsy[yy].in2 = (int)(((double)pointsy[yy].out2 * (double)oo->current.image.h * e->current.viewport.h) / ((double)e->current.drawable_height * (double)tileh));
							    }
							  else /* |   | * |   | */
							    {
							       pointsy[yy].out1 = visy + (yy * tileh) - tiley;
							       pointsy[yy].out2 = tileh;
							       pointsy[yy].in1 = 0;
							       pointsy[yy].in2 = func_image_get_height(im);
							    }
						       }
						  }
						for (yy = 0; yy < hh; yy++)
						  {
						     for (xx = 0; xx < ww; xx++)
						       {
							  func_image_draw(im, 
									  e->current.display,
									  e->current.drawable,
									  e->current.drawable_width,
									  e->current.drawable_height,
									  pointsx[xx].in1, pointsy[yy].in1, 
									  pointsx[xx].in2, pointsy[yy].in2, 
									  pointsx[xx].out1, pointsy[yy].out1, 
									  pointsx[xx].out2, pointsy[yy].out2);
						       }
						  }
						free(pointsx);
						free(pointsy);
						func_image_free(im);
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
				   default:
				      break;
				   }
			      }
			 }
		    }
	       }
	     func_flush_draw(e->current.display, e->current.drawable);
	  }
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

void
evas_set_scale_smoothness(Evas e, int smooth)
{
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	__evas_imlib_image_set_smooth_scaling(smooth);
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	break;
     case RENDER_METHOD_3D_HARDWARE:
	__evas_gl_image_set_smooth_scaling(smooth);
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     default:
	return 0;
	break;
     }
}

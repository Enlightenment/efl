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

#ifndef SPANS_COMMON
#define SPANS_COMMON(x1, w1, x2, w2) \
(!((((x2) + (w2)) <= (x1)) || ((x2) >= ((x1) + (w1)))))
#define RECTS_INTERSECT(x, y, w, h, xx, yy, ww, hh) \
((SPANS_COMMON((x), (w), (xx), (ww))) && (SPANS_COMMON((y), (h), (yy), (hh))))
#endif

void _evas_get_current_clipped_geometry(Evas e, Evas_Object o, double *x, double *y, double *w, double *h);
void _evas_get_previous_clipped_geometry(Evas e, Evas_Object o, double *x, double *y, double *w, double *h);

void
_evas_object_get_current_translated_coords(Evas e, Evas_Object o, 
					   int *x, int *y, int *w, int *h, int clip)
{
   double ox, oy, ow, oh;
   
   ox = o->current.x; oy = o->current.y;
   ow = o->current.w; oh = o->current.h;
   if (clip)
     _evas_get_current_clipped_geometry(e, o, &ox, &oy, &ow, &oh);
   *x = (int)
      (((ox - e->current.viewport.x) * 
	(double)e->current.drawable_width) / 
       e->current.viewport.w);
   *y = (int)
      (((oy - e->current.viewport.y) * 
	(double)e->current.drawable_height) / 
       e->current.viewport.h);
   *w = (int)
      ((ow * (double)e->current.drawable_width) /
       e->current.viewport.w);
   *h = (int)
      ((oh * (double)e->current.drawable_height) /
       e->current.viewport.h);
}

void
_evas_object_get_previous_translated_coords(Evas e, Evas_Object o, 
					   int *x, int *y, int *w, int *h, int clip)
{
   double ox, oy, ow, oh;
   
   ox = o->previous.x; oy = o->previous.y;
   ow = o->previous.w; oh = o->previous.h;
   if (clip)
     _evas_get_previous_clipped_geometry(e, o, &ox, &oy, &ow, &oh);
   *x = (int)
      (((ox - e->previous.viewport.x) * 
	(double)e->previous.drawable_width) / 
       e->previous.viewport.w);
   *y = (int)
      (((oy - e->previous.viewport.y) * 
	(double)e->previous.drawable_height) / 
       e->previous.viewport.h);
   *w = (int)
      ((ow * (double)e->previous.drawable_width) /
       e->previous.viewport.w);
   *h = (int)
      ((oh * (double)e->previous.drawable_height) /
       e->previous.viewport.h);
}

void
_evas_clip_obscures(Evas e)
{
   Imlib_Updates up, old_up, ob;
   
   up = e->updates;
   if (!up) return;
   for (ob = e->obscures; ob; ob = imlib_updates_get_next(ob))
     {
	int ox, oy, ow, oh;
	
	imlib_updates_get_coordinates(ob, &ox, &oy, &ow, &oh);	
	e->updates = NULL;
	old_up = up;
	while (up)
	  {
	     int x, y, w, h;
	     
	     imlib_updates_get_coordinates(up, &x, &y, &w, &h);
	     if (RECTS_INTERSECT(x, y, w, h, ox, oy, ow, oh))
	       {
		  int rx, ry, rw, rh;
		  
		  /* left */
		  rx = x; ry = y;
		  rw = ox - x; rh = h;
		  if ((rw > 0) && (rh > 0)) 
		     e->updates = imlib_update_append_rect(e->updates, rx, ry, rw, rh);
		  /* right */
		  rx = ox + ow; ry = y;
		  rw = x + w - (ox + ow); rh = h;
		  if ((rw > 0) && (rh > 0)) 
		     e->updates = imlib_update_append_rect(e->updates, rx, ry, rw, rh);
		  /* top */
		  rx = ox; ry = y;
		  if (ox < x) rx = x;
		  rw = ow; rh = oy - y;
		  if ((rx + rw) > (x + w)) rw = (x + w) - rx;
		  if ((rw > 0) && (rh > 0)) 
		     e->updates = imlib_update_append_rect(e->updates, rx, ry, rw, rh);
		  /* bottom */
		  rx = ox; ry = oy + oh;
		  if (ox < x) rx = x;
		  rw = ow; rh = (y + h) - ry;
		  if ((rx + rw) > (x + w)) rw = (x + w) - rx;
		  if ((rw > 0) && (rh > 0)) 
		     e->updates = imlib_update_append_rect(e->updates, rx, ry, rw, rh);
	       }
	     else
		e->updates = imlib_update_append_rect(e->updates, x, y, w, h);
	     up = imlib_updates_get_next(up);	
	  }
	if (old_up) imlib_updates_free(old_up);
	up = e->updates;
     }
}

/* for parts of an evas that are obscured for output */
void
evas_add_obscured_rect(Evas e, int x, int y, int w, int h)
{
   if (!e) return;
   if (w <= 0) return;
   if (h <= 0) return;
   e->obscures = imlib_update_append_rect(e->obscures, x, y, w, h);
}

void
evas_clear_obscured_rects(Evas e)
{
   if (!e) return;
   if (e->obscures)
     {
	imlib_updates_free(e->obscures);
	e->obscures = NULL;
     }
}

/* for exposes or forced redraws (relative to output drawable) */
void
evas_update_rect(Evas e, int x, int y, int w, int h)
{
   if (!e) return;
   if (w <= 0) return;
   if (h <= 0) return;
   e->updates = imlib_update_append_rect(e->updates, x, y, w, h);
   e->changed = 1;
}

/* drawing */
void
evas_render(Evas e)
{
   Imlib_Updates u;
   
   u = evas_render_updates(e);
   if (u) imlib_updates_free(u);
}

Imlib_Updates
evas_render_updates(Evas e)
{
   Imlib_Updates up = NULL;
   Evas_List delete_objects;
   Evas_List l, ll;
   void (*func_draw_add_rect) (Display *disp, Imlib_Image dstim, Window win, int x, int y, int w, int h);
   void * (*func_image_new_from_file) (Display *disp, char *file);   
   void (*func_image_set_borders) (void *im, int left, int right, int top, int bottom);
   void (*func_image_draw) (void *im, Display *disp, Imlib_Image dstim, Window w, int win_w, int win_h, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int cr, int cg, int cb, int ca);
   void (*func_image_free) (void *im);   
   void (*func_flush_draw) (Display *disp, Imlib_Image dstim, Window w);
   void (*func_init) (Display *disp, int screen, int colors);
   int (*func_image_get_width) (void *im);
   int (*func_image_get_height) (void *im);
   void * (*func_text_font_new) (Display *disp, char *font, int size);
   void (*func_text_font_free) (void *fn);
   void (*func_text_draw) (void *fn, Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x, int y, char *text, int r, int g, int b, int a);
   void (*func_rectangle_draw) (Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x, int y, int w, int h, int r, int g, int b, int a);
   void (*func_line_draw) (Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x1, int y1, int x2, int y2, int r, int g, int b, int a);
   void (*func_gradient_draw) (void *gr, Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x, int y, int w, int h, double angle);
   void (*func_poly_draw) (Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, Evas_List points, int r, int g, int b, int a);
   void (*func_set_clip_rect) (int on, int x, int y, int w, int h, int r, int g, int b, int a);
   
   if (!e) return NULL;
   if ((e->current.render_method == RENDER_METHOD_IMAGE) &&
       (!e->current.image))
      return NULL;
   if ((e->current.render_method != RENDER_METHOD_IMAGE) &&
       ((!e->current.display) ||
	(!e->current.visual) ||
	(!e->current.colormap) ||
	(!e->current.drawable))) 
      return NULL;
   if ((!e->changed) || 
       (e->current.drawable_width <= 0) || 
       (e->current.drawable_height <= 0) ||
       (e->current.viewport.w <= 0) || 
       (e->current.viewport.h <= 0))
      return NULL;
   
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	func_draw_add_rect       = __evas_imlib_draw_add_rect;
	func_image_new_from_file = __evas_imlib_image_new_from_file;
	func_image_set_borders   = __evas_imlib_image_set_borders;
	func_image_draw          = __evas_imlib_image_draw;
	func_image_free          = __evas_imlib_image_free;
	func_flush_draw          = __evas_imlib_flush_draw;
	func_init                = __evas_imlib_init;
	func_image_get_width     = __evas_imlib_image_get_width;
	func_image_get_height    = __evas_imlib_image_get_height;
	func_text_font_new       = __evas_imlib_text_font_new;
	func_text_font_free      = __evas_imlib_text_font_free;
	func_text_draw           = __evas_imlib_text_draw;
	func_rectangle_draw      = __evas_imlib_rectangle_draw;
	func_line_draw           = __evas_imlib_line_draw;
	func_gradient_draw       = __evas_imlib_gradient_draw;
	func_poly_draw           = __evas_imlib_poly_draw;
	func_set_clip_rect       = __evas_imlib_set_clip_rect;
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	func_draw_add_rect       = __evas_x11_draw_add_rect;
	func_image_new_from_file = __evas_x11_image_new_from_file;
	func_image_set_borders   = __evas_x11_image_set_borders;
	func_image_draw          = __evas_x11_image_draw;
	func_image_free          = __evas_x11_image_free;
	func_flush_draw          = __evas_x11_flush_draw;
	func_init                = __evas_x11_init;
	func_image_get_width     = __evas_x11_image_get_width;
	func_image_get_height    = __evas_x11_image_get_height;
	func_text_font_new       = __evas_x11_text_font_new;
	func_text_font_free      = __evas_x11_text_font_free;
	func_text_draw           = __evas_x11_text_draw;
	func_rectangle_draw      = __evas_x11_rectangle_draw;
	func_line_draw           = __evas_x11_line_draw;
	func_gradient_draw       = __evas_x11_gradient_draw;
	func_poly_draw           = __evas_x11_poly_draw;
	func_set_clip_rect       = __evas_x11_set_clip_rect;
	break;
#ifdef HAVE_GL
     case RENDER_METHOD_3D_HARDWARE:
	func_draw_add_rect       = __evas_gl_draw_add_rect;
	func_image_new_from_file = __evas_gl_image_new_from_file;
	func_image_set_borders   = __evas_gl_image_set_borders;
	func_image_draw          = __evas_gl_image_draw;
	func_image_free          = __evas_gl_image_free;
	func_flush_draw          = __evas_gl_flush_draw;
	func_init                = __evas_gl_init;
	func_image_get_width     = __evas_gl_image_get_width;
	func_image_get_height    = __evas_gl_image_get_height;
	func_text_font_new       = __evas_gl_text_font_new;
	func_text_font_free      = __evas_gl_text_font_free;
	func_text_draw           = __evas_gl_text_draw;
	func_rectangle_draw      = __evas_gl_rectangle_draw;
	func_line_draw           = __evas_gl_line_draw;
	func_gradient_draw       = __evas_gl_gradient_draw;
	func_poly_draw           = __evas_gl_poly_draw;
	func_set_clip_rect       = __evas_gl_set_clip_rect;
	break;
#endif
     case RENDER_METHOD_ALPHA_HARDWARE:
	func_draw_add_rect       = __evas_render_draw_add_rect;
	func_image_new_from_file = __evas_render_image_new_from_file;
	func_image_set_borders   = __evas_render_image_set_borders;
	func_image_draw          = __evas_render_image_draw;
	func_image_free          = __evas_render_image_free;
	func_flush_draw          = __evas_render_flush_draw;
	func_init                = __evas_render_init;
	func_image_get_width     = __evas_render_image_get_width;
	func_image_get_height    = __evas_render_image_get_height;
	func_text_font_new       = __evas_render_text_font_new;
	func_text_font_free      = __evas_render_text_font_free;
	func_text_draw           = __evas_render_text_draw;
	func_rectangle_draw      = __evas_render_rectangle_draw;
	func_line_draw           = __evas_render_line_draw;
	func_gradient_draw       = __evas_render_gradient_draw;
	func_poly_draw           = __evas_render_poly_draw;
	func_set_clip_rect       = __evas_render_set_clip_rect;
	break;
     case RENDER_METHOD_IMAGE:
	func_draw_add_rect       = __evas_image_draw_add_rect;
	func_image_new_from_file = __evas_image_image_new_from_file;
	func_image_set_borders   = __evas_image_image_set_borders;
	func_image_draw          = __evas_image_image_draw;
	func_image_free          = __evas_image_image_free;
	func_flush_draw          = __evas_image_flush_draw;
	func_init                = __evas_image_init;
	func_image_get_width     = __evas_image_image_get_width;
	func_image_get_height    = __evas_image_image_get_height;
	func_text_font_new       = __evas_image_text_font_new;
	func_text_font_free      = __evas_image_text_font_free;
	func_text_draw           = __evas_image_text_draw;
	func_rectangle_draw      = __evas_image_rectangle_draw;
	func_line_draw           = __evas_image_line_draw;
	func_gradient_draw       = __evas_image_gradient_draw;
	func_poly_draw           = __evas_image_poly_draw;
	func_set_clip_rect       = __evas_image_set_clip_rect;
	break;
     default:
	break;
     }
   if ((e->current.viewport.x != e->previous.viewport.x) || 
       (e->current.viewport.y != e->previous.viewport.y))
      evas_update_rect(e, 
		       0, 0, 
		       e->current.drawable_width, 
		       e->current.drawable_height);
   else if ((e->current.viewport.w != e->previous.viewport.w) || 
	    (e->current.viewport.h != e->previous.viewport.h))
     {
	if (((double)(e->current.viewport.w - e->previous.viewport.w) ==
	     (double)(e->current.drawable_width - e->previous.drawable_width)) &&
	    ((double)(e->current.viewport.w - e->previous.viewport.w) ==
	     (double)(e->current.drawable_width - e->previous.drawable_width)))
	  {
	     if ((e->current.drawable_width > e->previous.drawable_width))
	       evas_update_rect(e, 
				e->current.drawable_width, 
				0, 
				e->current.drawable_width - e->previous.drawable_width, 
				e->current.drawable_height);
	     if ((e->current.drawable_width > e->previous.drawable_width) &&
		 (e->current.drawable_height > e->previous.drawable_height))
	       evas_update_rect(e, 
				e->current.drawable_width, 
				e->current.drawable_height, 
				e->current.drawable_width - e->previous.drawable_width,
				e->current.drawable_height - e->previous.drawable_height);
	     if ((e->current.drawable_height > e->previous.drawable_height))
	       evas_update_rect(e, 
				0,
				e->current.drawable_height, 
				e->current.drawable_width, 
				e->current.drawable_height - e->previous.drawable_height);
	  }
	else
	  evas_update_rect(e, 
			   0, 0, 
			   e->current.drawable_width, 
			   e->current.drawable_height);
     }
   
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
	     int real_change, prop_change, clip_change;

	     real_change = 0;
	     prop_change = 0;
	     clip_change = 0;
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
			(o->current.layer != o->previous.layer) ||
			(o->current.stacking) ||
			(o->clip.changed)))
		      )
		    {
		       if (((o->current.visible != o->previous.visible) ||
			    ((o->current.visible) &&
			     (o->current.zoomscale != o->previous.zoomscale) ||
			     (o->current.layer != o->previous.layer) ||
			     (o->current.stacking))))
			  prop_change = 1;
		       if ((!prop_change) &&
			   (o->type == OBJECT_RECTANGLE))
			 {
			    Evas_Object_Rectangle oo;
			    
			    oo = o;
			    if ((oo->current.r != oo->previous.r) ||
				(oo->current.g != oo->previous.g) ||
				(oo->current.b != oo->previous.b) ||
				(oo->current.a != oo->previous.a)
				)
			       prop_change = 1;
			 }
		       real_change = 1;
		       clip_change = o->clip.changed;
		    }		  
		  
		  o->current.stacking = 0;
		  if ((!real_change) && (o->current.visible))
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
				     (oo->current.border.l != oo->previous.border.l) ||
				     (oo->current.border.r != oo->previous.border.r) ||
				     (oo->current.border.t != oo->previous.border.t) ||
				     (oo->current.border.b != oo->previous.border.b) ||
				     (oo->current.fill.x != oo->previous.fill.x) ||
				     (oo->current.fill.y != oo->previous.fill.y) ||
				     (oo->current.fill.w != oo->previous.fill.w) ||
				     (oo->current.fill.h != oo->previous.fill.h) ||
				     (oo->current.color.r != oo->previous.color.r) ||
				     (oo->current.color.g != oo->previous.color.g) ||
				     (oo->current.color.b != oo->previous.color.b) ||
				     (oo->current.color.a != oo->previous.color.a)
				     )
				    real_change = 1;
				 oo->current.new_data = 0;
				 oo->previous = oo->current;
			      }
			    break;
			 case OBJECT_TEXT:
			      {
				 Evas_Object_Text oo;
				 
				 oo = o;
				 if (((oo->current.text) && (!oo->previous.text)) ||
				     ((oo->current.font) && (!oo->previous.font)) ||
				     (oo->current.size != oo->previous.size) ||
				     (oo->current.r != oo->previous.r) ||
				     (oo->current.g != oo->previous.g) ||
				     (oo->current.b != oo->previous.b) ||
				     (oo->current.a != oo->previous.a)
				     )
				    real_change = 1;
				 oo->previous = oo->current;
			      }
			    break;
			 case OBJECT_RECTANGLE:
			      {
				 Evas_Object_Rectangle oo;
				 
				 oo = o;
				 if ((oo->current.r != oo->previous.r) ||
				     (oo->current.g != oo->previous.g) ||
				     (oo->current.b != oo->previous.b) ||
				     (oo->current.a != oo->previous.a)
				     )
				   {
				      real_change = 1;
				      prop_change = 1;
				   }
				 oo->previous = oo->current;
			      }
			    break;
			 case OBJECT_LINE:
			      {
				 Evas_Object_Line oo;
				 
				 oo = o;
				 if ((oo->current.x1 != oo->previous.x1) ||
				     (oo->current.y1 != oo->previous.y1) ||
				     (oo->current.x2 != oo->previous.x2) ||
				     (oo->current.y2 != oo->previous.y2) ||
				     (oo->current.r != oo->previous.r) ||
				     (oo->current.g != oo->previous.g) ||
				     (oo->current.b != oo->previous.b) ||
				     (oo->current.a != oo->previous.a)
				     )
				    real_change = 1;
				 oo->previous = oo->current;
			      }
			    break;
			 case OBJECT_GRADIENT_BOX:
			      {
				 Evas_Object_Gradient_Box oo;
				 
				 oo = o;
				 if ((oo->current.new_gradient) ||
				     (oo->current.angle != oo->previous.angle))
				    real_change = 1;
				 oo->current.new_gradient = 0;
				 oo->previous = oo->current;
			      }
			    break;
			 case OBJECT_POLYGON:
			      {
				 Evas_Object_Poly oo;
				 
				 oo = o;
				 if ((oo->previous.points != oo->current.points) ||
				     (oo->current.r != oo->previous.r) ||
				     (oo->current.g != oo->previous.g) ||
				     (oo->current.b != oo->previous.b) ||
				     (oo->current.a != oo->previous.a))
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
		  
		  /* special case for rectangle since its all one color */
		  if ((o->type == OBJECT_RECTANGLE) &&
		      (!prop_change) &&
		      (RECTS_INTERSECT(o->current.x, o->current.y,
				       o->current.w, o->current.h,
				       o->previous.x, o->previous.y,
				       o->previous.w, o->previous.h)))
		    {
		       int xx, yy, ww, hh;
		       int x1[4], y1[4], i, j;
		       Evas_List rl, rll;
		       Evas_Rectangle r;
		       
		       _evas_object_get_previous_translated_coords(e, o, 
								   &x, &y, 
								   &w, &h, 
								   1 - clip_change);
		       _evas_object_get_current_translated_coords(e, o, 
								  &xx, &yy, 
								  &ww, &hh, 
								  1 - clip_change);
		       rl = NULL;
		       if (x < xx)
			 {
			    x1[0] = x;
			    x1[1] = xx;
			 }
		       else
			 {
			    x1[0] = xx;
			    x1[1] = x;
			 }
		       if ((x + w) < (xx + ww))
			 {
			    x1[2] = x + w;
			    x1[3] = xx + ww;
			 }
		       else
			 {
			    x1[2] = xx + ww;
			    x1[3] = x + w;
			 }
		       if (y < yy)
			 {
			    y1[0] = y;
			    y1[1] = yy;
			 }
		       else
			 {
			    y1[0] = yy;
			    y1[1] = y;
			 }
		       if ((y + h) < (yy + hh))
			 {
			    y1[2] = y + h;
			    y1[3] = yy + hh;
			 }
		       else
			 {
			    y1[2] = yy + hh;
			    y1[3] = y + h;
			 }
		       for (j = 0; j < 3; j++)
			 {
			    for (i = 0; i < 3; i++)
			      {
				 r = malloc(sizeof(struct _Evas_Rectangle)); 
				 rl = evas_list_append(rl, r);
				 r->x = x1[i];
				 r->y = y1[j];
				 r->w = x1[i + 1] - x1[i];
				 r->h = y1[j + 1] - y1[j];
			      }
			 }
		       if (rl)
			 {
			    for (rll = rl; rll; rll = rll->next)
			      {
				 r = rll->data;
				 if ((r->w > 0) && (r->h > 0))
				   {
				      int intsec1, intsec2;
				      
				      intsec1 = 0;
				      intsec2 = 0;
				      if (RECTS_INTERSECT(r->x, r->y, r->w, r->h,
							  x, y, w, h))
					 intsec1 = 1;
				      if (RECTS_INTERSECT(r->x, r->y, r->w, r->h,
							  xx, yy, ww, hh))
					 intsec2 = 1;
				      if ((intsec1 ^ intsec2))
					 evas_update_rect(e, r->x, r->y, r->w, r->h);
				   }
				 free(r);
			      }
			    evas_list_free(rl);
			 }
		    }
		  else
		    {
		       _evas_object_get_previous_translated_coords(e, o, 
								   &x, &y, 
								   &w, &h, 
								   1 - clip_change);
		       evas_update_rect(e, x, y, w, h);
		       _evas_object_get_current_translated_coords(e, o, 
								  &x, &y, 
								  &w, &h, 
								  1 - clip_change);
		       evas_update_rect(e, x, y, w, h);
		    }
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
   
   func_init(e->current.display, e->current.screen, e->current.colors);
   up = NULL;
   if (e->updates)
     {
	_evas_clip_obscures(e);
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
				     e->current.image,
				     e->current.drawable,
				     x, y, w, h);
		  u = imlib_updates_get_next(u);
	       }
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
								       &w, &h, 
								       1 - o->clip.changed);
			    o->clip.changed = 0;
			    if (RECTS_INTERSECT(0, 0, 
						e->current.drawable_width,
						e->current.drawable_height,
						x, y, w, h) &&
				(!o->clip.list))
			      {
				 if (!o->clip.changed)
				   _evas_object_get_current_translated_coords(e, o, 
									      &x, &y, 
									      &w, &h, 
									      0);
				 if (o->clip.object)
				   {
				      Evas_Object_Rectangle oo;
				      int clr, clg, clb, cla;
				      int clx, cly, clw, clh;
				      
				      clr = 255;
				      clg = 255;
				      clb = 255;
				      cla = 255;
				      if (o->clip.object->type == OBJECT_RECTANGLE)
					{
					   oo = o->clip.object;
					   
					   clr = oo->current.r;
					   clg = oo->current.g;
					   clb = oo->current.b;
					   cla = oo->current.a;
					}
				      _evas_object_get_current_translated_coords(e, o, 
										 &clx, &cly, 
										 &clw, &clh, 
										 1);
				      if ((clw > 1) && (clh > 1))
					func_set_clip_rect(1, clx, cly, clw, clh, clr, clg, clb, cla);
				      else
					func_set_clip_rect(1, e->current.drawable_width + 1, e->current.drawable_height + 1, 1, 1, 255, 255, 255, 255);
				   }
				 else
				   func_set_clip_rect(0, 0, 0, 0, 0, 255, 255, 255, 255);
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
						func_image_set_borders(im,
								       oo->current.border.l,
								       oo->current.border.r,
								       oo->current.border.t,
								       oo->current.border.b);
						for (yy = 0; yy < hh; yy++)
						  {
						     for (xx = 0; xx < ww; xx++)
						       {
							  func_image_draw(im, 
									  e->current.display,
									  e->current.image,
									  e->current.drawable,
									  e->current.drawable_width,
									  e->current.drawable_height,
									  pointsx[xx].in1, pointsy[yy].in1, 
									  pointsx[xx].in2, pointsy[yy].in2, 
									  pointsx[xx].out1, pointsy[yy].out1, 
									  pointsx[xx].out2, pointsy[yy].out2,
									  oo->current.color.r,
									  oo->current.color.g,
									  oo->current.color.b,
									  oo->current.color.a);
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
					   void *fn;
					   
					   oo = o;
					   fn = func_text_font_new(e->current.display, oo->current.font, oo->current.size);
					   if (fn)
					     {
						func_text_draw(fn, 
							       e->current.display,
							       e->current.image,
							       e->current.drawable,
							       e->current.drawable_width,
							       e->current.drawable_height,
							       o->current.x,
							       o->current.y,
							       oo->current.text,
							       oo->current.r,
							       oo->current.g,
							       oo->current.b,
							       oo->current.a);
						func_text_font_free(fn);
					     }
					}
				      break;
				   case OBJECT_RECTANGLE:
					{
					   Evas_Object_Rectangle oo;
					   
					   oo = o;
					   if (oo->current.a != 0)
					      func_rectangle_draw(e->current.display,
								  e->current.image,
								  e->current.drawable,
								  e->current.drawable_width,
								  e->current.drawable_height,
								  o->current.x,
								  o->current.y,
								  o->current.w,
								  o->current.h,
								  oo->current.r,
								  oo->current.g,
								  oo->current.b,
								  oo->current.a);
					}
				      break;
				   case OBJECT_LINE:
					{
					   Evas_Object_Line oo;
					   
					   oo = o;
					   func_line_draw(e->current.display,
							  e->current.image,
							  e->current.drawable,
							  e->current.drawable_width,
							  e->current.drawable_height,
							  oo->current.x1,
							  oo->current.y1,
							  oo->current.x2,
							  oo->current.y2,
							  oo->current.r,
							  oo->current.g,
							  oo->current.b,
							  oo->current.a);
					}
				      break;
				   case OBJECT_GRADIENT_BOX:
					{
					   Evas_Object_Gradient_Box oo;
					   
					   oo = o;
					   if (o->renderer_data.method[e->current.render_method])
					      func_gradient_draw(o->renderer_data.method[e->current.render_method],
								 e->current.display,
								 e->current.image,
								 e->current.drawable,
								 e->current.drawable_width,
								 e->current.drawable_height,
								 o->current.x,
								 o->current.y,
								 o->current.w,
								 o->current.h,
								 oo->current.angle);
					}
				      break;
				   case OBJECT_POLYGON:
					{
					   Evas_Object_Poly oo;
					   
					   oo = o;
					   
					   if (oo->current.points)
					      func_poly_draw(e->current.display,
							     e->current.image,
							     e->current.drawable,
							     e->current.drawable_width,
							     e->current.drawable_height,
							     oo->current.points,
							     oo->current.r,
							     oo->current.g,
							     oo->current.b,
							     oo->current.a);
					}
				      break;
				   default:
				      break;
				   }
			      }
			 }
		    }
	       }
	     func_flush_draw(e->current.display, e->current.image, e->current.drawable);
	  }
     }
   e->previous = e->current;
   return up;
}

/* query for settings to use */
Visual *
evas_get_optimal_visual(Evas e, Display *disp)
{
   if (!e) return NULL;
   if (!disp) return NULL;
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	if (__evas_imlib_capable(disp))
	   return __evas_imlib_get_visual(disp, e->current.screen);
	else
	  {
	  }
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	if (__evas_x11_capable(disp))
	   return __evas_x11_get_visual(disp, e->current.screen);
	else
	  {
	  }
	break;
     case RENDER_METHOD_3D_HARDWARE:
	if (__evas_gl_capable(disp))
	   return __evas_gl_get_visual(disp, e->current.screen);
	else
	  {
	     e->current.render_method = RENDER_METHOD_ALPHA_SOFTWARE;
	     return evas_get_optimal_visual(e, disp);
	  }
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	if (__evas_render_capable(disp))
	   return __evas_render_get_visual(disp, e->current.screen);
	else
	  {
	     e->current.render_method = RENDER_METHOD_ALPHA_SOFTWARE;
	     return evas_get_optimal_visual(e, disp);
	  }
	break;
     case RENDER_METHOD_IMAGE:
	if (__evas_image_capable(disp))
	   return __evas_image_get_visual(disp, e->current.screen);
	else
	  {
	  }
	break;
     default:
	return NULL;
	break;
     }
   return NULL;
}

Colormap
evas_get_optimal_colormap(Evas e, Display *disp)
{
   if (!e) return 0;
   if (!disp) return 0;
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	if (__evas_imlib_capable(disp))
	   return __evas_imlib_get_colormap(disp, e->current.screen);
	else
	  {
	  }
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	if (__evas_x11_capable(disp))
	   return __evas_x11_get_colormap(disp, e->current.screen);
	else
	  {
	  }
	break;
     case RENDER_METHOD_3D_HARDWARE:
	if (__evas_gl_capable(disp))
	   return __evas_gl_get_colormap(disp, e->current.screen);
	else
	  {
	     e->current.render_method = RENDER_METHOD_ALPHA_SOFTWARE;
	     return evas_get_optimal_colormap(e, disp);
	  }
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	if (__evas_render_capable(disp))
	   return __evas_render_get_colormap(disp, e->current.screen);
	else
	  {
	     e->current.render_method = RENDER_METHOD_ALPHA_SOFTWARE;
	     return evas_get_optimal_colormap(e, disp);
	  }
	break;
     case RENDER_METHOD_IMAGE:
	if (__evas_image_capable(disp))
	   return __evas_image_get_colormap(disp, e->current.screen);
	else
	  {
	  }
	break;
     default:
	return 0;
	break;
     }
   return 0;
}

void
evas_get_drawable_size(Evas e, int *w, int *h)
{
   if (!e) return;
   if (w) *w = e->current.drawable_width;
   if (h) *h = e->current.drawable_height;
}

void
evas_get_viewport(Evas e, double *x, double *y, double *w, double *h)
{
   if (!e) return;
   if (x) *x = e->current.viewport.x;
   if (y) *y = e->current.viewport.y;
   if (w) *w = e->current.viewport.w;
   if (h) *h = e->current.viewport.h;
}

/* the output settings */
void
evas_set_output(Evas e, Display *disp, Drawable d, Visual *v, Colormap c)
{
   if (!e) return;
   e->current.display = disp;
   e->current.drawable = d;
   e->current.visual = v;
   e->current.colormap = c;
   e->changed = 1;
}

void
evas_set_output_image(Evas e, Imlib_Image image)
{
   if (!e) return;
   e->current.image = image;
   e->changed = 1;   
}

void
evas_set_output_colors(Evas e, int colors)
{
   if (!e) return;
   if (colors < 2) colors = 1;
   else if (colors > 256) colors = 256;
   e->current.colors = colors;
   e->changed = 1;
}

void
evas_set_output_size(Evas e, int w, int h)
{
   if (!e) return;
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   e->current.drawable_width = w;
   e->current.drawable_height = h;
   e->changed = 1;
}

void
evas_set_output_viewport(Evas e, double x, double y, double w, double h)
{
   if (!e) return;
   e->current.viewport.x = x;
   e->current.viewport.y = y;
   e->current.viewport.w = w;
   e->current.viewport.h = h;
   e->changed = 1;
}

void
evas_set_output_method(Evas e, Evas_Render_Method method)
{
   if (!e) return;
   if ((method != RENDER_METHOD_ALPHA_SOFTWARE) &&
       (method != RENDER_METHOD_BASIC_HARDWARE) &&
       (method != RENDER_METHOD_3D_HARDWARE) &&
       (method != RENDER_METHOD_ALPHA_HARDWARE) &&
       (method != RENDER_METHOD_IMAGE))
      return;
   if (!e->current.display)
     {
	e->current.render_method = method;
	e->changed = 1;
     }
}

void
evas_set_scale_smoothness(Evas e, int smooth)
{
   if (!e) return;
   if (smooth <= 0) smooth = 0;
   else smooth = 1;
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	__evas_imlib_image_set_smooth_scaling(smooth);
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	__evas_x11_image_set_smooth_scaling(smooth);
	break;
     case RENDER_METHOD_3D_HARDWARE:
	__evas_gl_image_set_smooth_scaling(smooth);
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	__evas_render_image_set_smooth_scaling(smooth);
	break;
     case RENDER_METHOD_IMAGE:
	__evas_image_image_set_smooth_scaling(smooth);
	break;
     default:
	return;
	break;
     }
}

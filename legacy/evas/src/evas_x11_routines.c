#include "evas_x11_routines.h"

#define CLIP(x, y, w, h, xx, yy, ww, hh) \
if (x < (xx)) {w += (x - (xx)); x = (xx);} \
if (y < (yy)) {h += (y - (yy)); y = (yy);} \
if ((x + w) > ((xx) + (ww))) {w = (ww) - (x - xx);} \
if ((y + h) > ((yy) + (hh))) {h = (hh) - (y - yy);}

static void __evas_x11_image_cache_flush(Display *disp);
static int  __evas_anti_alias = 1;
static Evas_List drawable_list = NULL;

static Visual *__evas_visual = NULL;
static Colormap __evas_cmap = 0;

/*****************************************************************************/
/* image internals ***********************************************************/
/*****************************************************************************/

static void
__evas_x11_image_cache_flush(Display *disp)
{
   int size;
   
   size = imlib_get_cache_size();
   imlib_set_cache_size(0);
   imlib_set_cache_size(size);
}

/*****************************************************************************/
/* image externals ***********************************************************/
/*****************************************************************************/

Evas_X11_Image *
__evas_x11_image_new_from_file(Display *disp, char *file)
{	
   return (Evas_X11_Image *)imlib_load_image(file);
}

void
__evas_x11_image_free(Evas_X11_Image *im)
{
   imlib_context_set_image((Imlib_Image)im);
   imlib_free_image();
}

void
__evas_x11_image_cache_empty(Display *disp)
{
   int size;
   
   size = imlib_get_cache_size();
   imlib_set_cache_size(0);
   imlib_set_cache_size(size);
}

void
__evas_x11_image_cache_set_size(Display *disp, int size)
{
   imlib_set_cache_size(size);
}

int
__evas_x11_image_cache_get_size(Display *disp)
{
   return imlib_get_cache_size();
}

void
__evas_x11_image_draw(Evas_X11_Image *im, 
		      Display *disp, Imlib_Image dstim, Window w, int win_w, int win_h,
		      int src_x, int src_y, int src_w, int src_h,
		      int dst_x, int dst_y, int dst_w, int dst_h,
		      int cr, int cg, int cb, int ca)
{
   Evas_List l;
   Imlib_Color_Modifier cm = NULL;
   Pixmap pmap = 0, mask = 0;

   if (ca == 0) return;
   if ((src_w == 0) || (src_h == 0) || (dst_w == 0) || (dst_w == 0)) return;

   if ((cr != 255) || (cg != 255) || (cb != 255) || (ca != 255))
     {
	DATA8 r[256], g[256], b[256], a[256];
	int i;
	
	cm = imlib_create_color_modifier();
	imlib_context_set_color_modifier(cm);
	for (i = 0; i < 256; i++)
	  {
	     r[i] = (i * cr) / 255;
	     g[i] = (i * cg) / 255;
	     b[i] = (i * cb) / 255;
	     a[i] = (i * ca) / 255;
	  }
	imlib_set_color_modifier_tables(r, g, b, a);
     }
   else
      imlib_context_set_color_modifier(NULL);

   imlib_context_set_display(disp);
   imlib_context_set_visual(__evas_visual);
   imlib_context_set_colormap(__evas_cmap);
   imlib_context_set_drawable(w);
   imlib_context_set_dither_mask(__evas_anti_alias);
   imlib_context_set_anti_alias(__evas_anti_alias);
   imlib_context_set_dither(1);
   imlib_context_set_blend(0);
   imlib_context_set_angle(0.0);
   imlib_context_set_operation(IMLIB_OP_COPY);
   imlib_context_set_direction(IMLIB_TEXT_TO_RIGHT);
   for(l = drawable_list; l; l = l->next)
     {
	Evas_X11_Drawable *dr;
	
	dr = l->data;
	if ((dr->win == w) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_X11_Update *up;
		  
		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      dst_x, dst_y, dst_w, dst_h))
		    {
		       Imlib_Border bd;
		       int xx, yy, ww, hh, iw, ih;
		       

		       if (!up->p)
			  up->p = XCreatePixmap(disp, w, up->w, up->h, dr->depth);
		       imlib_context_set_image(im);
		       imlib_image_get_border(&bd);
		       /* if we haven't created a pixmap for thew image yet this */
		       /* round of updates */

		       iw = imlib_image_get_width();
		       ih = imlib_image_get_height();
		       ww = (iw * dst_w) / src_w;
		       hh = (ih * dst_h) / src_h;
		       xx = (src_x * src_w) / dst_w;
		       yy = (src_y * src_h) / dst_h;
		       if (!pmap)
			  imlib_render_pixmaps_for_whole_image_at_size(&pmap, &mask,
								       ww, hh);
		       if (mask)
			 {
			    XSetClipMask(disp, dr->gc, mask);
			    XSetClipOrigin(disp, dr->gc, dst_x - up->x - src_x, dst_y - up->y - src_y);
			 }
		       else
			 {
			    XSetClipMask(disp, dr->gc, None);
			 }
		       if (pmap)
			  XCopyArea(disp, pmap, up->p, dr->gc, 
				    xx, yy, 
				    dst_w, dst_h, 
				    dst_x - up->x, dst_y - up->y);
		    }
	       }
	  }
     }
   if (pmap) imlib_free_pixmap_and_mask(pmap);
   if (cm)
     {
	imlib_free_color_modifier();
	imlib_context_set_color_modifier(NULL);
     }
}

int
__evas_x11_image_get_width(Evas_X11_Image *im)
{
   imlib_context_set_image((Imlib_Image)im);
   return imlib_image_get_width();
}

int
__evas_x11_image_get_height(Evas_X11_Image *im)
{
   imlib_context_set_image((Imlib_Image)im);
   return imlib_image_get_height();
}

void
__evas_x11_image_set_borders(Evas_X11_Image *im, int left, int right,
			       int top, int bottom)
{
   Imlib_Border bd;
   
   imlib_context_set_image((Imlib_Image)im);
   bd.left = left;
   bd.right = right;
   bd.top = top;
   bd.bottom = bottom;
   imlib_image_set_border(&bd);
}

void
__evas_x11_image_set_smooth_scaling(int on)
{
   __evas_anti_alias = on;
}



























/*****************************************************************************/
/* font internals ************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* font externals ************************************************************/
/*****************************************************************************/

Evas_X11_Font *
__evas_x11_text_font_new(Display *disp, char *font, int size)
{
   return NULL;
}

void
__evas_x11_text_font_free(Evas_X11_Font *fn)
{
}

int
__evas_x11_text_font_get_ascent(Evas_X11_Font *fn)
{
   return 1;
}

int
__evas_x11_text_font_get_descent(Evas_X11_Font *fn)
{
   return 1;
}

int
__evas_x11_text_font_get_max_ascent(Evas_X11_Font *fn)
{
   return 1;
}

int
__evas_x11_text_font_get_max_descent(Evas_X11_Font *fn)
{
   return 1;
}

void
__evas_x11_text_font_get_advances(Evas_X11_Font *fn, char *text,
				    int *advance_horiz,
				    int *advance_vert)
{
}

int
__evas_x11_text_font_get_first_inset(Evas_X11_Font *fn, char *text)
{
   return 1;
}

void
__evas_x11_text_font_add_path(char *path)
{
}

void
__evas_x11_text_font_del_path(char *path)
{
}

char **
__evas_x11_text_font_list_paths(int *count)
{
   return NULL;
}

void
__evas_x11_text_cache_empty(Display *disp)
{
}

void
__evas_x11_text_cache_set_size(Display *disp, int size)
{
}

int
__evas_x11_text_cache_get_size(Display *disp)
{
   return 0;
}

void
__evas_x11_text_draw(Evas_X11_Font *fn, Display *disp, Imlib_Image dstim, Window win, 
		       int win_w, int win_h, int x, int y, char *text, 
		       int r, int g, int b, int a)
{
}

void
__evas_x11_text_get_size(Evas_X11_Font *fn, char *text, int *w, int *h)
{
}

int
__evas_x11_text_get_character_at_pos(Evas_X11_Font *fn, char *text, 
				       int x, int y, 
				       int *cx, int *cy, int *cw, int *ch)
{
   return -1;
}

void
__evas_x11_text_get_character_number(Evas_X11_Font *fn, char *text, 
				       int num, 
				       int *cx, int *cy, int *cw, int *ch)
{
}

























/*****************************************************************************/
/* rectangle externals *******************************************************/
/*****************************************************************************/

void              __evas_x11_rectangle_draw(Display *disp, Imlib_Image dstim, Window win,
					      int win_w, int win_h,
					      int x, int y, int w, int h,
					      int r, int g, int b, int a)
{
   Evas_List l;
   DATA32 pixel;
   Imlib_Image im = NULL;
   Pixmap pmap = 0, mask = 0, s_mask = 0;

   imlib_context_set_color(r, g, b, a);
   imlib_context_set_display(disp);
   imlib_context_set_visual(__evas_visual);
   imlib_context_set_colormap(__evas_cmap);
   imlib_context_set_drawable(win);
   imlib_context_set_dither_mask(1);
   imlib_context_set_anti_alias(0);
   imlib_context_set_dither(1);
   imlib_context_set_blend(0);
   imlib_context_set_angle(0.0);
   imlib_context_set_operation(IMLIB_OP_COPY);
   imlib_context_set_direction(IMLIB_TEXT_TO_RIGHT);
   imlib_context_set_color_modifier(NULL);
   for(l = drawable_list; l; l = l->next)
     {
	Evas_X11_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_X11_Update *up;

		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      x, y, w, h))
		    {
		       if (!up->p)
			  up->p = XCreatePixmap(disp, win, up->w, up->h, dr->depth);
		       if (!im)
			 {
			    pixel = (a << 24) | (r << 16) | (g << 8) | b;
			    im = imlib_create_image_using_data(1, 1, &pixel);
			    imlib_context_set_image(im);
			    if (a < 255) imlib_image_set_has_alpha(1);
			    if (!pmap)
			       imlib_render_pixmaps_for_whole_image_at_size(&pmap, &mask,
									    32, 32);
			 }
		       if (a < 255)
			 {
			    if (mask)
			      {
				 int xx, yy, ww, hh, x1, y1, xs, ys;
				 GC gc;
				 XGCValues gcv;
				 
				 xx = x - up->x;
				 yy = y - up->y;
				 ww = w;
				 hh = h;
				 CLIP(xx, yy, ww, hh, 0, 0, up->w, up->h);
				 s_mask = XCreatePixmap(disp, win, ww, hh, 1);
				 gc = XCreateGC(disp, s_mask, 0, &gcv);
				 xs = (x - up->x) % 32;
				 if (xs > 0) xs = 0;
				 else
				   {
				      while (xs < 0) xs += 32;
				      xs -= 32;
				   }
				 ys = (y - up->y) % 32;
				 if (ys > 0) ys = 0;
				 else
				   {
				      while (ys < 0) ys += 32;
				      ys -= 32;
				   }
				 for (y1 = ys; y1 < hh; y1 += 32)
				   {
				      for (x1 = xs; x1 < ww; x1 += 32)
					 XCopyArea(disp, mask, s_mask, gc,
						   0, 0, 32, 32,
						   x1, y1);
				   }
				 XSetClipMask(disp, dr->gc, s_mask);
				 XSetClipOrigin(disp, dr->gc, xx, yy);
				 XFreeGC(disp, gc);
			      }
			    else
			      {
				 XSetClipMask(disp, dr->gc, None);
			      }
			 }
		       if (pmap)
			 {
			    XSetFillStyle(disp, dr->gc, FillTiled);
			    XSetTile(disp, dr->gc, pmap);
			    XSetTSOrigin(disp, dr->gc, x - up->x, y - up->y);
			 }
		       else
			 {
			    XSetFillStyle(disp, dr->gc, FillSolid);
			    XSetTile(disp, dr->gc, None);
			 }
		       XFillRectangle(disp, up->p, dr->gc, x - up->x, y - up->y, w, h);
		       if (s_mask) XFreePixmap(disp, s_mask);
		       s_mask = 0;
		    }
	       }
	  }
     }
   if (pmap) imlib_free_pixmap_and_mask(pmap);	
   if (im) 
     {
	imlib_context_set_image(im);
	imlib_free_image();
     }
}

















/*****************************************************************************/
/* rectangle externals *******************************************************/
/*****************************************************************************/

void              __evas_x11_line_draw(Display *disp, Imlib_Image dstim, Window win,
					 int win_w, int win_h,
					 int x1, int y1, int x2, int y2,
					 int r, int g, int b, int a)
{
   Evas_List l;
   int x, y, w, h;
   DATA32 pixel;
   Imlib_Image im = NULL;
   Pixmap pmap = 0, mask = 0, s_mask = 0;

   imlib_context_set_color(r, g, b, a);
   imlib_context_set_display(disp);
   imlib_context_set_visual(__evas_visual);
   imlib_context_set_colormap(__evas_cmap);
   imlib_context_set_drawable(win);
   imlib_context_set_dither_mask(1);
   imlib_context_set_anti_alias(0);
   imlib_context_set_dither(1);
   imlib_context_set_blend(0);
   imlib_context_set_angle(0.0);
   imlib_context_set_operation(IMLIB_OP_COPY);
   imlib_context_set_direction(IMLIB_TEXT_TO_RIGHT);
   imlib_context_set_color_modifier(NULL);
   w = x2 - x1;
   if (w < 0) w = -w;
   h = y2 - y1;
   if (h < 0) h = -h;
   if (x1 < x2) x = x1;
   else x = x2;
   if (y1 < y2) y = y1;
   else y = y2;
   w++; h++;
   for(l = drawable_list; l; l = l->next)
     {
	Evas_X11_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_X11_Update *up;

		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      x, y, w, h))
		    {
		       if (!up->p)
			  up->p = XCreatePixmap(disp, win, up->w, up->h, dr->depth);
		       if (!im)
			 {
			    pixel = (a << 24) | (r << 16) | (g << 8) | b;
			    im = imlib_create_image_using_data(1, 1, &pixel);
			    imlib_context_set_image(im);
			    if (a < 255)
			       imlib_image_set_has_alpha(1);
			    if (!pmap)
			       imlib_render_pixmaps_for_whole_image_at_size(&pmap, &mask,
									    32, 32);
			 }
		       if (a < 255)
			 {
			    if (mask)
			      {
				 int xx, yy, ww, hh, x1, y1, xs, ys;
				 GC gc;
				 XGCValues gcv;
				 
				 xx = x - up->x;
				 yy = y - up->y;
				 ww = w;
				 hh = h;
				 CLIP(xx, yy, ww, hh, 0, 0, up->w, up->h);
				 s_mask = XCreatePixmap(disp, win, ww, hh, 1);
				 gc = XCreateGC(disp, s_mask, 0, &gcv);
				 xs = (x - up->x) % 32;
				 if (xs > 0) xs = 0;
				 else
				   {
				      while (xs < 0) xs += 32;
				      xs -= 32;
				   }
				 ys = (y - up->y) % 32;
				 if (ys > 0) ys = 0;
				 else
				   {
				      while (ys < 0) ys += 32;
				      ys -= 32;
				   }
				 for (y1 = ys; y1 < hh; y1 += 32)
				   {
				      for (x1 = xs; x1 < ww; x1 += 32)
					 XCopyArea(disp, mask, s_mask, gc,
						   0, 0, 32, 32,
						   x1, y1);
				   }
				 XSetClipMask(disp, dr->gc, s_mask);
				 XSetClipOrigin(disp, dr->gc, xx, yy);
				 XFreeGC(disp, gc);
			      }
			    else
			      {
				 XSetClipMask(disp, dr->gc, None);
			      }
			 }
		       if (pmap)
			 {
			    XSetFillStyle(disp, dr->gc, FillTiled);
			    XSetTile(disp, dr->gc, pmap);
			    XSetTSOrigin(disp, dr->gc, x - up->x, y - up->y);
			 }
		       else
			 {
			    XSetFillStyle(disp, dr->gc, FillSolid);
			    XSetTile(disp, dr->gc, None);
			 }
		       XDrawLine(disp, up->p, dr->gc, x1 - up->x, y1 - up->y, x2 - up->x, y2 - up->y);
		       if (s_mask) XFreePixmap(disp, s_mask);
		       s_mask = 0;
		    }
	       }
	  }
     }
   if (pmap) imlib_free_pixmap_and_mask(pmap);	
   if (im) 
     {
	imlib_context_set_image(im);
	imlib_free_image();
     }
}

















/*****************************************************************************/
/* gradient externals ********************************************************/
/*****************************************************************************/


Evas_X11_Graident *
__evas_x11_gradient_new(Display *disp)
{
   return (Evas_X11_Graident *)imlib_create_color_range();
}

void
__evas_x11_gradient_free(Evas_X11_Graident *gr)
{
   imlib_context_set_color_range((Imlib_Color_Range)gr);
   imlib_free_color_range();
}

void
__evas_x11_gradient_color_add(Evas_X11_Graident *gr, int r, int g, int b, int a, int dist)
{
   imlib_context_set_color_range((Imlib_Color_Range)gr);
   imlib_context_set_color(r, g, b, a);
   imlib_add_color_to_color_range(dist);
}

void
__evas_x11_gradient_draw(Evas_X11_Graident *gr, Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x, int y, int w, int h, double angle)
{
   Evas_List l;
   
   imlib_context_set_display(disp);
   imlib_context_set_visual(__evas_visual);
   imlib_context_set_colormap(__evas_cmap);
   imlib_context_set_drawable(win);
   imlib_context_set_dither_mask(1);
   imlib_context_set_anti_alias(0);
   imlib_context_set_dither(1);
   imlib_context_set_blend(0);
   imlib_context_set_angle(0.0);
   imlib_context_set_operation(IMLIB_OP_COPY);
   imlib_context_set_direction(IMLIB_TEXT_TO_RIGHT);
   imlib_context_set_color_modifier(NULL);
   imlib_context_set_color_range((Imlib_Color_Range)gr);
   for(l = drawable_list; l; l = l->next)
     {
	Evas_X11_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_X11_Update *up;

		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      x, y, w, h))
		    {
		       int xx, yy, ww, hh;
		       Imlib_Image im = NULL;
		       Pixmap pmap = 0, mask = 0;
		       
		       xx = x - up->x;
		       yy = y - up->y;
		       ww = w;
		       hh = h;
		       CLIP(xx, yy, ww, hh, 0, 0, up->w, up->h);
		       if (!up->p)
			  up->p = XCreatePixmap(disp, win, up->w, up->h, dr->depth);
		       im = imlib_create_image(ww, hh);
		       if (im)
			 {
			    DATA32 *data;
			    
			    imlib_context_set_image(im);
			    data = imlib_image_get_data();
			    memset(data, 0, ww * hh * sizeof(DATA32));
			    imlib_image_put_back_data(data);
			    imlib_image_set_has_alpha(1);
			    imlib_image_fill_color_range_rectangle(x - xx - up->x, y - yy - up->y, w, h, angle);
			    pmap = XCreatePixmap(disp, win, ww, hh, dr->depth);
			    mask = XCreatePixmap(disp, win, ww, hh, 1);
			    imlib_context_set_drawable(pmap);
			    imlib_context_set_mask(mask);
			    imlib_render_image_on_drawable(0, 0);
			    imlib_context_set_mask(0);
			 }
		       if (mask)
			 {
			    XSetClipMask(disp, dr->gc, mask);
			    XSetClipOrigin(disp, dr->gc, xx, yy);
			 }
		       else
			 {
			    XSetClipMask(disp, dr->gc, None);
			 }
		       if (pmap)
			  XCopyArea(disp, pmap, up->p, dr->gc, 
				    0, 0, 
				    ww, hh, 
				    xx, yy);
		       if (pmap) XFreePixmap(disp, pmap);
		       if (mask) XFreePixmap(disp, mask);
		       if (im) 
			 {
			    imlib_context_set_image(im);
			    imlib_free_image();
			 }
		    }
	       }
	  }
     }
}











/************/
/* polygons */
/************/
void
__evas_x11_poly_draw (Display *disp, Imlib_Image dstim, Window win, 
		      int win_w, int win_h, 
		      Evas_List points, 
		      int r, int g, int b, int a)
{
}










/*****************************************************************************/
/* general externals *********************************************************/
/*****************************************************************************/

void
__evas_x11_sync(Display *disp)
{
   XSync(disp, False);
}

void
__evas_x11_flush_draw(Display *disp, Imlib_Image dstim, Window win)
{
   Evas_List l;
   
/*   
*/
   for(l = drawable_list; l; l = l->next)
     {
	Evas_X11_Drawable *dr;
	
	dr = l->data;
	
	XSetClipMask(disp, dr->gc, None);
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_X11_Update *up;
		  
		  up = ll->data;

		  if (up->p)
		    {
		       XCopyArea(disp, up->p, win, dr->gc, 
				 0, 0, up->w, up->h, up->x, up->y);
		       XFreePixmap(disp, up->p);
		    }

		  free(up);
	       }
	     if (dr->tmp_images)
		dr->tmp_images = evas_list_free(dr->tmp_images);
	  }
	XFreeGC(disp, dr->gc);
	free(dr);
     }
   if (drawable_list)
      drawable_list = evas_list_free(drawable_list);
   drawable_list = NULL;
}

   
int
__evas_x11_capable(Display *disp)
{
   return 1;
}

Visual *
__evas_x11_get_visual(Display *disp, int screen)
{
   int depth;
   
   __evas_visual = imlib_get_best_visual(disp, screen, &depth);
   return __evas_visual;
}

XVisualInfo *
__evas_x11_get_visual_info(Display *disp, int screen)
{
   static XVisualInfo *vi = NULL;
   XVisualInfo vi_template;
   int n;
   
   if (vi) return vi;
   vi_template.visualid = (__evas_x11_get_visual(disp, screen))->visualid;
   vi_template.screen = screen;
   vi = XGetVisualInfo(disp, VisualIDMask | VisualScreenMask, &vi_template ,&n);
   return vi;
}

Colormap
__evas_x11_get_colormap(Display *disp, int screen)
{
   Visual *v;
   
   if (__evas_cmap) return __evas_cmap;
   v = __evas_x11_get_visual(disp, screen);
   __evas_cmap = DefaultColormap(disp, screen);
   return __evas_cmap;
   __evas_cmap = XCreateColormap(disp, RootWindow(disp, screen), v, AllocNone);
   return __evas_cmap;
}

void
__evas_x11_init(Display *disp, int screen, int colors)
{
   static int initted = 0;
   
   if (!initted)
     {
	imlib_set_color_usage(216);
	imlib_set_font_cache_size(1024 * 1024);
	imlib_set_cache_size(8 * 1024 * 1024);
	initted = 1;
     }
   imlib_set_color_usage(colors);
}

void
__evas_x11_draw_add_rect(Display *disp, Imlib_Image dstim, Window win, 
			   int x, int y, int w, int h)
{
   Evas_List l;
   
   for(l = drawable_list; l; l = l->next)
     {
	Evas_X11_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_X11_Update *up;
	     
	     up = malloc(sizeof(Evas_X11_Update));
	     up->x = x;
	     up->y = y;
	     up->w = w;
	     up->h = h;
	     up->p = 0;
	     dr->tmp_images = evas_list_append(dr->tmp_images, up);
	  }
	return;
     }
     {
	Evas_X11_Drawable *dr;
	Evas_X11_Update *up;
	GC gc;
	XGCValues gcv;
	XWindowAttributes att;
	
	gc = XCreateGC(disp, win, 0, &gcv);
	XGetWindowAttributes(disp, win, &att);
	dr = malloc(sizeof(Evas_X11_Drawable));
	dr->win = win;
	dr->disp = disp;
	dr->tmp_images = NULL;
	dr->gc = gc;
	dr->depth = att.depth;
	up = malloc(sizeof(Evas_X11_Update));
	up->x = x;
	up->y = y;
	up->w = w;
	up->h = h;
	up->p = 0;
	drawable_list = evas_list_append(drawable_list, dr);
	dr->tmp_images = evas_list_append(dr->tmp_images, up);
     }
}

#include "evas_image_routines.h"

static void __evas_image_image_cache_flush(Display *disp);
static int  __evas_anti_alias = 1;
static Evas_List drawable_list = NULL;

/* the current clip region and color */
static int       __evas_clip            = 0;
static int       __evas_clip_x          = 0;
static int       __evas_clip_y          = 0;
static int       __evas_clip_w          = 0;
static int       __evas_clip_h          = 0;
static int       __evas_clip_r          = 0;
static int       __evas_clip_g          = 0;
static int       __evas_clip_b          = 0;
static int       __evas_clip_a          = 0;

/*****************************************************************************/
/* image internals ***********************************************************/
/*****************************************************************************/

static void
__evas_image_image_cache_flush(Display *disp)
{
   int size;
   
   size = imlib_get_cache_size();
   imlib_set_cache_size(0);
   imlib_set_cache_size(size);
}

/*****************************************************************************/
/* image externals ***********************************************************/
/*****************************************************************************/

Evas_Image_Image *
__evas_image_image_new_from_file(Display *disp, char *file)
{	
   return (Evas_Image_Image *)imlib_load_image(file);
}

void
__evas_image_image_free(Evas_Image_Image *im)
{
   imlib_context_set_image((Imlib_Image)im);
   imlib_free_image();
}

void
__evas_image_image_cache_empty(Display *disp)
{
   int size;
   
   size = imlib_get_cache_size();
   imlib_set_cache_size(0);
   imlib_set_cache_size(size);
}

void
__evas_image_image_cache_set_size(Display *disp, int size)
{
   imlib_set_cache_size(size);
}

int
__evas_image_image_cache_get_size(Display *disp)
{
   return imlib_get_cache_size();
}

void
__evas_image_image_draw(Evas_Image_Image *im, 
			Display *disp, Imlib_Image dstim, Window w, int win_w, int win_h,
			int src_x, int src_y, int src_w, int src_h,
			int dst_x, int dst_y, int dst_w, int dst_h,
			int cr, int cg, int cb, int ca)
{
   Evas_List l;
   Imlib_Color_Modifier cm = NULL;

   if (__evas_clip)
     {
	cr = (cr * __evas_clip_r) / 255;
	cg = (cg * __evas_clip_g) / 255;
	cb = (cb * __evas_clip_b) / 255;
	ca = (ca * __evas_clip_a) / 255;
     }   
   if (ca == 0) return;   
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
   
   imlib_context_set_angle(0.0);
   imlib_context_set_operation(IMLIB_OP_COPY);
   imlib_context_set_direction(IMLIB_TEXT_TO_RIGHT);
   imlib_context_set_anti_alias(__evas_anti_alias);
   imlib_context_set_blend(1);
   for(l = drawable_list; l; l = l->next)
     {
	Evas_Image_Drawable *dr;
	
	dr = l->data;
	
	if (dr->im == dstim)
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_Image_Update *up;
		  
		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      dst_x, dst_y, dst_w, dst_h))
		    {
		       if (__evas_clip) 
			 imlib_context_set_cliprect(__evas_clip_x - up->x, 
						    __evas_clip_y - up->y, 
						    __evas_clip_w, 
						    __evas_clip_h);
		       else imlib_context_set_cliprect(0, 0, 0, 0);
		       
                       if (!up->image)
			 {
			    DATA32 *data;
			    
			    up->image = imlib_create_image(up->w, up->h);
			    if (up->image)
			      {
				 imlib_context_set_image(up->image);
				 data = imlib_image_get_data();
				 memset(data, 0, up->w * up->h * sizeof(DATA32));
				 imlib_image_put_back_data(data);
				 imlib_image_set_has_alpha(1);
			      }
			 }
		       imlib_context_set_image(up->image);
		       imlib_blend_image_onto_image(im, 1,
						    src_x, src_y, src_w, src_h,
						    dst_x - up->x, dst_y - up->y, dst_w, dst_h);
		    }
	       }
	  }
     }
   if (cm)
     {
	imlib_free_color_modifier();
	imlib_context_set_color_modifier(NULL);
     }
}

int
__evas_image_image_get_width(Evas_Image_Image *im)
{
   imlib_context_set_image((Imlib_Image)im);
   return imlib_image_get_width();
}

int
__evas_image_image_get_height(Evas_Image_Image *im)
{
   imlib_context_set_image((Imlib_Image)im);
   return imlib_image_get_height();
}

void
__evas_image_image_set_borders(Evas_Image_Image *im, int left, int right,
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
__evas_image_image_set_smooth_scaling(int on)
{
   __evas_anti_alias = on;
}



























/*****************************************************************************/
/* font internals ************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* font externals ************************************************************/
/*****************************************************************************/

Evas_Image_Font *
__evas_image_text_font_new(Display *disp, char *font, int size)
{
   char buf[4096];
   
   sprintf(buf, "%s/%i", font, size);
   return (Evas_Image_Font *)imlib_load_font(buf);
}

void
__evas_image_text_font_free(Evas_Image_Font *fn)
{
   imlib_context_set_font((Imlib_Font)fn);
   imlib_free_font();
}

int
__evas_image_text_font_get_ascent(Evas_Image_Font *fn)
{
   imlib_context_set_font((Imlib_Font)fn);
   return imlib_get_font_ascent();
}

int
__evas_image_text_font_get_descent(Evas_Image_Font *fn)
{
   imlib_context_set_font((Imlib_Font)fn);
   return imlib_get_font_descent();
}

int
__evas_image_text_font_get_max_ascent(Evas_Image_Font *fn)
{
   imlib_context_set_font((Imlib_Font)fn);
   return imlib_get_maximum_font_ascent();
}

int
__evas_image_text_font_get_max_descent(Evas_Image_Font *fn)
{
   imlib_context_set_font((Imlib_Font)fn);
   return imlib_get_maximum_font_descent();
}

void
__evas_image_text_font_get_advances(Evas_Image_Font *fn, char *text,
				    int *advance_horiz,
				    int *advance_vert)
{
   imlib_context_set_font((Imlib_Font)fn);
   imlib_get_text_advance(text, advance_horiz, advance_vert);
}

int
__evas_image_text_font_get_first_inset(Evas_Image_Font *fn, char *text)
{
   imlib_context_set_font((Imlib_Font)fn);
   return imlib_get_text_inset(text);
}

void
__evas_image_text_font_add_path(char *path)
{
   imlib_add_path_to_font_path(path);
}

void
__evas_image_text_font_del_path(char *path)
{
   imlib_remove_path_from_font_path(path);
}

char **
__evas_image_text_font_list_paths(int *count)
{
   return imlib_list_font_path(count);
}

void
__evas_image_text_cache_empty(Display *disp)
{
   int size;
   
   size = imlib_get_font_cache_size();
   imlib_set_font_cache_size(0);
   imlib_set_font_cache_size(size);
}

void
__evas_image_text_cache_set_size(Display *disp, int size)
{
   imlib_set_font_cache_size(size);
}

int
__evas_image_text_cache_get_size(Display *disp)
{
   return imlib_get_font_cache_size();
}

void
__evas_image_text_draw(Evas_Image_Font *fn, Display *disp, Imlib_Image dstim, Window win, 
		       int win_w, int win_h, int x, int y, char *text, 
		       int cr, int cg, int cb, int ca)
{
   Evas_List l;
   int w, h;
   
   if (__evas_clip)
     {
	cr = (cr * __evas_clip_r) / 255;
	cg = (cg * __evas_clip_g) / 255;
	cb = (cb * __evas_clip_b) / 255;
	ca = (ca * __evas_clip_a) / 255;
     }   
   if ((!fn) || (!text)) return;
   if (ca == 0) return;
   imlib_context_set_color(cr, cg, cb, ca);
   imlib_context_set_font((Imlib_Font)fn);
   imlib_context_set_angle(0.0);
   imlib_context_set_operation(IMLIB_OP_COPY);
   imlib_context_set_color_modifier(NULL);
   imlib_context_set_direction(IMLIB_TEXT_TO_RIGHT);
   imlib_context_set_anti_alias(1);
   imlib_context_set_blend(1);
   imlib_get_text_size(text, &w, &h);
   for(l = drawable_list; l; l = l->next)
     {
	Evas_Image_Drawable *dr;
	
	dr = l->data;
	
	if (dr->im == dstim)
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_Image_Update *up;

		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      x, y, w, h))
		    {
		       if (__evas_clip) 
			 imlib_context_set_cliprect(__evas_clip_x - up->x, 
						    __evas_clip_y - up->y, 
						    __evas_clip_w, 
						    __evas_clip_h);
		       else imlib_context_set_cliprect(0, 0, 0, 0);
                       if (!up->image)
			 {
			    DATA32 *data;
			    
			    up->image = imlib_create_image(up->w, up->h);
			    if (up->image)
			      {
				 imlib_context_set_image(up->image);
				 data = imlib_image_get_data();
				 memset(data, 0, up->w * up->h * sizeof(DATA32));
				 imlib_image_put_back_data(data);
				 imlib_image_set_has_alpha(1);
			      }
			 }
		       imlib_context_set_image(up->image);
		       imlib_text_draw(x - up->x, y - up->y, text);
		    }
	       }
	  }
     }
}

void
__evas_image_text_get_size(Evas_Image_Font *fn, char *text, int *w, int *h)
{
   if ((!fn) || (!text)) 
      {
	 *w = 0; *h = 0;
	 return;
      }
   imlib_context_set_font((Imlib_Font)fn);
   imlib_get_text_size(text, w, h);
}

int
__evas_image_text_get_character_at_pos(Evas_Image_Font *fn, char *text, 
				       int x, int y, 
				       int *cx, int *cy, int *cw, int *ch)
{
   imlib_context_set_font((Imlib_Font)fn);
   return imlib_text_get_index_and_location(text, x, y, cx, cy, cw, ch);
}

void
__evas_image_text_get_character_number(Evas_Image_Font *fn, char *text, 
				       int num, 
				       int *cx, int *cy, int *cw, int *ch)
{
   imlib_context_set_font((Imlib_Font)fn);
   imlib_text_get_location_at_index(text, num, cx, cy, cw, ch);
}

























/*****************************************************************************/
/* rectangle externals *******************************************************/
/*****************************************************************************/

void              __evas_image_rectangle_draw(Display *disp, Imlib_Image dstim, Window win,
					      int win_w, int win_h,
					      int x, int y, int w, int h,
					      int cr, int cg, int cb, int ca)
{
   Evas_List l;
   
   if (__evas_clip)
     {
	cr = (cr * __evas_clip_r) / 255;
	cg = (cg * __evas_clip_g) / 255;
	cb = (cb * __evas_clip_b) / 255;
	ca = (ca * __evas_clip_a) / 255;
     }
   if (ca == 0) return;
   imlib_context_set_color(cr, cg, cb, ca);
   imlib_context_set_angle(0.0);
   imlib_context_set_operation(IMLIB_OP_COPY);
   imlib_context_set_color_modifier(NULL);
   imlib_context_set_direction(IMLIB_TEXT_TO_RIGHT);
   imlib_context_set_anti_alias(__evas_anti_alias);
   imlib_context_set_blend(1);
   for(l = drawable_list; l; l = l->next)
     {
	Evas_Image_Drawable *dr;
	
	dr = l->data;
	
	if (dr->im == dstim)
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_Image_Update *up;

		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      x, y, w, h))
		    {
		       if (__evas_clip) imlib_context_set_cliprect(__evas_clip_x - up->x, __evas_clip_y - up->y, __evas_clip_w, __evas_clip_h);
		       else imlib_context_set_cliprect(0, 0, 0, 0);
                       if (!up->image)
			 {
			    DATA32 *data;
			    
			    up->image = imlib_create_image(up->w, up->h);
			    if (up->image)
			      {
				 imlib_context_set_image(up->image);
				 data = imlib_image_get_data();
				 memset(data, 0, up->w * up->h * sizeof(DATA32));
				 imlib_image_put_back_data(data);
				 imlib_image_set_has_alpha(1);
			      }
			 }
		       imlib_context_set_image(up->image);
		       imlib_image_fill_rectangle(x - up->x, y - up->y, w, h);
		    }
	       }
	  }
     }
}

















/*****************************************************************************/
/* rectangle externals *******************************************************/
/*****************************************************************************/

void              __evas_image_line_draw(Display *disp, Imlib_Image dstim, Window win,
					 int win_w, int win_h,
					 int x1, int y1, int x2, int y2,
					 int cr, int cg, int cb, int ca)
{
   Evas_List l;
   int x, y, w, h;
   
   if (__evas_clip)
     {
	cr = (cr * __evas_clip_r) / 255;
	cg = (cg * __evas_clip_g) / 255;
	cb = (cb * __evas_clip_b) / 255;
	ca = (ca * __evas_clip_a) / 255;
     }
   if (ca == 0) return;
   imlib_context_set_color(cr, cg, cb, ca);
   imlib_context_set_angle(0.0);
   imlib_context_set_operation(IMLIB_OP_COPY);
   imlib_context_set_color_modifier(NULL);
   imlib_context_set_direction(IMLIB_TEXT_TO_RIGHT);
   imlib_context_set_anti_alias(1);
   imlib_context_set_blend(1);
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
	Evas_Image_Drawable *dr;
	
	dr = l->data;
	
	if (dr->im == dstim)
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_Image_Update *up;

		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      x, y, w, h))
		    {
		       if (__evas_clip) imlib_context_set_cliprect(__evas_clip_x - up->x, __evas_clip_y - up->y, __evas_clip_w, __evas_clip_h);
		       else imlib_context_set_cliprect(0, 0, 0, 0);
                       if (!up->image)
			 {
			    DATA32 *data;
			    
			    up->image = imlib_create_image(up->w, up->h);
			    if (up->image)
			      {
				 imlib_context_set_image(up->image);
				 data = imlib_image_get_data();
				 memset(data, 0, up->w * up->h * sizeof(DATA32));
				 imlib_image_put_back_data(data);
				 imlib_image_set_has_alpha(1);
			      }
			 }
		       imlib_context_set_image(up->image);
		       imlib_image_draw_line(x1 - up->x, y1 - up->y, x2 - up->x, y2 - up->y, 0);
		    }
	       }
	  }
     }
}

















/****************************************************************************/
/* gradient externals ********************************************************/
/*****************************************************************************/


Evas_Image_Graident *
__evas_image_gradient_new(Display *disp)
{
   Evas_Image_Graident *gr;
   
   gr = malloc(sizeof(Evas_Image_Graident));
   gr->colors = NULL;
}

void
__evas_image_gradient_free(Evas_Image_Graident *gr)
{
   Evas_List l;
   
   if (gr->colors)
     {
	for (l = gr->colors; l; l = l->next)
	  {
	     free(l->data);
	  }
	evas_list_free(gr->colors);
     }
   free(gr);
}

void
__evas_image_gradient_color_add(Evas_Image_Graident *gr, int r, int g, int b, int a, int dist)
{
   Evas_Image_Color *cl;
   
   cl = malloc(sizeof(Evas_Image_Color));
   cl->r = r;
   cl->g = g;
   cl->b = b;
   cl->a = a;
   cl->dist = dist;
   gr->colors = evas_list_append(gr->colors, cl);
}

void
__evas_image_gradient_draw(Evas_Image_Graident *gr, Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x, int y, int w, int h, double angle)
{
   Evas_List l;
   Imlib_Color_Range cr;

   if ((__evas_clip) && (__evas_clip_a == 0)) return;
   imlib_context_set_angle(angle);
   imlib_context_set_operation(IMLIB_OP_COPY);
   imlib_context_set_color_modifier(NULL);
   imlib_context_set_direction(IMLIB_TEXT_TO_RIGHT);
   imlib_context_set_anti_alias(1);
   imlib_context_set_blend(1);
   cr = imlib_create_color_range();
   imlib_context_set_color_range(cr);
     {
	Evas_List l;
	
	for (l = gr->colors; l; l = l->next)
	  {
	     Evas_Image_Color *cl;
	     
	     cl = l->data;
	     if (__evas_clip)
	       imlib_context_set_color((cl->r * __evas_clip_r) / 255, 
				       (cl->g * __evas_clip_g) / 255, 
				       (cl->b * __evas_clip_b) / 255, 
				       (cl->a * __evas_clip_a) / 255);
	     else
	       imlib_context_set_color(cl->r, cl->g, cl->b, cl->a);	       
	     imlib_add_color_to_color_range(cl->dist);
	  }
     }
   for(l = drawable_list; l; l = l->next)
     {
	Evas_Image_Drawable *dr;
	
	dr = l->data;
	
	if (dr->im == dstim)
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_Image_Update *up;

		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      x, y, w, h))
		    {
		       if (__evas_clip) imlib_context_set_cliprect(__evas_clip_x - up->x, __evas_clip_y - up->y, __evas_clip_w, __evas_clip_h);
		       else imlib_context_set_cliprect(0, 0, 0, 0);
                       if (!up->image)
			 {
			    DATA32 *data;
			    
			    up->image = imlib_create_image(up->w, up->h);
			    if (up->image)
			      {
				 imlib_context_set_image(up->image);
				 data = imlib_image_get_data();
				 memset(data, 0, up->w * up->h * sizeof(DATA32));
				 imlib_image_put_back_data(data);
				 imlib_image_set_has_alpha(1);
			      }
			 }
		       imlib_context_set_image(up->image);
		       imlib_image_fill_color_range_rectangle(x - up->x, y - up->y, w, h, angle);
		    }
	       }
	  }
     }
   imlib_free_color_range();
}




/************/
/* polygons */
/************/
void
__evas_image_poly_draw (Display *disp, Imlib_Image dstim, Window win, 
			int win_w, int win_h, 
			Evas_List points, 
			int cr, int cg, int cb, int ca)
{
   Evas_List l, l2;
   int x, y, w, h;

   if (__evas_clip)
     {
	cr = (cr * __evas_clip_r) / 255;
	cg = (cg * __evas_clip_g) / 255;
	cb = (cb * __evas_clip_b) / 255;
	ca = (ca * __evas_clip_a) / 255;
     }
   if (ca == 0) return;
   imlib_context_set_color(cr, cg, cb, ca);
   imlib_context_set_angle(0.0);
   imlib_context_set_operation(IMLIB_OP_COPY);
   imlib_context_set_color_modifier(NULL);
   imlib_context_set_direction(IMLIB_TEXT_TO_RIGHT);
   imlib_context_set_anti_alias(1);
   imlib_context_set_blend(1);
   
   x = y = w = h = 0;
   if (points)
     {
	Evas_Point p;
	
	p = points->data;
	x = p->x;
	y = p->y;
	w = 1;
	h = 1;
     }
   for (l2 = points; l2; l2 = l2->next)
     {
	Evas_Point p;
	
	p = l2->data;
	if (p->x < x) 
	   {
	      w += x - p->x;
	      x = p->x;
	   }
	if (p->x > (x + w)) 
	   w = p->x - x;
	if (p->y < y) 
	   {
	      h += y - p->y;
	      y = p->y;
	   }
	if (p->y > (y + h)) 
	   h = p->y - y;
     }
   for(l = drawable_list; l; l = l->next)
     {
	Evas_Image_Drawable *dr;
	
	dr = l->data;
	
	if (dr->im == dstim)
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_Image_Update *up;

		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      x, y, w, h))
		    {
		       ImlibPolygon pol;
		       
		       if (__evas_clip) imlib_context_set_cliprect(__evas_clip_x - up->x, __evas_clip_y - up->y, __evas_clip_w, __evas_clip_h);
		       else imlib_context_set_cliprect(0, 0, 0, 0);
		       
                       if (!up->image)
			 {
			    DATA32 *data;
			    
			    up->image = imlib_create_image(up->w, up->h);
			    if (up->image)
			      {
				 imlib_context_set_image(up->image);
				 data = imlib_image_get_data();
				 memset(data, 0, up->w * up->h * sizeof(DATA32));
				 imlib_image_put_back_data(data);
				 imlib_image_set_has_alpha(1);
			      }
			 }
		       imlib_context_set_image(up->image);
		       pol = imlib_polygon_new();
		       for (l2 = points; l2; l2 = l2->next)
			 {
			    Evas_Point p;
			    
			    p = l2->data;
			    imlib_polygon_add_point(pol, p->x - up->x, p->y - up->y);
			 }
		       imlib_image_fill_polygon(pol);
		       imlib_polygon_free(pol);
		    }
	       }
	  }
     }
}













/*****************************************************************************/
/* general externals *********************************************************/
/*****************************************************************************/

static Visual *__evas_visual = NULL;
static Colormap __evas_cmap = 0;

void
__evas_image_set_clip_rect(int on, int x, int y, int w, int h, int r, int g, int b, int a)
{    
   __evas_clip = on;
   __evas_clip_x = x;
   __evas_clip_y = y;
   __evas_clip_w = w;
   __evas_clip_h = h;
   __evas_clip_r = r;
   __evas_clip_g = g;
   __evas_clip_b = b;
   __evas_clip_a = a;
}

void
__evas_image_sync(Display *disp)
{
}

void
__evas_image_flush_draw(Display *disp, Imlib_Image dstim, Window win)
{
   Evas_List l;
   
   imlib_context_set_blend(1);
   imlib_context_set_color_modifier(NULL);
   imlib_context_set_cliprect(0, 0, 0, 0);
   imlib_context_set_angle(0);
   
   for(l = drawable_list; l; l = l->next)
     {
	Evas_Image_Drawable *dr;
	
	dr = l->data;
	
	if (dr->im == dstim)
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_Image_Update *up;
		  
		  up = ll->data;
		  
		  if (up->image)
		    {
                       int w, h;
		       
		       w = up->w; h = up->h;
		       imlib_context_set_image(dr->im);
		       imlib_blend_image_onto_image(up->image, 1,
						    0, 0, w, h,
						    up->x, up->y, w, h);
		       imlib_context_set_image(up->image);
		       imlib_free_image();
		    }
		  free(up);
	       }
	     if (dr->tmp_images)
		dr->tmp_images = evas_list_free(dr->tmp_images);
	  }
	free(dr);
     }
   if (drawable_list)
      drawable_list = evas_list_free(drawable_list);
   drawable_list = NULL;
}

void
__evas_image_set_vis_cmap(Visual *vis, Colormap cmap)
{
}
   
int
__evas_image_capable(Display *disp)
{
   return 1;
}

Visual *
__evas_image_get_visual(Display *disp, int screen)
{
   int depth;
   
   __evas_visual = imlib_get_best_visual(disp, screen, &depth);
   return __evas_visual;
}

XVisualInfo *
__evas_image_get_visual_info(Display *disp, int screen)
{
   static XVisualInfo *vi = NULL;
   XVisualInfo vi_template;
   int n;
   
   if (vi) return vi;
   vi_template.visualid = (__evas_image_get_visual(disp, screen))->visualid;
   vi_template.screen = screen;
   vi = XGetVisualInfo(disp, VisualIDMask | VisualScreenMask, &vi_template ,&n);
   return vi;
}

Colormap
__evas_image_get_colormap(Display *disp, int screen)
{
   Visual *v;
   
   if (__evas_cmap) return __evas_cmap;
   v = __evas_image_get_visual(disp, screen);
   __evas_cmap = DefaultColormap(disp, screen);
   return __evas_cmap;
   __evas_cmap = XCreateColormap(disp, RootWindow(disp, screen), v, AllocNone);
   return __evas_cmap;
}

void
__evas_image_init(Display *disp, int screen, int colors)
{
   static int initted = 0;
   
   if (!initted)
     {
	imlib_set_font_cache_size(1024 * 1024);
	imlib_set_cache_size(8 * 1024 * 1024);
	initted = 1;
     }
}

void
__evas_image_draw_add_rect(Display *disp, Imlib_Image dstim, Window win, 
			   int x, int y, int w, int h)
{
   Evas_List l;
   
   for(l = drawable_list; l; l = l->next)
     {
	Evas_Image_Drawable *dr;
	
	dr = l->data;
	
	if (dr->im == dstim)
	  {
	     Evas_Image_Update *up;
	     
	     up = malloc(sizeof(Evas_Image_Update));
	     up->x = x;
	     up->y = y;
	     up->w = w;
	     up->h = h;
	     up->image = NULL;
	     dr->tmp_images = evas_list_append(dr->tmp_images, up);
	  }
	return;
     }
     {
	Evas_Image_Drawable *dr;
	Evas_Image_Update *up;
	
	dr = malloc(sizeof(Evas_Image_Drawable));
	dr->im = dstim;
	dr->tmp_images = NULL;
	up = malloc(sizeof(Evas_Image_Update));
	up->x = x;
	up->y = y;
	up->w = w;
	up->h = h;
	up->image = NULL;
	drawable_list = evas_list_append(drawable_list, dr);
	dr->tmp_images = evas_list_append(dr->tmp_images, up);
     }
}

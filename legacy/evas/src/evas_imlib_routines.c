#include "evas_imlib_routines.h"

static void __evas_imlib_image_cache_flush(Display *disp);
static int  __evas_anti_alias = 1;
static Evas_List drawable_list = NULL;

/*****************************************************************************/
/* image internals ***********************************************************/
/*****************************************************************************/

static void
__evas_imlib_image_cache_flush(Display *disp)
{
   int size;
   
   size = imlib_get_cache_size();
   imlib_set_cache_size(0);
   imlib_set_cache_size(size);
}

/*****************************************************************************/
/* image externals ***********************************************************/
/*****************************************************************************/

Evas_Imlib_Image *
__evas_imlib_image_new_from_file(Display *disp, char *file)
{	
   return (Evas_Imlib_Image *)imlib_load_image(file);
}

void
__evas_imlib_image_free(Evas_Imlib_Image *im)
{
   imlib_context_set_image((Imlib_Image)im);
   imlib_free_image();
}

void
__evas_imlib_image_cache_empty(Display *disp)
{
   int size;
   
   size = imlib_get_cache_size();
   imlib_set_cache_size(0);
   imlib_set_cache_size(size);
}

void
__evas_imlib_image_cache_set_size(Display *disp, int size)
{
   imlib_set_cache_size(size);
}

int
__evas_imlib_image_cache_get_size(Display *disp)
{
   return imlib_get_cache_size();
}

void
__evas_imlib_image_draw(Evas_Imlib_Image *im, 
			Display *disp, Window w, int win_w, int win_h,
			int src_x, int src_y, int src_w, int src_h,
			int dst_x, int dst_y, int dst_w, int dst_h)
{
   Evas_List l;
   
   imlib_context_set_angle(0.0);
   imlib_context_set_operation(IMLIB_OP_COPY);
   imlib_context_set_color_modifier(NULL);
   imlib_context_set_direction(IMLIB_TEXT_TO_RIGHT);
   imlib_context_set_anti_alias(__evas_anti_alias);
   for(l = drawable_list; l; l = l->next)
     {
	Evas_Imlib_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == w) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_Imlib_Update *up;
		  
		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      dst_x, dst_y, dst_w, dst_h))
		    {
		       if (!up->image)
			  up->image = imlib_create_image(up->w, up->h);
		       imlib_context_set_image(up->image);
		       imlib_blend_image_onto_image(im, 0,
						    src_x, src_y, src_w, src_h,
						    dst_x - up->x, dst_y - up->y, dst_w, dst_h);
		    }
	       }
	  }
     }
}

int
__evas_imlib_image_get_width(Evas_Imlib_Image *im)
{
   imlib_context_set_image((Imlib_Image)im);
   return imlib_image_get_width();
}

int
__evas_imlib_image_get_height(Evas_Imlib_Image *im)
{
   imlib_context_set_image((Imlib_Image)im);
   return imlib_image_get_height();
}

void
__evas_imlib_image_set_borders(Evas_Imlib_Image *im, int left, int right,
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
__evas_imlib_image_set_smooth_scaling(int on)
{
   __evas_anti_alias = on;
}



























/*****************************************************************************/
/* font internals ************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* font externals ************************************************************/
/*****************************************************************************/

Evas_Imlib_Font *
__evas_imlib_text_font_new(Display *disp, char *font, int size)
{
   char buf[4096];
   
   sprintf(buf, "%s/%i", font, size);
   return (Evas_Imlib_Font *)imlib_load_font(buf);
}

void
__evas_imlib_text_font_free(Evas_Imlib_Font *fn)
{
   imlib_context_set_font((Imlib_Font)fn);
   imlib_free_font();
}

void
__evas_imlib_text_font_add_path(char *path)
{
   imlib_add_path_to_font_path(path);
}

void
__evas_imlib_text_font_del_path(char *path)
{
   imlib_remove_path_from_font_path(path);
}

char **
__evas_imlib_text_font_list_paths(int *count)
{
   return imlib_list_font_path(count);
}

void
__evas_imlib_text_cache_empty(Display *disp)
{
   int size;
   
   size = imlib_get_font_cache_size();
   imlib_set_font_cache_size(0);
   imlib_set_font_cache_size(size);
}

void
__evas_imlib_text_cache_set_size(Display *disp, int size)
{
   imlib_set_font_cache_size(size);
}

int
__evas_imlib_text_cache_get_size(Display *disp)
{
   return imlib_get_font_cache_size();
}

void
__evas_imlib_text_draw(Evas_Imlib_Font *fn, Display *disp, Window win, 
		       int win_w, int win_h, int x, int y, char *text, 
		       int r, int g, int b, int a)
{
   Evas_List l;
   int w, h;
   
   if ((!fn) || (!text)) return;
   imlib_context_set_color(r, g, b, a);
   imlib_context_set_font((Imlib_Font)fn);
   imlib_context_set_angle(0.0);
   imlib_context_set_operation(IMLIB_OP_COPY);
   imlib_context_set_color_modifier(NULL);
   imlib_context_set_direction(IMLIB_TEXT_TO_RIGHT);
   imlib_context_set_anti_alias(1);
   imlib_get_text_size(text, &w, &h);
   for(l = drawable_list; l; l = l->next)
     {
	Evas_Imlib_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_Imlib_Update *up;

		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      x, y, w, h))
		    {
		       if (!up->image)
			  up->image = imlib_create_image(up->w, up->h);
		       imlib_context_set_image(up->image);
		       imlib_text_draw(x - up->x, y - up->y, text);
		    }
	       }
	  }
     }
}

void
__evas_imlib_text_get_size(Evas_Imlib_Font *fn, char *text, int *w, int *h)
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
__evas_imlib_text_get_character_at_pos(Evas_Imlib_Font *fn, char *text, 
				       int x, int y, 
				       int *cx, int *cy, int *cw, int *ch)
{
   imlib_context_set_font((Imlib_Font)fn);
   return imlib_text_get_index_and_location(text, x, y, cx, cy, cw, ch);
}

void
__evas_imlib_text_get_character_number(Evas_Imlib_Font *fn, char *text, 
				       int num, 
				       int *cx, int *cy, int *cw, int *ch)
{
   imlib_context_set_font((Imlib_Font)fn);
   imlib_text_get_location_at_index(text, num, cx, cy, cw, ch);
}

























/*****************************************************************************/
/* rectangle externals *******************************************************/
/*****************************************************************************/

void              __evas_imlib_rectangle_draw(Display *disp, Window win,
					      int win_w, int win_h,
					      int x, int y, int w, int h,
					      int r, int g, int b, int a)
{
   Evas_List l;
   
   imlib_context_set_color(r, g, b, a);
   imlib_context_set_angle(0.0);
   imlib_context_set_operation(IMLIB_OP_COPY);
   imlib_context_set_color_modifier(NULL);
   imlib_context_set_direction(IMLIB_TEXT_TO_RIGHT);
   imlib_context_set_anti_alias(__evas_anti_alias);
   for(l = drawable_list; l; l = l->next)
     {
	Evas_Imlib_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_Imlib_Update *up;

		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      x, y, w, h))
		    {
		       if (!up->image)
			  up->image = imlib_create_image(up->w, up->h);
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

void              __evas_imlib_line_draw(Display *disp, Window win,
					 int win_w, int win_h,
					 int x1, int y1, int x2, int y2,
					 int r, int g, int b, int a)
{
   Evas_List l;
   int x, y, w, h;
   
   imlib_context_set_color(r, g, b, a);
   imlib_context_set_angle(0.0);
   imlib_context_set_operation(IMLIB_OP_COPY);
   imlib_context_set_color_modifier(NULL);
   imlib_context_set_direction(IMLIB_TEXT_TO_RIGHT);
   imlib_context_set_anti_alias(1);
   w = x2 - x1;
   if (w < 0) w = -w;
   h = y2 - y1;
   if (h < 0) h = -h;
   if (x1 < x2) x = x1;
   else x = x2;
   if (y1 < y2) y = y1;
   else y = y2;
   for(l = drawable_list; l; l = l->next)
     {
	Evas_Imlib_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_Imlib_Update *up;

		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      x, y, w, h))
		    {
		       if (!up->image)
			  up->image = imlib_create_image(up->w, up->h);
		       imlib_context_set_image(up->image);
		       imlib_image_draw_line(x1 - up->x, y1 - up->y, x2 - up->x, y2 - up->y, 0);
		    }
	       }
	  }
     }
}

















/*****************************************************************************/
/* gradient externals ********************************************************/
/*****************************************************************************/


Evas_Imlib_Graident *
__evas_imlib_gradient_new(Display *disp)
{
   return (Evas_Imlib_Graident *)imlib_create_color_range();
}

void
__evas_imlib_gradient_free(Evas_Imlib_Graident *gr)
{
   imlib_context_set_color_range((Imlib_Color_Range)gr);
   imlib_free_color_range();
}

void
__evas_imlib_gradient_color_add(Evas_Imlib_Graident *gr, int r, int g, int b, int a, int dist)
{
   imlib_context_set_color_range((Imlib_Color_Range)gr);
   imlib_context_set_color(r, g, b, a);
   imlib_add_color_to_color_range(dist);
}

void
__evas_imlib_gradient_draw(Evas_Imlib_Graident *gr, Display *disp, Window win, int win_w, int win_h, int x, int y, int w, int h, double angle)
{
   Evas_List l;
   
   imlib_context_set_angle(angle);
   imlib_context_set_operation(IMLIB_OP_COPY);
   imlib_context_set_color_modifier(NULL);
   imlib_context_set_direction(IMLIB_TEXT_TO_RIGHT);
   imlib_context_set_color_range((Imlib_Color_Range)gr);
   imlib_context_set_anti_alias(1);
   for(l = drawable_list; l; l = l->next)
     {
	Evas_Imlib_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_Imlib_Update *up;

		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      x, y, w, h))
		    {
		       if (!up->image)
			  up->image = imlib_create_image(up->w, up->h);
		       imlib_context_set_image(up->image);
		       imlib_image_fill_color_range_rectangle(x - up->x, y - up->y, w, h, angle);
		    }
	       }
	  }
     }
}

















/*****************************************************************************/
/* general externals *********************************************************/
/*****************************************************************************/

static Visual *__evas_visual;
static Colormap __evas_cmap;

void
__evas_imlib_sync(Display *disp)
{
   XSync(disp, False);
}

void
__evas_imlib_flush_draw(Display *disp, Window win)
{
   Evas_List l;
   
   imlib_context_set_display(disp);
   imlib_context_set_visual(__evas_visual);
   imlib_context_set_colormap(__evas_cmap);
   imlib_context_set_drawable(win);
   imlib_context_set_dither(1);
   
   for(l = drawable_list; l; l = l->next)
     {
	Evas_Imlib_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_Imlib_Update *up;
		  
		  up = ll->data;
		  
		  if (up->image)
		    {
		       imlib_context_set_image(up->image);
		       imlib_render_image_on_drawable(up->x, up->y);
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

   
   int
__evas_imlib_capable(Display *disp)
{
   return 1;
}

Visual *
__evas_imlib_get_visual(Display *disp, int screen)
{
   int depth;
   
   __evas_visual = imlib_get_best_visual(disp, screen, &depth);
   return __evas_visual;
}

XVisualInfo *
__evas_imlib_get_visual_info(Display *disp, int screen)
{
   static XVisualInfo *vi = NULL;
   XVisualInfo vi_template;
   int n;
   
   if (vi) return vi;
   vi_template.visualid = (__evas_imlib_get_visual(disp, screen))->visualid;
   vi_template.screen = screen;
   vi = XGetVisualInfo(disp, VisualIDMask | VisualScreenMask, &vi_template ,&n);
   return vi;
}

Colormap
__evas_imlib_get_colormap(Display *disp, int screen)
{
   __evas_cmap = DefaultColormap(disp, screen);
   return __evas_cmap;
}

void
__evas_imlib_init(Display *disp, int screen)
{
   imlib_set_font_cache_size(1024 * 1024);
   imlib_set_cache_size(8 * 1024 * 1024);
   imlib_set_color_usage(128);
   __evas_imlib_get_visual(disp, screen);
   __evas_imlib_get_visual(disp, screen);
   __evas_imlib_get_colormap(disp, screen);
}

void
__evas_imlib_draw_add_rect(Display *disp, Window win, 
			   int x, int y, int w, int h)
{
   Evas_List l;
   
   for(l = drawable_list; l; l = l->next)
     {
	Evas_Imlib_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_Imlib_Update *up;
	     
	     up = malloc(sizeof(Evas_Imlib_Update));
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
	Evas_Imlib_Drawable *dr;
	Evas_Imlib_Update *up;
	
	dr = malloc(sizeof(Evas_Imlib_Drawable));
	dr->win = win;
	dr->disp = disp;
	dr->tmp_images = NULL;
	up = malloc(sizeof(Evas_Imlib_Update));
	up->x = x;
	up->y = y;
	up->w = w;
	up->h = h;
	up->image = NULL;
	drawable_list = evas_list_append(drawable_list, dr);
	dr->tmp_images = evas_list_append(dr->tmp_images, up);
     }
}

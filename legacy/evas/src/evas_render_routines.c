#include "evas_render_routines.h"


#ifdef HAVE_RENDER

static void __evas_render_image_cache_flush(Display *disp);
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
__evas_render_image_cache_flush(Display *disp)
{
   int size;
   
   size = imlib_get_cache_size();
   imlib_set_cache_size(0);
   imlib_set_cache_size(size);
}

/*****************************************************************************/
/* image externals ***********************************************************/
/*****************************************************************************/

Evas_Render_Image *
__evas_render_image_new_from_file(Display *disp, char *file)
{
   Evas_Render_Image *im;
   Imlib_Image i;
   XRenderPictFormat fmt, *format, *format_color;
   Visual *visual;
   int screen;

   /* need to look for image i local cache */
   /* not found - load */
   i = imlib_load_image(file);
   if (!i) return NULL;
   imlib_context_set_image(i);
   im = malloc(sizeof(Evas_Render_Image));
   memset(im, 0, sizeof(Evas_Render_Image));
   /* this stuff needs to become a context lookup for the display pointer */
   visual = DefaultVisual (disp, screen);
   format = XRenderFindVisualFormat (disp, visual);
   fmt.depth = 32;
   fmt.type = PictTypeDirect;
   format_color = XRenderFindFormat(disp, PictFormatType | PictFormatDepth, &fmt, 0);
   
   im->file = malloc(strlen(file) + 1);
   strcpy(im->file, file);
   im->references = 1;
   im->disp = disp;
   im->has_alpha = imlib_image_has_alpha();
   im->w = imlib_image_get_width();
   im->h = imlib_image_get_height();
   im->pmap = XCreatePixmap (disp, RootWindow (disp, screen), im->w, im->h, fmt.depth);
   im->pic = XRenderCreatePicture (disp, im->pmap, &fmt, 0, 0);
   
   /* need to xshmputimage to the pixmap */
   /* need to add to local cache list */
   return im;
}

void
__evas_render_image_free(Evas_Render_Image *im)
{
   im->references--;
   if (im->references <= 0)
     {
	/* need to flush cache */
     }
}

void
__evas_render_image_cache_empty(Display *disp)
{
   int size;
   
   size = imlib_get_cache_size();
   imlib_set_cache_size(0);
   imlib_set_cache_size(size);
}

void
__evas_render_image_cache_set_size(Display *disp, int size)
{
   imlib_set_cache_size(size);
}

int
__evas_render_image_cache_get_size(Display *disp)
{
   return imlib_get_cache_size();
}

void
__evas_render_image_draw(Evas_Render_Image *im, 
			Display *disp, Imlib_Image dstim, Window w, int win_w, int win_h,
			int src_x, int src_y, int src_w, int src_h,
			int dst_x, int dst_y, int dst_w, int dst_h,
			int cr, int cg, int cb, int ca)
{
   Evas_List l;

   if (ca == 0) return;
   
   for(l = drawable_list; l; l = l->next)
     {
	Evas_Render_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == w) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_Render_Update *up;
		  
		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      dst_x, dst_y, dst_w, dst_h))
		    {
		    }
	       }
	  }
     }
}

int
__evas_render_image_get_width(Evas_Render_Image *im)
{
   return 0;
}

int
__evas_render_image_get_height(Evas_Render_Image *im)
{
   return 0;
}

void
__evas_render_image_set_borders(Evas_Render_Image *im, int left, int right,
			       int top, int bottom)
{
}

void
__evas_render_image_set_smooth_scaling(int on)
{
   __evas_anti_alias = on;
}



























/*****************************************************************************/
/* font internals ************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* font externals ************************************************************/
/*****************************************************************************/

Evas_Render_Font *
__evas_render_text_font_new(Display *disp, char *font, int size)
{
   return NULL;
}

void
__evas_render_text_font_free(Evas_Render_Font *fn)
{
}

int
__evas_render_text_font_get_ascent(Evas_Render_Font *fn)
{
   return 0;
}

int
__evas_render_text_font_get_descent(Evas_Render_Font *fn)
{
   return 0;
}

int
__evas_render_text_font_get_max_ascent(Evas_Render_Font *fn)
{
   return 0;
}

int
__evas_render_text_font_get_max_descent(Evas_Render_Font *fn)
{
   return 0;
}

void
__evas_render_text_font_get_advances(Evas_Render_Font *fn, char *text,
				    int *advance_horiz,
				    int *advance_vert)
{
}

int
__evas_render_text_font_get_first_inset(Evas_Render_Font *fn, char *text)
{
   return 0;
}

void
__evas_render_text_font_add_path(char *path)
{
}

void
__evas_render_text_font_del_path(char *path)
{
}

char **
__evas_render_text_font_list_paths(int *count)
{
   return NULL;
}

void
__evas_render_text_cache_empty(Display *disp)
{
}

void
__evas_render_text_cache_set_size(Display *disp, int size)
{
}

int
__evas_render_text_cache_get_size(Display *disp)
{
   return 0;
}

void
__evas_render_text_draw(Evas_Render_Font *fn, Display *disp, Imlib_Image dstim, Window win, 
		       int win_w, int win_h, int x, int y, char *text, 
		       int r, int g, int b, int a)
{
   Evas_List l;
   int w, h;
   
   if ((!fn) || (!text)) return;
   for(l = drawable_list; l; l = l->next)
     {
	Evas_Render_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_Render_Update *up;

		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      x, y, w, h))
		    {
		    }
	       }
	  }
     }
}

void
__evas_render_text_get_size(Evas_Render_Font *fn, char *text, int *w, int *h)
{
   if ((!fn) || (!text)) 
      {
	 *w = 0; *h = 0;
	 return;
      }
}

int
__evas_render_text_get_character_at_pos(Evas_Render_Font *fn, char *text, 
				       int x, int y, 
				       int *cx, int *cy, int *cw, int *ch)
{
   return -1;
}

void
__evas_render_text_get_character_number(Evas_Render_Font *fn, char *text, 
				       int num, 
				       int *cx, int *cy, int *cw, int *ch)
{
}

























/*****************************************************************************/
/* rectangle externals *******************************************************/
/*****************************************************************************/

void              __evas_render_rectangle_draw(Display *disp, Imlib_Image dstim, Window win,
					      int win_w, int win_h,
					      int x, int y, int w, int h,
					      int r, int g, int b, int a)
{
   Evas_List l;
   for(l = drawable_list; l; l = l->next)
     {
	Evas_Render_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_Render_Update *up;

		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      x, y, w, h))
		    {
		    }
	       }
	  }
     }
}

















/*****************************************************************************/
/* rectangle externals *******************************************************/
/*****************************************************************************/

void              __evas_render_line_draw(Display *disp, Imlib_Image dstim, Window win,
					 int win_w, int win_h,
					 int x1, int y1, int x2, int y2,
					 int r, int g, int b, int a)
{
   Evas_List l;
   int x, y, w, h;
   
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
	Evas_Render_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_Render_Update *up;

		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      x, y, w, h))
		    {
		    }
	       }
	  }
     }
}

















/****************************************************************************/
/* gradient externals ********************************************************/
/*****************************************************************************/


Evas_Render_Graident *
__evas_render_gradient_new(Display *disp)
{
   return NULL;
}

void
__evas_render_gradient_free(Evas_Render_Graident *gr)
{
}

void
__evas_render_gradient_color_add(Evas_Render_Graident *gr, int r, int g, int b, int a, int dist)
{
}

void
__evas_render_gradient_draw(Evas_Render_Graident *gr, Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x, int y, int w, int h, double angle)
{
   Evas_List l;
   
   for(l = drawable_list; l; l = l->next)
     {
	Evas_Render_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_Render_Update *up;

		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      x, y, w, h))
		    {
		    }
	       }
	  }
     }
}




/************/
/* polygons */
/************/
void
__evas_render_poly_draw (Display *disp, Imlib_Image dstim, Window win, 
			int win_w, int win_h, 
			Evas_List points, 
			int r, int g, int b, int a)
{
   Evas_List l, l2;
   int x, y, w, h;
   
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
	Evas_Render_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_Render_Update *up;

		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      x, y, w, h))
		    {
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
__evas_render_set_clip_rect(int on, int x, int y, int w, int h, int r, int g, int b, int a)
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
__evas_render_sync(Display *disp)
{
   XSync(disp, False);
}

void
__evas_render_flush_draw(Display *disp, Imlib_Image dstim, Window win)
{
   Evas_List l;
   
   for(l = drawable_list; l; l = l->next)
     {
	Evas_Render_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_Render_Update *up;
		  
		  up = ll->data;
		  
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
__evas_render_set_vis_cmap(Visual *vis, Colormap cmap)
{
   __evas_visual = vis;
   __evas_cmap = cmap;
}
   
int
__evas_render_capable(Display *disp)
{
   return 1;
}

Visual *
__evas_render_get_visual(Display *disp, int screen)
{
   int depth;
   
   __evas_visual = imlib_get_best_visual(disp, screen, &depth);
   return __evas_visual;
}

XVisualInfo *
__evas_render_get_visual_info(Display *disp, int screen)
{
   static XVisualInfo *vi = NULL;
   XVisualInfo vi_template;
   int n;
   
   if (vi) return vi;
   vi_template.visualid = (__evas_render_get_visual(disp, screen))->visualid;
   vi_template.screen = screen;
   vi = XGetVisualInfo(disp, VisualIDMask | VisualScreenMask, &vi_template ,&n);
   return vi;
}

Colormap
__evas_render_get_colormap(Display *disp, int screen)
{
   Visual *v;
   
   if (__evas_cmap) return __evas_cmap;
   v = __evas_render_get_visual(disp, screen);
   __evas_cmap = DefaultColormap(disp, screen);
   return __evas_cmap;
   __evas_cmap = XCreateColormap(disp, RootWindow(disp, screen), v, AllocNone);
   return __evas_cmap;
}

void
__evas_render_init(Display *disp, int screen, int colors)
{
   static int initted = 0;
   
   if (!initted)
     {
	initted = 1;
     }
}

void
__evas_render_draw_add_rect(Display *disp, Imlib_Image dstim, Window win, 
			   int x, int y, int w, int h)
{
   Evas_List l;
   
   for(l = drawable_list; l; l = l->next)
     {
	Evas_Render_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_Render_Update *up;
	     
	     up = malloc(sizeof(Evas_Render_Update));
	     up->x = x;
	     up->y = y;
	     up->w = w;
	     up->h = h;
	     dr->tmp_images = evas_list_append(dr->tmp_images, up);
	  }
	return;
     }
     {
	Evas_Render_Drawable *dr;
	Evas_Render_Update *up;
	
	dr = malloc(sizeof(Evas_Render_Drawable));
	dr->win = win;
	dr->disp = disp;
	dr->tmp_images = NULL;
	up = malloc(sizeof(Evas_Render_Update));
	up->x = x;
	up->y = y;
	up->w = w;
	up->h = h;
	drawable_list = evas_list_append(drawable_list, dr);
	dr->tmp_images = evas_list_append(dr->tmp_images, up);
     }
}

#else

/***************/
/* image stuff */
/***************/
Evas_Render_Image *__evas_render_image_new_from_file(Display *disp, char *file){return NULL;}
void           __evas_render_image_free(Evas_Render_Image *im){}
void           __evas_render_image_cache_empty(Display *disp){}
void           __evas_render_image_cache_set_size(Display *disp, int size){}
int            __evas_render_image_cache_get_size(Display *disp){return 0;}
int            __evas_render_image_get_width(Evas_Render_Image *im){return 0;}
int            __evas_render_image_get_height(Evas_Render_Image *im){return 0;}
void           __evas_render_image_set_borders(Evas_Render_Image *im, int left, int right, int top, int bottom){}
void           __evas_render_image_set_smooth_scaling(int on){}
void           __evas_render_image_draw(Evas_Render_Image *im, Display *disp, Imlib_Image dstim, Window w, int win_w, int win_h, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int cr, int cg, int cb, int ca){}

/********/
/* text */
/********/
Evas_Render_Font  *__evas_render_text_font_new(Display *disp, char *font, int size){return NULL;}
void           __evas_render_text_font_free(Evas_Render_Font *fn){}
void           __evas_render_text_font_add_path(char *path){}
int            __evas_render_text_font_get_ascent(Evas_Render_Font *fn){return 0;}
int            __evas_render_text_font_get_descent(Evas_Render_Font *fn){return 0;}
int            __evas_render_text_font_get_max_ascent(Evas_Render_Font *fn){return 0;}
int            __evas_render_text_font_get_max_descent(Evas_Render_Font *fn){return 0;}
void           __evas_render_text_font_get_advances(Evas_Render_Font *fn, char *text, int *advance_horiz, int *advance_vert){}
int            __evas_render_text_font_get_first_inset(Evas_Render_Font *fn, char *text){return 0;}
void           __evas_render_text_font_del_path(char *path){}
char         **__evas_render_text_font_list_paths(int *count){return NULL;}
void           __evas_render_text_cache_empty(Display *disp){}
void           __evas_render_text_cache_set_size(Display *disp, int size){}
int            __evas_render_text_cache_get_size(Display *disp){return 0;}
void           __evas_render_text_get_size(Evas_Render_Font *fn, char *text, int *w, int *h){}
int            __evas_render_text_get_character_at_pos(Evas_Render_Font *fn, char *text, int x, int y, int *cx, int *cy, int *cw, int *ch){return 0;}
void           __evas_render_text_get_character_number(Evas_Render_Font *fn, char *text, int num, int *cx, int *cy, int *cw, int *ch){}
void           __evas_render_text_draw(Evas_Render_Font *fn, Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x, int y, char *text, int r, int g, int b, int a){}

/**************/
/* rectangles */
/**************/
void           __evas_render_rectangle_draw(Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x, int y, int w, int h, int r, int g, int b, int a){}

/*********/
/* lines */
/*********/
void           __evas_render_line_draw(Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x1, int y1, int x2, int y2, int r, int g, int b, int a){}

/*************/
/* gradients */
/*************/
Evas_Render_Graident *__evas_render_gradient_new(Display *disp){return NULL;}
void              __evas_render_gradient_free(Evas_Render_Graident *gr){}
void              __evas_render_gradient_color_add(Evas_Render_Graident *gr, int r, int g, int b, int a, int dist){}
void              __evas_render_gradient_draw(Evas_Render_Graident *gr, Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x, int y, int w, int h, double angle){}

/************/
/* polygons */
/************/
void              __evas_render_poly_draw (Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, Evas_List points, int r, int g, int b, int a) {}

/***********/
/* drawing */
/***********/
void         __evas_render_set_clip_rect(int on, int x, int y, int w, int h, int r, int g, int b, int a) {}
void         __evas_render_init(Display *disp, int screen, int colors){}
int          __evas_render_capable(Display *disp){return 0;}
void         __evas_render_flush_draw(Display *disp, Imlib_Image dstim, Window win){}
void         __evas_render_sync(Display *disp){}
Visual      *__evas_render_get_visual(Display *disp, int screen){return NULL;}
XVisualInfo *__evas_render_get_visual_info(Display *disp, int screen){return NULL;}
Colormap     __evas_render_get_colormap(Display *disp, int screen){return 0;}
void         __evas_render_draw_add_rect(Display *disp, Imlib_Image dstim, Window win, int x, int y, int w, int h){}

#endif

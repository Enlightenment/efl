#include "evas_imlib_routines.h"

#define SPANS_COMMON(x1, w1, x2, w2) \
(!((((x2) + (w2)) <= (x1)) || ((x2) >= ((x1) + (w1)))))
#define RECTS_INTERSECT(x, y, w, h, xx, yy, ww, hh) \
((SPANS_COMMON((x), (w), (xx), (ww))) && (SPANS_COMMON((y), (h), (yy), (hh))))

static Evas_List drawable_list = NULL;

/*****************************************************************************/
/* image internals ***********************************************************/
/*****************************************************************************/

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
__evas_imlib_image_cache_flush(Display *disp)
{
   int size;
   
   size = imlib_get_cache_size();
   imlib_set_cache_size(0);
   imlib_set_cache_size(size);
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
						    dst_x, dst_y, dst_w, dst_h);
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
__evas_imlib_text_draw(Evas_Imlib_Font *fn, Display *disp, Window win, int x, int y,
		    char *text, int r, int g, int b, int a)
{
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

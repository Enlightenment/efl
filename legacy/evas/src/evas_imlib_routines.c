#include "evas_imlib_routines.h"

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

void
__evas_imlib_sync(Display *disp)
{
   XSync(disp, False);
}

void
__evas_imlib_flush_draw(Display *disp, Window win)
{
   imlib_context_set_display(disp);
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
   return imlib_get_best_visual(disp, screen, &depth);
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
   return DefaultColormap(disp, screen);
}

void
__evas_imlib_init(Display *disp)
{
   imlib_context_set_display(disp);
}


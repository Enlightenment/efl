#include "evas_gl_routines.h"

static int __evas_gl_configuration[] = 
{
   GLX_DOUBLEBUFFER, 
   GLX_RGBA, 
   GLX_RED_SIZE,   1,
   GLX_GREEN_SIZE, 1,
   GLX_BLUE_SIZE,  1, 
   None
};

static void           __evas_gl_image_copy_image_rect_to_texture(Evas_GL_Image *im, int x, int y, int w, int h, int tw, int th, GLuint texture);
static void           __evas_gl_image_move_state_data_to_texture(Evas_GL_Image *im);
static void           __evas_gl_image_calc_tex_and_poly(Evas_GL_Image *im, int x, double *x1, double *x2, int *tx, int *txx, double *dtx, double *dtxx, int tw, int w, int edge);
static void           __evas_gl_image_set_context_for_dest(Evas_GL_Image *im, Display *disp, Window w, int win_w, int win_h);
static Evas_GL_Image *__evas_gl_create_image(void);
static Evas_GL_Image *__evas_gl_image_create_from_file(Display *disp, char *file);
static void           __evas_gl_image_destroy(Evas_GL_Image *im);
static void           __evas_gl_image_cache_flush(Display *disp);

static XVisualInfo *__evas_vi               = NULL;
static GLXContext   __evas_gl_cx            = 0;
static Window       __evas_context_window   = 0;
static Evas_List    __evas_images           = NULL;
static int          __evas_image_cache_max  = 16 *1024 * 1024;
static int          __evas_image_cache_used = 0;


























/*****************************************************************************/
/* image internals ***********************************************************/
/*****************************************************************************/

static void
__evas_gl_image_copy_image_rect_to_texture(Evas_GL_Image *im, int x, int y, 
				      int w, int h, int tw, int th, 
				      GLuint texture)
{
   int tx, ty;
   DATA32 *data, *p1, *p2;
   
   glBindTexture(GL_TEXTURE_2D, texture);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   
   data = malloc(tw * th * 4);
   for (ty = 0; ty < h; ty++)
     {
	p1 = im->data + ((y + ty) * im->w) + x;
	p2 = data + (ty * tw);
	for (tx = 0; tx < w; tx++)
	  {
	     *p2 =
		((*p1 & 0xff000000)) |
		((*p1 & 0x00ff0000) >> 16) |
		((*p1 & 0x0000ff00)) |
		((*p1 & 0x000000ff) << 16);
	     p2++; p1++;
	  }
	
     }
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, data);
   free(data);
}

static void
__evas_gl_image_move_state_data_to_texture(Evas_GL_Image *im)
{
   int i, x, y;
   XSetWindowAttributes att;
   int image_data = 0;
   Imlib_Image image = NULL;
   
   if ((!im->data) && (im->file))
     {
	image = imlib_load_image(im->file);
	if (image)
	  {
	     imlib_context_set_image(image);
	     im->data = imlib_image_get_data_for_reading_only();
	     imlib_free_image();
	     image_data = 1;
	  }
     }
   if (!im->data) return;
	
   im->texture.w = im->w / (im->texture.max_size - 2);
   if (im->w > ((im->texture.max_size - 2) * im->texture.w))
     {
	int shift = 0;
	
	im->texture.edge_w = im->w - (im->texture.w * (im->texture.max_size - 2));
	while (im->texture.edge_w < (im->texture.max_size >> shift)) shift++;
	im->texture.edge_w = im->texture.max_size >> (shift - 1);
	im->texture.w++;
     }
   else
      im->texture.edge_w = im->texture.max_size;
   im->texture.h = im->h / (im->texture.max_size - 2);
   if (im->h > ((im->texture.max_size - 2) * im->texture.h))
     {
	int shift = 0;
	
	im->texture.edge_h = im->h - (im->texture.h * (im->texture.max_size - 2));
	while (im->texture.edge_h < (im->texture.max_size >> shift)) shift++;
	im->texture.edge_h = im->texture.max_size >> (shift - 1);
	im->texture.h++;
     }
   else
      im->texture.edge_h = im->texture.max_size;

   if (!__evas_context_window)
     {
	att.colormap = im->buffer.colormap;
	att.border_pixel = 0;
	att.event_mask = 0;
	__evas_context_window = XCreateWindow(im->buffer.display,
					      RootWindow(im->buffer.display, DefaultScreen(im->buffer.display)),
					      0, 0, 32, 32, 0, 
					      im->buffer.visual_info->depth,
					      InputOutput, 
					      im->buffer.visual_info->visual,
					      CWColormap | CWBorderPixel | CWEventMask,
					      &att);
	im->buffer.window = __evas_context_window;
	im->texture.textures = malloc(sizeof(GLuint) * im->texture.w * im->texture.h);
	glXMakeCurrent(im->buffer.display, im->buffer.window, im->context);
     }
   else
     {
	im->buffer.window = __evas_context_window;
	im->texture.textures = malloc(sizeof(GLuint) * im->texture.w * im->texture.h);
	glXMakeCurrent(im->buffer.display, im->buffer.window, im->context);
     }
   glGenTextures(im->texture.w * im->texture.h, im->texture.textures);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   glEnable(GL_TEXTURE_2D);

   for (i = 0, y = 0; y < im->texture.h; y++)
     {
	for (x = 0; x < im->texture.w; x++, i++)
	  {
	     int xx, yy, ww, hh, tw, th;
	     
	     if (x == (im->texture.w - 1)) tw = im->texture.edge_w;
	     else tw = im->texture.max_size;
	     if (y == (im->texture.h - 1)) th = im->texture.edge_h;
	     else th = im->texture.max_size;

	     if (x == 0) xx = 0;
	     else xx = x * (im->texture.max_size - 2) - 1;
	     if (x == (im->texture.w - 1)) ww = im->w - (x * (im->texture.max_size- 2));
	     else ww = im->texture.max_size - 2;
	     ww += 2;
	     if (x == 0) ww--;
	     if (x == (im->texture.w - 1)) ww--;

	     if (y == 0) yy = 0;
	     else yy = y * (im->texture.max_size - 2) - 1;
	     if (y == (im->texture.h - 1)) hh = im->h - (y * (im->texture.max_size- 2));
	     else hh = im->texture.max_size - 2;
	     hh += 2;
	     if (y == 0) hh--;
	     if (y == (im->texture.h - 1)) hh--;
	     
	     __evas_gl_image_copy_image_rect_to_texture(im, xx, yy, ww, hh, tw, th, 
						  im->texture.textures[i]);
	     __evas_image_cache_used += (tw * th * 4);
	  }
     }
   /* done - set the actual image state to textured */
   im->state = EVAS_STATE_TEXTURE;
   if (image_data)
     {
	imlib_context_set_image(image);
	imlib_image_put_back_data(im->data);
	im->data = NULL;
	imlib_free_image();
     }
}

static void 
__evas_gl_image_calc_tex_and_poly(Evas_GL_Image *im, int x, double *x1, double *x2,
				  int *tx, int *txx, double *dtx, double *dtxx, int tw, int w, int edge)
{
   if ((x == 0) && (tw > 1))
     {
	*tx  = 0;
	*txx = im->texture.max_size - 2;
	*dtx  = (double)*tx  / (im->texture.max_size);
	*dtxx = (double)*txx / (im->texture.max_size);
     }
   else if (x < (tw - 1))
     {
	*tx  = 1;
	*txx = im->texture.max_size - 1;
	*dtx  = (double)*tx  / (im->texture.max_size);
	*dtxx = (double)*txx / (im->texture.max_size);
     }
   else if ((x == 0) && (tw == 1))
     {		
	*tx  = 0;
	*txx = w - (x * (im->texture.max_size - 2));
	*dtx  = (double)*tx  / (double)edge;
	*dtxx = (double)*txx / (double)edge;
     }
   else 
     {
	*tx = 1;
	*txx = w - (x * (im->texture.max_size - 2));
	*dtx  = (double)*tx  / (double)edge;
	*dtxx = (double)*txx / (double)edge;
     }
   
   if (x == 0)
      *x1 = 0.0;
   else
      *x1 = (double)(x * (im->texture.max_size - 2));
   
   if (x < (tw - 1))
      *x2 = (double)((x + 1) * (im->texture.max_size - 2));
   else
      *x2 = (double)w;
}

static void
__evas_gl_image_set_context_for_dest(Evas_GL_Image *im, Display *disp, Window w,
				     int win_w, int win_h)
{
   if (im->buffer.dest != w)
     {
	glXMakeCurrent(disp, w, im->context);
	im->buffer.dest = w;
     }
   if (im->alpha)
     {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
     }
   else
     {
	glDisable(GL_BLEND);
     }
   glEnable(GL_DITHER);
   glShadeModel(GL_FLAT);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
   glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
   /* why doesnt scissor work ? */
   /*   
    glEnable(GL_SCISSOR_TEST);
    glScissor(dst_x, win_h - dst_y - 1, dst_w, dst_h); 
    */
   if ((win_w != im->buffer.dest_w) || (win_h != im->buffer.dest_h))
     {
	glViewport(0, 0, win_w, win_h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, win_w, 0, win_h, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glScalef(1, -1, 1);
	glTranslatef(0, -win_h, 0);   
	im->buffer.dest_w = win_w;
	im->buffer.dest_h = win_h;
     }
}

static Evas_GL_Image *
__evas_gl_create_image(void)
{
   Evas_GL_Image *im;
   
   im = malloc(sizeof(Evas_GL_Image));
   memset(im, 0, sizeof(Evas_GL_Image));
   return im;
}

static Evas_GL_Image *
__evas_gl_image_create_from_file(Display *disp, char *file)
{
   Evas_GL_Image *im;
   Imlib_Image i;
   
   if (!file) return NULL;
   im = __evas_gl_create_image();
   im->file = strdup(file);
   i = imlib_load_image(file);
   if (i)
     {
	imlib_context_set_image(i);
	im->w = imlib_image_get_width();
	im->h = imlib_image_get_height();
	im->alpha = imlib_image_has_alpha();
	imlib_free_image();
     }
   else
     {
	im->w = 0;
	im->h = 0;
     }
   im->data = NULL;
   im->texture.max_size = 256;
   im->texture.w = 0;
   im->texture.h = 0;
   im->texture.edge_w = 0;
   im->texture.edge_h = 0;
   im->texture.textures = NULL;

   __evas_gl_init(disp);   
   im->context = __evas_gl_cx;
   im->buffer.display = disp;
   im->buffer.colormap = __evas_gl_get_colormap(disp, 0);
   im->buffer.visual_info = __evas_vi;
   im->buffer.window = 0;
   im->buffer.dest = 0;
   im->buffer.dest_w = 0;
   im->buffer.dest_h = 0;

   im->references = 1;

   return im;
}

static void
__evas_gl_image_destroy(Evas_GL_Image *im)
{
   if (im->file) free(im->file);
   if (im->data) free(im->data);
   if (im->texture.textures)
     {
	glDeleteTextures(im->texture.w * im->texture.h, im->texture.textures);
	free(im->texture.textures);
     }
   free(im);
}

static void
__evas_gl_image_cache_flush(Display *disp)
{
   while (__evas_image_cache_used > __evas_image_cache_max)
     {
	Evas_GL_Image *im = NULL, *im_last;
        Evas_List l;
	
	im_last = NULL;
        for (l = __evas_images; l; l = l->next)
          {
	     im = l->data;
	     
	     if (im->references <= 0)
		im_last = im;
	  }
	if (im_last)
	  {
	     __evas_images = evas_list_remove(__evas_images, im_last);
	     __evas_gl_image_destroy(im_last);
	  }
     }
   disp = NULL;
}

/*****************************************************************************/
/* image externals ***********************************************************/
/*****************************************************************************/

void
__evas_gl_image_draw(Evas_GL_Image *im, 
		     Display *disp, Window w, int win_w, int win_h,
		     int src_x, int src_y, int src_w, int src_h,
		     int dst_x, int dst_y, int dst_w, int dst_h)
{
   int x, y, i;   
   double dx, dy, dw, dh;
   
   if (im->state != EVAS_STATE_TEXTURE)
      __evas_gl_image_move_state_data_to_texture(im);
   __evas_gl_image_set_context_for_dest(im, disp, w, win_w, win_h);
   /* project src and dst rects to overall dest rect */
   dw = (((double)dst_w * (double)im->w)/ (double)src_w);
   dx = (double)dst_x - (((double)dst_w * (double)src_x)/ (double)src_w);
   
   dh = (((double)dst_h * (double)im->h)/ (double)src_h);
   dy = (double)dst_y - (((double)dst_h * (double)src_y)/ (double)src_h);
   for (y = 0, i = 0; y < im->texture.h; y++)
     {
	for (x = 0; x < im->texture.w; x++, i++)
	  {
	     int tx, ty, txx, tyy;
	     double x1, y1, x2, y2;
	     double dtx, dtxx, dty, dtyy;
	     
	     
	     __evas_gl_image_calc_tex_and_poly(im, x, &x1, &x2, &tx, &txx, 
					       &dtx, &dtxx, 
					       im->texture.w, im->w, 
					       im->texture.edge_w);
	     __evas_gl_image_calc_tex_and_poly(im, y, &y1, &y2, &ty, &tyy, 
					       &dty, &dtyy, 
					       im->texture.h, im->h, 
					       im->texture.edge_h);
             x1 = dx + ((x1 * dw) / (double)im->w);
             y1 = dy + ((y1 * dh) / (double)im->h);
             x2 = dx + ((x2 * dw) / (double)im->w);
             y2 = dy + ((y2 * dh) / (double)im->h);
	     glBindTexture(GL_TEXTURE_2D, im->texture.textures[i]);
	     glBegin(GL_QUADS);
	     glTexCoord2d(dtx,  dty);  glVertex2d(x1, y1);
	     glTexCoord2d(dtxx, dty);  glVertex2d(x2, y1);
	     glTexCoord2d(dtxx, dtyy); glVertex2d(x2, y2);
	     glTexCoord2d(dtx,  dtyy); glVertex2d(x1, y2);
	     glEnd();
	  }
     }
/*   glDisable(GL_SCISSOR_TEST);*/
}

Evas_GL_Image *
__evas_gl_image_new_from_file(Display *disp, char *file)
{	
   Evas_GL_Image *im;
   Evas_List l;
   
   for (l = __evas_images; l; l = l->next)
     {
	im = l->data;
	
	if (((im->file) && (im->buffer.display == disp)) &&
	    (!strcmp(im->file, file)))
	  {
	     if (l != __evas_images)
	       {
		  __evas_images = evas_list_remove(__evas_images, im);
		  __evas_images = evas_list_prepend(__evas_images, im);
	       }
	     im->references++;
	     return im;
	  }
     }
   
   im = __evas_gl_image_create_from_file(disp, file);
   __evas_images = evas_list_prepend(__evas_images, im);
   
   return im;
}

void
__evas_gl_image_free(Evas_GL_Image *im)
{
   im->references--;
   if (im->references <= 0)
      __evas_gl_image_cache_flush(im->buffer.display);
}

void
__evas_gl_image_cache_empty(Display *disp)
{
   Evas_GL_Image *im = NULL, *im_last;
   Evas_List l;
   im_last = (Evas_GL_Image *)1;

   while (im_last)
     {
	im_last = NULL;
	for (l = __evas_images; l; l = l->next)
	  {
	     im = l->data;
	     
	     if (im->references <= 0)
		im_last = im;
	  }
	if (im_last)
	  {
	     __evas_images = evas_list_remove(__evas_images, im_last);
	     __evas_gl_image_destroy(im_last);
	  }
     }
   disp = NULL;
}

void
__evas_gl_image_cache_set_size(Display *disp, int size)
{
   __evas_image_cache_max = size;
   __evas_gl_image_cache_flush(disp);
}

int
__evas_gl_image_cache_get_size(Display *disp)
{
   return __evas_image_cache_max;
   disp = NULL;
}

int
__evas_gl_image_get_width(Evas_GL_Image *im)
{
   return im->w;
}

int
__evas_gl_image_get_height(Evas_GL_Image *im)
{
   return im->h;
}































/*****************************************************************************/
/* font internals ************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* font externals ************************************************************/
/*****************************************************************************/

Evas_GL_Font *
__evas_gl_text_font_new(Display *disp, char *font, int size)
{
}

void
__evas_gl_text_font_free(Evas_GL_Font *fn)
{
}

void
__evas_gl_text_font_add_path(char *path)
{
}

void
__evas_gl_text_font_del_path(char *path)
{
}

char **
__evas_gl_text_font_list_paths(int *count)
{
}

void
__evas_gl_text_cache_empty(Display *disp)
{
}

void
__evas_gl_text_cache_set_size(Display *disp, int size)
{
}

int
__evas_gl_text_cache_get_size(Display *disp)
{
}

void
__evas_gl_text_draw(Evas_GL_Font *fn, Display *disp, Window win, int x, int y,
		    char *text, int r, int g, int b, int a)
{
}




































/*****************************************************************************/
/* general externals *********************************************************/
/*****************************************************************************/

void
__evas_gl_sync(Display *disp)
{
   glXWaitGL();
   XSync(disp, False);
}

void
__evas_gl_flush_draw(Display *disp, Window win)
{
   glXSwapBuffers(disp, win);
}

int
__evas_gl_capable(Display *disp)
{
   int eb, evb;
   return glXQueryExtension(disp, &eb, &evb);
}

Visual *
__evas_gl_get_visual(Display *disp, int screen)
{
   static Display *d = NULL;

   if (d != disp)
     {
	d = disp;
	__evas_vi = glXChooseVisual(disp, screen, __evas_gl_configuration);
     }
   return __evas_vi->visual;
}

XVisualInfo *
__evas_gl_get_visual_info(Display *disp, int screen)
{
   __evas_gl_get_visual(disp, screen);
   return __evas_vi;
}

Colormap
__evas_gl_get_colormap(Display *disp, int screen)
{
   static Display *d = NULL;
   static Colormap cmap = 0;

   if (!__evas_vi) __evas_gl_get_visual(disp, screen);
   if (d != disp)
     {
	d = disp;
	cmap = XCreateColormap(disp, RootWindow(disp, screen), __evas_vi->visual, 0);
     }
   return cmap;
}

void
__evas_gl_init(Display *disp)
{
   if (__evas_gl_cx) return;
   
   if (!__evas_gl_capable(disp)) return;
   __evas_gl_get_visual(disp, 0);
   /* direct rendering client */
   __evas_gl_cx = glXCreateContext(disp, __evas_vi, NULL, GL_TRUE);
   /* GLX indirect */
/*   __evas_gl_cx = glXCreateContext(disp, __evas_vi, NULL, GL_FALSE);*/
}

void
__evas_gl_draw_add_rect(Display *disp, Window win,
			int x, int y, int w, int h)
{
}


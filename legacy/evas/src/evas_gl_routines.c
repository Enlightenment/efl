#include "evas_gl_routines.h"
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef HAVE_GL

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
static Evas_GL_Image *__evas_gl_create_image(void);
static Evas_GL_Image *__evas_gl_image_create_from_file(Display *disp, char *file);
static void           __evas_gl_image_free_textures(Evas_GL_Image *im);
static void           __evas_gl_image_destroy(Evas_GL_Image *im);
static void           __evas_gl_image_cache_flush(Display *disp);

static void           __evas_gl_text_font_render_textures(Evas_GL_Font *f);
static TT_Raster_Map *__evas_gl_text_font_raster_new(int width, int height);
static void           __evas_gl_text_font_raster_free(TT_Raster_Map * rmap);
static void           __evas_gl_text_font_path_add(const char *path);
static void           __evas_gl_text_font_path_del(const char *path);
static char         **__evas_gl_text_font_path_list(int *num_ret);
static int            __evas_gl_is_file(char *file);
static Evas_GL_Font  *__evas_gl_text_font_load(char *font, int size);
static void           __evas_gl_text_calc_size(Evas_GL_Font *f, int *width, int *height, char *text);
static void           __evas_gl_text_font_destroy(Evas_GL_Font *font);
static void           __evas_gl_text_paste(Evas_GL_Font *f, char *text, Display *disp, Window w, int win_w, int win_h, int x, int y, int r, int g, int b, int a);
static void           __evas_gl_text_cache_flush(void);


static XVisualInfo *__evas_vi               = NULL;
static GLXContext   __evas_gl_cx            = 0;
static Window       __evas_context_window   = 0;
static Evas_List    __evas_images           = NULL;
static int          __evas_image_cache_max  = 16 *1024 * 1024;
static int          __evas_image_cache_used = 0;

static Display    *__evas_current_disp      = NULL;
static Window      __evas_current_win       = 0;

static Evas_List   __evas_fonts = NULL;
static int         __evas_fpath_num = 0;
static char      **__evas_fpath = NULL;
static TT_Engine   __evas_engine;
static char        __evas_have_engine = 0;
static int         __evas_font_cache_max = 512 * 1024;
static int         __evas_font_cache_used = 0;

static int         __evas_anti_alias = 1;

const int          __evas_rend_lut[9] = { 0, 64, 128, 192, 255, 255, 255, 255, 255};

#define TT_VALID( handle )  ( ( handle ).z != NULL )

/*
#ifdef HAVE_GLU
#undef HAVE_GLU
#endif
*/





















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
#if 0   
#ifdef HAVE_GLU
   if (__evas_anti_alias)
     {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
     }
   else
#else      
   if (__evas_anti_alias)
     {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     }
   else
#endif      
     {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     }
#endif   
   data = malloc(tw * th * 4);
   for (ty = 0; ty < h; ty++)
     {
	p1 = im->data + ((y + ty) * im->w) + x;
	p2 = data + (ty * tw);
	for (tx = 0; tx < w; tx++)
	  {
#ifndef WORDS_BIGENDIAN
	     *p2 =
		((*p1 & 0xff000000)) |
		((*p1 & 0x00ff0000) >> 16) |
		((*p1 & 0x0000ff00)) |
		((*p1 & 0x000000ff) << 16);
#else
	     *p2 =
		((*p1 & 0xff000000) >> 24) |
		((*p1 & 0x00ff0000) <<  8) |
		((*p1 & 0x0000ff00) <<  8) |
		((*p1 & 0x000000ff) <<  8);
#endif	     
	     p2++; p1++;
	  }
	if (tx < tw)
	   *p2 = p2[-1];
     }
   if (ty < th)
     {
	p1 = data + ((ty - 1) * tw);
	p2 = data + (ty * tw);
	for (tx = 0; tx < w; tx++)
	  {
	     *p2 = *p1;
	     p2++; p1++;
	  }
	if (tx < tw)
	   *p2 = p2[-1];
     }
#ifdef HAVE_GLU
   if (__evas_anti_alias)
     {
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8, tw, th, GL_RGBA, 
			  GL_UNSIGNED_BYTE, data);
	if (glGetError() == GL_OUT_OF_MEMORY)
	  {
	     __evas_gl_image_cache_empty(im->buffer.display);
	     __evas_gl_text_cache_empty(im->buffer.display);
	     gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8, tw, th, GL_RGBA, 
			       GL_UNSIGNED_BYTE, data);
	  }
     }
   else
#endif      
     {
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tw, th, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, data);
	if (glGetError() == GL_OUT_OF_MEMORY)
	  {
	     __evas_gl_image_cache_empty(im->buffer.display);
	     __evas_gl_text_cache_empty(im->buffer.display);
	     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tw, th, 0,
			  GL_RGBA, GL_UNSIGNED_BYTE, data);
	  }
     }
   free(data);
}

static void
__evas_gl_image_move_state_data_to_texture(Evas_GL_Image *im)
{
   int i, x, y;
   int image_data = 0;
   Imlib_Image image = NULL;
   
   if ((!im->data) && (im->file))
     {
	image = imlib_load_image(im->file);
	if (image)
	  {
	     imlib_context_set_image(image);
	     im->data = imlib_image_get_data_for_reading_only();
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
	XSetWindowAttributes att;
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
	__evas_current_disp = im->buffer.display;
	__evas_current_win = im->buffer.window;
     }
   else
     {
	im->buffer.window = __evas_context_window;
	im->texture.textures = malloc(sizeof(GLuint) * im->texture.w * im->texture.h);
	glXMakeCurrent(im->buffer.display, im->buffer.window, im->context);
	__evas_current_disp = im->buffer.display;
	__evas_current_win = im->buffer.window;
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
__evas_gl_image_free_textures(Evas_GL_Image *im)
{
   if ((__evas_current_win != im->buffer.dest) || 
       (__evas_current_disp != im->buffer.display))
     {
	glXMakeCurrent(im->buffer.display, im->buffer.dest, im->context);
	__evas_current_disp = im->buffer.display;
	__evas_current_win = im->buffer.dest;
     }
   if (im->texture.textures)
     {
	__evas_image_cache_used -= 
	   ((((im->texture.w - 1) * im->texture.max_size) *
	     ((im->texture.h - 1) * im->texture.max_size)) +
	    ((im->texture.w - 1) * im->texture.edge_h) +
	    ((im->texture.h - 1) * im->texture.edge_w) +
	    (im->texture.edge_w * im->texture.edge_h)) * 4;
	glDeleteTextures(im->texture.w * im->texture.h, im->texture.textures);
	free(im->texture.textures);
	im->texture.textures = NULL;
     }
   im->state = EVAS_STATE_DATA;
}

static void
__evas_gl_image_destroy(Evas_GL_Image *im)
{
   if (im->file) free(im->file);
   if (im->data) free(im->data);
   __evas_gl_image_free_textures(im);
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
		     Display *disp, Imlib_Image dstim, Window w, int win_w, int win_h,
		     int src_x, int src_y, int src_w, int src_h,
		     int dst_x, int dst_y, int dst_w, int dst_h,
		     int cr, int cg, int cb, int ca)
{
   int x, y, i;   
   double dx, dy, dw, dh;

   if (ca == 0) return;
   if ((src_w <= 0) || (src_h <= 0) || (dst_w <= 0) || (dst_h <= 0)) return;
   if (im->state != EVAS_STATE_TEXTURE)
      __evas_gl_image_move_state_data_to_texture(im);
   if ((__evas_current_win != w) || (__evas_current_disp != disp))
     {
	glXMakeCurrent(disp, w, im->context);
	__evas_current_disp = disp;
	__evas_current_win = w;
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
   
   src_y = im->h - src_h - src_y; 
   
   dw = (((double)dst_w * (double)im->w)/ (double)src_w);
   dx = (double)dst_x - (((double)dst_w * (double)src_x)/ (double)src_w);
   dh = (((double)dst_h * (double)im->h)/ (double)src_h);
   dy = (double)dst_y - (((double)dst_h * (double)src_y)/ (double)src_h);
   
   glEnable(GL_DITHER);
   glEnable(GL_TEXTURE_2D);
   glShadeModel(GL_FLAT);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
   glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
   
   glViewport(dst_x, win_h - dst_y - dst_h, dst_w, dst_h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, dst_w, 0, dst_h, -1, 1);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glScalef(1, -1, 1);
   glTranslatef((double)(-dst_x), (dst_y - dy) -dy - dh 
		, 0);   
   im->buffer.dest_w = win_w;
   im->buffer.dest_h = win_h;

   glColor4d(((double)cr) / 255.0,
	     ((double)cg) / 255.0,
	     ((double)cb) / 255.0,
	     ((double)ca) / 255.0);
   /* project src and dst rects to overall dest rect */
   if ((im->direct) || 
       ((im->bl == 0) && (im->br == 0) && (im->bt == 0) && (im->bb == 0)))
     {
	if ((im->bl == 0) && (im->br == 0) && (im->bt == 0) && (im->bb == 0))
	  {
#ifdef HAVE_GLU
	     if (__evas_anti_alias)
	       {
		  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	       }
	     else
#else
		if (__evas_anti_alias)
	       {
		  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	       }
	     else
#endif
	       {
		  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	       }
	  }
	else
	  {
	     if (__evas_anti_alias)
	       {
		  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	       }
	     else
	       {
		  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	       }
	  }
	for (y = 0, i = 0; y < im->texture.h; y++)
	  {
	     for (x = 0; x < im->texture.w; x++, i++)
	       {
		  int tx, ty, txx, tyy;
		  double x1, y1, x2, y2;
		  double dtx, dtxx, dty, dtyy;
		  
		  
		  __evas_gl_image_calc_tex_and_poly(im, x, &x1, &x2, 
						    &tx, &txx, 
						    &dtx, &dtxx, 
						    im->texture.w, im->w, 
						    im->texture.edge_w);
		  __evas_gl_image_calc_tex_and_poly(im, y, &y1, &y2, 
						    &ty, &tyy, 
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
     }
   else
     {
	int bl, br, bt, bb;
	
	im->direct = 1;
	bl = im->bl;
	br = im->br;
	bt = im->bt;
	bb = im->bb;
	if ((bl + br) > (int)dw)
	  {
	     bl = (int)dw / 2;
	     br = (int)dw - bl;
	  }
	if ((bt + bb) > (int)dh)
	  {
	     bt = (int)dh / 2;
	     bb = (int)dh - bt;
	  }
	__evas_gl_image_draw(im, disp, dstim, w, win_w, win_h,
			     0, 0, bl, bt,
			     dx, dy, bl, bt, cr, cg, cb, ca);
	__evas_gl_image_draw(im, disp, dstim, w, win_w, win_h,
			     bl, 0, im->w - bl - br, bt,
			     dx + bl, dy, dw - bl - br, bt, cr, cg, cb, ca);
	__evas_gl_image_draw(im, disp, dstim, w, win_w, win_h,
			     im->w - br, 0, br, bt,
			     dx + dw - br, dy, br, bt, cr, cg, cb, ca);

	__evas_gl_image_draw(im, disp, dstim, w, win_w, win_h,
			     0, bt, bl, im->h - bt - bb,
			     dx, dy + bt, bl, dh - bt - bb, cr, cg, cb, ca);
	__evas_gl_image_draw(im, disp, dstim, w, win_w, win_h,
			     bl, bt, im->w - bl - br, im->h - bt - bb,
			     dx + bl, dy + bt, dw - bl - br, dh - bt - bb, cr, cg, cb, ca);
	__evas_gl_image_draw(im, disp, dstim, w, win_w, win_h,
			     im->w - br, bt, br, im->h - bt - bb,
			     dx + dw - br, dy + bt, br, dh - bt - bb, cr, cg, cb, ca);
	
	__evas_gl_image_draw(im, disp, dstim, w, win_w, win_h,
			     0, im->h - bb, bl, bb,
			     dx, dy + dh - bb, bl, bb, cr, cg, cb, ca);
	__evas_gl_image_draw(im, disp, dstim, w, win_w, win_h,
			     bl, im->h - bb, im->w - bl - br, bb,
			     dx + bl, dy + dh - bb, dw - bl - br, bb, cr, cg, cb, ca);
	__evas_gl_image_draw(im, disp, dstim, w, win_w, win_h,
			     im->w - br, im->h - bb, br, bb,
			     dx + dw - br, dy + dh - bb, br, bb, cr, cg, cb, ca);
	im->direct = 0;
     }
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
	     if (im->references == 0)
		__evas_image_cache_used -= 
		((((im->texture.w - 1) * im->texture.max_size) *
		  ((im->texture.h - 1) * im->texture.max_size)) +
		 ((im->texture.w - 1) * im->texture.edge_h) +
		 ((im->texture.h - 1) * im->texture.edge_w) +
		 (im->texture.edge_w * im->texture.edge_h)) * 4;
	     im->references++;
	     return im;
	  }
     }
   
   im = __evas_gl_image_create_from_file(disp, file);
   if (im)
      __evas_images = evas_list_prepend(__evas_images, im);
   
   return im;
}

void
__evas_gl_image_free(Evas_GL_Image *im)
{
   im->references--;
   if (im->references == 0)
     {
	__evas_image_cache_used += 
	   ((((im->texture.w - 1) * im->texture.max_size) *
	     ((im->texture.h - 1) * im->texture.max_size)) +
	    ((im->texture.w - 1) * im->texture.edge_h) +
	    ((im->texture.h - 1) * im->texture.edge_w) +
	    (im->texture.edge_w * im->texture.edge_h)) * 4;
     }
   if (im->references <= 0)
      __evas_gl_image_cache_flush(im->buffer.display);
}

void
__evas_gl_image_cache_empty(Display *disp)
{
   Evas_GL_Image *im = NULL, *im_last;
   Evas_List l;
   int size;

   im_last = (Evas_GL_Image *)1;
   while (im_last)
     {
	im_last = NULL;
	for (l = __evas_images; l; l = l->next)
	  {
	     im = l->data;

	     __evas_gl_image_free_textures(im);
	     if (im->references <= 0)
		im_last = im;
	  }
	if (im_last)
	  {
	     __evas_images = evas_list_remove(__evas_images, im_last);
	     __evas_gl_image_destroy(im_last);
	  }
     }
   size = imlib_get_cache_size();
   imlib_set_cache_size(0);
   imlib_set_cache_size(size);
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

void
__evas_gl_image_set_borders(Evas_GL_Image *im, int left, int right,
			    int top, int bottom)
{
   im->bl = left;
   im->br = right;
   im->bt = top;
   im->bb = bottom;
   if ((im->bl + im->br) > im->w)
     {
	im->bl = im->w / 2;
	im->br = im->w - im->bl;
     }
   if ((im->bt + im->bb) > im->h)
     {
	im->bt = im->h / 2;
	im->bb = im->h - im->bt;
     }
}

void
__evas_gl_image_set_smooth_scaling(int on)
{
   if (on != __evas_anti_alias)
      __evas_gl_image_cache_empty(__evas_current_disp);
   __evas_anti_alias = on;
}




























/*****************************************************************************/
/* font internals ************************************************************/
/*****************************************************************************/



static void
__evas_gl_text_paste(Evas_GL_Font *f, char *text,
		     Display *disp, Window win, int win_w, int win_h,
		     int x, int y, int r, int g, int b, int a)
{
   int                 i, j, off, rows, adj, w, h;
   int                 x_offset, y_offset;
   TT_F26Dot6          xx, yy, xmin, ymin, xmax, ymax;
   TT_Glyph_Metrics    metrics;
   float               rr, gg, bb, aa;
   GLuint              last_tex;

   j = text[0];
   TT_Get_Glyph_Metrics(f->glyphs[j], &metrics);
   x_offset = (-metrics.bearingX) / 64;
   y_offset = -(f->max_descent / 64);

   if ((__evas_current_win != win) || (__evas_current_disp != disp))
     {
	glXMakeCurrent(disp, win, f->context);
	__evas_current_disp = disp;
	__evas_current_win = win;
	f->buffer.dest = win;
     }
   glEnable(GL_BLEND);
   glEnable(GL_TEXTURE_2D);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_DITHER);
   glShadeModel(GL_FLAT);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
   glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
   rr = (float)r / 255;
   gg = (float)g / 255;
   bb = (float)b / 255;
   aa = (float)a / 255;
   glColor4f(rr, gg, bb, aa);
   glViewport(0, 0, win_w, win_h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, win_w, 0, win_h, -1, 1);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glScalef(1, -1, 1);
   glTranslatef(0, -win_h, 0);
   f->buffer.dest_w = win_w;
   f->buffer.dest_h = win_h;

   __evas_gl_text_calc_size(f, &w, &h, text);
   rows = h;
   glBindTexture(GL_TEXTURE_2D, f->glyphinfo[j].texture);
   last_tex = f->glyphinfo[j].texture;

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   for (i = 0; text[i]; i++)
     {
	j = text[i];
	if (!TT_VALID(f->glyphs[j]))
	   continue;
	
	TT_Get_Glyph_Metrics(f->glyphs[j], &metrics);
	
	xmin = metrics.bbox.xMin & -64;
	ymin = metrics.bbox.yMin & -64;
	xmax = (metrics.bbox.xMax + 63) & -64;
	ymax = (metrics.bbox.yMax + 63) & -64;

	xmin = (xmin >> 6) + x_offset;
	ymin = (ymin >> 6) + y_offset;
	xmax = (xmax >> 6) + x_offset;
	ymax = (ymax >> 6) + y_offset;

	if (last_tex != f->glyphinfo[j].texture)
	  {
	     glBindTexture(GL_TEXTURE_2D, f->glyphinfo[j].texture);
/* if i don't have these 2 lines.. it screwes up*/
	     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	     last_tex = f->glyphinfo[j].texture;
	  }
	if (ymin < 0) off = 0;
	else off = rows - ymin - 1;
	adj = (rows - ymax) - ((f->max_ascent - f->max_descent) >> 6);
	
	glBegin(GL_QUADS);
	glTexCoord2d(f->glyphinfo[j].x1, f->glyphinfo[j].y1); 
	glVertex2i(x + xmin,     y + ymin + off + adj);
	glTexCoord2d(f->glyphinfo[j].x2, f->glyphinfo[j].y1); 
	glVertex2i(x + xmax + 1, y + ymin + off + adj);
	glTexCoord2d(f->glyphinfo[j].x2, f->glyphinfo[j].y2); 
	glVertex2i(x + xmax + 1, y + ymax + off + adj + 1);
	glTexCoord2d(f->glyphinfo[j].x1, f->glyphinfo[j].y2); 
	glVertex2i(x + xmin,     y + ymax + off + adj + 1);
	glEnd();
	
	x_offset += metrics.advance / 64;
     }
}

static void
__evas_gl_text_font_render_textures(Evas_GL_Font *f)
{
   int i, maxw, maxh, cols, rows, pop, j, k, c, r, x, y;
   int didlast = 0;
   unsigned char *data = NULL;
   
   maxw = 0; 
   maxh = 0;
   for (i = 0; i < f->num_glyph; i++)
     {
	int w, h;
	char text[2];
	
	text[0] = (char)i;
	text[1] = 0;
	__evas_gl_text_calc_size(f, &w, &h, text);
	if (w > maxw) maxw = w;
	if (h > maxh) maxh = h;
     }
   if ((maxw == 0) || (maxh == 0)) return;
   maxw++;
   maxh++;
   cols = f->max_texture_size / maxw;
   rows = f->max_texture_size / maxh;
   if ((cols < 1) || (rows < 1)) return;
   i = f->num_glyph;
   pop = 0;
   while (i > 0)
     {
	pop++;
	i -= cols * rows;
     }
   f->num_textures = pop;
   f->textures = malloc(f->num_textures * sizeof(GLuint));
   glGenTextures(f->num_textures, f->textures);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glEnable(GL_TEXTURE_2D);
   j = 0;
   c = 0; 
   r = 0;
   
   data = malloc(256 * 256);
   glBindTexture(GL_TEXTURE_2D, f->textures[0]);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   for (j = 0, k = 0, i = 0; i < f->num_glyph; i++)
     {
	TT_F26Dot6          xmin, ymin, xmax, ymax;
	int w, h;
	TT_Raster_Map      *rtmp;
	
	if (TT_VALID(f->glyphs[i]))
	  {
	     TT_Glyph_Metrics    metrics;
	     
	     TT_Get_Glyph_Metrics(f->glyphs[i], &metrics);
	     xmin = metrics.bbox.xMin & -64;
	     ymin = metrics.bbox.yMin & -64;
	     xmax = (metrics.bbox.xMax + 63) & -64;
	     ymax = (metrics.bbox.yMax + 63) & -64;
	     w = ((xmax - xmin) / 64) + 1;
	     h = ((ymax - ymin) / 64) + 1;
	     rtmp = __evas_gl_text_font_raster_new(((xmax - xmin) / 64) + 1,
						   ((ymax - ymin) / 64) + 1);
	     if (rtmp)
	       {
		  TT_Get_Glyph_Pixmap(f->glyphs[i], rtmp, -xmin, -ymin);
		  f->glyphinfo[i].texture = f->textures[j];
		  f->glyphinfo[i].px = c * maxw;
		  f->glyphinfo[i].py = r * maxh;
		  f->glyphinfo[i].pw = w;
		  f->glyphinfo[i].ph = h;
		  f->glyphinfo[i].x1 = ((double)f->glyphinfo[i].px / 
					(double)f->max_texture_size);
		  f->glyphinfo[i].y1 = ((double)f->glyphinfo[i].py / 
					(double)f->max_texture_size);
		  f->glyphinfo[i].x2 = ((double)(f->glyphinfo[i].px + w) / 
					(double)f->max_texture_size);
		  f->glyphinfo[i].y2 = ((double)(f->glyphinfo[i].py + h) / 
					(double)f->max_texture_size);
		  
		  for (y = 0; y < h; y++)
		    {
		       for (x = 0; x < w; x++)
			 {
			    int val, rval;
			    
			    rval = (int)(((unsigned char *)(rtmp->bitmap))[((rtmp->rows - y -1) * rtmp->cols) + x]);
			    val = __evas_rend_lut[rval];
			    data[(((f->glyphinfo[i].py + y) << 8) + f->glyphinfo[i].px + x)] = val;
			 }
		    }
		  __evas_gl_text_font_raster_free(rtmp);
	       }
	  }
		 
	k++;
	if (k == (cols * rows))
	  {
	     glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA4, f->max_texture_size, f->max_texture_size, 0,
			  GL_ALPHA, GL_UNSIGNED_BYTE, data);
	     if (glGetError() == GL_OUT_OF_MEMORY)
	       {
		  __evas_gl_image_cache_empty(f->buffer.display);
		  __evas_gl_text_cache_empty(f->buffer.display);
		  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA4, f->max_texture_size, f->max_texture_size, 0,
			       GL_ALPHA, GL_UNSIGNED_BYTE, data);
	       }
	     j++;
	     if (j >= f->num_textures)
		didlast = 1;
	     else
	       {
		  glBindTexture(GL_TEXTURE_2D, f->textures[j]);
		  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	       }
	     k = 0;
	     r = 0;
	     c = 0;
	  }
	else
	  {
	     c++;
	     if (c == cols)
	       {
		  c = 0;
		  r++;
	       }
	  }
     }
   if (!didlast)
     {
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA4, f->max_texture_size, f->max_texture_size, 0,
		     GL_ALPHA, GL_UNSIGNED_BYTE, data);
	if (glGetError() == GL_OUT_OF_MEMORY)
	  {
	     __evas_gl_image_cache_empty(f->buffer.display);
	     __evas_gl_text_cache_empty(f->buffer.display);
	     glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA4, f->max_texture_size, f->max_texture_size, 0,
			  GL_ALPHA, GL_UNSIGNED_BYTE, data);
	  }
     }
   if (data)
     {
	free(data);
	data = NULL;
     }
}

static TT_Raster_Map *
__evas_gl_text_font_raster_new(int width, int height)
{
   TT_Raster_Map      *rmap;
   
   rmap = malloc(sizeof(TT_Raster_Map));
   if (!rmap) return NULL;
   rmap->width = (width + 3) & -4;
   rmap->rows = height;
   rmap->flow = TT_Flow_Up;
   rmap->cols = rmap->width;
   rmap->size = rmap->rows * rmap->width;
   if (rmap->size <= 0)
     {
	free(rmap);
	return NULL;
     }
   rmap->bitmap = malloc(rmap->size);
   if (!rmap->bitmap)
     {
	free(rmap);
	return NULL;
     }
   memset(rmap->bitmap, 0, rmap->size);
   return rmap;
}

static void
__evas_gl_text_font_raster_free(TT_Raster_Map * rmap)
{
   if (rmap->bitmap) free(rmap->bitmap);
   free(rmap);
}

static void
__evas_gl_text_font_path_add(const char *path)
{
   __evas_fpath_num++;
   if (!__evas_fpath) __evas_fpath = malloc(sizeof(char *));
   else __evas_fpath = realloc(__evas_fpath, (__evas_fpath_num * sizeof(char *)));
   __evas_fpath[__evas_fpath_num - 1] = strdup(path);
}

static void
__evas_gl_text_font_path_del(const char *path)
{
   int i, j;
   
   for (i = 0; i < __evas_fpath_num; i++)
     {
	if (!strcmp(path, __evas_fpath[i]))
	  {
	     __evas_fpath_num--;
	     for (j = i; j < __evas_fpath_num; j++) __evas_fpath[j] = __evas_fpath[j + 1];
	     if (__evas_fpath_num > 0) __evas_fpath = realloc(__evas_fpath, __evas_fpath_num * sizeof(char *));
	     else
	       {
		  free(__evas_fpath);
		  __evas_fpath = NULL;
	       }
	  }
     }
}

static char **
__evas_gl_text_font_path_list(int *num_ret)
{
   *num_ret = __evas_fpath_num;
   return __evas_fpath;
}

static int
__evas_gl_is_file(char *file)
{
   struct stat         st;

   if (stat(file, &st) < 0)
      return 0;
   return 1;
}

static Evas_GL_Font *
__evas_gl_text_font_load(char *font, int size)
{
   Evas_GL_Font *f;
   TT_Error            error;
   TT_CharMap          char_map;
   TT_Glyph_Metrics    metrics;
   TT_Instance_Metrics imetrics;
   int                 dpi = 96;
   unsigned short      i, n, code, load_flags;
   unsigned short      num_glyphs = 0, no_cmap = 0;
   unsigned short      platform, encoding;
   int                 j, upm, ascent, descent;
   char                *name, *file = NULL, *tmp;

   if (!__evas_have_engine)
     {
	error = TT_Init_FreeType(&__evas_engine);
	if (error)
	   return NULL;
	__evas_have_engine = 1;
     }
   tmp = malloc(strlen(font) + 4 + 1);
   if (!tmp)
      return NULL;
   sprintf(tmp, "%s.ttf", font);
   if (__evas_gl_is_file(tmp))
      file = strdup(tmp);
   else
     {
	sprintf(tmp, "%s.TTF", font);
	if (__evas_gl_is_file(tmp))
	   file = strdup(tmp);
	else
	  {
	     sprintf(tmp, "%s", font);
	     if (__evas_gl_is_file(tmp))
		file = strdup(tmp);
	  }
     }
   free(tmp);
   if (!file)
     {
	for (j = 0; (j < __evas_fpath_num) && (!file); j++)
	  {
	     tmp = malloc(strlen(__evas_fpath[j]) + 1 + strlen(font) + 4 + 1);
	     if (!tmp)
		return NULL;
	     else
	       {
		  sprintf(tmp, "%s/%s.ttf", __evas_fpath[j], font);
		  if (__evas_gl_is_file(tmp))
		     file = strdup(tmp);
		  else
		    {
		       sprintf(tmp, "%s/%s.TTF", __evas_fpath[j], font);
		       if (__evas_gl_is_file(tmp))
			  file = strdup(tmp);
		       else
			 {
			    sprintf(tmp, "%s/%s", __evas_fpath[j], font);
			    if (__evas_gl_is_file(tmp))
			       file = strdup(tmp);
			 }
		    }
	       }
	     free(tmp);
	  }
     }
   /* didnt find a file? abort */
   if (!file)
      return NULL;
   /* allocate */
   f = malloc(sizeof(Evas_GL_Font));
   /* put in name and references */
   f->file = strdup(font);
   f->size = size;
   f->references = 1;
   /* remember engine */
   f->engine = __evas_engine;
   f->mem_use = 0;
   error = TT_Open_Face(f->engine, file, &f->face);
   if (error)
     {
	free(f->file);
	free(f);
	/*      fprintf(stderr, "Unable to open font\n"); */
	return NULL;
     }
   free(file);
   error = TT_Get_Face_Properties(f->face, &f->properties);
   if (error)
     {
	TT_Close_Face(f->face);
	free(f->file);
	free(f);
	/*      fprintf(stderr, "Unable to get face properties\n"); */
	return NULL;
     }
   
   error = TT_New_Instance(f->face, &f->instance);
   if (error)
     {
	TT_Close_Face(f->face);
	free(f->file);
	free(f);
	/*      fprintf(stderr, "Unable to create instance\n"); */
	return NULL;
     }
   
   TT_Set_Instance_Resolutions(f->instance, dpi, dpi);
   TT_Set_Instance_CharSize(f->instance, size * 64);
   n = f->properties.num_CharMaps;
   
   /* get ascent & descent */
   TT_Get_Instance_Metrics(f->instance, &imetrics);
   upm = f->properties.header->Units_Per_EM;
   ascent = (f->properties.horizontal->Ascender * imetrics.y_ppem) / upm;
   descent = (f->properties.horizontal->Descender * imetrics.y_ppem) / upm;
   if (descent < 0)
      descent = -descent;
   f->ascent = ascent;
   f->descent = descent;
   
   for (i = 0; i < n; i++)
     {
	TT_Get_CharMap_ID(f->face, i, &platform, &encoding);
	if ((platform == 3 && encoding == 1) ||
	    (platform == 0 && encoding == 0))
	  {
	     TT_Get_CharMap(f->face, i, &char_map);
	     break;
	  }
     }
   if (i == n)
      TT_Get_CharMap(f->face, 0, &char_map);   
   f->num_glyph = 256;
   f->glyphs = (TT_Glyph *)malloc(f->num_glyph * sizeof(TT_Glyph));
   memset(f->glyphs, 0, f->num_glyph * sizeof(TT_Glyph));
   f->glyphinfo = (Evas_GL_Glyph_Info *)malloc(sizeof(Evas_GL_Glyph_Info) * f->num_glyph);
   f->textures = NULL;
   f->num_textures = 0;

   load_flags = TTLOAD_SCALE_GLYPH | TTLOAD_HINT_GLYPH;
   f->max_descent = 0;
   f->max_ascent = 0;
   for (i = 0; i < f->num_glyph; ++i)
     {
	if (TT_VALID(f->glyphs[i]))
	   continue;
	
	if (no_cmap)
	  {
	     code = (i - ' ' + 1) < 0 ? 0 : (i - ' ' + 1);
	     if (code >= num_glyphs)
		code = 0;
	  }
	else
	   code = TT_Char_Index(char_map, i);
	
	TT_New_Glyph(f->face, &f->glyphs[i]);
	TT_Load_Glyph(f->instance, f->glyphs[i], code, load_flags);
	TT_Get_Glyph_Metrics(f->glyphs[i], &metrics);
	if ((metrics.bbox.yMin & -64) < f->max_descent)
	   f->max_descent = (metrics.bbox.yMin & -64);
	if (((metrics.bbox.yMax + 63) & -64) > f->max_ascent)
	   f->max_ascent = ((metrics.bbox.yMax + 63) & -64);
     }
   /* work around broken fonts - some just have wrong ascent and */
   /* descent members */
   if (((f->ascent == 0) && (f->descent == 0)) || (f->ascent == 0))
     {
	f->ascent = f->max_ascent / 64;
	f->descent = -f->max_descent / 64;
     }
   /* all ent well in loading, so add to head of font list and return */
   /* we dont need the file handle hanging around so flush it out */
   TT_Flush_Face(f->face);
   return f;
}

static void
__evas_gl_text_calc_size(Evas_GL_Font *f, int *width, int *height, char *text)
{
   int                 i, ascent, descent, pw, ph;
   TT_Glyph_Metrics    gmetrics;
   
   ascent = f->ascent;
   descent = f->descent;
   pw = 0;
   ph = ((f->max_ascent) - f->max_descent) / 64;
   
   for (i = 0; text[i]; i++)
     {
	unsigned char       j;
	
	j = text[i];
	if (!TT_VALID(f->glyphs[j]))
	   continue;
	TT_Get_Glyph_Metrics(f->glyphs[j], &gmetrics);
	if (i == 0)
	   pw += ((-gmetrics.bearingX) / 64);
	if (text[i + 1] == 0)
	   pw += (gmetrics.bbox.xMax / 64);
	else
	   pw += gmetrics.advance / 64;
     }
   *width = pw;
   *height = ph;
}

static void
__evas_gl_text_font_destroy(Evas_GL_Font *font)
{
   int                 i;

   __evas_font_cache_used -= 256 * 128 * font->num_textures;
   /* free freetype instance stuff */
   TT_Done_Instance(font->instance);
   TT_Close_Face(font->face);
   /* free all cached glyphs */
   for (i = 0; i < font->num_glyph; i++)
     {
	if (!TT_VALID(font->glyphs[i]))
	   TT_Done_Glyph(font->glyphs[i]);
     }
   /* free glyph info */
   free(font->glyphs);
   free(font->glyphinfo);
   if (font->textures) 
     {
	glDeleteTextures(font->num_textures, font->textures);
	free(font->textures);
      }
   /* free font struct & name */
   free(font->file);
   free(font);
}

static void
__evas_gl_text_cache_flush(void)
{
   while (__evas_font_cache_used > __evas_font_cache_max)
     {
	Evas_List l;
	Evas_GL_Font *last_f;
	
	last_f = NULL;
	for (l = __evas_fonts; l; l = l->next)
	  {
	     Evas_GL_Font *f;
	     
	     f = l->data;
	     if (f->references == 0)
		last_f = f;
	  }
	if (last_f)
	  {
	     __evas_fonts = evas_list_remove(__evas_fonts, last_f);
	     __evas_gl_text_font_destroy(last_f);
	  }	   
     }
}


/*****************************************************************************/
/* font externals ************************************************************/
/*****************************************************************************/

Evas_GL_Font *
__evas_gl_text_font_new(Display *disp, char *font, int size)
{
   Evas_GL_Font *f;
   Evas_List l;
   
   for (l = __evas_fonts; l; l = l->next)
     {
	f = l->data;
	if ((f->buffer.display == disp) && (!strcmp(font, f->file)) &&
	    (f->size == size)) 
	  {
	     if (f->references == 0)
		__evas_font_cache_used -= 256 * 128 * f->num_textures;
	     f->references++;
	     if (l != __evas_fonts)
	       {
		  __evas_fonts = evas_list_remove(__evas_fonts, f);
		  __evas_fonts = evas_list_prepend(__evas_fonts, f);	     
	       }
	     return f;
	  }	    
     }
   f = __evas_gl_text_font_load(font, size);
   if (!f) return NULL;
   f->context = __evas_gl_cx;
   f->max_texture_size = 256;
   f->buffer.display = disp;
   f->buffer.colormap = __evas_gl_get_colormap(disp, 0);
   f->buffer.visual_info = __evas_vi;
   f->buffer.window = 0;
   f->buffer.dest = 0;
   f->buffer.dest_w = 0;
   f->buffer.dest_h = 0;
   f->buffer.display = disp;
   if (!__evas_context_window)
     {
	XSetWindowAttributes att;
	
	att.colormap = f->buffer.colormap;
	att.border_pixel = 0;
	att.event_mask = 0;
	__evas_context_window = XCreateWindow(f->buffer.display,
					      RootWindow(f->buffer.display, DefaultScreen(f->buffer.display)),
					      0, 0, 32, 32, 0, 
					      f->buffer.visual_info->depth,
					      InputOutput, 
					      f->buffer.visual_info->visual,
					      CWColormap | CWBorderPixel | CWEventMask,
					      &att);
	f->buffer.window = __evas_context_window;
	glXMakeCurrent(f->buffer.display, f->buffer.window, f->context);
     }
   else
     {
	f->buffer.window = __evas_context_window;
	glXMakeCurrent(f->buffer.display, f->buffer.window, f->context);
     }
   __evas_gl_text_font_render_textures(f);
   __evas_fonts = evas_list_prepend(__evas_fonts, f);
   return f;
}

void
__evas_gl_text_font_free(Evas_GL_Font *fn)
{
   fn->references--;
   if (fn->references == 0)
      __evas_font_cache_used += 256 * 128 * fn->num_textures;
   if (fn->references >= 0)
   __evas_gl_text_cache_flush();
}

int
__evas_gl_text_font_get_ascent(Evas_GL_Font *fn)
{
   return fn->ascent;
}

int
__evas_gl_text_font_get_descent(Evas_GL_Font *fn)
{
   return fn->descent;
}

int
__evas_gl_text_font_get_max_ascent(Evas_GL_Font *fn)
{
   return fn->max_ascent / 64;
}

int
__evas_gl_text_font_get_max_descent(Evas_GL_Font *fn)
{
   return fn->max_descent / 64;
}

void
__evas_gl_text_font_get_advances(Evas_GL_Font *fn, char *text, 
				 int *advance_horiz,
				 int *advance_vert)
{
   int                 i, ascent, descent, pw, ph;
   TT_Glyph_Metrics    gmetrics;
   
   ascent = fn->ascent;
   descent = fn->descent;
   pw = 0;
   ph = ascent + descent;
   
   for (i = 0; text[i]; i++)
     {
	unsigned char       j;
	
	j = text[i];
	if (!TT_VALID(fn->glyphs[j]))
	   continue;
	TT_Get_Glyph_Metrics(fn->glyphs[j], &gmetrics);
	if (i == 0)
	   pw += ((-gmetrics.bearingX) / 64);
	pw += gmetrics.advance / 64;
     }
   *advance_horiz = pw;
   *advance_vert = ph;
}

int
__evas_gl_text_font_get_first_inset(Evas_GL_Font *fn, char *text)
{
   int                 i;
   TT_Glyph_Metrics    gmetrics;

   for (i = 0; text[i]; i++)
     {
	unsigned char       j;
	
	j = text[i];
	if (!TT_VALID(fn->glyphs[j]))
	   continue;
	TT_Get_Glyph_Metrics(fn->glyphs[j], &gmetrics);
	return ((-gmetrics.bearingX) / 64);
     }
   return 0;
}

void
__evas_gl_text_font_add_path(char *path)
{
   __evas_gl_text_font_path_add(path);
}

void
__evas_gl_text_font_del_path(char *path)
{
   __evas_gl_text_font_path_del(path);
}

char **
__evas_gl_text_font_list_paths(int *count)
{
   return __evas_gl_text_font_path_list(count);
}

void
__evas_gl_text_cache_empty(Display *disp)
{
   Evas_List l;
   Evas_GL_Font *last_f;
   
   last_f = (Evas_GL_Font *)1;
   while (last_f)
     {
	last_f = NULL;
	for (l = __evas_fonts; l; l = l->next)
	  {
	     Evas_GL_Font *f;
	     
	     f = l->data;
	     if (f->references == 0)
		last_f = f;
	  }
	if (last_f)
	  {
	     __evas_fonts = evas_list_remove(__evas_fonts, last_f);
	     __evas_gl_text_font_destroy(last_f);
	  }	   
     }
}

void
__evas_gl_text_cache_set_size(Display *disp, int size)
{
   __evas_font_cache_max = size;
   __evas_gl_text_cache_flush();
}

int
__evas_gl_text_cache_get_size(Display *disp)
{
   return __evas_font_cache_max;
}

void
__evas_gl_text_draw(Evas_GL_Font *fn, Display *disp, Imlib_Image dstim, Window win, 
		    int win_w, int win_h, int x, int y, char *text, 
		    int r, int g, int b, int a)
{
   if ((!fn) || (!text)) return;
   __evas_gl_text_paste(fn, text, disp, win, win_w, win_h, x, y, r, g, b, a);
}

void
__evas_gl_text_get_size(Evas_GL_Font *fn, char *text, int *w, int *h)
{
   if ((!fn) || (!text)) 
      {
	 *w = 0; *h = 0;
	 return;
      }
   __evas_gl_text_calc_size(fn, w, h, text);
}

int
__evas_gl_text_get_character_at_pos(Evas_GL_Font *fn, char *text, int x, int y, int *cx, int *cy, int *cw, int *ch)
{
   int                 i, px, ppx;
   TT_Glyph_Metrics    gmetrics;
   
   if ((y < 0) || (y > (fn->ascent + fn->descent)))
      return -1;
   if (cy)
      *cy = 0;
   if (ch)
      *ch = fn->ascent + fn->descent;
   ppx = 0;
   px = 0;
   for (i = 0; text[i]; i++)
     {
	unsigned char       j;
	
	j = text[i];
	if (!TT_VALID(fn->glyphs[j]))
	   continue;
	TT_Get_Glyph_Metrics(fn->glyphs[j], &gmetrics);
	ppx = px;
	if (i == 0)
	   px += ((-gmetrics.bearingX) / 64);
	if (text[i + 1] == 0)
	   px += (gmetrics.bbox.xMax / 64);
	else
	   px += gmetrics.advance / 64;
	if ((x >= ppx) && (x < px))
	  {
	     if (cx)
		*cx = ppx;
	     if (cw)
		*cw = px - ppx;
	     return i;
	  }
     }
   *cw = 0;
   *ch = 0;
   *cx = 0;
   *cy = 0;
   return -1;
}

void
__evas_gl_text_get_character_number(Evas_GL_Font *fn, char *text, int num, int *cx, int *cy, int *cw, int *ch)
{
   int                 i, px, ppx;
   TT_Glyph_Metrics    gmetrics;
   
   if (cy)
      *cy = 0;
   if (ch)
      *ch = fn->ascent + fn->descent;
   ppx = 0;
   px = 0;
   for (i = 0; text[i]; i++)
     {
	unsigned char       j;
	
	j = text[i];
	if (!TT_VALID(fn->glyphs[j]))
	   continue;
	TT_Get_Glyph_Metrics(fn->glyphs[j], &gmetrics);
	ppx = px;
	if (i == 0)
	   px += ((-gmetrics.bearingX) / 64);
	if (text[i + 1] == 0)
	   px += (gmetrics.bbox.xMax / 64);
	else
	   px += gmetrics.advance / 64;
	if (i == num)
	  {
	     if (cx)
		*cx = ppx;
	     if (cw)
		*cw = px - ppx;
	     return;
	  }
     }
   *cw = 0;
   *ch = 0;
   *cx = 0;
   *cy = 0;
}
























/*****************************************************************************/
/* rectangle externals *******************************************************/
/*****************************************************************************/

void           __evas_gl_rectangle_draw(Display *disp, Imlib_Image dstim, Window win,
					int win_w, int win_h,
					int x, int y, int w, int h,
					int r, int g, int b, int a)
{
   float rr, gg, bb, aa;
   static int dest_w = 0, dest_h = 0;
   
   if ((__evas_current_win != win) || (__evas_current_disp != disp))
     {
	glXMakeCurrent(disp, win, __evas_gl_cx);
	__evas_current_disp = disp;
	__evas_current_win = win;
     }
   rr = (float)r / 255;
   gg = (float)g / 255;
   bb = (float)b / 255;
   aa = (float)a / 255;
   glColor4f(rr, gg, bb, aa);
   glViewport(0, 0, win_w, win_h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, win_w, 0, win_h, -1, 1);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glScalef(1, -1, 1);
   glTranslatef(0, -win_h, 0);
   dest_w = win_w;
   dest_h = win_h;
   if (a < 255)
     {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
     }
   else
      glDisable(GL_BLEND);
   glEnable(GL_DITHER);
   glDisable(GL_TEXTURE_2D);
   glShadeModel(GL_FLAT);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
   glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
   
   glBegin(GL_QUADS);
   glVertex2i(x, y);
   glVertex2i(x + w, y);
   glVertex2i(x + w, y + h);
   glVertex2i(x, y + h);
   glEnd();
   
   glEnable(GL_TEXTURE_2D);
}




















/*****************************************************************************/
/* line externals ************************************************************/
/*****************************************************************************/

void              __evas_gl_line_draw(Display *disp, Imlib_Image dstim, Window win,
				      int win_w, int win_h,
				      int x1, int y1, int x2, int y2,
				      int r, int g, int b, int a)
{
   float rr, gg, bb, aa;
   static int dest_w = 0, dest_h = 0;
   
   if ((__evas_current_win != win) || (__evas_current_disp != disp))
     {
	glXMakeCurrent(disp, win, __evas_gl_cx);
	__evas_current_disp = disp;
	__evas_current_win = win;
     }
   rr = (float)r / 255;
   gg = (float)g / 255;
   bb = (float)b / 255;
   aa = (float)a / 255;
   glColor4f(rr, gg, bb, aa);
   glViewport(0, 0, win_w, win_h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, win_w, 0, win_h, -1, 1);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glScalef(1, -1, 1);
   glTranslatef(0, -win_h, 0);
   dest_w = win_w;
   dest_h = win_h;

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_DITHER);
   glDisable(GL_TEXTURE_2D);
   glShadeModel(GL_FLAT);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
   glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
   glEnable(GL_LINE_SMOOTH);
   
   glBegin(GL_LINES);
   glVertex2d((double)x1 + 0.5, (double)y1 + 0.5);
   glVertex2d((double)x2 + 0.5, (double)y2 + 0.5);
   glEnd();
   
   glEnable(GL_TEXTURE_2D);
}

















/*****************************************************************************/
/* gradient internals ********************************************************/
/*****************************************************************************/

static void
__evas_gl_gradient_free_texture(Evas_GL_Graident *gr)
{
   if (gr->texture_w > 0)
      glDeleteTextures(1, &(gr->texture));
   gr->texture_w = 0;
}

static void
__evas_gl_gradient_gen_texture(Evas_GL_Graident *gr)
{
   Evas_List p;
   unsigned char *map;
   unsigned int *pmap;
   int i, ll, v1, v2, j, inc, r, g, b, a, rr, gg, bb, aa, v, vv, l;

   if (!gr->colors) return;
   if (!gr->colors->next) return;
   ll = 1;
   for (p = gr->colors; p; p = p->next)
     {
	Evas_GL_Graident_Color *cl;
	
	cl = p->data;
	ll += cl->dist;
     }
   pmap = malloc(ll * 4);
   map = malloc(gr->max_texture_size * 4);
   i = 0;
   for (p = gr->colors; p; p = p->next)
     {
        Evas_GL_Graident_Color *cl, *cl2;
	
	cl = p->data;
	if (p->next)
	   cl2 = p->next->data;
	if (p->next)
	  {
	     for (j = 0; j < cl2->dist; j++)
	       {
		  v1 = (j << 16) / cl2->dist;
		  v2 = 65536 - v1;
		  r = ((cl->r * v2) + (cl2->r * v1)) >> 16;
		  g = ((cl->g * v2) + (cl2->g * v1)) >> 16;
		  b = ((cl->b * v2) + (cl2->b * v1)) >> 16;
		  a = ((cl->a * v2) + (cl2->a * v1)) >> 16;
		  pmap[i++] = (a << 24) | (r << 16) | (g << 8) | b;
	       }
	  }
	else
	  {
	     r = cl->r;
	     g = cl->g;
	     b = cl->b;
	     a = cl->a;
	     pmap[i++] = (a << 24) | (r << 16) | (g << 8) | b;
	  }
     }
   inc = ((ll - 1) << 16) / (gr->max_texture_size);
   l = 0;
   j = 0;
   if (ll > gr->max_texture_size)
     {
	for (i = 0; i < gr->max_texture_size; i++)
	  {
	     v = pmap[l >> 16];
	     if ((l >> 16) < ll)
		vv = pmap[(l >> 16) + 1];
	     else
		vv = pmap[(l >> 16)];
	     v1 = l - ((l >> 16) << 16);
	     v2 = 65536 - v1;
	     b = ((v)      ) & 0xff;
	     g = ((v) >> 8 ) & 0xff;
	     r = ((v) >> 16) & 0xff;
	     a = ((v) >> 24) & 0xff;
	     bb = ((vv)      ) & 0xff;
	     gg = ((vv) >> 8 ) & 0xff;
	     rr = ((vv) >> 16) & 0xff;
	     aa = ((vv) >> 24) & 0xff;
	     r = ((r * v2) + (rr * v1)) >> 16;
	     g = ((g * v2) + (gg * v1)) >> 16;
	     b = ((b * v2) + (bb * v1)) >> 16;
	     a = ((a * v2) + (aa * v1)) >> 16;
	     map[j++] = r;
	     map[j++] = g;
	     map[j++] = b;
	     map[j++] = a;
	     l += inc;
	  }
     }
   else
     {
	for (i = 0; i < ll; i++)
	  {	
	     v = pmap[i];
             b = ((v)      ) & 0xff;
	     g = ((v) >> 8 ) & 0xff;
	     r = ((v) >> 16) & 0xff;
	     a = ((v) >> 24) & 0xff;
             map[j++] = r;
	     map[j++] = g;
	     map[j++] = b;
	     map[j++] = a;
	  }
	for (i = ll; i < gr->max_texture_size; i++)
	  {
             map[j++] = r;
	     map[j++] = g;
	     map[j++] = b;
	     map[j++] = a;
	  }
     }
   if (!__evas_context_window)
     {
	XSetWindowAttributes att;
	att.colormap = gr->buffer.colormap;
	att.border_pixel = 0;
	att.event_mask = 0;
	__evas_context_window = XCreateWindow(gr->buffer.display,
					      RootWindow(gr->buffer.display, DefaultScreen(gr->buffer.display)),
					      0, 0, 32, 32, 0, 
					      gr->buffer.visual_info->depth,
					      InputOutput, 
					      gr->buffer.visual_info->visual,
					      CWColormap | CWBorderPixel | CWEventMask,
					      &att);
	gr->buffer.window = __evas_context_window;
	glXMakeCurrent(gr->buffer.display, gr->buffer.window, gr->context);
	__evas_current_disp = gr->buffer.display;
	__evas_current_win = gr->buffer.window;
     }
   else
     {
	gr->buffer.window = __evas_context_window;
	glXMakeCurrent(gr->buffer.display, gr->buffer.window, gr->context);
	__evas_current_disp = gr->buffer.display;
	__evas_current_win = gr->buffer.window;
     }
   /* the texture */
   /***************************************/
   /*c1....c2.......c3......c4.....c5...c6*/
   /***************************************/
   glGenTextures(1, &(gr->texture));
   glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, gr->texture);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gr->max_texture_size, 1, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, map);
   if (glGetError() == GL_OUT_OF_MEMORY)
     {
	__evas_gl_image_cache_empty(gr->buffer.display);
	__evas_gl_text_cache_empty(gr->buffer.display);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gr->max_texture_size, 1, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, map);
     }
   gr->texture_w = ll;
   gr->texture_h = 1;
   free(pmap);
   free(map);
}

/*****************************************************************************/
/* gradient externals ********************************************************/
/*****************************************************************************/

Evas_GL_Graident *
__evas_gl_gradient_new(Display *disp)
{
   Evas_GL_Graident *gr;
   
   gr = malloc(sizeof(Evas_GL_Graident));
   gr->colors = NULL;
   gr->context = __evas_gl_cx;
   gr->max_texture_size = 256;
   gr->texture_w = 0;
   gr->texture_h = 0;
   gr->texture = 0;
   gr->buffer.display = disp;
   gr->buffer.colormap = __evas_gl_get_colormap(disp, 0);
   gr->buffer.visual_info = __evas_vi;
   gr->buffer.window = 0;
   gr->buffer.dest = 0;
   gr->buffer.dest_w = 0;
   gr->buffer.dest_h = 0;
   gr->buffer.display = disp;
   return gr;
}

void
__evas_gl_gradient_free(Evas_GL_Graident *gr)
{
   Evas_List l;
   
   __evas_gl_gradient_free_texture(gr);
   for (l = gr->colors; l; l = l->next)
      free(l->data);
   evas_list_free(gr->colors);
}

void
__evas_gl_gradient_color_add(Evas_GL_Graident *gr, int r, int g, int b, 
			     int a, int dist)
{
   Evas_GL_Graident_Color *cl;
   
   cl = malloc(sizeof(Evas_GL_Graident_Color));
   cl->r = r;
   cl->g = g;
   cl->b = b;
   cl->a = a;
   if (gr->colors) cl->dist = dist;
   else cl->dist = 0;
   gr->colors = evas_list_append(gr->colors, cl);
   __evas_gl_gradient_free_texture(gr);
}

void
__evas_gl_gradient_draw(Evas_GL_Graident *gr, 
			Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h,
			int x, int y, int w, int h, double angle)
{
   int i;
   static int dest_w = 0, dest_h = 0;
   double max, t[8];
   

   if (gr->texture_w == 0)
      __evas_gl_gradient_gen_texture(gr);
   if ((__evas_current_win != win) || (__evas_current_disp != disp))
     {
	glXMakeCurrent(disp, win, __evas_gl_cx);
	__evas_current_disp = disp;
	__evas_current_win = win;
     }
   glColor4f(1.0, 1.0, 1.0, 1.0);
   glViewport(0, 0, win_w, win_h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, win_w, 0, win_h, -1, 1);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glScalef(1, -1, 1);
   glTranslatef(0, -win_h, 0);
   dest_w = win_w;
   dest_h = win_h;

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_DITHER);
   glEnable(GL_TEXTURE_2D);
   glShadeModel(GL_FLAT);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
   glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
   glBindTexture(GL_TEXTURE_2D, gr->texture);   
   
   t[0] = cos(((-angle + 45 + 90) * 2 * 3.141592654) / 360);
   t[1] = sin(((-angle + 45 + 90) * 2 * 3.141592654) / 360);
   
   t[2] = cos(((-angle + 45 + 180) * 2 * 3.141592654) / 360);
   t[3] = sin(((-angle + 45 + 180) * 2 * 3.141592654) / 360);
   
   t[4] = cos(((-angle + 45 + 270) * 2 * 3.141592654) / 360);
   t[5] = sin(((-angle + 45 + 270) * 2 * 3.141592654) / 360);
   
   t[6] = cos(((-angle + 45 + 0) * 2 * 3.141592654) / 360);
   t[7] = sin(((-angle + 45 + 0) * 2 * 3.141592654) / 360);
   max = 0;
   for (i = 0; i < 8; i++)
     {
	if ((t[i] < 0) && (-t[i] > max)) max = -t[i];
	else if ((t[i] > max)) max = t[i];
     }
   if (max > 0)
     {
	for (i = 0; i < 8; i++) t[i] *= 1 / max;
     }
   for (i = 0; i < 8; i+=2)
     {
	t[i] = (((0.5) + (t[i] / 2)) * (double)gr->texture_w) / (double)gr->max_texture_size;
	t[i + 1] = ((0.5) - (t[i + 1] / 2));
     }
   
   glBegin(GL_QUADS);
   glTexCoord2d(t[0],  t[1]); glVertex2i(x, y);
   glTexCoord2d(t[2],  t[3]); glVertex2i(x + w, y);
   glTexCoord2d(t[4],  t[5]); glVertex2i(x + w, y + h);
   glTexCoord2d(t[6],  t[7]); glVertex2i(x, y + h);
   glEnd();
}













/* something is wrong here - GL experts? the polys dont get tesselated */
/* correctly */
/*
#ifdef HAVE_GLU
static void
__evas_gl_tess_begin_cb(GLenum which)
{
   glBegin(which);
}

static void
__evas_gl_tess_end_cb(void)
{
   glEnd();
}

static void
__evas_gl_tess_error_cb(GLenum errorcode)
{
}

static void
__evas_gl_tess_vertex_cb(GLvoid *vertex)
{
   glVertex2dv(vertex);
}

static void
__evas_gl_tess_combine_cb(GLdouble coords[3],
			  GLdouble *vertex_data[4],
			  GLfloat weight[4], GLdouble **data_out)
{
   GLdouble *vertex;
   int i;
   
   vertex = (GLdouble *) malloc(6 * sizeof(GLdouble));
   vertex[0] = coords[0];
   vertex[1] = coords[1];
   vertex[2] = coords[2];
   for (i = 3; i < 6; i++)
      vertex[i] = 
      weight[0] * vertex_data[0][i] +
      weight[1] * vertex_data[1][i] +
      weight[2] * vertex_data[2][i] +
      weight[3] * vertex_data[3][i];
   *data_out = vertex;
}
#endif
*/

/************/
/* polygons */
/************/
void
__evas_gl_poly_draw (Display *disp, Imlib_Image dstim, Window win, 
		     int win_w, int win_h, 
		     Evas_List points, 
		     int r, int g, int b, int a)
{
   Evas_List l2;
   float rr, gg, bb, aa;
   static int dest_w = 0, dest_h = 0;
   
   if ((__evas_current_win != win) || (__evas_current_disp != disp))
     {
	glXMakeCurrent(disp, win, __evas_gl_cx);
	__evas_current_disp = disp;
	__evas_current_win = win;
     }
   rr = (float)r / 255;
   gg = (float)g / 255;
   bb = (float)b / 255;
   aa = (float)a / 255;
   glColor4f(rr, gg, bb, aa);
   glViewport(0, 0, win_w, win_h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, win_w, 0, win_h, -1, 1);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glScalef(1, -1, 1);
   glTranslatef(0, -win_h, 0);
   dest_w = win_w;
   dest_h = win_h;
   if (a < 255)
     {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
     }
   else
      glDisable(GL_BLEND);
   glEnable(GL_DITHER);
   glDisable(GL_TEXTURE_2D);
   glShadeModel(GL_FLAT);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
   glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

/*
#ifdef HAVE_GLU
     {
	static void *tess = NULL;
	
	if (!tess)
	  {
	     tess = gluNewTess();
	     
	     gluTessCallback(tess, GLU_TESS_BEGIN, __evas_gl_tess_begin_cb);
	     gluTessCallback(tess, GLU_TESS_END, __evas_gl_tess_end_cb);
	     gluTessCallback(tess, GLU_TESS_ERROR, __evas_gl_tess_error_cb);
	     gluTessCallback(tess, GLU_TESS_VERTEX, __evas_gl_tess_vertex_cb);
	     gluTessCallback(tess, GLU_TESS_COMBINE, __evas_gl_tess_combine_cb);
	     gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
	  }
	gluTessBeginPolygon(tess, NULL);
	gluTessBeginContour(tess);
	for (l2 = points; l2; l2 = l2->next)
	  {
	     Evas_Point p;
	     GLdouble glp[3];
	     
	     p = l2->data;
	     glp[0] = p->x;
	     glp[1] = p->y;
	     glp[2] = 0;
	     gluTessVertex(tess, glp, glp);
	  }
	gluTessEndContour(tess);
	gluTessEndPolygon(tess);
     }
#else
*/
/*
#warning   "You do not have GLU and thus polygons that are not convex will not"
#warning   "render correctly."
*/
   glBegin(GL_POLYGON);
   for (l2 = points; l2; l2 = l2->next)
     {
	Evas_Point p;
	
	p = l2->data;
	glVertex2d(p->x, p->y);
     }
   glEnd();
/*#endif*/
   glEnable(GL_TEXTURE_2D);
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
__evas_gl_flush_draw(Display *disp, Imlib_Image dstim, Window win)
{
   if ((__evas_current_win != win) || (__evas_current_disp != disp))
     {
	glXMakeCurrent(disp, win, __evas_gl_cx);
	__evas_current_disp = disp;
	__evas_current_win = win;
     }
   glXSwapBuffers(disp, win);
}

int
__evas_gl_capable(Display *disp)
{
   int eb, evb;

   if (__evas_gl_cx) return 1;
   if (glXQueryExtension(disp, &eb, &evb))
     {
	__evas_gl_init(disp, 0, 256);
	if (__evas_gl_cx) return 1;
	return 0;
     }
   return 0;
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
__evas_gl_init(Display *disp, int screen, int colors)
{
   if (__evas_gl_cx) return;
   
   __evas_gl_get_visual(disp, screen);
   /* direct rendering client */
   __evas_gl_cx = glXCreateContext(disp, __evas_vi, NULL, GL_TRUE);
   /* GLX indirect */
/*   __evas_gl_cx = glXCreateContext(disp, __evas_vi, NULL, GL_FALSE);*/
}

void
__evas_gl_draw_add_rect(Display *disp, Imlib_Image dstim, Window win,
			int x, int y, int w, int h)
{
   return;
}

#else

/***************/
/* image stuff */
/***************/
Evas_GL_Image *__evas_gl_image_new_from_file(Display *disp, char *file){return NULL;}
void           __evas_gl_image_free(Evas_GL_Image *im){}
void           __evas_gl_image_cache_empty(Display *disp){}
void           __evas_gl_image_cache_set_size(Display *disp, int size){}
int            __evas_gl_image_cache_get_size(Display *disp){return 0;}
int            __evas_gl_image_get_width(Evas_GL_Image *im){return 0;}
int            __evas_gl_image_get_height(Evas_GL_Image *im){return 0;}
void           __evas_gl_image_set_borders(Evas_GL_Image *im, int left, int right, int top, int bottom){}
void           __evas_gl_image_set_smooth_scaling(int on){}
void           __evas_gl_image_draw(Evas_GL_Image *im, Display *disp, Imlib_Image dstim, Window w, int win_w, int win_h, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int cr, int cg, int cb, int ca){}

/********/
/* text */
/********/
Evas_GL_Font  *__evas_gl_text_font_new(Display *disp, char *font, int size){return NULL;}
void           __evas_gl_text_font_free(Evas_GL_Font *fn){}
void           __evas_gl_text_font_add_path(char *path){}
int            __evas_gl_text_font_get_ascent(Evas_GL_Font *fn){return 0;}
int            __evas_gl_text_font_get_descent(Evas_GL_Font *fn){return 0;}
int            __evas_gl_text_font_get_max_ascent(Evas_GL_Font *fn){return 0;}
int            __evas_gl_text_font_get_max_descent(Evas_GL_Font *fn){return 0;}
void           __evas_gl_text_font_get_advances(Evas_GL_Font *fn, char *text, int *advance_horiz, int *advance_vert){}
int            __evas_gl_text_font_get_first_inset(Evas_GL_Font *fn, char *text){return 0;}
void           __evas_gl_text_font_del_path(char *path){}
char         **__evas_gl_text_font_list_paths(int *count){return NULL;}
void           __evas_gl_text_cache_empty(Display *disp){}
void           __evas_gl_text_cache_set_size(Display *disp, int size){}
int            __evas_gl_text_cache_get_size(Display *disp){return 0;}
void           __evas_gl_text_get_size(Evas_GL_Font *fn, char *text, int *w, int *h){}
int            __evas_gl_text_get_character_at_pos(Evas_GL_Font *fn, char *text, int x, int y, int *cx, int *cy, int *cw, int *ch){return 0;}
void           __evas_gl_text_get_character_number(Evas_GL_Font *fn, char *text, int num, int *cx, int *cy, int *cw, int *ch){}
void           __evas_gl_text_draw(Evas_GL_Font *fn, Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x, int y, char *text, int r, int g, int b, int a){}

/**************/
/* rectangles */
/**************/
void           __evas_gl_rectangle_draw(Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x, int y, int w, int h, int r, int g, int b, int a){}

/*********/
/* lines */
/*********/
void           __evas_gl_line_draw(Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x1, int y1, int x2, int y2, int r, int g, int b, int a){}

/*************/
/* gradients */
/*************/
Evas_GL_Graident *__evas_gl_gradient_new(Display *disp){return NULL;}
void              __evas_gl_gradient_free(Evas_GL_Graident *gr){}
void              __evas_gl_gradient_color_add(Evas_GL_Graident *gr, int r, int g, int b, int a, int dist){}
void              __evas_gl_gradient_draw(Evas_GL_Graident *gr, Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x, int y, int w, int h, double angle){}

/************/
/* polygons */
/************/
void              __evas_gl_poly_draw (Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, Evas_List points, int r, int g, int b, int a) {}

/***********/
/* drawing */
/***********/
void         __evas_gl_init(Display *disp, int screen, int colors){}
int          __evas_gl_capable(Display *disp){return 0;}
void         __evas_gl_flush_draw(Display *disp, Imlib_Image dstim, Window win){}
void         __evas_gl_sync(Display *disp){}
Visual      *__evas_gl_get_visual(Display *disp, int screen){return NULL;}
XVisualInfo *__evas_gl_get_visual_info(Display *disp, int screen){return NULL;}
Colormap     __evas_gl_get_colormap(Display *disp, int screen){return 0;}
void         __evas_gl_draw_add_rect(Display *disp, Imlib_Image dstim, Window win, int x, int y, int w, int h){}

#endif

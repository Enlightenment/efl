#include "evas_gl_routines.h"
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>

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

static void __evas_gl_text_font_render_textures(Evas_GL_Font *f);
static TT_Raster_Map *__evas_gl_text_font_raster_new(int width, int height);
static void __evas_gl_text_font_raster_free(TT_Raster_Map * rmap);
static void __evas_gl_text_font_path_add(const char *path);
static void __evas_gl_text_font_path_del(const char *path);
static char **__evas_gl_text_font_path_list(int *num_ret);
static int __evas_gl_is_file(char *file);
static Evas_GL_Font *__evas_gl_text_font_load(char *font, int size);
static void __evas_gl_text_calc_size(Evas_GL_Font *f, int *width, int *height, char *text);
static void __evas_gl_text_font_destroy(Evas_GL_Font *font);
static void __evas_gl_text_paste(Evas_GL_Font *f, char *text,
				 Display *disp, Window w, int win_w, int win_h,
				 int x, int y, int r, int g, int b, int a);


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

const int          __evas_rend_lut[9] = { 0, 64, 128, 192, 255, 255, 255, 255, 255};

#define TT_VALID( handle )  ( ( handle ).z != NULL )

























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
   glColor4f(1.0, 1.0, 1.0, 1.0);
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
   if ((win_w != f->buffer.dest_w) || (win_h != f->buffer.dest_h))
     {
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
     }

   __evas_gl_text_calc_size(f, &w, &h, text);
   rows = h;
   glBindTexture(GL_TEXTURE_2D, f->glyphinfo[j].texture);
   last_tex = f->glyphinfo[j].texture;
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
   /* free font struct & name */
   free(font->file);
   free(font);
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
__evas_gl_text_draw(Evas_GL_Font *fn, Display *disp, Window win, 
		    int win_w, int win_h, int x, int y, char *text, 
		    int r, int g, int b, int a)
{
   if ((!fn) || (!text)) return;
   __evas_gl_text_paste(fn, text, disp, win, win_w, win_h, x, y, r, g, b, a);
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
__evas_gl_init(Display *disp, int screen)
{
   if (__evas_gl_cx) return;
   
   if (!__evas_gl_capable(disp)) return;
   __evas_gl_get_visual(disp, screen);
   /* direct rendering client */
   __evas_gl_cx = glXCreateContext(disp, __evas_vi, NULL, GL_TRUE);
   /* GLX indirect */
/*   __evas_gl_cx = glXCreateContext(disp, __evas_vi, NULL, GL_FALSE);*/
}

void
__evas_gl_draw_add_rect(Display *disp, Window win,
			int x, int y, int w, int h)
{
   return;
   disp = NULL;
   win = 0;
   x = 0;
   y = 0;
   w = 0;
   h = 0;
}


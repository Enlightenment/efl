#include "evas_gl_routines.h"
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>

/* uncomment this is partial buffer swaps slow - problem with glcopypixels? */
#define GLSWB 1
/* uncomment if your GL implimentation is CRAP at clipping */
/* #define GLNOCLIP 1 */

#define INTERSECTS(x, y, w, h, xx, yy, ww, hh) \
     ((x < (xx + ww)) && \
	 (y < (yy + hh)) && \
	 ((x + w) > xx) && \
	 ((y + h) > yy))

#define CLIP_TO(_x, _y, _w, _h, _cx, _cy, _cw, _ch) \
       { \
	  if (INTERSECTS(_x, _y, _w, _h, _cx, _cy, _cw, _ch)) \
		 { \
		    if (_x < _cx) \
		      { \
			 _w += _x - _cx; \
			 _x = _cx; \
			 if (_w < 0) _w = 0; \
		      } \
		    if ((_x + _w) > (_cx + _cw)) \
		      _w = _cx + _cw - _x; \
		    if (_y < _cy) \
		      { \
			 _h += _y - _cy; \
			 _y = _cy; \
                         if (_h < 0) _h = 0; \
		      } \
		    if ((_y + _h) > (_cy + _ch)) \
		      _h = _cy + _ch - _y; \
		 } \
		 else \
		 { \
		    _w = 0; _h = 0; \
		 } \
	      }


#define TT_VALID( handle )  ( ( handle ).z != NULL )

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

static Evas_GL_Window  *__evas_current  = NULL;

static Evas_List __evas_windows         = NULL;
static Evas_List __evas_contexts        = NULL;
static Evas_List __evas_images          = NULL;
static Evas_List __evas_fonts           = NULL;

static int       __evas_image_cache     = 0;
static int       __evas_image_cache_max = 512 * 1024;

static char    **__evas_fpath           = NULL;
static int       __evas_fpath_num       = 0;
static int       __evas_font_cache      = 0;
static int       __evas_font_cache_max  = 512 * 1024;

static int       __evas_rend_lut[9]   = { 0, 64, 128, 192, 255, 255, 255, 255, 255};

static int       __evas_have_tt_engine  = 0;
static TT_Engine __evas_tt_engine;

/* smooth (linear interp / supersample/mipmap scaling or "nearest" sampling */
static int       __evas_smooth          = 1;
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

static Evas_GL_Context *
__evas_gl_context_new(Display *disp, int screen)
{
   Evas_GL_Context *c;
   XSetWindowAttributes att;
   int eb, evb;
   
   if (!glXQueryExtension(disp, &eb, &evb)) return NULL;
   c = malloc(sizeof(Evas_GL_Context));
   c->disp = disp;
   c->screen = screen;
   c->visualinfo = glXChooseVisual(c->disp, c->screen, __evas_gl_configuration);
   c->visual = c->visualinfo->visual;
   c->context = glXCreateContext(c->disp, c->visualinfo, NULL, GL_TRUE);
   c->root = RootWindow(c->disp, c->screen);
   c->colormap = XCreateColormap(c->disp, c->root, c->visual, 0);
   att.colormap = c->colormap;
   att.border_pixel = 0;
   att.event_mask = 0;
   c->win = XCreateWindow(c->disp, c->root, 0, 0, 1, 1, 0,
			  c->visualinfo->depth, InputOutput,
			  c->visual, CWColormap | CWBorderPixel | CWEventMask,
			  &att);
   glXMakeCurrent(c->disp, c->win, c->context);
   glShadeModel(GL_FLAT);
   glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
   glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);   
   glEnable(GL_LINE_SMOOTH);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glPixelStorei(GL_PACK_ALIGNMENT, 1);
   
   c->texture = 0;
   glDisable(GL_TEXTURE_2D);
   c->dither = 0;
   glDisable(GL_DITHER);
   c->blend = 1;
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);   

   c->color = 0xffffffff;
   glColor4f(1.0, 1.0, 1.0, 1.0);
   
   c->clip.active = 0;
   c->clip.x = 0;
   c->clip.y = 0;
   c->clip.w = 0;
   c->clip.h = 0;
   glDisable(GL_SCISSOR_TEST);
   
   c->read_buf = GL_BACK;
   glReadBuffer(GL_BACK);
   c->write_buf = GL_FRONT;
   glDrawBuffer(GL_BACK);
   
   c->bound_texture = NULL;
   
   /* FIXME: need to determine these */
   c->max_texture_depth = 32;
   c->max_texture_size = 256;
   
   return c;
}

static Evas_GL_Context *
__evas_gl_context_lookup(Display *disp, int screen)
{
   Evas_List l;
   Evas_GL_Context *c;
   
   for (l = __evas_contexts; l; l = l->next)
     {
	c = l->data;
	if ((c->disp == disp) && (c->screen == screen))
	  {
	     if (l != __evas_contexts)
	       {
		  __evas_contexts = evas_list_remove(__evas_contexts, c);
		  __evas_contexts = evas_list_prepend(__evas_contexts, c);
	       }
	     return c;
	  }
     }
   c = __evas_gl_context_new(disp, screen);
   if (!c) return NULL;
   __evas_contexts = evas_list_prepend(__evas_contexts, c);
   return c;
}

static Evas_GL_Window *
__evas_gl_window_new(Display *disp, Window win)
{
   Evas_GL_Context *c;
   Evas_GL_Window *w;
   Window root;
   int screen, i;
   XWindowAttributes att;
   
   XGetWindowAttributes(disp, win, &att);
   root = att.root;
   screen = 0;
   for (i = 0; i < ScreenCount(disp); i++)
     {
	if (RootWindow(disp, i) == root) 
	  {
	     screen = i;
	     break;
	  }
     }
   c = __evas_gl_context_lookup(disp, screen);
   if (!c) return NULL;
   w = malloc(sizeof(Evas_GL_Window));
   w->disp = disp;
   w->win = win;
   w->context = c;
   w->root = root;
   w->screen = screen;
   w->updates = NULL;
   w->w = 0;
   w->h = 0;
   return w;
}

static Evas_GL_Window *
__evas_gl_window_lookup(Display *disp, Window win)
{
   Evas_List l;
   Evas_GL_Window *w;
   
   for (l = __evas_windows; l; l = l->next)
     {
	w = l->data;
	if ((w->win == win) && (w->disp == disp))
	  {
	     if (l != __evas_windows)
	       {
		  __evas_windows = evas_list_remove(__evas_windows, w);
		  __evas_windows = evas_list_prepend(__evas_windows, w);
	       }
	     return w;
	  }
     }
   w = __evas_gl_window_new(disp, win);
   if (!w) return NULL;
   __evas_windows = evas_list_prepend(__evas_windows, w);
   return w;
}

static Evas_GL_Window *
__evas_gl_window_current(Display *disp, Window win, int w, int h)
{
   Evas_GL_Window *glw;
   
   glw = __evas_gl_window_lookup(disp, win);
   if (!glw) return NULL;
   if (glw != __evas_current) 
     {
	double dr, dg, db, da;
	
	__evas_current = glw;
	glw->w = w;
	glw->h = h;
	glXMakeCurrent(glw->disp, glw->win, glw->context->context);
	glShadeModel(GL_FLAT);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);   
	glEnable(GL_LINE_SMOOTH);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	
	glViewport(0, 0, glw->w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, glw->w, 0, glw->h, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glScalef(1, -1, 1);
	glTranslatef(0, - glw->h, 0);	
	if (glw->context->dither) glEnable(GL_DITHER);
	else glDisable(GL_DITHER);
	if (glw->context->blend) 
	  {
	     glEnable(GL_BLEND);
	     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	  }
	else glDisable(GL_BLEND);
	if (glw->context->texture) glEnable(GL_TEXTURE_2D);
	else glDisable(GL_TEXTURE_2D);
	glColor4d((double)((glw->context->color >> 24) & 0xff) / 255.0,
		  (double)((glw->context->color >> 16) & 0xff) / 255.0,
		  (double)((glw->context->color >>  8) & 0xff) / 255.0,
		  (double)((glw->context->color      ) & 0xff) / 255.0);
	if (glw->context->clip.active)
	  {
	     glEnable(GL_SCISSOR_TEST);
	     glScissor(glw->context->clip.x, 
		       glw->h - glw->context->clip.y - glw->context->clip.h,
		       glw->context->clip.w, 
		       glw->context->clip.h);
	  }
	else
	  glDisable(GL_SCISSOR_TEST);
	if (glw->context->bound_texture)
	  glBindTexture(GL_TEXTURE_2D, glw->context->bound_texture->texture);
     }
   return glw;
}

static void
__evas_gl_window_dither(Evas_GL_Window *w, int onoff)
{
   if (!w) return;
   if (w->context->dither == onoff) return;

   w->context->dither = onoff;
   if (w->context->dither) glEnable(GL_DITHER);
   else glDisable(GL_DITHER);
}

static void
__evas_gl_window_blend(Evas_GL_Window *w, int onoff)
{
   if (!w) return;
   if (w->context->blend == onoff) return;

   w->context->blend = onoff;
   if (w->context->blend) 
     {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
     }
   else glDisable(GL_BLEND);
}

static void
__evas_gl_window_texture(Evas_GL_Window *w, int onoff)
{
   if (!w) return;
   if (w->context->texture == onoff) return;

   w->context->texture = onoff;
   if (w->context->texture) glEnable(GL_TEXTURE_2D);
   else glDisable(GL_TEXTURE_2D);
}

static void
__evas_gl_window_color(Evas_GL_Window *w, int r, int g, int b, int a)
{
   DATA32 col;
   
   if (!w) return;
   col = (r << 24) | (g << 16) | (b << 8) | (a);
   if (w->context->color == col) return;
   
   w->context->color = col;
   glColor4d((double)r / 255.0,
	     (double)g / 255.0,
	     (double)b / 255.0,
	     (double)a / 255.0);
}

static void
__evas_gl_window_clip(Evas_GL_Window *w, int c, int cx, int cy, int cw, int ch)
{
   if (!w) return;
   if (w->context->clip.active == c) 
     {
	if ((c) && 
	    (w->context->clip.x == cx) && (w->context->clip.y == cy) &&
	    (w->context->clip.w == cw) && (w->context->clip.h == ch))
	return;
     }
   w->context->clip.active = c;
   w->context->clip.x = cx;
   w->context->clip.y = cy;
   w->context->clip.w = cw;
   w->context->clip.h = ch;
   if (w->context->clip.active)
     {
	glEnable(GL_SCISSOR_TEST);
	glScissor(w->context->clip.x, 
		  w->h - w->context->clip.y - w->context->clip.h, 
		  w->context->clip.w, 
		  w->context->clip.h);
     }
   else
     glDisable(GL_SCISSOR_TEST);
}

static void
__evas_gl_window_write_buf(Evas_GL_Window *w, GLenum buf)
{
   if (!w) return;
   if (w->context->write_buf != buf)
     {
	w->context->write_buf = buf;
	glDrawBuffer(buf);
     }
}

static void
__evas_gl_window_read_buf(Evas_GL_Window *w, GLenum buf)
{
   if (!w) return;
   if (w->context->read_buf != buf)
     {
	w->context->read_buf = buf;
	glReadBuffer(buf);
     }
}

static void
__evas_gl_window_swap_rect(Evas_GL_Window *w, int rx, int ry, int rw, int rh)
{
   if (!w) return;
   
   __evas_gl_window_read_buf(w, GL_BACK);
   __evas_gl_window_write_buf(w, GL_FRONT);
   __evas_gl_window_blend(w, 0);
   __evas_gl_window_clip(w, 0, 0, 0, 0, 0);
   __evas_gl_window_dither(w, 0);
   ry = w->h - ry - rh;
   glRasterPos2i(rx, w->h - ry);
   glCopyPixels(rx, ry, rw, rh, GL_COLOR);
}

static void
__evas_gl_window_use_texture(Evas_GL_Window *w, Evas_GL_Texture *tex, int smooth)
{
   if (!w) return;
   if (w->context->bound_texture != tex)
     {
	glBindTexture(GL_TEXTURE_2D, tex->texture);
	w->context->bound_texture = tex;
     }
   if (smooth != tex->smooth)
     {
#ifdef HAVE_GLU
	if (smooth)
	  {
	     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	  }
	else
#else
	if (smooth)
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
	tex->smooth = smooth;
     }
}

static Evas_GL_Texture *
__evas_gl_texture_new(Evas_GL_Window *w, Imlib_Image im, int ix, int iy, int iw, int ih)
{
   Evas_GL_Texture *tex;
   int tx, ty, tw, th, im_w, im_h;
   DATA32 *data, *p1, *p2, *im_data;
   int shift;
   Imlib_Image prev_im;
   
   if (!w) return NULL;
   tex = malloc(sizeof(Evas_GL_Texture));
   shift = 1; while (iw > shift) shift = shift << 1; tw = shift;
   shift = 1; while (ih > shift) shift = shift << 1; th = shift;
   tex->w = tw;
   tex->h = th;
   glGenTextures(1, &(tex->texture));
   glBindTexture(GL_TEXTURE_2D, tex->texture);   
   w->context->bound_texture = tex;
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
   tex->smooth = 0;
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   prev_im = imlib_context_get_image();
   imlib_context_set_image(im);
   data = malloc(tw * th * 4);
   im_data = imlib_image_get_data_for_reading_only();
   im_w = imlib_image_get_width();
   im_h = imlib_image_get_height();
   for (ty = 0; ty < ih; ty++)
     {
	p1 = im_data + ((iy + ty) * im_w) + ix;
	p2 = data + (ty * tw);
	for (tx = 0; tx < iw; tx++)
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
	  {
	     for (; tx < tw; tx++) *p2 = p2[-1];
	  }
     }
   if (ty < th)
     {
	for (; ty < th; ty++)
	  {
	     p1 = data + ((ty - 1) * tw);
	     p2 = data + (ty * tw);
	     for (tx = 0; tx < tw; tx++)
	       {
		  *p2 = *p1; p2++; p1++;
	       }
	  }
     }
#ifdef HAVE_GLU
   if (imlib_image_has_alpha())
     {
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8, tw, th, GL_RGBA,
			  GL_UNSIGNED_BYTE, data);
     }
   else
     {
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, tw, th, GL_RGBA,
			  GL_UNSIGNED_BYTE, data);
     }
#else
   if (imlib_image_has_alpha())
     {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tw, th, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, data);
     }
   else
     {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, tw, th, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, data);
     }
#endif   
   free(data);
   imlib_image_put_back_data(im_data);
   imlib_context_set_image(prev_im);
   return tex;
}

static void
__evas_gl_texture_free(Evas_GL_Window *w, Evas_GL_Texture *tex)
{
   if (!w) return;
   glDeleteTextures(1, &tex->texture);
   if (w->context->bound_texture == tex) w->context->bound_texture = NULL;
   free(tex);
}

static Evas_GL_Texmesh *
__evas_gl_make_image_textures(Evas_GL_Window *w, Evas_GL_Image *image)
{
   Evas_List l;
   Evas_GL_Texmesh *tm;
   int tx, ty, ex, ey, tw, th, ix, iy, iw, ih;
   int i, shift;
   Imlib_Image prev_im;
   
   for (l = image->textures; l; l = l->next)
     {
	tm = l->data;
	if (tm->context == w->context) return tm;
     }
   tm = malloc(sizeof(Evas_GL_Texmesh));
   tm->window = w;
   tm->context = w->context;
   
   image->textures = evas_list_prepend(image->textures, tm);
   
   if ((!image->im) && (image->file))
     image->im = imlib_load_image(image->file);

   tx = (image->w - 2) / (w->context->max_texture_size - 2);
   ex = (image->w - 1) - (tx * (w->context->max_texture_size - 2));
   if (tx == 0) ex = image->w;
   tm->tiles.x_left = ex;
   shift = 1; while (ex > shift) shift = shift << 1; ex = shift;
   
   ty = (image->h - 2) / (w->context->max_texture_size - 2);
   ey = (image->h - 1) - (ty * (w->context->max_texture_size - 2));
   if (ty == 0) ey = image->h;
   tm->tiles.y_left = ey;
   shift = 1; while (ey > shift) shift = shift << 1; ey = shift;
   
   tm->tiles.x = tx;
   tm->tiles.y = ty;
   tm->tiles.x_edge = ex;
   tm->tiles.y_edge = ey;
   tm->textures = malloc(sizeof(Evas_GL_Texmesh *) * (tm->tiles.x + 1) * (tm->tiles.y + 1));
   i = 0;
   for (ty = 0; ty <= tm->tiles.y; ty++)
     {
	/* figure out image y,h */
	/* texture is only an edge size */
	if ((ty == 0) && (tm->tiles.y == 0))
	  {
	     iy = 0;
	     ih = image->h;
	  }
	/* start edge */
	else if (ty == 0)
	  {
	     iy = 0;
	     ih = w->context->max_texture_size;
	  }
	/* end edge */
	else if (ty == tm->tiles.y)
	  {
	     iy = ty * (w->context->max_texture_size - 2) + 1;
	     ih = image->h - iy;
	  }
	/* middle tex */
	else
	  {
	     iy = ty * (w->context->max_texture_size - 2) + 1;
	     ih = w->context->max_texture_size;
	  }
	for (tx = 0; tx <= tm->tiles.x; tx++)
	  {
	     /* figure out image x,w */
	     /* texture is only an edge size */
	     if ((tx == 0) && (tm->tiles.x == 0))
	       {
		  ix = 0;
		  iw = image->w;
	       }
	     /* start edge */
	     else if (tx == 0)
	       {
		  ix = 0;
		  iw = w->context->max_texture_size;
	       }
	     /* end edge */
	     else if (tx == tm->tiles.x)
	       {
		  ix = tx * (w->context->max_texture_size - 2) + 1;
		  iw = image->w - ix;
	       }
	     /* middle tex */
	     else
	       {
		  ix = tx * (w->context->max_texture_size - 2) + 1;
		  iw = w->context->max_texture_size;
	       }
	     if (image->im)
	       tm->textures[i] = __evas_gl_texture_new(w, image->im, 
						       ix, iy, iw, ih);
	     i++;
	  }
     }
   if ((image->file) && (image->im))
     {
	prev_im = imlib_context_get_image();
	imlib_context_set_image(image->im);
	imlib_free_image_and_decache();
	image->im = NULL;
	imlib_context_set_image(prev_im);
     }
   return tm;
}

static void
__evas_gl_free_image_textures(Evas_GL_Image *image)
{
   Evas_List l;
   
   if (image->textures)
     {
	for (l = image->textures; l; l = l->next)
	  {
	     Evas_GL_Texmesh *tm;
	     Evas_GL_Window *w;
	     int i;
	     
	     tm = l->data;
	     w = __evas_gl_window_current(tm->window->disp, tm->window->win, 
					  tm->window->w, tm->window->h);
	     for (i = 0; i < (tm->tiles.x + 1) * (tm->tiles.y + 1); i++)
	       __evas_gl_texture_free(w, tm->textures[i]);
	     free(tm->textures);
	     tm->tiles.x = 0;
	     tm->tiles.y = 0;
	     tm->tiles.x_edge = 0;
	     tm->tiles.y_edge = 0;
	     free(tm);
	  }
	evas_list_free(image->textures);
     }
   image->textures = NULL;
}

static Evas_GL_Image *
__evas_gl_image_alloc(char *file)
{
   Evas_GL_Image *image;
   Imlib_Image im, prev_im;
   
   im = imlib_load_image(file);
   if (!im) return NULL;
   image = malloc(sizeof(Evas_GL_Image));
   image->file = strdup(file);
   image->im = im;
   prev_im = imlib_context_get_image();
   imlib_context_set_image(im);
   image->w = imlib_image_get_width();
   image->h = imlib_image_get_height();
   image->has_alpha = imlib_image_has_alpha();
   image->border.l = 0;
   image->border.r = 0;
   image->border.t = 0;
   image->border.b = 0;
   image->references = 1;
   image->textures = NULL;
   imlib_context_set_image(prev_im);
   return image;
}

static void
__evas_gl_image_dealloc(Evas_GL_Image *image)
{
   Imlib_Image prev_im;
   
   if (image->file) free(image->file);
   if (image->im)
     {
	prev_im = imlib_context_get_image();
	imlib_context_set_image(image->im);
	imlib_free_image();
	imlib_context_set_image(prev_im);
     }
   if (image->textures)
     __evas_gl_free_image_textures(image);
   free(image);
}

static void
__evas_gl_image_cache_flush(void)
{
   while (__evas_image_cache > __evas_image_cache_max)
     {
	Evas_GL_Image *im, *im_last;
	Evas_List l;
	
	im_last = NULL;
	for (l = __evas_images; l; l = l->next)
	  {
	     im = l->data;
	     if (im->references == 0) im_last = im;
	  }
	if (im_last)
	  {
	     __evas_image_cache -= im_last->w * im_last->h * 4;
	     __evas_images = evas_list_remove(__evas_images, im_last);
	     __evas_gl_image_dealloc(im_last);
	  }
     }
}

static Evas_GL_Graident_Texture *
__evas_gl_gradient_texture_new(Evas_GL_Window *w, Evas_GL_Graident *gr)
{
   Evas_List l;
   Evas_GL_Graident_Texture *tg;
   
   for (l = gr->textures; l; l = l->next)
     {
	tg = l->data;
	if (tg->context == w->context) return tg;
     }
   tg = malloc(sizeof(Evas_GL_Graident_Texture));
   tg->window = w;
   tg->context = w->context;
   tg->texture = NULL;
   gr->textures = evas_list_prepend(gr->textures, tg);
   return tg;
}

static void
__evas_gl_gradient_texture_free(Evas_GL_Graident_Texture *tg)
{
   if (tg->texture)
     __evas_gl_texture_free(tg->window, tg->texture);
   free(tg);
}

static Evas_GL_Texture *
__evas_gl_texture_alpha_new(Evas_GL_Window *w)
{
   Evas_GL_Texture *tex;
   DATA8 *data;
   
   if (!w) return NULL;
   tex = malloc(sizeof(Evas_GL_Texture));
   tex->w = w->context->max_texture_size;
   tex->h = w->context->max_texture_size;
   glGenTextures(1, &(tex->texture));
   glBindTexture(GL_TEXTURE_2D, tex->texture);   
   w->context->bound_texture = tex;
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
   tex->smooth = 0;
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   data = malloc(w->context->max_texture_size * w->context->max_texture_size);
   memset(data, 0, w->context->max_texture_size * w->context->max_texture_size);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA4, 
		w->context->max_texture_size, w->context->max_texture_size, 0,
		GL_ALPHA, GL_UNSIGNED_BYTE, data);
   free(data);
   return tex;
}

static void
__evas_gl_texture_paste_data(Evas_GL_Window *w, Evas_GL_Texture *tex, int x, int y, DATA8 *data, int iw, int ih)
{
   __evas_gl_window_use_texture(w, tex, 0);
   glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, iw, ih, 
		   GL_ALPHA, GL_UNSIGNED_BYTE, data);
}

static Evas_GL_Glyph *
__evas_gl_text_font_get_glyph(Evas_GL_Font *fn, int glyph)
{
   Evas_GL_Glyph *g;
   Evas_List l;
   int hash;
   int code;
   
   hash = glyph & 0xff;
   for (l = fn->glyphs[hash]; l; l = l->next)
     {
	g = l->data;
	if (g->glyph_id == glyph) 
	  {
	     if (l != fn->glyphs[hash])
	       {
		  fn->glyphs[hash] = evas_list_remove(fn->glyphs[hash], g);
		  fn->glyphs[hash] = evas_list_prepend(fn->glyphs[hash], g);
	       }
	     return g;
	  }
     }
   g = malloc(sizeof(Evas_GL_Glyph));
   g->glyph_id = glyph;
   TT_New_Glyph(fn->face, &(g->glyph));
   code = TT_Char_Index(fn->char_map, glyph);
   TT_Load_Glyph(fn->instance, g->glyph, code, TTLOAD_SCALE_GLYPH | TTLOAD_HINT_GLYPH);   
   TT_Get_Glyph_Metrics(g->glyph, &(g->metrics));
   g->texture = NULL;
   g->textures = NULL;
   g->tex.x1 = 0.0;
   g->tex.x2 = 0.0;
   g->tex.y1 = 0.0;
   g->tex.y2 = 0.0;
   fn->glyphs[hash] = evas_list_prepend(fn->glyphs[hash], g);   
   return g;
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
__evas_gl_text_font_render_glyph(Evas_GL_Window *glw, Evas_GL_Font *fn, Evas_GL_Glyph *g)
{
   Evas_List l;
   Evas_GL_Font_Texture *ft;
   Evas_GL_Glyph_Texture *gt;
   Evas_GL_Texture *tex;
   int tx, ty, tw, th, xmin, ymin, xmax, ymax;
   TT_Raster_Map *rmap;
   
   if ((g->texture) && (g->texture->context == glw->context)) return;
   for (l = g->textures; l; l = l->next)
     {
	gt = l->data;
	if (gt->context == glw->context)
	  {
	     g->texture = gt;
	     g->tex.x1 = gt->tex.x1;
	     g->tex.x2 = gt->tex.x2;
	     g->tex.y1 = gt->tex.y1;
	     g->tex.y2 = gt->tex.y2;
	     return;
	  }
     }
   
   xmin = g->metrics.bbox.xMin & -64;
   ymin = g->metrics.bbox.yMin & -64;
   xmax = (g->metrics.bbox.xMax + 63) & -64;
   ymax = (g->metrics.bbox.yMax + 63) & -64;
   tw = ((xmax - xmin) / 64) + 1;
   th = ((ymax - ymin) / 64) + 1;
   /* could never fit glyph into max texture size tile */
   if ((tw > glw->context->max_texture_size) ||
       (th > glw->context->max_texture_size)) 
     {
	g->texture = NULL;
	return;
     }
   
   /* find a new texture with space */
   ft = NULL;
   tex = NULL;
   tx = 0;
   ty = 0;
   for (l = fn->textures; l; l = l->next)
     {
	ft = l->data;
	if (ft->context == glw->context)
	  {
	     tex = ft->texture;
	     
	     if ((tex->h - ft->cursor.y) >= th)
	       {
		  if ((tex->w - ft->cursor.x) >= tw)
		    {
		       if (ft->cursor.row_h < th) ft->cursor.row_h = th;
		       tx = ft->cursor.x;
		       ty = ft->cursor.y;
		       ft->cursor.x += tw;
		       goto done;
		    }
		  else
		    {
		       ty = ft->cursor.y + ft->cursor.row_h;
		       if ((tex->h - ty) >= th)
			 {
			    ft->cursor.y += ft->cursor.row_h;
			    ft->cursor.row_h = th;
			    ft->cursor.x = tw;
			    tx = 0;
			    ty = ft->cursor.y;
			    goto done;
			 }
		    }
	       }
	  }
	ft = NULL;
     }
   done:
    if (!ft)
     {
	/* make a new texture with space */
	tex = __evas_gl_texture_alpha_new(glw);
	ft = malloc(sizeof(Evas_GL_Font_Texture));
	ft->cursor.x = tw;
	ft->cursor.y = 0;
	ft->cursor.row_h = th;
	ft->window = glw;
	ft->context = glw->context;
	ft->texture = tex;
	
	fn->textures = evas_list_prepend(fn->textures, ft);
	tx = 0;
	ty = 0;
     }
   /* paste glyph in free spot in texture */
   gt = malloc(sizeof(Evas_GL_Glyph_Texture));

   rmap = __evas_gl_text_font_raster_new(tw, th);
   if (rmap)
     {
	int x, y;
	DATA8 *data;
	
	TT_Get_Glyph_Pixmap(g->glyph, rmap, -xmin, -ymin);
	data = malloc(tw * th * sizeof(DATA8));
	for (y = 0; y < th; y++)
	  {
	     for (x = 0; x < tw; x++)
	       {
		  int rval;
		  
		  rval = (int)(((unsigned char *)(rmap->bitmap))[((rmap->rows - y - 1) * rmap->cols) + x]);
		  data[(y * tw) + x] = __evas_rend_lut[rval];
	       }
	  }
	__evas_gl_texture_paste_data(glw, tex, tx, ty, data, tw, th);
	free(data);
	__evas_gl_text_font_raster_free(rmap);
     }
   gt->texture = tex;
   gt->window = glw;
   gt->context = glw->context;
   gt->tex.x1 = (double)tx / (double)glw->context->max_texture_size;
   gt->tex.x2 = (double)(tx + tw) / (double)glw->context->max_texture_size;
   gt->tex.y1 = (double)ty / (double)glw->context->max_texture_size;
   gt->tex.y2 = (double)(ty + th) / (double)glw->context->max_texture_size;
   g->textures = evas_list_prepend(g->textures, gt);
   
   g->texture = gt;
   g->tex.x1 = gt->tex.x1;
   g->tex.x2 = gt->tex.x2;
   g->tex.y1 = gt->tex.y1;
   g->tex.y2 = gt->tex.y2;
}

static int
__evas_gl_is_file(char *file)
{
   struct stat         st;
   
   if (stat(file, &st) < 0)
     return 0;
   return 1;
}

static char *
__evas_gl_font_find(char *font)
{
   char buf[4096];
   char *ext[] = {".ttf", ".TTF", ""};
   int i, j;
   
   for (i = 0; i < 3; i++)
     {
	sprintf(buf, "%s%s", font, ext[i]);
	if (__evas_gl_is_file(buf)) return strdup(buf);
     }
   for (j = 0; j < __evas_fpath_num; j++)
     {
	for (i = 0; i < 3; i++)
	  {
	     sprintf(buf, "%s/%s%s", __evas_fpath[j], font, ext[i]);
	     if (__evas_gl_is_file(buf)) return strdup(buf);
	  }
     }
   return NULL;
}

static Evas_GL_Font *
__evas_gl_font_load(char *font, int size)
{
   Evas_GL_Font *fn;
   char *file;
   TT_Error error;
   int i, num_cmap, upm;
   const int dpi = 96;
   
   file = __evas_gl_font_find(font);
   if (!file) return;
   if (!__evas_have_tt_engine)
     {	
	error = TT_Init_FreeType(&__evas_tt_engine);
	if (error) return NULL;
	__evas_have_tt_engine = 1;	
     }
   fn = malloc(sizeof(Evas_GL_Font));
   fn->font = strdup(font);
   fn->size = size;
   fn->engine = __evas_tt_engine;
   error = TT_Open_Face(fn->engine, file, &fn->face);
   if (error)
     {
	free(fn->font);
	free(fn);
	free(file);
	return NULL;
     }
   free(file);
   error = TT_Get_Face_Properties(fn->face, &fn->properties);
   if (error)
     {
	TT_Close_Face(fn->face);
	free(fn->font);
	free(fn);
	return NULL;
     }
   error = TT_New_Instance(fn->face, &fn->instance);   
   if (error)
     {
	TT_Close_Face(fn->face);
	free(fn->font);
	free(fn);
	return NULL;
     }
   TT_Set_Instance_Resolutions(fn->instance, dpi, dpi);
   TT_Set_Instance_CharSize(fn->instance, size * 64);
   TT_Get_Instance_Metrics(fn->instance, &fn->metrics);
   upm = fn->properties.header->Units_Per_EM;
   fn->ascent = (fn->properties.horizontal->Ascender * fn->metrics.y_ppem) / upm;
   fn->descent = (fn->properties.horizontal->Descender * fn->metrics.y_ppem) / upm;
   if (fn->descent < 0) fn->descent = -fn->descent;
   num_cmap = fn->properties.num_CharMaps;
   for (i = 0; i < num_cmap; i++)
     {
	unsigned short      platform, encoding;
	
	TT_Get_CharMap_ID(fn->face, i, &platform, &encoding);
	if ((platform == 3 && encoding == 1) ||
	    (platform == 0 && encoding == 0))
	  {
	     TT_Get_CharMap(fn->face, i, &fn->char_map);
	     break;
	  }	
     }
   if (i == num_cmap)
     TT_Get_CharMap(fn->face, 0, &fn->char_map);
   fn->max_descent = 0;
   fn->max_ascent = 0;
   memset(fn->glyphs, 0, sizeof(Evas_List) * 256);
   fn->textures = NULL;
   fn->references = 1;
   
   /* go thru the first 256 glyps to calculate max ascent/descent */
     {
	Evas_GL_Glyph *g;
	
	for (i = 0; i < 256; i++)
	  {
	     g = __evas_gl_text_font_get_glyph(fn, i);
	     if (!g) continue;
	     if (!TT_VALID(g->glyph)) continue;
	     if ((g->metrics.bbox.yMin & -64) < fn->max_descent)
	       fn->max_descent = (g->metrics.bbox.yMin & -64);
	     if (((g->metrics.bbox.yMax + 63) & -64) > fn->max_ascent)
	       fn->max_ascent = ((g->metrics.bbox.yMax + 63) & -64);
	  }
     }
   
   if (((fn->ascent == 0) && (fn->descent == 0)) || (fn->ascent == 0))
     {
	fn->ascent = fn->max_ascent / 64;
	fn->descent = -fn->max_descent / 64;
     }   
   TT_Flush_Face(fn->face);
   return fn;
}

static void
__evas_gl_text_font_cache_flush(void)
{
   Evas_List l;
   Evas_GL_Font *fn_last;
   Evas_GL_Font *fn;
   
   while (__evas_font_cache > __evas_font_cache_max)
     {
	fn_last = NULL;
	for (l = __evas_fonts; l; l = l->next)
	  {	     
	     fn = l->data;
	     if (fn->references == 0) fn_last = fn;
	  }
	if (fn_last)
	  {
	     int i;
	     
	     for (l = fn_last->textures; l; l = l->next)
	       {
		  Evas_GL_Font_Texture *ft;
		  
		  ft = l->data;
		  __evas_font_cache -= ft->texture->w * ft->texture->h;
	       }
	     TT_Done_Instance(fn_last->instance);
	     TT_Close_Face(fn_last->face);
	     if (fn_last->font) free(fn_last->font);
	     for (i = 0; i < 256; i++)
	       {
		  if (fn_last->glyphs[i])
		    {
		       for (l = fn_last->glyphs[i]; l; l = l->next)
			 {
			    Evas_GL_Glyph *g;
			    Evas_List ll;
			    
			    g = l->data;
			    if (g->textures)
			      {
				 for (ll = g->textures; ll; ll = ll->next)
				   {
				      Evas_GL_Glyph_Texture *gt;
				      
				      gt = ll->data;
				      free(gt);
				   }
				 evas_list_free(g->textures);
			      }
			    free(g);
			 }
		       evas_list_free(fn_last->glyphs[i]);
		    }
	       }
	     if (fn_last->textures)
	       {
		  for (l = fn_last->textures; l; l = l->next)
		    {
		       Evas_GL_Font_Texture *ft;
		       
		       ft = l->data;
		       __evas_gl_texture_free(ft->window, ft->texture);
		       free(ft);
		    }
		  evas_list_free(fn_last->textures);
	       }
	     __evas_fonts = evas_list_remove(__evas_fonts, fn_last);
	     free(fn_last);
	  }
     }
}

/*****************************************************************************/
/* image externals ***********************************************************/
/*****************************************************************************/

void
__evas_gl_image_draw(Evas_GL_Image *im, 
		     Display *disp, Imlib_Image dstim, 
		     Window w, int win_w, int win_h,
		     int src_x, int src_y, int src_w, int src_h,
		     int dst_x, int dst_y, int dst_w, int dst_h,
		     int cr, int cg, int cb, int ca)
{
   Evas_GL_Window *glw;
   Evas_List l;
   Evas_GL_Texmesh *tm;

   double scx, scy;
   double x0, y0, w0, h0;
   int go;
   
   if (__evas_clip)
     {
	cr = (cr * __evas_clip_r) / 255;
	cg = (cg * __evas_clip_g) / 255;
	cb = (cb * __evas_clip_b) / 255;
	ca = (ca * __evas_clip_a) / 255;
     }
   if (ca <= 0) return;
   if (src_w < 1) src_w = 1;
   if (src_h < 1) src_h = 1;
   if (dst_w < 1) return;
   if (dst_h < 1) return;
   glw = __evas_gl_window_current(disp, w, win_w, win_h);
   if (!glw) return;
   __evas_gl_window_texture(glw, 1);
   __evas_gl_window_blend(glw, im->has_alpha);
   __evas_gl_window_write_buf(glw, GL_BACK);
   __evas_gl_window_read_buf(glw, GL_BACK);
   __evas_gl_window_color(glw, cr, cg, cb, ca);
   __evas_gl_window_dither(glw, 1);

   tm = __evas_gl_make_image_textures(glw, im);
   scx = (double)dst_w / (double)src_w;
   scy = (double)dst_h / (double)src_h;
   
   x0 = (double)dst_x - ((double)src_x * scx);
   y0 = (double)dst_y - ((double)src_y * scy);
   w0 = (double)im->w * scx;
   h0 = (double)im->h * scy;

#ifndef GLNOCLIP   
   for (l = glw->updates; l; l = l->next)
     {
	Evas_GL_Rect *rect;
	
	go = 1;
	rect = l->data;
	  {
	     int rx, ry, rw, rh;
	     
	     rx = dst_x;
	     ry = dst_y;
	     rw = dst_w;
	     rh = dst_h;
	     CLIP_TO(rx, ry, rw, rh, rect->x, rect->y, rect->w, rect->h);
	     if (__evas_clip)
	       {
		  CLIP_TO(rx, ry, rw, rh, 
			  __evas_clip_x, __evas_clip_y, 
			  __evas_clip_w, __evas_clip_h);
	       }
	     if ((rw > 0) && (rh > 0))
	       __evas_gl_window_clip(glw, 1, rx, ry, rw, rh);	     
	     else go = 0;
	  }
#else	
	go = 1;
	  {
	     int rx, ry, rw, rh;
	     
	     rx = dst_x;
	     ry = dst_y;
	     rw = dst_w;
	     rh = dst_h;
             if (__evas_clip)
	       {
		  CLIP_TO(rx, ry, rw, rh, 
			  __evas_clip_x, __evas_clip_y, 
			  __evas_clip_w, __evas_clip_h);
		  if ((rw > 0) && (rh > 0))
		    __evas_gl_window_clip(glw, 1, rx, ry, rw, rh);	     
		  else go = 0;
	       }
	     else
	       {
		  if (!((src_x != 0) || (src_w != im->w) ||
			(src_y != 0) || (src_h != im->h)))
		    __evas_gl_window_clip(glw, 0, 0, 0, 0, 0);
	       }
	  }
#endif	
	/* render image here */
	if (go)
	  {
	     int tx, ty, t;
	     double ty1, ty2, tx1, tx2;
	     double x1, x2, y1, y2;
	     int smooth = 0;
	     
	     if (__evas_smooth)
	       {
		  if ((dst_w == src_w) && (dst_h == src_h))
		    smooth = 0;
		  else smooth = __evas_smooth;
	       }
	     else
	       smooth = 0;
	     t = 0;
	     for (ty = 0; ty <= tm->tiles.y; ty++)
	       {
		  if ((ty == 0) && (ty < tm->tiles.y))
		    {
		       y1 = y0;
		       y2 = y1 + ((double)(glw->context->max_texture_size - 1) * scy);
		       ty1 = 0.0;
		       ty2 = (double)(glw->context->max_texture_size - 1) / (double)glw->context->max_texture_size;
		    }
		  else if (ty < tm->tiles.y)
		    {
		       y1 = y0 + (((double)(glw->context->max_texture_size - 1) + ((double)(ty - 1) * (double)(glw->context->max_texture_size - 2))) * scy);
		       y2 = y1 + ((double)(glw->context->max_texture_size - 2) * scy);
		       ty1 = 1.0 / (double)glw->context->max_texture_size;
		       ty2 = (double)(glw->context->max_texture_size - 1) / (double)glw->context->max_texture_size;
		    }
		  else
		    {
		       if (ty == 0) y1 = y0;
		       else 
			 y1 = y0 + h0 - ((double)tm->tiles.y_left * scy);
		       y2 = y0 + h0;
		       if (ty == 0) ty1 = 0;
		       else ty1 = 1 / (double)tm->tiles.y_edge;
		       ty2 = (double)tm->tiles.y_left / (double)tm->tiles.y_edge;
		    }
		  for (tx = 0; tx <= tm->tiles.x; tx++)
		    {
		       if ((tx == 0) && (tx < tm->tiles.x))
			 {
			    x1 = x0;
			    x2 = x1 + (255 * scx);
			    tx1 = 0.0;
			    tx2 = (double)(glw->context->max_texture_size - 1) / (double)glw->context->max_texture_size;
			 }
		       else if (tx < tm->tiles.x)
			 {
			    x1 = x0 + (((double)(glw->context->max_texture_size - 1) + ((double)(tx - 1) * (double)(glw->context->max_texture_size - 2))) * scx);
			    x2 = x1 + ((double)(glw->context->max_texture_size - 2) * scx);
			    tx1 = 1.0 / (double)glw->context->max_texture_size;
			    tx2 = (double)(glw->context->max_texture_size - 1) / (double)glw->context->max_texture_size;
			 }
		       else
			 {
			    if (tx == 0) x1 = x0;
			    else
			      x1 = x0 + w0 - ((double)tm->tiles.x_left * scx);
			    x2 = x0 + w0;
			    if (tx == 0) tx1 = 0;
			    else tx1 = 1 / (double)tm->tiles.x_edge;
			    tx2 = (double)tm->tiles.x_left / (double)tm->tiles.x_edge;
			 }
		       __evas_gl_window_use_texture(glw, tm->textures[t++], smooth);
		       glBegin(GL_QUADS);
		       glTexCoord2d(tx1, ty1); glVertex2d(x1, y1);
		       glTexCoord2d(tx2, ty1); glVertex2d(x2, y1);
		       glTexCoord2d(tx2, ty2); glVertex2d(x2, y2);
		       glTexCoord2d(tx1, ty2); glVertex2d(x1, y2);
		       glEnd();			       
		    }		  
	       }
	  }
#ifndef GLNOCLIP   
     }
#endif
}

Evas_GL_Image *
__evas_gl_image_new_from_file(Display *disp, char *file)
{
   Evas_List l;
   Evas_GL_Image *image;
   
   for (l = __evas_images; l; l = l->next)
     {
	
	image = l->data;
	
	if ((image->file) && (!strcmp(image->file, file)))
	  {
	     if (image->references == 0)
	       {
		  __evas_image_cache -= image->w * image->h * 4;
	       }
	     image->references++;
	     if (l != __evas_images)
	       {
		  __evas_images = evas_list_remove(__evas_images, image);
		  __evas_images = evas_list_prepend(__evas_images, image);
	       }
	     return image;
	  }
     }
   image = __evas_gl_image_alloc(file);
   if (image) __evas_images = evas_list_prepend(__evas_images, image);
   return image;
}

void
__evas_gl_image_free(Evas_GL_Image *im)
{
   if (im->references > 0)
     im->references--;
   if (im->references == 0)
     {
	__evas_image_cache += im->w * im->h * 4;
	__evas_gl_image_cache_flush();
     }
}

void
__evas_gl_image_cache_empty(Display *disp)
{
   int prev_cache;
   
   prev_cache = __evas_image_cache_max;
   __evas_image_cache_max = 0;
   __evas_gl_image_cache_flush();
   __evas_image_cache_max = prev_cache;
}

void
__evas_gl_image_cache_set_size(Display *disp, int size)
{
   __evas_image_cache_max = size;
   __evas_gl_image_cache_flush();
}

int
__evas_gl_image_cache_get_size(Display *disp)
{
   return __evas_image_cache_max;
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
   im->border.l = left;
   im->border.r = right;
   im->border.t = top;
   im->border.b = bottom;
}

void
__evas_gl_image_set_smooth_scaling(int on)
{
   __evas_smooth = on;
}

/*****************************************************************************/
/* font externals ************************************************************/
/*****************************************************************************/

Evas_GL_Font *
__evas_gl_text_font_new(Display *disp, char *font, int size)
{
   Evas_List l;
   Evas_GL_Font *fn;

   if (!font) return NULL;
   for (l = __evas_fonts; l; l = l->next)
     {
	Evas_GL_Font *fn;
	
	fn = l->data;
	if (!strcmp(fn->font, font) && (size == fn->size))
	  {
	     if (l != __evas_fonts)
	       {
		  __evas_fonts = evas_list_remove(__evas_fonts, fn);
		  __evas_fonts = evas_list_prepend(__evas_fonts, fn);
	       }
	     if (fn->references == 0)
	       {
		  for (l = fn->textures; l; l = l->next)
		    {
		       Evas_GL_Font_Texture *ft;
		       
		       ft = l->data;
		       __evas_font_cache -= ft->texture->w * ft->texture->h;
		    }
	       }
	     fn->references++;
	     return fn;
	  }
     }
   fn = __evas_gl_font_load(font, size);
   if (!fn) return NULL;
   __evas_fonts = evas_list_prepend(__evas_fonts, fn);
   return fn;
}

void
__evas_gl_text_font_free(Evas_GL_Font *fn)
{
   if (!fn) return;
   if (fn->references >= 0)
     {
	fn->references--;
	if (fn->references == 0)
	  {
	     Evas_List l;
	     
	     for (l = fn->textures; l; l = l->next)
	       {
		  Evas_GL_Font_Texture *ft;
		  
		  ft = l->data;
		  __evas_font_cache += ft->texture->w * ft->texture->h;
	       }
	  }
     }
   __evas_gl_text_font_cache_flush();
}

int
__evas_gl_text_font_get_ascent(Evas_GL_Font *fn)
{
   if (!fn) return 0;
   return fn->ascent;
}

int
__evas_gl_text_font_get_descent(Evas_GL_Font *fn)
{
   if (!fn) return 0;
   return fn->descent;
}

int
__evas_gl_text_font_get_max_ascent(Evas_GL_Font *fn)
{
   if (!fn) return 0;
   return fn->max_ascent;
}

int
__evas_gl_text_font_get_max_descent(Evas_GL_Font *fn)
{
   if (!fn) return 0;
   return fn->max_descent;
}

void
__evas_gl_text_font_get_advances(Evas_GL_Font *fn, char *text, 
				 int *advance_horiz,
				 int *advance_vert)
{
   if (advance_horiz) *advance_horiz = 0;
   if (advance_horiz) *advance_vert = 0;
   if (!fn) return;
   if (!text) return;
   if (text[0] == 0) return;
}

int
__evas_gl_text_font_get_first_inset(Evas_GL_Font *fn, char *text)
{
   if (!fn) return 0;
   if (!text) return 0;
   if (text[0] == 0) return 0;
}

void
__evas_gl_text_font_add_path(char *path)
{
   int i;
   
   for (i = 0; i < __evas_fpath_num; i++)
     {
	if (!strcmp(path, __evas_fpath[i])) return;
     }
   __evas_fpath_num++;
   if (!__evas_fpath) __evas_fpath = malloc(sizeof(char *));
   else __evas_fpath = realloc(__evas_fpath, 
			       (__evas_fpath_num * sizeof(char *)));
   __evas_fpath[__evas_fpath_num - 1] = strdup(path);
}

void
__evas_gl_text_font_del_path(char *path)
{
   int i, j;
   
   for (i = 0; i < __evas_fpath_num; i++)
     {
	if (!strcmp(path, __evas_fpath[i]))
	  {
	     __evas_fpath_num--;
	     for (j = i; j < __evas_fpath_num; j++) 
	       __evas_fpath[j] = __evas_fpath[j + 1];
	     if (__evas_fpath_num > 0) 
	       __evas_fpath = realloc(__evas_fpath, 
				      __evas_fpath_num * sizeof(char *));
	     else
	       {
		  free(__evas_fpath);
		  __evas_fpath = NULL;
	       }
	  }
     }
}

char **
__evas_gl_text_font_list_paths(int *count)
{
   *count = __evas_fpath_num;
   return __evas_fpath;
}

void
__evas_gl_text_cache_empty(Display *disp)
{
   int prev_cache;
   
   prev_cache = __evas_font_cache_max;
   __evas_font_cache_max = 0;
   __evas_gl_text_font_cache_flush();
   __evas_font_cache_max = prev_cache;
}

void
__evas_gl_text_cache_set_size(Display *disp, int size)
{
   __evas_font_cache_max = size;   
   __evas_gl_text_font_cache_flush();
}

int
__evas_gl_text_cache_get_size(Display *disp)
{
   return __evas_font_cache_max;
}

void
__evas_gl_text_draw(Evas_GL_Font *fn, Display *disp, Imlib_Image dstim, Window win, 
		    int win_w, int win_h, int x, int y, char *text, 
		    int cr, int cg, int cb, int ca)
{
   Evas_GL_Window *glw;
   Evas_List l;
   int go;
   
   if (__evas_clip)
     {
	cr = (cr * __evas_clip_r) / 255;
	cg = (cg * __evas_clip_g) / 255;
	cb = (cb * __evas_clip_b) / 255;
	ca = (ca * __evas_clip_a) / 255;
     }
   if (ca <= 0) return;
   glw = __evas_gl_window_current(disp, win, win_w, win_h);
   if (!glw) return;
   __evas_gl_window_texture(glw, 1);
   __evas_gl_window_blend(glw, 1);
   __evas_gl_window_write_buf(glw, GL_BACK);
   __evas_gl_window_read_buf(glw, GL_BACK);
   __evas_gl_window_color(glw, cr, cg, cb, ca);
   __evas_gl_window_dither(glw, 1);

#ifndef GLNOCLIP   
   for (l = glw->updates; l; l = l->next)
     {
	Evas_GL_Rect *rect;
	
	go = 1;
	rect = l->data;
	  {
	     int rx, ry, rw, rh;
	     
	     rx = 0;
	     ry = 0;
	     rw = win_w;
	     rh = win_h;
	     CLIP_TO(rx, ry, rw, rh, rect->x, rect->y, rect->w, rect->h);
	     if (__evas_clip)
	       {
		  CLIP_TO(rx, ry, rw, rh, 
			  __evas_clip_x, __evas_clip_y, 
			  __evas_clip_w, __evas_clip_h);
	       }
	     if ((rw > 0) && (rh > 0))
	       __evas_gl_window_clip(glw, 1, rx, ry, rw, rh);	     
	     else go = 0;
	  }
#else	
	go = 1;
	  {
	     int rx, ry, rw, rh;
	     
	     rx = 0;
	     ry = 0;
	     rw = win_w;
	     rh = win_h;
             if (__evas_clip)
	       {
		  CLIP_TO(rx, ry, rw, rh, 
			  __evas_clip_x, __evas_clip_y, 
			  __evas_clip_w, __evas_clip_h);
		  if ((rw > 0) && (rh > 0))
		    __evas_gl_window_clip(glw, 1, rx, ry, rw, rh);	     
		  else go = 0;
	       }
	     else
	       __evas_gl_window_clip(glw, 0, 0, 0, 0, 0);
	  }
#endif	
	/* render string here */
	if (go)
	  {
	     int i;
	     int x_offset, y_offset;
	     int glyph, rows;
	     Evas_GL_Glyph *g;
	     	     
	     if (text[0] == 0) return;
	     glyph = ((unsigned char *)text)[0];
	     g = __evas_gl_text_font_get_glyph(fn, glyph);
	     if (!TT_VALID(g->glyph)) 
#ifndef GLNOCLIP	       
	       continue;
#else
	       return;
#endif	     
	     x_offset = 0;
	     if (g) x_offset = - (g->metrics.bearingX / 64);
	     y_offset = -(fn->max_descent / 64);
	     __evas_gl_text_get_size(fn, text, NULL, &rows);
	     for (i = 0; text[i]; i++)
	       {
		  int xmin, ymin, xmax, ymax, off, adj;
		  
		  /* for internationalization this here wouldnt just use */
		  /* the char value of the text[i] but translate form utf-8 */
		  /* or whetever and incriment i appropriately and set g to */
		  /* the glyph index */
		  glyph = ((unsigned char *)text)[i];
		  g = __evas_gl_text_font_get_glyph(fn, glyph);
		  __evas_gl_text_font_render_glyph(glw, fn, g);
		  if (!g) continue;
		  if (!TT_VALID(g->glyph)) continue;
		  
		  xmin = g->metrics.bbox.xMin & -64;
		  ymin = g->metrics.bbox.yMin & -64;
		  xmax = (g->metrics.bbox.xMax + 63) & -64;
		  ymax = (g->metrics.bbox.yMax + 63) & -64;
		  
		  xmin = (xmin >> 6) + x_offset;
		  ymin = (ymin >> 6) + y_offset;
		  xmax = (xmax >> 6) + x_offset;
		  ymax = (ymax >> 6) + y_offset;
		  
		  if (ymin < 0) off = 0;
		  else off = rows - ymin - 1;
		  adj = (rows - ymax) - 
		    ((fn->max_ascent - fn->max_descent) >> 6);
		  if ((g->texture) && (g->texture->texture))
		    {
		       __evas_gl_window_texture(glw, 1);
		       __evas_gl_window_use_texture(glw, 
						    g->texture->texture, 
						    0);
		    }
		  else
		    __evas_gl_window_texture(glw, 0);
		  glBegin(GL_QUADS);
		  glTexCoord2d(g->tex.x1, g->tex.y1);
		  glVertex2i(x + xmin,     y + ymin + off + adj);
		  glTexCoord2d(g->tex.x2, g->tex.y1);
		  glVertex2i(x + xmax + 1, y + ymin + off + adj);
		  glTexCoord2d(g->tex.x2, g->tex.y2);
		  glVertex2i(x + xmax + 1, y + ymax + off + adj + 1);
		  glTexCoord2d(g->tex.x1, g->tex.y2);
		  glVertex2i(x + xmin,     y + ymax + off + adj + 1);
		  glEnd();
		  x_offset += g->metrics.advance / 64;		  
	       }
	  }
#ifndef GLNOCLIP   
     }
#endif
}

void
__evas_gl_text_get_size(Evas_GL_Font *fn, char *text, int *w, int *h)
{
   int i, pw, ph;
   Evas_GL_Glyph *g;
   int glyph;
   
   if (!fn) return;
   if (!text) return;
   if (text[0] == 0) return;
   pw = 0;
   ph = (fn->max_ascent - fn->max_descent) / 64;
   for (i = 0; text[i]; i++)
     {
	/* for internationalization this here wouldnt just use */
	/* the char value of the text[i] but translate form utf-8 */
	/* or whetever and incriment i appropriately and set g to */
	/* the glyph index */
	glyph = ((unsigned char *)text)[i];
	g = __evas_gl_text_font_get_glyph(fn, glyph);
	if (!g) continue;
	if (!TT_VALID(g->glyph)) continue;
	if (i == 0)
	  pw += ((-g->metrics.bearingX) / 64);
	if (text[i + 1] == 0) /* last char - ineternationalization issue */
	  pw += (g->metrics.bbox.xMax / 64);
	else
	  pw += g->metrics.advance / 64;	
     }
   if (w) *w = pw + 1;
   if (h) *h = ph + 1;
}

int
__evas_gl_text_get_character_at_pos(Evas_GL_Font *fn, char *text, int x, int y, int *cx, int *cy, int *cw, int *ch)
{
   int                 i, px, ppx;
   
   if (cx) *cx = 0;
   if (cy) *cy = 0;
   if (cw) *cw = 0;
   if (ch) *ch = 0;
   if (!fn) return -1;
   if (!text) return -1;
   if (text[0] == 0) return -1;

   if ((y < 0) || (y > (fn->ascent + fn->descent))) return -1;
   if (cy) *cy = 0;
   if (ch) *ch = fn->ascent + fn->descent;
   ppx = 0;
   px = 0;
   for (i = 0; text[i]; i++)
     {
	Evas_GL_Glyph *g;
	int glyph;
	
        glyph = ((unsigned char *)text)[i];
	g = __evas_gl_text_font_get_glyph(fn, glyph);
	if (!g) continue;
	if (!TT_VALID(g->glyph)) continue;
        if (i == 0)
	  px += ((-g->metrics.bearingX) / 64);
	if (text[i + 1] == 0)
	  px += (g->metrics.bbox.xMax / 64);
	else
	  px += g->metrics.advance / 64;
	if ((x >= ppx) && (x < px))
	  {
	     if (cx)
	       *cx = ppx;
	     if (cw)
	       *cw = px - ppx;
	     return i;
	  }
     }
   return -1;   
}

void
__evas_gl_text_get_character_number(Evas_GL_Font *fn, char *text, int num, int *cx, int *cy, int *cw, int *ch)
{
   int                 i, px, ppx;
   
   if (cx) *cx = 0;
   if (cy) *cy = 0;
   if (cw) *cw = 0;
   if (ch) *ch = 0;
   if (!fn) return;
   if (!text) return;
   if (text[0] == 0) return;
   
   if (cy) *cy = 0;
   if (ch) *ch = fn->ascent + fn->descent;
   ppx = 0;
   px = 0;
   for (i = 0; text[i]; i++)
     {
	Evas_GL_Glyph *g;
	int glyph;	
	
	glyph = ((unsigned char *)text)[i];
	g = __evas_gl_text_font_get_glyph(fn, glyph);
	if (!g) continue;
	if (!TT_VALID(g->glyph)) continue;
	ppx = px;
	if (i == 0)
	  px += ((-g->metrics.bearingX) / 64);
	if (text[i + 1] == 0)
	  px += (g->metrics.bbox.xMax / 64);
	else
	  px += g->metrics.advance / 64;
	if (i == num)
	  {
	     if (cx) *cx = ppx;
	     if (cw) *cw = px - ppx;
	     return;
	  }
     }
}

/*****************************************************************************/
/* rectangle externals *******************************************************/
/*****************************************************************************/

void           __evas_gl_rectangle_draw(Display *disp, Imlib_Image dstim, Window win,
					int win_w, int win_h,
					int x, int y, int w, int h,
					int cr, int cg, int cb, int ca)
{
   Evas_GL_Window *glw;
   Evas_List l;
   int go;
   
   if (__evas_clip)
     {
	cr = (cr * __evas_clip_r) / 255;
	cg = (cg * __evas_clip_g) / 255;
	cb = (cb * __evas_clip_b) / 255;
	ca = (ca * __evas_clip_a) / 255;
     }
   if (ca <= 0) return;
   if (w < 1) return;
   if (h < 1) return;
   glw = __evas_gl_window_current(disp, win, win_w, win_h);
   if (!glw) return;
   __evas_gl_window_texture(glw, 0);
   if (ca < 255) __evas_gl_window_blend(glw, 1);
   else __evas_gl_window_blend(glw, 0);
   __evas_gl_window_write_buf(glw, GL_BACK);
   __evas_gl_window_read_buf(glw, GL_BACK);
   __evas_gl_window_color(glw, cr, cg, cb, ca);
   __evas_gl_window_dither(glw, 1);

#ifndef GLNOCLIP   
   for (l = glw->updates; l; l = l->next)
     {
	Evas_GL_Rect *rect;
	
	go = 1;
	rect = l->data;
	  {
	     int rx, ry, rw, rh;
	     
	     rx = x;
	     ry = y;
	     rw = w;
	     rh = h;
	     CLIP_TO(rx, ry, rw, rh, rect->x, rect->y, rect->w, rect->h);
	     if (__evas_clip)
	       {
		  CLIP_TO(rx, ry, rw, rh, 
			  __evas_clip_x, __evas_clip_y, 
			  __evas_clip_w, __evas_clip_h);
	       }
	     if ((rw > 0) && (rh > 0))
	       __evas_gl_window_clip(glw, 1, rx, ry, rw, rh);	     
	     else go = 0;
	  }
#else	
	go = 1;
	  {
	     int rx, ry, rw, rh;
	     
	     rx = x;
	     ry = y;
	     rw = w;
	     rh = h;
             if (__evas_clip)
	       {
		  CLIP_TO(rx, ry, rw, rh, 
			  __evas_clip_x, __evas_clip_y, 
			  __evas_clip_w, __evas_clip_h);
		  if ((rw > 0) && (rh > 0))
		    __evas_gl_window_clip(glw, 1, rx, ry, rw, rh);	     
		  else go = 0;
	       }
	     else
	       __evas_gl_window_clip(glw, 0, 0, 0, 0, 0);
	  }
#endif	
	/* render rect here */
	if (go)
	  {
	     double x1, x2, y1, y2;
	     
	     x1 = (double)x;
	     y1 = (double)y;
	     x2 = (double)(x + w);
	     y2 = (double)(y + h);
	     glBegin(GL_QUADS);
	     glVertex2d(x1, y1);
	     glVertex2d(x2, y1);
	     glVertex2d(x2, y2);
	     glVertex2d(x1, y2);
	     glEnd();
	  }
#ifndef GLNOCLIP   
     }
#endif
}

/*****************************************************************************/
/* line externals ************************************************************/
/*****************************************************************************/

void              __evas_gl_line_draw(Display *disp, Imlib_Image dstim, Window win,
				      int win_w, int win_h,
				      int x1, int y1, int x2, int y2,
				      int cr, int cg, int cb, int ca)
{
   Evas_GL_Window *glw;
   Evas_List l;
   int go;
   
   if (__evas_clip)
     {
	cr = (cr * __evas_clip_r) / 255;
	cg = (cg * __evas_clip_g) / 255;
	cb = (cb * __evas_clip_b) / 255;
	ca = (ca * __evas_clip_a) / 255;
     }
   if (ca <= 0) return;
   glw = __evas_gl_window_current(disp, win, win_w, win_h);
   if (!glw) return;
   __evas_gl_window_texture(glw, 0);
   if (ca < 255) __evas_gl_window_blend(glw, 1);
   else __evas_gl_window_blend(glw, 0);
   __evas_gl_window_write_buf(glw, GL_BACK);
   __evas_gl_window_read_buf(glw, GL_BACK);
   __evas_gl_window_color(glw, cr, cg, cb, ca);
   __evas_gl_window_dither(glw, 1);

#ifndef GLNOCLIP   
   for (l = glw->updates; l; l = l->next)
     {
	Evas_GL_Rect *rect;
	
	go = 1;
	rect = l->data;
	  {
	     int rx, ry, rw, rh;
	     
	     rx = 0;
	     ry = 0;
	     rw = win_w;
	     rh = win_h;
	     CLIP_TO(rx, ry, rw, rh, rect->x, rect->y, rect->w, rect->h);
	     if (__evas_clip)
	       {
		  CLIP_TO(rx, ry, rw, rh, 
			  __evas_clip_x, __evas_clip_y, 
			  __evas_clip_w, __evas_clip_h);
	       }
	     if ((rw > 0) && (rh > 0))
	       __evas_gl_window_clip(glw, 1, rx, ry, rw, rh);	     
	     else go = 0;
	  }
#else	
	go = 1;
	  {
	     int rx, ry, rw, rh;
	     
	     rx = 0;
	     ry = 0;
	     rw = win_w;
	     rh = win_h;
             if (__evas_clip)
	       {
		  CLIP_TO(rx, ry, rw, rh, 
			  __evas_clip_x, __evas_clip_y, 
			  __evas_clip_w, __evas_clip_h);
		  if ((rw > 0) && (rh > 0))
		    __evas_gl_window_clip(glw, 1, rx, ry, rw, rh);	     
		  else go = 0;
	       }
	     else
	       __evas_gl_window_clip(glw, 0, 0, 0, 0, 0);
	  }
#endif	
	/* render rect here */
	if (go)
	  {
	     glBegin(GL_LINES);
	     glVertex2d((double)x1 + 0.5, (double)y1 + 0.5);
	     glVertex2d((double)x2 + 0.5, (double)y2 + 0.5);
	     glEnd();
	  }
#ifndef GLNOCLIP   
     }
#endif
}

/*****************************************************************************/
/* gradient externals ********************************************************/
/*****************************************************************************/

Evas_GL_Graident *
__evas_gl_gradient_new(Display *disp)
{
   Evas_GL_Graident *gr;
   
   gr = malloc(sizeof(Evas_GL_Graident));
   gr->col_range = NULL;
   gr->textures = NULL;
}

void
__evas_gl_gradient_free(Evas_GL_Graident *gr)
{
   if (gr->col_range)
     {
	imlib_context_set_color_range(gr->col_range);
	imlib_free_color_range();
     }
   if (gr->textures);
     {
	Evas_List l;
	
	for (l = gr->textures; l; l = l->next)
	  {
	     Evas_GL_Graident_Texture *tg;
	     
	     tg = l->data;
	     __evas_gl_gradient_texture_free(tg);
	  }
	evas_list_free(gr->textures);
     }
   free(gr);
}

void
__evas_gl_gradient_color_add(Evas_GL_Graident *gr, int r, int g, int b, 
			     int a, int dist)
{
   if (!gr->col_range) gr->col_range = imlib_create_color_range();
   imlib_context_set_color_range(gr->col_range);
   imlib_context_set_color(r, g, b, a);
   imlib_add_color_to_color_range(dist);   
}

void
__evas_gl_gradient_draw(Evas_GL_Graident *gr, 
			Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h,
			int x, int y, int w, int h, double angle)
{
   Evas_GL_Window *glw;
   Evas_List l;
   int go;
   int cr, cg, cb, ca;
   Evas_GL_Graident_Texture *tg;
   
   if (__evas_clip)
     {
	cr = __evas_clip_r;
	cg = __evas_clip_g;
	cb = __evas_clip_b;
	ca = __evas_clip_a;
     }
   else
     {
	cr = 255;
	cg = 255;
	cb = 255;
	ca = 255;
     }
   if (ca <= 0) return;
   if (w < 1) return;
   if (h < 1) return;
   glw = __evas_gl_window_current(disp, win, win_w, win_h);
   if (!glw) return;
   __evas_gl_window_texture(glw, 1);
   __evas_gl_window_blend(glw, 1);
   __evas_gl_window_write_buf(glw, GL_BACK);
   __evas_gl_window_read_buf(glw, GL_BACK);
   __evas_gl_window_color(glw, cr, cg, cb, ca);
   __evas_gl_window_dither(glw, 1);

#ifndef GLNOCLIP   
   for (l = glw->updates; l; l = l->next)
     {
	Evas_GL_Rect *rect;
	
	go = 1;
	rect = l->data;
	  {
	     int rx, ry, rw, rh;
	     
	     rx = x;
	     ry = y;
	     rw = w;
	     rh = h;
	     CLIP_TO(rx, ry, rw, rh, rect->x, rect->y, rect->w, rect->h);
	     if (__evas_clip)
	       {
		  CLIP_TO(rx, ry, rw, rh, 
			  __evas_clip_x, __evas_clip_y, 
			  __evas_clip_w, __evas_clip_h);
	       }
	     if ((rw > 0) && (rh > 0))
	       __evas_gl_window_clip(glw, 1, rx, ry, rw, rh);	     
	     else go = 0;
	  }
#else	
	go = 1;
	  {
	     int rx, ry, rw, rh;
	     
	     rx = x;
	     ry = y;
	     rw = w;
	     rh = h;
             if (__evas_clip)
	       {
		  CLIP_TO(rx, ry, rw, rh, 
			  __evas_clip_x, __evas_clip_y, 
			  __evas_clip_w, __evas_clip_h);
		  if ((rw > 0) && (rh > 0))
		    __evas_gl_window_clip(glw, 1, rx, ry, rw, rh);	     
		  else go = 0;
	       }
	     else
	       __evas_gl_window_clip(glw, 0, 0, 0, 0, 0);
	  }
#endif	
	/* render gradient here */
	if (go)
	  {
	     double x1, x2, y1, y2;
	     double max, t[8];
	     int i;
	     
	     tg = __evas_gl_gradient_texture_new(glw, gr);
	     if (!tg->texture)
	       {
		  Imlib_Image prev_im, im;
		  DATA32 *data;
		  int tw, th;
		  
		  tw = glw->context->max_texture_size;
		  th = 4;
		  im = imlib_create_image(tw, th);
		  prev_im = imlib_context_get_image();
		  imlib_context_set_image(im);
		  imlib_image_set_has_alpha(1);
		  data = imlib_image_get_data();
		  memset(data, 0, tw * th * sizeof(DATA32)); 
		  imlib_image_put_back_data(data);
		  imlib_context_set_color_range(gr->col_range);
		  imlib_image_fill_color_range_rectangle(1, 1, tw - 2, th - 2, 
							 270.0);
		  data = imlib_image_get_data();
		  memcpy(&(data[(1) + (0 * tw)]), &(data[(1) + (1 * tw)]),
			 (tw - 2) * sizeof(DATA32));
		  memcpy(&(data[(1) + (3 * tw)]), &(data[(1) + (2 * tw)]),
			 (tw - 2) * sizeof(DATA32));
		  data[(0) + (0 * tw)] = data[(1) + (0 * tw)];
		  data[(0) + (1 * tw)] = data[(1) + (1 * tw)];
		  data[(0) + (2 * tw)] = data[(1) + (2 * tw)];
		  data[(0) + (3 * tw)] = data[(1) + (3 * tw)];
		  data[(tw - 1) + (0 * tw)] = data[(tw - 2) + (0 * tw)];
		  data[(tw - 1) + (1 * tw)] = data[(tw - 2) + (1 * tw)];
		  data[(tw - 1) + (2 * tw)] = data[(tw - 2) + (2 * tw)];
		  data[(tw - 1) + (3 * tw)] = data[(tw - 2) + (3 * tw)];
		  imlib_image_put_back_data(data);
		  tg->texture = __evas_gl_texture_new(glw, im, 0, 0, tw, th);
		  imlib_free_image();
	       }
	     __evas_gl_window_use_texture(glw, tg->texture, 1);
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
		  t[i] = (1.0 +
			  ((((0.5) + (t[i] / 2.0)) * 
			    ((double)glw->context->max_texture_size - 2.0)))) / 
			  (double)glw->context->max_texture_size;
		  t[i + 1] = (1.0 + ((((0.5) - (t[i + 1] / 2.0))) * 2.0)) / 4.0;
	       }	     
	     x1 = (double)x;
	     y1 = (double)y;
	     x2 = (double)(x + w);
	     y2 = (double)(y + h);
	     glBegin(GL_QUADS);
	     glTexCoord2d(t[0],  t[1]); glVertex2d(x1, y1);
	     glTexCoord2d(t[2],  t[3]); glVertex2d(x2, y1);
	     glTexCoord2d(t[4],  t[5]); glVertex2d(x2, y2);
	     glTexCoord2d(t[6],  t[7]); glVertex2d(x1, y2);
	     glEnd();
	  }
#ifndef GLNOCLIP   
     }
#endif
}

/************/
/* polygons */
/************/
void
__evas_gl_poly_draw (Display *disp, Imlib_Image dstim, Window win, 
		     int win_w, int win_h, 
		     Evas_List points, 
		     int cr, int cg, int cb, int ca)
{
   Evas_GL_Window *glw;
   Evas_List l;
   int go;
   
   if (__evas_clip)
     {
	cr = (cr * __evas_clip_r) / 255;
	cg = (cg * __evas_clip_g) / 255;
	cb = (cb * __evas_clip_b) / 255;
	ca = (ca * __evas_clip_a) / 255;
     }
   if (ca <= 0) return;
   glw = __evas_gl_window_current(disp, win, win_w, win_h);
   if (!glw) return;
   __evas_gl_window_texture(glw, 0);
   if (ca < 255) __evas_gl_window_blend(glw, 1);
   else __evas_gl_window_blend(glw, 0);
   __evas_gl_window_write_buf(glw, GL_BACK);
   __evas_gl_window_read_buf(glw, GL_BACK);
   __evas_gl_window_color(glw, cr, cg, cb, ca);
   __evas_gl_window_dither(glw, 1);

#ifndef GLNOCLIP   
   for (l = glw->updates; l; l = l->next)
     {
	Evas_GL_Rect *rect;
	
	go = 1;
	rect = l->data;
	  {
	     int rx, ry, rw, rh;
	     
	     rx = 0;
	     ry = 0;
	     rw = win_w;
	     rh = win_h;
	     CLIP_TO(rx, ry, rw, rh, rect->x, rect->y, rect->w, rect->h);
	     if (__evas_clip)
	       {
		  CLIP_TO(rx, ry, rw, rh, 
			  __evas_clip_x, __evas_clip_y, 
			  __evas_clip_w, __evas_clip_h);
	       }
	     if ((rw > 0) && (rh > 0))
	       __evas_gl_window_clip(glw, 1, rx, ry, rw, rh);	     
	     else go = 0;
	  }
#else	
	go = 1;
	  {
	     int rx, ry, rw, rh;
	     
	     rx = 0;
	     ry = 0;
	     rw = win_w;
	     rh = win_h;
             if (__evas_clip)
	       {
		  CLIP_TO(rx, ry, rw, rh, 
			  __evas_clip_x, __evas_clip_y, 
			  __evas_clip_w, __evas_clip_h);
		  if ((rw > 0) && (rh > 0))
		    __evas_gl_window_clip(glw, 1, rx, ry, rw, rh);	     
		  else go = 0;
	       }
	     else
	       __evas_gl_window_clip(glw, 0, 0, 0, 0, 0);
	  }
#endif	
	/* render poly here */
	if (go)
	  {
	     Evas_List l;
	     
	     glBegin(GL_POLYGON);
	     for (l = points; l; l = l->next)
	       {
		  Evas_Point p;
		  
		  p = l->data;
		  glVertex2d(p->x, p->y);
	       }
	     glEnd();
	  }
#ifndef GLNOCLIP   
     }
#endif
}

/*****************************************************************************/
/* general externals *********************************************************/
/*****************************************************************************/

void
__evas_gl_set_clip_rect(int on, int x, int y, int w, int h, int r, int g, int b, int a)
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
__evas_gl_sync(Display *disp)
{
   glFlush();
   glXWaitGL();
   XSync(disp, False);
}

void
__evas_gl_flush_draw(Display *disp, Imlib_Image dstim, Window win)
{
   Evas_GL_Window *glw;
   Evas_List l;

   glw = __evas_gl_window_lookup(disp, win);
   if (!glw) return;
   if (glw->updates)
     {
	for (l = glw->updates; l; l = l->next)
	  {
	     Evas_GL_Rect *rect;
	     
	     rect = l->data;
#ifndef GLSWB	     
	     __evas_gl_window_swap_rect(glw, rect->x, rect->y, rect->w, rect->h);
#endif	     
	     free(rect);
	  }
	evas_list_free(glw->updates);
	glw->updates = NULL;	
     }
#ifdef GLSWB   
   glXSwapBuffers(disp, win);
#endif   
}

int
__evas_gl_capable(Display *disp)
{
   int eb, evb;
   Evas_List l;
   static Evas_List capables = NULL;

   for (l = capables; l; l = l->next)
     {
	if (l->data == disp) return 1;
     }
    if (glXQueryExtension(disp, &eb, &evb))
     {
	capables = evas_list_prepend(capables, disp);
	return 1;
     }
   return 0;
}

static XVisualInfo *__evas_vi = NULL;

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
}

void
__evas_gl_draw_add_rect(Display *disp, Imlib_Image dstim, Window win,
			int x, int y, int w, int h)
{
   Evas_GL_Window *glw;
   Evas_GL_Rect *rect;
   
   glw = __evas_gl_window_lookup(disp, win);
   if (!glw) return;
   rect = malloc(sizeof(Evas_GL_Rect));
   rect->x = x;
   rect->y = y;
   rect->w = w;
   rect->h = h;
   glw->updates = evas_list_append(glw->updates, rect);
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

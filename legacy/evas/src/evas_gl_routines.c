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

void
__evas_gl_copy_image_rect_to_texture(Evas_GL_Image *im, int x, int y, 
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

void
__evas_gl_move_state_data_to_texture(Evas_GL_Image *im)
{
   int i, x, y;
   XSetWindowAttributes att;
   
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

   att.colormap = im->buffer.colormap;
   att.border_pixel = 0;
   att.event_mask = 0;
   im->buffer.window = XCreateWindow(im->buffer.display,
				     RootWindow(im->buffer.display, DefaultScreen(im->buffer.display)),
				     0, 0, 32, 32, 0, 
				     im->buffer.visual_info->depth,
				     InputOutput, 
				     im->buffer.visual_info->visual,
				     CWColormap | CWBorderPixel | CWEventMask,
				     &att);
   im->texture.textures = malloc(sizeof(GLuint) * im->texture.w * im->texture.h);
   glXMakeCurrent(im->buffer.display, im->buffer.window, im->context);
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
	     
	     __evas_gl_copy_image_rect_to_texture(im, xx, yy, ww, hh, tw, th, 
						  im->texture.textures[i]);
	  }
     }
   /* done - set the actual image state to textured */
   im->state = EVAS_STATE_TEXTURE;
}

void __evas_calc_tex_and_poly(Evas_GL_Image *im, int x, double *x1, double *x2,
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

void __evas_gl_render_to_window(Evas_GL_Image *im, 
				Display *disp, Window w, int win_w, int win_h,
				int src_x, int src_y, int src_w, int src_h,
				int dst_x, int dst_y, int dst_w, int dst_h)
{
   int x, y, i;   
   double dx, dy, dw, dh;
   
   if (im->state != EVAS_STATE_TEXTURE)
      __evas_gl_move_state_data_to_texture(im);
   glXMakeCurrent(disp, w, im->context);
/*   
   glClearColor(0.7, 0.7, 0.7, 1.0);
   glClear(GL_COLOR_BUFFER_BIT);
*/   
   glEnable(GL_BLEND);
   glShadeModel(GL_FLAT);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
   glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
/* why doesnt scissor work ? */
/*   
   glEnable(GL_SCISSOR_TEST);
   glScissor(dst_x, win_h - dst_y - 1, dst_w, dst_h); 
*/
/* translate all poly coords in ints to normal X coord space */
   glViewport(0, 0, win_w, win_h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, win_w, 0, win_h, -1, 1);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glScalef(1, -1, 1);
   glTranslatef(0, -win_h, 0);   
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
	     
	     
	     __evas_calc_tex_and_poly(im, x, &x1, &x2, &tx, &txx, 
				      &dtx, &dtxx, 
				      im->texture.w, im->w, 
				      im->texture.edge_w);
	     __evas_calc_tex_and_poly(im, y, &y1, &y2, &ty, &tyy, 
				      &dty, &dtyy, 
				      im->texture.h, im->h, 
				      im->texture.edge_h);
             x1 = dx + ((x1 * dw) / (double)im->w);
             y1 = dy + ((y1 * dh) / (double)im->h);
             x2 = dx + ((x2 * dw) / (double)im->w);
             y2 = dy + ((y2 * dh) / (double)im->h);
	     glBindTexture(GL_TEXTURE_2D, im->texture.textures[i]);
	     glBegin(GL_POLYGON);
	     glTexCoord2d(dtx,  dty);  glVertex2d(x1, y1);
	     glTexCoord2d(dtxx, dty);  glVertex2d(x2, y1);
	     glTexCoord2d(dtxx, dtyy); glVertex2d(x2, y2);
	     glTexCoord2d(dtx,  dtyy); glVertex2d(x1, y2);
	     glEnd();
	  }
     }
   glXSwapBuffers(im->buffer.display, w);
   glDisable(GL_SCISSOR_TEST);
}

Evas_GL_Image *
__evas_gl_create_image(void)
{
   Evas_GL_Image *im;
   
   im = malloc(sizeof(Evas_GL_Image));
   memset(im, 0, sizeof(Evas_GL_Image));
   return im;
}

static XVisualInfo *__evas_vi = NULL;
static GLXContext __evas_gl_cx = 0;

int
__evas_gl_capable(Display *disp)
{
   int eb, evb;
   return glXQueryExtension(disp, &eb, &evb);
}

Visual *
__evas_gl_get_visual(Display *disp)
{
   static Display *d = NULL;

   if (d != disp)
     {
	d = disp;
	__evas_vi = glXChooseVisual(disp, DefaultScreen(disp), __evas_gl_configuration);
     }
   return __evas_vi->visual;
}

Colormap
__evas_gl_get_colormap(Display *disp)
{
   static Display *d = NULL;
   static Colormap cmap = 0;

   if (!__evas_vi) __evas_gl_get_visual(disp);
   if (d != disp)
     {
	d = disp;
	cmap = XCreateColormap(disp, RootWindow(disp, DefaultScreen(disp)), __evas_vi->visual, 0);
     }
   return cmap;
}

void
__evas_gl_init(Display *disp)
{
   if (__evas_gl_cx) return;
   
   if (!__evas_gl_capable(disp)) return;
   __evas_gl_get_visual(disp);
   /* direct rendering client */
   __evas_gl_cx = glXCreateContext(disp, __evas_vi, NULL, GL_TRUE);
   /* GLX indirect */
/*   __evas_gl_cx = glXCreateContext(disp, __evas_vi, NULL, GL_TRUE);*/
}

Evas_GL_Image *
__evas_gl_image_new_from_file(Display *disp, char *file)
{
   Evas_GL_Image *im;
   Imlib_Image i;
   
   i = imlib_load_image(file);
   if (!i) return NULL;
   imlib_context_set_image(i);
   im = __evas_gl_create_image();
   im->w = imlib_image_get_width();
   im->h = imlib_image_get_height();
   im->data = imlib_image_get_data();
   im->texture.max_size = 256;
   __evas_gl_init(disp);
   im->context = __evas_gl_cx;
   im->buffer.display = disp;
   im->buffer.colormap = __evas_gl_get_colormap(disp);
   im->buffer.visual_info = __evas_vi;
   return im;
}

void
__evas_sync(Display *disp)
{
   glXWaitGL();
   XSync(disp, False);
}

void
__evas_flush_draw(Display *disp, Window win)
{
   glXSwapBuffers(disp, win);
}

void
__evas_draw_rectangle(Display *disp, Window win, int x, int y, int w, int h,
		      int r, int g, int b, int a)
{
}

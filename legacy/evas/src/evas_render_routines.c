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

static char              x_does_shm = -1;
static int               list_num = 0;
static XImage          **list_xim = NULL;
static XShmSegmentInfo **list_si = NULL;
static Display         **list_d = NULL;
static char             *list_used = NULL;
static int               list_mem_use = 0;
static int               list_max_mem = 0;
static int               list_max_count = 0;
static char              _x_err = 0;

static Visual *__evas_visual = NULL;
static Colormap __evas_cmap = 0;

/* the fucntion we use for catching the error */
static void
__evas_render_image_ximage_tmp_x_error(Display * d, XErrorEvent * ev)
{
      _x_err = 1;
      return;
      d = NULL;
      ev = NULL;
}

static void
__evas_render_image_ximage_flush(Display *d)
{
   int i;
   XImage *xim;
   char did_free = 1;
   
   while (((list_mem_use > list_max_mem) || (list_num > list_max_count)) &&
	  (did_free))
     {
	did_free = 0;
	for (i = 0; i < list_num; i++)
	  {
	     if (list_used[i] == 0)
	       {
		  int j;
		  
		  xim = list_xim[i];
		  list_mem_use -= xim->bytes_per_line * xim->height;
		  if (list_si[i])
		    XShmDetach(d, list_si[i]);
		  XDestroyImage(xim);
		  if (list_si[i])
		    {
		       shmdt(list_si[i]->shmaddr);
		       shmctl(list_si[i]->shmid, IPC_RMID, 0);
		       free(list_si[i]);
		    }
		  list_num--;
		  for (j = i; j < list_num; j++)
		    {
		       list_xim[j] = list_xim[j + 1];
		       list_si[j] = list_si[j + 1];
		       list_used[j] = list_used[j + 1];
		       list_d[j] = list_d[j + 1];
		    }
		  if (list_num == 0)
		    {
		       if (list_xim) free(list_xim);
		       if (list_si) free(list_si);
		       if (list_used) free(list_used);
		       if (list_d) free(list_d);
		       list_xim = NULL;
		       list_si = NULL;
		       list_used = NULL;
		       list_d = NULL;
		    }
		  else
		    {
		       list_xim = realloc(list_xim, sizeof(XImage *) * list_num);
		       list_si = realloc(list_si, sizeof(XShmSegmentInfo *) * list_num);
		       list_used = realloc(list_used, sizeof(char) * list_num);
		       list_d = realloc(list_d, sizeof(Display *) * list_num);
		    }
		  did_free = 1;
	       }
	  }
     }
}

XImage *
__evas_render_image_ximage_new(Display *d, Visual *v, int depth, int w, int h, char *shared)
{
   XImage                  *xim;
   int                      i;
   
   if (x_does_shm < 0)
     {
	if (XShmQueryExtension(d))
	  x_does_shm = 1;
	else
	  x_does_shm = 0;
     }
   *shared = 0;
   for (i = 0; i < list_num; i++)
     {
	if ((list_xim[i]->bits_per_pixel == depth) &&
	    (list_xim[i]->width >= w) &&
	    (list_xim[i]->height >= h) &&
	    (!list_used[i]))
	  {
	     list_used[i] = 1;
	     if (list_si[i])
	       *shared = 1;
	     return list_xim[i];
	  }
     }
   list_num++;
   if (list_num == 1)
     {
	list_xim  = malloc(sizeof(XImage *)          * list_num);
	list_si   = malloc(sizeof(XShmSegmentInfo *) * list_num);
	list_used = malloc(sizeof(char)              * list_num);
	list_d    = malloc(sizeof(Display *)         * list_num);
     }
   else
     {
	list_xim  = realloc(list_xim,  sizeof(XImage *)          * list_num);
	list_si   = realloc(list_si,   sizeof(XShmSegmentInfo *) * list_num);
	list_used = realloc(list_used, sizeof(char)              * list_num);
	list_d    = realloc(list_d,    sizeof(Display *)         * list_num);
     }
   list_si[list_num - 1] = malloc(sizeof(XShmSegmentInfo));
   
   xim = NULL;
   if (x_does_shm)
     {
	xim = XShmCreateImage(d, v, depth, ZPixmap, NULL,
			      list_si[list_num - 1], w, h);
	if (xim)
	  {
	     list_xim[list_num - 1] = xim;
	     list_si[list_num - 1]->shmid =
	       shmget(IPC_PRIVATE, xim->bytes_per_line * xim->height,
		      IPC_CREAT | 0777);
	     if (list_si[list_num - 1]->shmid != -1)
	       {
		  list_si[list_num - 1]->readOnly = False;
		  list_si[list_num - 1]->shmaddr = xim->data =
		    shmat(list_si[list_num - 1]->shmid, 0, 0);
		  if (xim->data != (char *)-1)
		    {
		       XErrorHandler ph;
		       
		       _x_err = 0;
		       ph = XSetErrorHandler((XErrorHandler) __evas_render_image_ximage_tmp_x_error);
		       XShmAttach(d, list_si[list_num - 1]);
		       XSync(d, False);
		       XSetErrorHandler((XErrorHandler) ph);
		       if (!_x_err)
			 {
			    list_used[list_num - 1] = 1;
			    list_mem_use += xim->bytes_per_line * xim->height;
			    *shared = 1;
			 }
		       else
			 {
			    x_does_shm = 0;
			    XDestroyImage(xim);
			    shmdt(list_si[list_num - 1]->shmaddr);
			    shmctl(list_si[list_num - 1]->shmid, IPC_RMID, 0);
			    xim = NULL;
			 }
		    }
		  else
		    {
		       XDestroyImage(xim);
		       shmctl(list_si[list_num - 1]->shmid, IPC_RMID, 0);
		       xim = NULL;
		    }
	       }
	     else
	       {
		  XDestroyImage(xim);
		  xim = NULL;
	       }
	  }
     }
   if (!xim)
     {
	free(list_si[list_num - 1]);
	list_si[list_num - 1] = NULL;
	xim = XCreateImage(d, v, depth, ZPixmap, 0, NULL, w, h, 32, 0);
	xim->data = malloc(xim->bytes_per_line * xim->height);
	list_xim[list_num - 1] = xim;
	list_mem_use += xim->bytes_per_line * xim->height;
	list_used[list_num - 1] = 1;
	list_d[list_num - 1] = d;
     }
   __evas_render_image_ximage_flush(d);
   return xim;
}


static void
__evas_render_image_ximage_free(Display *d, XImage *xim)
{
   int i;
   
   for (i = 0; i < list_num; i++)
     {
	if (list_xim[i] == xim)
	  {
	     list_used[i] = 0;
	     __evas_render_image_ximage_flush(d);
	     return;
	  }
     }
}

static void
__evas_render_image_ximage_put(Display *d, Drawable dst, GC gc, XImage *xim, 
			       int sx, int sy, int dx, int dy, int w, int h,
			       int shm)
{
   if (shm)
     XShmPutImage(d, dst, gc, xim, sx, sy, dx, dy, w, h, False);
   else
     XPutImage(d, dst, gc, xim, sx, sy, dx, dy, w, h);   
   if (shm)
     XSync(d, False);   
}


static Picture
__evas_render_create_picuture(Display *d, Drawable dst, 
			      XRenderPictFormat *format)
{
   Picture pic;
   XRenderPictureAttributes att;
   
   att.repeat = True;
   att.dither = True;
   att.component_alpha = True;
   pic = XRenderCreatePicture (d, dst, format,
/*
 CPRepeat |
 CPDither |
 CPComponentAlpha,
 */
			       0,
			       &att);
   return pic;
}

static void
__evas_render_populate_update(Evas_Render_Update *up)
{
   XRenderPictFormat fmt, *format, *format_color;
   Visual *visual;
   int screen;

   screen = 0;
   visual = __evas_visual;
   format = XRenderFindVisualFormat (up->disp, visual);
   if (!format) printf("eek no format!\n");
   fmt.depth = 32;
   fmt.type = PictTypeDirect;
   format_color = XRenderFindFormat(up->disp, PictFormatType | PictFormatDepth,
				    &fmt, 0);
   if (!format_color) printf("eek no format_color\n");
   up->pmap = XCreatePixmap (up->disp, up->drawable,
			     up->w, up->h, fmt.depth);
   up->pic = __evas_render_create_picuture(up->disp, up->pmap, format_color);
}

static void
__evas_render_dump_current_image_to_pixmap(Display *disp, Pixmap pmap, 
					   Visual *v, int depth, int w, int h)
{
   DATA32 *data, *buf;
   int x, y;
   XImage *xim;
   char shm;
   static GC gc = 0;
   XGCValues gcv;
   int has_alpha;
   
   data = imlib_image_get_data_for_reading_only();
   shm = 0;
   xim = __evas_render_image_ximage_new(disp, v, depth, w, h, &shm);
   if (!xim) return;
   if (!gc)
     {
	gcv.graphics_exposures = False;
	gc = XCreateGC(disp, pmap, GCGraphicsExposures, &gcv);
     }
   has_alpha = imlib_image_has_alpha();
   if (has_alpha)
     {
	for (y = 0; y < h; y++)
	  {	
	     buf = xim->data + (y * (xim->bytes_per_line));
	     for (x = 0; x < w; x++)
	       {
		  int r, g, b, a;
		  
		  r = ((*data) >> 16) & 0xff;
		  g = ((*data) >> 8 ) & 0xff;
		  b = ((*data)      ) & 0xff;
		  a = ((*data) >> 24) & 0xff;
		  r = (r * a) / 255;
		  g = (g * a) / 255;
		  b = (b * a) / 255;
		  
		  *buf = (a << 24) | (r << 16) | (g << 8) | b;
		  buf++;
		  data++;
	       }
	  }
     }
   else
     {
	for (y = 0; y < h; y++)
	  {	
	     buf = xim->data + (y * (xim->bytes_per_line));
	     for (x = 0; x < w; x++)
	       {
		  int r, g, b, a;
		  
		  r = ((*data) >> 16) & 0xff;
		  g = ((*data) >> 8 ) & 0xff;
		  b = ((*data)      ) & 0xff;
		  a = 255;
		  *buf = (a << 24) | (r << 16) | (g << 8) | b;
		  buf++;
		  data++;
	       }
	  }
     }
   __evas_render_image_ximage_put(disp, pmap, gc, xim, 0, 0, 0, 0, w, h, shm);
   __evas_render_image_ximage_free(disp, xim);
   imlib_image_put_back_data(data);
}

static int
__evas_render_create_dest_buf(Display *disp, Visual *v, int w, int h,
			      int screen, Pixmap *pmap_ret, Picture *pic_ret)
{
   XRenderPictFormat fmt, *format, *format_color;
   
   format = XRenderFindVisualFormat(disp, v);
   if (!format) printf("eek no format!\n");
   fmt.depth = 32;
   fmt.type = PictTypeDirect;
   format_color = XRenderFindFormat(disp, PictFormatType | PictFormatDepth,
				    &fmt, 0);
   if (!format_color) printf("eek no format_color\n");
   *pmap_ret = XCreatePixmap(disp, RootWindow(disp, screen), w, h, fmt.depth);
   *pic_ret = __evas_render_create_picuture(disp, *pmap_ret, format_color);   
   return format_color->depth;
}

/*****************************************************************************/
/* image externals ***********************************************************/
/*****************************************************************************/

Evas_Render_Image *
__evas_render_image_new_from_file(Display *disp, char *file)
{
   Evas_Render_Image *im;
   Imlib_Image i;
   int screen, depth;

   /* need to look for image i local cache */
   /* not found - load */
   screen = 0;
   i = imlib_load_image(file);
   if (!i) return NULL;
   imlib_context_set_image(i);
   im = malloc(sizeof(Evas_Render_Image));
   memset(im, 0, sizeof(Evas_Render_Image));
   im->image = i;
   im->file = malloc(strlen(file) + 1);
   strcpy(im->file, file);
   im->references = 1;
   im->disp = disp;
   im->has_alpha = imlib_image_has_alpha();
   im->w = imlib_image_get_width();
   im->h = imlib_image_get_height();
/*   
   depth = __evas_render_create_dest_buf(disp, __evas_visual, im->w, im->h, 
					 screen, &(im->pmap), &(im->pic));
   __evas_render_dump_current_image_to_pixmap(disp, im->pmap, __evas_visual, 
					      depth, im->w, im->h);
*/
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
   if ((src_w == 0) || (src_h == 0) || (dst_w == 0) || (dst_w == 0)) return;
   
   if (__evas_clip)
     {
	cr = (cr * __evas_clip_r) / 255;
	cg = (cg * __evas_clip_g) / 255;
	cb = (cb * __evas_clip_b) / 255;
	ca = (ca * __evas_clip_a) / 255;
     }
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
                       int xx, yy, ww, hh, iw, ih, dx, dy, dw, dh;
		       
		       if (!up->pmap)
			 __evas_render_populate_update(up);
		       dx = dst_x;
		       dy = dst_y;
		       dw = dst_w;
		       dh = dst_h;
		       iw = im->w;
		       ih = im->h;
		       ww = (iw * dw) / src_w;
		       hh = (ih * dh) / src_h;
		       xx = (src_x * dw) / src_w;
		       yy = (src_y * dh) / src_h;
		       if (__evas_clip)
			 {
			    int px, py;
			    
			    px = dx;
			    py = dy;
			    CLIP_TO(dx, dy, dw, dh,
				    __evas_clip_x, __evas_clip_y,
				    __evas_clip_w, __evas_clip_h);
			    xx += dx - px;
			    yy += dy - py;
			 }
		       if ((dw > 1) && (dh > 1))
			 {
			    if ((im->current.w != ww) ||
				(im->current.h != hh) ||
				(cr != im->current.pr) || 
				(cg != im->current.pg) ||
				(cb != im->current.pb) || 
				(ca != im->current.pa) ||
				(__evas_anti_alias != im->current.smooth)
			      )
			      {
				 if (im->pmap) XFreePixmap(im->disp, im->pmap);
				 if (im->pic) XRenderFreePicture(im->disp, im->pic);
				 im->pmap = 0;
				 im->pic = 0;
			      }
			    if (!im->pmap)
			      {
				 int screen = 0;
				 int depth;
				 
				 im->current.w = ww;
				 im->current.h = hh;
				 im->current.pr = cr;
				 im->current.pg = cg;
				 im->current.pb = cb;
				 im->current.pa = ca;
				 im->current.smooth = __evas_anti_alias;
				 if ((im->current.border.l != 0) ||
				     (im->current.border.r != 0) ||
				     (im->current.border.t != 0) ||
				     (im->current.border.b != 0) ||
				     (im->current.pr != 255) ||	
				     (im->current.pg != 255) ||
				     (im->current.pb != 255) ||
				     (im->current.pa != 255)
				     )
				   {
				      Imlib_Image i;
				      Imlib_Border bd;
				      Imlib_Color_Modifier cm = NULL;
				      
				      cm = imlib_create_color_modifier();
				      imlib_context_set_color_modifier(cm);
				      if ((cr != 255) || (cg != 255) || (cb != 255) || (ca != 255))
					{
					   DATA8 r[256], g[256], b[256], a[256];
					   int j;
					   
					   cm = imlib_create_color_modifier();
					   imlib_context_set_color_modifier(cm);
					   for (j = 0; j < 256; j++)
					     {
						r[j] = (j * cr) / 255;
						g[j] = (j * cg) / 255;
						b[j] = (j * cb) / 255;
						a[j] = (j * ca) / 255;
					     }
					   imlib_set_color_modifier_tables(r, g, b, a);
					}
				      else
					imlib_context_set_color_modifier(NULL);
				      
				      i = imlib_create_image(im->current.w, im->current.h);
				      imlib_context_set_image(i);
				      imlib_image_set_has_alpha(1);
				      imlib_image_clear();
				      bd.left = im->current.border.l;
				      bd.right = im->current.border.r;
				      bd.top = im->current.border.t;
				      bd.bottom = im->current.border.b;
				      imlib_image_set_border(&bd);
				      imlib_context_set_anti_alias(__evas_anti_alias);
				      imlib_context_set_blend(1);
				      imlib_context_set_operation(IMLIB_OP_COPY);
				      imlib_context_set_angle(0.0);
				      
				      imlib_blend_image_onto_image(im->image, 1,
								   0, 0, im->w, im->h,
								   0, 0, dst_w, dst_h);
				      if (cm)
					{
					   imlib_free_color_modifier();
					   imlib_context_set_color_modifier(NULL);
					}
				   }
				 else
				   {
				      Imlib_Image i;
				      
				      imlib_context_set_image(im->image);
				      i = imlib_create_cropped_scaled_image(0, 0, 
									    im->w, im->h,
									    dst_w, dst_h);				      
				      imlib_context_set_image(i);
				   }
				 depth = __evas_render_create_dest_buf(disp, 
								       __evas_visual, 
								       im->current.w, 
								       im->current.h,
								       screen, 
								       &(im->pmap), 
								       &(im->pic));
			         __evas_render_dump_current_image_to_pixmap(disp, 
									    im->pmap, 
									    __evas_visual,
									    depth, 
									    im->current.w, 
									    im->current.h);
				 imlib_free_image();
			      }
/*			    if (im->has_alpha)*/
			      XRenderComposite (disp,
						PictOpOver,
						im->pic,
						None,
						up->pic,
						xx, yy, 0, 0, 
						dx - up->x, dy - up->y,
						dw, dh);
/* WOW... PictOpSrc is SLOOOOOOOOW
 else
			      XRenderComposite (disp,
						PictOpSrc,
						im->pic,
						None,
						up->pic,
						xx, yy, 0, 0, 
						dx - up->x, dy - up->y,
						dw, dh);
*/			 }
		    }
	       }
	  }
     }
}

int
__evas_render_image_get_width(Evas_Render_Image *im)
{
   return im->w;
}

int
__evas_render_image_get_height(Evas_Render_Image *im)
{
   return im->h;
}

void
__evas_render_image_set_borders(Evas_Render_Image *im, int left, int right,
			       int top, int bottom)
{
   if ((left == im->current.border.l) &&
       (right == im->current.border.r) &&
       (top == im->current.border.t) &&
       (bottom == im->current.border.b)) return;
   if (im->pmap) XFreePixmap(im->disp, im->pmap);
   if (im->pic) XRenderFreePicture(im->disp, im->pic);
   im->pmap = 0;
   im->pic = 0;
   im->current.border.l = left;
   im->current.border.r = right;
   im->current.border.t = top;
   im->current.border.b = bottom;
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
	XRenderPictFormat   *format, *format_color;
	Picture     picture;
	
	dr = l->data;
	format = XRenderFindVisualFormat (disp, __evas_visual);
	if (!format) printf("eek no format!\n");
	picture = __evas_render_create_picuture(disp, win, format);
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_Render_Update *up;
		  
		  up = ll->data;
		  if (up->pmap)
		    {
/* WOW. PictOpSrc is slooow!
		       XRenderComposite(disp, PictOpSrc,
					up->pic, None, picture,
					0, 0, 0, 0, up->x, up->y, 
					up->w, up->h);		       
*/		       XRenderComposite(disp, PictOpOver,
					up->pic, None, picture,
					0, 0, 0, 0, up->x, up->y, 
					up->w, up->h);		       
		       XRenderFreePicture(disp, up->pic);
		       XFreePixmap(disp, up->pmap);		       
		    }
		  free(up);
	       }
	     if (dr->tmp_images)
		dr->tmp_images = evas_list_free(dr->tmp_images);
	  }
	XRenderFreePicture(disp, picture);
	free(dr);
     }
   __evas_render_sync(disp);
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
	     up->drawable = win;
	     up->disp = disp;
	     up->pmap = 0;
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
	up->drawable = win;
	up->disp = disp;
	up->pmap = 0;
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

#include "evas_x11_routines.h"
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>

#define CLIP(x, y, w, h, xx, yy, ww, hh) \
if (x < (xx)) {w += (x - (xx)); x = (xx);} \
if (y < (yy)) {h += (y - (yy)); y = (yy);} \
if ((x + w) > ((xx) + (ww))) {w = (ww) - (x - xx);} \
if ((y + h) > ((yy) + (hh))) {h = (hh) - (y - yy);}

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


static void __evas_x11_image_cache_flush(Display *disp);
static int  __evas_anti_alias = 1;
static Evas_List drawable_list = NULL;

static Visual *__evas_visual = NULL;
static Colormap __evas_cmap = 0;

static Evas_List __evas_images          = NULL;
static int       __evas_image_cache     = 0;
static int       __evas_image_cache_max = 512 * 1024;

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
__evas_x11_image_cache_flush(Display *disp)
{
   Evas_X11_Image *im, *im_last;
   Evas_List l;
   
   while (__evas_image_cache > __evas_image_cache_max)
     {
	for (l = __evas_images; l; l = l->next)
	  {
	     im = l->data;
	     
	     if (im->references == 0) im_last = im;
	  }
	if (im_last)
	  {
	     im = im_last;
	     
	     __evas_images = evas_list_remove(__evas_images, im);
	     if (im->pmap) imlib_free_pixmap_and_mask(im->pmap);
	     if (im->gc) XFreeGC(im->disp, im->gc);
	     if (im->file) free(im->file);
	     if (im->image) 
	       {
		  imlib_context_set_image(im->image);		  
		  imlib_free_image_and_decache();
	       }
	     __evas_image_cache -= im->pw * im->ph * 4;
	     free(im);
	  }
     }
}

/*****************************************************************************/
/* image externals ***********************************************************/
/*****************************************************************************/

Evas_X11_Image *
__evas_x11_image_new_from_file(Display *disp, char *file)
{
   Evas_X11_Image *im;
   Evas_List l;
   
   for (l = __evas_images; l; l = l->next)
     {
	im = l->data;
	if ((!strcmp(im->file, file)) && (im->disp == disp))
	  {
	     if (l != __evas_images)
	       {
		  __evas_images = evas_list_remove(__evas_images, im);
		  __evas_images = evas_list_prepend(__evas_images, im);
	       }
	     if (im->references == 0)
	       {
		  __evas_image_cache -= im->pw * im->ph * 4;		  
	       }
	     im->references++;
	     return im;
	  }
     }
   im = malloc(sizeof(Evas_X11_Image));
   memset(im, 0, sizeof(Evas_X11_Image));
   im->image = imlib_load_image(file);
   if (!im->image) 
     {
	free(im);
	return NULL;
     }
   im->disp = disp;
   im->file = strdup(file);
   im->references = 1;
   imlib_context_set_image(im->image);
   im->w = imlib_image_get_width();
   im->h = imlib_image_get_height();
   im->has_alpha =  imlib_image_has_alpha();
   __evas_images = evas_list_prepend(__evas_images, im);
   return im;
}

void
__evas_x11_image_free(Evas_X11_Image *im)
{
   im->references--;
   if (im->references == 0)
     {
	/* flush */
	__evas_image_cache += im->pw * im->ph * 4;
	__evas_x11_image_cache_flush(im->disp);
     }
}

void
__evas_x11_image_cache_empty(Display *disp)
{
   int size;
   
   size = imlib_get_cache_size();
   imlib_set_cache_size(0);
   imlib_set_cache_size(size);
   size = __evas_image_cache_max;
   __evas_image_cache_max = 0;
   __evas_x11_image_cache_flush(disp);
   __evas_image_cache_max = size;
}

void
__evas_x11_image_cache_set_size(Display *disp, int size)
{
   __evas_image_cache_max = size;
   imlib_set_cache_size(size * 2);
   __evas_x11_image_cache_flush(disp);
}

int
__evas_x11_image_cache_get_size(Display *disp)
{
   return __evas_image_cache_max;
}

void
__evas_x11_image_draw(Evas_X11_Image *im, 
		      Display *disp, Imlib_Image dstim, Window w, int win_w, int win_h,
		      int src_x, int src_y, int src_w, int src_h,
		      int dst_x, int dst_y, int dst_w, int dst_h,
		      int cr, int cg, int cb, int ca)
{
   Evas_List l;
   Imlib_Color_Modifier cm = NULL;

   if (ca == 0) return;
   if ((src_w == 0) || (src_h == 0) || (dst_w == 0) || (dst_w == 0)) return;

   if (__evas_clip)
     {
	cr = (cr * __evas_clip_r) / 255;
	cg = (cg * __evas_clip_g) / 255;
	cb = (cb * __evas_clip_b) / 255;                               
	ca = (ca * __evas_clip_a) / 255;
     }                      
   if (ca < 128) return;
   if ((cr != 255) || (cg != 255) || (cb != 255) || (ca != 255))
     {
	DATA8 r[256], g[256], b[256], a[256];
	int i;
	
	cm = imlib_create_color_modifier();
	imlib_context_set_color_modifier(cm);
	for (i = 0; i < 256; i++)
	  {
	     r[i] = (i * cr) / 255;
	     g[i] = (i * cg) / 255;
	     b[i] = (i * cb) / 255;
	     a[i] = (i * ca) / 255;
	  }
	imlib_set_color_modifier_tables(r, g, b, a);
     }
   else
      imlib_context_set_color_modifier(NULL);

   imlib_context_set_display(disp);
   imlib_context_set_visual(__evas_visual);
   imlib_context_set_colormap(__evas_cmap);
   imlib_context_set_drawable(w);
   imlib_context_set_dither_mask(0);
   imlib_context_set_anti_alias(0);
   if (imlib_get_visual_depth(disp, __evas_visual) < 8) 
     imlib_context_set_dither(__evas_anti_alias);
   else imlib_context_set_dither(0);
   imlib_context_set_blend(0);
   imlib_context_set_angle(0.0);
   imlib_context_set_operation(IMLIB_OP_COPY);
   imlib_context_set_direction(IMLIB_TEXT_TO_RIGHT);
   for(l = drawable_list; l; l = l->next)
     {
	Evas_X11_Drawable *dr;
	
	dr = l->data;
	if ((dr->win == w) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_X11_Update *up;
		  
		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      dst_x, dst_y, dst_w, dst_h))
		    {
		       int xx, yy, ww, hh, iw, ih, dx, dy, dw, dh;
		       

		       if (!up->p)
			  up->p = XCreatePixmap(disp, w, up->w, up->h, dr->depth);

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
			    if ((ww != im->pw) || (hh != im->ph) ||
				(cr != im->pr) || (cg != im->pg) || 
				(cb != im->pb) || (ca != im->pa))
			      {
				 if (im->pmap) imlib_free_pixmap_and_mask(im->pmap);
				 if (im->gc) XFreeGC(im->disp, im->gc);
				 im->pmap = 0;
				 im->mask = 0;
				 im->gc = 0;
				 im->pw = 0;
				 im->ph = 0;
			      }
			    if (!im->pmap)
			      {
				 XGCValues gcv;
				 
				 imlib_context_set_image(im->image);
				 im->pw = ww;
				 im->ph = hh;
				 im->pr = cr;
				 im->pg = cg;
				 im->pb = cb;
				 im->pa = ca;
				 imlib_render_pixmaps_for_whole_image_at_size(&im->pmap, &im->mask,
									      ww, hh);
				 gcv.graphics_exposures = False;
				 im->gc = XCreateGC(disp, w, 
						    GCGraphicsExposures, &gcv);
				 im->win = w;
				 if (im->mask) 
				   XSetClipMask(disp, im->gc, im->mask);
			      }
			    if (im->mask)
			      XSetClipOrigin(disp, im->gc, 
					     dst_x - up->x - src_x, 
					     dst_y - up->y - src_y);
			    if (im->pmap)
			      XCopyArea(disp, 
					im->pmap, up->p, 
					im->gc, 
					xx, yy, 
					dw, dh, 
					dx - up->x, dy - up->y);
			 }
		    }
	       }
	  }
     }
   if (cm)
     {
	imlib_free_color_modifier();
	imlib_context_set_color_modifier(NULL);
     }
}

int
__evas_x11_image_get_width(Evas_X11_Image *im)
{
   return im->w;
}

int
__evas_x11_image_get_height(Evas_X11_Image *im)
{
   return im->h;
}

void
__evas_x11_image_set_borders(Evas_X11_Image *im, int left, int right,
			       int top, int bottom)
{
   Imlib_Border bd;
   
   imlib_context_set_image(im->image);
   imlib_image_get_border(&bd);
   if ((bd.left != left) || (bd.right != right) || 
       (bd.top != top) || (bd.bottom != bottom))
     {
	if (im->pmap) imlib_free_pixmap_and_mask(im->pmap);
	if (im->gc) XFreeGC(im->disp, im->gc);
	im->pmap = 0;
	im->mask = 0;
	im->gc = 0;
	im->pw = 0;
	im->ph = 0;
	bd.left = left;
	bd.right = right;
	bd.top = top;
	bd.bottom = bottom;
	imlib_image_set_border(&bd);
     }
}

void
__evas_x11_image_set_smooth_scaling(int on)
{
   __evas_anti_alias = on;
}



























/*****************************************************************************/
/* font internals ************************************************************/
/*****************************************************************************/

#define TT_VALID( handle )  ( ( handle ).z != NULL )

static Evas_List        __evas_fonts           = NULL;

static char    **__evas_fpath           = NULL;
static int       __evas_fpath_num       = 0;
static int       __evas_font_cache      = 0;
static int       __evas_font_cache_max  = 512 * 1024;

static int       __evas_have_tt_engine  = 0;
static TT_Engine __evas_tt_engine;

static Evas_X11_Glyph *
__evas_x11_text_font_get_glyph(Evas_X11_Font *fn, int glyph)
{
   Evas_X11_Glyph *g;
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
   g = malloc(sizeof(Evas_X11_Glyph));
   g->glyph_id = glyph;
   TT_New_Glyph(fn->face, &(g->glyph));
   code = TT_Char_Index(fn->char_map, glyph);
   TT_Load_Glyph(fn->instance, g->glyph, code, TTLOAD_SCALE_GLYPH | TTLOAD_HINT_GLYPH);
   TT_Get_Glyph_Metrics(g->glyph, &(g->metrics));
   g->pw = 0;
   g->ph = 0;
   g->pmap = 0;
   fn->glyphs[hash] = evas_list_prepend(fn->glyphs[hash], g);
   return g;
}

static TT_Raster_Map *
__evas_x11_text_font_raster_new(int width, int height)
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
__evas_x11_text_font_raster_free(TT_Raster_Map * rmap)
{
   if (rmap->bitmap) free(rmap->bitmap);
   free(rmap);
}

static void
__evas_x11_text_font_render_glyph(Window win, Evas_X11_Font *fn, Evas_X11_Glyph *g)
{
   int tw, th, xmin, ymin, xmax, ymax;
   TT_Raster_Map *rmap;
   
   if (g->pmap) return;
   xmin = g->metrics.bbox.xMin & -64;
   ymin = g->metrics.bbox.yMin & -64;
   xmax = (g->metrics.bbox.xMax + 63) & -64;
   ymax = (g->metrics.bbox.yMax + 63) & -64;
   tw = ((xmax - xmin) / 64) + 1;
   th = ((ymax - ymin) / 64) + 1;
   
   g->pmap = XCreatePixmap(fn->disp, win, tw, th, 1);
   g->pw = tw;
   g->ph = th;

   fn->mem_use += (((g->pw -1) | 0x7) + 1) * g->ph / 8;
   
   rmap = __evas_x11_text_font_raster_new(tw, th);
   if (rmap)
     {
	int x, y;
	XImage *xim;
        XGCValues gcv;
	GC gc;
	
	xim = XCreateImage(fn->disp, __evas_visual, 1, ZPixmap, 0, NULL, tw, th, 8, 0);
	xim->data = malloc(xim->bytes_per_line * xim->height);	
	TT_Get_Glyph_Bitmap(g->glyph, rmap, -xmin, -ymin);
	for (y = 0; y < th; y++)
	  {
	     for (x = 0; x < tw; x++)
	       {
		  DATA8 rval;
		  
		  rval = (DATA8)(((unsigned char *)(rmap->bitmap))[((rmap->rows - y - 1) * rmap->cols) + (x >> 3)]);
		  rval >>= (7 - (x - ((x >> 3) << 3)));
		  XPutPixel(xim, x, y, rval);
	       }
	  }	
	gc = XCreateGC(fn->disp, g->pmap, 0, &gcv);
	XPutImage(fn->disp, g->pmap, gc, xim, 0, 0, 0, 0, tw, th);
	XDestroyImage(xim);
	XFreeGC(fn->disp, gc);
	__evas_x11_text_font_raster_free(rmap);
     }
}

__evas_x11_is_file(char *file)
{
   struct stat         st;
   
   if (stat(file, &st) < 0)
     return 0;
   return 1;
}

static char *
__evas_x11_font_find(char *font)
{
   char buf[4096];
   char *ext[] = {".ttf", ".TTF", ""
   };
   int i, j;
   
   for (i = 0; i < 3; i++)
     {
	sprintf(buf, "%s%s", font, ext[i]);
	if (__evas_x11_is_file(buf)) return strdup(buf);
     }
   for (j = 0; j < __evas_fpath_num; j++)
     {
	for (i = 0; i < 3; i++)
	  {
	     sprintf(buf, "%s/%s%s", __evas_fpath[j], font, ext[i]);
	     if (__evas_x11_is_file(buf)) return strdup(buf);
	  }
     }
   return NULL;
}

static Evas_X11_Font *
__evas_x11_font_load(char *font, int size)
{
   Evas_X11_Font *fn;
   char *file;
   TT_Error error;
   int i, num_cmap, upm;
   const int dpi = 96;
   
   file = __evas_x11_font_find(font);
   if (!file) return NULL;
   if (!__evas_have_tt_engine)
     {
	error = TT_Init_FreeType(&__evas_tt_engine);
	if (error) return NULL;
	__evas_have_tt_engine = 1;
     }
   fn = malloc(sizeof(Evas_X11_Font));
   fn->font = strdup(font);
   fn->size = size;
   fn->engine = __evas_tt_engine;
   fn->mem_use = 0;
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
   fn->references = 1;
   
   /* go thru the first 256 glyps to calculate max ascent/descent */
     {
	Evas_X11_Glyph *g;
	
	for (i = 0; i < 256; i++)
	  {
	     g = __evas_x11_text_font_get_glyph(fn, i);
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
__evas_x11_text_font_cache_flush(void)
{
   Evas_List l;
   Evas_X11_Font *fn_last;
   Evas_X11_Font *fn;
   
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
	     
	     __evas_font_cache -= fn->mem_use;
	     TT_Done_Instance(fn_last->instance);
	     TT_Close_Face(fn_last->face);
	     if (fn_last->font) free(fn_last->font);
	     for (i = 0; i < 256; i++)
	       {
		  if (fn_last->glyphs[i])
		    {
		       for (l = fn_last->glyphs[i]; l; l = l->next)
			 {
			    Evas_X11_Glyph *g;
			    Evas_List ll;
			    
			    g = l->data;
			    if (g->pmap)
			      XFreePixmap(fn->disp, g->pmap);
			    free(g);
			 }
		       evas_list_free(fn_last->glyphs[i]);
		    }
	       }
	     __evas_fonts = evas_list_remove(__evas_fonts, fn_last);
	     free(fn_last);
	  }
     }
}
/*****************************************************************************/
/* font externals ************************************************************/
/*****************************************************************************/

Evas_X11_Font *
__evas_x11_text_font_new(Display *disp, char *font, int size)
{
   Evas_List l;
   Evas_X11_Font *fn;
   
   if (!font) return NULL;
   for (l = __evas_fonts; l; l = l->next)
     {
	Evas_X11_Font *fn;
	
	fn = l->data;
	if (!strcmp(fn->font, font) && (size == fn->size))
	  {
	     if (l != __evas_fonts)
	       {
		  __evas_fonts = evas_list_remove(__evas_fonts, fn);
		  __evas_fonts = evas_list_prepend(__evas_fonts, fn);
	       }
	     if (fn->references == 0)
	       __evas_font_cache -= fn->mem_use;
	     fn->references++;
	     return fn;
	  }
     }
   fn = __evas_x11_font_load(font, size);
   if (!fn) return NULL;
   __evas_fonts = evas_list_prepend(__evas_fonts, fn);
   return fn;
}

void
__evas_x11_text_font_free(Evas_X11_Font *fn)
{
   if (!fn) return;
   if (fn->references >= 0)
     {
	fn->references--;
	if (fn->references == 0)
	  __evas_font_cache += fn->mem_use;
     }
   __evas_x11_text_font_cache_flush();
}

int
__evas_x11_text_font_get_ascent(Evas_X11_Font *fn)
{
   if (!fn) return 0;
   return fn->ascent;
}

int
__evas_x11_text_font_get_descent(Evas_X11_Font *fn)
{
   if (!fn) return 0;
   return fn->descent;
}

int
__evas_x11_text_font_get_max_ascent(Evas_X11_Font *fn)
{
   if (!fn) return 0;
   return fn->max_ascent;
}

int
__evas_x11_text_font_get_max_descent(Evas_X11_Font *fn)
{
   if (!fn) return 0;
   return fn->max_descent;
}

void
__evas_x11_text_font_get_advances(Evas_X11_Font *fn, char *text,
				    int *advance_horiz,
				    int *advance_vert)
{
   int                 i, ascent, descent, pw, ph;
   
   if (advance_horiz) *advance_horiz = 0;
   if (advance_horiz) *advance_vert = 0;
   if (!fn) return;
   if (!text) return;
   if (text[0] == 0) return;
   
   ascent = fn->ascent;
   descent = fn->descent;
   pw = 0;
   ph = ascent + descent;
   
   for (i = 0; text[i]; i++)
     {
	Evas_X11_Glyph *g;
	int glyph;
	
	glyph = ((unsigned char *)text)[i];
	g = __evas_x11_text_font_get_glyph(fn, glyph);
	if (!g) continue;
	if (!TT_VALID(g->glyph)) continue;
	if (i == 0)
	  pw += ((-g->metrics.bearingX) / 64);
	pw += g->metrics.advance / 64;
     }
   *advance_horiz = pw;
   *advance_vert = ph;
}

int
__evas_x11_text_font_get_first_inset(Evas_X11_Font *fn, char *text)
{
   int                 i;
   
   if (!fn) return 0;
   if (!text) return 0;
   if (text[0] == 0) return 0;
   
   for (i = 0; text[i]; i++)
     {
	Evas_X11_Glyph *g;
	int glyph;
	
	glyph = ((unsigned char *)text)[i];
	g = __evas_x11_text_font_get_glyph(fn, glyph);
	if (!g) continue;
	if (!TT_VALID(g->glyph)) continue;
             return ((-g->metrics.bearingX) / 64);
     }
   return 0;
}

void
__evas_x11_text_font_add_path(char *path)
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
__evas_x11_text_font_del_path(char *path)
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
__evas_x11_text_font_list_paths(int *count)
{
   *count = __evas_fpath_num;
   return __evas_fpath;
}

void
__evas_x11_text_cache_empty(Display *disp)
{
   int prev_cache;

   prev_cache = __evas_font_cache_max;
   __evas_font_cache_max = 0;
   __evas_x11_text_font_cache_flush();
   __evas_font_cache_max = prev_cache;
}

void
__evas_x11_text_cache_set_size(Display *disp, int size)
{
   __evas_font_cache_max = size;
   __evas_x11_text_font_cache_flush();
}

int
__evas_x11_text_cache_get_size(Display *disp)
{
   return __evas_font_cache_max;
}

void
__evas_x11_text_draw(Evas_X11_Font *fn, Display *disp, Imlib_Image dstim, Window win, 
		       int win_w, int win_h, int x, int y, char *text, 
		       int cr, int cg, int cb, int ca)
{
   Evas_List l;
   DATA32 pixel;
   int w, h;

   if (__evas_clip)
     {
	cr = (cr * __evas_clip_r) / 255;
	cg = (cg * __evas_clip_g) / 255;
	cb = (cb * __evas_clip_b) / 255;                               
	ca = (ca * __evas_clip_a) / 255;
     }
   if (ca < 128) return;
   imlib_context_set_display(disp);
   imlib_context_set_visual(__evas_visual);
   imlib_context_set_colormap(__evas_cmap);
   imlib_context_set_drawable(win);
   imlib_context_set_color(cr, cg, cb, ca);
   pixel = imlib_render_get_pixel_color();
   imlib_context_set_dither_mask(0);
   imlib_context_set_anti_alias(0);
   imlib_context_set_dither(0);
   imlib_context_set_blend(0);
   imlib_context_set_angle(0.0);
   imlib_context_set_operation(IMLIB_OP_COPY);
   imlib_context_set_direction(IMLIB_TEXT_TO_RIGHT);
   imlib_context_set_color_modifier(NULL);
   __evas_x11_text_get_size(fn, text, &w, &h);
   fn->disp = disp;
   for(l = drawable_list; l; l = l->next)
     {
	Evas_X11_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_X11_Update *up;

		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      x, y, w, h))
		    {
		       if (!up->p)
			 up->p = XCreatePixmap(disp, win, up->w, up->h, dr->depth);
		       XSetForeground(disp, dr->gc, pixel);
		       XSetClipMask(disp, dr->gc, None);
		       XSetClipOrigin(disp, dr->gc, 0, 0);
		       XSetFillStyle(disp, dr->gc, FillStippled);
			 {
			    int xx, yy, ww, hh;
			    XRectangle rect;
			    
			    xx = up->x;
			    yy = up->y;
			    ww = up->w;
			    hh = up->h;
			    
			    if (__evas_clip)
			      {
				 CLIP_TO(xx, yy, ww, hh,
					 __evas_clip_x, __evas_clip_y,
					 __evas_clip_w, __evas_clip_h);
			      }
			    rect.x = xx - up->x;
			    rect.y = yy - up->y;
			    rect.width = ww;
			    rect.height = hh;
			    XSetClipRectangles(disp, dr->gc, 0, 0, 
					       &rect, 1, Unsorted);
			      {
				 int i;
				 int x_offset, y_offset;
				 int glyph, rows;
				 Evas_X11_Glyph *g;
				 
				 if (text[0] == 0) return;
				 glyph = ((unsigned char *)text)[0];
				 g = __evas_x11_text_font_get_glyph(fn, glyph);
				 if (!TT_VALID(g->glyph)) continue;
				 x_offset = 0;
				 if (g) x_offset = - (g->metrics.bearingX / 64);
				 y_offset = -(fn->max_descent / 64);
				 rows = h;
				 for (i = 0; text[i]; i++)
				   {
				      int xmin, ymin, xmax, ymax, off, adj;
				      
				      /* for internationalization this here wouldnt just use */
				      /* the char value of the text[i] but translate form utf-8 */
				      /* or whetever and incriment i appropriately and set g to */
				      /* the glyph index */
				      glyph = ((unsigned char *)text)[i];
				      g = __evas_x11_text_font_get_glyph(fn, glyph);
				      __evas_x11_text_font_render_glyph(win, fn, g);
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
				      if (g->pmap)
					{
					   XSetStipple(disp, dr->gc, g->pmap);
					   XSetTSOrigin(disp, dr->gc,
							x + xmin - up->x, 
							y + ymin + off + adj - up->y);
					   XFillRectangle(disp, up->p, dr->gc, 
							  x + xmin - up->x, 
							  y + ymin + off + adj - up->y, 
							  xmax - xmin + 1, 
							  ymax - ymin + 1);
					   x_offset += g->metrics.advance / 64;
					}
				   }
			      }
			 }
		    }
	       }
	  }
     }
}

void
__evas_x11_text_get_size(Evas_X11_Font *fn, char *text, int *w, int *h)
{
   int i, pw, ph;
   Evas_X11_Glyph *g;
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
	g = __evas_x11_text_font_get_glyph(fn, glyph);
	if (!g) continue;
	if (!TT_VALID(g->glyph)) continue;
	if (i == 0)
	  pw += ((-g->metrics.bearingX) / 64);
	if (text[i + 1] == 0) /* last char - ineternationalization issue */
	  pw += (g->metrics.bbox.xMax / 64);
	else
	  pw += g->metrics.advance / 64;
     }
   if (w) *w = pw;
   if (h) *h = ph;
}

int
__evas_x11_text_get_character_at_pos(Evas_X11_Font *fn, char *text, 
				       int x, int y, 
				       int *cx, int *cy, int *cw, int *ch)
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
	Evas_X11_Glyph *g;
	int glyph;
	
	glyph = ((unsigned char *)text)[i];
	g = __evas_x11_text_font_get_glyph(fn, glyph);
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
__evas_x11_text_get_character_number(Evas_X11_Font *fn, char *text, 
				       int num, 
				       int *cx, int *cy, int *cw, int *ch)
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
	Evas_X11_Glyph *g;
	int glyph;
	
	glyph = ((unsigned char *)text)[i];
	g = __evas_x11_text_font_get_glyph(fn, glyph);
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

void              __evas_x11_rectangle_draw(Display *disp, Imlib_Image dstim, Window win,
					      int win_w, int win_h,
					      int x, int y, int w, int h,
					      int cr, int cg, int cb, int ca)
{
   Evas_List l;
   DATA32 pixel;

   if (__evas_clip)
     {
	cr = (cr * __evas_clip_r) / 255;
	cg = (cg * __evas_clip_g) / 255;
	cb = (cb * __evas_clip_b) / 255;                               
	ca = (ca * __evas_clip_a) / 255;
     }
   if (ca < 128) return;
   imlib_context_set_display(disp);
   imlib_context_set_visual(__evas_visual);
   imlib_context_set_colormap(__evas_cmap);
   imlib_context_set_drawable(win);
   imlib_context_set_color(cr, cg, cb, ca);
   pixel = imlib_render_get_pixel_color();
   imlib_context_set_dither_mask(0);
   imlib_context_set_anti_alias(0);
   imlib_context_set_dither(0);
   imlib_context_set_blend(0);
   imlib_context_set_angle(0.0);
   imlib_context_set_operation(IMLIB_OP_COPY);
   imlib_context_set_direction(IMLIB_TEXT_TO_RIGHT);
   imlib_context_set_color_modifier(NULL);
   for(l = drawable_list; l; l = l->next)
     {
	Evas_X11_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_X11_Update *up;

		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      x, y, w, h))
		    {
		       if (!up->p)
			  up->p = XCreatePixmap(disp, win, up->w, up->h, dr->depth);
		       XSetForeground(disp, dr->gc, pixel);
		       XSetClipMask(disp, dr->gc, None);
		       XSetClipOrigin(disp, dr->gc, 0, 0);
		       XSetFillStyle(disp, dr->gc, FillSolid);
			 {
			    int xx, yy, ww, hh;
			    
			    xx = x;
			    yy = y;
			    ww = w;
			    hh = h;
			    
			    CLIP_TO(xx, yy, ww, hh, up->x, up->y, up->w, up->h);
			    if (__evas_clip)
			      {
				 CLIP_TO(xx, yy, ww, hh, 
					 __evas_clip_x, __evas_clip_y, 
					 __evas_clip_w, __evas_clip_h);
			      }
			    if ((w > 1) && (h > 1))
			      XFillRectangle(disp, up->p, dr->gc, 
					     xx - up->x, yy - up->y, ww, hh);
			 }
		    }
	       }
	  }
     }
}

















/*****************************************************************************/
/* rectangle externals *******************************************************/
/*****************************************************************************/

void              __evas_x11_line_draw(Display *disp, Imlib_Image dstim, Window win,
					 int win_w, int win_h,
					 int x1, int y1, int x2, int y2,
					 int cr, int cg, int cb, int ca)
{
   Evas_List l;
   int x, y, w, h;
   DATA32 pixel;

   if (__evas_clip)
     {
	cr = (cr * __evas_clip_r) / 255;
	cg = (cg * __evas_clip_g) / 255;
	cb = (cb * __evas_clip_b) / 255;                               
	ca = (ca * __evas_clip_a) / 255;
     }
   if (ca < 128) return;
   imlib_context_set_display(disp);
   imlib_context_set_visual(__evas_visual);
   imlib_context_set_colormap(__evas_cmap);
   imlib_context_set_drawable(win);
   imlib_context_set_color(cr, cg, cb, ca);
   pixel = imlib_render_get_pixel_color();
   imlib_context_set_dither_mask(0);
   imlib_context_set_anti_alias(0);
   imlib_context_set_dither(0);
   imlib_context_set_blend(0);
   imlib_context_set_angle(0.0);
   imlib_context_set_operation(IMLIB_OP_COPY);
   imlib_context_set_direction(IMLIB_TEXT_TO_RIGHT);
   imlib_context_set_color_modifier(NULL);
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
	Evas_X11_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_X11_Update *up;

		  up = ll->data;
		  
		  /* if image intersects image update - render */
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      x, y, w, h))
		    {
		       if (!up->p)
			 up->p = XCreatePixmap(disp, win, up->w, up->h, dr->depth);
		       XSetForeground(disp, dr->gc, pixel);
		       XSetClipMask(disp, dr->gc, None);
		       XSetClipOrigin(disp, dr->gc, 0, 0);
		       XSetFillStyle(disp, dr->gc, FillSolid);
			 {
			    int xx, yy, ww, hh;
			    XRectangle rect;
			    
			    xx = up->x;
			    yy = up->y;
			    ww = up->w;
			    hh = up->h;
			    
			    if (__evas_clip)
			      {
				 CLIP_TO(xx, yy, ww, hh,
					 __evas_clip_x, __evas_clip_y,
					 __evas_clip_w, __evas_clip_h);
			      }
			    rect.x = xx - up->x;
			    rect.y = yy - up->y;
			    rect.width = ww;
			    rect.height = hh;
			    XSetClipRectangles(disp, dr->gc, 0, 0, 
					       &rect, 1, Unsorted);
			    XDrawLine(disp, up->p, dr->gc, 
				      x1 - up->x, y1 - up->y, 
				      x2 - up->x, y2 - up->y);
			 }
		    }
	       }
	  }
     }
}

















/*****************************************************************************/
/* gradient externals ********************************************************/
/*****************************************************************************/


Evas_X11_Graident *
__evas_x11_gradient_new(Display *disp)
{
   Evas_X11_Graident *gr;

   gr = malloc(sizeof(Evas_X11_Graident));
   gr->colors = NULL;
}

void
__evas_x11_gradient_free(Evas_X11_Graident *gr)
{
   Evas_List l;

   if (gr->colors)
     {
	for (l = gr->colors; l; l = l->next)
	  {  
	     free(l->data);
	  }
	evas_list_free(gr->colors);
     }
   free(gr);
}

void
__evas_x11_gradient_color_add(Evas_X11_Graident *gr, int r, int g, int b, int a, int dist)
{
   Evas_X11_Color *cl;
                        
   cl = malloc(sizeof(Evas_X11_Color));
   cl->r = r;
   cl->g = g;
   cl->b = b;
   cl->a = a;
   cl->dist = dist;
   gr->colors = evas_list_append(gr->colors, cl);
}

void
__evas_x11_gradient_draw(Evas_X11_Graident *gr, Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x, int y, int w, int h, double angle)
{
   Evas_List l;
   
   imlib_context_set_display(disp);
   imlib_context_set_visual(__evas_visual);
   imlib_context_set_colormap(__evas_cmap);
   imlib_context_set_drawable(win);
   imlib_context_set_dither_mask(0);
   imlib_context_set_anti_alias(0);
   imlib_context_set_dither(0);
   imlib_context_set_blend(0);
   imlib_context_set_angle(0.0);
   imlib_context_set_operation(IMLIB_OP_COPY);
   imlib_context_set_direction(IMLIB_TEXT_TO_RIGHT);
   imlib_context_set_color_modifier(NULL);

   while (angle < 0.0) angle += 360.0;
   while (angle > 360.0) angle -= 360.0;
   
   for (l = drawable_list; l; l = l->next)
     {
	Evas_X11_Drawable *dr;
	
	dr = l->data;
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_X11_Update *up;

		  up = ll->data;
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      x, y, w, h))
		    {
		       int i, p, tot;
		       int xx, yy, ww, hh;
		       XRectangle rect;
		       Evas_List l2;
		       
		       if (!up->p)
			  up->p = XCreatePixmap(disp, win, up->w, up->h, dr->depth);
		       
		       XSetClipMask(disp, dr->gc, None);
		       XSetClipOrigin(disp, dr->gc, 0, 0);
		       XSetFillStyle(disp, dr->gc, FillSolid);
		       		       
		       xx = x;
		       yy = y;
		       ww = w;
		       hh = h;		       
		       if (__evas_clip)
			 {
			    CLIP_TO(xx, yy, ww, hh,
				    __evas_clip_x, __evas_clip_y,
				    __evas_clip_w, __evas_clip_h);
			 }
		       tot = 0;
		       for (l2 = gr->colors; l2; l2 = l2->next)
			 {
			    Evas_X11_Color *c1;
			    
			    c1 = l2->data;
			    if (l2 != gr->colors) tot += c1->dist;
			 }
		       tot <<= 2;
		       if ((ww > 1) && (hh > 1))
			 {
			    rect.x = xx - up->x;
			    rect.y = yy - up->y;
			    rect.width = ww;
			    rect.height = hh;
			    XSetClipRectangles(disp, dr->gc, 0, 0,
					       &rect, 1, Unsorted);
			    p = 0;
			    for (l2 = gr->colors; l2; l2 = l2->next)
			      {
				 int r, g, b, a;
				 int dist;
				 Evas_X11_Color *c1, *c2;
				 XPoint xpoints[4];
				 DATA32 pixel;
				 double x1, y1, x2, y2, x3, y3, per1, per2;
				 
				 c1 = l2->data;
				 if (!l2->next) break;;
				 c2 = l2->next->data;
				 dist = c2->dist;
				 
				 for (i = 0; i < (dist << 2); i++)
				   {
				      r = ((c1->r * ((dist << 2) - i)) + (c2->r * i)) / (dist << 2);
				      g = ((c1->g * ((dist << 2) - i)) + (c2->g * i)) / (dist << 2);
				      b = ((c1->b * ((dist << 2) - i)) + (c2->b * i)) / (dist << 2);
				      a = ((c1->a * ((dist << 2) - i)) + (c2->a * i)) / (dist << 2);
				      if (__evas_clip)
					{
					   r = (r * __evas_clip_r) / 255;
					   g = (g * __evas_clip_g) / 255;
					   b = (b * __evas_clip_b) / 255;
					   a = (a * __evas_clip_a) / 255;
					}
				      if (a >= 128)
					{
					   /*      |  0/x2 */
					   /*      |_ /    */
					   /*      |a/     */
					   /*    -_|/      */
					   /*      |-_     */
					   /* x3 1/|  x1   */
					   
					   x1 = cos((angle * 2.0 * 3.141592654) / 360.0);
					   y1 = sin((angle * 2.0 * 3.141592654) / 360.0);
					   x2 = cos(((angle - 90) * 2.0 * 3.141592654) / 360.0);
					   y2 = sin(((angle - 90) * 2.0 * 3.141592654) / 360.0);
					   
					   if ((angle > 0.0) && (angle <= 90.0))         {x3 = -1.0; y3 =  1.0;}
					   else if ((angle > 90.0) && (angle <= 180.0))  {x3 = -1.0; y3 = -1.0;}
					   else if ((angle > 180.0) && (angle <= 270.0)) {x3 =  1.0; y3 = -1.0;}
					   else                                          {x3 =  1.0; y3 =  1.0;}
					   
					   if ((angle > 315.0) || (angle <= 45.0))       {x2 *= -1.0 / y2; y2 = -1.0;}
					   else if ((angle > 45.0) && (angle <= 135.0))  {y2 *=  1.0 / x2; x2 =  1.0;}
					   else if ((angle > 135.0) && (angle <= 225.0)) {x2 *=  1.0 / y2; y2 =  1.0;}
					   else                                          {y2 *= -1.0 / x2; x2 = -1.0;}
					   
					   x3 = (x3 + 1.0) / 2.0;
					   y3 = (y3 + 1.0) / 2.0;
					   
					   x2 *= 0.5;
					   y2 *= 0.5;
					   
					   per1 = (-1.0 + ((((double)(tot - p - 1) / (double)tot) - 0.5) * 2.0));
					   per2 = (-1.0 + ((((double)(tot - p) / (double)tot) - 0.5) * 2.0));
					   
					   xpoints[0].x = x - up->x + (int)
					     ((x3 + (2.0 * x1) + (-x2 * per2)) * (double)w);
					   xpoints[0].y = y - up->y + (int)
					     ((y3 + (2.0 * y1) + (-y2 * per2)) * (double)h);
					   
					   xpoints[1].x = x - up->x + (int)
					     ((x3 + (2.0 * x1) + (-x2 * per1)) * (double)w);
					   xpoints[1].y = y - up->y + (int)
					     ((y3 + (2.0 * y1) + (-y2 * per1)) * (double)h);
					   
					   xpoints[2].x = x - up->x + (int)
					     ((x3 - (2.0 * x1) + (-x2 * per1)) * (double)w);
					   xpoints[2].y = y - up->y + (int)
					     ((y3 - (2.0 * y1) + (-y2 * per1)) * (double)h);
					   
					   xpoints[3].x = x - up->x + (int)
					     ((x3 - (2.0 * x1) + (-x2 * per2)) * (double)w);
					   xpoints[3].y = y - up->y + (int)
					     ((y3 - (2.0 * y1) + (-y2 * per2)) * (double)h);
					   
					   imlib_context_set_color(r, g, b, a);
					   pixel = 0;
					   pixel = imlib_render_get_pixel_color();
					   XSetForeground(disp, dr->gc, pixel);
					   XFillPolygon(disp, up->p, dr->gc, 
							xpoints, 4, 
							Convex, CoordModeOrigin);
					}
				      p++;
				   }
			      }
			 }
		    }
	       }
	  }
     }
}











/************/
/* polygons */
/************/
void
__evas_x11_poly_draw (Display *disp, Imlib_Image dstim, Window win, 
		      int win_w, int win_h, 
		      Evas_List points, 
		      int cr, int cg, int cb, int ca)
{
   Evas_List l, l2;
   DATA32 pixel;
   int x, y, w, h;

   if (__evas_clip)
     {
	cr = (cr * __evas_clip_r) / 255;
	cg = (cg * __evas_clip_g) / 255;
	cb = (cb * __evas_clip_b) / 255;                               
	ca = (ca * __evas_clip_a) / 255;
     }
   if (ca < 128) return;
   imlib_context_set_display(disp);
   imlib_context_set_visual(__evas_visual);
   imlib_context_set_colormap(__evas_cmap);
   imlib_context_set_drawable(win);
   imlib_context_set_color(cr, cg, cb, ca);
   pixel = imlib_render_get_pixel_color();
   imlib_context_set_dither_mask(0);
   imlib_context_set_anti_alias(0);
   imlib_context_set_dither(0);
   imlib_context_set_blend(0);
   imlib_context_set_angle(0.0);
   imlib_context_set_operation(IMLIB_OP_COPY);
   imlib_context_set_direction(IMLIB_TEXT_TO_RIGHT);
   imlib_context_set_color_modifier(NULL);
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
	Evas_X11_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_X11_Update *up;
		  
		  up = ll->data;
		  
		  if (RECTS_INTERSECT(up->x, up->y, up->w, up->h,
				      x, y, w, h))
		    {
		       if (!up->p)
			  up->p = XCreatePixmap(disp, win, up->w, up->h, dr->depth);
		       XSetForeground(disp, dr->gc, pixel);
		       XSetClipMask(disp, dr->gc, None);
		       XSetClipOrigin(disp, dr->gc, 0, 0);
		       XSetFillStyle(disp, dr->gc, FillSolid);
			 {
			    XPoint *xpoints;
			    int point_count, i;
			    int xx, yy, ww, hh;
			    XRectangle rect;
			    
			    xx = up->x;
			    yy = up->y;
			    ww = up->w;
			    hh = up->h;
			    
			    if (__evas_clip)
			      {
				 CLIP_TO(xx, yy, ww, hh,
					 __evas_clip_x, __evas_clip_y,
					 __evas_clip_w, __evas_clip_h);
			      }
			    rect.x = xx - up->x;
			    rect.y = yy - up->y;
			    rect.width = ww;
			    rect.height = hh;
			    XSetClipRectangles(disp, dr->gc, 0, 0, 
					       &rect, 1, Unsorted);
			    point_count = 0;
			    for (l2 = points; l2; l2 = l2->next) point_count++;
			    xpoints = malloc(point_count * sizeof(XPoint));
			    for (l2 = points, i = 0; l2; l2 = l2->next, i++)
			      {
				 Evas_Point p;
				 
				 p = l2->data;
				 xpoints[i].x = p->x - up->x;
				 xpoints[i].y = p->y - up->y;
			      }
			    XFillPolygon(disp, up->p, dr->gc, xpoints, 
					 point_count, Complex, 
					 CoordModeOrigin);
			    free(xpoints);
			 }
		    }
	       }
	  }
     }
}










/*****************************************************************************/
/* general externals *********************************************************/
/*****************************************************************************/

void
__evas_x11_set_clip_rect(int on, int x, int y, int w, int h, int r, int g, int b, int a)
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
__evas_x11_sync(Display *disp)
{
   XSync(disp, False);
}

void
__evas_x11_flush_draw(Display *disp, Imlib_Image dstim, Window win)
{
   Evas_List l;
   
   for(l = drawable_list; l; l = l->next)
     {
	Evas_X11_Drawable *dr;
	
	dr = l->data;
	
	XSetClipMask(disp, dr->gc, None);
	XSetClipOrigin(disp, dr->gc, 0, 0);
	XSetFillStyle(disp, dr->gc, FillSolid);
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_List ll;
	     
	     for (ll = dr->tmp_images; ll; ll = ll->next)
	       {
		  Evas_X11_Update *up;
		  
		  up = ll->data;

		  if (up->p)
		    {
		       XCopyArea(disp, up->p, win, dr->gc, 
				 0, 0, up->w, up->h, up->x, up->y);
		       XFreePixmap(disp, up->p);
		    }
		  free(up);
	       }
	     if (dr->tmp_images)
		dr->tmp_images = evas_list_free(dr->tmp_images);
	  }
	XFreeGC(disp, dr->gc);
	free(dr);
     }
   if (drawable_list)
      drawable_list = evas_list_free(drawable_list);
   drawable_list = NULL;
}

   
int
__evas_x11_capable(Display *disp)
{
   return 1;
}

Visual *
__evas_x11_get_visual(Display *disp, int screen)
{
   int depth;
   
   __evas_visual = imlib_get_best_visual(disp, screen, &depth);
   return __evas_visual;
}

XVisualInfo *
__evas_x11_get_visual_info(Display *disp, int screen)
{
   static XVisualInfo *vi = NULL;
   XVisualInfo vi_template;
   int n;
   
   if (vi) return vi;
   vi_template.visualid = (__evas_x11_get_visual(disp, screen))->visualid;
   vi_template.screen = screen;
   vi = XGetVisualInfo(disp, VisualIDMask | VisualScreenMask, &vi_template ,&n);
   return vi;
}

Colormap
__evas_x11_get_colormap(Display *disp, int screen)
{
   Visual *v;
   
   if (__evas_cmap) return __evas_cmap;
   v = __evas_x11_get_visual(disp, screen);
   __evas_cmap = DefaultColormap(disp, screen);
   return __evas_cmap;
   __evas_cmap = XCreateColormap(disp, RootWindow(disp, screen), v, AllocNone);
   return __evas_cmap;
}

void
__evas_x11_init(Display *disp, int screen, int colors)
{
   static int initted = 0;
   
   if (!initted)
     {
	imlib_set_color_usage(216);
	imlib_set_font_cache_size(1024 * 1024);
	imlib_set_cache_size(8 * 1024 * 1024);
	initted = 1;
     }
   imlib_set_color_usage(colors);
}

void
__evas_x11_draw_add_rect(Display *disp, Imlib_Image dstim, Window win, 
			   int x, int y, int w, int h)
{
   Evas_List l;
   
   for(l = drawable_list; l; l = l->next)
     {
	Evas_X11_Drawable *dr;
	
	dr = l->data;
	
	if ((dr->win == win) && (dr->disp == disp))
	  {
	     Evas_X11_Update *up;
	     
	     up = malloc(sizeof(Evas_X11_Update));
	     up->x = x;
	     up->y = y;
	     up->w = w;
	     up->h = h;
	     up->p = 0;
	     dr->tmp_images = evas_list_append(dr->tmp_images, up);
	  }
	return;
     }
     {
	Evas_X11_Drawable *dr;
	Evas_X11_Update *up;
	GC gc;
	XGCValues gcv;
	XWindowAttributes att;
	
	gc = XCreateGC(disp, win, 0, &gcv);
	XGetWindowAttributes(disp, win, &att);
	dr = malloc(sizeof(Evas_X11_Drawable));
	dr->win = win;
	dr->disp = disp;
	dr->tmp_images = NULL;
	dr->gc = gc;
	dr->depth = att.depth;
	up = malloc(sizeof(Evas_X11_Update));
	up->x = x;
	up->y = y;
	up->w = w;
	up->h = h;
	up->p = 0;
	drawable_list = evas_list_append(drawable_list, dr);
	dr->tmp_images = evas_list_append(dr->tmp_images, up);
     }
}

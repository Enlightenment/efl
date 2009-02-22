#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_XRender_Xcb.h"

static Eina_Hash *_xr_image_hash        = NULL;
static int        _xr_image_cache_size  = 0;
static int        _xr_image_cache_usage = 0;
static Eina_List *_xr_image_cache       = NULL;
static Eina_Hash *_xr_image_dirty_hash  = NULL;

static void
__xre_image_dirty_hash_add(XR_Image *im)
{
   char buf[64];

   if (!im->data) return;
   snprintf(buf, sizeof(buf), "%p", im->data);
   if (!_xr_image_dirty_hash) _xr_image_dirty_hash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(_xr_image_dirty_hash, buf, im);
}

static void
__xre_image_dirty_hash_del(XR_Image *im)
{
   char buf[64];

   if (!im->data) return;
   snprintf(buf, sizeof(buf), "%p", im->data);
   eina_hash_del(_xr_image_dirty_hash, buf, im);
}

static XR_Image *
__xre_image_dirty_hash_find(void *data)
{
   char buf[64];

   snprintf(buf, sizeof(buf), "%p", data);
   return eina_hash_find(_xr_image_dirty_hash, buf);
}

static XR_Image *
__xre_image_find(char *fkey)
{
   XR_Image *im;

   im = eina_hash_find(_xr_image_hash, fkey);
   if (!im)
     {
	Eina_List *l;

	EINA_LIST_FOREACH(_xr_image_cache, l, im)
	  {
	     if (!strcmp(im->fkey, fkey))
	       {
		  _xr_image_cache = eina_list_remove_list(_xr_image_cache, l);
		  if (!_xr_image_hash) _xr_image_hash = eina_hash_string_superfast_new(NULL);
		  eina_hash_add(_xr_image_hash, im->fkey, im);
		  _xr_image_cache_usage -= (im->w * im->h * 4);
		  break;
	       }
	     im = NULL;
	  }
     }
   if (im) im->references++;
   return im;
}

XR_Image *
_xre_image_load(Xcb_Image_Info *xcbinf, const char *file, const char *key, Evas_Image_Load_Opts *lo)
{
   char      buf[4096];
   XR_Image *im;

   if (!file) return NULL;
   if (!lo)
     {
	if (key)
	  snprintf(buf, sizeof(buf), "/@%p@%x@/%s//://%s", xcbinf->conn, xcbinf->root, file, key);
	else
	  snprintf(buf, sizeof(buf), "/@%p@%x@/%s", xcbinf->conn, xcbinf->root, file);
     }
   else
     {
	if (key)
	  snprintf(buf, sizeof(buf), "//@/%i/%1.5f/%ix%i//@%p@%x@/%s//://%s", lo->scale_down_by, lo->dpi, lo->w, lo->h, xcbinf->conn, xcbinf->root, file, key);
	else
	  snprintf(buf, sizeof(buf), "//@/%i/%1.5f/%ix%i//@%p@%x@/%s", lo->scale_down_by, lo->dpi, lo->w, lo->h, xcbinf->conn, xcbinf->root, file);
     }
   im = __xre_image_find(buf);
   if (im)
     {
	return im;
     }

   im = calloc(1, sizeof(XR_Image));
   if (!im) return NULL;
   im->im = evas_common_load_image_from_file(file, key, lo);
   if (!im->im)
     {
	free(im);
	return NULL;
     }
   im->xcbinf = xcbinf;
   im->xcbinf->references++;
   im->cs.space = EVAS_COLORSPACE_ARGB8888;
   im->fkey = strdup(buf);
   im->file = (char *)eina_stringshare_add(file);
   if (key) im->key = (char *)eina_stringshare_add(key);
   im->w = im->im->cache_entry.w;
   im->h = im->im->cache_entry.h;
   im->references = 1;
   if (lo) im->load_opts = *lo;
   if (im->im->info.comment) im->comment = (char *)eina_stringshare_add(im->im->info.comment);
/*    if (im->im->info.format == 1) im->format = eina_stringshare_add("png"); */
   if (im->im->cache_entry.flags.alpha) im->alpha = 1;
   if (!_xr_image_hash) _xr_image_hash = eina_hash_string_superfast_new(NULL);
   eina_hash_direct_add(_xr_image_hash, im->fkey, im);
   return im;
}

XR_Image *
_xre_image_new_from_data(Xcb_Image_Info *xcbinf, int w, int h, void *data, int alpha, int cspace)
{
   XR_Image *im;

   im = calloc(1, sizeof(XR_Image));
   if (!im) return NULL;
   im->xcbinf = xcbinf;
   im->xcbinf->references++;
   im->cs.space = cspace;
   im->w = w;
   im->h = h;
   im->references = 1;
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	im->data = data;
	im->alpha = alpha;
	break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
	im->cs.data = data;
	im->cs.no_free = 1;
	break;
      default:
	abort();
	break;
     }
   im->dirty = 1;
   __xre_image_dirty_hash_add(im);
   return im;
}

XR_Image *
_xre_image_new_from_copied_data(Xcb_Image_Info *xcbinf, int w, int h, void *data, int alpha, int cspace)
{
   XR_Image *im;

   im = calloc(1, sizeof(XR_Image));
   if (!im) return NULL;
   im->cs.space = cspace;
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	im->data = malloc(w * h * 4);
	if (!im->data)
	  {
	     free(im);
	     return NULL;
	  }
	if (data)
	  {
	     Gfx_Func_Copy func;

	     func = evas_common_draw_func_copy_get(w * h, 0);
	     if (func) func(data, im->data, w * h);
	     evas_common_cpu_end_opt();
	  }
	im->alpha = alpha;
	im->free_data = 1;
        break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
        im->cs.no_free = 0;
        im->cs.data = calloc(1, h * sizeof(unsigned char *) * 2);
	if ((data) && (im->cs.data))
	  memcpy(im->cs.data, data, h * sizeof(unsigned char *) * 2);
	break;
      default:
	abort();
	break;
     }
   im->w = w;
   im->h = h;
   im->references = 1;
   im->xcbinf = xcbinf;
   im->xcbinf->references++;
   im->dirty = 1;
   __xre_image_dirty_hash_add(im);
   return im;
}

XR_Image *
_xre_image_new(Xcb_Image_Info *xcbinf, int w, int h)
{
   XR_Image *im;

   im = calloc(1, sizeof(XR_Image));
   if (!im) return NULL;
   im->data = malloc(w * h * 4);
   if (!im->data)
     {
	free(im);
	return NULL;
     }
   im->w = w;
   im->h = h;
   im->references = 1;
   im->cs.space = EVAS_COLORSPACE_ARGB8888;
   im->xcbinf = xcbinf;
   im->xcbinf->references++;
   im->free_data = 1;
   im->alpha = 1;
   im->dirty = 1;
   __xre_image_dirty_hash_add(im);
   return im;
}

static void
__xre_image_real_free(XR_Image *im)
{
   if (im->cs.data)
     {
	if (!im->cs.no_free) free(im->cs.data);
     }
   if (im->file) eina_stringshare_del(im->file);
   if (im->key) eina_stringshare_del(im->key);
   if (im->fkey) free(im->fkey);
   if (im->im) evas_cache_image_drop(&im->im->cache_entry);
   if ((im->data) && (im->dirty)) __xre_image_dirty_hash_del(im);
   if ((im->free_data) && (im->data)) free(im->data);
   if (im->surface) _xr_render_surface_free(im->surface);
   if (im->format) eina_stringshare_del(im->format);
   if (im->comment) eina_stringshare_del(im->comment);
   if (im->updates) evas_common_tilebuf_free(im->updates);
   _xr_image_info_free(im->xcbinf);
   free(im);
}

void
_xre_image_free(XR_Image *im)
{
   im->references--;
   if (im->references != 0) return;
   if (!im->dirty)
     {
	if (im->fkey)
	  eina_hash_del(_xr_image_hash, im->fkey, im);
	_xr_image_cache = eina_list_prepend(_xr_image_cache, im);
	_xr_image_cache_usage += (im->w * im->h * 4);
	_xre_image_cache_set(_xr_image_cache_size);
     }
   else
     {
	__xre_image_real_free(im);
     }
}

void
_xre_image_region_dirty(XR_Image *im, int x, int y, int w, int h)
{
   if (!im->updates)
     {
	im->updates = evas_common_tilebuf_new(im->w, im->h);
	if (im->updates) evas_common_tilebuf_set_tile_size(im->updates, 8, 8);
     }
   if (im->updates)
     evas_common_tilebuf_add_redraw(im->updates, x, y, w, h);
}

void
_xre_image_dirty(XR_Image *im)
{
   if (im->dirty) return;
   if (im->fkey)
     eina_hash_del(_xr_image_hash, im->fkey, im);
   im->dirty = 1;
   __xre_image_dirty_hash_add(im);
}

XR_Image *
_xre_image_copy(XR_Image *im)
{
   XR_Image *im2;
   void     *data = NULL;

   if (im->data) data = im->data;
   else if (im->cs.data) data = im->cs.data;
   else
     {
	if (!im->im)
          im->im = evas_common_load_image_from_file(im->file, im->key, &(im->load_opts));
	if (im->im)
	  {
             evas_cache_image_load_data(&im->im->cache_entry);
	     data = im->im->image.data;
	  }
     }
   if (!data) return NULL;
   im2 = _xre_image_new_from_copied_data(im->xcbinf, im->w, im->h, data, im->alpha, im->cs.space);
   return im2;
}

void
_xre_image_resize(XR_Image *im, int w, int h)
{
   if ((w == im->w) && (h == im->h)) return;
   if (im->surface)
     {
	Xcb_Render_Surface *old_surface;

	old_surface = im->surface;
	im->surface = _xr_render_surface_new(old_surface->xcbinf, w + 2, h + 2, old_surface->fmt, old_surface->alpha);
	_xr_render_surface_free(old_surface);
     }
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (im->data)
	  {
	     if (im->free_data)
	       {
		  if (im->data) free(im->data);
		  im->data = malloc(w * h * 4);
	       }
	  }
	else if (im->im)
	  {
	     evas_cache_image_drop(&im->im->cache_entry);
	     im->im = NULL;
	     if (im->free_data)
	       {
		  if (im->data) free(im->data);
		  im->data = malloc(w * h * 4);
	       }
	  }
	else
	  {
	     im->data = malloc(w * h * 4);
	     im->free_data = 1;
	  }
        break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
	if (im->data)
	  {
	     if (im->free_data)
	       {
		  if (im->data) free(im->data);
	       }
	     im->data = NULL;
	  }
	if (im->im)
	  {
	     evas_cache_image_drop(&im->im->cache_entry);
	     im->im = NULL;
	  }
	if (!im->cs.no_free)
	  {
	     if (im->cs.data) free(im->cs.data);
	     im->cs.data = calloc(1, h * sizeof(unsigned char *) * 2);
	  }
	break;
      default:
	abort();
	break;
     }
   __xre_image_dirty_hash_del(im);
   __xre_image_dirty_hash_add(im);
   im->w = w;
   im->h = h;
}

void *
_xre_image_data_get(XR_Image *im)
{
   void *data = NULL;

   if (im->data) data = im->data;
   else if (im->cs.data) data = im->cs.data;
   else
     {
	if (!im->im) im->im = evas_common_load_image_from_file(im->file, im->key, &(im->load_opts));
	if (im->im)
	  {
             evas_cache_image_load_data(&im->im->cache_entry);
	     data = im->im->image.data;
	  }
     }
   return data;
}

XR_Image *
_xre_image_data_find(void *data)
{
   XR_Image *im;

   im = __xre_image_dirty_hash_find(data);
   if (im)
     {
	im->references++;
     }
   return im;
}

void
_xre_image_data_put(XR_Image *im, void *data)
{
   void *imdata = NULL;

   if (!data) return;
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (im->im)
	  {
	     if (data == im->im->image.data) return;
	     evas_cache_image_drop(&im->im->cache_entry);
	     im->im = NULL;
	  }
	if (im->cs.data == data) return;
	if (im->data)
	  {
	     if (im->data == data) return;
	     if (im->free_data) free(im->data);
	     im->free_data = 0;
	  }
	im->data = data;
	im->free_data = 0;
        break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
	if (im->data)
	  {
	     if (im->free_data) free(im->data);
	     im->data = NULL;
	  }
	im->free_data = 0;
	if (data == im->cs.data) return;
	if (!im->cs.no_free)
	  {
	     if (im->cs.data) free(im->cs.data);
	  }
	im->cs.data = data;
	break;
      default:
	abort();
	break;
     }
   __xre_image_dirty_hash_del(im);
   __xre_image_dirty_hash_add(im);
   if (im->surface)
     {
	_xr_render_surface_free(im->surface);
	im->surface = NULL;
     }
   if (!im->dirty)
     {
	if (im->fkey)
	  eina_hash_del(_xr_image_hash, im->fkey, im);
	im->dirty = 1;
     }
   if (im->updates)
     {
	evas_common_tilebuf_free(im->updates);
	im->updates = NULL;
     }
}

void
_xre_image_alpha_set(XR_Image *im, int alpha)
{
   if (im->alpha == alpha) return;
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	im->alpha = alpha;
	if (im->surface)
	  {
             Xcb_Render_Surface *old_surface;

	     old_surface = im->surface;
	     im->surface = NULL;
	     if (im->alpha)
	       im->surface = _xr_render_surface_new(im->xcbinf, im->w + 2, im->h + 2, im->xcbinf->fmt32, 1);
	     else
	       {
		  /* FIXME: if im->depth == 16, use xcbinf->fmtdef */
		  if ((im->xcbinf->depth == 16) &&
		      (im->xcbinf->visual->red_mask == 0xf800) &&
		      (im->xcbinf->visual->green_mask == 0x07e0) &&
		      (im->xcbinf->visual->blue_mask == 0x001f))
		    im->surface = _xr_render_surface_new(im->xcbinf, im->w + 2, im->h + 2, im->xcbinf->fmtdef, 0);
		  else
		    im->surface = _xr_render_surface_new(im->xcbinf, im->w + 2, im->h + 2, im->xcbinf->fmt24, 0);
	       }
	     if (im->surface)
	       _xr_render_surface_copy(old_surface, im->surface, 0, 0, 0, 0, im->w + 2, im->h + 2);
	     _xr_render_surface_free(old_surface);
	  }
	if (im->updates)
	  {
	     evas_common_tilebuf_free(im->updates);
	     im->updates = NULL;
	  }
      default:
	break;
     }
}

int
_xre_image_alpha_get(XR_Image *im)
{
   if (im->im)
     {
	if (im->im->cache_entry.space != EVAS_COLORSPACE_ARGB8888) return 0;
     }
   return im->alpha;
}

void
_xre_image_border_set(XR_Image *im, int l, int r, int t, int b)
{
   if (!im) return;
   _xre_image_surface_gen(im);
   if (l < 1) l = 0;
   if (r < 1) r = 0;
   if (t < 1) t = 0;
   if (b < 1) b = 0;
   if (im->surface)
     {
	if (l | r | t | b)
	  im->surface->bordered = 1;
	else
	  im->surface->bordered = 0;
    }
}

void
_xre_image_border_get(XR_Image *im, int *l, int *r, int *t, int *b)
{
}

void
_xre_image_surface_gen(XR_Image *im)
{
   void *data = NULL;
   void *tdata = NULL;

   if ((im->surface) && (!im->updates)) return;
   if (im->data) data = im->data;
   else
     {
	if (!im->im) im->im = evas_common_load_image_from_file(im->file, im->key, &(im->load_opts));
	if (im->im)
	  {
             evas_cache_image_load_data(&im->im->cache_entry);
	     data = im->im->image.data;
	  }
     }
   if (!data)
     {
	switch (im->cs.space)
	  {
	   case EVAS_COLORSPACE_ARGB8888:
	     return;
	     break;
	   case EVAS_COLORSPACE_YCBCR422P601_PL:
	   case EVAS_COLORSPACE_YCBCR422P709_PL:
	     if ((im->cs.data) && (*((unsigned char **)im->cs.data)))
	       {
		  tdata = malloc(im->w * im->h * sizeof(DATA32));
		  if (tdata)
		    evas_common_convert_yuv_420p_601_rgba(im->cs.data,
							  tdata,
							  im->w, im->h);
		  data = tdata;
	       }
	     break;
	   default:
	     abort();
	     break;
	  }
	if (!data) return;
     }
   if (im->surface)
     {
	if (im->updates)
	  {
	     Tilebuf_Rect *rects, *r;

	     rects = evas_common_tilebuf_get_render_rects(im->updates);
	     if (rects)
	       {
		  EINA_INLIST_FOREACH(rects, r)
		    {
		       int rx, ry, rw, rh;

		       rx = r->x; ry = r->y; rw = r->w, rh = r->h;
		       RECTS_CLIP_TO_RECT(rx, ry, rw, rh, 0, 0, im->w, im->h);
		       if (im->alpha)
			 _xr_render_surface_argb_pixels_fill(im->surface, im->w, im->h, data, rx, ry, rw, rh, 1, 1);
		       else
		       /* FIXME: if im->depth == 16 - convert to 16bpp then
			* upload */
			 _xr_render_surface_rgb_pixels_fill(im->surface, im->w, im->h, data, rx, ry, rw, rh, 1, 1);
		    }
		  evas_common_tilebuf_free_render_rects(rects);
	       }
	     evas_common_tilebuf_free(im->updates);
	     im->updates = NULL;
	  }
	if (tdata) free(tdata);
	return;
     }
   if (im->alpha)
     {
	im->surface = _xr_render_surface_new(im->xcbinf, im->w + 2, im->h + 2, im->xcbinf->fmt32, 1);
	_xr_render_surface_argb_pixels_fill(im->surface, im->w, im->h, data, 0, 0, im->w, im->h, 1, 1);
     }
   else
     {
	/* FIXME: if im->xcbinf->depth == 16, use xcbinf->fmtdef */
	if ((im->xcbinf->depth == 16) &&
	    (im->xcbinf->visual->red_mask == 0xf800) &&
	    (im->xcbinf->visual->green_mask == 0x07e0) &&
	    (im->xcbinf->visual->blue_mask == 0x001f))
	  im->surface = _xr_render_surface_new(im->xcbinf, im->w + 2, im->h + 2, im->xcbinf->fmtdef, 0);
	else
	  im->surface = _xr_render_surface_new(im->xcbinf, im->w + 2, im->h + 2, im->xcbinf->fmt24, 0);
	/* FIXME: if im->depth == 16 - convert to 16bpp then
	 * upload */
	_xr_render_surface_rgb_pixels_fill(im->surface, im->w, im->h, data, 0, 0, im->w, im->h, 1, 1);
     }
   /* fill borders */
   _xr_render_surface_copy(im->surface, im->surface,
			   1, 1,
			   0, 1,
			   1, im->h);
   _xr_render_surface_copy(im->surface, im->surface,
			   0, 1,
			   0, 0,
			   im->w + 2, 1);
   _xr_render_surface_copy(im->surface, im->surface,
			   im->w, 1,
			   im->w + 1, 1,
			   1, im->h);
   _xr_render_surface_copy(im->surface, im->surface,
			   0, im->h,
			   0, im->h + 1,
			   im->w + 2, 1);
   if ((im->im) && (!im->dirty))
     {
	evas_cache_image_drop(&im->im->cache_entry);
	im->im = NULL;
     }
   if (tdata) free(tdata);
}

void
_xre_image_cache_set(int size)
{
   _xr_image_cache_size = size;
   while (_xr_image_cache_usage > _xr_image_cache_size)
     {
	Eina_List *l;

	l = eina_list_last(_xr_image_cache);
	if (l)
	  {
	     XR_Image *im;

	     im = l->data;
	     _xr_image_cache = eina_list_remove_list(_xr_image_cache, l);
	     _xr_image_cache_usage -= (im->w * im->h * 4);
	     __xre_image_real_free(im);
	  }
     }
}

int
_xre_image_cache_get(void)
{
   return _xr_image_cache_size;
}

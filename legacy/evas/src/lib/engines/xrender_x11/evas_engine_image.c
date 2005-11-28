#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "evas_engine_api_xrender_x11.h"
#include "Evas_Engine_XRender_X11.h"

static Evas_Hash *_xr_image_hash        = NULL;
static int        _xr_image_cache_size  = 0;
static int        _xr_image_cache_usage = 0;
static Evas_List *_xr_image_cache       = NULL;
static Evas_Hash *_xr_image_dirty_hash  = NULL;

static void
__xre_image_dirty_hash_add(XR_Image *im)
{
   char buf[64];
   
   if (!im->data) return;
   snprintf(buf, sizeof(buf), "%p", im->data);
   _xr_image_dirty_hash = evas_hash_add(_xr_image_dirty_hash, buf, im);
}

static void
__xre_image_dirty_hash_del(XR_Image *im)
{
   char buf[64];
   
   if (!im->data) return;
   snprintf(buf, sizeof(buf), "%p", im->data);
   _xr_image_dirty_hash = evas_hash_del(_xr_image_dirty_hash, buf, im);
}

static XR_Image *
__xre_image_dirty_hash_find(void *data)
{
   char buf[64];
   
   snprintf(buf, sizeof(buf), "%p", data);
   return evas_hash_find(_xr_image_dirty_hash, buf);
}

static XR_Image *
__xre_image_find(char *fkey)
{
   XR_Image *im;
   
   im = evas_hash_find(_xr_image_hash, fkey);
   if (!im)
     {
	Evas_List *l;
	
	for (l = _xr_image_cache; l; l = l->next)
	  {
	     im = l->data;
	     if (!strcmp(im->fkey, fkey))
	       {
		  _xr_image_cache = evas_list_remove_list(_xr_image_cache, l);
		  _xr_image_hash = evas_hash_add(_xr_image_hash, im->fkey, im);
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
_xre_image_load(Ximage_Info *xinf, char *file, char *key)
{
   XR_Image *im;
   char buf[4096];

   if (!file) return NULL;
   if (key)
     snprintf(buf, sizeof(buf), "/@%p@%lx@/%s//://%s", xinf->disp, xinf->root, file, key);
   else
     snprintf(buf, sizeof(buf), "/@%p@%lx@/%s", xinf->disp, xinf->root, file);
   im = __xre_image_find(buf);
   if (im)
     {
	return im;
     }
   
   im = calloc(1, sizeof(XR_Image));
   if (!im) return NULL;
   im->im = evas_common_load_image_from_file(file, key);
   if (!im->im)
     {
	free(im);
	return NULL;
     }
   im->xinf = xinf;
   im->xinf->references++;
   im->fkey = strdup(buf);
   im->file = evas_stringshare_add(file);
   if (key) im->key = evas_stringshare_add(key);
   im->w = im->im->image->w;
   im->h = im->im->image->h;
   im->references = 1;
   if (im->im->info.comment) im->comment = evas_stringshare_add(im->im->info.comment);
   if (im->im->info.format == 1) im->format = evas_stringshare_add("png");
   if (im->im->flags & RGBA_IMAGE_HAS_ALPHA) im->alpha = 1;
   _xr_image_hash = evas_hash_direct_add(_xr_image_hash, im->fkey, im);
   return im;
}

XR_Image *
_xre_image_new_from_data(Ximage_Info *xinf, int w, int h, void *data)
{
   XR_Image *im;

   im = calloc(1, sizeof(XR_Image));
   if (!im) return NULL;
   im->xinf = xinf;
   im->xinf->references++;
   im->w = w;
   im->h = h;
   im->references = 1;
   im->data = data;
   im->alpha = 1;
   im->dirty = 1;
   __xre_image_dirty_hash_add(im);
   return im;
}

XR_Image *
_xre_image_new_from_copied_data(Ximage_Info *xinf, int w, int h, void *data)
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
   if (data)
     {
	Gfx_Func_Blend_Src_Dst func;
	
	func = evas_common_draw_func_copy_get(w * h, 0);
	if (func) func(data, im->data, w * h);
	evas_common_cpu_end_opt();
     }
   im->w = w;
   im->h = h;
   im->references = 1;
   im->xinf = xinf;
   im->xinf->references++;
   im->free_data = 1;
   im->alpha = 1;
   im->dirty = 1;
   __xre_image_dirty_hash_add(im);
   return im;
}

XR_Image *
_xre_image_new(Ximage_Info *xinf, int w, int h)
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
   im->xinf = xinf;
   im->xinf->references++;
   im->free_data = 1;
   im->alpha = 1;
   im->dirty = 1;
   __xre_image_dirty_hash_add(im);
   return im;
}

static void
__xre_image_real_free(XR_Image *im)
{
   if (im->file) evas_stringshare_del(im->file);
   if (im->key) evas_stringshare_del(im->key);
   if (im->fkey) free(im->fkey);
   if (im->im) evas_common_image_unref(im->im);
   if ((im->data) && (im->dirty)) __xre_image_dirty_hash_del(im);
   if ((im->free_data) && (im->data)) free(im->data);
   if (im->surface) _xr_render_surface_free(im->surface);
   if (im->format) evas_stringshare_del(im->format);
   if (im->comment) evas_stringshare_del(im->comment);
   if (im->updates) evas_common_tilebuf_free(im->updates);
   _xr_image_info_free(im->xinf);
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
	  _xr_image_hash = evas_hash_del(_xr_image_hash, im->fkey, im);
	_xr_image_cache = evas_list_prepend(_xr_image_cache, im);
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
     _xr_image_hash = evas_hash_del(_xr_image_hash, im->fkey, im);
   im->dirty = 1;
   __xre_image_dirty_hash_add(im);
}

XR_Image *
_xre_image_copy(XR_Image *im)
{
   XR_Image *im2;
   void *data = NULL;

   if (im->data) data = im->data;
   else
     {
	if (!im->im) im->im = evas_common_load_image_from_file(im->file, im->key);
	if (im->im)
	  {
	     evas_common_load_image_data_from_file(im->im);
	     data = im->im->image->data;
	  }
     }
   if (!data) return NULL;
   im2 = _xre_image_new_from_copied_data(im->xinf, im->w, im->h, data);
   if (im2) im2->alpha = im->alpha;
   if ((im->im) && (!im->dirty))
     {
	evas_common_image_unref(im->im);
	im->im = NULL;
     }
   return im2;
}

void
_xre_image_resize(XR_Image *im, int w, int h)
{
   if ((w == im->w) && (h == im->h)) return;
   if (im->surface)
     {
	Xrender_Surface *old_surface;
	int x = 0, y = 0, ww, hh;
	
	ww = w; hh = h;
	RECTS_CLIP_TO_RECT(x, y, ww, hh, 0, 0, im->w, im->h);
	old_surface = im->surface;
	im->surface = _xr_render_surface_new(old_surface->xinf, w, h, old_surface->fmt, old_surface->alpha);
	if (im->surface)
	  _xr_render_surface_copy(old_surface, im->surface, 0, 0, 0, 0, ww, hh);
	_xr_render_surface_free(old_surface);
     }
   if (im->data)
     {
	Gfx_Func_Blend_Src_Dst func;
	int x = 0, y = 0, ww, hh;
	unsigned int *sp, *dp;
	void *data;
	
	data = malloc(w * h * 4);
	if (!data)
	  {
	     if (im->surface)
	       {
		  _xr_render_surface_free(im->surface);
		  im->surface = NULL;
	       }
	     return;
	  }
	ww = w; hh = h;
	
	RECTS_CLIP_TO_RECT(x, y, ww, hh, 0, 0, im->w, im->h);
	func = evas_common_draw_func_copy_get(w * h, 0);
	if (func)
	  {
	     for (y = 0; y < hh; y++)
	       {
		  sp = ((unsigned int *)im->data) + (y * im->w);
		  dp = ((unsigned int *)data) + (y * w);
		  func(sp, dp, ww);
	       }
	     evas_common_cpu_end_opt();
	  }
	__xre_image_dirty_hash_del(im);
	free(im->data);
	im->data = data;
	__xre_image_dirty_hash_add(im);
     }
   else if (im->im)
     {
	RGBA_Image *im_old;
	
	im_old = im->im;
	im->im = evas_common_image_create(w, h);
	if (!im->im)
	  {
	     im->im = im_old;
	     if (im->surface)
	       {
		  _xr_render_surface_free(im->surface);
		  im->surface = NULL;
	       }
	     return;
	  }
	evas_common_load_image_data_from_file(im_old);
	if (im_old->image->data)
	  {
	     int x = 0, y = 0, ww, hh;
	     
	     ww = w; hh = h;
	     RECTS_CLIP_TO_RECT(x, y, ww, hh, 0, 0, im->w, im->h);
             evas_common_blit_rectangle(im_old, im->im, 0, 0, ww, hh, 0, 0);
	     evas_common_cpu_end_opt();
	  }
	im->free_data = 1;
	im->data = im->im->image->data;
	im->im->image->data = NULL;
        evas_common_image_unref(im->im);
	im->im = NULL;
        evas_common_image_unref(im_old);
	__xre_image_dirty_hash_add(im);
     }
   else
     {
	im->data = malloc(w * h * 4);
	im->free_data = 1;
	__xre_image_dirty_hash_add(im);
     }
   im->w = w;
   im->h = h;
}

void *
_xre_image_data_get(XR_Image *im)
{
   void *data = NULL;
   
   if (im->data) data = im->data;
   else
     {
	if (!im->im) im->im = evas_common_load_image_from_file(im->file, im->key);
	if (im->im)
	  {
	     evas_common_load_image_data_from_file(im->im);
	     data = im->im->image->data;
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
   if (im->data)
     {
	imdata = im->data;
	if (data == imdata) return;
	__xre_image_dirty_hash_del(im);
	if (im->free_data) free(im->data);
     }
   else
     {
	if (im->im) imdata = im->im->image->data;
	if (data == imdata) return;
	if (im->im)
	  {
	     evas_common_image_unref(im->im);
	     im->im = NULL;
	  }
     }
   im->data = data;
   __xre_image_dirty_hash_add(im);
   im->free_data = 0;
   if (im->surface)
     {
	_xr_render_surface_free(im->surface);
	im->surface = NULL;
     }
   if (!im->dirty)
     {
	if (im->fkey)
	  _xr_image_hash = evas_hash_del(_xr_image_hash, im->fkey, im);
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
   im->alpha = alpha;
   if (im->surface)
     {
	Xrender_Surface *old_surface;
	
	old_surface = im->surface;
	im->surface = NULL;
	if (im->alpha)
	  im->surface = _xr_render_surface_new(im->xinf, im->w, im->h, im->xinf->fmt32, 1);
	else
	  im->surface = _xr_render_surface_new(im->xinf, im->w, im->h, im->xinf->fmt24, 0);
	if (im->surface)
	  _xr_render_surface_copy(old_surface, im->surface, 0, 0, 0, 0, im->w, im->h);
	_xr_render_surface_free(old_surface);
     }
   if (im->updates)
     {
	evas_common_tilebuf_free(im->updates);
	im->updates = NULL;
     }
}

int
_xre_image_alpha_get(XR_Image *im)
{
   return im->alpha;
}

void
_xre_image_surface_gen(XR_Image *im)
{
   void *data = NULL;

   if ((im->surface) && (!im->updates)) return;
   if (im->data) data = im->data;
   else
     {
	if (!im->im) im->im = evas_common_load_image_from_file(im->file, im->key);
	if (im->im)
	  {
	     evas_common_load_image_data_from_file(im->im);
	     data = im->im->image->data;
	  }
     }
   if (!data) return;
   if (im->surface)
     {
	if (im->updates)
	  {
	     Tilebuf_Rect *rects, *r;
	     
	     rects = evas_common_tilebuf_get_render_rects(im->updates);
	     if (rects)
	       {
		  for (r = rects; r; r = (Tilebuf_Rect *)((Evas_Object_List *)r)->next)
		    {
		       int rx, ry, rw, rh;
		       
		       rx = r->x; ry = r->y; rw = r->w, rh = r->h;
		       RECTS_CLIP_TO_RECT(rx, ry, rw, rh, 0, 0, im->w, im->h);
		       if (im->alpha)
			 _xr_render_surface_argb_pixels_fill(im->surface, im->w, im->h, data, rx, ry, rw, rh);
		       else
			 _xr_render_surface_rgb_pixels_fill(im->surface, im->w, im->h, data, rx, ry, rw, rh);
		    }
		  evas_common_tilebuf_free_render_rects(rects);
	       }
	     evas_common_tilebuf_free(im->updates);
	     im->updates = NULL;
	  }
	return;
     }
   if (im->alpha)
     {
	im->surface = _xr_render_surface_new(im->xinf, im->w, im->h, im->xinf->fmt32, 1);
	_xr_render_surface_argb_pixels_fill(im->surface, im->w, im->h, data, 0, 0, im->w, im->h);
     }
   else
     {
	im->surface = _xr_render_surface_new(im->xinf, im->w, im->h, im->xinf->fmt24, 0);
	_xr_render_surface_rgb_pixels_fill(im->surface, im->w, im->h, data, 0, 0, im->w, im->h);
     }
   if ((im->im) && (!im->dirty))
     {
	evas_common_image_unref(im->im);
	im->im = NULL;
     }
}

void
_xre_image_cache_set(int size)
{
   _xr_image_cache_size = size;
   while (_xr_image_cache_usage > _xr_image_cache_size)
     {
	Evas_List *l;
	
	l = evas_list_last(_xr_image_cache);
	if (l)
	  {
	     XR_Image *im;
	     
	     im = l->data;
	     _xr_image_cache = evas_list_remove_list(_xr_image_cache, l);
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

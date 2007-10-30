#include "evas_soft16.h"

static inline int
_calc_stride(int w)
{
   int pad;

   pad = w % 4;
   if (!pad)  return w;
   else return w + 4 - pad;
}

static void
soft16_image_alloc_pixels(Soft16_Image *im)
{
   if (im->pixels) return;

   im->pixels = malloc(IMG_BYTE_SIZE(im->stride, im->h, im->have_alpha));
   if (!im->pixels) return;
   if (im->have_alpha) im->alpha = (DATA8 *)(im->pixels + (im->stride * im->h));
   im->free_pixels = 1;
}

static Soft16_Image *
soft16_image_alloc(int w, int h, int stride, int have_alpha, int have_pixels)
{
   Soft16_Image *im;

   im = calloc(1, sizeof(Soft16_Image));
   if (!im) return NULL;

   im->w = w;
   im->h = h;
   im->stride = stride;
   im->have_alpha = have_alpha;
   im->references = 1;
   if (!have_pixels) return im;

   soft16_image_alloc_pixels(im);
   if (!im->pixels)
     {
	free(im);
	return NULL;
     }

   return im;
}

Soft16_Image *
soft16_image_new(int w, int h, int stride, int have_alpha, DATA16 *pixels,
		 int copy)
{
   Soft16_Image *im;

   if (stride < 0) stride = _calc_stride(w);

   im = soft16_image_alloc(w, h, stride, have_alpha, copy);
   if (!im) return NULL;

   if (pixels)
     {
	if (copy)
	  memcpy(im->pixels, pixels, IMG_BYTE_SIZE(stride, h, have_alpha));
	else
	  {
	     im->pixels = pixels;
	     if (have_alpha) im->alpha = (DATA8 *)(im->pixels + (stride * h));
	  }
     }
   return im;
}

static void
soft16_image_cache_key(const Evas_Image_Load_Opts *lo, const char *key,
		       const char *file, char *buf, unsigned bufsize)
{
   if ((!lo) ||
       ((lo->scale_down_by == 0) && (lo->dpi == 0.0) &&
	((lo->w == 0) || (lo->h == 0))))
     {
	if (key) snprintf(buf, bufsize, "%s//://%s", file, key);
	else strncpy(buf, file, bufsize);
     }
   else
     {
	if (key)
	   snprintf(buf, bufsize, "//@/%i/%1.5f/%ix%i//%s//://%s",
		    lo->scale_down_by, lo->dpi, lo->w, lo->h,
		    file, key);
	else
	   snprintf(buf, bufsize, "//@/%i/%1.5f/%ix%i//%s",
		    lo->scale_down_by, lo->dpi, lo->w, lo->h,
		    file);
     }
}

static inline void
soft16_image_cache_key_from_img(const Soft16_Image *im, char *buf,
				unsigned bufsize)
{
   soft16_image_cache_key(&im->lo, im->key, im->file, buf, bufsize);
}

void
soft16_image_free(Soft16_Image *im)
{
   if (!im)
     return;

   im->references--;
   if (im->references > 0)
     return;

   if (im->cache_key)
     soft16_image_cache_put(im);
   else
     soft16_image_destroy(im);
}

void
soft16_image_destroy(Soft16_Image *im)
{
   if (im->file) evas_stringshare_del(im->file);
   if (im->key) evas_stringshare_del(im->key);
   if (im->free_pixels) free(im->pixels);
   free(im);
}

static Soft16_Image *
soft16_image_load_new(const char *file, const char *key,
		      Evas_Image_Load_Opts *lo)
{
   Soft16_Image *im;
   RGBA_Image *sim;
   int have_alpha;

   sim = evas_common_load_image_from_file(file, key, lo);
   if (!sim) return NULL;

   have_alpha = !!(sim->flags & RGBA_IMAGE_HAS_ALPHA);
   im = soft16_image_alloc(sim->image->w, sim->image->h,
			   _calc_stride(sim->image->w), have_alpha, 0);
   if (!im)
     {
        evas_cache_image_drop(sim);
	return NULL;
     }

   im->source_im = sim;
   im->timestamp = im->source_im->timestamp;
   im->laststat = im->source_im->laststat;

   if (lo) im->lo = *lo;
   if (file) im->file = evas_stringshare_add(file);
   if (key) im->key = evas_stringshare_add(key);

   return im;
}

Soft16_Image *
soft16_image_load(const char *file, const char *key, int *error,
		  Evas_Image_Load_Opts *lo)
{
   Soft16_Image *im;
   char buf[4096 + 1024];

   *error = 0;
   if (!file)
     return NULL;

   soft16_image_cache_key(lo, key, file, buf, sizeof(buf));
   im = soft16_image_cache_get(buf);
   if (im)
     return im;

   im = soft16_image_load_new(file, key, lo);
   if (im)
     soft16_image_cache_add(im, buf);

   return im;
}

static void
_soft16_image_rgba32_import(Soft16_Image *im, DATA32 *src)
{
   DATA32 *sp;
   DATA16 *dpl;

   sp = src;
   dpl = im->pixels;
   if (im->alpha)
     soft16_image_convert_from_rgba(im, src);
   else
     soft16_image_convert_from_rgb(im, src);
}

void
soft16_image_load_data(Soft16_Image *im)
{
   if (!im) return;
   if (im->pixels) return;
   if (!im->source_im) return;

   evas_common_load_image_data_from_file(im->source_im);
   if (im->source_im->image->data)
     {
	if (!im->pixels) soft16_image_alloc_pixels(im);
	if (im->pixels) _soft16_image_rgba32_import(im, im->source_im->image->data);
     }
   evas_cache_image_drop(im->source_im);
   im->source_im = NULL;
}

static inline void
_get_clip(const RGBA_Draw_Context *dc, const Soft16_Image *im,
	  Evas_Rectangle *clip)
{
   if (dc->clip.use)
     {
	clip->x = dc->clip.x;
	clip->y = dc->clip.y;
	clip->w = dc->clip.w;
	clip->h = dc->clip.h;
	if (clip->x < 0)
	  {
	     clip->w += clip->x;
	     clip->x = 0;
	  }
	if (clip->y < 0)
	  {
	     clip->h += clip->y;
	     clip->y = 0;
	  }
	if ((clip->x + clip->w) > im->w) clip->w = im->w - clip->x;
	if ((clip->y + clip->h) > im->h) clip->h = im->h - clip->y;
     }
   else
     {
	clip->x = 0;
	clip->y = 0;
	clip->w = im->w;
	clip->h = im->h;
     }
}

static inline int
_is_empty_rectangle(const Evas_Rectangle *r)
{
   return (r->w < 1) || (r->h < 1);
}

static inline void
_shrink(int *s_pos, int *s_size, int pos, int size)
{
   int d;

   d = (*s_pos) - pos;
   if (d < 0)
     {
	(*s_size) += d;
	(*s_pos) = pos;
     }

   d = size + pos - (*s_pos);
   if ((*s_size) > d)
     (*s_size) = d;
}

static int
_soft16_adjust_areas(Evas_Rectangle *src,
		     int src_max_x, int src_max_y,
		     Evas_Rectangle *dst,
		     int dst_max_x, int dst_max_y,
		     Evas_Rectangle *dst_clip)
{
   if (_is_empty_rectangle(src) ||
       _is_empty_rectangle(dst) ||
       _is_empty_rectangle(dst_clip))
     return 0;

   /* shrink clip */
   _shrink(&dst_clip->x, &dst_clip->w, dst->x, dst->w);
   _shrink(&dst_clip->y, &dst_clip->h, dst->y, dst->h);
   if (_is_empty_rectangle(dst_clip)) return 0;

   /* sanitise x */
   if (src->x < 0)
     {
	dst->x -= (src->x * dst->w) / src->w;
	dst->w += (src->x * dst->w) / src->w;
	src->w += src->x;
	src->x = 0;
     }
   if (src->x >= src_max_x) return 0;
   if ((src->x + src->w) > src_max_x)
     {
	dst->w = (dst->w * (src_max_x - src->x)) / (src->w);
	src->w = src_max_x - src->x;
     }
   if (dst->w <= 0) return 0;
   if (src->w <= 0) return 0;
   if (dst_clip->x < 0)
     {
	dst_clip->w += dst_clip->x;
	dst_clip->x = 0;
     }
   if (dst_clip->w <= 0) return 0;
   if (dst_clip->x >= dst_max_x) return 0;

   _shrink(&dst_clip->x, &dst_clip->w, dst->x, dst_max_x);
   if (dst_clip->w <= 0) return 0;

   /* sanitise y */
   if (src->y < 0)
     {
	dst->y -= (src->y * dst->h) / src->h;
	dst->h += (src->y * dst->h) / src->h;
	src->h += src->y;
	src->y = 0;
     }
   if (src->y >= src_max_y) return 0;
   if ((src->y + src->h) > src_max_y)
     {
	dst->h = (dst->h * (src_max_y - src->y)) / (src->h);
	src->h = src_max_y - src->y;
     }
   if (dst->h <= 0) return 0;
   if (src->h <= 0) return 0;
   if (dst_clip->y < 0)
     {
	dst_clip->h += dst_clip->y;
	dst_clip->y = 0;
     }
   if (dst_clip->h <= 0) return 0;
   if (dst_clip->y >= dst_max_y) return 0;

   _shrink(&dst_clip->y, &dst_clip->h, dst->y, dst_max_y);
   if (dst_clip->h <= 0) return 0;

   return 1;
}

static void
_soft16_image_draw_sampled_int(Soft16_Image *src, Soft16_Image *dst,
			       RGBA_Draw_Context *dc,
			       Evas_Rectangle sr, Evas_Rectangle dr)
{
   Evas_Rectangle cr;

   if (!(RECTS_INTERSECT(dr.x, dr.y, dr.w, dr.h, 0, 0, dst->w, dst->h)))
     return;
   if (!(RECTS_INTERSECT(sr.x, sr.y, sr.w, sr.h, 0, 0, src->w, src->h)))
     return;

   _get_clip(dc, dst, &cr);
   if (!_soft16_adjust_areas(&sr, src->w, src->h, &dr, dst->w, dst->h, &cr))
     return;

   if ((dr.w == sr.w) && (dr.h == sr.h))
     soft16_image_draw_unscaled(src, dst, dc, sr, dr, cr);
   else
     soft16_image_draw_scaled_sampled(src, dst, dc, sr, dr, cr);
}

void
soft16_image_draw(Soft16_Image *src, Soft16_Image *dst,
		  RGBA_Draw_Context *dc,
		  int src_region_x, int src_region_y,
		  int src_region_w, int src_region_h,
		  int dst_region_x, int dst_region_y,
		  int dst_region_w, int dst_region_h,
		  int smooth)
{
   Evas_Rectangle sr, dr;
   Cutout_Rects *rects;
   Cutout_Rect  *r;
   struct RGBA_Draw_Context_clip clip_bkp;
   int i;

   /* handle cutouts here! */
   dr.x = dst_region_x;
   dr.y = dst_region_y;
   dr.w = dst_region_w;
   dr.h = dst_region_h;

   if (_is_empty_rectangle(&dr)) return;
   if (!(RECTS_INTERSECT(dr.x, dr.y, dr.w, dr.h, 0, 0, dst->w, dst->h)))
     return;

   sr.x = src_region_x;
   sr.y = src_region_y;
   sr.w = src_region_w;
   sr.h = src_region_h;

   if (_is_empty_rectangle(&sr)) return;
   if (!(RECTS_INTERSECT(sr.x, sr.y, sr.w, sr.h, 0, 0, src->w, src->h)))
     return;

   /* no cutouts - cut right to the chase */
   if (!dc->cutout.rects)
     {
	_soft16_image_draw_sampled_int(src, dst, dc, sr, dr);
	return;
     }

   /* save out clip info */
   clip_bkp = dc->clip;
   evas_common_draw_context_clip_clip(dc, 0, 0, dst->w, dst->h);
   evas_common_draw_context_clip_clip(dc, dst_region_x, dst_region_y, dst_region_w, dst_region_h);
   /* our clip is 0 size.. abort */
   if ((dc->clip.w <= 0) || (dc->clip.h <= 0))
     {
	dc->clip = clip_bkp;
	return;
     }
   rects = evas_common_draw_context_apply_cutouts(dc);
   for (i = 0; i < rects->active; i++)
     {
	r = rects->rects + i;
	evas_common_draw_context_set_clip(dc, r->x, r->y, r->w, r->h);
	_soft16_image_draw_sampled_int(src, dst, dc, sr, dr);
     }
   evas_common_draw_context_apply_clear_cutouts(rects);
   dc->clip = clip_bkp;
}

Soft16_Image *
soft16_image_alpha_set(Soft16_Image *im, int have_alpha)
{
   if (im->have_alpha == have_alpha) return im;
   im->have_alpha = have_alpha;

   if ((im->pixels) && (im->free_pixels) && (im->references == 1))
     {
	int size;

	size = im->stride * im->h;
	if (!im->have_alpha)
	  {
	     im->pixels = realloc(im->pixels, size * 2);
	     im->alpha = NULL;
	  }
	else
	  {
	     im->pixels = realloc(im->pixels, size * 3);
	     im->alpha = (DATA8*)(im->pixels + size);
	     memset(im->alpha, 0x1f, size);
	  }

	if (im->cache_key)
	  soft16_image_cache_del(im);
	return im;
     }
   else
     {
	Soft16_Image *im_new;

	im_new = soft16_image_new(im->w, im->h, im->stride, have_alpha,
				  im->pixels, 1);
	soft16_image_free(im);
	return im_new;
     }
}

Soft16_Image *
soft16_image_size_set(Soft16_Image *old_im, int w, int h)
{
   Soft16_Image *new_im;
   DATA16 *dp, *sp;
   int i, cw, ch, ew;

   if ((old_im->w == w) && (old_im->h == h)) return old_im;

   new_im = soft16_image_new(w, h, -1, old_im->have_alpha, NULL, 1);

   if (old_im->w < new_im->w)
     cw = old_im->w;
   else
     cw = new_im->w;

   ew = new_im->w - cw;

   if (old_im->h < new_im->h)
     ch = old_im->h;
   else
     ch = new_im->h;

   dp = new_im->pixels;
   sp = old_im->pixels;
   for (i = 0; i < ch; i++)
     {
	memcpy(dp, sp, cw * sizeof(DATA16));
	if (ew > 0) memset(dp, 0, ew * sizeof(DATA16));

	dp += new_im->stride;
	sp += old_im->stride;
     }

   if (old_im->have_alpha)
     {
	DATA8 *dp, *sp;

	dp = new_im->alpha;
	sp = old_im->alpha;
	for (i = 0; i < ch; i++)
	  {
	     memcpy(dp, sp, cw * sizeof(DATA8));
	     if (ew > 0) memset(dp, 0, ew * sizeof(DATA8));

	     dp += new_im->stride;
	     sp += old_im->stride;
	  }
     }

   soft16_image_free(old_im);
   return new_im;
}

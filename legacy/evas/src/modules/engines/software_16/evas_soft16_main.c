#include "evas_soft16.h"
#include "evas_soft16_scanline_blend.c"

#define IMG_BYTE_SIZE(stride, height, has_alpha)                       \
   ((stride) * (height) * (!(has_alpha) ? 2 : 3))

static Evas_Hash *_soft16_image_cache_hash = NULL;

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

   im = soft16_image_alloc(w, h, stride, have_alpha, copy);
   if (!im) return NULL;

   if (copy) memcpy(im->pixels, pixels, IMG_BYTE_SIZE(stride, h, have_alpha));
   else
     {
	im->pixels = pixels;
	if (have_alpha) im->alpha = (DATA8 *)(im->pixels + (stride * h));
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
   if (!im) return;
   im->references--;
   if (im->references > 0) return;
   if (im->file)
     {
	char buf[4096 + 1024];
	soft16_image_cache_key_from_img(im, buf, sizeof(buf));
	_soft16_image_cache_hash = evas_hash_del(_soft16_image_cache_hash,
						 buf, im);
     }
   if (im->file) evas_stringshare_del(im->file);
   if (im->key) evas_stringshare_del(im->key);
   if (im->free_pixels) free(im->pixels);
   free(im);
}

#define STAT_GAP 2

static Soft16_Image *
soft16_image_cache_get(const char *cache_key)
{
   Soft16_Image *im;

   im = evas_hash_find(_soft16_image_cache_hash, cache_key);
   if (im)
     {
	time_t t;

	t = time(NULL);
	if ((t - im->laststat) > STAT_GAP)
	  {
	     struct stat st;

	     if (stat(im->file, &st) < 0) return NULL;
	     if (st.st_mtime != im->timestamp) return NULL;

	     im->laststat = t;
	  }
	im->references++;
     }

   return im;
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
	evas_common_image_unref(sim);
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
   soft16_image_cache_key(lo, key, file, buf, sizeof(buf));
   im = soft16_image_cache_get(buf);
   if (im) return im;

   im = soft16_image_load_new(file, key, lo);
   if (im) _soft16_image_cache_hash = evas_hash_add(_soft16_image_cache_hash, buf, im);

   return im;
}

static void
_soft16_image_rgba32_import(Soft16_Image *im, DATA32 *src)
{
   DATA32 *sp;
   DATA16 *dpl;

   /* FIXME: dither and optimize */
   sp = src;
   dpl = im->pixels;
   if (im->alpha)
     {
	DATA8 *dal;
	dal = im->alpha;
	int y;

	for (y = 0; y < im->h; y++)
	  {
	     DATA16 *dp, *dp_end;
	     DATA8 *da;

	     dp = dpl;
	     dp_end = dp + im->w;
	     da = dal;

	     for (; dp < dp_end; da++, dp++, sp++)
	       {
		  *da = A_VAL(sp) >> 3;
		  *dp = RGB_565_FROM_COMPONENTS(R_VAL(sp), G_VAL(sp), B_VAL(sp));
	       }

	     dpl += im->stride;
	     dal += im->stride;
	  }
     }
   else
     {
	int y;

	for (y = 0; y < im->h; y++)
	  {
	     DATA16 *dp, *dp_end;

	     dp = dpl;
	     dp_end = dp + im->w;

	     for (; dp < dp_end; dp++, sp++)
		*dp = RGB_565_FROM_COMPONENTS(R_VAL(sp), G_VAL(sp), B_VAL(sp));

	     dpl += im->stride;
	  }
     }
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
   evas_common_image_unref(im->source_im);
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
_soft16_image_draw_unscaled_solid_solid(Soft16_Image *src, Soft16_Image *dst,
					RGBA_Draw_Context *dc,
					int src_offset, int dst_offset,
					int w, int h)
{
   DATA16 *src_itr, *dst_itr;
   int y;

   src_itr = src->pixels + src_offset;
   dst_itr = dst->pixels + dst_offset;

   for (y = 0; y < h; y++)
     {
	_soft16_scanline_blend_solid_solid(src_itr, dst_itr, w);
	src_itr += src->stride;
	dst_itr += dst->stride;
     }
}

static void
_soft16_image_draw_unscaled_transp_solid(Soft16_Image *src, Soft16_Image *dst,
					 RGBA_Draw_Context *dc,
					 int src_offset, int dst_offset,
					 int w, int h)

{
   DATA16 *src_itr, *dst_itr;
   DATA8 *alpha_itr;
   int y;

   src_itr = src->pixels + src_offset;
   alpha_itr = src->alpha + src_offset;
   dst_itr = dst->pixels + dst_offset;

   for (y = 0; y < h; y++)
     {
	_soft16_scanline_blend_transp_solid(src_itr, alpha_itr, dst_itr, w);
	src_itr += src->stride;
	alpha_itr += src->stride;
	dst_itr += dst->stride;
     }
}

static inline void
_soft16_image_draw_unscaled_no_mul(Soft16_Image *src, Soft16_Image *dst,
                                   RGBA_Draw_Context *dc,
                                   int src_offset, int dst_offset,
                                   int width, int height)
{
   if (src->have_alpha && (!dst->have_alpha))
      _soft16_image_draw_unscaled_transp_solid(src, dst, dc,
                                               src_offset, dst_offset,
                                               width, height);
   else if ((!src->have_alpha) && (!dst->have_alpha))
      _soft16_image_draw_unscaled_solid_solid(src, dst, dc,
                                              src_offset, dst_offset,
                                              width, height);
   else
      fprintf(stderr,
              "Unsupported draw of unscaled images src->have_alpha=%d, "
              "dst->have_alpha=%d, WITHOUT COLOR MUL\n",
              src->have_alpha, dst->have_alpha);
}

static void
_soft16_image_draw_unscaled_solid_solid_mul_alpha(Soft16_Image *src,
                                                  Soft16_Image *dst,
                                                  RGBA_Draw_Context *dc,
                                                  int src_offset,
                                                  int dst_offset,
                                                  int w, int h)
{
   DATA16 *src_itr, *dst_itr;
   int y, rel_alpha;

   src_itr = src->pixels + src_offset;
   dst_itr = dst->pixels + dst_offset;

   rel_alpha = A_VAL(&dc->mul.col) >> 3;
   if ((rel_alpha < 1) || (rel_alpha > 31)) return;
   rel_alpha = 31 - rel_alpha;

   for (y = 0; y < h; y++)
     {
	_soft16_scanline_blend_solid_solid_mul_alpha(src_itr, dst_itr, w,
                                                     rel_alpha);
	src_itr += src->stride;
	dst_itr += dst->stride;
     }
}

static void
_soft16_image_draw_unscaled_transp_solid_mul_alpha(Soft16_Image *src,
                                                   Soft16_Image *dst,
                                                   RGBA_Draw_Context *dc,
                                                   int src_offset,
                                                   int dst_offset,
                                                   int w, int h)

{
   DATA16 *src_itr, *dst_itr;
   DATA8 *alpha_itr;
   int y, rel_alpha;

   src_itr = src->pixels + src_offset;
   alpha_itr = src->alpha + src_offset;
   dst_itr = dst->pixels + dst_offset;

   rel_alpha = A_VAL(&dc->mul.col) >> 3;
   if ((rel_alpha < 1) || (rel_alpha > 31)) return;
   rel_alpha = 31 - rel_alpha;

   for (y = 0; y < h; y++)
     {
	_soft16_scanline_blend_transp_solid_mul_alpha(src_itr, alpha_itr,
                                                      dst_itr, w, rel_alpha);
	src_itr += src->stride;
	alpha_itr += src->stride;
	dst_itr += dst->stride;
     }
}

static inline void
_soft16_image_draw_unscaled_mul_alpha(Soft16_Image *src, Soft16_Image *dst,
                                      RGBA_Draw_Context *dc,
                                      int src_offset, int dst_offset,
                                      int width, int height)
{
   if (src->have_alpha && (!dst->have_alpha))
      _soft16_image_draw_unscaled_transp_solid_mul_alpha
         (src, dst, dc, src_offset, dst_offset, width, height);
   else if ((!src->have_alpha) && (!dst->have_alpha))
      _soft16_image_draw_unscaled_solid_solid_mul_alpha
         (src, dst, dc, src_offset, dst_offset, width, height);
   else
      fprintf(stderr,
              "Unsupported draw of unscaled images src->have_alpha=%d, "
              "dst->have_alpha=%d, WITH ALPHA MUL %d\n",
              src->have_alpha, dst->have_alpha, A_VAL(&dc->mul.col));
}

static void
_soft16_image_draw_unscaled_solid_solid_mul_color(Soft16_Image *src,
                                                  Soft16_Image *dst,
                                                  RGBA_Draw_Context *dc,
                                                  int src_offset,
                                                  int dst_offset,
                                                  int w, int h)
{
   DATA16 *src_itr, *dst_itr;
   int y, rel_alpha, r, g, b;

   src_itr = src->pixels + src_offset;
   dst_itr = dst->pixels + dst_offset;

   rel_alpha = A_VAL(&dc->mul.col) >> 3;
   if ((rel_alpha < 1) || (rel_alpha > 31)) return;

   r = R_VAL(&dc->mul.col);
   g = G_VAL(&dc->mul.col);
   b = B_VAL(&dc->mul.col);
   /* we'll divide by 256 to make it faster, try to improve things a bit */
   if (r > 127) r++;
   if (g > 127) g++;
   if (b > 127) b++;

   if (rel_alpha == 31)
      for (y = 0; y < h; y++)
         {
            _soft16_scanline_blend_solid_solid_mul_color_solid
               (src_itr, dst_itr, w, r, g, b);
            src_itr += src->stride;
            dst_itr += dst->stride;
         }
   else
      for (y = 0; y < h; y++)
         {
            _soft16_scanline_blend_solid_solid_mul_color_transp
               (src_itr, dst_itr, w, rel_alpha, r, g, b);
            src_itr += src->stride;
            dst_itr += dst->stride;
         }
}

static void
_soft16_image_draw_unscaled_transp_solid_mul_color(Soft16_Image *src,
                                                   Soft16_Image *dst,
                                                   RGBA_Draw_Context *dc,
                                                   int src_offset,
                                                   int dst_offset,
                                                   int w, int h)

{
   DATA16 *src_itr, *dst_itr;
   DATA8 *alpha_itr;
   int y, rel_alpha, r, g, b;

   src_itr = src->pixels + src_offset;
   alpha_itr = src->alpha + src_offset;
   dst_itr = dst->pixels + dst_offset;

   rel_alpha = A_VAL(&dc->mul.col) >> 3;
   if ((rel_alpha < 1) || (rel_alpha > 31)) return;
   rel_alpha = 31 - rel_alpha;

   r = R_VAL(&dc->mul.col);
   g = G_VAL(&dc->mul.col);
   b = B_VAL(&dc->mul.col);
   /* we'll divide by 256 to make it faster, try to improve things a bit */
   if (r > 127) r++;
   if (g > 127) g++;
   if (b > 127) b++;

   if (rel_alpha == 0)
      for (y = 0; y < h; y++)
         {
            _soft16_scanline_blend_transp_solid_mul_color_solid
               (src_itr, alpha_itr, dst_itr, w, r, g, b);
            src_itr += src->stride;
            alpha_itr += src->stride;
            dst_itr += dst->stride;
         }
   else
      for (y = 0; y < h; y++)
         {
            _soft16_scanline_blend_transp_solid_mul_color_transp
               (src_itr, alpha_itr, dst_itr, w, rel_alpha, r, g, b);
            src_itr += src->stride;
            alpha_itr += src->stride;
            dst_itr += dst->stride;
         }
}

static inline void
_soft16_image_draw_unscaled_mul_color(Soft16_Image *src, Soft16_Image *dst,
                                      RGBA_Draw_Context *dc,
                                      int src_offset, int dst_offset,
                                      int width, int height)
{
   if (src->have_alpha && (!dst->have_alpha))
      _soft16_image_draw_unscaled_transp_solid_mul_color
         (src, dst, dc, src_offset, dst_offset, width, height);
   else if ((!src->have_alpha) && (!dst->have_alpha))
      _soft16_image_draw_unscaled_solid_solid_mul_color
         (src, dst, dc, src_offset, dst_offset, width, height);
   else
      fprintf(stderr,
              "Unsupported draw of unscaled images src->have_alpha=%d, "
              "dst->have_alpha=%d, WITH COLOR MUL 0x%08x\n",
              src->have_alpha, dst->have_alpha, dc->mul.col);
}

static inline void
_soft16_image_draw_unscaled_mul(Soft16_Image *src, Soft16_Image *dst,
                                RGBA_Draw_Context *dc,
                                int src_offset, int dst_offset,
                                int width, int height)
{
   if ((A_VAL(&dc->mul.col) == R_VAL(&dc->mul.col)) &&
       (A_VAL(&dc->mul.col) == G_VAL(&dc->mul.col)) &&
       (A_VAL(&dc->mul.col) == B_VAL(&dc->mul.col)))
      _soft16_image_draw_unscaled_mul_alpha(src, dst, dc, src_offset,
                                            dst_offset, width, height);
   else
      _soft16_image_draw_unscaled_mul_color(src, dst, dc, src_offset,
                                            dst_offset, width, height);
}

static void
_soft16_image_draw_unscaled(Soft16_Image *src, Soft16_Image *dst,
			    RGBA_Draw_Context *dc,
			    const Evas_Rectangle sr,
			    const Evas_Rectangle dr,
			    const Evas_Rectangle cr)
{
   int src_offset_rows, src_offset, dst_offset;

   src_offset_rows = (cr.y - dr.y) + sr.y;
   src_offset = (src_offset_rows * src->stride) + (cr.x - dr.x) + sr.x;

   dst_offset = cr.x + (cr.y * dst->stride);

   if ((!dc->mul.use) || (dc->mul.col == 0xffffffff))
       _soft16_image_draw_unscaled_no_mul(src, dst, dc, src_offset, dst_offset,
                                          cr.w, cr.h);
   else if (dc->mul.col != 0x00000000)
       _soft16_image_draw_unscaled_mul(src, dst, dc, src_offset, dst_offset,
                                       cr.w, cr.h);
}

static void
_soft16_image_draw_sampled_int(Soft16_Image *src, Soft16_Image *dst,
			       RGBA_Draw_Context *dc,
			       Evas_Rectangle sr, Evas_Rectangle dr)
{
   Evas_Rectangle cr;
/*    Scanline_Func func; */
   int      x, y;
   int     *lin_ptr;
   DATA16  *buf, *dptr;
   DATA16 **row_ptr = NULL;
   DATA16  *ptr, *dst_ptr, *src_data, *dst_data;
   DATA8   *bufa, *daptr;
   DATA8   **rowa_ptr = NULL;
   DATA8   *aptr, *dsta_ptr, *srca_data, *dsta_data;

   if (!(RECTS_INTERSECT(dr.x, dr.y, dr.w, dr.h, 0, 0, dst->w, dst->h)))
     return;
   if (!(RECTS_INTERSECT(sr.x, sr.y, sr.w, sr.h, 0, 0, src->w, src->h)))
     return;

   _get_clip(dc, dst, &cr);
   if (!_soft16_adjust_areas(&sr, src->w, src->h, &dr, dst->w, dst->h, &cr))
      return;

   src_data = src->pixels;
   srca_data = src->alpha;
   dst_data = dst->pixels;
   dsta_data = dst->alpha;

   /* figure out dest start ptr */
   dst_ptr = dst_data + cr.x + (cr.y * dst->stride);
   dsta_ptr = dsta_data + cr.x + (cr.y * dst->stride);

/*    func = _soft16_scanline_copy; */
/*    if      (( src->alpha) && ( dst->alpha)) func = _soft16_scanline_blend; */
/*    else if (( src->alpha) && (!dst->alpha)) func = _soft16_scanline_blend; */
/*    else if ((!src->alpha) && ( dst->alpha)) func = _soft16_scanline_copy; */
/*    else if ((!src->alpha) && (!dst->alpha)) func = _soft16_scanline_copy; */

   /* not being scaled at all */
   if ((dr.w == sr.w) && (dr.h == sr.h))
      _soft16_image_draw_unscaled(src, dst, dc, sr, dr, cr);
#if 0
   else
     {
	/* allocate scale lookup tables */
	lin_ptr = alloca(cr.w * sizeof(int));
	row_ptr = alloca(cr.h * sizeof(DATA16 *));
	rowa_ptr = alloca(cr.h * sizeof(DATA8 *));
	
	/* fill scale tables */
	for (x = 0; x < cr.w; x++)
	  lin_ptr[x] = (((x + cr.x - dr.x) * sr.w) / dr.w) + sr.x;
	for (y = 0; y < cr.h; y++)
	  {
	     row_ptr[y] = src_data + 
	       (((((y + cr.y - dr.y) * sr.h) / 
		  dr.h) + sr.y) * src->stride);
	     rowa_ptr[y] = srca_data + 
	       (((((y + cr.y - dr.y) * sr.h) / 
		  dr.h) + sr.y) * src->stride);
	  }
	/* scale to dst */
	dptr = dst_ptr;
	daptr = dsta_ptr;
	if ((!(src->alpha)) && (!(dst->alpha)) && (!dc->mul.use))
	  {
	     for (y = 0; y < cr.h; y++)
	       {
		  dst_ptr = dptr;
		  for (x = 0; x < cr.w; x++)
		    {
		       ptr = row_ptr[y] + lin_ptr[x];
		       *dst_ptr = *ptr;
		       dst_ptr++;
		    }
		  dptr += dst->stride;
	       }
	  }
	else
	  {
	     /* a scanline buffer */
	     buf = alloca(cr.w * sizeof(DATA16) * 2);
	     if (src->alpha)
	       {
		  bufa = alloca(cr.w * sizeof(DATA8) * 2);
		  for (y = 0; y < cr.h; y++)
		    {
		       dst_ptr = dptr;
		       for (x = 0; x < cr.w; x++)
			 {
			    ptr = row_ptr[y] + lin_ptr[x];
			    aptr = rowa_ptr[y] + lin_ptr[x];
			    BLEND(ptr, aptr, dst_ptr);
			    dst_ptr++;
			 }
/*		       
		       dst_ptr = buf;
		       dsta_ptr = bufa;
		       for (x = 0; x < cr.w; x++)
			 {
			    ptr = row_ptr[y] + lin_ptr[x];
			    *dst_ptr = *ptr;
			    dst_ptr++;
			    
			    aptr = rowa_ptr[y] + lin_ptr[x];
			    *dsta_ptr = *aptr;
			    dsta_ptr++;
			 }
		       func(buf, bufa, NULL, dc->mul.col, dptr, cr.w);
 */
		       dptr += dst->stride;
		    }
	       }
	     else
	       {
		  for (y = 0; y < cr.h; y++)
		    {
		       dst_ptr = dptr;
		       for (x = 0; x < cr.w; x++)
			 {
			    ptr = row_ptr[y] + lin_ptr[x];
			    *dst_ptr = *ptr;
			    dst_ptr++;
			 }
/*		       
		       dst_ptr = buf;
		       for (x = 0; x < cr.w; x++)
			 {
			    ptr = row_ptr[y] + lin_ptr[x];
			    *dst_ptr = *ptr;
			    dst_ptr++;
			 }
		       func(buf, NULL, NULL, dc->mul.col, dptr, cr.w);
 */
		       dptr += dst->stride;
		    }
	       }
	  }
     }
#endif
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
   int c, cx, cy, cw, ch;
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
   c = dc->clip.use; cx = dc->clip.x; cy = dc->clip.y; cw = dc->clip.w; ch = dc->clip.h;
   evas_common_draw_context_clip_clip(dc, 0, 0, dst->w, dst->h);
   evas_common_draw_context_clip_clip(dc, dst_region_x, dst_region_y, dst_region_w, dst_region_h);
   /* our clip is 0 size.. abort */
   if ((dc->clip.w <= 0) || (dc->clip.h <= 0))
     {
	dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;
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
   /* restore clip info */
   dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;
}


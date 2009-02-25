#include "evas_common_soft16.h"

static Evas_Cache_Image *eci = NULL;
static int               reference = 0;

static Image_Entry      *_evas_common_soft16_image_new(void);
static void              _evas_common_soft16_image_delete(Image_Entry *ie);

static int               _evas_common_soft16_image_surface_alloc(Image_Entry *ie, int w, int h);
static void              _evas_common_soft16_image_surface_delete(Image_Entry *ie);
static DATA32		*_evas_common_soft16_image_surface_pixels(Image_Entry *ie);

static int               _evas_common_load_soft16_image_from_file(Image_Entry *ie);
static void              _evas_common_soft16_image_unload(Image_Entry *ie);

static void              _evas_common_soft16_image_dirty_region(Image_Entry *im, int x, int y, int w, int h);
static int               _evas_common_soft16_image_dirty(Image_Entry *ie_dst, const Image_Entry *ie_src);

static int               _evas_common_soft16_image_ram_usage(Image_Entry *ie);

static int               _evas_common_soft16_image_size_set(Image_Entry *ie_dst, const Image_Entry *ie_im, int w, int h);
static int               _evas_common_soft16_image_from_copied_data(Image_Entry* ie_dst, int w, int h, DATA32 *image_data, int alpha, int cspace);
static int               _evas_common_soft16_image_from_data(Image_Entry* ie_dst, int w, int h, DATA32 *image_data, int alpha, int cspace);
static int               _evas_common_soft16_image_colorspace_set(Image_Entry* ie_dst, int cspace);

static int               _evas_common_load_soft16_image_data_from_file(Image_Entry *ie);

static void
_evas_common_soft16_image_debug(const char* context, Image_Entry *eim)
{
   fprintf(stderr, "[16] %p = [%s] {%s,%s} %i [%i|%i]\n", eim, context, eim->file, eim->key, eim->references, eim->w, eim->h);
}

static const Evas_Cache_Image_Func      _evas_common_soft16_image_func =
{
   _evas_common_soft16_image_new,
   _evas_common_soft16_image_delete,
   _evas_common_soft16_image_surface_alloc,
   _evas_common_soft16_image_surface_delete,
   _evas_common_soft16_image_surface_pixels,
   _evas_common_load_soft16_image_from_file,
   _evas_common_soft16_image_unload,
   _evas_common_soft16_image_dirty_region,
   _evas_common_soft16_image_dirty,
   _evas_common_soft16_image_size_set,
   _evas_common_soft16_image_from_copied_data,
   _evas_common_soft16_image_from_data,
   _evas_common_soft16_image_colorspace_set,
   _evas_common_load_soft16_image_data_from_file,
   _evas_common_soft16_image_ram_usage,
/*    _evas_common_soft16_image_debug */
   NULL
};

EAPI void
evas_common_soft16_image_init(void)
{
   if (!eci)
     eci = evas_cache_image_init(&_evas_common_soft16_image_func);
   reference++;
}

EAPI void
evas_common_soft16_image_shutdown(void)
{
   if (--reference == 0)
     {
// DISABLE for now - something wrong with cache shutdown freeing things
// still in use - rage_thumb segv's now.
// 
// actually - i think i see it. cache ref goes to 0 (and thus gets freed)
// because in eng_setup() when a buffer changes size it is FIRST freed
// THEN allocated again - thus brignhjing ref to 0 then back to 1 immediately
// where it should stay at 1. - see evas_engine.c in the buffer enigne for
// example. eng_output_free() is called BEFORE _output_setup(). although this
// is only a SIGNE of the problem. we can patch this up with either freeing
// after the setup (so we just pt a ref of 2 then back to 1), or just 
// evas_common_image_init() at the start and evas_common_image_shutdown()
// after it all. really ref 0 should only be reached when no more canvases
// with no more objects exist anywhere.
//        evas_cache_image_shutdown(eci);
//        eci = NULL;
     }
}

EAPI Evas_Cache_Image *
evas_common_soft16_image_cache_get(void)
{
   return eci;
}

static Image_Entry *
_evas_common_soft16_image_new(void)
{
   Soft16_Image *im;

   im = calloc(1, sizeof(Soft16_Image));
   if (!im) return NULL;

   im->stride = -1;

   return (Image_Entry *) im;
}

static void
_evas_common_soft16_image_delete(Image_Entry *ie)
{
   memset(ie, 0xFF, sizeof (Soft16_Image));
   free(ie);
}

static int
_evas_common_soft16_image_surface_alloc(Image_Entry *ie, int w, int h)
{
   Soft16_Image *im = (Soft16_Image *) ie;

   if (im->stride < 0) im->stride = _calc_stride(w);

   im->pixels = realloc(im->pixels, IMG_BYTE_SIZE(im->stride, h, ie->flags.alpha));
   if (!im->pixels) return -1;

   if (ie->flags.alpha)
     {
        im->alpha = (DATA8 *)(im->pixels + (im->stride * h));
        im->flags.free_alpha = 0;
     }
   im->flags.free_pixels = 1;

   return 0;
}

static void
_evas_common_soft16_image_surface_delete(Image_Entry *ie)
{
   Soft16_Image *im = (Soft16_Image *) ie;

   if (im->flags.free_pixels)
     free(im->pixels);
   im->pixels = NULL;
   im->flags.free_pixels = 0;

   if (im->flags.free_alpha)
     free(im->alpha);
   im->alpha = NULL;
   im->flags.free_alpha = 0;
}

static DATA32 *
_evas_common_soft16_image_surface_pixels(Image_Entry *ie __UNUSED__)
{
   abort();

   return NULL;
}

static int
_evas_common_load_soft16_image_from_file(Image_Entry *ie)
{
   Soft16_Image *sim = (Soft16_Image *) ie;
   RGBA_Image   *im;
   int           error = 0;

   im = (RGBA_Image *) evas_cache_image_request(evas_common_image_cache_get(), sim->cache_entry.file, sim->cache_entry.key, &sim->cache_entry.load_opts, &error);
   sim->source = im;
   if (!sim->source) return -1;

   sim->cache_entry.w = sim->source->cache_entry.w;
   sim->cache_entry.h = sim->source->cache_entry.h;
   ie->flags.alpha = im->cache_entry.flags.alpha;
   if (sim->stride < 0) sim->stride = _calc_stride(sim->cache_entry.w);

   return 0;
}

static void
_evas_common_soft16_image_unload(Image_Entry *ie __UNUSED__)
{
}

static void
_evas_common_soft16_image_dirty_region(Image_Entry *im __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
}

static int
_evas_common_soft16_image_dirty(Image_Entry *ie_dst, const Image_Entry *ie_src)
{
   Soft16_Image *dst = (Soft16_Image *) ie_dst;
   Soft16_Image *src = (Soft16_Image *) ie_src;

   evas_cache_image_load_data(&src->cache_entry);
   evas_cache_image_surface_alloc(&dst->cache_entry,
                                  src->cache_entry.w, src->cache_entry.h);

/*    evas_common_blit_rectangle(src, dst, 0, 0, src->cache_entry.w, src->cache_entry.h, 0, 0); */

   return 0;
}

static int
_evas_common_soft16_image_ram_usage(Image_Entry *ie)
{
   Soft16_Image *im = (Soft16_Image *) ie;

   if (im->pixels && im->flags.free_pixels)
     return IMG_BYTE_SIZE(im->stride, im->cache_entry.h, ie->flags.alpha);
   return 0;
}

static int
_evas_common_soft16_image_size_set(Image_Entry *ie_dst, const Image_Entry *ie_im, int w __UNUSED__, int h __UNUSED__)
{
   Soft16_Image *dst = (Soft16_Image *) ie_dst;
   Soft16_Image *im = (Soft16_Image *) ie_im;

   dst->flags = im->flags;

   return 0;
}

static int
_evas_common_soft16_image_from_data(Image_Entry* ie_dst, int w, int h, DATA32 *image_data, int alpha, int cspace __UNUSED__)
{
   Soft16_Image *im = (Soft16_Image *) ie_dst;

   /* FIXME: handle colorspace */
   ie_dst->w = w;
   ie_dst->h = h;
   ie_dst->flags.alpha = alpha;

   im->flags.free_pixels = 0;
   im->flags.free_alpha = 0;

   /* FIXME: That's bad, the application must be aware of the engine internal. */
   im->pixels = (DATA16 *) image_data;
   if (ie_dst->flags.alpha)
     im->alpha = (DATA8 *)(im->pixels + (im->stride * h));

   return 0;
}

static int
_evas_common_soft16_image_from_copied_data(Image_Entry* ie_dst, int w __UNUSED__, int h, DATA32 *image_data, int alpha __UNUSED__, int cspace __UNUSED__)
{
   Soft16_Image *im = (Soft16_Image *) ie_dst;

   /* FIXME: handle colorspace */
   if (image_data)
     memcpy(im->pixels, image_data, IMG_BYTE_SIZE(im->stride, h, ie_dst->flags.alpha));
   else
     memset(im->pixels, 0, IMG_BYTE_SIZE(im->stride, h, ie_dst->flags.alpha));

   return 0;
}

static int
_evas_common_soft16_image_colorspace_set(Image_Entry* ie_dst __UNUSED__, int cspace __UNUSED__)
{
   /* FIXME: handle colorspace */
   return 0;
}

static int
_evas_common_load_soft16_image_data_from_file(Image_Entry *ie)
{
   Soft16_Image *im = (Soft16_Image *) ie;

   if (im->pixels) return 0;
   if (!im->source) return -1;

   evas_cache_image_load_data(&im->source->cache_entry);
   if (im->source->image.data)
     {
        DATA32 *sp;

        evas_cache_image_surface_alloc(&im->cache_entry,
                                       im->source->cache_entry.w,
                                       im->source->cache_entry.h);

        sp = im->source->image.data;
        if (im->alpha)
          soft16_image_convert_from_rgba(im, sp);
        else
          soft16_image_convert_from_rgb(im, sp);
     }
   evas_cache_image_drop(&im->source->cache_entry);
   im->source = NULL;

   return 0;
}

/* Soft16_Image * */
/* soft16_image_new(int w, int h, int stride, int have_alpha, DATA16 *pixels, */
/* 		 int copy) */
/* { */
/*    Soft16_Image *im; */

/*    if (stride < 0) stride = _calc_stride(w); */

/*    im = soft16_image_alloc(w, h, stride, have_alpha, copy); */
/*    if (!im) return NULL; */

/*    if (pixels) */
/*      { */
/* 	if (copy) */
/* 	  memcpy(im->pixels, pixels, IMG_BYTE_SIZE(stride, h, have_alpha)); */
/* 	else */
/* 	  { */
/* 	     im->pixels = pixels; */
/* 	     if (have_alpha) im->alpha = (DATA8 *)(im->pixels + (stride * h)); */
/* 	  } */
/*      } */
/*    return im; */
/* } */

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
	if ((clip->x + clip->w) > im->cache_entry.w) clip->w = im->cache_entry.w - clip->x;
	if ((clip->y + clip->h) > im->cache_entry.h) clip->h = im->cache_entry.h - clip->y;
     }
   else
     {
	clip->x = 0;
	clip->y = 0;
	clip->w = im->cache_entry.w;
	clip->h = im->cache_entry.h;
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

   _shrink(&dst_clip->x, &dst_clip->w, 0, dst_max_x);
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

   _shrink(&dst_clip->y, &dst_clip->h, 0, dst_max_y);
   if (dst_clip->h <= 0) return 0;

   return 1;
}

static void
_soft16_image_draw_sampled_int(Soft16_Image *src, Soft16_Image *dst,
			       RGBA_Draw_Context *dc,
			       Evas_Rectangle sr, Evas_Rectangle dr)
{
   Evas_Rectangle cr;

   if (!(RECTS_INTERSECT(dr.x, dr.y, dr.w, dr.h, 0, 0, dst->cache_entry.w, dst->cache_entry.h)))
     return;
   if (!(RECTS_INTERSECT(sr.x, sr.y, sr.w, sr.h, 0, 0, src->cache_entry.w, src->cache_entry.h)))
     return;

   _get_clip(dc, dst, &cr);
   if (!_soft16_adjust_areas(&sr, src->cache_entry.w, src->cache_entry.h, &dr, dst->cache_entry.w, dst->cache_entry.h, &cr))
     return;

   if ((dr.w == sr.w) && (dr.h == sr.h))
     soft16_image_draw_unscaled(src, dst, dc, sr, dr, cr);
   else
     soft16_image_draw_scaled_sampled(src, dst, dc, sr, dr, cr);
}

EAPI void
soft16_image_draw(Soft16_Image *src, Soft16_Image *dst,
		  RGBA_Draw_Context *dc,
		  int src_region_x, int src_region_y,
		  int src_region_w, int src_region_h,
		  int dst_region_x, int dst_region_y,
		  int dst_region_w, int dst_region_h,
		  int smooth __UNUSED__)
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
   if (!(RECTS_INTERSECT(dr.x, dr.y, dr.w, dr.h, 0, 0, dst->cache_entry.w, dst->cache_entry.h)))
     return;

   sr.x = src_region_x;
   sr.y = src_region_y;
   sr.w = src_region_w;
   sr.h = src_region_h;

   if (_is_empty_rectangle(&sr)) return;
   if (!(RECTS_INTERSECT(sr.x, sr.y, sr.w, sr.h, 0, 0, src->cache_entry.w, src->cache_entry.h)))
     return;

   /* no cutouts - cut right to the chase */
   if (!dc->cutout.rects)
     {
	_soft16_image_draw_sampled_int(src, dst, dc, sr, dr);
	return;
     }

   /* save out clip info */
   clip_bkp = dc->clip;
   evas_common_draw_context_clip_clip(dc, 0, 0, dst->cache_entry.w, dst->cache_entry.h);
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

EAPI Soft16_Image *
soft16_image_alpha_set(Soft16_Image *im, int have_alpha)
{
   Soft16_Image   *new_im;

   if (im->cache_entry.flags.alpha == have_alpha) return im;

   new_im = (Soft16_Image *) evas_cache_image_alone(&im->cache_entry);

   new_im->cache_entry.flags.alpha = have_alpha;

   if (im->cache_entry.w > 0
       && im->cache_entry.h)
     new_im = (Soft16_Image *) evas_cache_image_size_set(&new_im->cache_entry, im->cache_entry.w, im->cache_entry.h);

   return new_im;
}

/* Soft16_Image * */
/* soft16_image_size_set(Soft16_Image *old_im, int w, int h) */
/* { */
/*    Soft16_Image *new_im; */
/*    DATA16 *dp, *sp; */
/*    int i, cw, ch, ew; */

/*    if ((old_im->cache_entry.w == w) && (old_im->cache_entry.h == h)) return old_im; */

/*    new_im = soft16_image_new(w, h, -1, old_im->flags.have_alpha, NULL, 1); */

/*    if (old_im->cache_entry.w < new_im->cache_entry.w) */
/*      cw = old_im->cache_entry.w; */
/*    else */
/*      cw = new_im->cache_entry.w; */

/*    ew = new_im->cache_entry.w - cw; */

/*    if (old_im->cache_entry.h < new_im->cache_entry.h) */
/*      ch = old_im->cache_entry.h; */
/*    else */
/*      ch = new_im->cache_entry.h; */

/*    dp = new_im->pixels; */
/*    sp = old_im->pixels; */
/*    for (i = 0; i < ch; i++) */
/*      { */
/* 	memcpy(dp, sp, cw * sizeof(DATA16)); */
/* 	if (ew > 0) memset(dp, 0, ew * sizeof(DATA16)); */

/* 	dp += new_im->stride; */
/* 	sp += old_im->stride; */
/*      } */

/*    if (old_im->flags.have_alpha) */
/*      { */
/* 	DATA8 *dp, *sp; */

/* 	dp = new_im->alpha; */
/* 	sp = old_im->alpha; */
/* 	for (i = 0; i < ch; i++) */
/* 	  { */
/* 	     memcpy(dp, sp, cw * sizeof(DATA8)); */
/* 	     if (ew > 0) memset(dp, 0, ew * sizeof(DATA8)); */

/* 	     dp += new_im->stride; */
/* 	     sp += old_im->stride; */
/* 	  } */
/*      } */

/*    evas_cache_image_drop(&old_im->cache_entry); */
/*    return new_im; */
/* } */

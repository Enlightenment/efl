#include "evas_common.h"
#include "evas_private.h"
#include "evas_soft16.h"

#define BLEND(s, a, d) \
   { DATA32 wsp, wdp; \
      wsp = (*s | (*s << 16)) & 0x07e0f81f; \
      wdp = (*d | (*d << 16)) & 0x07e0f81f; \
      wdp = (((wdp * (32 - *a)) >> 5) & 0x07e0f81f) + wsp; \
      *d = wdp | (wdp >> 16); \
   }

typedef void (*Scanline_Func) (DATA16 *s, DATA8 *a, DATA8 *m, DATA32 c, DATA16 *d, int l);

static void
_soft16_image_rgba32_import(Soft16_Image *im, DATA32 *src);
static void
_soft16_image_draw_sampled_int(Soft16_Image *src, Soft16_Image *dst,
			       RGBA_Draw_Context *dc,
			       int src_region_x, int src_region_y,
			       int src_region_w, int src_region_h,
			       int dst_region_x, int dst_region_y,
			       int dst_region_w, int dst_region_h);
static void 
_soft16_scanline_blend(DATA16 *s, DATA8 *a, DATA8 *m, DATA32 c, DATA16 *d, int l);
static void
_soft16_scanline_copy(DATA16 *s, DATA8 *a, DATA8 *m, DATA32 c, DATA16 *d, int l);

static Evas_Hash *_soft16_image_cache_hash = NULL;

Soft16_Image *
soft16_image_new(int w, int h, int stride, int have_alpha, DATA16 *pixels, int copy)
{
   Soft16_Image *im;
   
   im = calloc(1, sizeof(Soft16_Image));
   if (!im) return NULL;
   im->w = w;
   im->h = h;
   im->stride = w;
   if (copy)
     {
	if (have_alpha)
	  {
	     im->pixels = malloc((im->stride * im->h * sizeof(DATA16)) +
				 (im->stride * im->h * sizeof(DATA8)));
	     if (!im->pixels)
	       {
		  free(im);
		  return NULL;
	       }
	     im->alpha = (DATA8 *)(im->pixels + (im->stride * im->h));
	     if (pixels)
	       memcpy(im->pixels, pixels, 
		      (im->stride * im->h * sizeof(DATA16)) +
		      (im->stride * im->h * sizeof(DATA8)));
	  }
	else
	  {
	     im->pixels = malloc(im->stride * im->h * sizeof(DATA16));
	     if (!im->pixels)
	       {
		  free(im);
		  return NULL;
	       }
	     if (pixels)
	       memcpy(im->pixels, pixels, 
		      im->stride * im->h * sizeof(DATA16));
	  }
	im->free_pixels = 1;
     }
   else
     {
	im->pixels = pixels;
	if (have_alpha) im->alpha = (DATA8 *)(im->pixels + (im->stride * im->h));
     }
   im->references = 1;
   return im;
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
	
	if ((im->lo.scale_down_by == 0) && (im->lo.dpi == 0.0) &&
	    ((im->lo.w == 0) || (im->lo.h == 0)))
	  {
	     if (im->key)
	       snprintf(buf, sizeof(buf), "%s//://%s", im->file, im->key);
	     else
	       snprintf(buf, sizeof(buf), "%s", im->file);
	  }
	else
	  {
	     if (im->key)
	       snprintf(buf, sizeof(buf), "//@/%i/%1.5f/%ix%i//%s//://%s", 
			im->lo.scale_down_by, im->lo.dpi, im->lo.w, im->lo.h,
			im->file, im->key);
	     else
	       snprintf(buf, sizeof(buf), "//@/%i/%1.5f/%ix%i//%s",
			im->lo.scale_down_by, im->lo.dpi, im->lo.w, im->lo.h,
			im->file);
	  }
	_soft16_image_cache_hash = evas_hash_del(_soft16_image_cache_hash,
						 buf, im);
     }
   if (im->file) evas_stringshare_del(im->file);
   if (im->key) evas_stringshare_del(im->key);
   if (im->free_pixels) free(im->pixels);
   free(im);
}

#define STAT_GAP 2

Soft16_Image *
soft16_image_load(const char *file, const char *key, int *error,
		  Evas_Image_Load_Opts *lo)
{
   Soft16_Image *im;
   RGBA_Image *sim;
   char buf[4096 + 1024];
   struct stat st;
   static time_t laststat = 0;
   time_t t, mt = 0;

   *error = 0;
   if (!(lo) ||
       ((lo->scale_down_by == 0) && (lo->dpi == 0.0) &&
	((lo->w == 0) || (lo->h == 0))))
     {
	if (key)
	  snprintf(buf, sizeof(buf), "%s//://%s", file, key);
	else
	  snprintf(buf, sizeof(buf), "%s", file);
     }
   else
     {
        if (key)
	  snprintf(buf, sizeof(buf), "//@/%i/%1.5f/%ix%i//%s//://%s",
		   lo->scale_down_by, lo->dpi, lo->w, lo->h, file, key);
	else
	  snprintf(buf, sizeof(buf), "//@/%i/%1.5f/%ix%i//%s",
		   lo->scale_down_by, lo->dpi, lo->w, lo->h, file);
     }
   im = evas_hash_find(_soft16_image_cache_hash, buf);
   if (im)
     {
	if ((t - im->laststat) < STAT_GAP)
	  {
	     im->references++;
	     return im;
	  }
	else
	  {
            struct stat st;
	     
	     if (stat(file, &st) < 0) return NULL;
	     mt = st.st_mtime;
	     if (mt == im->timestamp)
	       {
		  im->laststat = t;
		  laststat = t;
		  im->references++;
		  return im;
	       }
	  }
     }
   sim = evas_common_load_image_from_file(file, key, lo);
   if (!sim) return NULL;
   im = calloc(1, sizeof(Soft16_Image));
   if (!im)
     {
	evas_common_image_unref(sim);
	return NULL;
     }
   im->source_im = sim;
   im->w = im->source_im->image->w;
   im->h = im->source_im->image->h;
   im->stride = im->w;
   im->timestamp = im->source_im->timestamp;
   im->laststat = im->source_im->laststat;
   if (lo) im->lo = *lo;
   if (file) im->file = evas_stringshare_add(file);
   if (key) im->key = evas_stringshare_add(key);
   if (im->source_im->flags & RGBA_IMAGE_HAS_ALPHA) im->have_alpha = 1;
   im->references = 1;
   _soft16_image_cache_hash = evas_hash_add(_soft16_image_cache_hash, buf, im);
   return im;
}


static void
_soft16_image_rgba32_import(Soft16_Image *im, DATA32 *src)
{
   DATA32 *sp, r, g, b, a;
   DATA16 *dp, *dpl;
   DATA8 *da, *dal;
   int x, y;
   
   /* FIXME: dither and optimize */
   sp = src;
   dpl = im->pixels;
   dal = im->alpha;
   if (dal)
     {
	for (y = 0; y < im->h; y++)
	  {
	     dp = dpl;
	     da = dal;
	     for (x = 0; x < im->w; x++)
	       {
		  a = (A_VAL(sp) * 32) / 255;
		  r = (R_VAL(sp) >> 3);
		  g = (G_VAL(sp) >> 2);
		  b = (B_VAL(sp) >> 3);
		  /* FIXME: not sure about this yet - also need to check alpha is not letss that G when alpha is upscaled to 6bits */
		  *da = a; /* scale 0-32 - yes, 1 over 5bits - makes bitshift math later work better */
		  if ((*da << 1) < g) g--;
		  *dp = (r << 11) | (g << 5) | (b);
		  dp++;
		  da++;
 		  sp++;
	       }
	     dpl += im->stride;
	     dal += im->stride;
	  }
     }
   else
     {
	for (y = 0; y < im->h; y++)
	  {
	     dp = dpl;
	     for (x = 0; x < im->w; x++)
	       {
		  *dp = 
		    ((R_VAL(sp) >> 3) << 11) |
		    ((G_VAL(sp) >> 2) << 5 ) |
		    ((B_VAL(sp) >> 3)      );
		  dp++;
 		  sp++;
	       }
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
	if (im->source_im->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
	     im->pixels = malloc((im->stride * im->h * sizeof(DATA16)) +
				 (im->stride * im->h * sizeof(DATA8)));
	     if (!im->pixels) goto done;
	     im->alpha = (DATA8 *)(im->pixels + (im->stride * im->h));
	  }
	else
	  {
	     im->pixels = malloc(im->stride * im->h * sizeof(DATA16));
	     if (!im->pixels) goto done;
	  }
	_soft16_image_rgba32_import(im, im->source_im->image->data);
	im->free_pixels = 1;
     }
   done:
   evas_common_image_unref(im->source_im);
   im->source_im = NULL;
}

static void
_soft16_image_draw_sampled_int(Soft16_Image *src, Soft16_Image *dst,
			       RGBA_Draw_Context *dc,
			       int src_region_x, int src_region_y,
			       int src_region_w, int src_region_h,
			       int dst_region_x, int dst_region_y,
			       int dst_region_w, int dst_region_h)
{
   Scanline_Func func;
   int      x, y;
   int     *lin_ptr;
   DATA16  *buf, *dptr;
   DATA16 **row_ptr = NULL;
   DATA16  *ptr, *dst_ptr, *src_data, *dst_data;
   DATA8   *bufa, *daptr;
   DATA8   **rowa_ptr = NULL;
   DATA8   *aptr, *dsta_ptr, *srca_data, *dsta_data;
   int      dst_jump, dst_stride, src_stride;
   int      dst_clip_x, dst_clip_y, dst_clip_w, dst_clip_h;
   int      src_w, src_h, dst_w, dst_h;


   if (!(RECTS_INTERSECT(dst_region_x, dst_region_y, dst_region_w, dst_region_h, 0, 0, dst->w, dst->h)))
     return;
   if (!(RECTS_INTERSECT(src_region_x, src_region_y, src_region_w, src_region_h, 0, 0, src->w, src->h)))
     return;
   
   src_w = src->w;
   src_h = src->h;
   dst_w = dst->w;
   dst_h = dst->h;

   if (dc->clip.use)
     {
	dst_clip_x = dc->clip.x;
	dst_clip_y = dc->clip.y;
	dst_clip_w = dc->clip.w;
	dst_clip_h = dc->clip.h;
	if (dst_clip_x < 0)
	  {
	     dst_clip_w += dst_clip_x;
	     dst_clip_x = 0;
	  }
	if (dst_clip_y < 0)
	  {
	     dst_clip_h += dst_clip_y;
	     dst_clip_y = 0;
	  }
	if ((dst_clip_x + dst_clip_w) > dst_w)
	  dst_clip_w = dst_w - dst_clip_x;
	if ((dst_clip_y + dst_clip_h) > dst_h)
	  dst_clip_h = dst_h - dst_clip_y;
     }
   else
     {
	dst_clip_x = 0;
	dst_clip_y = 0;
	dst_clip_w = dst_w;
	dst_clip_h = dst_h;
     }

   if (dst_clip_x < dst_region_x)
     {
	dst_clip_w += dst_clip_x - dst_region_x;
	dst_clip_x = dst_region_x;
     }
   if ((dst_clip_x + dst_clip_w) > (dst_region_x + dst_region_w))
     dst_clip_w = dst_region_x + dst_region_w - dst_clip_x;
   if (dst_clip_y < dst_region_y)
     {
	dst_clip_h += dst_clip_y - dst_region_y;
	dst_clip_y = dst_region_y;
     }
   if ((dst_clip_y + dst_clip_h) > (dst_region_y + dst_region_h))
     dst_clip_h = dst_region_y + dst_region_h - dst_clip_y;
   
   if ((src_region_w <= 0) || (src_region_h <= 0) ||
       (dst_region_w <= 0) || (dst_region_h <= 0) ||
       (dst_clip_w <= 0) || (dst_clip_h <= 0))
     return;

   /* sanitise x */
   if (src_region_x < 0)
     {
	dst_region_x -= (src_region_x * dst_region_w) / src_region_w;
	dst_region_w += (src_region_x * dst_region_w) / src_region_w;
	src_region_w += src_region_x;
	src_region_x = 0;
     }
   if (src_region_x >= src_w) return;
   if ((src_region_x + src_region_w) > src_w)
     {
	dst_region_w = (dst_region_w * (src_w - src_region_x)) / (src_region_w);
	src_region_w = src_w - src_region_x;
     }
   if (dst_region_w <= 0) return;
   if (src_region_w <= 0) return;
   if (dst_clip_x < 0)
     {
	dst_clip_w += dst_clip_x;
	dst_clip_x = 0;
     }
   if (dst_clip_w <= 0) return;
   if (dst_clip_x >= dst_w) return;
   if (dst_clip_x < dst_region_x)
     {
	dst_clip_w += (dst_clip_x - dst_region_x);
	dst_clip_x = dst_region_x;
     }
   if ((dst_clip_x + dst_clip_w) > dst_w)
     {
	dst_clip_w = dst_w - dst_clip_x;
     }
   if (dst_clip_w <= 0) return;
   
   /* sanitise y */
   if (src_region_y < 0)
     {
	dst_region_y -= (src_region_y * dst_region_h) / src_region_h;
	dst_region_h += (src_region_y * dst_region_h) / src_region_h;
	src_region_h += src_region_y;
	src_region_y = 0;
     }
   if (src_region_y >= src_h) return;
   if ((src_region_y + src_region_h) > src_h)
     {
	dst_region_h = (dst_region_h * (src_h - src_region_y)) / (src_region_h);
	src_region_h = src_h - src_region_y;
     }
   if (dst_region_h <= 0) return;
   if (src_region_h <= 0) return;
   if (dst_clip_y < 0)
     {
	dst_clip_h += dst_clip_y;
	dst_clip_y = 0;
     }
   if (dst_clip_h <= 0) return;
   if (dst_clip_y >= dst_h) return;
   if (dst_clip_y < dst_region_y)
     {
	dst_clip_h += (dst_clip_y - dst_region_y);
	dst_clip_y = dst_region_y;
     }
   if ((dst_clip_y + dst_clip_h) > dst_h)
     {
	dst_clip_h = dst_h - dst_clip_y;
     }
   if (dst_clip_h <= 0) return;

   src_data = src->pixels;
   srca_data = src->alpha;
   dst_data = dst->pixels;
   dsta_data = dst->alpha;
   
   dst_stride = dst->stride;
   src_stride = src->stride;
   
   /* figure out dest start ptr */
   dst_ptr = dst_data + dst_clip_x + (dst_clip_y * dst_stride);
   dsta_ptr = dsta_data + dst_clip_x + (dst_clip_y * dst_stride);

   func = _soft16_scanline_copy;
   if      (( src->alpha) && ( dst->alpha)) func = _soft16_scanline_blend;
   else if (( src->alpha) && (!dst->alpha)) func = _soft16_scanline_blend;
   else if ((!src->alpha) && ( dst->alpha)) func = _soft16_scanline_copy;
   else if ((!src->alpha) && (!dst->alpha)) func = _soft16_scanline_copy;

   /* not being scaled at all */
   if ((dst_region_w == src_region_w) && (dst_region_h == src_region_h))
     {
	ptr = src_data + ((dst_clip_y - dst_region_y + src_region_y) * src_stride) + (dst_clip_x - dst_region_x) + src_region_x;
	aptr = srca_data + ((dst_clip_y - dst_region_y + src_region_y) * src_stride) + (dst_clip_x - dst_region_x) + src_region_x;
	for (y = 0; y < dst_clip_h; y++)
	  {
	     /* * blend here [clip_w *] ptr -> dst_ptr * */
	     func(ptr, aptr, NULL, dc->mul.col, dst_ptr, dst_clip_w);
	     ptr += src_stride;
	     aptr += src_stride;
	     dst_ptr += dst_stride;
	  }
     }
   else
     {
	/* allocate scale lookup tables */
	lin_ptr = alloca(dst_clip_w * sizeof(int));
	row_ptr = alloca(dst_clip_h * sizeof(DATA16 *));
	rowa_ptr = alloca(dst_clip_h * sizeof(DATA8 *));
	
	/* fill scale tables */
	for (x = 0; x < dst_clip_w; x++)
	  lin_ptr[x] = (((x + dst_clip_x - dst_region_x) * src_region_w) / dst_region_w) + src_region_x;
	for (y = 0; y < dst_clip_h; y++)
	  {
	     row_ptr[y] = src_data + 
	       (((((y + dst_clip_y - dst_region_y) * src_region_h) / 
		  dst_region_h) + src_region_y) * src_stride);
	     rowa_ptr[y] = srca_data + 
	       (((((y + dst_clip_y - dst_region_y) * src_region_h) / 
		  dst_region_h) + src_region_y) * src_stride);
	  }
	/* scale to dst */
	dptr = dst_ptr;
	daptr = dsta_ptr;
	if ((!(src->alpha)) && (!(dst->alpha)) && (!dc->mul.use))
	  {
	     for (y = 0; y < dst_clip_h; y++)
	       {
		  dst_ptr = dptr;
		  for (x = 0; x < dst_clip_w; x++)
		    {
		       ptr = row_ptr[y] + lin_ptr[x];
		       *dst_ptr = *ptr;
		       dst_ptr++;
		    }
		  dptr += dst_stride;
	       }
	  }
	else
	  {
	     /* a scanline buffer */
	     buf = alloca(dst_clip_w * sizeof(DATA16) * 2);
	     if (src->alpha)
	       {
		  bufa = alloca(dst_clip_w * sizeof(DATA8) * 2);
		  for (y = 0; y < dst_clip_h; y++)
		    {
		       dst_ptr = dptr;
		       for (x = 0; x < dst_clip_w; x++)
			 {
			    ptr = row_ptr[y] + lin_ptr[x];
			    aptr = rowa_ptr[y] + lin_ptr[x];
			    BLEND(ptr, aptr, dst_ptr);
			    dst_ptr++;
			 }
/*		       
		       dst_ptr = buf;
		       dsta_ptr = bufa;
		       for (x = 0; x < dst_clip_w; x++)
			 {
			    ptr = row_ptr[y] + lin_ptr[x];
			    *dst_ptr = *ptr;
			    dst_ptr++;
			    
			    aptr = rowa_ptr[y] + lin_ptr[x];
			    *dsta_ptr = *aptr;
			    dsta_ptr++;
			 }
		       func(buf, bufa, NULL, dc->mul.col, dptr, dst_clip_w);
 */
		       dptr += dst_stride;
		    }
	       }
	     else
	       {
		  for (y = 0; y < dst_clip_h; y++)
		    {
		       dst_ptr = dptr;
		       for (x = 0; x < dst_clip_w; x++)
			 {
			    ptr = row_ptr[y] + lin_ptr[x];
			    *dst_ptr = *ptr;
			    dst_ptr++;
			 }
/*		       
		       dst_ptr = buf;
		       for (x = 0; x < dst_clip_w; x++)
			 {
			    ptr = row_ptr[y] + lin_ptr[x];
			    *dst_ptr = *ptr;
			    dst_ptr++;
			 }
		       func(buf, NULL, NULL, dc->mul.col, dptr, dst_clip_w);
 */
		       dptr += dst_stride;
		    }
	       }
	  }
     }
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
   Cutout_Rects *rects;
   Cutout_Rect  *r;
   int          c, cx, cy, cw, ch;
   int          i;
   /* handle cutouts here! */
   
   if ((dst_region_w <= 0) || (dst_region_h <= 0)) return;
   if (!(RECTS_INTERSECT(dst_region_x, dst_region_y, dst_region_w, dst_region_h, 0, 0, dst->w, dst->h)))
     return;
   /* no cutouts - cut right to the chase */
   if (!dc->cutout.rects)
     {
	_soft16_image_draw_sampled_int(src, dst, dc,
				       src_region_x, src_region_y,
				       src_region_w, src_region_h,
				       dst_region_x, dst_region_y,
				       dst_region_w, dst_region_h);
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
	_soft16_image_draw_sampled_int(src, dst, dc,
				       src_region_x, src_region_y,
				       src_region_w, src_region_h,
				       dst_region_x, dst_region_y,
				       dst_region_w, dst_region_h);
	
     }
   evas_common_draw_context_apply_clear_cutouts(rects);
   /* restore clip info */
   dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;
}


static void
_soft16_scanline_blend(DATA16 *s, DATA8 *a, DATA8 *m, DATA32 c, DATA16 *d, int l)
{
   DATA16 *e = d + l;
   DATA32 wsp, wdp;
   DATA32 xsp, xdp;
   DATA32 ysp, ydp;
   DATA32 zsp, zdp;

/*   
   if (((unsigned long)(((DATA8 *)s)) & 0x3) == 
       (((unsigned long)((DATA8 *)d)) & 0x3))
     {
	if (((unsigned long)(((DATA8 *)s)) & 0x3) == 2)
	  {
	     wsp = (*s | (*s << 16)) & 0x07e0f81f;
	     wdp = (*d | (*d << 16)) & 0x07e0f81f;
	     wdp = (((wdp * (32 - *a)) >> 5) & 0x07e0f81f) + wsp;
	     *d = wdp | (wdp >> 16);
	     s++; a++; d++;
	  }
	e -= 3;
	while (d < e)
	  {
	     wsp = (s[0] | (s[0] << 16)) & 0x07e0f81f;
	     xsp = (s[1] | (s[1] << 16)) & 0x07e0f81f;
	     ysp = (s[2] | (s[2] << 16)) & 0x07e0f81f;
	     zsp = (s[3] | (s[3] << 16)) & 0x07e0f81f;
	     wdp = (d[0] | (d[0] << 16)) & 0x07e0f81f;
	     xdp = (d[1] | (d[1] << 16)) & 0x07e0f81f;
	     ydp = (d[2] | (d[2] << 16)) & 0x07e0f81f;
	     zdp = (d[3] | (d[3] << 16)) & 0x07e0f81f;
	     wdp = (((wdp * (32 - a[0])) >> 5) & 0x07e0f81f) + wsp;
	     xdp = (((xdp * (32 - a[1])) >> 5) & 0x07e0f81f) + xsp;
	     ydp = (((ydp * (32 - a[2])) >> 5) & 0x07e0f81f) + ysp;
	     zdp = (((zdp * (32 - a[3])) >> 5) & 0x07e0f81f) + zsp;
	     *((DATA32 *)d) = 
	       ((xdp | (xdp >> 16)) << 16) | 
	       ((wdp | (wdp >> 16)) & 0x0000ffff);
	     d += 2;
	     *((DATA32 *)d) = 
	       ((zdp | (zdp >> 16)) << 16) | 
	       ((ydp | (ydp >> 16)) & 0x0000ffff);
	     d += 2;
	     s += 4; a += 4;
	  }
	e += 3;
	while (d < e)
	  {
	     wsp = (*s | (*s << 16)) & 0x07e0f81f;
	     wdp = (*d | (*d << 16)) & 0x07e0f81f;
	     wdp = (((wdp * (32 - *a)) >> 5) & 0x07e0f81f) + wsp;
	     *d = wdp | (wdp >> 16);
             s++; a++; d++;
	  }
     }
   else
 */
     {
	while (d < e)
	  {
	     wsp = (*s | (*s << 16)) & 0x07e0f81f;
	     wdp = (*d | (*d << 16)) & 0x07e0f81f;
	     wdp = (((wdp * (32 - *a)) >> 5) & 0x07e0f81f) + wsp;
	     *d = wdp | (wdp >> 16);
	     s++; a++; d++;
	  }
     }
}

//#define MEMCPY 1
//#define SIMPLE 1

static void
_soft16_scanline_copy(DATA16 *s, DATA8 *a, DATA8 *m, DATA32 c, DATA16 *d, int l)
{
#ifdef MEMCPY  
   memcpy(d, s, l * sizeof(DATA16));   
#else   
   DATA16 *e = d + l;

#ifdef SIMPLE
   while (d < e)
     {
	*d = *s;
	d++;
	s++;
     }
#else
   if (((unsigned long)(((DATA8 *)s)) & 0x3) == 
       (((unsigned long)((DATA8 *)d)) & 0x3))
     {
        if (((unsigned long)(((DATA8 *)s)) & 0x3) == 2)
	  {
	     *d = *s;
	     d++;
	     s++;
	  }
	e -= 1;
	while (d < e)
	  {
	     *((DATA32 *)d) = *((DATA32 *)s);
	     d += 2;
	     s += 2;
	  }
	e += 1;
	if (d < e)
	  {
	     *d = *s;
	  }
     }
   else
     {
	while (d < e)
	  {
	     *d = *s;
	     d++;
	     s++;
	  }
     }
#endif 
#endif   
}

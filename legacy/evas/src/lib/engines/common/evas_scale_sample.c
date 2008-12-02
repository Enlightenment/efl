/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"
#include "evas_blend_private.h"

void scale_rgba_in_to_out_clip_sample_internal(RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);

#ifndef BUILD_SCALE_SMOOTH
#ifdef BUILD_SCALE_SAMPLE
EAPI void
evas_common_scale_rgba_in_to_out_clip_smooth(RGBA_Image *src, RGBA_Image *dst,
				 RGBA_Draw_Context *dc,
				 int src_region_x, int src_region_y,
				 int src_region_w, int src_region_h,
				 int dst_region_x, int dst_region_y,
				 int dst_region_w, int dst_region_h)
{
   evas_common_scale_rgba_in_to_out_clip_sample(src, dst, dc,
				    src_region_x, src_region_y,
				    src_region_w, src_region_h,
				    dst_region_x, dst_region_y,
				    dst_region_w, dst_region_h);
}
#endif
#endif

#ifdef BUILD_SCALE_SAMPLE
EAPI void
evas_common_scale_rgba_in_to_out_clip_sample(RGBA_Image *src, RGBA_Image *dst,
				 RGBA_Draw_Context *dc,
				 int src_region_x, int src_region_y,
				 int src_region_w, int src_region_h,
				 int dst_region_x, int dst_region_y,
				 int dst_region_w, int dst_region_h)
{
   Cutout_Rects *rects;
   Cutout_Rect  *r;
   int          c, cx, cy, cw, ch;
   int          i;
   /* handle cutouts here! */

   if ((dst_region_w <= 0) || (dst_region_h <= 0)) return;
   if (!(RECTS_INTERSECT(dst_region_x, dst_region_y, dst_region_w, dst_region_h, 0, 0, dst->cache_entry.w, dst->cache_entry.h)))
     return;
   /* no cutouts - cut right to the chase */
   if (!dc->cutout.rects)
     {
	scale_rgba_in_to_out_clip_sample_internal(src, dst, dc,
						  src_region_x, src_region_y,
						  src_region_w, src_region_h,
						  dst_region_x, dst_region_y,
						  dst_region_w, dst_region_h);
	return;
     }
   /* save out clip info */
   c = dc->clip.use; cx = dc->clip.x; cy = dc->clip.y; cw = dc->clip.w; ch = dc->clip.h;
   evas_common_draw_context_clip_clip(dc, 0, 0, dst->cache_entry.w, dst->cache_entry.h);
   evas_common_draw_context_clip_clip(dc, dst_region_x, dst_region_y, dst_region_w, dst_region_h);
   /* our clip is 0 size.. abort */
   if ((dc->clip.w <= 0) || (dc->clip.h <= 0))
     {
	dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;
	return;
     }
   rects = evas_common_draw_context_apply_cutouts(dc);
   for (i = 0; i < rects->active; ++i)
     {
	r = rects->rects + i;
	evas_common_draw_context_set_clip(dc, r->x, r->y, r->w, r->h);
	scale_rgba_in_to_out_clip_sample_internal(src, dst, dc,
						  src_region_x, src_region_y,
						  src_region_w, src_region_h,
						  dst_region_x, dst_region_y,
						  dst_region_w, dst_region_h);

     }
   evas_common_draw_context_apply_clear_cutouts(rects);
   /* restore clip info */
   dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;
}

void
scale_rgba_in_to_out_clip_sample_internal(RGBA_Image *src, RGBA_Image *dst,
					  RGBA_Draw_Context *dc,
					  int src_region_x, int src_region_y,
					  int src_region_w, int src_region_h,
					  int dst_region_x, int dst_region_y,
					  int dst_region_w, int dst_region_h)
{
   int      x, y;
   int     *lin_ptr;
   DATA32  *buf, *dptr;
   DATA32 **row_ptr;
   DATA32  *ptr, *dst_ptr, *src_data, *dst_data;
   int      dst_jump;
   int      dst_clip_x, dst_clip_y, dst_clip_w, dst_clip_h;
   int      src_w, src_h, dst_w, dst_h;
   RGBA_Gfx_Func func;

   if (!(RECTS_INTERSECT(dst_region_x, dst_region_y, dst_region_w, dst_region_h, 0, 0, dst->cache_entry.w, dst->cache_entry.h)))
     return;
   if (!(RECTS_INTERSECT(src_region_x, src_region_y, src_region_w, src_region_h, 0, 0, src->cache_entry.w, src->cache_entry.h)))
     return;

   src_w = src->cache_entry.w;
   src_h = src->cache_entry.h;
   dst_w = dst->cache_entry.w;
   dst_h = dst->cache_entry.h;

   src_data = src->image.data;
   dst_data = dst->image.data;

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

   /* allocate scale lookup tables */
   lin_ptr = alloca(dst_clip_w * sizeof(int));
   row_ptr = alloca(dst_clip_h * sizeof(DATA32 *));

   /* figure out dst jump */
   dst_jump = dst_w - dst_clip_w;

   /* figure out dest start ptr */
   dst_ptr = dst_data + dst_clip_x + (dst_clip_y * dst_w);

   if (dc->mul.use)
     func = evas_common_gfx_func_composite_pixel_color_span_get(src, dc->mul.col, dst, dst_clip_w, dc->render_op);
   else
     func = evas_common_gfx_func_composite_pixel_span_get(src, dst, dst_clip_w, dc->render_op);

   if ((dst_region_w == src_region_w) && (dst_region_h == src_region_h))
     {
	ptr = src_data + ((dst_clip_y - dst_region_y + src_region_y) * src_w) + (dst_clip_x - dst_region_x) + src_region_x;
	for (y = 0; y < dst_clip_h; y++)
	  {
	    /* * blend here [clip_w *] ptr -> dst_ptr * */
#ifdef EVAS_SLI
	     if (((y + dst_clip_y) % dc->sli.h) == dc->sli.y)
#endif
	       {
		  func(ptr, NULL, dc->mul.col, dst_ptr, dst_clip_w);
	       }
	    ptr += src_w;
	    dst_ptr += dst_w;
	  }
     }
   else
     {
       /* fill scale tables */
	for (x = 0; x < dst_clip_w; x++)
	    lin_ptr[x] = (((x + dst_clip_x - dst_region_x) * src_region_w) / dst_region_w) + src_region_x;
	for (y = 0; y < dst_clip_h; y++)
	    row_ptr[y] = src_data + (((((y + dst_clip_y - dst_region_y) * src_region_h) / dst_region_h)
			+ src_region_y) * src_w);
	/* scale to dst */
	dptr = dst_ptr;
#ifdef DIRECT_SCALE
	if ((dc->render_op == _EVAS_RENDER_COPY) ||
            ((!src->cache_entry.flags.alpha) &&
	     (!dst->cache_entry.flags.alpha) &&
	     (!dc->mul.use)))
	  {
	     for (y = 0; y < dst_clip_h; y++)
	       {
#ifdef EVAS_SLI
		 if (((y + dst_clip_y) % dc->sli.h) == dc->sli.y)
#endif
		   {
		      dst_ptr = dptr;
		      for (x = 0; x < dst_clip_w; x++)
			{
			   ptr = row_ptr[y] + lin_ptr[x];
			   *dst_ptr = *ptr;
			   dst_ptr++;
			}
		   }
		 dptr += dst_w;
	      }
	  }
	else
#endif
	  {
	    /* a scanline buffer */
	    buf = alloca(dst_clip_w * sizeof(DATA32));
	    for (y = 0; y < dst_clip_h; y++)
	      {
#ifdef EVAS_SLI
		 if (((y + dst_clip_y) % dc->sli.h) == dc->sli.y)
#endif
		   {
		      dst_ptr = buf;
		      for (x = 0; x < dst_clip_w; x++)
			{
			   ptr = row_ptr[y] + lin_ptr[x];
			   *dst_ptr = *ptr;
			   dst_ptr++;
			}
		      /* * blend here [clip_w *] buf -> dptr * */
		      func(buf, NULL, dc->mul.col, dptr, dst_clip_w);
		   }
		dptr += dst_w;
	      }
	  }
     }
}
#else
#ifdef BUILD_SCALE_SMOOTH
EAPI void
evas_common_scale_rgba_in_to_out_clip_sample(RGBA_Image *src, RGBA_Image *dst,
				 RGBA_Draw_Context *dc,
				 int src_region_x, int src_region_y,
				 int src_region_w, int src_region_h,
				 int dst_region_x, int dst_region_y,
				 int dst_region_w, int dst_region_h)
{
   evas_common_scale_rgba_in_to_out_clip_smooth(src, dst, dc,
				    src_region_x, src_region_y,
				    src_region_w, src_region_h,
				    dst_region_x, dst_region_y,
				    dst_region_w, dst_region_h);
}
#endif
#endif

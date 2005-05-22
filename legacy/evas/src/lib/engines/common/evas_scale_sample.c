#include "evas_common.h"

void scale_rgba_in_to_out_clip_sample_internal(RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);

#ifndef BUILD_SCALE_SMOOTH
#ifdef BUILD_SCALE_SAMPLE
void
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
void
evas_common_scale_rgba_in_to_out_clip_sample(RGBA_Image *src, RGBA_Image *dst,
				 RGBA_Draw_Context *dc,
				 int src_region_x, int src_region_y,
				 int src_region_w, int src_region_h,
				 int dst_region_x, int dst_region_y,
				 int dst_region_w, int dst_region_h)
{
   int c, cx, cy, cw, ch;
   Cutout_Rect *rects, *r;
   Evas_Object_List *l;
   /* handle cutouts here! */

   if ((dst_region_w <= 0) || (dst_region_h <= 0)) return;
   if (!(RECTS_INTERSECT(dst_region_x, dst_region_y, dst_region_w, dst_region_h, 0, 0, dst->image->w, dst->image->h)))
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
   evas_common_draw_context_clip_clip(dc, 0, 0, dst->image->w, dst->image->h);
   evas_common_draw_context_clip_clip(dc, dst_region_x, dst_region_y, dst_region_w, dst_region_h);
   /* our clip is 0 size.. abort */
   if ((dc->clip.w <= 0) || (dc->clip.h <= 0))
     {
	dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;
	return;
     }
   rects = evas_common_draw_context_apply_cutouts(dc);
   for (l = (Evas_Object_List *)rects; l; l = l->next)
     {
	r = (Cutout_Rect *)l;
	evas_common_draw_context_set_clip(dc, r->x, r->y, r->w, r->h);
	scale_rgba_in_to_out_clip_sample_internal(src, dst, dc,
						  src_region_x, src_region_y,
						  src_region_w, src_region_h,
						  dst_region_x, dst_region_y,
						  dst_region_w, dst_region_h);

     }
   evas_common_draw_context_apply_free_cutouts(rects);
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

   if (!(RECTS_INTERSECT(dst_region_x, dst_region_y, dst_region_w, dst_region_h, 0, 0, dst->image->w, dst->image->h)))
     return;
   if (!(RECTS_INTERSECT(src_region_x, src_region_y, src_region_w, src_region_h, 0, 0, src->image->w, src->image->h)))
     return;

   src_w = src->image->w;
   src_h = src->image->h;
   dst_w = dst->image->w;
   dst_h = dst->image->h;

   src_data = src->image->data;
   dst_data = dst->image->data;

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
   lin_ptr = malloc(dst_clip_w * sizeof(int));
   if (!lin_ptr) goto no_lin_ptr;
   row_ptr = malloc(dst_clip_h * sizeof(DATA32 *));
   if (!row_ptr) goto no_row_ptr;

   /* figure out dst jump */
   dst_jump = dst_w - dst_clip_w;

   /* figure out dest start ptr */
   dst_ptr = dst_data + dst_clip_x + (dst_clip_y * dst_w);

   if (dc->mod.use)
     {
	Gfx_Func_Blend_Src_Cmod_Dst func;

	func = evas_common_draw_func_blend_cmod_get(src, dst, dst_clip_w);
	/* if 1:1 scale */

	if ((dst_region_w == src_region_w) &&
	    (dst_region_h == src_region_h))
	  {
	     ptr = src_data + ((dst_clip_y - dst_region_y + src_region_y) * src_w) + (dst_clip_x - dst_region_x) + src_region_x;
	     for (y = 0; y < dst_clip_h; y++)
	       {
		  /* * blend here [clip_w *] ptr -> dst_ptr * */
		  func(ptr, dst_ptr, dst_clip_w, dc->mod.r, dc->mod.g, dc->mod.b, dc->mod.a);
		  ptr += src_w;
		  dst_ptr += dst_w;
	       }
	  }
	else
	  {
	     /* a scanline buffer */
	     buf = malloc(dst_clip_w * sizeof(DATA32));
	     if (!buf) goto no_buf;

	     /* fill scale tables */
	     for (x = 0; x < dst_clip_w; x++)
	       lin_ptr[x] = (((x + dst_clip_x - dst_region_x) * src_region_w) / dst_region_w) + src_region_x;
	     for (y = 0; y < dst_clip_h; y++)
	       row_ptr[y] = src_data + (((((y + dst_clip_y - dst_region_y) * src_region_h) / dst_region_h)
					+ src_region_y) * src_w);
	     /* scale to dst */
	     dptr = dst_ptr;
	     for (y = 0; y < dst_clip_h; y++)
	       {
		  dst_ptr = buf;
		  for (x = 0; x < dst_clip_w; x++)
		    {
		       ptr = row_ptr[y] + lin_ptr[x];
		       *dst_ptr = *ptr;
		       dst_ptr++;
		    }
		  /* * blend here [clip_w *] buf -> dptr * */
		  func(buf, dptr, dst_clip_w, dc->mod.r, dc->mod.g, dc->mod.b, dc->mod.a);
		  dptr += dst_w;
	       }
	     free(buf);
	  }
     }
   else if (dc->mul.use)
     {
	Gfx_Func_Blend_Src_Mul_Dst func;

	func = evas_common_draw_func_blend_mul_get(src, dc->mul.col, dst, dst_clip_w);
	/* if 1:1 scale */
	if ((dst_region_w == src_region_w) &&
	    (dst_region_h == src_region_h))
	  {
	     ptr = src_data + ((dst_clip_y - dst_region_y + src_region_y) * src_w) + (dst_clip_x - dst_region_x) + src_region_x;
	     for (y = 0; y < dst_clip_h; y++)
	       {
		  /* * blend here [clip_w *] ptr -> dst_ptr * */
		  func(ptr, dst_ptr, dst_clip_w, dc->mul.col);
		  ptr += src_w;
		  dst_ptr += dst_w;
	       }
	  }
	else
	  {
	     /* a scanline buffer */
	     buf = malloc(dst_clip_w * sizeof(DATA32));
	     if (!buf) goto no_buf;

	     /* fill scale tables */
	     for (x = 0; x < dst_clip_w; x++)
	       lin_ptr[x] = (((x + dst_clip_x - dst_region_x) * src_region_w) / dst_region_w) + src_region_x;
	     for (y = 0; y < dst_clip_h; y++)
	       row_ptr[y] = src_data + (((((y + dst_clip_y - dst_region_y) * src_region_h) / dst_region_h)
					+ src_region_y) * src_w);
	     /* scale to dst */
	     dptr = dst_ptr;
	     for (y = 0; y < dst_clip_h; y++)
	       {
		  dst_ptr = buf;
		  for (x = 0; x < dst_clip_w; x++)
		    {
		       ptr = row_ptr[y] + lin_ptr[x];
		       *dst_ptr = *ptr;
		       dst_ptr++;
		    }
		  /* * blend here [clip_w *] buf -> dptr * */
		  func(buf, dptr, dst_clip_w, dc->mul.col);
		  dptr += dst_w;
	       }
	     free(buf);
	  }
     }
   else
     {
	Gfx_Func_Blend_Src_Dst func;

	func = evas_common_draw_func_blend_get(src, dst, dst_clip_w);
	/* if 1:1 scale */
	if ((dst_region_w == src_region_w) &&
	    (dst_region_h == src_region_h))
	  {
	     ptr = src_data + ((dst_clip_y - dst_region_y + src_region_y) * src_w) + (dst_clip_x - dst_region_x) + src_region_x;
	     for (y = 0; y < dst_clip_h; y++)
	       {
		  /* * blend here [clip_w *] ptr -> dst_ptr * */
		  func(ptr, dst_ptr, dst_clip_w);
		  ptr += src_w;
		  dst_ptr += dst_w;
	       }
	  }
	else
	  {
	     /* a scanline buffer */
	     buf = malloc(dst_clip_w * sizeof(DATA32));
	     if (!buf) goto no_buf;

	     /* fill scale tables */
	     for (x = 0; x < dst_clip_w; x++)
	       lin_ptr[x] = (((x + dst_clip_x - dst_region_x) * src_region_w) / dst_region_w) + src_region_x;
	     for (y = 0; y < dst_clip_h; y++)
	       row_ptr[y] = src_data + (((((y + dst_clip_y - dst_region_y) * src_region_h) / dst_region_h)
					+ src_region_y) * src_w);
	     /* scale to dst */
	     dptr = dst_ptr;
	     for (y = 0; y < dst_clip_h; y++)
	       {
		  dst_ptr = buf;
		  for (x = 0; x < dst_clip_w; x++)
		    {
		       ptr = row_ptr[y] + lin_ptr[x];
		       *dst_ptr = *ptr;
		       dst_ptr++;
		    }
		  /* * blend here [clip_w *] buf -> dptr * */
		  func(buf, dptr, dst_clip_w);
		  dptr += dst_w;
	       }
	     free(buf);
	  }
     }

   no_buf:
   /* free scale tables */
   free(row_ptr);
   no_row_ptr:
   free(lin_ptr);

   //_WIN32_WCE
   no_lin_ptr: ;
}
#else
#ifdef BUILD_SCALE_SMOOTH
void
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

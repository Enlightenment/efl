void
SCALE_FUNC(RGBA_Image *src, RGBA_Image *dst,
	   RGBA_Draw_Context *dc,
	   int src_region_x, int src_region_y,
	   int src_region_w, int src_region_h,
	   int dst_region_x, int dst_region_y,
	   int dst_region_w, int dst_region_h)
{
   int      x, y;
   int     *lin_ptr, *lin2_ptr;
   int     *interp_x, *interp_y;
   int     *sample_x, *sample_y;
   char    *iterate_x, *iterate_y;
   DATA32  *buf, *dptr;
   DATA32 **row_ptr, **row2_ptr;
   DATA32  *ptr, *dst_ptr, *dst_data, *ptr2, *ptr3, *ptr4;
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
	if ((dst_clip_w <= 0) || (dst_clip_h <= 0)) return;
	if ((dst_clip_x + dst_clip_w) > dst_w) dst_clip_w = dst_w - dst_clip_x;
	if ((dst_clip_y + dst_clip_h) > dst_h) dst_clip_h = dst_h - dst_clip_y;
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

   lin_ptr = malloc(dst_clip_w * sizeof(int));
   if (!lin_ptr) goto no_lin_ptr;
   row_ptr = malloc(dst_clip_h * sizeof(DATA32 *));
   if (!row_ptr) goto no_row_ptr;
   lin2_ptr = malloc(dst_clip_w * sizeof(int));
   if (!lin2_ptr) goto no_lin2_ptr;
   row2_ptr = malloc(dst_clip_h * sizeof(DATA32 *));
   if (!row2_ptr) goto no_row2_ptr;
   interp_x = malloc(dst_clip_w * sizeof(int));
   if (!interp_x) goto no_interp_x;
   interp_y = malloc(dst_clip_h * sizeof(int));
   if (!interp_y) goto no_interp_y;
   sample_x = malloc(dst_clip_w * sizeof(int) * 3);
   if (!sample_x) goto no_sample_x;
   sample_y = malloc(dst_clip_h * sizeof(int) * 3);
   if (!sample_y) goto no_sample_y;
   iterate_x = malloc(dst_clip_w * sizeof(char));
   if (!iterate_x) goto no_iterate_x;
   iterate_y = malloc(dst_clip_h * sizeof(char));
   if (!iterate_y) goto no_iterate_y;

   /* figure out dst jump */
   dst_jump = dst_w - dst_clip_w;

   /* figure out dest start ptr */
   dst_ptr = dst_data + dst_clip_x + (dst_clip_y * dst_w);

/* FIXME:
 *
 * things to do later for speedups:
 *
 * break upscale into 3 cases (as listed below - up:up, 1:up, up:1)
 *
 * break downscale into more cases (as listed below)
 *
 * roll func (blend/copy/cultiply/cmod) code into inner loop of scaler.
 * (578 fps vs 550 in mmx upscale in evas demo - this means probably
 *  a good 10-15% speedup over the func call, but means massively larger
 *  code)
 *
 * anything involving downscaling has no mmx equivalent code and maybe the
 * C could do with a little work.
 *
 * ---------------------------------------------------------------------------
 *
 * (1 = no scaling (1:1 ratio), + = scale up, - = scale down)
 * (* == fully optimised mmx, # = fully optimised C)
 *
 * h:v mmx C
 *
 * 1:1 *   #
 *
 * +:+ *   #
 * 1:+ *   #
 * +:1 *   #
 *
 * 1:-
 * -:1
 * +:-
 * -:+
 * -:-
 *
 */

   /* if 1:1 scale */
   if ((dst_region_w == src_region_w) &&
       (dst_region_h == src_region_h))
     {
#include "evas_scale_smooth_scaler_noscale.c"
     }
   else
     {
	Gfx_Func_Blend_Src_Cmod_Dst func_cmod;
	Gfx_Func_Blend_Src_Mul_Dst  func_mul;
	Gfx_Func_Blend_Src_Dst      func;

	/* a scanline buffer */
	buf = malloc(dst_clip_w * sizeof(DATA32));
	if (!buf) goto no_buf;

	func      = evas_common_draw_func_blend_get      (src, dst, dst_clip_w);
	func_cmod = evas_common_draw_func_blend_cmod_get (src, dst, dst_clip_w);
	func_mul  = evas_common_draw_func_blend_mul_get  (src, dc->mul.col, dst, dst_clip_w);

	/* scaling up only - dont need anything except original */
	if ((dst_region_w >= src_region_w) && (dst_region_h >= src_region_h))
	  {
#include "evas_scale_smooth_scaler_up.c"
	  }
	else
	  /* scaling down... funkiness */
	  {
#include "evas_scale_smooth_scaler_down.c"
	  }
	free(buf);
     }
   no_buf:
   /* free scale tables */
   free(iterate_y);
   no_iterate_y:
   free(iterate_x);
   no_iterate_x:
   free(sample_y);
   no_sample_y:
   free(sample_x);
   no_sample_x:
   free(interp_y);
   no_interp_y:
   free(interp_x);
   no_interp_x:
   free(row2_ptr);
   no_row2_ptr:
   free(lin2_ptr);
   no_lin2_ptr:
   free(row_ptr);
   no_row_ptr:
   free(lin_ptr);
   no_lin_ptr:
   ;
}

{
   DATA32  **ypoints;
   int     *xpoints;
   int     *xapoints, *xapp;
   int     *yapoints, *yapp;
   DATA32  *buf, *src_data;

   RGBA_Gfx_Func      func, func2 = NULL;

   src_data = src->image.data;

   /* some maximum region sizes to avoid insane calc point tables */
   SCALE_CALC_X_POINTS(xpoints, src_region_w, dst_region_w, dst_clip_x - dst_region_x, dst_clip_w);
   SCALE_CALC_Y_POINTS(ypoints, src_data, src_w, src_region_h, dst_region_h, dst_clip_y - dst_region_y, dst_clip_h);
   SCALE_CALC_A_POINTS(xapoints, src_region_w, dst_region_w, dst_clip_x - dst_region_x, dst_clip_w);
   SCALE_CALC_A_POINTS(yapoints, src_region_h, dst_region_h, dst_clip_y - dst_region_y, dst_clip_h);

   /* a scanline buffer */
   buf = alloca(dst_clip_w * sizeof(DATA32));

   if (!mask_ie)
     {
        if (mul_col != 0xffffffff)
          func = evas_common_gfx_func_composite_pixel_color_span_get(src->cache_entry.flags.alpha, src->cache_entry.flags.alpha_sparse, mul_col, dst->cache_entry.flags.alpha, dst_clip_w, render_op);
        else
          func = evas_common_gfx_func_composite_pixel_span_get(src->cache_entry.flags.alpha, src->cache_entry.flags.alpha_sparse, dst->cache_entry.flags.alpha, dst_clip_w, render_op);
     }
   else
     {
        if (mul_col != 0xffffffff)
          {
             func = evas_common_gfx_func_composite_pixel_mask_span_get(src->cache_entry.flags.alpha, src->cache_entry.flags.alpha_sparse, dst->cache_entry.flags.alpha, dst_clip_w, render_op);
             func2 = evas_common_gfx_func_composite_pixel_color_span_get(src->cache_entry.flags.alpha, src->cache_entry.flags.alpha_sparse, mul_col, dst->cache_entry.flags.alpha, dst_clip_w, EVAS_RENDER_COPY);
          }
        else
          func = evas_common_gfx_func_composite_pixel_mask_span_get(src->cache_entry.flags.alpha, src->cache_entry.flags.alpha_sparse, dst->cache_entry.flags.alpha, dst_clip_w, render_op);
     }

   /* scaling down vertically */
   if ((dst_region_w >= src_region_w) &&
       (dst_region_h <  src_region_h))
     {
#include "evas_scale_smooth_scaler_downy.c"
     }
   /* scaling down horizontally */
   else if ((dst_region_w < src_region_w) &&
            (dst_region_h >=  src_region_h))
     {
#include "evas_scale_smooth_scaler_downx.c"
     }
   /* scaling down both vertically & horizontally */
   else if ((dst_region_w < src_region_w) &&
            (dst_region_h <  src_region_h))
     {
#include "evas_scale_smooth_scaler_downx_downy.c"
     }
}

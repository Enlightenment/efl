{
   DATA32  **ypoints;
   int     *xpoints;
   int     *xapoints, *xapp;
   int     *yapoints, *yapp;
   DATA32  *buf, *src_data;
   
   RGBA_Gfx_Func      func;

   src_data = src->image->data;

   xpoints = scale_calc_x_points(src_region_w, dst_region_w);
   ypoints = scale_calc_y_points(src_data, src_w, src_region_h, dst_region_h);
   xapoints = scale_calc_a_points(src_region_w, dst_region_w);
   yapoints = scale_calc_a_points(src_region_h, dst_region_h);
   if ( (!xpoints) || (!ypoints) || (!xapoints) || (!yapoints) )
	goto done_scale_down;
   
   /* a scanline buffer */
   buf = alloca(dst_clip_w * sizeof(DATA32));
   
   if (dc->mul.use)
	func = evas_common_gfx_func_composite_pixel_color_span_get(src, dc->mul.col, dst, dst_clip_w, dc->render_op);
   else
	func = evas_common_gfx_func_composite_pixel_span_get(src, dst, dst_clip_w, dc->render_op);
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
     
   done_scale_down:
   if (xpoints) free(xpoints);
   if (ypoints) free(ypoints);
   if (xapoints) free(xapoints);
   if (yapoints) free(yapoints);
}

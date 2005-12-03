{
   DATA32   *sptr;
   int       x, y;
   DATA32  **ypoints;
   int     *xpoints;
   int     *xapoints, *xapp;
   int     *yapoints, *yapp;
   DATA32  *buf, *src_data;
   RGBA_Image  *line_buf;
   
   Gfx_Func_Blend_Src_Cmod_Dst func_cmod;
   Gfx_Func_Blend_Src_Mul_Dst  func_mul;
   Gfx_Func_Blend_Src_Dst      func;

   src_data = src->image->data;

   xpoints = scale_calc_x_points(src_region_w, dst_region_w);
   ypoints = scale_calc_y_points(src_data, src_w, src_region_h, dst_region_h);
   xapoints = scale_calc_a_points(src_region_w, dst_region_w);
   yapoints = scale_calc_a_points(src_region_h, dst_region_h);
   if ( (!xpoints) || (!ypoints) || (!xapoints) || (!yapoints) )
	goto done_scale_down;
   
   /* a scanline buffer */
   line_buf = evas_common_image_line_buffer_obtain(dst_clip_w);
   if (!line_buf)
      goto done_scale_down;
   buf = line_buf->image->data;
   
   func      = evas_common_draw_func_blend_get      (src, dst, dst_clip_w);
   func_cmod = evas_common_draw_func_blend_cmod_get (src, dst, dst_clip_w);
   func_mul  = evas_common_draw_func_blend_mul_get  (src, dc->mul.col, dst, dst_clip_w);

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
   evas_common_image_line_buffer_release();
   if (xpoints) free(xpoints);
   if (ypoints) free(ypoints);
   if (xapoints) free(xapoints);
   if (yapoints) free(yapoints);
}

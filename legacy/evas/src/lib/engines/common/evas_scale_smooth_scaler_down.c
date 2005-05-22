#define RGBA_COMPOSE(r, g, b, a)  ((a) << 24) | ((r) << 16) | ((g) << 8) | (b)
#define INV_XAP                   (256 - xapoints[dxx + x])
#define XAP                       (xapoints[dxx + x])
#define INV_YAP                   (256 - yapoints[dyy + y])
#define YAP                       (yapoints[dyy + y])

{
   DATA32 *sptr;
   int x, y;
   DATA32 **ypoints;
   int *xpoints;
   int *xapoints;
   int *yapoints;
   DATA32 *src_data;

   src_data = src->image->data;

   xpoints = scale_calc_x_points(src_region_w, dst_region_w);
   ypoints = scale_calc_y_points(src_data, src->image->w, src_region_h, dst_region_h);
   xapoints = scale_calc_a_points(src_region_w, dst_region_w);
   yapoints = scale_calc_a_points(src_region_h, dst_region_h);
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
   free(xpoints);
   free(ypoints);
   free(xapoints);
   free(yapoints);
}

#include "evas_common.h"
#include "evas_private.h"
#include <math.h>

EAPI void
evas_common_transform_affine_rect_bounds_get(Evas_Common_Transform *t,
                                             int x, int y, int w, int h,
                                             int *x_out, int *y_out, int *w_out, int *h_out)
{
   double d;
   int   axx, axy, ayx, ayy;
   int   z, x0, x1, y0, y1;

   evas_common_cpu_end_opt();  // work damn it!

   if (!t || !x_out || !y_out || !w_out || !h_out)  return;
   if (t->is_identity)
     {
	*x_out = x;  *y_out = y;
	*w_out = w;  *h_out = h;
	return;
     }
   d = (t->mxx * (double)t->myy) - (t->mxy * (double)t->myx);
   if (fabs(d) < 0.000030517578125)
     {
	*x_out = 0;  *y_out = 0;
	*w_out = 0;  *h_out = 0;
	return;
     }
   d = 1.0 / d;
   axx = d * t->myy * 65536;  ayy = d * t->mxx * 65536;
   axy = -d * t->mxy * 65536;  ayx = -d * t->myx * 65536;

   x0 = x1 = (axx * x) + (axy * y);
   z = (axx * (x + w)) + (axy * y);
   if (z < x0)  x0 = z;
   if (z > x1)  x1 = z;
   z = (axx * x) + (axy * (y + h));
   if (z < x0)  x0 = z;
   if (z > x1)  x1 = z;
   z = (axx * (x + w)) + (axy * (y + h));
   if (z < x0)  x0 = z;
   if (z > x1)  x1 = z;
   z = x0 - (((axx * (t->mxz - 0.5)) + (axy * (t->myz - 0.5)))) - 32768;
   *x_out = z >> 16;
   *w_out = ((x1 + 65535) >> 16) - ((x0 - 65535) >> 16);  // kludgy, but...

   y0 = y1 = (ayx * x) + (ayy * y);
   z = (ayx * (x + w)) + (ayy * y);
   if (z < y0)  y0 = z;
   if (z > y1)  y1 = z;
   z = (ayx * x) + (ayy * (y + h));
   if (z < y0)  y0 = z;
   if (z > y1)  y1 = z;
   z = (ayx * (x + w)) + (ayy * (y + h));
   if (z < y0)  y0 = z;
   if (z > y1)  y1 = z;
   z = y0 - ((ayx * (t->mxz - 0.5)) + (ayy * (t->myz - 0.5))) - 32768;
   *y_out = z >> 16;
   *h_out = ((y1 + 65535) >> 16) - ((y0 - 65535) >> 16);  // kludgy, but...
}

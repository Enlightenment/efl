#include "evas_common.h"
#include "evas_private.h"
#include <math.h>

EAPI void
evas_transform_identity_set(Evas_Transform *t)
{
   if (t)
     {
	t->mxx = t->myy = t->mzz = 1;
	t->mxy = t->mxz = 0;
	t->myx = t->myz = 0;
	t->mzx = t->mzy = 0;
     }
}

EAPI void
evas_transform_rotate(double angle, Evas_Transform *t)
{
   double ca, sa;
   float mxx, mxy, mxz;
   float myx, myy, myz;

   if (!t) return;
   if (angle == 0) return;

   angle = (-angle * M_PI) / 180.0;
   ca = cos(angle);  sa = sin(angle);
   if ((ca == 1) && (sa == 0)) return;

   mxx = t->mxx;  mxy = t->mxy;  mxz = t->mxz;
   myx = t->myx;  myy = t->myy;  myz = t->myz;

   t->mxx = (ca * mxx) + (sa * myx);
   t->mxy = (ca * mxy) + (sa * myy);
   t->mxz = (ca * mxz) + (sa * myz);

   t->myx = (-sa * mxx) + (ca * myx);
   t->myy = (-sa * mxy) + (ca * myy);
   t->myz = (-sa * mxz) + (ca * myz);
}

EAPI void
evas_transform_translate(float dx, float dy, Evas_Transform *t)
{
   if (!t) return;
   if (!dx && !dy) return;

   t->mxx += dx * t->mzx;
   t->mxy += dx * t->mzy;
   t->mxz += dx * t->mzz;

   t->myx += dy * t->mzx;
   t->myy += dy * t->mzy;
   t->myz += dy * t->mzz;
}

EAPI void
evas_transform_scale(float sx, float sy, Evas_Transform *t)
{
   if (!t) return;
   if ((sx == 1) && (sy == 1)) return;

   t->mxx = sx * t->mxx;
   t->mxy = sx * t->mxy;
   t->mxz = sx * t->mxz;

   t->myx = sy * t->myx;
   t->myy = sy * t->myy;
   t->myz = sy * t->myz;
}

EAPI void
evas_transform_shear(float sh, float sv, Evas_Transform *t)
{
   float mxx, mxy, mxz;
   float myx, myy, myz;

   if (!t) return;
   if ((sh == 1) && (sv == 1)) return;

   mxx = t->mxx;  mxy = t->mxy;  mxz = t->mxz;
   myx = t->myx;  myy = t->myy;  myz = t->myz;

   t->mxx = mxx + (sh * myx);
   t->mxy = mxy + (sh * myy);
   t->mxz = mxz + (sh * myz);

   t->myx = (sv * mxx) + myx;
   t->myy = (sv * mxy) + myy;
   t->myz = (sv * mxz) + myz;
}

EAPI void
evas_transform_compose(Evas_Transform *l, Evas_Transform *t)
{
   float mxx, mxy, mxz;
   float myx, myy, myz;
   float mzx, mzy, mzz;

   if (!t || !l) return;

   mxx = t->mxx;  mxy = t->mxy;  mxz = t->mxz;
   myx = t->myx;  myy = t->myy;  myz = t->myz;
   mzx = t->mzx;  mzy = t->mzy;  mzz = t->mzz;

   t->mxx = (l->mxx * mxx) + (l->mxy * myx) + (l->mxz * mzx);
   t->mxy = (l->mxx * mxy) + (l->mxy * myy) + (l->mxz * mzy);
   t->mxz = (l->mxx * mxz) + (l->mxy * myz) + (l->mxz * mzz);

   t->myx = (l->myx * mxx) + (l->myy * myx) + (l->myz * mzx);
   t->myy = (l->myx * mxy) + (l->myy * myy) + (l->myz * mzy);
   t->myz = (l->myx * mxz) + (l->myy * myz) + (l->myz * mzz);

   t->mzx = (l->mzx * mxx) + (l->mzy * myx) + (l->mzz * mzx);
   t->mzy = (l->mzx * mxy) + (l->mzy * myy) + (l->mzz * mzy);
   t->mzz = (l->mzx * mxz) + (l->mzy * myz) + (l->mzz * mzz);
}

EAPI void
evas_transform_affine_rect_bounds_get(Evas_Transform *t, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Evas_Coord *rx, Evas_Coord *ry, Evas_Coord *rw, Evas_Coord *rh)
{
   double d;
   int axx, axy, ayx, ayy;

   if (!t)  return;
   if ( (t->mxx == 1) && (t->myy == 1) && 
        (t->mxy == 0) && (t->myx == 0) &&
        (t->mxz == 0) && (t->myz == 0) )
     {
	if (rx) *rx = x;
	if (ry) *ry = y;
	if (rw) *rw = w;
	if (rh) *rh = h;
	return;
     }
   d = (t->mxx * (double)t->myy) - (t->mxy * (double)t->myx);
   if (fabs(d) < 0.000030517578125)
     {
	if (rx) *rx = 0;
	if (ry) *ry = 0;
	if (rw) *rw = 0;
	if (rh) *rh = 0;
	return;
     }
   d = 1.0 / d;
   axx = d * t->myy * 65536;  ayy = d * t->mxx * 65536;
   axy = -d * t->mxy * 65536;  ayx = -d * t->myx * 65536;

   if (rx || rh)
     {
	int z, x0, x1 = x0 = (axx * x) + (axy * y);

	z = (axx * (x + w)) + (axy * y);
	if (z < x0)  x0 = z;
	if (z > x1)  x1 = z;
	z = (axx * x) + (axy * (y + h));
	if (z < x0)  x0 = z;
	if (z > x1)  x1 = z;
	z = (axx * (x + w)) + (axy * (y + h));
	if (z < x0)  x0 = z;
	if (z > x1)  x1 = z;
	if (rx)
	  {
	    z = x0 - (((axx * (t->mxz - 0.5)) + (axy * (t->myz - 0.5)))) - 32768;
	    *rx = z >> 16;
	  }
	if (rw)
	   *rw = ((x1 + 0xffff) >> 16) - ((x0 - 0xffff) >> 16);  // kludgy, but...
     }
   if (rx || rh)
     {
	int z, y0, y1 = y0 = (ayx * x) + (ayy * y);

	z = (ayx * (x + w)) + (ayy * y);
	if (z < y0)  y0 = z;
	if (z > y1)  y1 = z;
	z = (ayx * x) + (ayy * (y + h));
	if (z < y0)  y0 = z;
	if (z > y1)  y1 = z;
	z = (ayx * (x + w)) + (ayy * (y + h));
	if (z < y0)  y0 = z;
	if (z > y1)  y1 = z;
	if (ry)
	  {
	    z = y0 - ((ayx * (t->mxz - 0.5)) + (ayy * (t->myz - 0.5))) - 32768;
	    *ry = z >> 16;
	  }
	if (rh)
	   *rh = ((y1 + 0xffff) >> 16) - ((y0 - 0xffff) >> 16);  // kludgy, but...
     }
}

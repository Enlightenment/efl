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

   angle = (angle * M_PI) / 180.0;
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

   t->mxx = dx * t->mzx;
   t->mxy = dx * t->mzy;
   t->mxz = dx * t->mzz;

   t->myx = dy * t->mzx;
   t->myy = dy * t->mzy;
   t->myz = dy * t->mzz;
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

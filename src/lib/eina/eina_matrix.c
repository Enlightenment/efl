/* EINA - Drawing Library
 * Copyright (C) 2007-2014 Jorge Luis Zapata
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eina_private.h"

#include <math.h>
#include <float.h>
#include <string.h>

#include "eina_fp.h"
#include "eina_rectangle.h"
#include "eina_quad.h"
#include "eina_matrix.h"
#include "eina_util.h"

#define MATRIX_XX(m) (m)->xx
#define MATRIX_XY(m) (m)->xy
#define MATRIX_XZ(m) (m)->xz
#define MATRIX_XW(m) (m)->xw
#define MATRIX_YX(m) (m)->yx
#define MATRIX_YY(m) (m)->yy
#define MATRIX_YZ(m) (m)->yz
#define MATRIX_YW(m) (m)->yw
#define MATRIX_ZX(m) (m)->zx
#define MATRIX_ZY(m) (m)->zy
#define MATRIX_ZZ(m) (m)->zz
#define MATRIX_ZW(m) (m)->zw
#define MATRIX_WX(m) (m)->wx
#define MATRIX_WY(m) (m)->wy
#define MATRIX_WZ(m) (m)->wz
#define MATRIX_WW(m) (m)->ww
#define MATRIX_SIZE 9

#define QUAD_X0(q) q->x0
#define QUAD_Y0(q) q->y0
#define QUAD_X1(q) q->x1
#define QUAD_Y1(q) q->y1
#define QUAD_X2(q) q->x2
#define QUAD_Y2(q) q->y2
#define QUAD_X3(q) q->x3
#define QUAD_Y3(q) q->y3

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/
/** @cond internal */
/*
 * In the range [-pi pi]
 * (4/pi)*x - ((4/(pi*pi))*x*abs(x))
 * http://www.devmaster.net/forums/showthread.php?t=5784
 */
#define EXTRA_PRECISION
static inline double
  _sin(double x)
{
   const double B = 4/M_PI;
   const double C = -4/(M_PI*M_PI);

   double y = (B * x) + (C * x * fabs(x));

#ifdef EXTRA_PRECISION
   //  const float Q = 0.775;
   const double P = 0.225;

   y = P * (y * fabs(y) - y) + y; // Q * y + P * y * abs(y)
#endif
   return y;
}

static inline double
_cos(double x)
{
   x += M_PI_2;

   if (x > M_PI)   // Original x > pi/2
     {
        x -= 2 * M_PI;   // Wrap: cos(x) = cos(x - 2 pi)
     }

   return _sin(x);
}
/** @endcond */

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
EAPI Eina_Matrix_Type
eina_matrix3_type_get(const Eina_Matrix3 *m)
{
   if (!EINA_DBL_EQ(MATRIX_ZX(m), 0.0) ||
       !EINA_DBL_EQ(MATRIX_ZY(m), 0.0) ||
       !EINA_DBL_EQ(MATRIX_ZZ(m), 1.0))
     return EINA_MATRIX_TYPE_PROJECTIVE;
   else
     {
        if (EINA_DBL_EQ(MATRIX_XX(m), 1.0) &&
            EINA_DBL_EQ(MATRIX_XY(m), 0.0) &&
            EINA_DBL_EQ(MATRIX_XZ(m), 0.0) &&
            EINA_DBL_EQ(MATRIX_YX(m), 0.0) &&
            EINA_DBL_EQ(MATRIX_YY(m), 1.0) &&
            EINA_DBL_EQ(MATRIX_YZ(m), 0.0))
          return EINA_MATRIX_TYPE_IDENTITY;
        else
          return EINA_MATRIX_TYPE_AFFINE;
     }
}

EAPI Eina_Matrix_Type
eina_matrix4_type_get(const Eina_Matrix4 *m)
{
   if (EINA_DBL_EQ(MATRIX_XX(m), 1.0) &&
       EINA_DBL_EQ(MATRIX_XY(m), 0.0) &&
       EINA_DBL_EQ(MATRIX_XZ(m), 0.0) &&
       EINA_DBL_EQ(MATRIX_XW(m), 0.0) &&
       EINA_DBL_EQ(MATRIX_YX(m), 0.0) &&
       EINA_DBL_EQ(MATRIX_YY(m), 1.0) &&
       EINA_DBL_EQ(MATRIX_YZ(m), 0.0) &&
       EINA_DBL_EQ(MATRIX_YW(m), 0.0) &&
       EINA_DBL_EQ(MATRIX_ZX(m), 0.0) &&
       EINA_DBL_EQ(MATRIX_ZY(m), 0.0) &&
       EINA_DBL_EQ(MATRIX_ZZ(m), 1.0) &&
       EINA_DBL_EQ(MATRIX_ZW(m), 0.0) &&
       EINA_DBL_EQ(MATRIX_WX(m), 0.0) &&
       EINA_DBL_EQ(MATRIX_WY(m), 0.0) &&
       EINA_DBL_EQ(MATRIX_WZ(m), 0.0) &&
       EINA_DBL_EQ(MATRIX_WW(m), 1.0))
     return EINA_MATRIX_TYPE_IDENTITY;
   return EINA_MATRIX_TYPE_AFFINE;
}

EAPI Eina_Matrix_Type
eina_matrix3_f16p16_type_get(const Eina_Matrix3_F16p16 *m)
{
   if ((MATRIX_ZX(m) != 0) || (MATRIX_ZY(m) != 0) || (MATRIX_ZZ(m) != 65536))
     return EINA_MATRIX_TYPE_PROJECTIVE;
   else
     {
        if ((MATRIX_XX(m) == 65536) && (MATRIX_XY(m) == 0) && (MATRIX_XZ(m) == 0) &&
            (MATRIX_YX(m) == 0) && (MATRIX_YY(m) == 65536) && (MATRIX_YZ(m) == 0))
          return EINA_MATRIX_TYPE_IDENTITY;
        else
          return EINA_MATRIX_TYPE_AFFINE;
     }
}

EAPI void
eina_matrix3_values_set(Eina_Matrix3 *m,
                        double xx, double xy, double xz,
                        double yx, double yy, double yz,
                        double zx, double zy, double zz)
{
   MATRIX_XX(m) = xx;
   MATRIX_XY(m) = xy;
   MATRIX_XZ(m) = xz;
   MATRIX_YX(m) = yx;
   MATRIX_YY(m) = yy;
   MATRIX_YZ(m) = yz;
   MATRIX_ZX(m) = zx;
   MATRIX_ZY(m) = zy;
   MATRIX_ZZ(m) = zz;
}

EAPI void
eina_matrix3_values_get(const Eina_Matrix3 *m,
                        double *xx, double *xy, double *xz,
                        double *yx, double *yy, double *yz,
                        double *zx, double *zy, double *zz)
{
   if (xx) *xx = MATRIX_XX(m);
   if (xy) *xy = MATRIX_XY(m);
   if (xz) *xz = MATRIX_XZ(m);
   if (yx) *yx = MATRIX_YX(m);
   if (yy) *yy = MATRIX_YY(m);
   if (yz) *yz = MATRIX_YZ(m);
   if (zx) *zx = MATRIX_ZX(m);
   if (zy) *zy = MATRIX_ZY(m);
   if (zz) *zz = MATRIX_ZZ(m);
}

EAPI void
eina_matrix4_values_set(Eina_Matrix4 *m,
                        double xx, double xy, double xz, double xw,
                        double yx, double yy, double yz, double yw,
                        double zx, double zy, double zz, double zw,
                        double wx, double wy, double wz, double ww)
{
   MATRIX_XX(m) = xx;
   MATRIX_XY(m) = xy;
   MATRIX_XZ(m) = xz;
   MATRIX_XW(m) = xw;
   MATRIX_YX(m) = yx;
   MATRIX_YY(m) = yy;
   MATRIX_YZ(m) = yz;
   MATRIX_YW(m) = yw;
   MATRIX_ZX(m) = zx;
   MATRIX_ZY(m) = zy;
   MATRIX_ZZ(m) = zz;
   MATRIX_ZW(m) = zw;
   MATRIX_WX(m) = wx;
   MATRIX_WY(m) = wy;
   MATRIX_WZ(m) = wz;
   MATRIX_WW(m) = ww;
}

EAPI void
eina_matrix4_values_get(const Eina_Matrix4 *m,
                        double *xx, double *xy, double *xz, double *xw,
                        double *yx, double *yy, double *yz, double *yw,
                        double *zx, double *zy, double *zz, double *zw,
                        double *wx, double *wy, double *wz, double *ww)
{
   if (xx) *xx = MATRIX_XX(m);
   if (xy) *xy = MATRIX_XY(m);
   if (xz) *xz = MATRIX_XZ(m);
   if (xw) *xw = MATRIX_XW(m);
   if (yx) *yx = MATRIX_YX(m);
   if (yy) *yy = MATRIX_YY(m);
   if (yz) *yz = MATRIX_YZ(m);
   if (yw) *yw = MATRIX_YW(m);
   if (zx) *zx = MATRIX_ZX(m);
   if (zy) *zy = MATRIX_ZY(m);
   if (zz) *zz = MATRIX_ZZ(m);
   if (zw) *zw = MATRIX_ZW(m);
   if (wx) *wx = MATRIX_WX(m);
   if (wy) *wy = MATRIX_WY(m);
   if (wz) *wz = MATRIX_WZ(m);
   if (ww) *ww = MATRIX_WW(m);
}

EAPI void
eina_matrix3_fixed_values_get(const Eina_Matrix3 *m,
                              Eina_F16p16 *xx, Eina_F16p16 *xy, Eina_F16p16 *xz,
                              Eina_F16p16 *yx, Eina_F16p16 *yy, Eina_F16p16 *yz,
                              Eina_F16p16 *zx, Eina_F16p16 *zy, Eina_F16p16 *zz)
{
   if (xx) *xx = eina_f16p16_double_from(MATRIX_XX(m));
   if (xy) *xy = eina_f16p16_double_from(MATRIX_XY(m));
   if (xz) *xz = eina_f16p16_double_from(MATRIX_XZ(m));
   if (yx) *yx = eina_f16p16_double_from(MATRIX_YX(m));
   if (yy) *yy = eina_f16p16_double_from(MATRIX_YY(m));
   if (yz) *yz = eina_f16p16_double_from(MATRIX_YZ(m));
   if (zx) *zx = eina_f16p16_double_from(MATRIX_ZX(m));
   if (zy) *zy = eina_f16p16_double_from(MATRIX_ZY(m));
   if (zz) *zz = eina_f16p16_double_from(MATRIX_ZZ(m));
}

EAPI void
eina_matrix3_matrix3_f16p16_to(const Eina_Matrix3 *m,
                               Eina_Matrix3_F16p16 *fm)
{
   eina_matrix3_fixed_values_get(m,
                                 &fm->xx, &fm->xy, &fm->xz,
                                 &fm->yx, &fm->yy, &fm->yz,
                                 &fm->zx, &fm->zy, &fm->zz);
}

EAPI void
eina_matrix3_point_transform(const Eina_Matrix3 *m,
                             double x, double y,
                             double *xr, double *yr)
{
   double xrr, yrr;

   if (!EINA_DBL_EQ(MATRIX_ZX(m), 0.0) &&
       !EINA_DBL_EQ(MATRIX_ZY(m), 0.0))
     {
        xrr = (x * MATRIX_XX(m) + y * MATRIX_XY(m) + MATRIX_XZ(m));
        yrr = (x * MATRIX_YX(m) + y * MATRIX_YY(m) + MATRIX_YZ(m));
     }
   else
     {
        xrr = (x * MATRIX_XX(m) + y * MATRIX_XY(m) + MATRIX_XZ(m)) /
          (x * MATRIX_ZX(m) + y * MATRIX_ZY(m) + MATRIX_ZZ(m));
        yrr = (x * MATRIX_YX(m) + y * MATRIX_YY(m) + MATRIX_YZ(m)) /
          (x * MATRIX_ZX(m) + y * MATRIX_ZY(m) + MATRIX_ZZ(m));
     }

   if (xr) *xr = xrr;
   if (yr) *yr = yrr;
}

EAPI void
eina_matrix3_rectangle_transform(const Eina_Matrix3 *m,
                                 const Eina_Rectangle *r,
                                 const Eina_Quad *q)
{
   eina_matrix3_point_transform(m, r->x, r->y, &((Eina_Quad *)q)->x0, &((Eina_Quad *)q)->y0);
   eina_matrix3_point_transform(m, r->x + r->w, r->y, &((Eina_Quad *)q)->x1, &((Eina_Quad *)q)->y1);
   eina_matrix3_point_transform(m, r->x + r->w, r->y + r->h, &((Eina_Quad *)q)->x2, &((Eina_Quad *)q)->y2);
   eina_matrix3_point_transform(m, r->x, r->y + r->h, &((Eina_Quad *)q)->x3, &((Eina_Quad *)q)->y3);
}

EAPI void
eina_matrix3_cofactor(const Eina_Matrix3 *m, Eina_Matrix3 *a)
{
   double a11, a12, a13, a21, a22, a23, a31, a32, a33;

   a11 = (MATRIX_YY(m) * MATRIX_ZZ(m)) - (MATRIX_YZ(m) * MATRIX_ZY(m));
   a12 = -1 * ((MATRIX_YX(m) * MATRIX_ZZ(m)) - (MATRIX_YZ(m) * MATRIX_ZX(m)));
   a13 = (MATRIX_YX(m) * MATRIX_ZY(m)) - (MATRIX_YY(m) * MATRIX_ZX(m));

   a21 = -1 * ((MATRIX_XY(m) * MATRIX_ZZ(m)) - (MATRIX_XZ(m) * MATRIX_ZY(m)));
   a22 = (MATRIX_XX(m) * MATRIX_ZZ(m)) - (MATRIX_XZ(m) * MATRIX_ZX(m));
   a23 = -1 * ((MATRIX_XX(m) * MATRIX_ZY(m)) - (MATRIX_XY(m) * MATRIX_ZX(m)));

   a31 = (MATRIX_XY(m) * MATRIX_YZ(m)) - (MATRIX_XZ(m) * MATRIX_YY(m));
   a32 = -1 * ((MATRIX_XX(m) * MATRIX_YZ(m)) - (MATRIX_XZ(m) * MATRIX_YX(m)));
   a33 = (MATRIX_XX(m) * MATRIX_YY(m)) - (MATRIX_XY(m) * MATRIX_YX(m));

   MATRIX_XX(a) = a11;
   MATRIX_XY(a) = a12;
   MATRIX_XZ(a) = a13;

   MATRIX_YX(a) = a21;
   MATRIX_YY(a) = a22;
   MATRIX_YZ(a) = a23;

   MATRIX_ZX(a) = a31;
   MATRIX_ZY(a) = a32;
   MATRIX_ZZ(a) = a33;
}

EAPI void
eina_matrix3_transpose(const Eina_Matrix3 *m, Eina_Matrix3 *a)
{
   MATRIX_XX(a) = MATRIX_XX(m);
   MATRIX_XY(a) = MATRIX_YX(m);
   MATRIX_XZ(a) = MATRIX_ZX(m);

   MATRIX_YX(a) = MATRIX_XY(m);
   MATRIX_YY(a) = MATRIX_YY(m);
   MATRIX_YZ(a) = MATRIX_ZY(m);

   MATRIX_ZX(a) = MATRIX_XZ(m);
   MATRIX_ZY(a) = MATRIX_YZ(m);
   MATRIX_ZZ(a) = MATRIX_ZZ(m);
}

EAPI void
eina_matrix3_adjoint(const Eina_Matrix3 *m, Eina_Matrix3 *a)
{
   Eina_Matrix3 cofactor;

   /* cofactor */
   eina_matrix3_cofactor(m, &cofactor);
   /* transpose */
   eina_matrix3_transpose(&cofactor, a);
}

EAPI double
eina_matrix3_determinant(const Eina_Matrix3 *m)
{
   double det;

   det = MATRIX_XX(m) * ((MATRIX_YY(m) * MATRIX_ZZ(m)) - (MATRIX_YZ(m) * MATRIX_ZY(m)));
   det -= MATRIX_XY(m) * ((MATRIX_YX(m) * MATRIX_ZZ(m)) - (MATRIX_YZ(m) * MATRIX_ZX(m)));
   det += MATRIX_XZ(m) * ((MATRIX_YX(m) * MATRIX_ZY(m)) - (MATRIX_YY(m) * MATRIX_ZX(m)));

   return det;
}

EAPI void
eina_matrix3_divide(Eina_Matrix3 *m, double scalar)
{
   MATRIX_XX(m) /= scalar;
   MATRIX_XY(m) /= scalar;
   MATRIX_XZ(m) /= scalar;

   MATRIX_YX(m) /= scalar;
   MATRIX_YY(m) /= scalar;
   MATRIX_YZ(m) /= scalar;

   MATRIX_ZX(m) /= scalar;
   MATRIX_ZY(m) /= scalar;
   MATRIX_ZZ(m) /= scalar;
}

EAPI void
eina_matrix3_inverse(const Eina_Matrix3 *m, Eina_Matrix3 *m2)
{
   double scalar;

   /* determinant */
   scalar = eina_matrix3_determinant(m);
   if (EINA_DBL_EQ(scalar, 0.0))
     {
        eina_matrix3_identity(m2);
        return;
     }
   /* do its adjoint */
   eina_matrix3_adjoint(m, m2);
   /* divide */
   eina_matrix3_divide(m2, scalar);
}

EAPI void
eina_matrix3_compose(const Eina_Matrix3 *m1,
                     const Eina_Matrix3 *m2,
                     Eina_Matrix3 *dst)
{
   double a11, a12, a13, a21, a22, a23, a31, a32, a33;

   a11 = (MATRIX_XX(m1) * MATRIX_XX(m2)) + (MATRIX_XY(m1) * MATRIX_YX(m2)) + (MATRIX_XZ(m1) * MATRIX_ZX(m2));
   a12 = (MATRIX_XX(m1) * MATRIX_XY(m2)) + (MATRIX_XY(m1) * MATRIX_YY(m2)) + (MATRIX_XZ(m1) * MATRIX_ZY(m2));
   a13 = (MATRIX_XX(m1) * MATRIX_XZ(m2)) + (MATRIX_XY(m1) * MATRIX_YZ(m2)) + (MATRIX_XZ(m1) * MATRIX_ZZ(m2));

   a21 = (MATRIX_YX(m1) * MATRIX_XX(m2)) + (MATRIX_YY(m1) * MATRIX_YX(m2)) + (MATRIX_YZ(m1) * MATRIX_ZX(m2));
   a22 = (MATRIX_YX(m1) * MATRIX_XY(m2)) + (MATRIX_YY(m1) * MATRIX_YY(m2)) + (MATRIX_YZ(m1) * MATRIX_ZY(m2));
   a23 = (MATRIX_YX(m1) * MATRIX_XZ(m2)) + (MATRIX_YY(m1) * MATRIX_YZ(m2)) + (MATRIX_YZ(m1) * MATRIX_ZZ(m2));

   a31 = (MATRIX_ZX(m1) * MATRIX_XX(m2)) + (MATRIX_ZY(m1) * MATRIX_YX(m2)) + (MATRIX_ZZ(m1) * MATRIX_ZX(m2));
   a32 = (MATRIX_ZX(m1) * MATRIX_XY(m2)) + (MATRIX_ZY(m1) * MATRIX_YY(m2)) + (MATRIX_ZZ(m1) * MATRIX_ZY(m2));
   a33 = (MATRIX_ZX(m1) * MATRIX_XZ(m2)) + (MATRIX_ZY(m1) * MATRIX_YZ(m2)) + (MATRIX_ZZ(m1) * MATRIX_ZZ(m2));

   MATRIX_XX(dst) = a11;
   MATRIX_XY(dst) = a12;
   MATRIX_XZ(dst) = a13;
   MATRIX_YX(dst) = a21;
   MATRIX_YY(dst) = a22;
   MATRIX_YZ(dst) = a23;
   MATRIX_ZX(dst) = a31;
   MATRIX_ZY(dst) = a32;
   MATRIX_ZZ(dst) = a33;
}

EAPI Eina_Bool
eina_matrix3_equal(const Eina_Matrix3 *m1, const Eina_Matrix3 *m2)
{
   if (!EINA_DBL_EQ(m1->xx, m2->xx) ||
       !EINA_DBL_EQ(m1->xy, m2->xy) ||
       !EINA_DBL_EQ(m1->xz, m2->xz) ||
       !EINA_DBL_EQ(m1->yx, m2->yx) ||
       !EINA_DBL_EQ(m1->yy, m2->yy) ||
       !EINA_DBL_EQ(m1->yz, m2->yz) ||
       !EINA_DBL_EQ(m1->zx, m2->zx) ||
       !EINA_DBL_EQ(m1->zy, m2->zy) ||
       !EINA_DBL_EQ(m1->zz, m2->zz))
     return EINA_FALSE;
   return EINA_TRUE;
}

EAPI void
eina_matrix3_f16p16_compose(const Eina_Matrix3_F16p16 *m1,
                            const Eina_Matrix3_F16p16 *m2,
                            Eina_Matrix3_F16p16 *dst)
{
   Eina_F16p16 a11, a12, a13, a21, a22, a23, a31, a32, a33;

   a11 = eina_f16p16_mul(MATRIX_XX(m1), MATRIX_XX(m2)) +
     eina_f16p16_mul(MATRIX_XY(m1), MATRIX_YX(m2)) +
     eina_f16p16_mul(MATRIX_XZ(m1), MATRIX_ZX(m2));
   a12 = eina_f16p16_mul(MATRIX_XX(m1), MATRIX_XY(m2)) +
     eina_f16p16_mul(MATRIX_XY(m1), MATRIX_YY(m2)) +
     eina_f16p16_mul(MATRIX_XZ(m1), MATRIX_ZY(m2));
   a13 = eina_f16p16_mul(MATRIX_XX(m1), MATRIX_XZ(m2)) +
     eina_f16p16_mul(MATRIX_XY(m1), MATRIX_YZ(m2)) +
     eina_f16p16_mul(MATRIX_XZ(m1), MATRIX_ZZ(m2));

   a21 = eina_f16p16_mul(MATRIX_YX(m1), MATRIX_XX(m2)) +
     eina_f16p16_mul(MATRIX_YY(m1), MATRIX_YX(m2)) +
     eina_f16p16_mul(MATRIX_YZ(m1), MATRIX_ZX(m2));
   a22 = eina_f16p16_mul(MATRIX_YX(m1), MATRIX_XY(m2)) +
     eina_f16p16_mul(MATRIX_YY(m1), MATRIX_YY(m2)) +
     eina_f16p16_mul(MATRIX_YZ(m1), MATRIX_ZY(m2));
   a23 = eina_f16p16_mul(MATRIX_YX(m1), MATRIX_XZ(m2)) +
     eina_f16p16_mul(MATRIX_YY(m1), MATRIX_YZ(m2)) +
     eina_f16p16_mul(MATRIX_YZ(m1), MATRIX_ZZ(m2));

   a31 = eina_f16p16_mul(MATRIX_ZX(m1), MATRIX_XX(m2)) +
     eina_f16p16_mul(MATRIX_ZY(m1), MATRIX_YX(m2)) +
     eina_f16p16_mul(MATRIX_ZZ(m1), MATRIX_ZX(m2));
   a32 = eina_f16p16_mul(MATRIX_ZX(m1), MATRIX_XY(m2)) +
     eina_f16p16_mul(MATRIX_ZY(m1), MATRIX_YY(m2)) +
     eina_f16p16_mul(MATRIX_ZZ(m1), MATRIX_ZY(m2));
   a33 = eina_f16p16_mul(MATRIX_ZX(m1), MATRIX_XZ(m2)) +
     eina_f16p16_mul(MATRIX_ZY(m1), MATRIX_YZ(m2)) +
     eina_f16p16_mul(MATRIX_ZZ(m1), MATRIX_ZZ(m2));

   MATRIX_XX(dst) = a11;
   MATRIX_XY(dst) = a12;
   MATRIX_XZ(dst) = a13;
   MATRIX_YX(dst) = a21;
   MATRIX_YY(dst) = a22;
   MATRIX_YZ(dst) = a23;
   MATRIX_ZX(dst) = a31;
   MATRIX_ZY(dst) = a32;
   MATRIX_ZZ(dst) = a33;
}

EAPI void
eina_matrix3_translate(Eina_Matrix3 *m, double tx, double ty)
{
   Eina_Matrix3 tmp;
   MATRIX_XX(&tmp) = 1;
   MATRIX_XY(&tmp) = 0;
   MATRIX_XZ(&tmp) = tx;
   MATRIX_YX(&tmp) = 0;
   MATRIX_YY(&tmp) = 1;
   MATRIX_YZ(&tmp) = ty;
   MATRIX_ZX(&tmp) = 0;
   MATRIX_ZY(&tmp) = 0;
   MATRIX_ZZ(&tmp) = 1;
   eina_matrix3_compose(m, &tmp, m);
}

EAPI void
eina_matrix3_scale(Eina_Matrix3 *m, double sx, double sy)
{
   Eina_Matrix3 tmp;
   MATRIX_XX(&tmp) = sx;
   MATRIX_XY(&tmp) = 0;
   MATRIX_XZ(&tmp) = 0;
   MATRIX_YX(&tmp) = 0;
   MATRIX_YY(&tmp) = sy;
   MATRIX_YZ(&tmp) = 0;
   MATRIX_ZX(&tmp) = 0;
   MATRIX_ZY(&tmp) = 0;
   MATRIX_ZZ(&tmp) = 1;
   eina_matrix3_compose(m, &tmp, m);
}

EAPI void
eina_matrix3_rotate(Eina_Matrix3 *m, double rad)
{
   double c, s;
#if 0
   c = cosf(rad);
   s = sinf(rad);
#else
   /* normalize the angle between -pi,pi */
   rad = fmod(rad + M_PI, 2 * M_PI) - M_PI;
   c = _cos(rad);
   s = _sin(rad);
#endif

   Eina_Matrix3 tmp;
   MATRIX_XX(&tmp) = c;
   MATRIX_XY(&tmp) = -s;
   MATRIX_XZ(&tmp) = 0;
   MATRIX_YX(&tmp) = s;
   MATRIX_YY(&tmp) = c;
   MATRIX_YZ(&tmp) = 0;
   MATRIX_ZX(&tmp) = 0;
   MATRIX_ZY(&tmp) = 0;
   MATRIX_ZZ(&tmp) = 1;
   eina_matrix3_compose(m, &tmp, m);
}

EAPI void
eina_matrix3_identity(Eina_Matrix3 *m)
{
   MATRIX_XX(m) = 1;
   MATRIX_XY(m) = 0;
   MATRIX_XZ(m) = 0;
   MATRIX_YX(m) = 0;
   MATRIX_YY(m) = 1;
   MATRIX_YZ(m) = 0;
   MATRIX_ZX(m) = 0;
   MATRIX_ZY(m) = 0;
   MATRIX_ZZ(m) = 1;
}

EAPI void
eina_matrix3_f16p16_identity(Eina_Matrix3_F16p16 *m)
{
   MATRIX_XX(m) = 65536;
   MATRIX_XY(m) = 0;
   MATRIX_XZ(m) = 0;
   MATRIX_YX(m) = 0;
   MATRIX_YY(m) = 65536;
   MATRIX_YZ(m) = 0;
   MATRIX_ZX(m) = 0;
   MATRIX_ZY(m) = 0;
   MATRIX_ZZ(m) = 65536;
}

EAPI Eina_Bool
eina_matrix3_square_quad_map(Eina_Matrix3 *m, const Eina_Quad *q)
{
   // x0 - x1 + x2 - x3
   double ex = QUAD_X0(q) - QUAD_X1(q) + QUAD_X2(q) - QUAD_X3(q);
   // y0 - y1 + y2 - y3
   double ey = QUAD_Y0(q) - QUAD_Y1(q) + QUAD_Y2(q) - QUAD_Y3(q);

   /* parallelogram */
   if (EINA_DBL_EQ(ex, 0.0) && EINA_DBL_EQ(ey, 0.0))
     {
        /* create the affine matrix */
        MATRIX_XX(m) = QUAD_X1(q) - QUAD_X0(q);
        MATRIX_XY(m) = QUAD_X2(q) - QUAD_X1(q);
        MATRIX_XZ(m) = QUAD_X0(q);

        MATRIX_YX(m) = QUAD_Y1(q) - QUAD_Y0(q);
        MATRIX_YY(m) = QUAD_Y2(q) - QUAD_Y1(q);
        MATRIX_YZ(m) = QUAD_Y0(q);

        MATRIX_ZX(m) = 0;
        MATRIX_ZY(m) = 0;
        MATRIX_ZZ(m) = 1;

        return EINA_TRUE;
     }
   else
     {
        double dx1 = QUAD_X1(q) - QUAD_X2(q); // x1 - x2
        double dx2 = QUAD_X3(q) - QUAD_X2(q); // x3 - x2
        double dy1 = QUAD_Y1(q) - QUAD_Y2(q); // y1 - y2
        double dy2 = QUAD_Y3(q) - QUAD_Y2(q); // y3 - y2
        double den = (dx1 * dy2) - (dx2 * dy1);

        if (EINA_DBL_EQ(den, 0.0))
          return EINA_FALSE;

        MATRIX_ZX(m) = ((ex * dy2) - (dx2 * ey)) / den;
        MATRIX_ZY(m) = ((dx1 * ey) - (ex * dy1)) / den;
        MATRIX_ZZ(m) = 1;
        MATRIX_XX(m) = QUAD_X1(q) - QUAD_X0(q) + (MATRIX_ZX(m) * QUAD_X1(q));
        MATRIX_XY(m) = QUAD_X3(q) - QUAD_X0(q) + (MATRIX_ZY(m) * QUAD_X3(q));
        MATRIX_XZ(m) = QUAD_X0(q);
        MATRIX_YX(m) = QUAD_Y1(q) - QUAD_Y0(q) + (MATRIX_ZX(m) * QUAD_Y1(q));
        MATRIX_YY(m) = QUAD_Y3(q) - QUAD_Y0(q) + (MATRIX_ZY(m) * QUAD_Y3(q));
        MATRIX_YZ(m) = QUAD_Y0(q);

        return EINA_TRUE;
     }
}

EAPI Eina_Bool
eina_matrix3_quad_square_map(Eina_Matrix3 *m,
                             const Eina_Quad *q)
{
   Eina_Matrix3 tmp;

   /* compute square to quad */
   if (!eina_matrix3_square_quad_map(&tmp, q))
     return EINA_FALSE;

   eina_matrix3_inverse(&tmp, m);
   /* make the projective matrix3 always have 1 on zz */
   if (!EINA_DBL_EQ(MATRIX_ZZ(m), 1.0))
     {
        eina_matrix3_divide(m, MATRIX_ZZ(m));
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
eina_matrix3_quad_quad_map(Eina_Matrix3 *m,
                           const Eina_Quad *src,
                           const Eina_Quad *dst)
{
   Eina_Matrix3 tmp;

   /* TODO check that both are actually quadrangles */
   if (!eina_matrix3_quad_square_map(m, src))
     return EINA_FALSE;
   if (!eina_matrix3_square_quad_map(&tmp, dst))
     return EINA_FALSE;
   eina_matrix3_compose(&tmp, m, m);

   return EINA_TRUE;
}

EAPI void
eina_matrix4_matrix3_to(Eina_Matrix3 *m3, const Eina_Matrix4 *m4)
{
   MATRIX_XX(m3) = MATRIX_XX(m4);
   MATRIX_XY(m3) = MATRIX_XY(m4);
   MATRIX_XZ(m3) = MATRIX_XW(m4);
   MATRIX_YX(m3) = MATRIX_YX(m4);
   MATRIX_YY(m3) = MATRIX_YY(m4);
   MATRIX_YZ(m3) = MATRIX_YW(m4);
   MATRIX_ZX(m3) = MATRIX_WX(m4);
   MATRIX_ZY(m3) = MATRIX_WY(m4);
   MATRIX_ZZ(m3) = MATRIX_WW(m4);
}

EAPI void
eina_matrix3_matrix4_to(Eina_Matrix4 *m4, const Eina_Matrix3 *m3)
{
   MATRIX_XX(m4) = MATRIX_XX(m3);
   MATRIX_XY(m4) = MATRIX_XY(m3);
   MATRIX_XZ(m4) = 0;
   MATRIX_XW(m4) = MATRIX_XZ(m3);
   MATRIX_YX(m4) = MATRIX_YX(m3);
   MATRIX_YY(m4) = MATRIX_YY(m3);
   MATRIX_YZ(m4) = 0;
   MATRIX_YW(m4) = MATRIX_YZ(m3);
   MATRIX_ZX(m4) = 0;
   MATRIX_ZY(m4) = 0;
   MATRIX_ZZ(m4) = 1;
   MATRIX_ZW(m4) = 0;
   MATRIX_WX(m4) = MATRIX_ZX(m3);
   MATRIX_WY(m4) = MATRIX_ZY(m3);
   MATRIX_WZ(m4) = 0;
   MATRIX_WW(m4) = MATRIX_ZZ(m3);
}

EAPI double
eina_matrix4_determinant(const Eina_Matrix4 *m)
{
   return
       MATRIX_XW(m) * MATRIX_YZ(m) * MATRIX_ZY(m) * MATRIX_WX(m)
     - MATRIX_XZ(m) * MATRIX_YW(m) * MATRIX_ZY(m) * MATRIX_WX(m)
     - MATRIX_XW(m) * MATRIX_YY(m) * MATRIX_ZZ(m) * MATRIX_WX(m)
     + MATRIX_XY(m) * MATRIX_YW(m) * MATRIX_ZZ(m) * MATRIX_WX(m)
     + MATRIX_XZ(m) * MATRIX_YY(m) * MATRIX_ZW(m) * MATRIX_WX(m)
     - MATRIX_XY(m) * MATRIX_YZ(m) * MATRIX_ZW(m) * MATRIX_WX(m)
     - MATRIX_XW(m) * MATRIX_YZ(m) * MATRIX_ZX(m) * MATRIX_WY(m)
     + MATRIX_XZ(m) * MATRIX_YW(m) * MATRIX_ZX(m) * MATRIX_WY(m)
     + MATRIX_XW(m) * MATRIX_YX(m) * MATRIX_ZZ(m) * MATRIX_WY(m)
     - MATRIX_XX(m) * MATRIX_YW(m) * MATRIX_ZZ(m) * MATRIX_WY(m)
     - MATRIX_XZ(m) * MATRIX_YX(m) * MATRIX_ZW(m) * MATRIX_WY(m)
     + MATRIX_XX(m) * MATRIX_YZ(m) * MATRIX_ZW(m) * MATRIX_WY(m)
     + MATRIX_XW(m) * MATRIX_YY(m) * MATRIX_ZX(m) * MATRIX_WZ(m)
     - MATRIX_XY(m) * MATRIX_YW(m) * MATRIX_ZX(m) * MATRIX_WZ(m)
     - MATRIX_XW(m) * MATRIX_YX(m) * MATRIX_ZY(m) * MATRIX_WZ(m)
     + MATRIX_XX(m) * MATRIX_YW(m) * MATRIX_ZY(m) * MATRIX_WZ(m)
     + MATRIX_XY(m) * MATRIX_YX(m) * MATRIX_ZW(m) * MATRIX_WZ(m)
     - MATRIX_XX(m) * MATRIX_YY(m) * MATRIX_ZW(m) * MATRIX_WZ(m)
     - MATRIX_XZ(m) * MATRIX_YY(m) * MATRIX_ZX(m) * MATRIX_WW(m)
     + MATRIX_XY(m) * MATRIX_YZ(m) * MATRIX_ZX(m) * MATRIX_WW(m)
     + MATRIX_XZ(m) * MATRIX_YX(m) * MATRIX_ZY(m) * MATRIX_WW(m)
     - MATRIX_XX(m) * MATRIX_YZ(m) * MATRIX_ZY(m) * MATRIX_WW(m)
     - MATRIX_XY(m) * MATRIX_YX(m) * MATRIX_ZZ(m) * MATRIX_WW(m)
     + MATRIX_XX(m) * MATRIX_YY(m) * MATRIX_ZZ(m) * MATRIX_WW(m);
}

EAPI Eina_Bool
eina_matrix4_normalized(Eina_Matrix4 *out, const Eina_Matrix4 *in)
{
   double det;

   det = eina_matrix4_determinant(in);
   if (fabs(det) < DBL_EPSILON) return EINA_FALSE;

   MATRIX_XX(out) = MATRIX_XX(in) / det;
   MATRIX_XY(out) = MATRIX_XY(in) / det;
   MATRIX_XZ(out) = MATRIX_XZ(in) / det;
   MATRIX_XW(out) = MATRIX_XW(in) / det;
   MATRIX_YX(out) = MATRIX_YX(in) / det;
   MATRIX_YY(out) = MATRIX_YY(in) / det;
   MATRIX_YZ(out) = MATRIX_YZ(in) / det;
   MATRIX_YW(out) = MATRIX_YW(in) / det;
   MATRIX_ZX(out) = MATRIX_ZX(in) / det;
   MATRIX_ZY(out) = MATRIX_ZY(in) / det;
   MATRIX_ZZ(out) = MATRIX_ZZ(in) / det;
   MATRIX_ZW(out) = MATRIX_ZW(in) / det;
   MATRIX_WX(out) = MATRIX_WX(in) / det;
   MATRIX_WY(out) = MATRIX_WY(in) / det;
   MATRIX_WZ(out) = MATRIX_WZ(in) / det;
   MATRIX_WW(out) = MATRIX_WW(in) / det;

   return EINA_TRUE;
}

EAPI Eina_Bool
eina_matrix4_inverse(Eina_Matrix4 *out, const Eina_Matrix4 *in)
{
   double det;

   MATRIX_XX(out) =
       MATRIX_YY(in)  * MATRIX_ZZ(in) * MATRIX_WW(in)
     - MATRIX_YY(in)  * MATRIX_ZW(in) * MATRIX_WZ(in)
     - MATRIX_ZY(in)  * MATRIX_YZ(in)  * MATRIX_WW(in)
     + MATRIX_ZY(in)  * MATRIX_YW(in)  * MATRIX_WZ(in)
     + MATRIX_WY(in) * MATRIX_YZ(in)  * MATRIX_ZW(in)
     - MATRIX_WY(in) * MATRIX_YW(in)  * MATRIX_ZZ(in);

   MATRIX_YX(out) =
     - MATRIX_YX(in)  * MATRIX_ZZ(in) * MATRIX_WW(in)
     + MATRIX_YX(in)  * MATRIX_ZW(in) * MATRIX_WZ(in)
     + MATRIX_ZX(in)  * MATRIX_YZ(in)  * MATRIX_WW(in)
     - MATRIX_ZX(in)  * MATRIX_YW(in)  * MATRIX_WZ(in)
     - MATRIX_WX(in) * MATRIX_YZ(in)  * MATRIX_ZW(in)
     + MATRIX_WX(in) * MATRIX_YW(in)  * MATRIX_ZZ(in);

   MATRIX_ZX(out) =
       MATRIX_YX(in)  * MATRIX_ZY(in) * MATRIX_WW(in)
     - MATRIX_YX(in)  * MATRIX_ZW(in) * MATRIX_WY(in)
     - MATRIX_ZX(in)  * MATRIX_YY(in) * MATRIX_WW(in)
     + MATRIX_ZX(in)  * MATRIX_YW(in) * MATRIX_WY(in)
     + MATRIX_WX(in) * MATRIX_YY(in) * MATRIX_ZW(in)
     - MATRIX_WX(in) * MATRIX_YW(in) * MATRIX_ZY(in);

   MATRIX_WX(out) =
     - MATRIX_YX(in)  * MATRIX_ZY(in) * MATRIX_WZ(in)
     + MATRIX_YX(in)  * MATRIX_ZZ(in) * MATRIX_WY(in)
     + MATRIX_ZX(in)  * MATRIX_YY(in) * MATRIX_WZ(in)
     - MATRIX_ZX(in)  * MATRIX_YZ(in) * MATRIX_WY(in)
     - MATRIX_WX(in) * MATRIX_YY(in) * MATRIX_ZZ(in)
     + MATRIX_WX(in) * MATRIX_YZ(in) * MATRIX_ZY(in);

   MATRIX_XY(out) =
     - MATRIX_XY(in)  * MATRIX_ZZ(in) * MATRIX_WW(in)
     + MATRIX_XY(in)  * MATRIX_ZW(in) * MATRIX_WZ(in)
     + MATRIX_ZY(in)  * MATRIX_XZ(in) * MATRIX_WW(in)
     - MATRIX_ZY(in)  * MATRIX_XW(in) * MATRIX_WZ(in)
     - MATRIX_WY(in) * MATRIX_XZ(in) * MATRIX_ZW(in)
     + MATRIX_WY(in) * MATRIX_XW(in) * MATRIX_ZZ(in);

   MATRIX_YY(out) =
       MATRIX_XX(in)  * MATRIX_ZZ(in) * MATRIX_WW(in)
     - MATRIX_XX(in)  * MATRIX_ZW(in) * MATRIX_WZ(in)
     - MATRIX_ZX(in)  * MATRIX_XZ(in) * MATRIX_WW(in)
     + MATRIX_ZX(in)  * MATRIX_XW(in) * MATRIX_WZ(in)
     + MATRIX_WX(in) * MATRIX_XZ(in) * MATRIX_ZW(in)
     - MATRIX_WX(in) * MATRIX_XW(in) * MATRIX_ZZ(in);

   MATRIX_ZY(out) =
     - MATRIX_XX(in)  * MATRIX_ZY(in) * MATRIX_WW(in)
     + MATRIX_XX(in)  * MATRIX_ZW(in) * MATRIX_WY(in)
     + MATRIX_ZX(in)  * MATRIX_XY(in) * MATRIX_WW(in)
     - MATRIX_ZX(in)  * MATRIX_XW(in) * MATRIX_WY(in)
     - MATRIX_WX(in) * MATRIX_XY(in) * MATRIX_ZW(in)
     + MATRIX_WX(in) * MATRIX_XW(in) * MATRIX_ZY(in);

   MATRIX_WY(out) =
       MATRIX_XX(in)  * MATRIX_ZY(in) * MATRIX_WZ(in)
     - MATRIX_XX(in)  * MATRIX_ZZ(in) * MATRIX_WY(in)
     - MATRIX_ZX(in)  * MATRIX_XY(in) * MATRIX_WZ(in)
     + MATRIX_ZX(in)  * MATRIX_XZ(in) * MATRIX_WY(in)
     + MATRIX_WX(in) * MATRIX_XY(in) * MATRIX_ZZ(in)
     - MATRIX_WX(in) * MATRIX_XZ(in) * MATRIX_ZY(in);

   MATRIX_XZ(out) =
       MATRIX_XY(in)  * MATRIX_YZ(in) * MATRIX_WW(in)
     - MATRIX_XY(in)  * MATRIX_YW(in) * MATRIX_WZ(in)
     - MATRIX_YY(in)  * MATRIX_XZ(in) * MATRIX_WW(in)
     + MATRIX_YY(in)  * MATRIX_XW(in) * MATRIX_WZ(in)
     + MATRIX_WY(in) * MATRIX_XZ(in) * MATRIX_YW(in)
     - MATRIX_WY(in) * MATRIX_XW(in) * MATRIX_YZ(in);

   MATRIX_YZ(out) =
     - MATRIX_XX(in)  * MATRIX_YZ(in) * MATRIX_WW(in)
     + MATRIX_XX(in)  * MATRIX_YW(in) * MATRIX_WZ(in)
     + MATRIX_YX(in)  * MATRIX_XZ(in) * MATRIX_WW(in)
     - MATRIX_YX(in)  * MATRIX_XW(in) * MATRIX_WZ(in)
     - MATRIX_WX(in) * MATRIX_XZ(in) * MATRIX_YW(in)
     + MATRIX_WX(in) * MATRIX_XW(in) * MATRIX_YZ(in);

   MATRIX_ZZ(out) =
       MATRIX_XX(in)  * MATRIX_YY(in) * MATRIX_WW(in)
     - MATRIX_XX(in)  * MATRIX_YW(in) * MATRIX_WY(in)
     - MATRIX_YX(in)  * MATRIX_XY(in) * MATRIX_WW(in)
     + MATRIX_YX(in)  * MATRIX_XW(in) * MATRIX_WY(in)
     + MATRIX_WX(in) * MATRIX_XY(in) * MATRIX_YW(in)
     - MATRIX_WX(in) * MATRIX_XW(in) * MATRIX_YY(in);

   MATRIX_WZ(out) =
     - MATRIX_XX(in)  * MATRIX_YY(in) * MATRIX_WZ(in)
     + MATRIX_XX(in)  * MATRIX_YZ(in) * MATRIX_WY(in)
     + MATRIX_YX(in)  * MATRIX_XY(in) * MATRIX_WZ(in)
     - MATRIX_YX(in)  * MATRIX_XZ(in) * MATRIX_WY(in)
     - MATRIX_WX(in) * MATRIX_XY(in) * MATRIX_YZ(in)
     + MATRIX_WX(in) * MATRIX_XZ(in) * MATRIX_YY(in);

   MATRIX_XW(out) =
     - MATRIX_XY(in) * MATRIX_YZ(in) * MATRIX_ZW(in)
     + MATRIX_XY(in) * MATRIX_YW(in) * MATRIX_ZZ(in)
     + MATRIX_YY(in) * MATRIX_XZ(in) * MATRIX_ZW(in)
     - MATRIX_YY(in) * MATRIX_XW(in) * MATRIX_ZZ(in)
     - MATRIX_ZY(in) * MATRIX_XZ(in) * MATRIX_YW(in)
     + MATRIX_ZY(in) * MATRIX_XW(in) * MATRIX_YZ(in);

   MATRIX_YW(out) =
       MATRIX_XX(in) * MATRIX_YZ(in) * MATRIX_ZW(in)
     - MATRIX_XX(in) * MATRIX_YW(in) * MATRIX_ZZ(in)
     - MATRIX_YX(in) * MATRIX_XZ(in) * MATRIX_ZW(in)
     + MATRIX_YX(in) * MATRIX_XW(in) * MATRIX_ZZ(in)
     + MATRIX_ZX(in) * MATRIX_XZ(in) * MATRIX_YW(in)
     - MATRIX_ZX(in) * MATRIX_XW(in) * MATRIX_YZ(in);

   MATRIX_ZW(out) =
     - MATRIX_XX(in) * MATRIX_YY(in) * MATRIX_ZW(in)
     + MATRIX_XX(in) * MATRIX_YW(in) * MATRIX_ZY(in)
     + MATRIX_YX(in) * MATRIX_XY(in) * MATRIX_ZW(in)
     - MATRIX_YX(in) * MATRIX_XW(in) * MATRIX_ZY(in)
     - MATRIX_ZX(in) * MATRIX_XY(in) * MATRIX_YW(in)
     + MATRIX_ZX(in) * MATRIX_XW(in) * MATRIX_YY(in);

   MATRIX_WW(out) =
       MATRIX_XX(in) * MATRIX_YY(in) * MATRIX_ZZ(in)
     - MATRIX_XX(in) * MATRIX_YZ(in) * MATRIX_ZY(in)
     - MATRIX_YX(in) * MATRIX_XY(in) * MATRIX_ZZ(in)
     + MATRIX_YX(in) * MATRIX_XZ(in) * MATRIX_ZY(in)
     + MATRIX_ZX(in) * MATRIX_XY(in) * MATRIX_YZ(in)
     - MATRIX_ZX(in) * MATRIX_XZ(in) * MATRIX_YY(in);

   det =
       MATRIX_XX(in) * MATRIX_XX(out)
     + MATRIX_XY(in) * MATRIX_YX(out)
     + MATRIX_XZ(in) * MATRIX_ZX(out)
     + MATRIX_XW(in) * MATRIX_WX(out);

   if (fabs(det) < DBL_EPSILON) return EINA_FALSE;

   det = 1.0 / det;

   MATRIX_XX(out) = MATRIX_XX(out) * det;
   MATRIX_XY(out) = MATRIX_XY(out) * det;
   MATRIX_XZ(out) = MATRIX_XZ(out) * det;
   MATRIX_XW(out) = MATRIX_XW(out) * det;
   MATRIX_YX(out) = MATRIX_YX(out) * det;
   MATRIX_YY(out) = MATRIX_YY(out) * det;
   MATRIX_YZ(out) = MATRIX_YZ(out) * det;
   MATRIX_YW(out) = MATRIX_YW(out) * det;
   MATRIX_ZX(out) = MATRIX_ZX(out) * det;
   MATRIX_ZY(out) = MATRIX_ZY(out) * det;
   MATRIX_ZZ(out) = MATRIX_ZZ(out) * det;
   MATRIX_ZW(out) = MATRIX_ZW(out) * det;
   MATRIX_WX(out) = MATRIX_WX(out) * det;
   MATRIX_WY(out) = MATRIX_WY(out) * det;
   MATRIX_WZ(out) = MATRIX_WZ(out) * det;
   MATRIX_WW(out) = MATRIX_WW(out) * det;

   return EINA_TRUE;
}

EAPI void
eina_matrix4_transpose(Eina_Matrix4 *out, const Eina_Matrix4 *in)
{
   MATRIX_XX(out) = MATRIX_XX(in);
   MATRIX_XY(out) = MATRIX_YX(in);
   MATRIX_XZ(out) = MATRIX_ZX(in);
   MATRIX_XW(out) = MATRIX_WX(in);
   MATRIX_YX(out) = MATRIX_XY(in);
   MATRIX_YY(out) = MATRIX_YY(in);
   MATRIX_YZ(out) = MATRIX_ZY(in);
   MATRIX_YW(out) = MATRIX_WY(in);
   MATRIX_ZX(out) = MATRIX_XZ(in);
   MATRIX_ZY(out) = MATRIX_YZ(in);
   MATRIX_ZZ(out) = MATRIX_ZZ(in);
   MATRIX_ZW(out) = MATRIX_WZ(in);
   MATRIX_WX(out) = MATRIX_XW(in);
   MATRIX_WY(out) = MATRIX_YW(in);
   MATRIX_WZ(out) = MATRIX_ZW(in);
   MATRIX_WW(out) = MATRIX_WW(in);
}

EAPI void
eina_matrix4_multiply_copy(Eina_Matrix4 *out,
                      const Eina_Matrix4 *mat_a, const Eina_Matrix4 *mat_b)
{
   if (out != mat_a && out != mat_b)
     {
        eina_matrix4_multiply(out, mat_a, mat_b);
     }
   else
     {
        Eina_Matrix4 result;

        eina_matrix4_multiply(&result, mat_a, mat_b);
        eina_matrix4_copy(out, &result);
     }
}

EAPI void
eina_matrix4_identity(Eina_Matrix4 *out)
{
   memset(out, 0, sizeof (Eina_Matrix4));

   MATRIX_XX(out) = 1.0;
   MATRIX_YY(out) = 1.0;
   MATRIX_ZZ(out) = 1.0;
   MATRIX_WW(out) = 1.0;
}

EAPI Eina_Matrix_Type
eina_matrix2_type_get(const Eina_Matrix2 *m)
{
   if (EINA_DBL_EQ(MATRIX_XX(m), 1.0) &&
       EINA_DBL_EQ(MATRIX_XY(m), 0.0) &&
       EINA_DBL_EQ(MATRIX_YX(m), 0.0) &&
       EINA_DBL_EQ(MATRIX_YY(m), 1.0))
     return EINA_MATRIX_TYPE_IDENTITY;
   return EINA_MATRIX_TYPE_AFFINE;
}

EAPI void
eina_matrix4_array_set(Eina_Matrix4 *m, const double *v)
{
   memcpy(&MATRIX_XX(m), v, sizeof(double) * 16);
}

EAPI void
eina_matrix4_copy(Eina_Matrix4 *dst, const Eina_Matrix4 *src)
{
   memcpy(dst, src, sizeof(Eina_Matrix4));
}

EAPI void
eina_matrix4_multiply(Eina_Matrix4 *out, const Eina_Matrix4 *mat_a,
                           const Eina_Matrix4 *mat_b)
{
   if (eina_matrix4_type_get(mat_a) == EINA_MATRIX_TYPE_IDENTITY)
     {
        eina_matrix4_copy(out, mat_b);
        return;
     }

   if (eina_matrix4_type_get(mat_b) == EINA_MATRIX_TYPE_IDENTITY)
     {
        eina_matrix4_copy(out, mat_a);
        return;
     }

   MATRIX_XX(out) = MATRIX_XX(mat_a) * MATRIX_XX(mat_b) + MATRIX_YX(mat_a) * MATRIX_XY(mat_b) +
                    MATRIX_ZX(mat_a) * MATRIX_XZ(mat_b) + MATRIX_WX(mat_a) * MATRIX_XW(mat_b);
   MATRIX_YX(out) = MATRIX_XX(mat_a) * MATRIX_YX(mat_b) + MATRIX_YX(mat_a) * MATRIX_YY(mat_b) +
                    MATRIX_ZX(mat_a) * MATRIX_YZ(mat_b) + MATRIX_WX(mat_a) * MATRIX_YW(mat_b);
   MATRIX_ZX(out) = MATRIX_XX(mat_a) * MATRIX_ZX(mat_b) + MATRIX_YX(mat_a) * MATRIX_ZY(mat_b) +
                    MATRIX_ZX(mat_a) * MATRIX_ZZ(mat_b) + MATRIX_WX(mat_a) * MATRIX_ZW(mat_b);
   MATRIX_WX(out) = MATRIX_XX(mat_a) * MATRIX_WX(mat_b) + MATRIX_YX(mat_a) * MATRIX_WY(mat_b) +
                    MATRIX_ZX(mat_a) * MATRIX_WZ(mat_b) + MATRIX_WX(mat_a) * MATRIX_WW(mat_b);

   MATRIX_XY(out) = MATRIX_XY(mat_a) * MATRIX_XX(mat_b) + MATRIX_YY(mat_a) * MATRIX_XY(mat_b) +
                    MATRIX_ZY(mat_a) * MATRIX_XZ(mat_b) + MATRIX_WY(mat_a) * MATRIX_XW(mat_b);
   MATRIX_YY(out) = MATRIX_XY(mat_a) * MATRIX_YX(mat_b) + MATRIX_YY(mat_a) * MATRIX_YY(mat_b) +
                    MATRIX_ZY(mat_a) * MATRIX_YZ(mat_b) + MATRIX_WY(mat_a) * MATRIX_YW(mat_b);
   MATRIX_ZY(out) = MATRIX_XY(mat_a) * MATRIX_ZX(mat_b) + MATRIX_YY(mat_a) * MATRIX_ZY(mat_b) +
                    MATRIX_ZY(mat_a) * MATRIX_ZZ(mat_b) + MATRIX_WY(mat_a) * MATRIX_ZW(mat_b);
   MATRIX_WY(out) = MATRIX_XY(mat_a) * MATRIX_WX(mat_b) + MATRIX_YY(mat_a) * MATRIX_WY(mat_b) +
                    MATRIX_ZY(mat_a) * MATRIX_WZ(mat_b) + MATRIX_WY(mat_a) * MATRIX_WW(mat_b);

   MATRIX_XZ(out) = MATRIX_XZ(mat_a) * MATRIX_XX(mat_b) + MATRIX_YZ(mat_a) * MATRIX_XY(mat_b) +
                    MATRIX_ZZ(mat_a) * MATRIX_XZ(mat_b) + MATRIX_WZ(mat_a) * MATRIX_XW(mat_b);
   MATRIX_YZ(out) = MATRIX_XZ(mat_a) * MATRIX_YX(mat_b) + MATRIX_YZ(mat_a) * MATRIX_YY(mat_b) +
                    MATRIX_ZZ(mat_a) * MATRIX_YZ(mat_b) + MATRIX_WZ(mat_a) * MATRIX_YW(mat_b);
   MATRIX_ZZ(out) = MATRIX_XZ(mat_a) * MATRIX_ZX(mat_b) + MATRIX_YZ(mat_a) * MATRIX_ZY(mat_b) +
                    MATRIX_ZZ(mat_a) * MATRIX_ZZ(mat_b) + MATRIX_WZ(mat_a) * MATRIX_ZW(mat_b);
   MATRIX_WZ(out) = MATRIX_XZ(mat_a) * MATRIX_WX(mat_b) + MATRIX_YZ(mat_a) * MATRIX_WY(mat_b) +
                    MATRIX_ZZ(mat_a) * MATRIX_WZ(mat_b) + MATRIX_WZ(mat_a) * MATRIX_WW(mat_b);

   MATRIX_XW(out) = MATRIX_XW(mat_a) * MATRIX_XX(mat_b) + MATRIX_YW(mat_a) * MATRIX_XY(mat_b) +
                    MATRIX_ZW(mat_a) * MATRIX_XZ(mat_b) + MATRIX_WW(mat_a) * MATRIX_XW(mat_b);
   MATRIX_YW(out) = MATRIX_XW(mat_a) * MATRIX_YX(mat_b) + MATRIX_YW(mat_a) * MATRIX_YY(mat_b) +
                    MATRIX_ZW(mat_a) * MATRIX_YZ(mat_b) + MATRIX_WW(mat_a) * MATRIX_YW(mat_b);
   MATRIX_ZW(out) = MATRIX_XW(mat_a) * MATRIX_ZX(mat_b) + MATRIX_YW(mat_a) * MATRIX_ZY(mat_b) +
                    MATRIX_ZW(mat_a) * MATRIX_ZZ(mat_b) + MATRIX_WW(mat_a) * MATRIX_ZW(mat_b);
   MATRIX_WW(out) = MATRIX_XW(mat_a) * MATRIX_WX(mat_b) + MATRIX_YW(mat_a) * MATRIX_WY(mat_b) +
                    MATRIX_ZW(mat_a) * MATRIX_WZ(mat_b) + MATRIX_WW(mat_a) * MATRIX_WW(mat_b);
}

EAPI void
eina_matrix4_ortho_set(Eina_Matrix4 *m,
                    double left, double right, double bottom, double top,
                    double dnear, double dfar)
{
   double   w = right - left;
   double   h = top - bottom;
   double   depth = dnear - dfar;

   MATRIX_XX(m) = 2.0f / w;
   MATRIX_XY(m) = 0.0f;
   MATRIX_XZ(m) = 0.0f;
   MATRIX_XW(m) = 0.0f;

   MATRIX_YX(m) = 0.0f;
   MATRIX_YY(m) = 2.0f / h;
   MATRIX_YZ(m) = 0.0f;
   MATRIX_YW(m) = 0.0f;

   MATRIX_ZX(m) = 0.0f;
   MATRIX_ZY(m) = 0.0f;
   MATRIX_ZZ(m) = 2.0f / depth;
   MATRIX_ZW(m) = 0.0f;

   MATRIX_WX(m) = -(right + left) / w;
   MATRIX_WY(m) = -(top + bottom) / h;
   MATRIX_WZ(m) = (dfar + dnear) / depth;
   MATRIX_WW(m) = 1.0f;
}

EAPI void
eina_matrix3_array_set(Eina_Matrix3 *m, const double *v)
{
   memcpy(&MATRIX_XX(m), v, sizeof(double) * 9);
}

EAPI void
eina_matrix3_copy(Eina_Matrix3 *dst, const Eina_Matrix3 *src)
{
   memcpy(dst, src, sizeof(Eina_Matrix3));
}

EAPI void
eina_matrix3_multiply(Eina_Matrix3 *out, const Eina_Matrix3 *mat_a, const Eina_Matrix3 *mat_b)
{
   if (eina_matrix3_type_get(mat_a) == EINA_MATRIX_TYPE_IDENTITY)
     {
        eina_matrix3_copy(out, mat_b);
        return;
     }

   if (eina_matrix3_type_get(mat_b) == EINA_MATRIX_TYPE_IDENTITY)
     {
        eina_matrix3_copy(out, mat_a);
        return;
     }

   eina_matrix3_compose(mat_a, mat_b, out);
}

EAPI void
eina_matrix3_multiply_copy(Eina_Matrix3 *out, const Eina_Matrix3 *mat_a, const Eina_Matrix3 *mat_b)
{
   if (out != mat_a && out != mat_b)
     {
        eina_matrix3_multiply(out, mat_a, mat_b);
     }
   else
     {
        Eina_Matrix3 tmp;

        eina_matrix3_multiply(&tmp, mat_a, mat_b);
        eina_matrix3_copy(out, &tmp);
     }
}

EAPI void
eina_matrix3_position_transform_set(Eina_Matrix3 *out, const double p_x,
								 const double p_y)
{
   eina_matrix3_identity(out);
   MATRIX_XZ(out) = p_x;
   MATRIX_YZ(out) = p_y;
}

EAPI void
eina_matrix3_scale_transform_set(Eina_Matrix3 *out, double s_x, double s_y)
{
   eina_matrix3_identity(out);
   MATRIX_XX(out) = s_x;
   MATRIX_YY(out) = s_y;
}

EAPI void
eina_normal3_matrix_get(Eina_Matrix3 *out, const Eina_Matrix4 *m)
{
   /* Normal matrix is a transposed matrix of inversed modelview.
    * And we need only upper-left 3x3 terms to work with. */

   double   det;

   double   a = MATRIX_XX(m);
   double   b = MATRIX_YX(m);
   double   c = MATRIX_ZX(m);

   double   d = MATRIX_XY(m);
   double   e = MATRIX_YY(m);
   double   f = MATRIX_ZY(m);

   double   g = MATRIX_XZ(m);
   double   h = MATRIX_YZ(m);
   double   i = MATRIX_ZZ(m);

   det = a * e * i + b * f * g + c * d * h - g * e * c - h * f * a - i * d * b;

   if (fabs(det) >= DBL_EPSILON) det = 1.0 / det;
   else det = 0.0;

   MATRIX_XX(out) = (e * i - f * h) * det;
   MATRIX_XY(out) = (h * c - i * b) * det;
   MATRIX_XZ(out) = (b * f - c * e) * det;

   MATRIX_YX(out) = (g * f - d * i) * det;
   MATRIX_YY(out) = (a * i - g * c) * det;
   MATRIX_YZ(out) = (d * c - a * f) * det;

   MATRIX_ZX(out) = (d * h - g * e) * det;
   MATRIX_ZY(out) = (g * b - a * h) * det;
   MATRIX_ZZ(out) = (a * e - d * b) * det;
}

EAPI void
eina_matrix2_values_set(Eina_Matrix2 *m,
                        double xx, double xy,
                        double yx, double yy)
{
   MATRIX_XX(m) = xx;
   MATRIX_XY(m) = xy;
   MATRIX_YX(m) = yx;
   MATRIX_YY(m) = yy;
}

EAPI void
eina_matrix2_values_get(const Eina_Matrix2 *m,
                        double *xx, double *xy,
                        double *yx, double *yy)
{
   if (xx) *xx = MATRIX_XX(m);
   if (xy) *xy = MATRIX_XY(m);
   if (yx) *yx = MATRIX_YX(m);
   if (yy) *yy = MATRIX_YY(m);
}

EAPI void
eina_matrix2_inverse(Eina_Matrix2 *out, const Eina_Matrix2 *mat)
{
   double         det;

   if (eina_matrix2_type_get(mat) == EINA_MATRIX_TYPE_IDENTITY)
     {
        eina_matrix2_copy(out, mat);
        return;
     }

   det = MATRIX_XX(mat) * MATRIX_YY(mat) - MATRIX_YX(mat) * MATRIX_XY(mat);

   if (EINA_DBL_EQ(det, 0.0))
     return;

   det = 1.0 / det;

   MATRIX_XX(out) =  MATRIX_YY(mat) * det;
   MATRIX_XY(out) = -MATRIX_XY(mat) * det;
   MATRIX_YX(out) = -MATRIX_YX(mat) * det;
   MATRIX_YY(out) =  MATRIX_XX(mat) * det;
}

EAPI void
eina_matrix2_identity(Eina_Matrix2 *m)
{
   MATRIX_XX(m) = 1.0;
   MATRIX_XY(m) = 0.0;

   MATRIX_YX(m) = 0.0;
   MATRIX_YY(m) = 1.0;
}

EAPI void
eina_matrix2_array_set(Eina_Matrix2 *m, const double *v)
{
   memcpy(&MATRIX_XX(m), v, sizeof(double) * 4);
}

EAPI void
eina_matrix2_copy(Eina_Matrix2 *dst, const Eina_Matrix2 *src)
{
   memcpy(dst, src, sizeof(Eina_Matrix2));
}

EAPI void
eina_matrix2_multiply(Eina_Matrix2 *out, const Eina_Matrix2 *mat_a, const Eina_Matrix2 *mat_b)
{
   if (eina_matrix2_type_get(mat_a) == EINA_MATRIX_TYPE_IDENTITY)
     {
        eina_matrix2_copy(out, mat_b);
        return;
     }

   if (eina_matrix2_type_get(mat_b) == EINA_MATRIX_TYPE_IDENTITY)
     {
        eina_matrix2_copy(out, mat_a);
        return;
     }

   MATRIX_XX(out) = MATRIX_XX(mat_a) * MATRIX_XX(mat_b) + MATRIX_YX(mat_a) * MATRIX_XY(mat_b);
   MATRIX_YX(out) = MATRIX_XX(mat_a) * MATRIX_YX(mat_b) + MATRIX_YX(mat_a) * MATRIX_YY(mat_b);

   MATRIX_XY(out) = MATRIX_XY(mat_a) * MATRIX_XX(mat_b) + MATRIX_YY(mat_a) * MATRIX_XY(mat_b);
   MATRIX_YY(out) = MATRIX_XY(mat_a) * MATRIX_YX(mat_b) + MATRIX_YY(mat_a) * MATRIX_YY(mat_b);
}

EAPI void
eina_matrix2_multiply_copy(Eina_Matrix2 *out, const Eina_Matrix2 *mat_a, const Eina_Matrix2 *mat_b)
{
   if (out != mat_a && out != mat_b)
     {
        eina_matrix2_multiply(out, mat_a, mat_b);
     }
   else
     {
        Eina_Matrix2 tmp;

        eina_matrix2_multiply(&tmp, mat_a, mat_b);
        eina_matrix2_copy(out, &tmp);
     }
}

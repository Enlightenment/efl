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

#include "eina_fp.h"
#include "eina_rectangle.h"
#include "eina_quad.h"
#include "eina_matrix.h"

#define MATRIX_XX(m) (m)->xx
#define MATRIX_XY(m) (m)->xy
#define MATRIX_XZ(m) (m)->xz
#define MATRIX_YX(m) (m)->yx
#define MATRIX_YY(m) (m)->yy
#define MATRIX_YZ(m) (m)->yz
#define MATRIX_ZX(m) (m)->zx
#define MATRIX_ZY(m) (m)->zy
#define MATRIX_ZZ(m) (m)->zz
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

   double y = (B * x) + (C * x * fabsf(x));

#ifdef EXTRA_PRECISION
   //  const float Q = 0.775;
   const double P = 0.225;

   y = P * (y * fabsf(y) - y) + y; // Q * y + P * y * abs(y)
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
   if ((MATRIX_ZX(m) != 0) || (MATRIX_ZY(m) != 0) || (MATRIX_ZZ(m) != 1))
     return EINA_MATRIX_TYPE_PROJECTIVE;
   else
     {
        if ((MATRIX_XX(m) == 1) && (MATRIX_XY(m) == 0) && (MATRIX_XZ(m) == 0) &&
            (MATRIX_YX(m) == 0) && (MATRIX_YY(m) == 1) && (MATRIX_YZ(m) == 0))
          return EINA_MATRIX_TYPE_IDENTITY;
        else
          return EINA_MATRIX_TYPE_AFFINE;
     }
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

   if (!MATRIX_ZX(m) && !MATRIX_ZY(m))
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
   if (!scalar)
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
   if (m1->xx != m2->xx ||
       m1->xy != m2->xy ||
       m1->xz != m2->xz ||
       m1->yx != m2->yx ||
       m1->yy != m2->yy ||
       m1->yz != m2->yz ||
       m1->zx != m2->zx ||
       m1->zy != m2->zy ||
       m1->zz != m2->zz)
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

   /* paralellogram */
   if (!ex && !ey)
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

        if (!den)
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
   if (MATRIX_ZZ(m) != 1)
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

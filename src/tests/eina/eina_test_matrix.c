/* EINA - EFL data type library
 * Copyright (C) 2015 Cedric Bail
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
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define DBL_EPSILON 0.0000001

#include <stdio.h>
#include <math.h>

#include <Eina.h>

#include "eina_suite.h"

EFL_START_TEST(eina_matrix2)
{
   Eina_Matrix2 m;
   Eina_Matrix2 n;
   double xx, xy, yx, yy;


   eina_matrix2_values_set(&m,
                           1, 0,
                           0, 1);
   fail_if(eina_matrix2_type_get(&m) != EINA_MATRIX_TYPE_IDENTITY);

   eina_matrix2_inverse(&n, &m);
   fail_if(eina_matrix2_type_get(&n) != EINA_MATRIX_TYPE_IDENTITY);

   eina_matrix2_values_get(&m,
                           &xx, &xy,
                           &yx, &yy);

   fail_if(!EINA_DBL_EQ(xx, yy) ||
           !EINA_DBL_EQ(yy, 1));
   fail_if(!EINA_DBL_EQ(xy, yx) ||
           !EINA_DBL_EQ(xy, 0));

}
EFL_END_TEST

EFL_START_TEST(eina_matrix2_operation)
{
   Eina_Matrix2 m1, m2, m3;
   double xx, xy, yx, yy;
   double arr[] = {1, 1, 1, 1};

   eina_matrix2_values_set(&m2,
                           2, 3,
                           3, 2);
   eina_matrix2_inverse(&m1, &m2);
   eina_matrix2_values_get(&m1,
                           &xx, &xy,
                           &yx, &yy);
   fail_if((xx - (-0.4) > DBL_EPSILON) || (xy - 0.6 > DBL_EPSILON) ||
           (yx - 0.6 > DBL_EPSILON) || (yy - (-0.4) > DBL_EPSILON));

   eina_matrix2_identity(&m1);
   eina_matrix2_values_get(&m1,
                           &xx, &xy,
                           &yx, &yy);
   fail_if(!EINA_DBL_EQ(xx, yy) ||
           !EINA_DBL_EQ(yy, 1));
   fail_if(!EINA_DBL_EQ(xy, yx) ||
           !EINA_DBL_EQ(xy, 0));

   eina_matrix2_array_set(&m1, arr);
   eina_matrix2_values_get(&m1,
                           &xx, &xy,
                           &yx, &yy);
   fail_if(!EINA_DBL_EQ(xx, yy) ||
           !EINA_DBL_EQ(yx, xy) ||
           !EINA_DBL_EQ(xy, 1));

   eina_matrix2_copy(&m2, &m1);
   eina_matrix2_values_get(&m2,
                           &xx, &xy,
                           &yx, &yy);
   fail_if(!EINA_DBL_EQ(xx, yy) ||
           !EINA_DBL_EQ(yx, xy) ||
           !EINA_DBL_EQ(xy, 1));

   eina_matrix2_values_set(&m1,
                           2, 3,
                           3, 2);
   eina_matrix2_multiply_copy(&m3, &m1, &m2);
   eina_matrix2_values_get(&m3,
                           &xx, &xy,
                           &yx, &yy);
   fail_if(!EINA_DBL_EQ(xx, yy) ||
           !EINA_DBL_EQ(yx, xy) ||
           !EINA_DBL_EQ(xy, 5));

   eina_matrix2_multiply(&m3, &m1, &m2);
   eina_matrix2_values_get(&m3,
                           &xx, &xy,
                           &yx, &yy);
   fail_if(!EINA_DBL_EQ(xx, yy) ||
           !EINA_DBL_EQ(yx, xy) ||
           !EINA_DBL_EQ(xy, 5));

}
EFL_END_TEST

EFL_START_TEST(eina_matrix4)
{
   Eina_Matrix4 m;
   Eina_Matrix4 n;
   double xx, xy, xz, xw,
     yx, yy, yz, yw,
     zx, zy, zz, zw,
     wx, wy, wz, ww;


   eina_matrix4_values_set(&m,
                           1, 0, 0, 0,
                           0, 1, 0, 0,
                           0, 0, 1, 0,
                           0, 0, 0, 1);
   fail_if(eina_matrix4_type_get(&m) != EINA_MATRIX_TYPE_IDENTITY);

   fail_if(!eina_matrix4_normalized(&n, &m));
   fail_if(eina_matrix4_type_get(&n) != EINA_MATRIX_TYPE_IDENTITY);

   fail_if(!eina_matrix4_inverse(&n, &m));
   fail_if(eina_matrix4_type_get(&n) != EINA_MATRIX_TYPE_IDENTITY);

   eina_matrix4_values_get(&m,
                           &xx, &xy, &xz, &xw,
                           &yx, &yy, &yz, &yw,
                           &zx, &zy, &zz, &zw,
                           &wx, &wy, &wz, &ww);

   fail_if(!EINA_DBL_EQ(xx, yy) ||
           !EINA_DBL_EQ(yy, zz) ||
           !EINA_DBL_EQ(zz, ww) ||
           !EINA_DBL_EQ(ww, 1));
   fail_if(!EINA_DBL_EQ(xy, xz) ||
           !EINA_DBL_EQ(xz, xw) ||
           !EINA_DBL_EQ(xw, yx) ||
           !EINA_DBL_EQ(yx, yz) ||
           !EINA_DBL_EQ(yz, yw) ||
           !EINA_DBL_EQ(yw, zx) ||
           !EINA_DBL_EQ(zx, zy) ||
           !EINA_DBL_EQ(zy, zw) ||
           !EINA_DBL_EQ(zw, wx) ||
           !EINA_DBL_EQ(wx, wy) ||
           !EINA_DBL_EQ(wy, wz) ||
           !EINA_DBL_EQ(wz, 0));

   eina_matrix4_values_set(&m,
                           1, 2, 3, 4,
                           5, 6, 7, 8,
                           9, 10, 11, 12,
                           13, 14, 15, 16);

   eina_matrix4_transpose(&n, &m);
   fail_if(!EINA_DBL_EQ(n.xx, 1) ||
           !EINA_DBL_EQ(n.xy, 5) ||
           !EINA_DBL_EQ(n.xz, 9) ||
           !EINA_DBL_EQ(n.xw, 13) ||
           !EINA_DBL_EQ(n.yx, 2) ||
           !EINA_DBL_EQ(n.yy, 6) ||
           !EINA_DBL_EQ(n.yz, 10) ||
           !EINA_DBL_EQ(n.yw, 14) ||
           !EINA_DBL_EQ(n.zx, 3) ||
           !EINA_DBL_EQ(n.zy, 7) ||
           !EINA_DBL_EQ(n.zz, 11) ||
           !EINA_DBL_EQ(n.zw, 15) ||
           !EINA_DBL_EQ(n.wx, 4) ||
           !EINA_DBL_EQ(n.wy, 8) ||
           !EINA_DBL_EQ(n.wz, 12) ||
           !EINA_DBL_EQ(n.ww, 16));

}
EFL_END_TEST

EFL_START_TEST(eina_matrix4_operation)
{
   double det;
   double l=5, r=4, b=4, t=3, dn=3, df=2;
   Eina_Matrix4 m, m1, m2;
   double xx, xy, xz, xw,
          yx, yy, yz, yw,
          zx, zy, zz, zw,
          wx, wy, wz, ww;
   const double arr[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

   eina_matrix4_values_set(&m,
                           0, 0, 2, 0,
                           2, 0, 0, 0,
                           0, 0, 0, 2,
                           0, 2, 0, 0);
   det = eina_matrix4_determinant(&m);
   fail_if(!EINA_DBL_EQ(det, -16));

   eina_matrix4_inverse(&m1, &m);
   eina_matrix4_values_get(&m1,
                           &xx, &xy, &xz, &xw,
                           &yx, &yy, &yz, &yw,
                           &zx, &zy, &zz, &zw,
                           &wx, &wy, &wz, &ww);
   fail_if (!EINA_DBL_EQ(xy, 0.5) ||
            !EINA_DBL_EQ(yw, xy) ||
            !EINA_DBL_EQ(zx, yw) ||
            !EINA_DBL_EQ(wz, zx));

   eina_matrix4_identity(&m1);
   eina_matrix4_values_get(&m1,
                           &xx, &xy, &xz, &xw,
                           &yx, &yy, &yz, &yw,
                           &zx, &zy, &zz, &zw,
                           &wx, &wy, &wz, &ww);
   fail_if (!EINA_DBL_EQ(xx, 1) ||
            !EINA_DBL_EQ(yy, xx) ||
            !EINA_DBL_EQ(zz, yy) ||
            !EINA_DBL_EQ(ww, zz));

   eina_matrix4_values_set(&m1,
                           0, 2, 2, 0,
                           2, 0, 0, 2,
                           2, 0, 0, 2,
                           0, 2, 2, 0);
   eina_matrix4_multiply_copy(&m2, &m1, &m);
   eina_matrix4_values_get(&m2,
                           &xx, &xy, &xz, &xw,
                           &yx, &yy, &yz, &yw,
                           &zx, &zy, &zz, &zw,
                           &wx, &wy, &wz, &ww);
   fail_if (!EINA_DBL_EQ(xx, 4) ||
            !EINA_DBL_EQ(xw, xx) ||
            !EINA_DBL_EQ(yy, xw) ||
            !EINA_DBL_EQ(yz, yy) ||
            !EINA_DBL_EQ(zy, yz) ||
            !EINA_DBL_EQ(zz, yy) ||
            !EINA_DBL_EQ(wx, xx) ||
            !EINA_DBL_EQ(ww, zy));

   eina_matrix4_multiply(&m2, &m1, &m);
   eina_matrix4_values_get(&m2,
                           &xx, &xy, &xz, &xw,
                           &yx, &yy, &yz, &yw,
                           &zx, &zy, &zz, &zw,
                           &wx, &wy, &wz, &ww);
   fail_if (!EINA_DBL_EQ(xx, 4) ||
            !EINA_DBL_EQ(xw, xx) ||
            !EINA_DBL_EQ(yy, xw) ||
            !EINA_DBL_EQ(yz, yy) ||
            !EINA_DBL_EQ(zy, yz) ||
            !EINA_DBL_EQ(zz, yy) ||
            !EINA_DBL_EQ(wx, xx) ||
            !EINA_DBL_EQ(ww, zy));

   eina_matrix4_array_set(&m1, arr);
   eina_matrix4_values_get(&m1,
                           &xx, &xy, &xz, &xw,
                           &yx, &yy, &yz, &yw,
                           &zx, &zy, &zz, &zw,
                           &wx, &wy, &wz, &ww);
   fail_if (!EINA_DBL_EQ(xx, 1) ||
            !EINA_DBL_EQ(xw, xx) ||
            !EINA_DBL_EQ(yy, xw) ||
            !EINA_DBL_EQ(yz, yy) ||
            !EINA_DBL_EQ(zy, yz) ||
            !EINA_DBL_EQ(zz, yy) ||
            !EINA_DBL_EQ(wx, xx) ||
            !EINA_DBL_EQ(ww, zy));

   eina_matrix4_copy(&m, &m1);
   eina_matrix4_values_get(&m1,
                           &xx, &xy, &xz, &xw,
                           &yx, &yy, &yz, &yw,
                           &zx, &zy, &zz, &zw,
                           &wx, &wy, &wz, &ww);
   fail_if (!EINA_DBL_EQ(xx, 1) ||
            !EINA_DBL_EQ(xw, xx) ||
            !EINA_DBL_EQ(yy, xw) ||
            !EINA_DBL_EQ(yz, yy) ||
            !EINA_DBL_EQ(zy, yz) ||
            !EINA_DBL_EQ(zz, yy) ||
            !EINA_DBL_EQ(wx, xx) ||
            !EINA_DBL_EQ(ww, zy));

   eina_matrix4_ortho_set(&m, l, r, b, t, dn, df);
   eina_matrix4_values_get(&m,
                           &xx, &xy, &xz, &xw,
                           &yx, &yy, &yz, &yw,
                           &zx, &zy, &zz, &zw,
                           &wx, &wy, &wz, &ww);
   fail_if (!EINA_DBL_EQ(xx, -2) ||
            !EINA_DBL_EQ(yy, -2) ||
            !EINA_DBL_EQ(zz, 2) ||
            !EINA_DBL_EQ(wx, 9) ||
            !EINA_DBL_EQ(wy, 7) ||
            !EINA_DBL_EQ(wz, 5) ||
            !EINA_DBL_EQ(ww, 1));
}
EFL_END_TEST

EFL_START_TEST(eina_matrix4_2_3)
{
   Eina_Matrix4 m4;
   Eina_Matrix3 m3, m3b;


   eina_matrix3_values_set(&m3,
                           1, 3, 2,
                           3, 1, 4,
                           2, 4, 1);

   eina_matrix3_matrix4_to(&m4, &m3);
   eina_matrix4_matrix3_to(&m3b, &m4);

   fail_if(memcmp(&m3, &m3b, sizeof (Eina_Matrix3)) != 0);

}
EFL_END_TEST

EFL_START_TEST(eina_matrix3)
{
   Eina_Bool ret;
   Eina_Matrix3 m, m1, m2;
   double xx, xy, xz,
          yx, yy, yz,
          zx, zy, zz;


   eina_matrix3_values_set(&m,
                           1, 0, 0,
                           0, 1, 0,
                           0, 0, 1);

   eina_matrix3_values_set(&m1,
                           1, 0, 0,
                           0, 1, 0,
                           0, 0, 1);

   eina_matrix3_values_set(&m2,
                           1, 1, 1,
                           0, 1, 0,
                           0, 0, 1);
   fail_if(eina_matrix3_type_get(&m) != EINA_MATRIX_TYPE_IDENTITY);

   eina_matrix3_values_get(&m,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);

   fail_if(!EINA_DBL_EQ(xx, yy) ||
           !EINA_DBL_EQ(yy, zz) ||
           !EINA_DBL_EQ(zz, 1));

   fail_if(!EINA_DBL_EQ(xy, xz) ||
           !EINA_DBL_EQ(yx, yz) ||
           !EINA_DBL_EQ(zx, zy) ||
           !EINA_DBL_EQ(zy, 0));

   ret = eina_matrix3_equal(&m, &m1);
   fail_if(ret != EINA_TRUE);

   ret = eina_matrix3_equal(&m1, &m2);
   fail_if(ret != EINA_FALSE);

}
EFL_END_TEST

#define MATRIX3_CMP(XX, XY, XZ, YX, YY, YZ, ZX, ZY, ZZ, AXX, AXY, AXZ, AYX, AYY, AYZ, AZX, AZY, AZZ) \
  (EINA_DBL_EQ(XX, AXX) && \
   EINA_DBL_EQ(XY, AXY) && \
   EINA_DBL_EQ(XZ, AXZ) && \
   EINA_DBL_EQ(YX, AYX) && \
   EINA_DBL_EQ(YY, AYY) && \
   EINA_DBL_EQ(YZ, AYZ) && \
   EINA_DBL_EQ(ZX, AZX) && \
   EINA_DBL_EQ(ZY, AZY) && \
   EINA_DBL_EQ(ZZ, AZZ))

EFL_START_TEST(eina_matrix3_operations)
{
   Eina_Matrix3 m1, m2, m3;
   double xx, xy, xz,
          yx, yy, yz,
          zx, zy, zz;
   double tx = 20, ty = 30, ret;
   const double arr[] = {1, 1, 1, 1, 1, 1, 1, 1, 1};

   eina_matrix3_values_set(&m1,
                           1, 0, 0,
                           0, 1, 0,
                           0, 0, 1);

   eina_matrix3_values_set(&m2,
                           1, 1, 1,
                           1, 1, 1,
                           1, 1, 1);


   eina_matrix3_compose(&m1, &m2, &m3);
   eina_matrix3_values_get(&m3,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);
   fail_if (!EINA_DBL_EQ(xx, xy) ||
            !EINA_DBL_EQ(xy, xz) ||
            !EINA_DBL_EQ(yx, yy) ||
            !EINA_DBL_EQ(yy, yz) ||
            !EINA_DBL_EQ(zx, zy) ||
            !EINA_DBL_EQ(zy, zz) ||
            !EINA_DBL_EQ(zz, 1));

   eina_matrix3_translate(&m1, tx, ty);
   eina_matrix3_values_get(&m1,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);
   fail_if(!MATRIX3_CMP(xx, xy, xz,
                        yx, yy, yz,
                        zx, zy, zz,
                        1, 0, tx,
                        0, 1, ty,
                        0, 0, 1));

   eina_matrix3_values_set(&m1,
                           1, 0, 0,
                           0, 1, 0,
                           0, 0, 1);

   eina_matrix3_scale(&m1, tx, ty);
   eina_matrix3_values_get(&m1,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);
   fail_if(!MATRIX3_CMP(xx, xy, xz,
                       yx, yy, yz,
                       zx, zy, zz,
                       tx, 0, 0,
                       0, ty, 0,
                       0, 0, 1));

   eina_matrix3_values_set(&m1,
                           1, 0, 0,
                           0, 1, 0,
                           0, 0, 1);
   eina_matrix3_rotate(&m1, M_PI/2);

   fail_if (!MATRIX3_CMP(round(m1.xx), round(m1.xy), round(m1.xz),
                         round(m1.yx), round(m1.yy), round(m1.yz),
                         round(m1.zx), round(m1.zy), round(m1.zz),
                         0, -1, 0,
                         1, 0, 0,
                         0, 0, 1));

   eina_matrix3_values_set(&m1,
                           1, 1, 1,
                           1, 1, 1,
                           1, 1, 1);
   eina_matrix3_identity(&m1);
   eina_matrix3_values_get(&m1,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);
   fail_if(!EINA_DBL_EQ(xx, yy) ||
           !EINA_DBL_EQ(yy, zz) ||
           !EINA_DBL_EQ(zz, 1));

   fail_if(!EINA_DBL_EQ(xy, xz) ||
           !EINA_DBL_EQ(yx, yz) ||
           !EINA_DBL_EQ(zx, zy) ||
           !EINA_DBL_EQ(zy, 0));

   eina_matrix3_values_set(&m1,
                           1, 2, 1,
                           2, 1, 1,
                           1, 2, 2);
   ret = eina_matrix3_determinant(&m1);
   fail_if(!EINA_DBL_EQ(ret, -3));

   eina_matrix3_values_set(&m1,
                           3, 3, 3,
                           3, 3, 3,
                           3, 3, 3);
   eina_matrix3_divide(&m1, 2);
   eina_matrix3_values_get(&m1,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);
   fail_if (!EINA_DBL_EQ(xx, xy) ||
            !EINA_DBL_EQ(xy, xz) ||
            !EINA_DBL_EQ(yx, yy) ||
            !EINA_DBL_EQ(yy, yz) ||
            !EINA_DBL_EQ(zx, zy) ||
            !EINA_DBL_EQ(zy, zz) ||
            !EINA_DBL_EQ(zz, 1.5));

   eina_matrix3_values_set(&m1,
                           0, 2, 0,
                           2, 2, 0,
                           2, 1, 2);
   eina_matrix3_inverse(&m1, &m2);
   eina_matrix3_values_get(&m2,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);
   fail_if(!MATRIX3_CMP(xx, xy, xz,
                        yx, yy, yz,
                        zx, zy, zz,
                        -0.5, 0.5, 0.0,
                        0.5, 0.0, 0.0,
                        0.25, -0.5, 0.5));

   eina_matrix3_values_set(&m1,
                           1, 2, 3,
                           4, 5, 6,
                           7, 8, 9);
   eina_matrix3_transpose(&m1, &m2);
   eina_matrix3_values_get(&m2,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);
   fail_if(!MATRIX3_CMP(xx, xy, xz,
                        yx, yy, yz,
                        zx, zy, zz,
                        1, 4, 7,
                        2, 5, 8,
                        3, 6, 9));

   eina_matrix3_values_set(&m1,
                           1, 2, 3,
                           0, 4, 5,
                           1, 0, 6);
   eina_matrix3_cofactor(&m1, &m2);
   eina_matrix3_values_get(&m2,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);
   fail_if(!MATRIX3_CMP(xx, xy, xz,
                        yx, yy, yz,
                        zx, zy, zz,
                        24, 5, -4,
                        -12, 3, 2,
                        -2, -5, 4));

   eina_matrix3_adjoint(&m1, &m2);
   eina_matrix3_values_get(&m2,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);
   fail_if(!MATRIX3_CMP(xx, xy, xz,
                        yx, yy, yz,
                        zx, zy, zz,
                        24, -12, -2,
                        5, 3, -5,
                        -4, 2, 4));

   eina_matrix3_values_set(&m2,
                           2, 2, 2,
                           1, 1, 1,
                           3, 3, 3);
   eina_matrix3_values_set(&m3,
                           3, 3, 3,
                           2, 2, 2,
                           1, 1, 1);
   eina_matrix3_multiply_copy(&m1, &m2, &m3);
   eina_matrix3_values_get(&m1,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);
   fail_if(!MATRIX3_CMP(xx, xy, xz,
                        yx, yy, yz,
                        zx, zy, zz,
                        12, 12, 12,
                        6, 6, 6,
                        18, 18, 18));

   eina_matrix3_values_set(&m2,
                           2, 2, 2,
                           1, 1, 1,
                           0, 0, 0);
   eina_matrix3_multiply(&m1, &m2, &m3);
   eina_matrix3_values_get(&m1,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);
   fail_if(!MATRIX3_CMP(xx, xy, xz,
                        yx, yy, yz,
                        zx, zy, zz,
                        12, 12, 12,
                        6, 6, 6,
                        0, 0, 0));

   eina_matrix3_copy(&m1, &m2);
   eina_matrix3_values_get(&m1,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);
   fail_if(!MATRIX3_CMP(xx, xy, xz,
                        yx, yy, yz,
                        zx, zy, zz,
                        2, 2, 2,
                        1, 1, 1,
                        0, 0, 0));

   eina_matrix3_array_set(&m1, arr);
   eina_matrix3_values_get(&m1,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);
   fail_if(!MATRIX3_CMP(xx, xy, xz,
                        yx, yy, yz,
                        zx, zy, zz,
                        1, 1, 1,
                        1, 1, 1,
                        1, 1, 1));

   eina_matrix3_position_transform_set(&m3, 5, 3);
   eina_matrix3_values_get(&m3,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);
   fail_if (!EINA_DBL_EQ(xz, 5) ||
            !EINA_DBL_EQ(yz, 3));

   eina_matrix3_scale_transform_set(&m2, 6, 7);
   eina_matrix3_values_get(&m2,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);
   fail_if (!EINA_DBL_EQ(xx, 6) ||
            !EINA_DBL_EQ(yy, 7));
}
EFL_END_TEST

EFL_START_TEST(eina_matrix3_f16p16)
{
   Eina_Matrix3_F16p16 m1;
   Eina_Matrix3 m2;
   Eina_F16p16 xx, xy, xz,
               yx, yy, yz,
               zx, zy, zz;
   Eina_Matrix3_F16p16 m3;


   eina_matrix3_values_set(&m2,
                           1, 0, 0,
                           0, 1, 0,
                           0, 0, 1);
   eina_matrix3_matrix3_f16p16_to(&m2, &m1);
   fail_if (eina_matrix3_f16p16_type_get(&m1) != EINA_MATRIX_TYPE_IDENTITY);

   eina_matrix3_fixed_values_get(&m2,
                                 &xx, &xy, &xz,
                                 &yx, &yy, &yz,
                                 &zx, &zy, &zz);
   fail_if(xx != yy ||
           yy != zz ||
           zz != 65536);

   fail_if(xy != xz ||
           yx != yz ||
           zx != zy ||
           zy != 0);

   eina_matrix3_values_set(&m2,
                           1, 2, 3,
                           4, 5, 6,
                           7, 8, 9);
   eina_matrix3_matrix3_f16p16_to(&m2, &m1);
   eina_matrix3_f16p16_identity(&m1);
   fail_if(m1.xx != m1.yy ||
           m1.yy != m1.zz ||
           m1.zz != 65536);

   fail_if(m1.xy != m1.xz ||
           m1.yx != m1.yz ||
           m1.zx != m1.zy ||
           m1.zy != 0);

   eina_matrix3_values_set(&m2,
                           1, 1, 1,
                           1, 1, 1,
                           1, 1, 1);
   eina_matrix3_matrix3_f16p16_to(&m2, &m1);
   eina_matrix3_f16p16_compose(&m1, &m1, &m3);
   fail_if (m3.xx != m3.xy ||
            m3.xy != m3.xz ||
            m3.yx != m3.yy ||
            m3.yy != m3.yz ||
            m3.zx != m3.zy ||
            m3.zy != m3.zz ||
            m3.zz != 196608);

}
EFL_END_TEST

EFL_START_TEST(eina_matrix3_map_transform)
{
   double x = 2, y = 3, x1, y1;
   Eina_Matrix3 m;
   Eina_Rectangle r;
   Eina_Quad q;
   Eina_Bool ret;


   eina_matrix3_values_set(&m,
                           0, 1, 0,
                           1, 0, 0,
                           0, 0, 1);

   eina_matrix3_point_transform(&m,
                                x, y,
                                &x1, &y1);
   fail_if(!EINA_DBL_EQ(x1, 3) ||
           !EINA_DBL_EQ(y1, 2));

   EINA_RECTANGLE_SET(&r, 0, 0, 3, 4);
   eina_matrix3_rectangle_transform(&m, &r, &q);

   fail_if(!EINA_DBL_EQ(q.x0, 0) ||
           !EINA_DBL_EQ(q.y0, 0) ||
           !EINA_DBL_EQ(q.x1, 0) ||
           !EINA_DBL_EQ(q.y1, 3) ||
           !EINA_DBL_EQ(q.x2, 4) ||
           !EINA_DBL_EQ(q.y2, 3) ||
           !EINA_DBL_EQ(q.x3, 4) ||
           !EINA_DBL_EQ(q.y3, 0));

   eina_quad_coords_set(&q,
                        0.0, 0.0,
                        3.0, 0.0,
                        3.0, 3.0,
                        0.0, 3.0);
   ret = eina_matrix3_square_quad_map(&m, &q);
   fail_if(ret != EINA_TRUE);

   fail_if(!MATRIX3_CMP(m.xx, m.xy, m.xz,
                        m.yx, m.yy, m.yz,
                        m.zx, m.zy, m.zz,
                        3, 0, 0,
                        0, 3, 0,
                        0, 0, 1));

   ret = eina_matrix3_quad_square_map(&m, &q);
   fail_if(ret != EINA_TRUE);

   fail_if(!EINA_DBL_EQ(q.x0, 0) ||
           !EINA_DBL_EQ(q.y0, 0) ||
           !EINA_DBL_EQ(q.x1, 3) ||
           !EINA_DBL_EQ(q.y1, 0) ||
           !EINA_DBL_EQ(q.x2, 3) ||
           !EINA_DBL_EQ(q.y2, 3) ||
           !EINA_DBL_EQ(q.x3, 0) ||
           !EINA_DBL_EQ(q.y3, 3));

}
EFL_END_TEST

EFL_START_TEST(eina_normal3_test)
{
   Eina_Matrix3 out;
   Eina_Matrix4 m;
   double xx, xy, xz,
          yx, yy, yz,
          zx, zy, zz;

   eina_matrix4_values_set(&m,
                           1, 0, 0, 0,
                           0, 1, 0, 0,
                           0, 0, 1, 0,
                           0, 0, 0, 1);
   eina_normal3_matrix_get(&out, &m);
   eina_matrix3_values_get(&out,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz
                           );

   fail_if((fabs(xy) - 1.0) > DBL_EPSILON ||
           (fabs(yx) - 1.0) > DBL_EPSILON ||
           (fabs(yz) - 1.0) > DBL_EPSILON ||
           (fabs(zy) - 1.0) > DBL_EPSILON
          );

   eina_matrix4_values_set(&m,
                           0, 0, 0, 0,
                           0, 1, 0, 0,
                           0, 0, 0, 0,
                           0, 0, 0, 1);
   eina_normal3_matrix_get(&out, &m);
   eina_matrix3_values_get(&out,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz
                           );
   fail_if((fabs(xy)) > DBL_EPSILON ||
           (fabs(yx)) > DBL_EPSILON ||
           (fabs(yz)) > DBL_EPSILON ||
           (fabs(zy)) > DBL_EPSILON
          );
}
EFL_END_TEST

void
eina_test_matrix(TCase *tc)
{
   tcase_add_test(tc, eina_matrix2);
   tcase_add_test(tc, eina_matrix2_operation);
   tcase_add_test(tc, eina_matrix4);
   tcase_add_test(tc, eina_matrix4_operation);
   tcase_add_test(tc, eina_matrix4_2_3);
   tcase_add_test(tc, eina_matrix3);
   tcase_add_test(tc, eina_matrix3_operations);
   tcase_add_test(tc, eina_matrix3_f16p16);
   tcase_add_test(tc, eina_matrix3_map_transform);
   tcase_add_test(tc, eina_normal3_test);
}

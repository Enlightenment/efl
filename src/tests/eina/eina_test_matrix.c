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

#include <math.h>
#include <stdio.h>

#include "eina_suite.h"
#include "Eina.h"

START_TEST(eina_matrix4)
{
   Eina_Matrix4 m;
   double xx, xy, xz, xw,
     yx, yy, yz, yw,
     zx, zy, zz, zw,
     wx, wy, wz, ww;

   eina_init();

   eina_matrix4_values_set(&m,
                           1, 0, 0, 0,
                           0, 1, 0, 0,
                           0, 0, 1, 0,
                           0, 0, 0, 1);
   fail_if(eina_matrix4_type_get(&m) != EINA_MATRIX_TYPE_IDENTITY);

   eina_matrix4_values_get(&m,
                           &xx, &xy, &xz, &xw,
                           &yx, &yy, &yz, &yw,
                           &zx, &zy, &zz, &zw,
                           &wx, &wy, &wz, &ww);

   fail_if(xx != yy ||
           yy != zz ||
           zz != ww ||
           ww != 1);
   fail_if(xy != xz ||
           xz != xw ||
           xw != yx ||
           yx != yz ||
           yz != yw ||
           yw != zx ||
           zx != zy ||
           zy != zw ||
           zw != wx ||
           wx != wy ||
           wy != wz ||
           wz != 0);

   eina_shutdown();
}
END_TEST

START_TEST(eina_matrix4_2_3)
{
   Eina_Matrix4 m4, m4b;
   Eina_Matrix3 m3;

   eina_init();

   eina_matrix4_values_set(&m4,
                           1, 3, 2, 0,
                           3, 1, 4, 0,
                           2, 4, 1, 0,
                           0, 0, 0, 1);

   eina_matrix4_matrix3_to(&m3, &m4);
   eina_matrix3_matrix4_to(&m4b, &m3);

   fail_if(memcmp(&m4, &m4b, sizeof (Eina_Matrix4)) != 0);

   eina_shutdown();
}
END_TEST

START_TEST(eina_matrix3)
{
   Eina_Bool ret;
   Eina_Matrix3 m, m1, m2;
   double xx, xy, xz,
          yx, yy, yz,
          zx, zy, zz;

          eina_init();

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

   fail_if(xx != yy ||
           yy != zz ||
           zz != 1);

   fail_if(xy != xz ||
           yx != yz ||
           zx != zy ||
           zy != 0);

   ret = eina_matrix3_equal(&m, &m1);
   fail_if(ret != EINA_TRUE);

   ret = eina_matrix3_equal(&m1, &m2);
   fail_if(ret != EINA_FALSE);

   eina_shutdown();
}
END_TEST

START_TEST(eina_matrix3_operations)
{
   Eina_Matrix3 m1, m2, m3;
   double xx, xy, xz,
          yx, yy, yz,
          zx, zy, zz;
   double tx = 20, ty = 30, ret;

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
   fail_if (xx != xy ||
            xy != xz ||
            yx != yy ||
            yy != yz ||
            zx != zy ||
            zy != zz ||
            zz != 1);

   eina_matrix3_translate(&m1, tx, ty);
   eina_matrix3_values_get(&m1,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);
   fail_if (xx != 1 || xy != 0 || xz != tx ||
            yx != 0 || yy != 1 || yz != ty ||
            zx != 0 || zy != 0 || zz != 1);

   eina_matrix3_values_set(&m1,
                           1, 0, 0,
                           0, 1, 0,
                           0, 0, 1);

   eina_matrix3_scale(&m1, tx, ty);
   eina_matrix3_values_get(&m1,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);
   fail_if (xx != tx || xy != 0 || xz != 0 ||
            yx != 0 || yy != ty || yz != 0 ||
            zx != 0 || zy != 0 || zz != 1);

   eina_matrix3_values_set(&m1,
                           1, 0, 0,
                           0, 1, 0,
                           0, 0, 1);
   eina_matrix3_rotate(&m1, M_PI/2);

   fail_if (round(m1.xx) != 0 || round(m1.xy) != -1 ||
            round(m1.xz) != 0 || round(m1.yx) != 1 ||
            round(m1.yy) != 0 || round(m1.yz) !=0 ||
            round(m1.zx) != 0 || round(m1.zy) != 0||
            round(m1.zz) != 1);

   eina_matrix3_values_set(&m1,
                           1, 1, 1,
                           1, 1, 1,
                           1, 1, 1);
   eina_matrix3_identity(&m1);
   eina_matrix3_values_get(&m1,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);
   fail_if(xx != yy ||
           yy != zz ||
           zz != 1);

   fail_if(xy != xz ||
           yx != yz ||
           zx != zy ||
           zy != 0);


   eina_matrix3_values_set(&m1,
                           1, 2, 1,
                           2, 1, 1,
                           1, 2, 2);
   ret = eina_matrix3_determinant(&m1);
   fail_if(ret != -3);

   eina_matrix3_values_set(&m1,
                           3, 3, 3,
                           3, 3, 3,
                           3, 3, 3);
   eina_matrix3_divide(&m1, 2);
   eina_matrix3_values_get(&m1,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);
   fail_if (xx != xy ||
            xy != xz ||
            yx != yy ||
            yy != yz ||
            zx != zy ||
            zy != zz ||
            zz != 1.5);

   eina_matrix3_values_set(&m1,
                           0, 2, 0,
                           2, 2, 0,
                           2, 1, 2);
   eina_matrix3_inverse(&m1, &m2);
   eina_matrix3_values_get(&m2,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);
   fail_if (xx != -0.5 || xy != 0.5 || xz != 0 ||
            yx != 0.5 || yy != 0 || yz != 0 ||
            zx != 0.25 || zy != -0.5 || zz != 0.5);

   eina_matrix3_values_set(&m1,
                           1, 2, 3,
                           4, 5, 6,
                           7, 8, 9);
   eina_matrix3_transpose(&m1, &m2);
   eina_matrix3_values_get(&m2,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);
   fail_if (xx != 1 || xy != 4 || xz != 7 ||
            yx != 2 || yy != 5 || yz != 8 ||
            zx != 3 || zy != 6 || zz != 9);

   eina_matrix3_values_set(&m1,
                           1, 2, 3,
                           0, 4, 5,
                           1, 0, 6);
   eina_matrix3_cofactor(&m1, &m2);
   eina_matrix3_values_get(&m2,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);
   fail_if (xx != 24 || xy != 5 || xz != -4 ||
            yx != -12 || yy != 3 || yz != 2 ||
            zx != -2 || zy != -5 || zz != 4);

   eina_matrix3_adjoint(&m1, &m2);
   eina_matrix3_values_get(&m2,
                           &xx, &xy, &xz,
                           &yx, &yy, &yz,
                           &zx, &zy, &zz);
   fail_if (xx != 24 || xy != -12 || xz != -2 ||
            yx != 5 || yy != 3 || yz != -5 ||
            zx != -4 || zy != 2 || zz != 4);
}
END_TEST

START_TEST(eina_matrix3_f16p16)
{
   Eina_Matrix3_F16p16 m1;
   Eina_Matrix3 m2;
   Eina_F16p16 xx, xy, xz,
               yx, yy, yz,
               zx, zy, zz;
   Eina_Matrix3_F16p16 m3;

   eina_init();

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

   eina_shutdown();
}
END_TEST

void
eina_test_matrix(TCase *tc)
{
   tcase_add_test(tc, eina_matrix4);
   tcase_add_test(tc, eina_matrix4_2_3);
   tcase_add_test(tc, eina_matrix3);
   tcase_add_test(tc, eina_matrix3_operations);
   tcase_add_test(tc, eina_matrix3_f16p16);
}

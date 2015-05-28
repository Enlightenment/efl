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

void
eina_test_matrix(TCase *tc)
{
   tcase_add_test(tc, eina_matrix4);
   tcase_add_test(tc, eina_matrix4_2_3);
}

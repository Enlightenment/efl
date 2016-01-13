/* EINA - EFL data type library
 * Copyright (C) 2016 Sergey Osadchy
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
#include <float.h>
#include <limits.h>

#include "eina_suite.h"
#include "Eina.h"

START_TEST(eina_test_vector2_operations)
{
   Eina_Vector2 v1;
   Eina_Vector2 v2;
   Eina_Vector2 v3;
   Eina_Matrix2 m2;
   Eina_Matrix3 m3;
   double x = 1;
   double y = 2;
   double res = 0;
   const double arr[] = {5, 5};

   eina_init();

   eina_vector2_set(&v1, x, y);
   fail_if((v1.x != 1) || (v1.y != 2));

   eina_vector2_array_set(&v2, &arr);
   fail_if((v2.x != 5) || (v2.y != 5));

   eina_vector2_copy(&v1, &v2);
   fail_if((v1.x != 5) || (v1.y != 5));

   eina_vector2_negate(&v1, &v2);
   fail_if((v1.x != -5) || (v1.y != -5));

   eina_vector2_set(&v1, x, y);
   eina_vector2_add(&v3, &v1, &v2);
   fail_if((v3.x != 6) || (v3.y != 7));

   eina_vector2_subtract(&v3, &v2, &v1);
   fail_if((v3.x != 4) || (v3.y != 3));

   eina_vector2_scale(&v3, &v1, y);
   fail_if((v3.x != 2) || (v3.y != 4));

   res = eina_vector2_dot_product(&v1, &v2);
   fail_if(res != 15);

   res = eina_vector2_length_get(&v2);
   fail_if((res - sqrt(50)) > DBL_EPSILON);

   res = eina_vector2_length_square_get(&v2);
   fail_if(res != 50);

   res = eina_vector2_distance_get(&v2, &v1);
   fail_if((res - sqrt(25)) > DBL_EPSILON);

   res = eina_vector2_distance_square_get(&v2, &v1);
   fail_if(res != 25);

   eina_vector2_normalize(&v3, &v2);
   fail_if((v3.x - v2.x / sqrt(50)) > DBL_EPSILON ||
           (v3.y - v2.y / sqrt(50)) > DBL_EPSILON);

   eina_matrix2_values_set(&m2,
                          2, 2,
                          2, 2);
   eina_vector2_transform(&v3, &m2, &v1);
   fail_if((v3.x != 6) || (v3.y != 6));

   eina_matrix3_values_set(&m3,
                          2, 2, 2,
                          2, 2, 2,
                          2, 2, 2);
   eina_vector2_homogeneous_position_transform(&v3, &m3, &v1);
   fail_if((v3.x != 1) || (v3.y != 1));

   eina_vector2_homogeneous_direction_transform(&v3, &m3, &v1);
   fail_if((v3.x != 6) || (v3.y != 6));

   eina_shutdown();
}
END_TEST

void
eina_test_vector(TCase *tc)
{
   tcase_add_test(tc, eina_test_vector2_operations);
}

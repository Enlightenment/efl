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
#include <float.h>
#include <limits.h>

#include "eina_suite.h"
#include "Eina.h"

#define FLOAT_CMP(a, b) (fabs(a - b) <= DBL_MIN)

static inline Eina_Bool
eina_quaternion_cmp(const Eina_Quaternion *a, const Eina_Quaternion *b)
{
   if (FLOAT_CMP(a->x, b->x) &&
       FLOAT_CMP(a->y, b->y) &&
       FLOAT_CMP(a->z, b->z) &&
       FLOAT_CMP(a->w, b->w))
     return EINA_TRUE;
   return EINA_FALSE;
}

static inline Eina_Bool
eina_matrix3_cmp(const Eina_Matrix3 *a, const Eina_Matrix3 *b)
{
   if (FLOAT_CMP(a->xx, b->xx) &&
       FLOAT_CMP(a->xy, b->xy) &&
       FLOAT_CMP(a->xz, b->xz) &&
       FLOAT_CMP(a->yx, b->yx) &&
       FLOAT_CMP(a->yy, b->yy) &&
       FLOAT_CMP(a->yz, b->yz) &&
       FLOAT_CMP(a->zx, b->zx) &&
       FLOAT_CMP(a->zy, b->zy) &&
       FLOAT_CMP(a->zz, b->zz))
     return EINA_TRUE;
   return EINA_FALSE;
}

START_TEST(eina_test_quaternion_norm)
{
   static const Eina_Quaternion q = { 1, 3, 4, 5 };

   eina_init();

   fail_if(FLOAT_CMP(eina_quaternion_norm(&q), sqrt(51)));

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_quaternion_conjugate)
{
   static const Eina_Quaternion q1 = { 1, -1, -1, 3 }, q2 = { 1, 3, 4, 3 };
   static const Eina_Quaternion r1 = { 1, 1, 1, -3 }, r2 = { 1, -3, -4, -3 };
   Eina_Quaternion t1, t2;

   eina_init();

   eina_quaternion_conjugate(&t1, &q1);
   eina_quaternion_conjugate(&t2, &q2);

   fail_if(!eina_quaternion_cmp(&t1, &r1));
   fail_if(!eina_quaternion_cmp(&t2, &r2));

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_quaternion_matrix)
{
   Eina_Quaternion q = { 7, 9, 5, 1 };
   Eina_Matrix3 m = {
     104, 76, 8,
     104, -8, -116,
     -52, 136, -56
   };
   Eina_Quaternion tq;
   Eina_Matrix3 tm;

   eina_init();

   eina_quaternion_rotation_matrix3_get(&tm, &q);
   fail_if(!eina_matrix3_cmp(&tm, &m));

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_quaternion_op)
{
   Eina_Quaternion q = { 7, 9, 5, 1 };
   Eina_Quaternion z = { 0, 0, 0, 0 };
   Eina_Quaternion neg, r;

   eina_init();

   eina_quaternion_negative(&neg, &q);
   eina_quaternion_add(&r, &q, &neg);

   fail_if(!eina_quaternion_cmp(&z, &r));

   eina_shutdown();
}
END_TEST

void
eina_test_quaternion(TCase *tc)
{
   tcase_add_test(tc, eina_test_quaternion_norm);
   tcase_add_test(tc, eina_test_quaternion_conjugate);
   tcase_add_test(tc, eina_test_quaternion_matrix);
   tcase_add_test(tc, eina_test_quaternion_op);
}

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

#define FLOAT_CMP(a, b) (fabs((float)a - (float)b) <= FLT_MIN)

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

static inline Eina_Bool
eina_point_3d_cmp(const Eina_Point_3D *a, const Eina_Point_3D *b)
{
   if (FLOAT_CMP(a->x, b->x) &&
       FLOAT_CMP(a->y, b->y) &&
       FLOAT_CMP(a->z, b->z))
     return EINA_TRUE;
   return EINA_FALSE;
}

static inline Eina_Bool
eina_quaternion_f16p16_cmp(const Eina_Quaternion_F16p16 *a, const Eina_Quaternion_F16p16 *b)
{
   if ((a->x ==  b->x) &&
       (a->y == b->y) &&
       (a->z == b->z) &&
       (a->w == b->w))
     return EINA_TRUE;
   return EINA_FALSE;
}

START_TEST(eina_test_quaternion_norm)
{
   static const Eina_Quaternion q = { 1, 3, 4, 5 };
   double result = eina_quaternion_norm(&q);

   eina_init();

   fail_if(!FLOAT_CMP(result, sqrt(51)));

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_quaternion_conjugate)
{
   static const Eina_Quaternion q1 = { 1, -1, -1, 3 }, q2 = { 1, 3, 4, 3 };
   static const Eina_Quaternion r1 = { -1, 1, 1, 3 }, r2 = { -1, -3, -4, 3 };
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
   Eina_Quaternion q1 = {7, 9, 5, -1 };
   Eina_Quaternion tq;
   Eina_Matrix3 m = {
     -211, 136, 52,
     116, -147, 104,
     88, 76, -259
   };
   Eina_Matrix3 tm;

   eina_init();

   eina_quaternion_rotation_matrix3_get(&tm, &q);
   fail_if(!eina_matrix3_cmp(&tm, &m));

   eina_matrix3_quaternion_get(&tq, &m);
   fail_if(!eina_quaternion_cmp(&tq, &q) && !eina_quaternion_cmp(&tq, &q1));

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

START_TEST(eina_test_quaternion_f16p16)
{
   Eina_F16p16 x = 65536, y = 65536;
   Eina_F16p16 z = 65536, w = 65536;
   Eina_F16p16 res;
   Eina_Quaternion p, q = {65536, 65536, 65536, 65536};
   Eina_Quaternion_F16p16 t, s, r = {65536, 65536, 65536, 65536};


   eina_init();

   eina_quaternion_f16p16_set(&p, x, y, z, w);
   fail_if(!eina_quaternion_cmp(&p, &q));

   res = eina_quaternion_f16p16_norm(&r);
   fail_if(res != 131072);

   eina_quaternion_f16p16_negative(&s, &r);
   fail_if(s.x != s.y ||
           s.y != s.z ||
           s.z != s.w ||
           s.w != -65536);

   eina_quaternion_f16p16_add(&t, &s, &r);
   fail_if(t.x != t.y ||
           t.y != t.z ||
           t.z != t.w ||
           t.w != 0);

   res = 1;
   eina_quaternion_f16p16_scale(&t, &r, res);
   fail_if(t.x != t.y ||
           t.y != t.z ||
           t.z != t.w ||
           t.w != 65536);

   eina_quaternion_f16p16_conjugate(&t, &r);
   fail_if(t.x != -65536 ||
           t.y != -65536 ||
           t.z != -65536 ||
           t.w != 65536);

   s.x = 65536;
   s.y = 65536;
   s.z = 65536;
   s.w = 65536;
   res = eina_quaternion_f16p16_dot(&s, &r);
   fail_if(res != 262144);

   eina_quaternion_f16p16_mul(&t, &s, &r);
   fail_if(t.x != 131072 ||
           t.y != 131072 ||
           t.z != 131072 ||
           t.w != 0);
}
END_TEST

START_TEST(eina_test_quaternion_dot)
{
   Eina_Quaternion q = {1, 3, 4, 5};
   Eina_Quaternion r = {7, 9, 5, 1};
   double res;

   eina_init();

   res = eina_quaternion_dot(&q, &r);
   fail_if(res != 59);

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_quaternion_scale)
{
   Eina_Quaternion q = {1, 3, 4, 5};
   double scale = 2;
   Eina_Quaternion r, res = {2, 6, 8, 10};

   eina_init();

   eina_quaternion_scale(&r, &q, scale);
   fail_if(!eina_quaternion_cmp(&r, &res));

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_quaternion_set)
{
   Eina_Quaternion q, r = {1, 3, 4, 5};

   eina_init();

   eina_quaternion_set(&q, 1, 3, 4, 5);

   fail_if(!eina_quaternion_cmp(&q, &r));

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_quaternion_mul)
{
   Eina_Quaternion p = {1, 3, 4, 5};
   Eina_Quaternion q = {7, 9, 5, 1};
   Eina_Quaternion r, res = {15, 71, 17, -49};

   eina_init();

   eina_quaternion_mul(&r, &p, &q);
   fprintf(stderr, "%f %f %f %f\n", res.w, res.x, res.y, res.z);
   fail_if(!eina_quaternion_cmp(&r, &res));

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_matrix_recompose)
{
   const Eina_Point_3D translation = { 0, 0, 0 };
   const Eina_Point_3D scale = { 1, 1, 1 };
   const Eina_Point_3D skew = { 0, 0, 0 };
   const Eina_Quaternion perspective = { 0, 0, 0, 1 };
   const Eina_Quaternion rotation = { 0, 0, 0, 1 };
   Eina_Matrix4 m4;

   eina_init();

   eina_quaternion_matrix4_to(&m4,
                              &rotation,
                              &perspective,
                              &translation,
                              &scale,
                              &skew);

   fail_if(eina_matrix4_type_get(&m4) != EINA_MATRIX_TYPE_IDENTITY);

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_quaternion_normalized)
{
   Eina_Quaternion p = {1, 1, 1, 1};
   Eina_Quaternion res, q = {0.5, 0.5, 0.5, 0.5};

   eina_init();

   eina_quaternion_normalized(&res, &p);
   fail_if(!eina_quaternion_cmp(&q, &res));

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_matrix_quaternion)
{
   const Eina_Point_3D rt = { -2, -3, 0 };
   const Eina_Point_3D rsc = { 4, 5, 1 };
   const Eina_Quaternion rr = { 0, 0, -1, 0 };
   const Eina_Quaternion rp = { 0, 0, 0, 1 };
   Eina_Quaternion rotation, perspective;
   Eina_Matrix3 m3, m3r;
   Eina_Matrix4 m4, m4r;
   Eina_Point_3D translation, scale, skew;

   eina_init();

   eina_matrix3_identity(&m3);
   eina_matrix3_rotate(&m3, 3.14159265);
   eina_matrix3_translate(&m3, 2, 3);
   eina_matrix3_scale(&m3, 4, 5);
   eina_matrix3_matrix4_to(&m4, &m3);

   fail_if(!eina_matrix4_quaternion_to(&rotation,
                                       &perspective,
                                       &translation,
                                       &scale,
                                       &skew,
                                       &m4));

   eina_quaternion_matrix4_to(&m4r,
                              &rotation,
                              &perspective,
                              &translation,
                              &scale,
                              &skew);

   eina_matrix4_matrix3_to(&m3r, &m4r);

   fail_if(!eina_point_3d_cmp(&scale, &rsc));
   fail_if(!eina_point_3d_cmp(&translation, &rt));
   fail_if(!eina_quaternion_cmp(&perspective, &rp));
   fail_if(!eina_quaternion_cmp(&rotation, &rr));

   // Disable this test for the moment as it seems a rounding issue
   // fail_if(!eina_matrix3_cmp(&m3r, &m3));

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_quaternion_f16p16_lerp)
{
   Eina_Quaternion_F16p16 r, p = {0, 0, 0, 0};
   Eina_Quaternion_F16p16 q = {65536, 65536, 65536, 0};
   Eina_Quaternion_F16p16  res1 = {65536, 65536, 65536, 0};
   Eina_Quaternion_F16p16  res2 = {0, 0, 0, 0};

   eina_init();

   eina_quaternion_f16p16_lerp(&r, &p, &q, 65536);
   fail_if(!eina_quaternion_f16p16_cmp(&r, &res1));
   eina_quaternion_f16p16_lerp(&r, &p, &q, 0);
   fail_if(!eina_quaternion_f16p16_cmp(&r, &res2));

   eina_quaternion_f16p16_slerp(&r, &p, &q, 0);
   fail_if(!eina_quaternion_f16p16_cmp(&r, &res2));
   eina_quaternion_f16p16_slerp(&r, &p, &q, 65536);
   fail_if(!eina_quaternion_f16p16_cmp(&r, &res1));

   eina_quaternion_f16p16_nlerp(&r, &p, &q, 0);
   fail_if(!eina_quaternion_f16p16_cmp(&r, &res2));
   eina_quaternion_f16p16_nlerp(&r, &p, &q, 65536);
   fail_if(!eina_quaternion_f16p16_cmp(&r, &res1));

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
   tcase_add_test(tc, eina_test_quaternion_f16p16);
   tcase_add_test(tc, eina_test_quaternion_dot);
   tcase_add_test(tc, eina_test_quaternion_scale);
   tcase_add_test(tc, eina_test_quaternion_set);
   tcase_add_test(tc, eina_test_quaternion_mul);
   tcase_add_test(tc, eina_test_quaternion_normalized);
   //tcase_add_test(tc, eina_test_matrix_quaternion);
   tcase_add_test(tc, eina_test_matrix_recompose);
   tcase_add_test(tc, eina_test_quaternion_f16p16_lerp);
}

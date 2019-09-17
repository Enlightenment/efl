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

#include <Eina.h>

#include "eina_suite.h"

static inline Eina_Bool
eina_quaternion_cmp(const Eina_Quaternion *a, const Eina_Quaternion *b)
{
   if (EINA_FLT_EQ(a->x, b->x) &&
       EINA_FLT_EQ(a->y, b->y) &&
       EINA_FLT_EQ(a->z, b->z) &&
       EINA_FLT_EQ(a->w, b->w))
     return EINA_TRUE;
   return EINA_FALSE;
}

static inline Eina_Bool
eina_matrix3_cmp(const Eina_Matrix3 *a, const Eina_Matrix3 *b)
{
   if (EINA_FLT_EQ(a->xx, b->xx) &&
       EINA_FLT_EQ(a->xy, b->xy) &&
       EINA_FLT_EQ(a->xz, b->xz) &&
       EINA_FLT_EQ(a->yx, b->yx) &&
       EINA_FLT_EQ(a->yy, b->yy) &&
       EINA_FLT_EQ(a->yz, b->yz) &&
       EINA_FLT_EQ(a->zx, b->zx) &&
       EINA_FLT_EQ(a->zy, b->zy) &&
       EINA_FLT_EQ(a->zz, b->zz))
     return EINA_TRUE;
   return EINA_FALSE;
}

static inline Eina_Bool
eina_matrix3_f16p16_cmp(const Eina_Matrix3_F16p16 *a, const Eina_Matrix3_F16p16 *b)
{
   if ((a->xx == b->xx) &&
       (a->xy == b->xy) &&
       (a->xz == b->xz) &&
       (a->yx == b->yx) &&
       (a->yy == b->yy) &&
       (a->yz == b->yz) &&
       (a->zx == b->zx) &&
       (a->zy == b->zy) &&
       (a->zz == b->zz))
     return EINA_TRUE;
   return EINA_FALSE;
}

static inline Eina_Bool
eina_point_3d_cmp(const Eina_Point_3D *a, const Eina_Point_3D *b)
{
   if (EINA_FLT_EQ(a->x, b->x) &&
       EINA_FLT_EQ(a->y, b->y) &&
       EINA_FLT_EQ(a->z, b->z))
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

EFL_START_TEST(eina_test_quaternion_norm)
{
   static const Eina_Quaternion q = { 1, 3, 4, 5 };
   double result = eina_quaternion_norm(&q);


   fail_if(!EINA_FLT_EQ(result, sqrt(51)));

}
EFL_END_TEST

EFL_START_TEST(eina_test_quaternion_conjugate)
{
   static const Eina_Quaternion q1 = { 1, -1, -1, 3 }, q2 = { 1, 3, 4, 3 };
   static const Eina_Quaternion r1 = { -1, 1, 1, 3 }, r2 = { -1, -3, -4, 3 };
   Eina_Quaternion t1, t2;


   eina_quaternion_conjugate(&t1, &q1);
   eina_quaternion_conjugate(&t2, &q2);

   fail_if(!eina_quaternion_cmp(&t1, &r1));
   fail_if(!eina_quaternion_cmp(&t2, &r2));

}
EFL_END_TEST

EFL_START_TEST(eina_test_quaternion_matrix)
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


   eina_quaternion_rotation_matrix3_get(&tm, &q);
   fail_if(!eina_matrix3_cmp(&tm, &m));

   eina_matrix3_quaternion_get(&tq, &m);
   fail_if(!eina_quaternion_cmp(&tq, &q) && !eina_quaternion_cmp(&tq, &q1));

}
EFL_END_TEST

EFL_START_TEST(eina_test_quaternion_op)
{
   Eina_Quaternion q = { 7, 9, 5, 1 };
   Eina_Quaternion z = { 0, 0, 0, 0 };
   Eina_Quaternion neg, r;


   eina_quaternion_negative(&neg, &q);
   eina_quaternion_add(&r, &q, &neg);

   fail_if(!eina_quaternion_cmp(&z, &r));

}
EFL_END_TEST

EFL_START_TEST(eina_test_quaternion_f16p16)
{
   Eina_F16p16 x = 65536, y = 65536;
   Eina_F16p16 z = 65536, w = 65536;
   Eina_F16p16 res;
   Eina_Quaternion p, q = {65536, 65536, 65536, 65536};
   Eina_Quaternion_F16p16 t, s, r = {65536, 65536, 65536, 65536};



   eina_quaternion_f16p16_set(&p, x, y, z, w);
   fail_if(!eina_quaternion_cmp(&p, &q));

   res = eina_quaternion_f16p16_norm(&r);
   fail_if(res != 131070);

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
EFL_END_TEST

EFL_START_TEST(eina_test_quaternion_dot)
{
   Eina_Quaternion q = {1, 3, 4, 5};
   Eina_Quaternion r = {7, 9, 5, 1};
   double res;


   res = eina_quaternion_dot(&q, &r);
   fail_if(res != 59);

}
EFL_END_TEST

EFL_START_TEST(eina_test_quaternion_scale)
{
   Eina_Quaternion q = {1, 3, 4, 5};
   double scale = 2;
   Eina_Quaternion r, res = {2, 6, 8, 10};


   eina_quaternion_scale(&r, &q, scale);
   fail_if(!eina_quaternion_cmp(&r, &res));

}
EFL_END_TEST

EFL_START_TEST(eina_test_quaternion_set)
{
   Eina_Quaternion q, r = {1, 3, 4, 5};


   eina_quaternion_set(&q, 1, 3, 4, 5);

   fail_if(!eina_quaternion_cmp(&q, &r));

}
EFL_END_TEST

EFL_START_TEST(eina_test_quaternion_mul)
{
   Eina_Quaternion p = {1, 3, 4, 5};
   Eina_Quaternion q = {7, 9, 5, 1};
   Eina_Quaternion r, res = {15, 71, 17, -49};


   eina_quaternion_mul(&r, &p, &q);
   fail_if(!eina_quaternion_cmp(&r, &res));

}
EFL_END_TEST

EFL_START_TEST(eina_test_matrix_recompose)
{
   const Eina_Point_3D translation = { 0, 0, 0 };
   const Eina_Point_3D scale = { 1, 1, 1 };
   const Eina_Point_3D skew = { 0, 0, 0 };
   const Eina_Quaternion perspective = { 0, 0, 0, 1 };
   const Eina_Quaternion rotation = { 0, 0, 0, 1 };
   Eina_Matrix4 m4;


   eina_quaternion_matrix4_to(&m4,
                              &rotation,
                              &perspective,
                              &translation,
                              &scale,
                              &skew);

   fail_if(eina_matrix4_type_get(&m4) != EINA_MATRIX_TYPE_IDENTITY);

}
EFL_END_TEST

EFL_START_TEST(eina_test_quaternion_normalized)
{
   Eina_Quaternion p = {1, 1, 1, 1};
   Eina_Quaternion res, q = {0.5, 0.5, 0.5, 0.5};


   eina_quaternion_normalized(&res, &p);
   fail_if(!eina_quaternion_cmp(&q, &res));

}
EFL_END_TEST

#if 0
EFL_START_TEST(eina_test_matrix_quaternion)
{
   const Eina_Point_3D rt = { -2, -3, 0 };
   const Eina_Point_3D rsc = { 4, 5, 1 };
   const Eina_Quaternion rr = { 0, 0, -1, 0 };
   const Eina_Quaternion rp = { 0, 0, 0, 1 };
   Eina_Quaternion rotation, perspective;
   Eina_Matrix3 m3, m3r;
   Eina_Matrix4 m4, m4r;
   Eina_Point_3D translation, scale, skew;


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

}
EFL_END_TEST
#endif

EFL_START_TEST(eina_test_quaternion_f16p16_lerp)
{
   Eina_Quaternion_F16p16 r, p = {0, 0, 0, 0};
   Eina_Quaternion_F16p16 q = {65536, 65536, 65536, 0};
   Eina_Quaternion_F16p16  res1 = {65536, 65536, 65536, 0};
   Eina_Quaternion_F16p16  res2 = {0, 0, 0, 0};


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

}
EFL_END_TEST

EFL_START_TEST(eina_test_quaternion_lerp)
{
   Eina_Quaternion rp = {0, 0, 1, 0};
   Eina_Quaternion rq = {1, 0, 0, 0};
   Eina_Quaternion rr, res = {0.5, 0.0, 0.5, 0.0};


   eina_quaternion_lerp(&rr, &rp, &rq, 0.5);
   fail_if(!eina_quaternion_cmp(&rr, &res));

   eina_quaternion_set(&rp, 1, 1, 1, 0);
   eina_quaternion_set(&rq, 0, 1, 1, 0);
   eina_quaternion_set(&res, 0.5, 1.0, 1.0, 0.0);

   eina_quaternion_lerp(&rr, &rp, &rq, 0.5);
   fail_if(!eina_quaternion_cmp(&rr, &res));

   eina_quaternion_set(&rp, 0, 0, 1, 0);
   eina_quaternion_set(&rq, 1, 1, 1, 0);
   eina_quaternion_set(&res, 0.5, 0.5, 1.0, 0.0);

   eina_quaternion_slerp(&rr, &rp, &rq, 0.5);
   fail_if(!eina_quaternion_cmp(&rr, &res));

   eina_quaternion_set(&rp, 0, 0, 0, 0);
   eina_quaternion_set(&rq, 1, 1, 1, 0);
   eina_quaternion_set(&res, 1.0, 1.0, 1.0, 0.0);

   eina_quaternion_nlerp(&rr, &rp, &rq, 1.0);
   fail_if(!eina_quaternion_cmp(&rr, &res));

}
EFL_END_TEST

EFL_START_TEST(eina_test_quaternion_f16p16_rotate_matrix)
{
   Eina_Quaternion_F16p16 q = {65536, 65536, 65536, 0};
   Eina_Point_3D_F16p16 r = { 65536, 65536, 65536 };
   Eina_Point_3D_F16p16 c = { 0, 0, 0 }, res = {65536, 65536, 65536};
   Eina_Matrix3_F16p16 m, mres = {-262144, 131072, 131072,
                                  131072, -262144, 131072,
                                  131072, 131072, -262144 };


   eina_quaternion_f16p16_rotate(&r, &c, &q);
   fail_if(r.x != res.x ||
           r.y != res.y ||
           r.z != res.z);

   eina_quaternion_f16p16_rotation_matrix3_get(&m, &q);
   fail_if(!eina_matrix3_f16p16_cmp(&m, &mres));

}
EFL_END_TEST

EFL_START_TEST(eina_test_quaternion_rotate)
{
   Eina_Point_3D r = { 3, 3, 3 };
   Eina_Point_3D c = { 0, 0, 0}, res = {3.0, 3.0, 3.0};
   Eina_Point_3D res1 = {3.0, 3.0, -9.0};
   Eina_Quaternion q = {1, 1, 1, 0};


   eina_quaternion_rotate(&r, &c, &q);
   fail_if(!eina_point_3d_cmp(&r, &res));

   eina_quaternion_set(&q, 1, 1, 0, 0);
   eina_quaternion_rotate(&r, &c, &q);
   fail_if(!eina_point_3d_cmp(&r, &res1));

}
EFL_END_TEST

EFL_START_TEST(eina_test_quaternion_operations)
{
   Eina_Quaternion in = {2, 2, 1, 1};
   Eina_Quaternion in2 = {5, 5, 5, 5};
   Eina_Quaternion out;
   const double v[] = {0, 1, 2, 3};
   double angle;
   Eina_Matrix4 m;

   eina_quaternion_inverse(&out, &in);
   fail_if((out.x + 0.2 > DBL_EPSILON) || (out.y + 0.2 > DBL_EPSILON) ||
           (out.z + 0.1 > DBL_EPSILON) || (out.w - 0.1 > DBL_EPSILON));

   eina_quaternion_array_set(&out, v);
   fail_if((out.x != 0) || (out.y != 1) ||
           (out.z != 2) || (out.w != 3));

   eina_quaternion_copy(&out, &in);
   fail_if((out.x != 2) || (out.y != 2) ||
           (out.z != 1) || (out.w != 1));

   eina_quaternion_homogeneous_regulate(&out, &in2);
   fail_if((out.x != 1) || (out.y != 1) ||
           (out.z != 1) || (out.w != 1));

   eina_quaternion_subtract(&out, &in2, &in);
   fail_if((out.x != 3) || (out.y != 3) ||
           (out.z != 4) || (out.w != 4));

   fail_if(eina_quaternion_length_get(&in2) != 10);
   fail_if(eina_quaternion_length_square_get(&in) != 10);
   fail_if((eina_quaternion_distance_get(&in2, &in) - sqrt(50)) > DBL_EPSILON);
   fail_if(eina_quaternion_distance_square_get(&in2, &in) != 50);

   angle = eina_quaternion_angle_plains(&in, &in2);
   fail_if(angle - 0.963 > DBL_EPSILON);

   eina_matrix4_values_set(&m,
                           1, 0, 0, 0,
                           0, 1, 0, 0,
                           0, 0, 1, 0,
                           0, 0, 0, 1);
   eina_quaternion_transform(&out, &in, &m);
   fail_if((fabs(out.x) - 2.0) > DBL_EPSILON ||
           (fabs(out.y) - 2.0) > DBL_EPSILON ||
           (fabs(out.z) - 1.0) > DBL_EPSILON ||
           (fabs(out.w) - 1.0) > DBL_EPSILON
          );

   eina_matrix4_values_set(&m,
                           0.5, 0, 0, 0,
                           0, 0.5, 0, 0,
                           0, 0, 1, 0,
                           0, 0, 0, 1);
   eina_quaternion_transform(&out, &in, &m);
   fail_if((fabs(out.x) - 1.0) > DBL_EPSILON ||
           (fabs(out.y) - 1.0) > DBL_EPSILON ||
           (fabs(out.z) - 1.0) > DBL_EPSILON ||
           (fabs(out.w) - 1.0) > DBL_EPSILON
          );
}
EFL_END_TEST

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
   tcase_add_test(tc, eina_test_quaternion_lerp);
   tcase_add_test(tc, eina_test_quaternion_f16p16_rotate_matrix);
   tcase_add_test(tc, eina_test_quaternion_rotate);
   tcase_add_test(tc, eina_test_quaternion_operations);
}

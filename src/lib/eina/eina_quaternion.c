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

#include "eina_fp.h"
#include "eina_matrix.h"
#include "eina_quaternion.h"
#include "eina_util.h"

EINA_API void
eina_quaternion_f16p16_set(Eina_Quaternion *out,
                           Eina_F16p16 x, Eina_F16p16 y,
                           Eina_F16p16 z, Eina_F16p16 w)
{
   out->w = w;
   out->x = x;
   out->y = y;
   out->z = z;
}

EINA_API Eina_F16p16
eina_quaternion_f16p16_norm(const Eina_Quaternion_F16p16 *q)
{
   Eina_F16p16 s;

   s = eina_f16p16_add(eina_f16p16_add(eina_f16p16_mul(q->w, q->w),
                                       eina_f16p16_mul(q->x, q->x)),
                       eina_f16p16_add(eina_f16p16_mul(q->y, q->y),
                                       eina_f16p16_mul(q->z, q->z)));

   return eina_f16p16_sqrt(s);
}

EINA_API void
eina_quaternion_f16p16_negative(Eina_Quaternion_F16p16 *out,
                                const Eina_Quaternion_F16p16 *in)
{
   out->w = eina_f16p16_sub(0, in->w);
   out->x = eina_f16p16_sub(0, in->x);
   out->y = eina_f16p16_sub(0, in->y);
   out->z = eina_f16p16_sub(0, in->z);
}

EINA_API void
eina_quaternion_f16p16_add(Eina_Quaternion_F16p16 *out,
                           const Eina_Quaternion_F16p16 *a,
                           const Eina_Quaternion_F16p16 *b)
{
   out->w = eina_f16p16_add(a->w, b->w);
   out->x = eina_f16p16_add(a->x, b->x);
   out->y = eina_f16p16_add(a->y, b->y);
   out->z = eina_f16p16_add(a->z, b->z);
}

EINA_API void
eina_quaternion_f16p16_mul(Eina_Quaternion_F16p16 *out,
                           const Eina_Quaternion_F16p16 *a,
                           const Eina_Quaternion_F16p16 *b)
{
   out->w = eina_f16p16_sub(eina_f16p16_sub(eina_f16p16_mul(a->w, b->w),
                                            eina_f16p16_mul(a->x, b->x)),
                            eina_f16p16_sub(eina_f16p16_mul(a->y, b->y),
                                            eina_f16p16_mul(a->z, b->z)));
   out->x = eina_f16p16_add(eina_f16p16_add(eina_f16p16_mul(a->w, b->x),
                                            eina_f16p16_mul(a->x, b->w)),
                            eina_f16p16_sub(eina_f16p16_mul(a->y, b->z),
                                            eina_f16p16_mul(a->z, b->y)));
   out->y = eina_f16p16_add(eina_f16p16_sub(eina_f16p16_mul(a->w, b->y),
                                            eina_f16p16_mul(a->x, b->z)),
                            eina_f16p16_add(eina_f16p16_mul(a->y, b->w),
                                            eina_f16p16_mul(a->z, b->x)));
   out->z = eina_f16p16_add(eina_f16p16_add(eina_f16p16_mul(a->w, b->z),
                                            eina_f16p16_mul(a->x, b->y)),
                            eina_f16p16_sub(eina_f16p16_mul(a->z, b->w),
                                            eina_f16p16_mul(a->y, b->x)));
}

EINA_API void
eina_quaternion_f16p16_scale(Eina_Quaternion_F16p16 *out,
                             const Eina_Quaternion_F16p16 *a,
                             Eina_F16p16 b)
{
   out->w = eina_f16p16_scale(a->w, b);
   out->x = eina_f16p16_scale(a->x, b);
   out->y = eina_f16p16_scale(a->y, b);
   out->z = eina_f16p16_scale(a->z, b);
}

EINA_API void
eina_quaternion_f16p16_conjugate(Eina_Quaternion_F16p16 *out,
                                 const Eina_Quaternion_F16p16 *in)
{
   out->w = in->w;
   out->x = eina_f16p16_sub(0, in->x);
   out->y = eina_f16p16_sub(0, in->y);
   out->z = eina_f16p16_sub(0, in->z);
}

EINA_API Eina_F16p16
eina_quaternion_f16p16_dot(const Eina_Quaternion_F16p16 *a,
                           const Eina_Quaternion_F16p16 *b)
{
   return eina_f16p16_add(eina_f16p16_add(eina_f16p16_mul(a->w, b->w),
                                          eina_f16p16_mul(a->x, b->x)),
                          eina_f16p16_add(eina_f16p16_mul(a->y, b->y),
                                          eina_f16p16_mul(a->z, b->z)));
}

EINA_API void
eina_quaternion_f16p16_normalized(Eina_Quaternion_F16p16 *out,
                                  const Eina_Quaternion_F16p16 *in)
{
   Eina_F16p16 norm;

   norm = eina_quaternion_f16p16_norm(in);
   eina_quaternion_f16p16_scale(out, in,
                                eina_f16p16_div(eina_f16p16_int_from(1),
                                                norm));
}

EINA_API void
eina_quaternion_f16p16_lerp(Eina_Quaternion_F16p16 *out,
                            const Eina_Quaternion_F16p16 *a,
                            const Eina_Quaternion_F16p16 *b,
                            Eina_F16p16 pos)
{
   if (pos == 0)
     {
        *out = *a;
        return ;
     }
   else if (pos == eina_f16p16_int_from(1))
     {
        *out = *b;
        return ;
     }

   out->w = eina_f16p16_add(a->w,
                            eina_f16p16_mul(eina_f16p16_sub(b->w, a->w),
                                            pos));
   out->x = eina_f16p16_add(a->x,
                            eina_f16p16_mul(eina_f16p16_sub(b->x, a->x),
                                            pos));
   out->y = eina_f16p16_add(a->y,
                            eina_f16p16_mul(eina_f16p16_sub(b->y, a->y),
                                            pos));
   out->z = eina_f16p16_add(a->z,
                            eina_f16p16_mul(eina_f16p16_sub(b->z, a->z),
                                            pos));
}

EINA_API void
eina_quaternion_f16p16_slerp(Eina_Quaternion_F16p16 *out,
                             const Eina_Quaternion_F16p16 *a,
                             const Eina_Quaternion_F16p16 *b,
                             Eina_F16p16 pos)
{
   Eina_Quaternion_F16p16 bp;
   Eina_Quaternion_F16p16 left, right;
   Eina_F16p16 dot;
   Eina_F16p16 pos1, pos2;

   if (pos == 0)
     {
        *out = *a;
        return ;
     }
   else if (pos == eina_f16p16_int_from(1))
     {
        *out = *b;
        return ;
     }

   dot = eina_quaternion_f16p16_dot(a, b);
   if (dot >= eina_f16p16_int_from(0))
     {
        bp = *b;
     }
   else
     {
        eina_quaternion_f16p16_negative(&bp, b);
        dot = eina_f16p16_sub(0, dot);
     }

   pos1 = eina_f16p16_sub(eina_f16p16_int_from(1), pos);
   pos2 = pos;
   if (eina_f16p16_sub(eina_f16p16_int_from(1), dot) > eina_f16p16_float_from(0.0000001))
     {
        Eina_F32p32 angle;
        Eina_F16p16 sangle;

        angle = eina_f32p32_double_from(acos(eina_f16p16_double_to(dot)));
        sangle = eina_f32p32_to_f16p16(eina_f32p32_sin(angle));
        if (sangle > eina_f16p16_float_from(0.0000001))
          {
             pos1 = eina_f32p32_to_f16p16(eina_f32p32_div(eina_f32p32_sin(eina_f32p32_mul(eina_f16p16_to_f32p32(pos1), angle)),
                                                          eina_f16p16_to_f32p32(sangle)));
             pos2 = eina_f32p32_to_f16p16(eina_f32p32_div(eina_f32p32_sin(eina_f32p32_mul(eina_f16p16_to_f32p32(pos2), angle)),
                                                          eina_f16p16_to_f32p32(sangle)));
          }
     }

   eina_quaternion_f16p16_scale(&left, a, pos1);
   eina_quaternion_f16p16_scale(&right, b, pos2);
   eina_quaternion_f16p16_add(out, &left, &right);
}

EINA_API void
eina_quaternion_f16p16_nlerp(Eina_Quaternion_F16p16 *out,
                             const Eina_Quaternion_F16p16 *a,
                             const Eina_Quaternion_F16p16 *b,
                             Eina_F16p16 pos)
{
   Eina_Quaternion_F16p16 bp, left, right;
   Eina_Quaternion_F16p16 not_normalize;
   double dot;

   if (pos == 0)
     {
        *out = *a;
        return ;
     }
   else if (pos == eina_f16p16_int_from(1))
     {
        *out = *b;
        return ;
     }

   dot = eina_quaternion_f16p16_dot(a, b);
   if (dot >= eina_f16p16_int_from(0))
     {
        bp = *b;
     }
   else
     {
        eina_quaternion_f16p16_negative(&bp, b);
        dot = -dot;
     }

   eina_quaternion_f16p16_scale(&left, a,
                                eina_f16p16_sub(eina_f16p16_int_from(1), pos));
   eina_quaternion_f16p16_scale(&right, b, pos);
   eina_quaternion_f16p16_add(&not_normalize, &left, &right);
   eina_quaternion_f16p16_normalized(out, &not_normalize);
}

EINA_API void
eina_quaternion_f16p16_rotate(Eina_Point_3D_F16p16 *p,
                              const Eina_Point_3D_F16p16 *center,
                              const Eina_Quaternion_F16p16 *q)
{
   const Eina_Point_3D_F16p16 zero = { 0, 0, 0 };
   Eina_F16p16 x, y, z;
   Eina_F16p16 uvx, uvy, uvz;
   Eina_F16p16 uuvx, uuvy, uuvz;

   if (!center) center = &zero;

   x = eina_f16p16_sub(p->x, center->x);
   y = eina_f16p16_sub(p->y, center->y);
   z = eina_f16p16_sub(p->z, center->z);

   uvx = eina_f16p16_sub(eina_f16p16_mul(q->y, z),
                         eina_f16p16_mul(q->z, y));
   uvy = eina_f16p16_sub(eina_f16p16_mul(q->z, x),
                         eina_f16p16_mul(q->x, z));
   uvz = eina_f16p16_sub(eina_f16p16_mul(q->x, y),
                         eina_f16p16_mul(q->y, x));

   uuvx = eina_f16p16_sub(eina_f16p16_mul(q->y, uvz),
                          eina_f16p16_mul(q->z, uvy));
   uuvy = eina_f16p16_sub(eina_f16p16_mul(q->z, uvx),
                          eina_f16p16_mul(q->x, uvz));
   uuvz = eina_f16p16_sub(eina_f16p16_mul(q->x, uvy),
                          eina_f16p16_mul(q->y, uvx));

   uvx = eina_f16p16_mul(eina_f16p16_scale(q->w, 2), uvx);
   uvy = eina_f16p16_mul(eina_f16p16_scale(q->w, 2), uvy);
   uvz = eina_f16p16_mul(eina_f16p16_scale(q->w, 2), uvz);

   uuvx = eina_f16p16_scale(uuvx, 2);
   uuvy = eina_f16p16_scale(uuvy, 2);
   uuvz = eina_f16p16_scale(uuvz, 2);

   p->x = eina_f16p16_add(eina_f16p16_add(center->x, x),
                          eina_f16p16_add(uvx, uuvx));
   p->y = eina_f16p16_add(eina_f16p16_add(center->y, y),
                          eina_f16p16_add(uvy, uuvy));
   p->z = eina_f16p16_add(eina_f16p16_add(center->z, z),
                          eina_f16p16_add(uvz, uuvz));
}

EINA_API void
eina_quaternion_f16p16_rotation_matrix3_get(Eina_Matrix3_F16p16 *m,
                                            const Eina_Quaternion_F16p16 *q)
{
   Eina_F16p16 x, y, z;
   Eina_F16p16 xx, xy, xz;
   Eina_F16p16 yy, yz;
   Eina_F16p16 zz;
   Eina_F16p16 wx, wy, wz;

   x = eina_f16p16_scale(q->x, 2);
   y = eina_f16p16_scale(q->y, 2);
   z = eina_f16p16_scale(q->z, 2);

   xx = eina_f16p16_mul(q->x, x);
   xy = eina_f16p16_mul(q->x, y);
   xz = eina_f16p16_mul(q->x, z);
   yy = eina_f16p16_mul(q->y, y);
   yz = eina_f16p16_mul(q->y, z);
   zz = eina_f16p16_mul(q->z, z);
   wx = eina_f16p16_mul(q->w, x);
   wy = eina_f16p16_mul(q->w, y);
   wz = eina_f16p16_mul(q->w, z);

   m->xx = eina_f16p16_sub(eina_f16p16_int_to(1),
                           eina_f16p16_add(yy, zz));
   m->xy = eina_f16p16_add(xy, wz);
   m->xz = eina_f16p16_sub(xz, wy);
   m->yx = eina_f16p16_sub(xy, wz);
   m->yy = eina_f16p16_sub(eina_f16p16_int_to(1),
                           eina_f16p16_add(xx, zz));
   m->yz = eina_f16p16_add(yz, wx);
   m->zx = eina_f16p16_add(xz, wy);
   m->zy = eina_f16p16_add(yz, wx);
   m->zz = eina_f16p16_sub(eina_f16p16_int_to(1),
                           eina_f16p16_add(xx, yy));
}

EINA_API void
eina_quaternion_set(Eina_Quaternion *out, double x,
                    double y, double z, double w)
{
   out->w = w;
   out->x = x;
   out->y = y;
   out->z = z;
}

EINA_API double
eina_quaternion_norm(const Eina_Quaternion *q)
{
   double s;

   s = q->w * q->w + q->x * q->x + q->y * q->y + q->z * q->z;

   return sqrt(s);
}

EINA_API void
eina_quaternion_negative(Eina_Quaternion *out,
                         const Eina_Quaternion *in)
{
   out->w = - in->w;
   out->x = - in->x;
   out->y = - in->y;
   out->z = - in->z;
}

EINA_API void
eina_quaternion_add(Eina_Quaternion *out,
                    const Eina_Quaternion *a,
                    const Eina_Quaternion *b)
{
   out->w = a->w + b->w;
   out->x = a->x + b->x;
   out->y = a->y + b->y;
   out->z = a->z + b->z;
}

EINA_API void
eina_quaternion_mul(Eina_Quaternion *out,
                    const Eina_Quaternion *a,
                    const Eina_Quaternion *b)
{
   out->w = a->w * b->w - a->x * b->x - a->y * b->y - a->z * b->z;
   out->x = a->w * b->x + a->x * b->w + a->y * b->z - a->z * b->y;
   out->y = a->w * b->y - a->x * b->z + a->y * b->w + a->z * b->x;
   out->z = a->w * b->z + a->x * b->y - a->y * b->x + a->z * b->w;
}

EINA_API void
eina_quaternion_scale(Eina_Quaternion *out,
                      const Eina_Quaternion *a,
                      double b)
{
   out->w = a->w * b;
   out->x = a->x * b;
   out->y = a->y * b;
   out->z = a->z * b;
}

EINA_API void
eina_quaternion_conjugate(Eina_Quaternion *out,
                          const Eina_Quaternion *in)
{
   out->w = in->w;
   out->x = - in->x;
   out->y = - in->y;
   out->z = - in->z;
}

EINA_API double
eina_quaternion_dot(const Eina_Quaternion *a,
                    const Eina_Quaternion *b)
{
   return a->w * b->w + a->x * b->x + a->y * b->y + a->z * b->z;
}

EINA_API void
eina_quaternion_normalized(Eina_Quaternion *out,
                           const Eina_Quaternion *in)
{
   double norm;

   norm = eina_quaternion_norm(in);
   eina_quaternion_scale(out, in, 1.0 / norm);
}

EINA_API void
eina_quaternion_lerp(Eina_Quaternion *out,
                     const Eina_Quaternion *a,
                     const Eina_Quaternion *b,
                     double pos)
{
   if (EINA_DBL_EQ(pos, 0.0))
     {
        *out = *a;
        return ;
     }
   else if (EINA_DBL_EQ(pos, 1.0))
     {
        *out = *b;
        return ;
     }

   out->w = a->w + pos * (b->w - a->w);
   out->x = a->x + pos * (b->x - a->x);
   out->y = a->y + pos * (b->y - a->y);
   out->z = a->z + pos * (b->z - a->z);
}

EINA_API void
eina_quaternion_slerp(Eina_Quaternion *out,
                      const Eina_Quaternion *a,
                      const Eina_Quaternion *b,
                      double pos)
{
   Eina_Quaternion bp;
   Eina_Quaternion left, right;
   double dot;
   double pos1, pos2;

   if (EINA_DBL_EQ(pos, 0.0))
     {
        *out = *a;
        return ;
     }
   else if (EINA_DBL_EQ(pos, 1.0))
     {
        *out = *b;
        return ;
     }

   dot = eina_quaternion_dot(a, b);
   if (dot >= 0)
     {
        bp = *b;
     }
   else
     {
        eina_quaternion_negative(&bp, b);
        dot = -dot;
     }

   pos1 = 1.0 - pos;
   pos2 = pos;
   if ((1.0 - dot) > 0.0000001)
     {
        double angle;
        double sangle;

        angle = acos(dot);
        sangle = sin(angle);
        if (sangle > 0.0000001)
          {
             pos1 = sin(pos1 * angle) / sangle;
             pos2 = sin(pos2 * angle) / sangle;
          }
     }

   eina_quaternion_scale(&left, a, pos1);
   eina_quaternion_scale(&right, b, pos2);
   eina_quaternion_add(out, &left, &right);
}

EINA_API void
eina_quaternion_nlerp(Eina_Quaternion *out,
                      const Eina_Quaternion *a,
                      const Eina_Quaternion *b,
                      double pos)
{
   Eina_Quaternion bp, left, right;
   Eina_Quaternion not_normalize;
   double dot;

   if (EINA_DBL_EQ(pos, 0.0))
     {
        *out = *a;
        return ;
     }
   else if (EINA_DBL_EQ(pos, 1.0))
     {
        *out = *b;
        return ;
     }

   dot = eina_quaternion_dot(a, b);
   if (dot >= 0)
     {
        bp = *b;
     }
   else
     {
        eina_quaternion_negative(&bp, b);
        dot = -dot;
     }

   eina_quaternion_scale(&left, a, 1.0 - pos);
   eina_quaternion_scale(&right, b, pos);
   eina_quaternion_add(&not_normalize, &left, &right);
   eina_quaternion_normalized(out, &not_normalize);
}

EINA_API void
eina_quaternion_rotate(Eina_Point_3D *p,
                       const Eina_Point_3D *center,
                       const Eina_Quaternion *q)
{
   double x, y, z;
   double uvx, uvy, uvz;
   double uuvx, uuvy, uuvz;

   x = p->x - center->x;
   y = p->y - center->y;
   z = p->z - center->z;

   uvx = q->y * z - q->z * y;
   uvy = q->z * x - q->x * z;
   uvz = q->x * y - q->y * x;

   uuvx = q->y * uvz - q->z * uvy;
   uuvy = q->z * uvx - q->x * uvz;
   uuvz = q->x * uvy - q->y * uvx;

   uvx *= (2.0f * q->w);
   uvy *= (2.0f * q->w);
   uvz *= (2.0f * q->w);

   uuvx *= 2.0f;
   uuvy *= 2.0f;
   uuvz *= 2.0f;

   p->x = center->x + x + uvx + uuvx;
   p->y = center->y + y + uvy + uuvy;
   p->z = center->z + z + uvz + uuvz;
}

EINA_API void
eina_quaternion_rotation_matrix3_get(Eina_Matrix3 *m,
                                     const Eina_Quaternion *q)
{
   double x, y, z;
   double xx, xy, xz;
   double yy, yz;
   double zz;
   double wx, wy, wz;

   x = 2.0 * q->x;
   y = 2.0 * q->y;
   z = 2.0 * q->z;

   xx = q->x * x;
   xy = q->x * y;
   xz = q->x * z;
   yy = q->y * y;
   yz = q->y * z;
   zz = q->z * z;
   wx = q->w * x;
   wy = q->w * y;
   wz = q->w * z;

   m->xx = 1.0 - yy - zz;
   m->xy = xy + wz;
   m->xz = xz - wy;
   m->yx = xy - wz;
   m->yy = 1.0 - xx - zz;
   m->yz = yz + wx;
   m->zx = xz + wy;
   m->zy = yz - wx;
   m->zz = 1.0 - xx - yy;
}

static inline double
_max(double a, double b)
{
   return a > b ? a : b;
}

static inline double
eina_point_3d_norm(Eina_Point_3D *p)
{
   return sqrt(p->x * p->x + p->y * p->y + p->z * p->z);
}

static inline void
eina_point_3d_normalize(Eina_Point_3D *p, double norm)
{
   double tmp = 1 / norm;

   p->x *= tmp;
   p->y *= tmp;
   p->z *= tmp;
}

static inline double
eina_point_3d_dot(const Eina_Point_3D *a, const Eina_Point_3D *b)
{
   return a->x * b->x + a->y * b->y + a->z * b->z;
}

static inline void
eina_point_3d_combine(Eina_Point_3D *out,
                      const Eina_Point_3D *a, const Eina_Point_3D *b,
                      double scale1, double scale2)
{
   out->x = a->x * scale1 + b->x * scale2;
   out->y = a->y * scale1 + b->y * scale2;
   out->z = a->z * scale1 + b->z * scale2;
}

static inline void
eina_point3d_cross(Eina_Point_3D *out,
                   const Eina_Point_3D *a, const Eina_Point_3D *b)
{
   out->x = a->y * b->z - a->z * b->y;
   out->y = a->z * b->x - a->x * b->z;
   out->z = a->x * b->y - a->y * b->x;
}

static inline void
eina_point3d_neg(Eina_Point_3D *out, const Eina_Point_3D *in)
{
   out->x = - in->x;
   out->y = - in->y;
   out->z = - in->z;
}

/* http://www.w3.org/TR/css3-transforms/#decomposing-a-3d-matrix */
EINA_API Eina_Bool
eina_matrix4_quaternion_to(Eina_Quaternion *rotation,
                           Eina_Quaternion *perspective,
                           Eina_Point_3D *translation,
                           Eina_Point_3D *scale,
                           Eina_Point_3D *skew,
                           const Eina_Matrix4 *m)
{
   Eina_Matrix4 n, pm;
   double det, factor;

   if (EINA_DBL_EQ(m->ww, 0.0)) return EINA_FALSE;

   // Normalize the matrix.
   factor = 1 / m->ww;

   n.xx = m->xx * factor;
   n.xy = m->xy * factor;
   n.xz = m->xz * factor;
   n.xw = m->xw * factor;
   n.yx = m->yx * factor;
   n.yy = m->yy * factor;
   n.yz = m->yz * factor;
   n.yw = m->yw * factor;
   n.zx = m->zx * factor;
   n.zy = m->zy * factor;
   n.zz = m->zz * factor;
   n.zw = m->zw * factor;
   n.wx = m->wx * factor;
   n.wy = m->wy * factor;
   n.wz = m->wz * factor;
   n.ww = m->ww * factor;

   pm = n;
   pm.xw = 0;
   pm.yw = 0;
   pm.zw = 0;
   pm.ww = 1;

   // If the perspective matrix is not invertible, we are also unable to
   // decompose, so we'll bail early.
   det = eina_matrix4_determinant(&pm);
   if (fabs(det) < DBL_EPSILON) return EINA_FALSE;

   // First, isolate perspective.
   if (perspective)
     {
        if (fabs(n.xw) < DBL_EPSILON ||
            fabs(n.yw) < DBL_EPSILON ||
            fabs(n.zw) < DBL_EPSILON)
          {
             Eina_Quaternion tmp;
             Eina_Matrix4 ipm, tipm;

             tmp.x = n.wx;
             tmp.y = n.wy;
             tmp.z = n.wz;
             tmp.w = n.ww;

             if (!eina_matrix4_inverse(&ipm, &pm))
               return EINA_FALSE;

             eina_matrix4_transpose(&tipm, &ipm);

             perspective->x = tipm.xx * tmp.x + tipm.yx * tmp.y + tipm.zx * tmp.z + tipm.wx * tmp.w;
             perspective->y = tipm.xy * tmp.x + tipm.yy * tmp.y + tipm.zy * tmp.z + tipm.wy * tmp.w;
             perspective->z = tipm.xz * tmp.x + tipm.yz * tmp.y + tipm.zz * tmp.z + tipm.wz * tmp.w;
             perspective->w = tipm.xw * tmp.x + tipm.yw * tmp.y + tipm.zw * tmp.z + tipm.ww * tmp.w;
          }
        else
          {
             perspective->x = perspective->y = perspective->z = 0;
             perspective->w = 1;
          }
     }

   if (translation)
     {
        translation->x = n.xw;
        translation->y = n.yw;
        translation->z = n.zw;
     }

   if (skew || scale || rotation)
     {
        Eina_Point_3D tsc, tsk, row0, row1, row2, cross;
        double tmp;

        // Make sure all pointer are defined
        if (!scale) scale = &tsc;
        if (!skew) skew = &tsk;

        row0.x = n.xx; row1.x = n.yx; row2.x = n.zx;
        row0.y = n.xy; row1.y = n.yy; row2.y = n.zy;
        row0.z = n.xz; row1.z = n.yz; row2.z = n.zz;

        // Compute X scale factor and normalize first row.
        scale->x = eina_point_3d_norm(&row0);
        eina_point_3d_normalize(&row0, scale->x);

        skew->x = eina_point_3d_dot(&row0, &row1);
        eina_point_3d_combine(&row1, &row1, &row0, 1.0, -skew->x);

        // Now, compute Y scale and normalize 2nd row.
        scale->y = eina_point_3d_norm(&row1);
        eina_point_3d_normalize(&row1, scale->y);
        skew->x /= scale->y;

        // Compute XZ and YZ shears, orthogonalize 3rd row
        skew->y = eina_point_3d_dot(&row0, &row2);
        eina_point_3d_combine(&row2, &row2, &row0, 1.0, -skew->y);
        skew->z = eina_point_3d_dot(&row1, &row2);
        eina_point_3d_combine(&row2, &row2, &row1, 1.0, -skew->z);

        // Next, get Z scale and normalize 3rd row.
        scale->z = eina_point_3d_norm(&row2);
        eina_point_3d_normalize(&row2, scale->z);

        tmp = 1 / scale->z;
        skew->y *= tmp;
        skew->z *= tmp;

        // At this point, the matrix (in rows) is orthonormal.
        // Check for a coordinate system flip.  If the determinant
        // is -1, then negate the matrix and the scaling factors.
        eina_point3d_cross(&cross, &row1, &row2);
        if (eina_point_3d_dot(&row0, &cross) < 0)
          {
             eina_point3d_neg(scale, scale);
             eina_point3d_neg(&row0, &row0);
             eina_point3d_neg(&row1, &row1);
             eina_point3d_neg(&row2, &row2);
          }

        if (rotation)
          {
             // Now, get the rotations out
             rotation->x = 0.5 * sqrt(_max(1 + row0.x - row1.y - row2.z, 0));
             rotation->y = 0.5 * sqrt(_max(1 - row0.x + row1.y - row2.z, 0));
             rotation->z = 0.5 * sqrt(_max(1 - row0.x - row1.y + row2.z, 0));
             rotation->w = 0.5 * sqrt(_max(1 + row0.x + row1.y + row2.z, 0));

             if (row2.y > row1.z) rotation->x = - rotation->x;
             if (row0.z > row2.x) rotation->y = - rotation->y;
             if (row1.x > row0.y) rotation->z = - rotation->z;
          }
     }

   return EINA_TRUE;
}

EINA_API void
eina_matrix3_quaternion_get(Eina_Quaternion *q,
                            const Eina_Matrix3 *m)
{
   double tval;
   double w, x, y, z;

   tval = m->xx + m->yy + m->zz;

   if (tval > 0)
     {

        double s = 0.5 / sqrtf(tval + 1.0);

        w = (0.25 / s);
        x = ((m->zy - m->yz) * s);
        y = ((m->xz - m->zx) * s);
        z = ((m->yx - m->xy) * s);
     }
   else if ((m->xx > m->yy) && (m->xx > m->zz))
     {
        double s = 2.0 * sqrtf(1.0 + m->xx - m->yy - m->zz);

        w = ((m->zy - m->yz) / s);
        x = (0.25 * s);
        y = ((m->xy + m->yx) / s);
        z = ((m->xz + m->zx) / s);
     }
   else if (m->yy > m->zz)
     {
        double s = 2.0 * sqrtf(1.0 + m->yy - m->xx - m->zz);

        w = ((m->xz - m->zx) / s);
        x = ((m->xy + m->yx) / s);
        y = (0.25 * s);
        z = ((m->yz + m->zy) / s);
     }
   else
     {
        double s = 2.0 * sqrtf(1.0 + m->zz - m->xx - m->yy);

        w = ((m->yx - m->xy) / s);
        x = ((m->xz + m->zx) / s);
        y = ((m->yz + m->zy) / s);
        z = (0.25 * s);
     }
   q->w = w;
   q->x = x;
   q->y = y;
   q->z = z;
}

EINA_API void
eina_quaternion_matrix4_to(Eina_Matrix4 *m,
                           const Eina_Quaternion *rotation,
                           const Eina_Quaternion *perspective,
                           const Eina_Point_3D *translation,
                           const Eina_Point_3D *scale,
                           const Eina_Point_3D *skew)
{
   Eina_Matrix4 rm, tmp;
   double x, y, z, w;

   eina_matrix4_identity(m);

   // apply perspective
   m->wx = perspective->x;
   m->wy = perspective->y;
   m->wz = perspective->z;
   m->ww = perspective->w;

   // apply translation
   m->xw = translation->x * m->xx + translation->y * m->xy + translation->z * m->xz;
   m->yw = translation->x * m->yx + translation->y * m->yy + translation->z * m->yz;
   m->zw = translation->x * m->zx + translation->y * m->zy + translation->z * m->zz;

   // apply rotation
   x = rotation->x;
   y = rotation->y;
   z = rotation->z;
   w = rotation->w;

   // Construct a composite rotation matrix from the quaternion values
   // rotationMatrix is a identity 4x4 matrix initially
   eina_matrix4_identity(&rm);

   rm.xx = 1 - 2 * (y * y + z * z);
   rm.xy = 2 * (x * y - z * w);
   rm.xz = 2 * (x * z + y * w);
   rm.yx = 2 * (x * y + z * w);
   rm.yy = 1 - 2 * (x * x + z * z);
   rm.yz = 2 * (y * z - x * w);
   rm.zx = 2 * (x * z - y * w);
   rm.zy = 2 * (y * z + x * w);
   rm.zz = 1 - 2 * (x * x + y * y);

   eina_matrix4_multiply(&tmp, m, &rm);

   // apply skew
   // rm is a identity 4x4 matrix initially
   if (!EINA_DBL_EQ(skew->z, 0.0))
     {
        Eina_Matrix4 cp;

        eina_matrix4_identity(&rm);
        rm.zx = skew->z;

        eina_matrix4_multiply(&cp, &tmp, &rm);
        tmp = cp;
     }

   if (!EINA_DBL_EQ(skew->y, 0.0))
     {
        Eina_Matrix4 cp;

        eina_matrix4_identity(&rm);
        rm.zy = 0;
        rm.zx = skew->y;

        eina_matrix4_multiply(&cp, &tmp, &rm);
        tmp = cp;
     }

   if (!EINA_DBL_EQ(skew->x, 0.0))
     {
        Eina_Matrix4 cp;

        eina_matrix4_identity(&rm);
        rm.zx = 0;
        rm.yx = skew->x;

        eina_matrix4_multiply(&cp, &tmp, &rm);
        tmp = cp;
     }

   // apply scale
   m->xx = tmp.xx * scale->x;
   m->xy = tmp.xy * scale->x;
   m->xz = tmp.xz * scale->x;
   m->xw = tmp.xw;
   m->yx = tmp.yx * scale->y;
   m->yy = tmp.yy * scale->y;
   m->yz = tmp.yz * scale->y;
   m->yw = tmp.yw;
   m->zx = tmp.zx * scale->z;
   m->zy = tmp.zy * scale->z;
   m->zz = tmp.zz * scale->z;
   m->zw = tmp.zw;
   m->wx = tmp.wx;
   m->wy = tmp.wy;
   m->wz = tmp.wz;
   m->ww = tmp.ww;
}

EINA_API void
eina_quaternion_inverse(Eina_Quaternion *out, const Eina_Quaternion *q)
{
   double norm = (q->x * q->x) + (q->y * q->y) + (q->z * q->z) + (q->w * q->w);

   if (norm > 0.0)
     {
        double inv_norm = 1.0 / norm;
        out->x = -q->x * inv_norm;
        out->y = -q->y * inv_norm;
        out->z = -q->z * inv_norm;
        out->w =  q->w * inv_norm;
     }
   else
     {
        out->x = 0.0;
        out->y = 0.0;
        out->z = 0.0;
        out->w = 0.0;
     }
}

EINA_API void
eina_quaternion_array_set(Eina_Quaternion *dst, const double *v)
{
   dst->x = v[0];
   dst->y = v[1];
   dst->z = v[2];
   dst->w = v[3];
}

EINA_API void
eina_quaternion_copy(Eina_Quaternion *dst, const Eina_Quaternion *src)
{
   dst->x = src->x;
   dst->y = src->y;
   dst->z = src->z;
   dst->w = src->w;
}

EINA_API void
eina_quaternion_homogeneous_regulate(Eina_Quaternion *out, const Eina_Quaternion *v)
{
   if (!EINA_DBL_EQ(v->w, 0.0))
     {
        double scale = 1.0 / v->w;

        out->x = v->x * scale;
        out->y = v->y * scale;
        out->z = v->z * scale;
        out->w = 1.0;
     }
}

EINA_API void
eina_quaternion_subtract(Eina_Quaternion *out, const Eina_Quaternion *a, const Eina_Quaternion *b)
{
   out->x = a->x - b->x;
   out->y = a->y - b->y;
   out->z = a->z - b->z;
   out->w = a->w - b->w;
}

EINA_API double
eina_quaternion_length_get(const Eina_Quaternion *v)
{
   return (double)sqrt((double)((v->x * v->x) + (v->y * v->y) +
                                (v->z * v->z) + (v->w * v->w)));
}

EINA_API double
eina_quaternion_length_square_get(const Eina_Quaternion *v)
{
   return (v->x * v->x) + (v->y * v->y) + (v->z * v->z) + (v->w * v->w);
}

EINA_API double
eina_quaternion_distance_get(const Eina_Quaternion *a, const Eina_Quaternion *b)
{
   Eina_Quaternion v;

   eina_quaternion_subtract(&v, a, b);
   return eina_quaternion_length_get(&v);
}

EINA_API double
eina_quaternion_distance_square_get(const Eina_Quaternion *a, const Eina_Quaternion *b)
{
   Eina_Quaternion v;

   eina_quaternion_subtract(&v, a, b);
   return eina_quaternion_length_square_get(&v);
}

EINA_API void
eina_quaternion_transform(Eina_Quaternion *out, const Eina_Quaternion *v, const Eina_Matrix4 *m)
{
   Eina_Quaternion tmp;

   if (eina_matrix4_type_get(m) == EINA_MATRIX_TYPE_IDENTITY)
     {
        eina_quaternion_copy(out, v);
        return;
     }

   tmp.x = (m->xx * v->x) + (m->yx * v->y) + (m->zx * v->z) + (m->wx * v->w);
   tmp.y = (m->xy * v->x) + (m->yy * v->y) + (m->zy * v->z) + (m->wy * v->w);
   tmp.z = (m->xz * v->x) + (m->yz * v->y) + (m->zz * v->z) + (m->wz * v->w);
   tmp.w = (m->xw * v->x) + (m->yw * v->y) + (m->zw * v->z) + (m->ww * v->w);

   eina_quaternion_copy(out, &tmp);
}

EINA_API double
eina_quaternion_angle_plains(Eina_Quaternion *a, Eina_Quaternion *b)
{
   return (double) ((a->x * b->x) + (a->y * b->y) + (a->z * b->z)) / ((sqrt((a->x * a->x) +
                    (a->y * a->y) + (a->z * a->z))) * (sqrt((b->x * b->x) + (b->y * b->y) +
                    (b->z * b->z))));
}

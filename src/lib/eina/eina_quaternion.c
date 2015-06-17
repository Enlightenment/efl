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
#include "eina_matrix.h"
#include "eina_quaternion.h"

EAPI void
eina_quaternion_f16p16_set(Eina_Quaternion *out,
                           Eina_F16p16 x, Eina_F16p16 y,
                           Eina_F16p16 z, Eina_F16p16 w)
{
   out->w = w;
   out->x = x;
   out->y = y;
   out->z = z;
}

EAPI Eina_F16p16
eina_quaternion_f16p16_norm(const Eina_Quaternion_F16p16 *q)
{
   Eina_F16p16 s;

   s = eina_f16p16_add(eina_f16p16_add(eina_f16p16_mul(q->w, q->w),
                                       eina_f16p16_mul(q->x, q->x)),
                       eina_f16p16_add(eina_f16p16_mul(q->y, q->y),
                                       eina_f16p16_mul(q->z, q->z)));

   return eina_f16p16_sqrt(s);
}

EAPI void
eina_quaternion_f16p16_negative(Eina_Quaternion_F16p16 *out,
                                const Eina_Quaternion_F16p16 *in)
{
   out->w = eina_f16p16_sub(0, in->w);
   out->x = eina_f16p16_sub(0, in->x);
   out->y = eina_f16p16_sub(0, in->y);
   out->z = eina_f16p16_sub(0, in->z);
}

EAPI void
eina_quaternion_f16p16_add(Eina_Quaternion_F16p16 *out,
                           const Eina_Quaternion_F16p16 *a,
                           const Eina_Quaternion_F16p16 *b)
{
   out->w = eina_f16p16_add(a->w, b->w);
   out->x = eina_f16p16_add(a->x, b->x);
   out->y = eina_f16p16_add(a->y, b->y);
   out->z = eina_f16p16_add(a->z, b->z);
}

EAPI void
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

EAPI void
eina_quaternion_f16p16_scale(Eina_Quaternion_F16p16 *out,
                             const Eina_Quaternion_F16p16 *a,
                             Eina_F16p16 b)
{
   out->w = eina_f16p16_scale(a->w, b);
   out->x = eina_f16p16_scale(a->x, b);
   out->y = eina_f16p16_scale(a->y, b);
   out->z = eina_f16p16_scale(a->z, b);
}

EAPI void
eina_quaternion_f16p16_conjugate(Eina_Quaternion_F16p16 *out,
                                 const Eina_Quaternion_F16p16 *in)
{
   out->w = in->w;
   out->x = eina_f16p16_sub(0, in->x);
   out->y = eina_f16p16_sub(0, in->y);
   out->z = eina_f16p16_sub(0, in->z);
}

EAPI Eina_F16p16
eina_quaternion_f16p16_dot(const Eina_Quaternion_F16p16 *a,
                           const Eina_Quaternion_F16p16 *b)
{
   return eina_f16p16_add(eina_f16p16_add(eina_f16p16_mul(a->w, b->w),
                                          eina_f16p16_mul(a->x, b->x)),
                          eina_f16p16_add(eina_f16p16_mul(a->y, b->y),
                                          eina_f16p16_mul(a->z, b->z)));
}

EAPI void
eina_quaternion_f16p16_normalized(Eina_Quaternion_F16p16 *out,
                                  const Eina_Quaternion_F16p16 *in)
{
   Eina_F16p16 norm;

   norm = eina_quaternion_f16p16_norm(in);
   eina_quaternion_f16p16_scale(out, in,
                                eina_f16p16_div(eina_f16p16_int_from(1),
                                                norm));
}

EAPI void
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

EAPI void
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

EAPI void
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

EAPI void
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

EAPI void
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

EAPI void
eina_quaternion_set(Eina_Quaternion *out, double x,
                    double y, double z, double w)
{
   out->w = w;
   out->x = x;
   out->y = y;
   out->z = z;
}

EAPI double
eina_quaternion_norm(const Eina_Quaternion *q)
{
   double s;

   s = q->w * q->w + q->x * q->x + q->y * q->y + q->z * q->z;

   return sqrt(s);
}

EAPI void
eina_quaternion_negative(Eina_Quaternion *out,
                         const Eina_Quaternion *in)
{
   out->w = - in->w;
   out->x = - in->x;
   out->y = - in->y;
   out->z = - in->z;
}

EAPI void
eina_quaternion_add(Eina_Quaternion *out,
                    const Eina_Quaternion *a,
                    const Eina_Quaternion *b)
{
   out->w = a->w + b->w;
   out->x = a->x + b->x;
   out->y = a->y + b->y;
   out->z = a->z + b->z;
}

EAPI void
eina_quaternion_mul(Eina_Quaternion *out,
                    const Eina_Quaternion *a,
                    const Eina_Quaternion *b)
{
   out->w = a->w * b->w - a->x * b->x - a->y * b->y - a->z * b->z;
   out->x = a->w * b->x + a->x * b->w + a->y * b->z - a->z * b->y;
   out->y = a->w * b->y - a->x * b->z + a->y * b->w + a->z * b->x;
   out->z = a->w * b->z + a->x * b->y - a->y * b->x + a->z * b->w;
}

EAPI void
eina_quaternion_scale(Eina_Quaternion *out,
                      const Eina_Quaternion *a,
                      double b)
{
   out->w = a->w * b;
   out->x = a->x * b;
   out->y = a->y * b;
   out->z = a->z * b;
}

EAPI void
eina_quaternion_conjugate(Eina_Quaternion *out,
                          const Eina_Quaternion *in)
{
   out->w = in->w;
   out->x = - in->x;
   out->y = - in->y;
   out->z = - in->z;
}

EAPI double
eina_quaternion_dot(const Eina_Quaternion *a,
                    const Eina_Quaternion *b)
{
   return a->w * b->w + a->x * b->x + a->y * b->y + a->z * b->z;
}

EAPI void
eina_quaternion_normalized(Eina_Quaternion *out,
                           const Eina_Quaternion *in)
{
   double norm;

   norm = eina_quaternion_norm(in);
   eina_quaternion_scale(out, in, 1.0 / norm);
}

EAPI void
eina_quaternion_lerp(Eina_Quaternion *out,
                     const Eina_Quaternion *a,
                     const Eina_Quaternion *b,
                     double pos)
{
   if (pos == 0)
     {
        *out = *a;
        return ;
     }
   else if (pos == 1.0)
     {
        *out = *b;
        return ;
     }

   out->w = a->w + pos * (b->w - a->w);
   out->x = a->x + pos * (b->x - a->x);
   out->y = a->y + pos * (b->y - a->y);
   out->z = a->z + pos * (b->z - a->z);
}

EAPI void
eina_quaternion_slerp(Eina_Quaternion *out,
                      const Eina_Quaternion *a,
                      const Eina_Quaternion *b,
                      double pos)
{
   Eina_Quaternion bp;
   Eina_Quaternion left, right;
   double dot;
   double pos1, pos2;

   if (pos == 0)
     {
        *out = *a;
        return ;
     }
   else if (pos == 1.0)
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

EAPI void
eina_quaternion_nlerp(Eina_Quaternion *out,
                      const Eina_Quaternion *a,
                      const Eina_Quaternion *b,
                      double pos)
{
   Eina_Quaternion bp, left, right;
   Eina_Quaternion not_normalize;
   double dot;

   if (pos == 0)
     {
        *out = *a;
        return ;
     }
   else if (pos == 1.0)
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

EAPI void
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

EAPI void
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

EAPI void
eina_matrix3_quaternion_get(Eina_Quaternion *q EINA_UNUSED,
                            const Eina_Matrix3 *m EINA_UNUSED)
{
}

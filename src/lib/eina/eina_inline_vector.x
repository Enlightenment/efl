/* EINA - EFL data type library
 * Copyright (C) 2016 Cedric Bail
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

#ifndef EINA_INLINE_VECTOR_X_
#define EINA_INLINE_VECTOR_X_

static inline void
eina_vector2_set(Eina_Vector2 *dst, double x, double y)
{
   dst->x = x;
   dst->y = y;
}

static inline void
eina_vector2_array_set(Eina_Vector2 *dst, const double *v)
{
   dst->x = v[0];
   dst->y = v[1];
}

static inline void
eina_vector2_copy(Eina_Vector2 *dst, const Eina_Vector2 *src)
{
   dst->x = src->x;
   dst->y = src->y;
}

static inline void
eina_vector2_negate(Eina_Vector2 *out, const Eina_Vector2 *v)
{
   out->x = -v->x;
   out->y = -v->y;
}

static inline void
eina_vector2_add(Eina_Vector2 *out, const Eina_Vector2 *a,
                 const Eina_Vector2 *b)
{
   out->x = a->x + b->x;
   out->y = a->y + b->y;
}

static inline void
eina_vector2_subtract(Eina_Vector2 *out, const Eina_Vector2 *a,
                      const Eina_Vector2 *b)
{
   out->x = a->x - b->x;
   out->y = a->y - b->y;
}

static inline void
eina_vector2_scale(Eina_Vector2 *out, const Eina_Vector2 *v, double scale)
{
   out->x = scale * v->x;
   out->y = scale * v->y;
}

static inline double
eina_vector2_dot_product(const Eina_Vector2 *a, const Eina_Vector2 *b)
{
   return (a->x * b->x) + (a->y * b->y);
}

static inline double
eina_vector2_length_get(const Eina_Vector2 *v)
{
   return sqrt((v->x * v->x) + (v->y * v->y));
}

static inline double
eina_vector2_length_square_get(const Eina_Vector2 *v)
{
   return (v->x * v->x) + (v->y * v->y);
}

static inline double
eina_vector2_distance_get(const Eina_Vector2 *a, const Eina_Vector2 *b)
{
   Eina_Vector2 v;

   eina_vector2_subtract(&v, a, b);
   return eina_vector2_length_get(&v);
}

static inline double
eina_vector2_distance_square_get(const Eina_Vector2 *a, const Eina_Vector2 *b)
{
   Eina_Vector2 v;

   eina_vector2_subtract(&v, a, b);
   return eina_vector2_length_square_get(&v);
}

static inline void
eina_vector2_normalize(Eina_Vector2 *out, const Eina_Vector2 *v)
{
   /* Assume "v" is not a zero vector */
   eina_vector2_scale(out, v, 1.0 / eina_vector2_length_get(v));
}

static inline void
eina_vector2_transform(Eina_Vector2 *out, const Eina_Matrix2 *m,
                       const Eina_Vector2 *v)
{
   Eina_Vector2 tmp;

   tmp.x = (m->xx * v->x) + (m->yx * v->y);
   tmp.y = (m->xy * v->x) + (m->yy * v->y);

   eina_vector2_copy(out, &tmp);
}

static inline void
eina_vector2_homogeneous_position_transform(Eina_Vector2 *out,
                                            const Eina_Matrix3 *m,
                                            const Eina_Vector2 *v)
{
   Eina_Vector2 tmp;

   tmp.x = (m->xx * v->x) + (m->yx * v->y) + m->zx;
   tmp.y = (m->xy * v->x) + (m->yy * v->y) + m->zy;

   eina_vector2_scale(out, &tmp, 1.0 / ((m->xz * v->x) + (m->yz * v->y) + m->zz));
}

static inline void
eina_vector2_homogeneous_direction_transform(Eina_Vector2 *out,
                                             const Eina_Matrix3 *m,
                                             const Eina_Vector2 *v)
{
   Eina_Vector2 tmp;

   tmp.x = (m->xx * v->x) + (m->yx * v->y);
   tmp.y = (m->xy * v->x) + (m->yy * v->y);

   eina_vector2_copy(out, &tmp);
}

static inline void
eina_vector3_set(Eina_Vector3 *dst, double x, double y, double z)
{
   dst->x = x;
   dst->y = y;
   dst->z = z;
}

static inline void
eina_vector3_array_set(Eina_Vector3 *dst, const double *v)
{
   dst->x = v[0];
   dst->y = v[1];
   dst->z = v[2];
}

static inline void
eina_vector3_copy(Eina_Vector3 *dst, const Eina_Vector3 *src)
{
   dst->x = src->x;
   dst->y = src->y;
   dst->z = src->z;
}

static inline void
eina_vector3_negate(Eina_Vector3 *out, const Eina_Vector3 *v)
{
   out->x = -v->x;
   out->y = -v->y;
   out->z = -v->z;
}

static inline void
eina_vector3_add(Eina_Vector3 *out, const Eina_Vector3 *a, const Eina_Vector3 *b)
{
   out->x = a->x + b->x;
   out->y = a->y + b->y;
   out->z = a->z + b->z;
}

static inline void
eina_vector3_subtract(Eina_Vector3 *out, const Eina_Vector3 *a, const Eina_Vector3 *b)
{
   out->x = a->x - b->x;
   out->y = a->y - b->y;
   out->z = a->z - b->z;
}

static inline void
eina_vector3_scale(Eina_Vector3 *out, const Eina_Vector3 *v, double scale)
{
   out->x = scale * v->x;
   out->y = scale * v->y;
   out->z = scale * v->z;
}

static inline void
eina_vector3_multiply(Eina_Vector3 *out, const Eina_Vector3 *a, const Eina_Vector3 *b)
{
   out->x = a->x * b->x;
   out->y = a->y * b->y;
   out->z = a->z * b->z;
}

static inline double
eina_vector3_dot_product(const Eina_Vector3 *a, const Eina_Vector3 *b)
{
   return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
}

static inline void
eina_vector3_cross_product(Eina_Vector3 *out, const Eina_Vector3 *a, const Eina_Vector3 *b)
{
   Eina_Vector3 tmp;

   tmp.x = a->y * b->z - a->z * b->y;
   tmp.y = a->z * b->x - a->x * b->z;
   tmp.z = a->x * b->y - a->y * b->x;

   eina_vector3_copy(out, &tmp);
}

static inline double
eina_vector3_length_get(const Eina_Vector3 *v)
{
   return (double)sqrt((double)((v->x * v->x) + (v->y * v->y) + (v->z * v->z)));
}

static inline double
eina_vector3_length_square_get(const Eina_Vector3 *v)
{
   return (v->x * v->x) + (v->y * v->y) + (v->z * v->z);
}

static inline double
eina_vector3_distance_get(const Eina_Vector3 *a, const Eina_Vector3 *b)
{
   Eina_Vector3 v;

   eina_vector3_subtract(&v, a, b);
   return eina_vector3_length_get(&v);
}

static inline double
eina_vector3_distance_square_get(const Eina_Vector3 *a, const Eina_Vector3 *b)
{
   Eina_Vector3 v;

   eina_vector3_subtract(&v, a, b);
   return eina_vector3_length_square_get(&v);
}

static inline double
eina_vector3_angle_get(const Eina_Vector3 *a, const Eina_Vector3 *b)
{
   double angle;

   angle = eina_vector3_dot_product(a, b) /
           (eina_vector3_length_get(a) * eina_vector3_length_get(b));
   return angle;
}

static inline void
eina_vector3_normalize(Eina_Vector3 *out, const Eina_Vector3 *v)
{
   /* Assume "v" is not a zero vector */
   eina_vector3_scale(out, v, 1.0 / eina_vector3_length_get(v));
}

static inline void
eina_vector3_transform(Eina_Vector3 *out, const Eina_Matrix3 *m,  const Eina_Vector3 *v)
{
   Eina_Vector3 tmp;

   if (eina_matrix3_type_get(m) == EINA_MATRIX_TYPE_IDENTITY)
     {
        eina_vector3_copy(out, v);
        return;
     }

   tmp.x = (m->xx * v->x) + (m->yx * v->y) + (m->zx * v->z);
   tmp.y = (m->xy * v->x) + (m->yy * v->y) + (m->zy * v->z);
   tmp.z = (m->xz * v->x) + (m->yz * v->y) + (m->zz * v->z);

   eina_vector3_copy(out, &tmp);
}

static inline void
eina_vector3_homogeneous_direction_transform(Eina_Vector3 *out, const Eina_Matrix4 *m,  const Eina_Vector3 *v)
{
   Eina_Vector3 tmp;

   if (eina_matrix4_type_get(m) == EINA_MATRIX_TYPE_IDENTITY)
     {
        eina_vector3_copy(out, v);
        return;
     }

   tmp.x = (m->xx * v->x) + (m->yx * v->y) + (m->zx * v->z);
   tmp.y = (m->xy * v->x) + (m->yy * v->y) + (m->zy * v->z);
   tmp.z = (m->xz * v->x) + (m->yz * v->y) + (m->zz * v->z);

   eina_vector3_copy(out, &tmp);
}

static inline void
eina_vector3_homogeneous_position_transform(Eina_Vector3 *out, const Eina_Matrix4 *m,
                                            const Eina_Vector3 *v)
{
   Eina_Vector3 tmp;

   if (eina_matrix4_type_get(m) == EINA_MATRIX_TYPE_IDENTITY)
     {
        eina_vector3_copy(out, v);
        return;
     }

   if (EINA_DOUBLE_EQUAL((m->xw * v->x) + (m->yw * v->y) +
                         (m->zw * v->z) + m->ww,
                         0.0))
     return;

   tmp.x = (m->xx * v->x) + (m->yx * v->y) + (m->zx * v->z) + m->wx;
   tmp.y = (m->xy * v->x) + (m->yy * v->y) + (m->zy * v->z) + m->wy;
   tmp.z = (m->xz * v->x) + (m->yz * v->y) + (m->zz * v->z) + m->wz;

   eina_vector3_scale(out, &tmp,
                      1.0 / ((m->xw * v->x) + (m->yw * v->y) +
                             (m->zw * v->z) + m->ww));
}

static inline void
eina_vector3_quaternion_rotate(Eina_Vector3 *out, const Eina_Vector3 *v,
                               const Eina_Quaternion *q)
{
   Eina_Vector3   uv, uuv;
   Eina_Vector3   axis;

   eina_vector3_set(&axis, q->x, q->y, q->z);

   eina_vector3_cross_product(&uv, &axis, v);
   eina_vector3_cross_product(&uuv, &axis, &uv);

   eina_vector3_scale(&uv, &uv, 2.0 * q->w);
   eina_vector3_scale(&uuv, &uuv, 2.0);

   out->x = v->x + uv.x + uuv.x;
   out->y = v->y + uv.y + uuv.y;
   out->z = v->z + uv.z + uuv.z;
}

static inline void
eina_vector3_orthogonal_projection_on_plane(Eina_Vector3 *out, const Eina_Vector3 *v,
                                            const Eina_Vector3 *normal)
{
   double a;
   Eina_Vector3 projection;

   /* Orthoprojection of vector on the plane is the difference
      between a vector and its orthogonal projection onto the orthogonal
      complement to the plane */
   a = eina_vector3_dot_product(v, normal) / eina_vector3_length_square_get(normal);
   eina_vector3_scale(&projection, normal, a);
   eina_vector3_subtract(out, v, &projection);

   return;
}

static inline void
eina_vector3_plane_by_points(Eina_Quaternion *out, const Eina_Vector3 *a,
                             const Eina_Vector3 *b, const Eina_Vector3 *c)
{
   out->x = (b->y - a->y) * (c->z - a->z) - (b->z - a->z) * (c->y - a->y);
   out->y = -(b->x - a->x) * (c->z - a->z) + (b->z - a->z) * (c->x - a->x);
   out->z = (b->x - a->x) * (c->y - a->y) - (b->y - a->y) * (c->x - a->x);
   out->w = (-a->x) * ((b->y - a->y)*(c->z - a->z) - (b->z - a->z) * (c->y - a->y)) -
            (-a->y) * ((b->x - a->x) * (c->z - a->z) - (b->z - a->z) * (c->x - a->x)) +
            (-a->z) * ((b->x - a->x) * (c->y - a->y) - (b->y - a->y) * (c->x - a->x));
}

static inline void
eina_vector3_homogeneous_position_set(Eina_Vector3 *out, const Eina_Quaternion *v)
{
   /* Assume "v" is a positional vector. (v->w != 0.0) */
   double h = 1.0 / v->w;

   out->x = v->x * h;
   out->y = v->y * h;
   out->z = v->z * h;
}

static inline void
eina_vector3_homogeneous_direction_set(Eina_Vector3 *out, const Eina_Quaternion *v)
{
   /* Assume "v" is a directional vector. (v->w == 0.0) */
   out->x = v->x;
   out->y = v->y;
   out->z = v->z;
}

static inline Eina_Bool
eina_vector3_equivalent(Eina_Vector3 *a, const Eina_Vector3 *b)
{
   /* Assume "v" is a directional vector. (v->w == 0.0) */
   return (EINA_DOUBLE_EQUAL(a->x, b->x) &&
           EINA_DOUBLE_EQUAL(a->y, b->y) &&
           EINA_DOUBLE_EQUAL(a->z, b->z));
}

static inline Eina_Bool
eina_vector3_triangle_equivalent(Eina_Vector3 *v0, Eina_Vector3 *v1,
                                 Eina_Vector3 *v2, Eina_Vector3 *w0,
                                 Eina_Vector3 *w1, Eina_Vector3 *w2)
{
   if ((EINA_DOUBLE_EQUAL(v0->x, w0->x) &&
        EINA_DOUBLE_EQUAL(v0->y, w0->y) &&
        EINA_DOUBLE_EQUAL(v0->z, w0->z)) &&
       (EINA_DOUBLE_EQUAL(v1->x, w1->x) &&
        EINA_DOUBLE_EQUAL(v1->y, w1->y) &&
        EINA_DOUBLE_EQUAL(v1->z, w1->z)) &&
       (EINA_DOUBLE_EQUAL(v2->x, w2->x) &&
        EINA_DOUBLE_EQUAL(v2->y, w2->y) &&
        EINA_DOUBLE_EQUAL(v2->z, w2->z)))
     return EINA_TRUE;
   return EINA_FALSE;
}

#endif

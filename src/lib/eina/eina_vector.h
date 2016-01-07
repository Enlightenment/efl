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

#ifndef EINA_VECTOR_H_
#define EINA_VECTOR_H_

#include "eina_matrix.h"

/**
 * @file
 * @ender_group{Eina_Vector_Type}
 * @ender_group{Eina_Vector2}
 */

typedef struct _Eina_Vector2 Eina_Vector2;

/**
 * @}
 * @defgroup Eina_Vector2 Vectors in floating point
 * @ingroup Eina_Basic
 * @brief Vector definition and operations
 * @{
 */

struct _Eina_Vector2
{
   double x;
   double y;
};

/**
 * @brief Set parameters to vector.
 *
 * @param dst The resulting vector.
 * @param x The x component.
 * @param y The y component.
 *
 * @since 1.17
 */
static inline void
eina_vector2_set(Eina_Vector2 *dst, double x, double y)
{
   dst->x = x;
   dst->y = y;
}

/**
 * @brief Set array to vector.
 *
 * @param dst The resulting vector.
 * @param v The the array[2] for set.
 *
 * Set to vector first 2 elements from array.
 *
 * @since 1.17
 */
static inline void
eina_vector2_array_set(Eina_Vector2 *dst, const double *v)
{
   dst->x = v[0];
   dst->y = v[1];
}

/**
 * @brief Copy vector.
 *
 * @param dst The vector copy.
 * @param src The vector for copy.
 *
 * @since 1.17
 */
static inline void
eina_vector2_copy(Eina_Vector2 *dst, const Eina_Vector2 *src)
{
   dst->x = src->x;
   dst->y = src->y;
}

/**
 * @brief Make negative vector.
 *
 * @param out The resulting vector.
 * @param v The current vector.
 *
 * @since 1.17
 */
static inline void
eina_vector2_negate(Eina_Vector2 *out, const Eina_Vector2 *v)
{
   out->x = -v->x;
   out->y = -v->y;
}

/**
 * @brief Add two vectors.
 *
 * @param out The resulting vector.
 * @param a The first member of the add.
 * @param b The second member of the add.
 *
 * @since 1.17
 */
static inline void
eina_vector2_add(Eina_Vector2 *out, const Eina_Vector2 *a,
                 const Eina_Vector2 *b)
{
   out->x = a->x + b->x;
   out->y = a->y + b->y;
}

/**
 * @brief Subtract two vectors
 *
 * @param out The resulting vector
 * @param a The first member of the subtract
 * @param b The second member of the subtract
 *
 * @since 1.17
 */
static inline void
eina_vector2_subtract(Eina_Vector2 *out, const Eina_Vector2 *a,
                      const Eina_Vector2 *b)
{
   out->x = a->x - b->x;
   out->y = a->y - b->y;
}

/**
 * @brief Scale vector.
 *
 * @param out The resulting vector.
 * @param v The vector for scale.
 * @param scale The scale value.
 *
 * @since 1.17
 */
static inline void
eina_vector2_scale(Eina_Vector2 *out, const Eina_Vector2 *v, double scale)
{
   out->x = scale * v->x;
   out->y = scale * v->y;
}

/**
 * @brief Return the dot product of the two vectors.
 *
 * @param a The first member.
 * @param b The secondt member.
 * @return The dot product.
 *
 * @since 1.17
 */
static inline double
eina_vector2_dot_product(const Eina_Vector2 *a, const Eina_Vector2 *b)
{
   return (a->x * b->x) + (a->y * b->y);
}

/**
 * @brief Return the length of the given vector.
 *
 * @param v The vector.
 * @return The length.
 *
 * @since 1.17
 */
static inline double
eina_vector2_length_get(const Eina_Vector2 *v)
{
   return sqrt((v->x * v->x) + (v->y * v->y));
}

/**
 * @brief Return the length in square of the given vector.
 *
 * @param v The vector.
 * @return The length in square.
 *
 * @since 1.17
 */
static inline double
eina_vector2_length_square_get(const Eina_Vector2 *v)
{
   return (v->x * v->x) + (v->y * v->y);
}

/**
 * @brief Return the distance between of two vectors.
 *
 * @param a The first vector.
 * @param b The second vector.
 * @return The distance.
 *
 * @since 1.17
 */
static inline double
eina_vector2_distance_get(const Eina_Vector2 *a, const Eina_Vector2 *b)
{
   Eina_Vector2 v;

   eina_vector2_subtract(&v, a, b);
   return eina_vector2_length_get(&v);
}

/**
 * @brief Return the distance in square between of two vectors.
 *
 * @param a The first vector.
 * @param b The second vector.
 * @return The distance in square.
 *
 * @since 1.17
 */
static inline double
eina_vector2_distance_square_get(const Eina_Vector2 *a, const Eina_Vector2 *b)
{
   Eina_Vector2 v;

   eina_vector2_subtract(&v, a, b);
   return eina_vector2_length_square_get(&v);
}

/**
 * @brief normalize vector.
 *
 * @param out The resulting vector.
 * @param v The vector for normalize.
 *
 * @since 1.17
 */
static inline void
eina_vector2_normalize(Eina_Vector2 *out, const Eina_Vector2 *v)
{
   /* Assume "v" is not a zero vector */
   eina_vector2_scale(out, v, 1.0 / eina_vector2_length_get(v));
}

/**
 * @brief Transform vector.
 *
 * @param out The resulting vector.
 * @param m The matrix for transform.
 * @param v The ector for transform.
 *
 * @since 1.17
 */
static inline void
eina_vector2_transform(Eina_Vector2 *out, const Eina_Matrix2 *m,
                       const Eina_Vector2 *v)
{
   Eina_Vector2 tmp;

   tmp.x = (m->xx * v->x) + (m->yx * v->y);
   tmp.y = (m->xy * v->x) + (m->yy * v->y);

   eina_vector2_copy(out, &tmp);
}

/**
 * @brief Homogeneous position transform vector.
 *
 * @param out The resulting vector.
 * @param m The matrix for transform.
 * @param v The ector for transform.
 *
 * @since 1.17
 */
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

/**
 * @brief Homogeneous direction ransform vector.
 *
 * @param out The resulting vector.
 * @param m The matrix for transform.
 * @param v The ector for transform.
 *
 * @since 1.17
 */
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

#endif

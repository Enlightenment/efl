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

#ifndef EINA_VECTOR_H_
#define EINA_VECTOR_H_

#include "eina_matrix.h"
#include "eina_quaternion.h"

/**
 * @file
 * @ender_group{Eina_Vector_Type}
 * @ender_group{Eina_Vector2}
 * @ender_group{Eina_Vector3}
 */

typedef struct _Eina_Vector2 Eina_Vector2;
typedef struct _Eina_Vector3 Eina_Vector3;

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
 * @}
 * @defgroup Eina_Vector3 Vectors in floating point
 * @ingroup Eina_Basic
 * @brief Vector definition and operations
 * @{
 */

struct _Eina_Vector3
{
   double x;
   double y;
   double z;
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
static inline void eina_vector2_set(Eina_Vector2 *dst, double x, double y);

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
static inline void eina_vector2_array_set(Eina_Vector2 *dst, const double *v);

/**
 * @brief Copy vector.
 *
 * @param dst The vector copy.
 * @param src The vector for copy.
 *
 * @since 1.17
 */
static inline void eina_vector2_copy(Eina_Vector2 *dst, const Eina_Vector2 *src);

/**
 * @brief Make negative vector.
 *
 * @param out The resulting vector.
 * @param v The current vector.
 *
 * @since 1.17
 */
static inline void eina_vector2_negate(Eina_Vector2 *out, const Eina_Vector2 *v);

/**
 * @brief Add two vectors.
 *
 * @param out The resulting vector.
 * @param a The first member of the add.
 * @param b The second member of the add.
 *
 * @since 1.17
 */
static inline void eina_vector2_add(Eina_Vector2 *out, const Eina_Vector2 *a, const Eina_Vector2 *b);

/**
 * @brief Subtract two vectors
 *
 * @param out The resulting vector
 * @param a The first member of the subtract
 * @param b The second member of the subtract
 *
 * @since 1.17
 */
static inline void eina_vector2_subtract(Eina_Vector2 *out, const Eina_Vector2 *a, const Eina_Vector2 *b);

/**
 * @brief Scale vector.
 *
 * @param out The resulting vector.
 * @param v The vector for scale.
 * @param scale The scale value.
 *
 * @since 1.17
 */
static inline void eina_vector2_scale(Eina_Vector2 *out, const Eina_Vector2 *v, double scale);

/**
 * @brief Return the dot product of the two vectors.
 *
 * @param a The first member.
 * @param b The second member.
 * @return The dot product.
 *
 * @since 1.17
 */
static inline double eina_vector2_dot_product(const Eina_Vector2 *a, const Eina_Vector2 *b);

/**
 * @brief Return the length of the given vector.
 *
 * @param v The vector.
 * @return The length.
 *
 * @since 1.17
 */
static inline double eina_vector2_length_get(const Eina_Vector2 *v);

/**
 * @brief Return the length in square of the given vector.
 *
 * @param v The vector.
 * @return The length in square.
 *
 * @since 1.17
 */
static inline double eina_vector2_length_square_get(const Eina_Vector2 *v);

/**
 * @brief Return the distance between of two vectors.
 *
 * @param a The first vector.
 * @param b The second vector.
 * @return The distance.
 *
 * @since 1.17
 */
static inline double eina_vector2_distance_get(const Eina_Vector2 *a, const Eina_Vector2 *b);

/**
 * @brief Return the distance in square between of two vectors.
 *
 * @param a The first vector.
 * @param b The second vector.
 * @return The distance in square.
 *
 * @since 1.17
 */
static inline double eina_vector2_distance_square_get(const Eina_Vector2 *a, const Eina_Vector2 *b);

/**
 * @brief normalize vector.
 *
 * @param out The resulting vector.
 * @param v The vector for normalize.
 *
 * @since 1.17
 */
static inline void eina_vector2_normalize(Eina_Vector2 *out, const Eina_Vector2 *v);

/**
 * @brief Transform vector.
 *
 * @param out The resulting vector.
 * @param m The matrix for transform.
 * @param v The vector for transform.
 *
 * @since 1.17
 */
static inline void eina_vector2_transform(Eina_Vector2 *out, const Eina_Matrix2 *m, const Eina_Vector2 *v);

/**
 * @brief Homogeneous position transform vector.
 *
 * @param out The resulting vector.
 * @param m The matrix for transform.
 * @param v The vector for transform.
 *
 * @since 1.17
 */
static inline void eina_vector2_homogeneous_position_transform(Eina_Vector2 *out, const Eina_Matrix3 *m, const Eina_Vector2 *v);

/**
 * @brief Homogeneous direction transform vector.
 *
 * @param out The resulting vector.
 * @param m The matrix for transform.
 * @param v The vector for transform.
 *
 * @since 1.17
 */
static inline void eina_vector2_homogeneous_direction_transform(Eina_Vector2 *out, const Eina_Matrix3 *m, const Eina_Vector2 *v);

/**
 * @brief Set parameters to vector.
 *
 * @param dst The resulting vector.
 * @param x The x component.
 * @param y The y component.
 * @param z The z component.
 *
 * @since 1.18
 */
static inline void eina_vector3_set(Eina_Vector3 *dst, double x, double y, double z);

/**
 * @brief Set array to vector.
 *
 * @param dst The resulting vector.
 * @param v The the array[3] for set.
 *
 * Set to vector first 3 elements from array.
 *
 * @since 1.18
 */
static inline void eina_vector3_array_set(Eina_Vector3 *dst, const double *v);

/**
 * @brief Copy vector.
 *
 * @param dst The vector copy.
 * @param src The vector for copy.
 *
 * @since 1.18
 */
static inline void eina_vector3_copy(Eina_Vector3 *dst, const Eina_Vector3 *src);

/**
 * @brief Make negative vector.
 *
 * @param out The resulting vector.
 * @param v The current vector.
 *
 * @since 1.18
 */
static inline void eina_vector3_negate(Eina_Vector3 *out, const Eina_Vector3 *v);

/**
 * @brief Add two vectors.
 *
 * @param out The resulting vector.
 * @param a The first member of the add.
 * @param b The second member of the add.
 *
 * @since 1.18
 */
static inline void eina_vector3_add(Eina_Vector3 *out, const Eina_Vector3 *a,
                           const Eina_Vector3 *b);

/**
 * @brief Subtract two vectors
 *
 * @param out The resulting vector
 * @param a The first member of the subtract
 * @param b The second member of the subtract
 *
 * @since 1.18
 */
static inline void eina_vector3_subtract(Eina_Vector3 *out, const Eina_Vector3 *a,
                                const Eina_Vector3 *b);

/**
 * @brief Scale vector.
 *
 * @param out The resulting vector.
 * @param v The vector for scale.
 * @param scale The scale value.
 *
 * @since 1.18
 */
static inline void eina_vector3_scale(Eina_Vector3 *out, const Eina_Vector3 *v, double scale);

/**
 * @brief Multiply two vectors
 *
 * @param out The resulting vector
 * @param a The first member
 * @param b The second member
 *
 * @since 1.18
 */
static inline void eina_vector3_multiply(Eina_Vector3 *out, const Eina_Vector3 *a,
                                const Eina_Vector3 *b);

/**
 * @brief Return the dot product of the two vectors.
 *
 * @param a The first member.
 * @param b The second member.
 * @return The dot product.
 *
 * @since 1.18
 */
static inline double eina_vector3_dot_product(const Eina_Vector3 *a, const Eina_Vector3 *b);

/**
 * @brief Create the cross product of the two vectors.
 *
 * @param out The resulting vector.
 * @param a The first member.
 * @param b The second member.
 *
 * @since 1.18
 */
static inline void eina_vector3_cross_product(Eina_Vector3 *out, const Eina_Vector3 *a,
                                     const Eina_Vector3 *b);

/**
 * @brief Return the length of the given vector.
 *
 * @param v The vector.
 * @return The length.
 *
 * @since 1.18
 */
static inline double eina_vector3_length_get(const Eina_Vector3 *v);

/**
 * @brief Return the length in square of the given vector.
 *
 * @param v The vector.
 * @return The length in square.
 *
 * @since 1.18
 */
static inline double eina_vector3_length_square_get(const Eina_Vector3 *v);

/**
 * @brief Return the distance between of two vectors.
 *
 * @param a The first vector.
 * @param b The second vector.
 * @return The distance.
 *
 * @since 1.18
 */
static inline double eina_vector3_distance_get(const Eina_Vector3 *a, const Eina_Vector3 *b);

/**
 * @brief Return the distance in square between of two vectors.
 *
 * @param a The first vector.
 * @param b The second vector.
 * @return The distance in square.
 *
 * @since 1.18
 */
static inline double eina_vector3_distance_square_get(const Eina_Vector3 *a,
                                             const Eina_Vector3 *b);

/**
 * @brief Return the angle between of two vectors.
 *
 * @param a The first vector.
 * @param b The second vector.
 * @return The angle.
 *
 * @since 1.18
 */
static inline double eina_vector3_angle_get(const Eina_Vector3 *a, const Eina_Vector3 *b);

/**
 * @brief normalize vector.
 *
 * @param out The resulting vector.
 * @param v The not NULL vector for normalize.
 *
 * @since 1.18
 */
static inline void eina_vector3_normalize(Eina_Vector3 *out, const Eina_Vector3 *v);

/**
 * @brief Transform vector.
 *
 * @param out The resulting vector.
 * @param m The matrix for transform.
 * @param v The vector for transform.
 *
 * @since 1.18
 */
static inline void eina_vector3_transform(Eina_Vector3 *out, const Eina_Matrix3 *m,
                                 const Eina_Vector3 *v);

/**
 * @brief Homogeneous direction transform vector.
 *
 * @param out The resulting vector.
 * @param m The matrix for transform.
 * @param v The vector for transform.
 *
 * @since 1.18
 */
static inline void eina_vector3_homogeneous_direction_transform(Eina_Vector3 *out, const Eina_Matrix4 *m,
                                                               const Eina_Vector3 *v);

/**
 * @brief Homogeneous position transform vector.
 *
 * @param out The resulting vector.
 * @param m The matrix for transform.
 * @param v The vector for transform.
 *
 * @since 1.18
 */
static inline void eina_vector3_homogeneous_position_transform(Eina_Vector3 *out, const Eina_Matrix4 *m,
                                                               const Eina_Vector3 *v);


/**
 * @brief Rotate vector.
 *
 * @param out The resulting vector.
 * @param v The vector for rotate.
 * @param q The quaternion in radians for rotate.
 *
 * @since 1.18
 */
static inline void eina_vector3_quaternion_rotate(Eina_Vector3 *out, const Eina_Vector3 *v,
                                                  const Eina_Quaternion *q);

/**
 * @brief Create orthogonal projection on plane between vector and normal.
 *
 * @param out The resulting vector.
 * @param v The vector for projection.
 * @param normal The normal for projection.
 *
 * @since 1.18
 */
static inline void eina_vector3_orthogonal_projection_on_plane(Eina_Vector3 *out, const Eina_Vector3 *v,
                                                               const Eina_Vector3 *normal);

/**
 * @brief Plane by points between three vectors.
 *
 * @param out The resulting quaternion of plane.
 * @param a The first member.
 * @param b The second member.
 * @param c The third member.
 *
 * @since 1.18
 */
static inline void eina_vector3_plane_by_points(Eina_Quaternion *out, const Eina_Vector3 *a,
                                                const Eina_Vector3 *b, const Eina_Vector3 *c);

/**
 * @brief Homogeneous position set.
 *
 * @param out The resulting vector.
 * @param v The quaternion for position.
 *
 * @since 1.18
 */
static inline void eina_vector3_homogeneous_position_set(Eina_Vector3 *out, const Eina_Quaternion *v);

/**
 * @brief Homogeneous direction set.
 *
 * @param out The resulting vector.
 * @param v The quaternion for direction.
 *
 * @since 1.18
 */
static inline void eina_vector3_homogeneous_direction_set(Eina_Vector3 *out, const Eina_Quaternion *v);

/**
 * @brief Check the equivalent between of two vectors.
 *
 * @param a The first vector.
 * @param b The second vector.
 * @return The EINA_TRUE if equivalent.
 *
 * @since 1.18
 */
static inline Eina_Bool eina_vector3_equivalent(Eina_Vector3 *a, const Eina_Vector3 *b);

/**
 * @brief Check the equivalent between of two triangles of vectors.
 *
 * @param v0 The first member of first triangle.
 * @param v1 The second member of first triangle.
 * @param v2 The third member of first triangle.
 * @param w0 The first member of second triangle.
 * @param w1 The second member of second triangle.
 * @param w2 The third member of second triangle.
 * @return The EINA_TRUE if equivalent.
 *
 * @since 1.18
 */
static inline Eina_Bool eina_vector3_triangle_equivalent(Eina_Vector3 *v0, Eina_Vector3 *v1,
                                                         Eina_Vector3 *v2, Eina_Vector3 *w0,
                                                         Eina_Vector3 *w1, Eina_Vector3 *w2);

/** @} */

#include "eina_inline_vector.x"

#endif

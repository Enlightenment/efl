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
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef EINA_QUATERNION_H_
#define EINA_QUATERNION_H_

typedef struct _Eina_Quaternion_F16p16 Eina_Quaternion_F16p16;
typedef struct _Eina_Quaternion Eina_Quaternion;
typedef struct _Eina_Point_3D Eina_Point_3D;
typedef struct _Eina_Point_3D_F16p16 Eina_Point_3D_F16p16;

struct _Eina_Quaternion
{
   double x;
   double y;
   double z;
   double w;
};

struct _Eina_Quaternion_F16p16
{
   Eina_F16p16 x;
   Eina_F16p16 y;
   Eina_F16p16 z;
   Eina_F16p16 w;
};

struct _Eina_Point_3D
{
   double x;
   double y;
   double z;
};

struct _Eina_Point_3D_F16p16
{
   Eina_F16p16 x;
   Eina_F16p16 y;
   Eina_F16p16 z;
};

EAPI void eina_quaternion_f16p16_set(Eina_Quaternion *out,
                                     Eina_F16p16 x, Eina_F16p16 y,
                                     Eina_F16p16 z, Eina_F16p16 w); /**< @since 1.15 */
EAPI Eina_F16p16 eina_quaternion_f16p16_norm(const Eina_Quaternion_F16p16 *q); /**< @since 1.15 */
EAPI void eina_quaternion_f16p16_negative(Eina_Quaternion_F16p16 *out,
                                          const Eina_Quaternion_F16p16 *in); /**< @since 1.15 */
EAPI void eina_quaternion_f16p16_add(Eina_Quaternion_F16p16 *out,
                                     const Eina_Quaternion_F16p16 *a,
                                     const Eina_Quaternion_F16p16 *b); /**< @since 1.15 */
EAPI void eina_quaternion_f16p16_mul(Eina_Quaternion_F16p16 *out,
                                     const Eina_Quaternion_F16p16 *a,
                                     const Eina_Quaternion_F16p16 *b); /**< @since 1.15 */
EAPI void eina_quaternion_f16p16_scale(Eina_Quaternion_F16p16 *out,
                                       const Eina_Quaternion_F16p16 *a,
                                       Eina_F16p16 b); /**< @since 1.15 */
EAPI void eina_quaternion_f16p16_conjugate(Eina_Quaternion_F16p16 *out,
                                           const Eina_Quaternion_F16p16 *in); /**< @since 1.15 */
EAPI Eina_F16p16 eina_quaternion_f16p16_dot(const Eina_Quaternion_F16p16 *a,
                                            const Eina_Quaternion_F16p16 *b); /**< @since 1.15 */
EAPI void eina_quaternion_f16p16_lerp(Eina_Quaternion_F16p16 *out,
                                      const Eina_Quaternion_F16p16 *a,
                                      const Eina_Quaternion_F16p16 *b,
                                      Eina_F16p16 pos); /**< @since 1.15 */
EAPI void eina_quaternion_f16p16_slerp(Eina_Quaternion_F16p16 *out,
                                       const Eina_Quaternion_F16p16 *a,
                                       const Eina_Quaternion_F16p16 *b,
                                       Eina_F16p16 pos); /**< @since 1.15 */
EAPI void eina_quaternion_f16p16_nlerp(Eina_Quaternion_F16p16 *out,
                                       const Eina_Quaternion_F16p16 *a,
                                       const Eina_Quaternion_F16p16 *b,
                                       Eina_F16p16 pos); /**< @since 1.15 */
EAPI void eina_quaternion_f16p16_rotate(Eina_Point_3D_F16p16 *p,
                                        const Eina_Point_3D_F16p16 *center,
                                        const Eina_Quaternion_F16p16 *q); /**< @since 1.15 */
EAPI void eina_quaternion_f16p16_rotation_matrix3_get(Eina_Matrix3_F16p16 *m,
                                                     const Eina_Quaternion_F16p16 *q); /**< @since 1.15 */

EAPI void eina_quaternion_set(Eina_Quaternion *q, double x,
                              double y, double z, double w);
EAPI double eina_quaternion_norm(const Eina_Quaternion *q); /**< @since 1.15 */
EAPI void eina_quaternion_negative(Eina_Quaternion *out,
                                   const Eina_Quaternion *in); /**< @since 1.15 */
EAPI void eina_quaternion_add(Eina_Quaternion *out,
                              const Eina_Quaternion *a,
                              const Eina_Quaternion *b); /**< @since 1.15 */
EAPI void eina_quaternion_mul(Eina_Quaternion *out,
                              const Eina_Quaternion *a,
                              const Eina_Quaternion *b); /**< @since 1.15 */
EAPI void eina_quaternion_scale(Eina_Quaternion *out,
                                const Eina_Quaternion *a,
                                double b); /**< @since 1.15 */
EAPI void eina_quaternion_conjugate(Eina_Quaternion *out,
                                    const Eina_Quaternion *in); /**< @since 1.15 */
EAPI double eina_quaternion_dot(const Eina_Quaternion *a,
                                const Eina_Quaternion *b); /**< @since 1.15 */
EAPI void eina_quaternion_normalized(Eina_Quaternion *out,
                                     const Eina_Quaternion *in); /**< @since 1.15 */
EAPI void eina_quaternion_lerp(Eina_Quaternion *out,
                               const Eina_Quaternion *a,
                               const Eina_Quaternion *b,
                               double pos); /**< @since 1.15 */
EAPI void eina_quaternion_slerp(Eina_Quaternion *out,
                                const Eina_Quaternion *a,
                                const Eina_Quaternion *b,
                                double pos); /**< @since 1.15 */
EAPI void eina_quaternion_nlerp(Eina_Quaternion *out,
                                const Eina_Quaternion *a,
                                const Eina_Quaternion *b,
                                double pos); /**< @since 1.15 */
EAPI void eina_quaternion_rotate(Eina_Point_3D *p,
                                 const Eina_Point_3D *center,
                                 const Eina_Quaternion *q); /**< @since 1.15 */
EAPI void eina_quaternion_rotation_matrix3_get(Eina_Matrix3 *m,
                                               const Eina_Quaternion *q); /**< @since 1.15 */
EAPI void eina_matrix3_quaternion_get(Eina_Quaternion *q,
                                      const Eina_Matrix3 *m); /**< @since 1.15 */
EAPI Eina_Bool eina_matrix4_quaternion_to(Eina_Quaternion *rotation,
                                          Eina_Quaternion *perspective,
                                          Eina_Point_3D *translation,
                                          Eina_Point_3D *scale,
                                          Eina_Point_3D *skew,
                                          const Eina_Matrix4 *m); /**< @since 1.16 */
EAPI void eina_quaternion_matrix4_to(Eina_Matrix4 *m,
                                     const Eina_Quaternion *rotation,
                                     const Eina_Quaternion *perspective,
                                     const Eina_Point_3D *translation,
                                     const Eina_Point_3D *scale,
                                     const Eina_Point_3D *skew); /**< @since 1.16 */
/**
 * @brief Compute the inverse of the given quaternion.
 *
 * @param out The quaternion to invert.
 * @param q The quaternion matrix.
 *
 * This function inverses the quaternion @p q and stores the result in
 * @p out.
 *
 * @since 1.17
 */
EAPI void eina_quaternion_inverse(Eina_Quaternion *out, const Eina_Quaternion *q);

/**
 * @brief Set array to quaternion.
 *
 * @param dst The result quaternion
 * @param v The the array[4] for set
 *
 * Set to quaternion first 4 elements from array
 *
 * @since 1.17
 */
EAPI void eina_quaternion_array_set(Eina_Quaternion *dst, const double *v);

/**
 * @brief Copy quaternion.
 *
 * @param dst The quaternion copy
 * @param src The quaternion for copy.
 *
 * @since 1.17
 */
EAPI void eina_quaternion_copy(Eina_Quaternion *dst,
                               const Eina_Quaternion *src);

/**
 * @brief Homogeneous quaternion
 *
 * @param out The resulting quaternion
 * @param v The given quaternion
 *
 * @since 1.17
 */
EAPI void eina_quaternion_homogeneous_regulate(Eina_Quaternion *out,
                                               const Eina_Quaternion *v);

/**
 * @brief Subtract two quaternions
 *
 * @param out The resulting quaternion
 * @param a The first member of the subtract
 * @param b The second member of the subtract
 *
 * @since 1.17
 */
EAPI void eina_quaternion_subtract(Eina_Quaternion *out, const Eina_Quaternion *a,
                                   const Eina_Quaternion *b);

/**
 * @brief Return the length of the given quaternion.
 *
 * @param v The quaternion.
 * @return The length.
 *
 * @since 1.17
 */
EAPI double eina_quaternion_length_get(const Eina_Quaternion *v);

/**
 * @brief Return the length in square of the given quaternion.
 *
 * @param v The quaternion.
 * @return The length in square.
 *
 * @since 1.17
 */
EAPI double eina_quaternion_length_square_get(const Eina_Quaternion *v);

/**
 * @brief Return the distance between of two quaternions.
 *
 * @param a The first quaternion.
 * @param b The second quaternion.
 * @return The distance.
 *
 * @since 1.17
 */
EAPI double eina_quaternion_distance_get(const Eina_Quaternion *a,
                                         const Eina_Quaternion *b);
/**
 * @brief Return the distance in square between of two quaternions.
 *
 * @param a The first quaternion.
 * @param b The second quaternion.
 * @return The distance in square.
 *
 * @since 1.17
 */
EAPI double eina_quaternion_distance_square_get(const Eina_Quaternion *a,
                                                const Eina_Quaternion *b);

/**
 * @brief Transform quaternion.
 *
 * @param out The result quaternion.
 * @param v The quaternion for transform.
 * @param m The matrix for transform.
 *
 * @since 1.17
 */
EAPI void eina_quaternion_transform(Eina_Quaternion *out, const Eina_Quaternion *v,
                                    const Eina_Matrix4 *m);

/**
 * @brief Return the angle plains between of two quaternions.
 *
 * @param a The first quaternion.
 * @param b The second quaternion.
 * @return The angle.
 *
 * @since 1.17
 */
EAPI double eina_quaternion_angle_plains(Eina_Quaternion *a, Eina_Quaternion *b);

#endif

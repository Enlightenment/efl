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
                                     Eina_F16p16 z, Eina_F16p16 w);
EAPI Eina_F16p16 eina_quaternion_f16p16_norm(const Eina_Quaternion_F16p16 *q);
EAPI void eina_quaternion_f16p16_negative(Eina_Quaternion_F16p16 *out,
                                          const Eina_Quaternion_F16p16 *in);
EAPI void eina_quaternion_f16p16_add(Eina_Quaternion_F16p16 *out,
                                     const Eina_Quaternion_F16p16 *a,
                                     const Eina_Quaternion_F16p16 *b);
EAPI void eina_quaternion_f16p16_mul(Eina_Quaternion_F16p16 *out,
                                     const Eina_Quaternion_F16p16 *a,
                                     const Eina_Quaternion_F16p16 *b);
EAPI void eina_quaternion_f16p16_scale(Eina_Quaternion_F16p16 *out,
                                       const Eina_Quaternion_F16p16 *a,
                                       Eina_F16p16 b);
EAPI void eina_quaternion_f16p16_conjugate(Eina_Quaternion_F16p16 *out,
                                           const Eina_Quaternion_F16p16 *in);
EAPI Eina_F16p16 eina_quaternion_f16p16_dot(const Eina_Quaternion_F16p16 *a,
                                            const Eina_Quaternion_F16p16 *b);
EAPI void eina_quaternion_f16p16_lerp(Eina_Quaternion_F16p16 *out,
                                      const Eina_Quaternion_F16p16 *a,
                                      const Eina_Quaternion_F16p16 *b,
                                      Eina_F16p16 pos);
EAPI void eina_quaternion_f16p16_slerp(Eina_Quaternion_F16p16 *out,
                                       const Eina_Quaternion_F16p16 *a,
                                       const Eina_Quaternion_F16p16 *b,
                                       Eina_F16p16 pos);
EAPI void eina_quaternion_f16p16_nlerp(Eina_Quaternion_F16p16 *out,
                                       const Eina_Quaternion_F16p16 *a,
                                       const Eina_Quaternion_F16p16 *b,
                                       Eina_F16p16 pos);
EAPI void eina_quaternion_f16p16_rotate(Eina_Point_3D_F16p16 *p,
                                        const Eina_Point_3D_F16p16 *center,
                                        const Eina_Quaternion_F16p16 *q);
EAPI void eina_quaternion_f16p16_rotation_matri3_get(Eina_Matrix3_F16p16 *m,
                                                     const Eina_Quaternion_F16p16 *q);

EAPI void eina_quaternion_set(Eina_Quaternion *q, double x,
                              double y, double z, double w);
EAPI double eina_quaternion_norm(const Eina_Quaternion *q);
EAPI void eina_quaternion_negative(Eina_Quaternion *out,
                                   const Eina_Quaternion *in);
EAPI void eina_quaternion_add(Eina_Quaternion *out,
                              const Eina_Quaternion *a,
                              const Eina_Quaternion *b);
EAPI void eina_quaternion_mul(Eina_Quaternion *out,
                              const Eina_Quaternion *a,
                              const Eina_Quaternion *b);
EAPI void eina_quaternion_scale(Eina_Quaternion *out,
                                const Eina_Quaternion *a,
                                double b);
EAPI void eina_quaternion_conjugate(Eina_Quaternion *out,
                                    const Eina_Quaternion *in);
EAPI double eina_quaternion_dot(const Eina_Quaternion *a,
                                const Eina_Quaternion *b);
EAPI void eina_quaternion_normalized(Eina_Quaternion *out,
                                     const Eina_Quaternion *in);
EAPI void eina_quaternion_lerp(Eina_Quaternion *out,
                               const Eina_Quaternion *a,
                               const Eina_Quaternion *b,
                               double pos);
EAPI void eina_quaternion_slerp(Eina_Quaternion *out,
                                const Eina_Quaternion *a,
                                const Eina_Quaternion *b,
                                double pos);
EAPI void eina_quaternion_nlerp(Eina_Quaternion *out,
                                const Eina_Quaternion *a,
                                const Eina_Quaternion *b,
                                double pos);
EAPI void eina_quaternion_rotate(Eina_Point_3D *p,
                                 const Eina_Point_3D *center,
                                 const Eina_Quaternion *q);
EAPI void eina_quaternion_rotation_matrix3_get(Eina_Matrix3 *m,
                                               const Eina_Quaternion *q);
EAPI void eina_matrix3_quaternion_get(Eina_Quaternion *q,
                                      const Eina_Matrix3 *m);

#endif

/* EINA - EFL data type library
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
#ifndef EINA_MATRIX3_H_
#define EINA_MATRIX3_H_

#include "eina_quad.h"

/**
 * @file
 * @ender_group{Eina_Matrix_Type}
 * @ender_group{Eina_Matrix3_F16p16}
 * @ender_group{Eina_Matrix3}
 */

/**
 * @defgroup Eina_Matrix_Type Matrices type
 * @ingroup Eina_Basic
 * @brief Matrix3 types
 * @{
 */
typedef enum _Eina_Matrix_Type
  {
    EINA_MATRIX_TYPE_IDENTITY, /**< Identity matrix3 type */
    EINA_MATRIX_TYPE_AFFINE, /**< Affine matrix3 type */
    EINA_MATRIX_TYPE_PROJECTIVE, /**< Projective matrix3 type */
    EINA_MATRIX_TYPE_LAST /**< The total number of matrix3 types */
  } Eina_Matrix_Type;

/**
 * @}
 * @defgroup Eina_Matrix3_F16p16 Matrices in fixed point
 * @ingroup Eina_Basic
 * @brief Fixed point matrices operations
 * @{
 */

/**
 * Fixed point matrix3 handler
 */
typedef struct _Eina_Matrix3_F16p16 Eina_Matrix3_F16p16;

struct _Eina_Matrix3_F16p16
{
   Eina_F16p16 xx; /**< xx in x' = (x * xx) + (y * xy) + xz */
   Eina_F16p16 xy; /**< xy in x' = (x * xx) + (y * xy) + xz */
   Eina_F16p16 xz; /**< xz in x' = (x * xx) + (y * xy) + xz */

   Eina_F16p16 yx; /**< yx in y' = (x * yx) + (y * yy) + yz */
   Eina_F16p16 yy; /**< yy in y' = (x * yx) + (y * yy) + yz */
   Eina_F16p16 yz; /**< yz in y' = (x * yx) + (y * yy) + yz */

   Eina_F16p16 zx; /**< zx in z' = (x * zx) + (y * zy) + zz */
   Eina_F16p16 zy; /**< zy in z' = (x * zx) + (y * zy) + zz */
   Eina_F16p16 zz; /**< zz in z' = (x * zx) + (y * zy) + zz */
};

/**
 * @brief Set the given fixed point matrix to the identity matrix.
 *
 * @param m The fixed point matrix to set
 *
 * This function sets @p m to the identity matrix. No check is done on
 * @p m.
 */
EAPI void eina_matrix3_f16p16_identity(Eina_Matrix3_F16p16 *m);

EAPI void eina_matrix3_f16p16_compose(const Eina_Matrix3_F16p16 *m1,
                                      const Eina_Matrix3_F16p16 *m2,
                                      Eina_Matrix3_F16p16 *dst);

/**
 * @brief Return the type of the given fixed point matrix.
 *
 * @param m The fixed point matrix.
 * @return The type of the matrix.
 *
 * This function returns the type of the matrix @p m. No check is done
 * on @p m.
 */
EAPI Eina_Matrix_Type eina_matrix3_f16p16_type_get(const Eina_Matrix3_F16p16 *m);

/**
 * @}
 * @defgroup Eina_Matrix3 Matrices in floating point
 * @ingroup Eina_Basic
 * @brief Matrix definition and operations
 * @{
 */

/** Helper macro for printf formatting */
#define EINA_MATRIX3_FORMAT "g %g %g | %g %g %g | %g %g %g"
/** Helper macro for printf formatting arg */
#define EINA_MATRIX3_ARGS(m) (m)->xx, (m)->xy, (m)->xz, \
    (m)->yx, (m)->yy, (m)->yz,                          \
    (m)->zx, (m)->zy, (m)->zz


/**
 * Floating point matrix3 handler
 */
typedef struct _Eina_Matrix3 Eina_Matrix3;

struct _Eina_Matrix3
{
   double xx; /**< xx in x' = (x * xx) + (y * xy) + xz */
   double xy; /**< xy in x' = (x * xx) + (y * xy) + xz */
   double xz; /**< xz in x' = (x * xx) + (y * xy) + xz */

   double yx; /**< yx in y' = (x * yx) + (y * yy) + yz */
   double yy; /**< yy in y' = (x * yx) + (y * yy) + yz */
   double yz; /**< yz in y' = (x * yx) + (y * yy) + yz */

   double zx; /**< zx in z' = (x * zx) + (y * zy) + zz */
   double zy; /**< zy in z' = (x * zx) + (y * zy) + zz */
   double zz; /**< zz in z' = (x * zx) + (y * zy) + zz */
};

/**
 * @brief Return the type of the given floating point matrix.
 *
 * @param m The floating point matrix.
 * @return The type of the matrix.
 *
 * This function returns the type of the matrix @p m. No check is done
 * on @p m.
 */
EAPI Eina_Matrix_Type eina_matrix3_type_get(const Eina_Matrix3 *m);

/**
 * @brief Set the values of the coefficients of the given floating
 * point matrix.
 *
 * @param m The floating point matrix.
 * @param xx The first coefficient value.
 * @param xy The second coefficient value.
 * @param xz The third coefficient value.
 * @param yx The fourth coefficient value.
 * @param yy The fifth coefficient value.
 * @param yz The sixth coefficient value.
 * @param zx The seventh coefficient value.
 * @param zy The heighth coefficient value.
 * @param zz The nineth coefficient value.
 *
 * This function sets the values of the coefficients of the matrix
 * @p m. No check is done on @p m.
 *
 * @see eina_matrix3_values_get()
 */
EAPI void eina_matrix3_values_set(Eina_Matrix3 *m,
                                  double xx, double xy, double xz,
                                  double yx, double yy, double yz,
                                  double zx, double zy, double zz);

/**
 * @brief Get the values of the coefficients of the given floating
 * point matrix.
 *
 * @param m The floating point matrix.
 * @param xx The first coefficient value.
 * @param xy The second coefficient value.
 * @param xz The third coefficient value.
 * @param yx The fourth coefficient value.
 * @param yy The fifth coefficient value.
 * @param yz The sixth coefficient value.
 * @param zx The seventh coefficient value.
 * @param zy The heighth coefficient value.
 * @param zz The nineth coefficient value.
 *
 * This function gets the values of the coefficients of the matrix
 * @p m. No check is done on @p m.
 *
 * @see eina_matrix3_values_set()
 */
EAPI void eina_matrix3_values_get(const Eina_Matrix3 *m,
                                  double *xx, double *xy, double *xz,
                                  double *yx, double *yy, double *yz,
                                  double *zx, double *zy, double *zz);

/**
 * @brief Get the values of the coefficients of the given fixed
 * point matrix.
 *
 * @param m The fixed point matrix.
 * @param xx The first coefficient value.
 * @param xy The second coefficient value.
 * @param xz The third coefficient value.
 * @param yx The fourth coefficient value.
 * @param yy The fifth coefficient value.
 * @param yz The sixth coefficient value.
 * @param zx The seventh coefficient value.
 * @param zy The heighth coefficient value.
 * @param zz The nineth coefficient value.
 *
 * This function gets the values of the coefficients of the matrix
 * @p m. No check is done on @p m.
 *
 * @see eina_matrix3_values_set()
 */
EAPI void eina_matrix3_fixed_values_get(const Eina_Matrix3 *m,
                                        Eina_F16p16 *xx, Eina_F16p16 *xy, Eina_F16p16 *xz,
                                        Eina_F16p16 *yx, Eina_F16p16 *yy, Eina_F16p16 *yz,
                                        Eina_F16p16 *zx, Eina_F16p16 *zy, Eina_F16p16 *zz);

/**
 * @brief Transform the given floating point matrix to the given fixed
 * point matrix.
 *
 * @param m The floating point matrix.
 * @param fm The fixed point matrix.
 *
 * This function transforms the floating point matrix @p m to a fixed
 * point matrix and store the coefficients into the fixed point matrix
 * @p fm.
 */
EAPI void eina_matrix3_matrix3_f16p16_to(const Eina_Matrix3 *m,
                                         Eina_Matrix3_F16p16 *fm);

/**
 * @brief Check whether the two given matrices are equal or not.
 *
 * @param m1 The first matrix.
 * @param m2 The second matrix.
 * @return EINA_TRUE if the two matrices are equal, 0 otherwise.
 *
 * This function return EINA_TRUE if thematrices @p m1 and @p m2 are
 * equal, EINA_FALSE otherwise. No check is done on the matrices.
 */
EAPI Eina_Bool eina_matrix3_equal(const Eina_Matrix3 *m1, const Eina_Matrix3 *m2);
EAPI void eina_matrix3_compose(const Eina_Matrix3 *m1,
                               const Eina_Matrix3 *m2,
                               Eina_Matrix3 *dst);

/**
 * Set the matrix values for a translation
 * @param[in] m The matrix to set the translation values
 * @param[in] tx The X coordinate translate
 * @param[in] ty The Y coordinate translate
 */
EAPI void eina_matrix3_translate(Eina_Matrix3 *t, double tx, double ty);

/**
 * Set the matrix values for a scale
 * @param[in] m The matrix to set the scale values
 * @param[in] sx The X coordinate scale
 * @param[in] sy The Y coordinate scale
 */
EAPI void eina_matrix3_scale(Eina_Matrix3 *t, double sx, double sy);

/**
 * Set the matrix values for a rotation
 * @param[in] m The matrix to set the rotation values
 * @param[in] rad The radius to rotate the matrix
 */
EAPI void eina_matrix3_rotate(Eina_Matrix3 *t, double rad);

/**
 * @brief Set the given floating point matrix to the identity matrix.
 *
 * @param m The floating point matrix to set
 *
 * This function sets @p m to the identity matrix. No check is done on
 * @p m.
 */
EAPI void eina_matrix3_identity(Eina_Matrix3 *t);

/**
 * @brief Return the determinant of the given matrix.
 *
 * @param m The matrix.
 * @return The determinant.
 *
 * This function returns the determinant of the matrix @p m. No check
 * is done on @p m.
 */
EAPI double eina_matrix3_determinant(const Eina_Matrix3 *m);

/**
 * @brief Divide the given matrix by the given scalar.
 *
 * @param m The matrix.
 * @param scalar The scalar number.
 *
 * This function divides the matrix @p m by @p scalar. No check
 * is done on @p m.
 */
EAPI void eina_matrix3_divide(Eina_Matrix3 *m, double scalar);

/**
 * @brief Compute the inverse of the given matrix.
 *
 * @param m The matrix to inverse.
 * @param m2 The inverse matrix.
 *
 * This function inverse the matrix @p m and stores the result in
 * @p m2. No check is done on @p m or @p m2. If @p m can not be
 * invertible, then @p m2 is set to the identity matrix.
 */
EAPI void eina_matrix3_inverse(const Eina_Matrix3 *m, Eina_Matrix3 *m2);
EAPI void eina_matrix3_transpose(const Eina_Matrix3 *m, Eina_Matrix3 *a);
EAPI void eina_matrix3_cofactor(const Eina_Matrix3 *m, Eina_Matrix3 *a);
EAPI void eina_matrix3_adjoint(const Eina_Matrix3 *m, Eina_Matrix3 *a);

EAPI void eina_matrix3_point_transform(const Eina_Matrix3 *m,
                                       double x, double y,
                                       double *xr, double *yr);
EAPI void eina_matrix3_rectangle_transform(const Eina_Matrix3 *m,
                                           const Eina_Rectangle *r,
                                           const Eina_Quad *q);

/**
 * @brief Creates a projective matrix that maps a quadrangle to a quadrangle
 */
EAPI Eina_Bool eina_matrix3_quad_quad_map(Eina_Matrix3 *m,
                                          const Eina_Quad *src,
                                          const Eina_Quad *dst);
EAPI Eina_Bool eina_matrix3_square_quad_map(Eina_Matrix3 *m,
                                            const Eina_Quad *q);
EAPI Eina_Bool eina_matrix3_quad_square_map(Eina_Matrix3 *m,
                                            const Eina_Quad *q);

/**
 * @}
 */
#endif /*EINA_MATRIX3_H_*/

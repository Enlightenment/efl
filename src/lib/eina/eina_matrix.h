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
#ifndef EINA_MATRIX_H_
#define EINA_MATRIX_H_

#include "eina_quad.h"

/**
 * @file
 * @ender_group{Eina_Matrix_Type}
 * @ender_group{Eina_Matrix3_F16p16}
 * @ender_group{Eina_Matrix3}
 */

/**
 * @addtogroup Eina_Data_Types_Group Data Types
 *
 * @{
 */

/**
 * @defgroup Eina_Matrix_Group Matrix
 *
 * @{
 */

/**
 * @typedef Eina_Matrix_Type
 * Matrix types
 */
typedef enum _Eina_Matrix_Type
  {
    EINA_MATRIX_TYPE_IDENTITY, /**< Identity matrix type */
    EINA_MATRIX_TYPE_AFFINE, /**< Affine matrix type */
    EINA_MATRIX_TYPE_PROJECTIVE, /**< Projective matrix type */
    EINA_MATRIX_TYPE_LAST /**< The total number of matrix types */
  } Eina_Matrix_Type;

/**
 * @defgroup Eina_Matrix3_Group 3x3 Matrices in floating point
 *
 * @brief Matrix definition and operations.
 *
 * @{
 */

/**
 * @typedef Eina_Matrix3
 * Floating point matrix3 handler
 */
typedef struct _Eina_Matrix3 Eina_Matrix3;

/**
 * @struct _Eina_Matrix3
 * Floating point matrix3 handler
 */
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
 * @}
 */

/**
 * @defgroup Eina_Matrix4_Group 4x4 Matrices in floating point
 *
 * @brief Matrix definition and operations.
 *
 * @{
 */

/**
 * @typedef Eina_Matrix4
 * Floating point matrix4 handler
 */
typedef struct _Eina_Matrix4 Eina_Matrix4;

/**
 * @struct Eina_Matrix4
 * Floating point matrix4 handler
 */
struct _Eina_Matrix4
{
   double xx; /**< xx in x' = (x * xx) + (y * xy) + (z * xz) + xw */
   double xy; /**< xy in x' = (x * xx) + (y * xy) + (z * xz) + xw */
   double xz; /**< xz in x' = (x * xx) + (y * xy) + (z * xz) + xw */
   double xw; /**< xw in x' = (x * xx) + (y * xy) + (z * xz) + xw */

   double yx; /**< yx in y' = (x * yx) + (y * yy) + (z * yz) + yw */
   double yy; /**< yy in y' = (x * yx) + (y * yy) + (z * yz) + yw */
   double yz; /**< yz in y' = (x * yx) + (y * yy) + (z * yz) + yw */
   double yw; /**< yw in y' = (x * yx) + (y * yy) + (z * yz) + yw */

   double zx; /**< zx in z' = (x * zx) + (y * zy) + (z * zz) + zw */
   double zy; /**< zy in z' = (x * zx) + (y * zy) + (z * zz) + zw */
   double zz; /**< zz in z' = (x * zx) + (y * zy) + (z * zz) + zw */
   double zw; /**< zw in z' = (x * zx) + (y * zy) + (z * zz) + zw */

   double wx; /**< wx in w' = (x * wx) + (y * wy) + (z * wz) + ww */
   double wy; /**< wy in w' = (x * wx) + (y * wy) + (z * wz) + ww */
   double wz; /**< wz in w' = (x * wx) + (y * wy) + (z * wz) + ww */
   double ww; /**< ww in w' = (x * wx) + (y * wy) + (z * wz) + ww */
};

/**
 * @}
 */

/**
 * @defgroup Eina_Matrix3_F16p16_Group 3x3 Matrices in fixed point
 *
 * @brief Fixed point matrix operations.
 * @{
 */

/**
 * @typedef Eina_Matrix3_F16p16
 * Fixed point matrix3 handler
 */
typedef struct _Eina_Matrix3_F16p16 Eina_Matrix3_F16p16;

/**
 * @struct Eina_Matrix3_F16p16
 * Fixed point matrix3 handler
 */
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
 * @brief Sets the given fixed point matrix to the identity matrix.
 *
 * @param[in] m The fixed point matrix to set
 *
 * This function sets @p m to the identity matrix. No check is done on
 * @p m.
 *
 * @since 1.14
 */
EAPI void eina_matrix3_f16p16_identity(Eina_Matrix3_F16p16 *m);

/**
 * @brief Sets dst as the matrix multiplication (composition) of two
 * Eina_F16p16 matrices.
 *
 * In matrix multiplication, AB, the resultant matrix is created from
 * the rows of A multiplied against the columns of B and summed.  This
 * is not a cummutative; i.e.  AB != BA, so the ordering of arguments
 * @p m1 and @p m2 matters.
 *
 * @param[in] m1 The first matrix.  Must be non-NULL.
 * @param[in] m2 The second matrix.  Must be non-NULL.
 * @param[out] dst The matrix for the results to be placed.
 *
 * @since 1.14
 */
EAPI void eina_matrix3_f16p16_compose(const Eina_Matrix3_F16p16 *m1,
                                      const Eina_Matrix3_F16p16 *m2,
                                      Eina_Matrix3_F16p16 *dst);

/**
 * @brief Returns the type of the given fixed point matrix.
 *
 * @param[in] m The fixed point matrix.
 * @return The type of the matrix.
 *
 * This function returns the type of the matrix @p m. No check is done
 * on @p m.
 *
 * @since 1.14
 */
EAPI Eina_Matrix_Type eina_matrix3_f16p16_type_get(const Eina_Matrix3_F16p16 *m);

/**
 * @}
 */

/**
 * @addtogroup Eina_Matrix3_Group
 *
 * @{
 */

/** Helper macro for printf formatting */
#define EINA_MATRIX3_FORMAT "g %g %g | %g %g %g | %g %g %g"
/** Helper macro for printf formatting arg */
#define EINA_MATRIX3_ARGS(m) (m)->xx, (m)->xy, (m)->xz, \
    (m)->yx, (m)->yy, (m)->yz,                          \
    (m)->zx, (m)->zy, (m)->zz



/**
 * @brief Returns the type of the given floating point matrix.
 *
 * @param[in] m The floating point matrix.
 * @return The type of the matrix.
 *
 * This function returns the type of the matrix @p m. No check is done
 * on @p m.
 *
 * @since 1.14
 */
EAPI Eina_Matrix_Type eina_matrix3_type_get(const Eina_Matrix3 *m);

/**
 * @brief Sets the values of the coefficients of the given floating
 * point matrix.
 *
 * @param[out] m The floating point matrix.
 * @param[in] xx The first coefficient value.
 * @param[in] xy The second coefficient value.
 * @param[in] xz The third coefficient value.
 * @param[in] yx The fourth coefficient value.
 * @param[in] yy The fifth coefficient value.
 * @param[in] yz The sixth coefficient value.
 * @param[in] zx The seventh coefficient value.
 * @param[in] zy The eighth coefficient value.
 * @param[in] zz The nineth coefficient value.
 *
 * This function sets the values of the coefficients of the matrix
 * @p m. No check is done on @p m.
 *
 * @see eina_matrix3_values_get()
 *
 * @since 1.14
 */
EAPI void eina_matrix3_values_set(Eina_Matrix3 *m,
                                  double xx, double xy, double xz,
                                  double yx, double yy, double yz,
                                  double zx, double zy, double zz);

/**
 * @brief Gets the values of the coefficients of the given floating
 * point matrix.
 *
 * @param[in]  m  The floating point matrix.
 * @param[out] xx The first coefficient value.
 * @param[out] xy The second coefficient value.
 * @param[out] xz The third coefficient value.
 * @param[out] yx The fourth coefficient value.
 * @param[out] yy The fifth coefficient value.
 * @param[out] yz The sixth coefficient value.
 * @param[out] zx The seventh coefficient value.
 * @param[out] zy The eighth coefficient value.
 * @param[out] zz The nineth coefficient value.
 *
 * This function gets the values of the coefficients of the matrix
 * @p m. No check is done on @p m.
 *
 * @see eina_matrix3_values_set()
 *
 * @since 1.14
 */
EAPI void eina_matrix3_values_get(const Eina_Matrix3 *m,
                                  double *xx, double *xy, double *xz,
                                  double *yx, double *yy, double *yz,
                                  double *zx, double *zy, double *zz);

/**
 * @brief Gets the values of the coefficients of the given fixed
 * point matrix.
 *
 * @param[in]  m  The fixed point matrix.
 * @param[out] xx The first coefficient value.
 * @param[out] xy The second coefficient value.
 * @param[out] xz The third coefficient value.
 * @param[out] yx The fourth coefficient value.
 * @param[out] yy The fifth coefficient value.
 * @param[out] yz The sixth coefficient value.
 * @param[out] zx The seventh coefficient value.
 * @param[out] zy The eighth coefficient value.
 * @param[out] zz The nineth coefficient value.
 *
 * This function gets the values of the coefficients of the matrix
 * @p m. No check is done on @p m.
 *
 * @see eina_matrix3_values_set()
 *
 * @since 1.14
 */
EAPI void eina_matrix3_fixed_values_get(const Eina_Matrix3 *m,
                                        Eina_F16p16 *xx, Eina_F16p16 *xy, Eina_F16p16 *xz,
                                        Eina_F16p16 *yx, Eina_F16p16 *yy, Eina_F16p16 *yz,
                                        Eina_F16p16 *zx, Eina_F16p16 *zy, Eina_F16p16 *zz);

/**
 * @brief Transforms the given floating point matrix to the given fixed
 * point matrix.
 *
 * @param[in] m The floating point matrix.
 * @param[out] fm The fixed point matrix.
 *
 * This function transforms the floating point matrix @p m to a fixed
 * point matrix and store the coefficients into the fixed point matrix
 * @p fm.
 *
 * @since 1.14
 */
EAPI void eina_matrix3_matrix3_f16p16_to(const Eina_Matrix3 *m,
                                         Eina_Matrix3_F16p16 *fm);

/**
 * @brief Checks whether the two given matrices are equal or not.
 *
 * @param[in] m1 The first matrix.
 * @param[in] m2 The second matrix.
 * @return EINA_TRUE if the two matrices are equal, @c 0 otherwise.
 *
 * This function return EINA_TRUE if the matrices @p m1 and @p m2 are
 * equal, EINA_FALSE otherwise. No check is done on the matrices.
 *
 * @since 1.14
 */
EAPI Eina_Bool eina_matrix3_equal(const Eina_Matrix3 *m1, const Eina_Matrix3 *m2);

/**
 * @brief Sets dst as the matrix multiplication (composition) of two matrices.
 *
 * In matrix multiplication, AB, the resultant matrix is created from
 * the rows of A multiplied against the columns of B and summed.  This
 * is not a cummutative; i.e.  AB != BA, so the ordering of arguments
 * @p m1 and @p m2 matters.
 *
 * @param[in] m1 The first matrix.  Must be non-NULL.
 * @param[in] m2 The second matrix.  Must be non-NULL.
 * @param[out] dst The matrix for the results to be placed.
 *
 * @since 1.14
 */
EAPI void eina_matrix3_compose(const Eina_Matrix3 *m1,
                               const Eina_Matrix3 *m2,
                               Eina_Matrix3 *dst);

/**
 * @brief Sets the matrix values for a translation.
 * @param[out] m The matrix to set the translation values
 * @param[in] tx The X coordinate translate
 * @param[in] ty The Y coordinate translate
 *
 * @since 1.14
 */
EAPI void eina_matrix3_translate(Eina_Matrix3 *t, double tx, double ty);

/**
 * @brief Sets the matrix values for a scale.
 * @param[out] m The matrix to set the scale values
 * @param[in] sx The X coordinate scale
 * @param[in] sy The Y coordinate scale
 *
 * @since 1.14
 */
EAPI void eina_matrix3_scale(Eina_Matrix3 *t, double sx, double sy);

/**
 * @brief Sets the matrix values for a rotation.
 * @param[out] t The matrix to set the rotation values
 * @param[in] rad The radius to rotate the matrix
 *
 * @since 1.14
 */
EAPI void eina_matrix3_rotate(Eina_Matrix3 *t, double rad);

/**
 * @brief Sets the given floating point matrix to the identity matrix.
 *
 * @param[out] m The floating point matrix to set
 *
 * This function sets @p m to the identity matrix. No check is done on
 * @p m.
 *
 * @since 1.14
 */
EAPI void eina_matrix3_identity(Eina_Matrix3 *t);

/**
 * @brief Returns the determinant of the given matrix.
 *
 * @param m[in] The matrix.
 * @return The determinant.
 *
 * This function returns the determinant of the matrix @p m. No check
 * is done on @p m.
 *
 * @since 1.14
 */
EAPI double eina_matrix3_determinant(const Eina_Matrix3 *m);

/**
 * @brief Divides the given matrix by the given scalar.
 *
 * @param[in,out] m The matrix.
 * @param[in] scalar The scalar number.
 *
 * This function divides the matrix @p m by @p scalar. No check
 * is done on @p m.
 *
 * @since 1.14
 */
EAPI void eina_matrix3_divide(Eina_Matrix3 *m, double scalar);

/**
 * @brief Computes the inverse of the given matrix.
 *
 * @param[in] m The source matrix.
 * @param[out] m2 The inverse matrix.
 *
 * This function inverts the matrix @p m and stores the result in
 * @p m2. No check is done on @p m or @p m2. If @p m can not be
 * invertible, then @p m2 is set to the identity matrix.
 *
 * @since 1.14
 */
EAPI void eina_matrix3_inverse(const Eina_Matrix3 *m, Eina_Matrix3 *m2);

/**
 * @brief Computes the transpose of the given matrix.
 *
 * @param[in] m The source matrix.
 * @param[out] a The transposed matrix.
 *
 * This function transposes the matrix @p m and stores the result in
 * @p a. No check is done on @p m or @p a.  The transpose of a matrix
 * essentially flips a matrix over its diagonal.
 *
 * @since 1.14
 */
EAPI void eina_matrix3_transpose(const Eina_Matrix3 *m, Eina_Matrix3 *a);

/**
 * @brief Computes the cofactor of the given matrix.
 *
 * @param[in] m The source matrix.
 * @param[out] a The cofactored matrix.
 *
 * This function cofactors the matrix @p m and stores the result in
 * @p a.  No check is done on @p m or @p a.  The cofactor of a matrix3 at
 * row i, column j is computed by taking the determinant of the
 * submatrix formed by deleting the i-th row and j-th column, and then
 * multiplying by (-1)^(i+j).
 *
 * @since 1.14
 */
EAPI void eina_matrix3_cofactor(const Eina_Matrix3 *m, Eina_Matrix3 *a);

/**
 * @brief Computes the adjoint of the given matrix.
 *
 * @param[in] m The matrix to be adjointed.
 * @param[out] a The adjoint matrix.
 *
 * This function finds the adjoint of the matrix @p m and stores the
 * result in @p a. No check is done on @p m or @p a.  The adjoint of a
 * matrix3 is effectively the transpose of its cofactor.
 *
 * @since 1.14
 */
EAPI void eina_matrix3_adjoint(const Eina_Matrix3 *m, Eina_Matrix3 *a);

/**
 * @brief Computes the transform of a 2D point using the given matrix.
 *
 * @param[in] m The transformation matrix to apply.
 * @param[in] x The x point to be transformed.
 * @param[in] y The y point to be transformed.
 * @param[out] xr The transformed x point.
 * @param[out] yr The transformed y point.
 *
 * Applies the transformation matrix @p m to the point (x,y), and stores
 * the result in (*xr,*yr).  No check is done on @p m; @p xr and @p yr
 * must point to valid memory.  A fast-path is included for if the zx
 * and zy components of the matrix are zero.
 *
 * @since 1.14
 */
EAPI void eina_matrix3_point_transform(const Eina_Matrix3 *m,
                                       double x, double y,
                                       double *xr, double *yr);
/**
 * @brief Computes the transformation of a rectangle using the given matrix.
 *
 * @param m[in] The transformation matrix to apply.
 * @param r[in] The rectangle to be transformed.
 * @param q[out] The resultant transformed points.
 *
 * Performs a point transformation of each corner of the rectangle @p r,
 * and stores the result in the quadrangle @p q.  No checks are done on the
 * inputs, and @p q must point to valid memory.
 *
 * @since 1.14
 */
EAPI void eina_matrix3_rectangle_transform(const Eina_Matrix3 *m,
                                           const Eina_Rectangle *r,
                                           const Eina_Quad *q);

/**
 * @brief Creates a projective matrix that maps a quadrangle to a quadrangle.
 *
 * @param[out] m The transformation matrix to create.
 * @param[in] src The source quadrangle.
 * @param[in] dst The destination quadrangle.
 * @return @c EINA_TRUE if matrix could be successfully created, @c EINA_FALSE otherwise.
 *
 * Calculates a matrix @p m that can be used to transform from an arbitrary
 * source quadrangle @p src to another arbitrary quadrangle @p dst.
 */
EAPI Eina_Bool eina_matrix3_quad_quad_map(Eina_Matrix3 *m,
                                          const Eina_Quad *src,
                                          const Eina_Quad *dst);

/**
 * @brief Creates a matrix for unit-square to quad mapping.
 *
 * @param[out] m The transformation matrix to create.
 * @param[in]  q The source quadrangle.
 * @return @c EINA_TRUE if matrix could be successfully created, @c EINA_FALSE otherwise.
 *
 * Calculates a matrix @p m that can be used to transform an arbitrary
 * quadrangle @p q into a square.  If @p q is a parallelogram, then a
 * fast path affine transformation is used, otherwise it computes the
 * matrix using a full projective transformation operation.  No other
 * checks are done on @p m or @p q.
 *
 * @since 1.14
 */
EAPI Eina_Bool eina_matrix3_square_quad_map(Eina_Matrix3 *m,
                                            const Eina_Quad *q);

/**
 * @brief Creates a matrix for mapping squares to match quad.
 *
 * @param[out] m The transformation matrix to create.
 * @param[in] q The source quadrangle.
 * @return @c EINA_FALSE on successful transform creation, @c EINA_FALSE otherwise.
 *
 * Calculates a matrix @p m that can be used to transform a square to
 * fit a given quadrangle.  The created matrix will always have its zz
 * element equal to 1.0.  The @p m matrix and @p q quad must be valid
 * memory.
 *
 * @since 1.14
 */
EAPI Eina_Bool eina_matrix3_quad_square_map(Eina_Matrix3 *m,
                                            const Eina_Quad *q);

/**
 * @brief Sets array to matrix.
 *
 * @param[out] m The result matrix
 * @param[in] v The the array[9] for set
 *
 * Set to matrix first 9 elements from array
 *
 * @since 1.17
 */
EAPI void eina_matrix3_array_set(Eina_Matrix3 *m, const double *v);

/**
 * @brief Copies matrix.
 *
 * @param[out] dst The matrix copy
 * @param[in] src The matrix for copy.
 *
 * @since 1.16
 */
EAPI void eina_matrix3_copy(Eina_Matrix3 *dst, const Eina_Matrix3 *src);

/**
 * @brief Multiplies two matrices.
 *
 * @param[out] out The resulting matrix
 * @param[in] mat_a The first member of the multiplication
 * @param[in] mat_b The second member of the multiplication
 *
 * @since 1.17
 */
EAPI void eina_matrix3_multiply(Eina_Matrix3 *out, const Eina_Matrix3 *mat_a,
                                const Eina_Matrix3 *mat_b);

/**
 * @brief Multiplies two matrices.
 *
 * @param[out] out The resulting matrix
 * @param[in] mat_a The first member of the multiplication
 * @param[in] mat_b The second member of the multiplication
 *
 * @since 1.17
 */
EAPI void eina_matrix3_multiply_copy(Eina_Matrix3 *out, const Eina_Matrix3 *mat_a,
                                     const Eina_Matrix3 *mat_b);

/**
 * @brief Transforms scale of matrix.
 *
 * @param[out] out The resulting matrix
 * @param[in] s_x The scale value for x
 * @param[in] s_y The scale value for y
 *
 * @since 1.17
 */
EAPI void eina_matrix3_scale_transform_set(Eina_Matrix3 *out, double s_x, double s_y);

/**
 * @brief Transforms position of matrix.
 *
 * @param[out] out The resulting matrix
 * @param[in] p_x The position value for x
 * @param[in] p_y The position value for y
 *
 * @since 1.17
 */
EAPI void eina_matrix3_position_transform_set(Eina_Matrix3 *out, const double p_x,
                                              const double p_y);

/**
 * @brief Sets normal of the given matrix.
 *
 * @param[out] out The result mtrix of normal
 * @param[in] m The matrix
 *
 * @since 1.17
 */
EAPI void eina_normal3_matrix_get(Eina_Matrix3 *out, const Eina_Matrix4 *m);

/**
 * @brief Converts an Eina_Matrix3 into an Eina_Matrix4.
 *
 * @param[out] m3 The destination Eina_Matrix3.
 * @param[in] m4 The source Eina_Matrix4.
 *
 * @since 1.15
 */
EAPI void eina_matrix3_matrix4_to(Eina_Matrix4 *m4, const Eina_Matrix3 *m3);

/**
 * @}
 */

/**
 * @addtogroup Eina_Matrix4_Group
 *
 * @{
 */

/**
 * @brief Returns the type of the given floating point matrix.
 *
 * @param[in] m The floating point matrix.
 * @return The type of the matrix.
 *
 * This function returns the type of the matrix @p m. No check is done
 * on @p m.
 *
 * @since 1.15
 */
EAPI Eina_Matrix_Type eina_matrix4_type_get(const Eina_Matrix4 *m);

/**
 * @brief Sets the values of the coefficients of the given floating
 * point matrix.
 *
 * @param[out] m The floating point matrix.
 * @param[in] xx The first coefficient value.
 * @param[in] xy The second coefficient value.
 * @param[in] xz The third coefficient value.
 * @param[in] xw The fourth coefficient value.
 * @param[in] yx The fifth coefficient value.
 * @param[in] yy The sixth coefficient value.
 * @param[in] yz The seventh coefficient value.
 * @param[in] yw The eighth coefficient value.
 * @param[in] zx The nineth coefficient value.
 * @param[in] zy The tenth coefficient value.
 * @param[in] zz The eleventh coefficient value.
 * @param[in] zw The twelfth coefficient value.
 * @param[in] wx The thirteenth coefficient value.
 * @param[in] wy The fourteenth coefficient value.
 * @param[in] wz The fifteenth coefficient value.
 * @param[in] ww The sizteenth coefficient value.
 *
 * This function sets the values of the coefficients of the matrix
 * @p m. No check is done on @p m.
 *
 * @see eina_matrix4_values_get()
 *
 * @since 1.15
 */
EAPI void eina_matrix4_values_set(Eina_Matrix4 *m,
                                  double xx, double xy, double xz, double xw,
                                  double yx, double yy, double yz, double yw,
                                  double zx, double zy, double zz, double zw,
                                  double wx, double wy, double wz, double ww);

/**
 * @brief Gets the values of the coefficients of the given floating
 * point matrix.
 *
 * @param[in] m The floating point matrix.
 * @param[out] xx The first coefficient value.
 * @param[out] xy The second coefficient value.
 * @param[out] xz The third coefficient value.
 * @param[out] xw The fourth coefficient value.
 * @param[out] yx The fifth coefficient value.
 * @param[out] yy The sixth coefficient value.
 * @param[out] yz The seventh coefficient value.
 * @param[out] yw The eighth coefficient value.
 * @param[out] zx The nineth coefficient value.
 * @param[out] zy The tenth coefficient value.
 * @param[out] zz The eleventh coefficient value.
 * @param[out] zw The twelfth coefficient value.
 * @param[out] wx The thirteenth coefficient value.
 * @param[out] wy The fourteenth coefficient value.
 * @param[out] wz The fifteenth coefficient value.
 * @param[out] ww The sizteenth coefficient value.
 *
 * This function gets the values of the coefficients of the matrix
 * @p m. No check is done on @p m.
 *
 * @see eina_matrix4_values_set()
 *
 * @since 1.15
 */
EAPI void eina_matrix4_values_get(const Eina_Matrix4 *m,
                                  double *xx, double *xy, double *xz, double *xw,
                                  double *yx, double *yy, double *yz, double *yw,
                                  double *zx, double *zy, double *zz, double *zw,
                                  double *wx, double *wy, double *wz, double *ww);

/**
 * @brief Returns the determinant of the given matrix.
 *
 * @param[in] m The matrix.
 * @return The determinant.
 *
 * This function returns the determinant of the matrix @p m. No check
 * is done on @p m.
 *
 * @since 1.16
 */
EAPI double eina_matrix4_determinant(const Eina_Matrix4 *m);

/**
 * @brief Returns the determinant of the given matrix.
 *
 * @param[out] out The normalized matrix
 * @param[in] in The matrix.
 * @return The determinant.
 *
 * This function returns the determinant of the matrix @p in. No check
 * is done on @p in.
 *
 * @since 1.16
 */
EAPI Eina_Bool eina_matrix4_normalized(Eina_Matrix4 *out,
                                       const Eina_Matrix4 *in);

/**
 * @brief Returns the inverse of the given matrix.
 *
 * @param[out] out The inverse matrix
 * @param[in] in The matrix.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * @since 1.16
 */
EAPI Eina_Bool eina_matrix4_inverse(Eina_Matrix4 *out, const Eina_Matrix4 *in);

/**
 * @brief Returns the transpose of the given matrix.
 *
 * @param[out] out The transpose matrix
 * @param[in] in The matrix.
 *
 * Just going to swap row and column.
 *
 * @since 1.16
 */
EAPI void eina_matrix4_transpose(Eina_Matrix4 *out, const Eina_Matrix4 *in);

/**
 * @brief Converts an Eina_Matrix4 into an Eina_Matrix3.
 *
 * @param[out] m3 The destination Eina_Matrix3.
 * @param[in] m4 The source Eina_Matrix4.
 *
 * @since 1.15
 */
EAPI void eina_matrix4_matrix3_to(Eina_Matrix3 *m3, const Eina_Matrix4 *m4);

/**
 * @brief Sets an identity matrix
 *
 * @param[out] out The matrix to set
 *
 * @since 1.16
 */
EAPI void eina_matrix4_identity(Eina_Matrix4 *out);

/**
 * @brief Multiplies two matrix.
 *
 * @param[out] out The resulting matrix
 * @param[in] mat_a The first member of the multiplication
 * @param[in] mat_b The second member of the multiplication
 *
 * @since 1.17
 */
EAPI void eina_matrix4_multiply_copy(Eina_Matrix4 *out,
                                     const Eina_Matrix4 *mat_a, const Eina_Matrix4 *mat_b);

/**
 * @brief Sets array to matrix.
 *
 * @param[out] m The result matrix
 * @param[in] v The the array[16] for set
 *
 * Set to matrix first 16 elements from array
 *
 * @since 1.17
 */
EAPI void eina_matrix4_array_set(Eina_Matrix4 *m, const double *v);

/**
 * @brief Copies matrix.
 *
 * @param[out] dst The matrix copy
 * @param[in] src The matrix for copy.
 *
 * @since 1.17
 */
EAPI void eina_matrix4_copy(Eina_Matrix4 *dst, const Eina_Matrix4 *src);

/**
 * @brief Multiplies two matrices with check.
 *
 * @param[out] out The resulting matrix
 * @param[in] mat_a The first member of the multiplication
 * @param[in] mat_b The second member of the multiplication
 *
 * @since 1.17
 */
EAPI void eina_matrix4_multiply(Eina_Matrix4 *out, const Eina_Matrix4 *mat_a,
                                const Eina_Matrix4 *mat_b);

/**
 * @brief Sets orthogonality matrix.
 *
 * @param[out] m The resulting matrix
 * @param[in] right The right value
 * @param[in] left The left value
 * @param[in] bottom The bottom value
 * @param[in] top The top value
 * @param[in] dnear The dnear value
 * @param[in] dfar The dfar value
 *
 * @since 1.17
 */
EAPI void eina_matrix4_ortho_set(Eina_Matrix4 *m,
                                 double left, double right, double bottom, double top,
                                 double dnear, double dfar);

/**
 * @}
 */


/**
 * @defgroup Eina_Matrix2_Group 2x2 Matrices in floating point
 *
 * @brief Matrix definition and operations
 * @{
 */

/**
 * @typedef Eina_Matrix2
 * Floating point matrix2 handler
 */
typedef struct _Eina_Matrix2 Eina_Matrix2;

/**
 * @struct Eina_Matrix2
 * Floating point matrix2 handler
 */
struct _Eina_Matrix2
{
   double xx;
   double xy;

   double yx;
   double yy;
};

/**
 * @brief Sets the values of the coefficients of the given floating
 * point matrix.
 *
 * @param[out] m The floating point matrix.
 * @param[in] xx The first coefficient value.
 * @param[in] xy The second coefficient value.
 * @param[in] yx The fourth coefficient value.
 * @param[in] yy The fifth coefficient value.
 *
 * This function sets the values of the coefficients of the matrix
 * @p m. No check is done on @p m.
 *
 * @see eina_matrix2_values_get()
 *
 * @since 1.17
 */
EAPI void eina_matrix2_values_set(Eina_Matrix2 *m, double xx, double xy,
                    double yx, double yy);

/**
 * @brief Gets the values of the coefficients of the given floating
 * point matrix.
 *
 * @param[out] m The floating point matrix.
 * @param[in] xx The first coefficient value.
 * @param[in] xy The second coefficient value.
 * @param[in] yx The fourth coefficient value.
 * @param[in] yy The fifth coefficient value.
 *
 * This function gets the values of the coefficients of the matrix
 * @p m. No check is done on @p m.
 *
 * @see eina_matrix2_values_set()
 *
 * @since 1.17
 */
EAPI void eina_matrix2_values_get(const Eina_Matrix2 *m, double *xx, double *xy,
                    double *yx, double *yy);

/**
 * @brief Computes the inverse with check of the given matrix.
 *
 * @param[out] out The matrix to inverse.
 * @param[in] mat The inverse matrix.
 *
 * This function inverse the matrix @p out and stores the result in
 * @p mat. No check is done on @p out or @p mat. If @p out can not be
 * invertible, then @p mat is set to the identity matrix.
 *
 * @since 1.17
 */
EAPI void eina_matrix2_inverse(Eina_Matrix2 *out, const Eina_Matrix2 *mat);

/**
 * @brief Sets the given floating point matrix to the identity matrix.
 *
 * @param[out] m The floating point matrix to set
 *
 * This function sets @p m to the identity matrix. No check is done on
 * @p m.
 *
 * @since 1.17
 */
EAPI void eina_matrix2_identity(Eina_Matrix2 *m);

/**
 * @brief Sets array to matrix.
 *
 * @param[out] m The result matrix
 * @param[in] v The the array[4] for set
 *
 * Set to matrix first 4 elements from array
 *
 * @since 1.17
 */
EAPI void eina_matrix2_array_set(Eina_Matrix2 *m, const double *v);

/**
 * @brief Copies matrix.
 *
 * @param[out] dst The matrix copy
 * @param[in] src The matrix for copy.
 *
 * @since 1.17
 */
EAPI void eina_matrix2_copy(Eina_Matrix2 *dst, const Eina_Matrix2 *src);

/**
 * @brief Multiplies two matrices.
 *
 * @param[out] out The resulting matrix
 * @param[in] mat_a The first member of the multiplication
 * @param[in] mat_b The second member of the multiplication
 *
 * @since 1.17
 */
EAPI void eina_matrix2_multiply(Eina_Matrix2 *out, const Eina_Matrix2 *mat_a,
                                const Eina_Matrix2 *mat_b);

/**
 * @brief Multiplies two matrices with check.
 *
 * @param[out] out The resulting matrix
 * @param[in] mat_a The first member of the multiplication
 * @param[in] mat_b The second member of the multiplication
 *
 * @since 1.17
 */
EAPI void eina_matrix2_multiply_copy(Eina_Matrix2 *out, const Eina_Matrix2 *mat_a,
                    const Eina_Matrix2 *mat_b);

/**
 * @brief Returns the type of the given floating point matrix.
 *
 * @param[in] m The floating point matrix.
 * @return The type of the matrix.
 *
 * This function returns the type of the matrix @p m. No check is done
 * on @p m.
 *
 * @since 1.17
 */
EAPI Eina_Matrix_Type eina_matrix2_type_get(const Eina_Matrix2 *m);

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#endif /*EINA_MATRIX_H_*/

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
 *
 * @since 1.14
 */
EAPI void eina_matrix3_f16p16_identity(Eina_Matrix3_F16p16 *m);

/**
 * @since 1.14
 */
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
 *
 * @since 1.14
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
 *
 * @since 1.14
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
 *
 * @since 1.14
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
 *
 * @since 1.14
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
 *
 * @since 1.14
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
 *
 * @since 1.14
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
 *
 * @since 1.14
 */
EAPI Eina_Bool eina_matrix3_equal(const Eina_Matrix3 *m1, const Eina_Matrix3 *m2);

/**
 * @since 1.14
 */
EAPI void eina_matrix3_compose(const Eina_Matrix3 *m1,
                               const Eina_Matrix3 *m2,
                               Eina_Matrix3 *dst);

/**
 * Set the matrix values for a translation
 * @param[in] m The matrix to set the translation values
 * @param[in] tx The X coordinate translate
 * @param[in] ty The Y coordinate translate
 *
 * @since 1.14
 */
EAPI void eina_matrix3_translate(Eina_Matrix3 *t, double tx, double ty);

/**
 * Set the matrix values for a scale
 * @param[in] m The matrix to set the scale values
 * @param[in] sx The X coordinate scale
 * @param[in] sy The Y coordinate scale
 *
 * @since 1.14
 */
EAPI void eina_matrix3_scale(Eina_Matrix3 *t, double sx, double sy);

/**
 * Set the matrix values for a rotation
 * @param[in] m The matrix to set the rotation values
 * @param[in] rad The radius to rotate the matrix
 *
 * @since 1.14
 */
EAPI void eina_matrix3_rotate(Eina_Matrix3 *t, double rad);

/**
 * @brief Set the given floating point matrix to the identity matrix.
 *
 * @param m The floating point matrix to set
 *
 * This function sets @p m to the identity matrix. No check is done on
 * @p m.
 *
 * @since 1.14
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
 *
 * @since 1.14
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
 *
 * @since 1.14
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
 *
 * @since 1.14
 */
EAPI void eina_matrix3_inverse(const Eina_Matrix3 *m, Eina_Matrix3 *m2);

/**
 * @since 1.14
 */
EAPI void eina_matrix3_transpose(const Eina_Matrix3 *m, Eina_Matrix3 *a);

/**
 * @since 1.14
 */
EAPI void eina_matrix3_cofactor(const Eina_Matrix3 *m, Eina_Matrix3 *a);

/**
 * @since 1.14
 */
EAPI void eina_matrix3_adjoint(const Eina_Matrix3 *m, Eina_Matrix3 *a);

/**
 * @since 1.14
 */
EAPI void eina_matrix3_point_transform(const Eina_Matrix3 *m,
                                       double x, double y,
                                       double *xr, double *yr);
/**
 * @since 1.14
 */
EAPI void eina_matrix3_rectangle_transform(const Eina_Matrix3 *m,
                                           const Eina_Rectangle *r,
                                           const Eina_Quad *q);

/**
 * @brief Creates a projective matrix that maps a quadrangle to a quadrangle
 */
EAPI Eina_Bool eina_matrix3_quad_quad_map(Eina_Matrix3 *m,
                                          const Eina_Quad *src,
                                          const Eina_Quad *dst);

/**
 * @since 1.14
 */
EAPI Eina_Bool eina_matrix3_square_quad_map(Eina_Matrix3 *m,
                                            const Eina_Quad *q);

/**
 * @since 1.14
 */
EAPI Eina_Bool eina_matrix3_quad_square_map(Eina_Matrix3 *m,
                                            const Eina_Quad *q);

/**
 * @}
 * @defgroup Eina_Matrix4 Matrices in floating point
 * @ingroup Eina_Basic
 * @brief Matrix definition and operations
 * @{
 */

typedef struct _Eina_Matrix4 Eina_Matrix4;
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
 * @defgroup Eina_Matrix2 Matrices in floating point
 * @ingroup Eina_Basic
 * @brief Matrix definition and operations
 * @{
 */

typedef struct _Eina_Matrix2 Eina_Matrix2;
struct _Eina_Matrix2
{
   double xx;
   double xy;

   double yx;
   double yy;
};
/**
 * @brief Return the type of the given floating point matrix.
 *
 * @param m The floating point matrix.
 * @return The type of the matrix.
 *
 * This function returns the type of the matrix @p m. No check is done
 * on @p m.
 *
 * @since 1.15
 */
EAPI Eina_Matrix_Type eina_matrix4_type_get(const Eina_Matrix4 *m);

/**
 * @brief Set the values of the coefficients of the given floating
 * point matrix.
 *
 * @param m The floating point matrix.
 * @param xx The first coefficient value.
 * @param xy The second coefficient value.
 * @param xz The third coefficient value.
 * @param xw The fourth coefficient value.
 * @param yx The fifth coefficient value.
 * @param yy The sixth coefficient value.
 * @param yz The seventh coefficient value.
 * @param yw The heighth coefficient value.
 * @param zx The nineth coefficient value.
 * @param zy The tenth coefficient value.
 * @param zz The eleventh coefficient value.
 * @param zw The twelfth coefficient value.
 * @param wx The thirteenth coefficient value.
 * @param wy The fourteenth coefficient value.
 * @param wz The fifteenth coefficient value.
 * @param ww The sizteenth coefficient value.
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
 * @brief Get the values of the coefficients of the given floating
 * point matrix.
 *
 * @param m The floating point matrix.
 * @param xx The first coefficient value.
 * @param xy The second coefficient value.
 * @param xz The third coefficient value.
 * @param xw The fourth coefficient value.
 * @param yx The fifth coefficient value.
 * @param yy The sixth coefficient value.
 * @param yz The seventh coefficient value.
 * @param yw The heighth coefficient value.
 * @param zx The nineth coefficient value.
 * @param zy The tenth coefficient value.
 * @param zz The eleventh coefficient value.
 * @param zw The twelfth coefficient value.
 * @param wx The thirteenth coefficient value.
 * @param wy The fourteenth coefficient value.
 * @param wz The fifteenth coefficient value.
 * @param ww The sizteenth coefficient value.
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
 * @brief Return the determinant of the given matrix.
 *
 * @param m The matrix.
 * @return The determinant.
 *
 * This function returns the determinant of the matrix @p m. No check
 * is done on @p m.
 *
 * @since 1.16
 */
EAPI double eina_matrix4_determinant(const Eina_Matrix4 *m);

/**
 * @brief Return the determinant of the given matrix.
 *
 * @param m The matrix.
 * @return The determinant.
 *
 * This function returns the determinant of the matrix @p m. No check
 * is done on @p m.
 *
 * @since 1.16
 */
EAPI Eina_Bool eina_matrix4_normalized(Eina_Matrix4 *out,
                                       const Eina_Matrix4 *in);

/**
 * @brief Return the inverse of the given matrix.
 *
 * @param out The inverse matrix
 * @param in The matrix.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * @since 1.16
 */
EAPI Eina_Bool eina_matrix4_inverse(Eina_Matrix4 *out, const Eina_Matrix4 *in);

/**
 * @brief Return the transpose of the given matrix.
 *
 * @param out The transpose matrix
 * @param in The matrix.
 *
 * Just going to swap row and column.
 *
 * @since 1.16
 */
EAPI void eina_matrix4_transpose(Eina_Matrix4 *out, const Eina_Matrix4 *in);

/**
 * @brief Convert an Eina_Matrix4 into an Eina_Matrix3.
 *
 * @param m3 The destination Eina_Matrix3.
 * @param m4 The source Eina_Matrix4.
 *
 * @since 1.15
 */
EAPI void eina_matrix4_matrix3_to(Eina_Matrix3 *m3, const Eina_Matrix4 *m4);

/**
 * @brief Convert an Eina_Matrix3 into an Eina_Matrix4.
 *
 * @param m3 The destination Eina_Matrix3.
 * @param m4 The source Eina_Matrix4.
 *
 * @since 1.15
 */
EAPI void eina_matrix3_matrix4_to(Eina_Matrix4 *m4, const Eina_Matrix3 *m3);

/**
 * @brief Set an identity matrix
 *
 * @param out The matrix to set
 *
 * @since 1.16
 */
EAPI void eina_matrix4_identity(Eina_Matrix4 *out);

/**
 * @brief Multiply two matrix
 *
 * @param out The resulting matrix
 * @param a The first member of the multiplication
 * @param b The second member of the multiplication
 *
 * @since 1.17
 */
EAPI void eina_matrix4_multiply_copy(Eina_Matrix4 *out,
                                     const Eina_Matrix4 *mat_a, const Eina_Matrix4 *mat_b);

/**
 * @brief Set array to matrix.
 *
 * @param m The result matrix
 * @param v The the array[16] for set
 *
 * Set to matrix first 16 elements from array
 *
 * @since 1.17
 */
EAPI void eina_matrix4_array_set(Eina_Matrix4 *m, const double *v);

/**
 * @brief Copy matrix.
 *
 * @param dst The matrix copy
 * @param src The matrix for copy.
 *
 * @since 1.17
 */
EAPI void eina_matrix4_copy(Eina_Matrix4 *dst, const Eina_Matrix4 *src);

/**
 * @brief Multiply two matrix with check
 *
 * @param out The resulting matrix
 * @param a The first member of the multiplication
 * @param b The second member of the multiplication
 *
 * @since 1.17
 */
EAPI void eina_matrix4_multiply(Eina_Matrix4 *out, const Eina_Matrix4 *mat_a,
                                const Eina_Matrix4 *mat_b);

/**
 * @brief Set orthogonality matrix
 *
 * @param m The resulting matrix
 * @param right The right value
 * @param left The left value
 * @param bottom The bottom value
 * @param top The top value
 * @param dneat The dnear value
 * @param dfar The dfar value
 *
 * @since 1.17
 */
EAPI void eina_matrix4_ortho_set(Eina_Matrix4 *m,
                                 double left, double right, double bottom, double top,
                                 double dnear, double dfar);

/**
 * @brief Set array to matrix.
 *
 * @param m The result matrix
 * @param v The the array[9] for set
 *
 * Set to matrix first 9 elements from array
 *
 * @since 1.17
 */
EAPI void eina_matrix3_array_set(Eina_Matrix3 *m, const double *v);

/**
 * @brief Copy matrix.
 *
 * @param dst The matrix copy
 * @param src The matrix for copy.
 *
 * @since 1.16
 */
EAPI void eina_matrix3_copy(Eina_Matrix3 *dst, const Eina_Matrix3 *src);

/*
 * @brief Multiply two matrix
 *
 * @param out The resulting matrix
 * @param a The first member of the multiplication
 * @param b The second member of the multiplication
 *
 * @since 1.17
 */
EAPI void eina_matrix3_multiply(Eina_Matrix3 *out, const Eina_Matrix3 *mat_a,
                                const Eina_Matrix3 *mat_b);

/**
 * @brief Multiply two matrix
 *
 * @param out The resulting matrix
 * @param a The first member of the multiplication
 * @param b The second member of the multiplication
 *
 * @since 1.17
 */
EAPI void eina_matrix3_multiply_copy(Eina_Matrix3 *out, const Eina_Matrix3 *mat_a,
                                     const Eina_Matrix3 *mat_b);

/**
 * @brief Transform scale of matrix
 *
 * @param out The resulting matrix
 * @param s_x The scale value for x
 * @param s_y The scale value for y
 *
 * @since 1.17
 */
EAPI void eina_matrix3_scale_transform_set(Eina_Matrix3 *out, double s_x, double s_y);

/**
 * @brief Transform position of matrix
 *
 * @param out The resulting matrix
 * @param p_x The position value for x
 * @param p_y The position value for y
 *
 * @since 1.17
 */
EAPI void eina_matrix3_position_transform_set(Eina_Matrix3 *out, const double p_x,
                                              const double p_y);

/**
 * @brief Set normal of the given matrix.
 *
 * @param out The result mtrix of normal
 * @param m The matrix
 *
 * @since 1.17
 */
EAPI void eina_normal3_matrix_get(Eina_Matrix3 *out, const Eina_Matrix4 *m);

/**
 * @brief Set the values of the coefficients of the given floating
 * point matrix.
 *
 * @param m The floating point matrix.
 * @param xx The first coefficient value.
 * @param xy The second coefficient value.
 * @param yx The fourth coefficient value.
 * @param yy The fifth coefficient value.
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
 * @brief Get the values of the coefficients of the given floating
 * point matrix.
 *
 * @param m The floating point matrix.
 * @param xx The first coefficient value.
 * @param xy The second coefficient value.
 * @param yx The fourth coefficient value.
 * @param yy The fifth coefficient value.
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
 * @brief Compute the inverse with check of the given matrix.
 *
 * @param m The matrix to inverse.
 * @param m2 The inverse matrix.
 *
 * This function inverse the matrix @p m and stores the result in
 * @p m2. No check is done on @p m or @p m2. If @p m can not be
 * invertible, then @p m2 is set to the identity matrix.
 *
 * @since 1.17
 */
EAPI void eina_matrix2_inverse(Eina_Matrix2 *out, const Eina_Matrix2 *mat);

/**
 * @brief Set the given floating point matrix to the identity matrix.
 *
 * @param m The floating point matrix to set
 *
 * This function sets @p m to the identity matrix. No check is done on
 * @p m.
 *
 * @since 1.17
 */
EAPI void eina_matrix2_identity(Eina_Matrix2 *m);

/**
 * @brief Set array to matrix.
 *
 * @param m The rsult matrix
 * @param v The the array[4] for set
 *
 * Set to matrix first 4 elements from array
 *
 * @since 1.17
 */
EAPI void eina_matrix2_array_set(Eina_Matrix2 *m, const double *v);

/**
 * @brief Copy matrix.
 *
 * @param dst The matrix copy
 * @param src The matrix for copy.
 *
 * @since 1.17
 */
EAPI void eina_matrix2_copy(Eina_Matrix2 *dst, const Eina_Matrix2 *src);

/**
 * @brief Multiply two matrix
 *
 * @param out The resulting matrix
 * @param a The first member of the multiplication
 * @param b The second member of the multiplication
 *
 * @since 1.17
 */
EAPI void eina_matrix2_multiply(Eina_Matrix2 *out, const Eina_Matrix2 *mat_a,
                                const Eina_Matrix2 *mat_b);

/**
 * @brief Multiply two matrix with check
 *
 * @param out The resulting matrix
 * @param a The first member of the multiplication
 * @param b The second member of the multiplication
 *
 * @since 1.17
 */
EAPI void eina_matrix2_multiply_copy(Eina_Matrix2 *out, const Eina_Matrix2 *mat_a,
                    const Eina_Matrix2 *mat_b);

/**
 * @brief Return the type of the given floating point matrix.
 *
 * @param m The floating point matrix.
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
#endif /*EINA_MATRIX3_H_*/

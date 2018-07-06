/* EINA - EFL data type library
 * Copyright (C) 2015 Subhransu Mohanty
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

#ifndef EINA_BEZIER_H
#define EINA_BEZIER_H

/**
 * Floating point cubic bezier curve
 */
typedef struct _Eina_Bezier Eina_Bezier;

struct _Eina_Bezier
{
   struct {
      double x; /**< x coordinate of start point */
      double y; /**< y coordinate of start point */
   } start;

   struct {
      double x; /**< x coordinate of 1st control point */
      double y; /**< y coordinate of 1st control point */
   } ctrl_start;

   struct {
      double x; /**< coordinate of 2nd control point */
      double y; /**< y coordinate of 2nd control point */
   } ctrl_end;

   struct {
      double x; /**< x coordinate of end point */
      double y; /**< y coordinate of end point */
   } end;
};

/**
 * @brief Sets the values of the points of the given floating
 * point cubic bezier curve.
 *
 * @param b[out] The floating point bezier.
 * @param start_x[in] The x coordinate of the start point.
 * @param start_y[in] The y coordinate of the start point.
 * @param ctrl_start_x[in] The x coordinate of the 1st control point.
 * @param ctrl_start_y[in] The y coordinate of the 1st control point.
 * @param ctrl_end_x[in] The X coordinate of the 2nd control point.
 * @param ctrl_end_y[in] The Y coordinate of the 2nd control point.
 * @param end_x[in] The X coordinate of the end point.
 * @param end_y[in] The Y coordinate of the end point.
 *
 * @p b. No check is done on @p b.
 * @since 1.16
 */
EAPI void eina_bezier_values_set(Eina_Bezier *b, double start_x, double start_y, double ctrl_start_x, double ctrl_start_y, double ctrl_end_x, double ctrl_end_y, double end_x, double end_y) EINA_ARG_NONNULL(1);

/**
 * @brief Gets the values of the points of the given floating
 * point cubic bezier curve.
 *
 * @param[in] b The floating point bezier.
 * @param[out] start_x x coordinate of start point.
 * @param[out] start_y y coordinate of start point.
 * @param[out] ctrl_start_x x coordinate of 1st control point.
 * @param[out] ctrl_start_y y coordinate of 1st control point.
 * @param[out] ctrl_end_x x coordinate of 2nd control point.
 * @param[out] ctrl_end_y y coordinate of 2nd control point.
 * @param[out] end_x x coordinate of end point.
 * @param[out] end_y y coordinate of end point.
 *
 * @p b. No check is done on @p b.
 * @since 1.16
 */
EAPI void eina_bezier_values_get(const Eina_Bezier *b, double *start_x, double *start_y, double *ctrl_start_x, double *ctrl_start_y, double *ctrl_end_x, double *ctrl_end_y, double *end_x, double *end_y) EINA_ARG_NONNULL(1);

/**
 * @brief Calculates the approximate length of the given floating point
 * cubic bezier curve.
 *
 * @param[in] b The floating point bezier.
 * @return The bezier's length.
 *
 * The curve length is approximated using the Alpha max plus beta min algorithm,
 * which is designed to give fast results with a maximum error of less than 7%
 * compared with the correct value.
 *
 * No check is done on @p b.
 * @since 1.16
 */
EAPI double eina_bezier_length_get(const Eina_Bezier *b) EINA_ARG_NONNULL(1);

/**
 * @brief Returns the relative position on a bezier at a given length.
 *
 * @param[in] b The floating point bezier.
 * @param[in] len The length along the bezier curve.
 * @return The relative position from 0.0 to 1.0.
 *
 * Calculates the proportional location on @p b as a number from 0.0 to
 * 1.0 that corresponds to a distance @p len along it.  Returns 1.0 if
 * @p len is equal or greater than the bezier's length.
 *
 * No check is done on @p b.
 *
 * @since 1.16
 */
EAPI double eina_bezier_t_at(const Eina_Bezier *b, double len) EINA_ARG_NONNULL(1);

/**
 * @brief Gets the point on the bezier curve at position t.
 *
 * @param[in] b The floating point bezier.
 * @param[in] t The floating point position between 0.0 and 1.0.
 * @param[out] px The corresponding point's X coordinate.
 * @param[out] py The corresponding point's Y coordinate.
 *
 * No check is done on @p b.
 * @since 1.16
 */
EAPI void eina_bezier_point_at(const Eina_Bezier *b, double t, double *px, double *py) EINA_ARG_NONNULL(1);

/**
 * @brief Determines the slope of the bezier at a given position.
 *
 * @param[in] b The floating point bezier.
 * @param[out] t The position along the bezier between 0.0 and 1.0.
 *
 * No check is done on @p b.
 * @since 1.16
 */
EAPI double eina_bezier_angle_at(const Eina_Bezier *b, double t) EINA_ARG_NONNULL(1);

/**
 * @brief Splits the bezier at a given length.
 *
 * @param[in] b The floating point bezier.
 * @param[in] len The length along the bezier to make the split.
 * @param[out] left The resultant split's left portion of the bezier.
 * @param[out] right The resultant split's right portion of the bezier.
 *
 * No check is done on @p b.
 * @since 1.16
 */
EAPI void eina_bezier_split_at_length(const Eina_Bezier *b, double len, Eina_Bezier *left, Eina_Bezier *right) EINA_ARG_NONNULL(1);

/**
 * @brief Calculates the bounding box for the bezier.
 *
 * @param[in] b The floating point bezier.
 * @param[out] x The X coordinate of the bounding box.
 * @param[out] y The Y coordinate of the bounding box.
 * @param[out] w The width of the bounding box.
 * @param[out] h The height of the bounding box.
 *
 * No check is done on @p b.
 * @since 1.17
 */
EAPI void eina_bezier_bounds_get(const Eina_Bezier *b, double *x, double *y, double *w, double *h) EINA_ARG_NONNULL(1);

/**
 * @brief Finds the bezier between the given interval.
 *
 * @param[in] b The floating point bezier.
 * @param[in] t0 The start of the interval.
 * @param[in] t1 The end of the interval.
 * @param[out] result The resulting bezier.
 *
 * No check is done on @p b.
 * @since 1.17
 */
EAPI void eina_bezier_on_interval(Eina_Bezier *b, double t0, double t1, Eina_Bezier *result);

#endif // EINA_BEZIER_H

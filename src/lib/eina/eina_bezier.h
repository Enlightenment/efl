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
 * @brief Set the values of the points of the given floating
 * point cubic bezier curve.
 *
 * @param b The floating point bezier.
 * @param start_x x coordinate of start point.
 * @param start_y y coordinate of start point.
 * @param ctrl_start_x x coordinate of 1st control point.
 * @param ctrl_start_y y coordinate of 1st control point.
 * @param ctrl_end_x x coordinate of 2nd control point.
 * @param ctrl_end_y y coordinate of 2nd control point.
 * @param end_x x coordinate of end point.
 * @param end_y y coordinate of end point.
 *
 * @p b. No check is done on @p b.
 * @since 1.16
 */
EAPI void eina_bezier_values_set(Eina_Bezier *b, double start_x, double start_y, double ctrl_start_x, double ctrl_start_y, double ctrl_end_x, double ctrl_end_y, double end_x, double end_y) EINA_ARG_NONNULL(1);

/**
 * @brief Get the values of the points of the given floating
 * point cubic bezier curve.
 *
 * @param b The floating point bezier.
 * @param start_x x coordinate of start point.
 * @param start_y y coordinate of start point.
 * @param ctrl_start_x x coordinate of 1st control point.
 * @param ctrl_start_y y coordinate of 1st control point.
 * @param ctrl_end_x x coordinate of 2nd control point.
 * @param ctrl_end_y y coordinate of 2nd control point.
 * @param end_x x coordinate of end point.
 * @param end_y y coordinate of end point.
 *
 * @p b. No check is done on @p b.
 * @since 1.16
 */
EAPI void eina_bezier_values_get(const Eina_Bezier *b, double *start_x, double *start_y, double *ctrl_start_x, double *ctrl_start_y, double *ctrl_end_x, double *ctrl_end_y, double *end_x, double *end_y) EINA_ARG_NONNULL(1);

/**
 * @brief Returns the length of the given floating
 * point cubic bezier curve.
 *
 * @param b The floating point bezier.
 *
 * @p b. No check is done on @p b.
 * @since 1.16
 */
EAPI double eina_bezier_length_get(const Eina_Bezier *b) EINA_ARG_NONNULL(1);

/**
 * @brief Returns the position of the given bezier
 * at given length.
 *
 * @param b The floating point bezier.
 * @param len The given length.
 *
 * @p b. No check is done on @p b.
 * @since 1.16
 */
EAPI double eina_bezier_t_at(const Eina_Bezier *b, double len) EINA_ARG_NONNULL(1);

/**
 * @brief Gets the point on the bezier curve at
 * position t.
 *
 * @param b The floating point bezier.
 * @param t The floating point position.
 *
 * @p b. No check is done on @p b.
 * @since 1.16
 */
EAPI void eina_bezier_point_at(const Eina_Bezier *b, double t, double *px, double *py) EINA_ARG_NONNULL(1);

/**
 * @brief Returns the slope  of the  bezier
 * at given length.
 *
 * @param b The floating point bezier.
 * @param len The given length.
 *
 * @p b. No check is done on @p b.
 * @since 1.16
 */
EAPI double eina_bezier_angle_at(const Eina_Bezier *b, double t) EINA_ARG_NONNULL(1);

/**
 * @brief split the bezier at given length.
 *
 * @param b The floating point bezier.
 * @param len The given length.
 *
 * @p b. No check is done on @p b.
 * @since 1.16
 */
EAPI void eina_bezier_split_at_length(const Eina_Bezier *b, double len, Eina_Bezier *left, Eina_Bezier *right) EINA_ARG_NONNULL(1);

/**
 * @brief get the bound of the the bezier.
 *
 * @param b The floating point bezier.
 * @param x x coordinate of bounding box.
 * @param y y coordinate of bounding box.
 * @param w width of bounding box.
 * @param h height of bounding box.
 *
 * @p b. No check is done on @p b.
 * @since 1.17
 */
EAPI void eina_bezier_bounds_get(const Eina_Bezier *b, double *x, double *y, double *w, double *h) EINA_ARG_NONNULL(1);

/**
 * @brief find the bezier at given interval.
 *
 * @param b The floating point bezier.
 * @param t0 The start interval.
 * @param t1 The end interval.
 * @param result The result bezier.
 *
 * @p b. No check is done on @p b.
 * @since 1.17
 */
EAPI void eina_bezier_on_interval(Eina_Bezier *b, double t0, double t1, Eina_Bezier *result);

#endif // EINA_BEZIER_H

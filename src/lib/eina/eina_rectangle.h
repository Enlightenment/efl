/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Jorge Luis Zapata Muga
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

#ifndef EINA_RECTANGLE_H_
#define EINA_RECTANGLE_H_

#include "eina_types.h"

/**
 * @addtogroup Eina_Rectangle_Group Rectangle
 *
 * @brief These functions provide rectangle management.
 */

/**
 * @addtogroup Eina_Tools_Group Tools
 *
 * @{
 */

/**
 * @defgroup Eina_Rectangle_Group Rectangle
 *
 * @{
 */

#define EINA_RECTANGLE_INIT { 0, 0, 0, 0}
#define EINA_RECTANGLE_FORMAT "dx%d - %dx%d"
#define EINA_RECTANGLE_ARGS(r) (r)->x, (r)->y, (r)->w, (r)->h

/**
 * @typedef Eina_Rectangle
 * Simple rectangle structure.
 */
typedef struct _Eina_Rectangle
{
   int x; /**< top-left x coordinate of rectangle */
   int y; /**< top-left y coordinate of rectangle */
   int w; /**< width of rectangle */
   int h; /**< height of rectangle */
} Eina_Rectangle;

/**
 * @typedef Eina_Rectangle_Pool
 * Type for an opaque pool of rectangle.
 */
typedef struct _Eina_Rectangle_Pool Eina_Rectangle_Pool;

/**
 * @typedef Eina_Rectangle_Pool_Type
 * Type for an Eina Pool based on packing algorithm.
 * @since 1.11
 */
typedef enum {
  Eina_Packing_Descending,            /**< Current */
  Eina_Packing_Ascending,             /**< sorting in assending order */
  Eina_Packing_Bottom_Left,           /**< sorting in bottemleft fasion */
  Eina_Packing_Bottom_Left_Skyline,   /**< bottemleft skyline  */
  Eina_Packing_Bottom_Left_Skyline_Improved   /**< optimized bottemleft skyline  */
} Eina_Rectangle_Packing;

/**
 * @typedef Eina_Rectangle_Outside
 * Enumeration gives the positions where a rectangle can be outside a other rectangle
 * @since 1.19
 */
typedef enum {
    EINA_RECTANGLE_OUTSIDE_TOP = 1,
    EINA_RECTANGLE_OUTSIDE_LEFT = 2,
    EINA_RECTANGLE_OUTSIDE_BOTTOM = 4,
    EINA_RECTANGLE_OUTSIDE_RIGHT = 8
} Eina_Rectangle_Outside;


/**
 * @brief Check if the given spans intersect.
 *
 * @param c1 The column of the first span.
 * @param l1 The length of the first span.
 * @param c2 The column of the second span.
 * @param l2 The length of the second span.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * This function returns #EINA_TRUE if the  given spans intersect, #EINA_FALSE
 * otherwise.
 */
static inline int         eina_spans_intersect(int c1, int l1, int c2, int l2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Check if the given rectangle is empty.
 *
 * @param r The rectangle to check.
 * @return #EINA_TRUE if the rectangle is empty, #EINA_FALSE otherwise.
 *
 * This function returns #EINA_TRUE if @p r is empty, #EINA_FALSE
 * otherwise. No check is done on @p r, so it must be a valid
 * rectangle.
 */
static inline Eina_Bool   eina_rectangle_is_empty(const Eina_Rectangle *r) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Set the coordinates and size of the given rectangle.
 *
 * @param r The rectangle.
 * @param x The top-left x coordinate of the rectangle.
 * @param y The top-left y coordinate of the rectangle.
 * @param w The width of the rectangle.
 * @param h The height of the rectangle.
 *
 * This function sets its top-left x coordinate to @p x, its top-left
 * y coordinate to @p y, its width to @p w and its height to @p h. No
 * check is done on @p r, so it must be a valid rectangle.
 */
static inline void        eina_rectangle_coords_from(Eina_Rectangle *r, int x, int y, int w, int h) EINA_ARG_NONNULL(1);

/**
 * @brief Check if the given rectangles intersect.
 *
 * @param r1 The first rectangle.
 * @param r2 The second rectangle.
 * @return #EINA_TRUE if the rectangles intersect, #EINA_FALSE otherwise.
 *
 * This function returns #EINA_TRUE if @p r1 and @p r2 intersect, #EINA_FALSE
 * otherwise. No check is done on @p r1 and @p r2, so they must be valid
 * rectangles.
 */
static inline Eina_Bool   eina_rectangles_intersect(const Eina_Rectangle *r1, const Eina_Rectangle *r2) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Check if the given x-coordinate is in the rectangle .
 *
 * @param r The rectangle.
 * @param x The x coordinate.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * This function returns #EINA_TRUE if @p x is in @p r with respect to
 * the horizontal direction, #EINA_FALSE otherwise. No check is done
 * on @p r, so it must be a valid rectangle.
 */
static inline Eina_Bool   eina_rectangle_xcoord_inside(const Eina_Rectangle *r, int x) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Check if the given y-coordinate is in the rectangle .
 *
 * @param r The rectangle.
 * @param y The y coordinate.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * This function returns #EINA_TRUE if @p y is in @p r with respect to
 * the vertical direction, #EINA_FALSE otherwise. No check is done
 * on @p r, so it must be a valid rectangle.
 */
static inline Eina_Bool   eina_rectangle_ycoord_inside(const Eina_Rectangle *r, int y) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Check if the given point is in the rectangle .
 *
 * @param r The rectangle.
 * @param x The x coordinate of the point.
 * @param y The y coordinate of the point.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * This function returns #EINA_TRUE if the point of coordinate (@p x,
 * @p y) is in @p r, #EINA_FALSE otherwise. No check is done on @p r,
 * so it must be a valid rectangle.
 */
static inline Eina_Bool   eina_rectangle_coords_inside(const Eina_Rectangle *r, int x, int y) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Get the union of two rectangles.
 *
 * @param dst The first rectangle.
 * @param src The second rectangle.
 *
 * This function get the union of the rectangles @p dst and @p src. The
 * result is stored in @p dst. No check is done on @p dst or @p src,
 * so they must be valid rectangles.
 */
static inline void        eina_rectangle_union(Eina_Rectangle *dst, const Eina_Rectangle *src) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Get the intersection of two rectangles.
 *
 * @param dst The first rectangle.
 * @param src The second rectangle.
 * @return #EINA_TRUE if the rectangles intersect, #EINA_FALSE
 * otherwise.
 *
 * This function get the intersection of the rectangles @p dst and
 * @p src. The result is stored in @p dst. No check is done on @p dst
 * or @p src, so they must be valid rectangles.
 */
static inline Eina_Bool   eina_rectangle_intersection(Eina_Rectangle *dst, const Eina_Rectangle *src) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief FIXME I am useless and used by no one
 *
 * @param in The inner rectangle.
 * @param out The outer rectangle.
 * @param res The resulting rectangle.
 *
 */
static inline void        eina_rectangle_rescale_in(const Eina_Rectangle *out, const Eina_Rectangle *in, Eina_Rectangle *res) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief FIXME I am useless and used by no one
 *
 * @param in The inner rectangle.
 * @param out The outer rectangle.
 * @param res The resulting rectangle.
 *
 */
static inline void        eina_rectangle_rescale_out(const Eina_Rectangle *out, const Eina_Rectangle *in, Eina_Rectangle *res) EINA_ARG_NONNULL(1, 2, 3);

/**
 *
 * @brief Tells whether a rectangle is valid or not.
 *
 * @param r The rectangle
 * @return #EINA_TRUE if the rectangle is valid, #EINA_FALSE otherwise.
 *
 * This function checks if both width and height attributes of the rectangle are
 * positive integers. If so, the rectangle is considered valid, else the
 * rectangle is invalid.
 */
static inline Eina_Bool   eina_rectangle_is_valid(const Eina_Rectangle *r) EINA_ARG_NONNULL(1);

/**
 *
 * @brief Gives the rectangle maximum x coordinate.
 *
 * @param thiz The rectangle
 * @return the maximum x coordinate
 *
 * This function calculates the maximum x coordinate of the rectangle by summing
 * the @p width with the current @p x coodinate of the rectangle.
 */
static inline int         eina_rectangle_max_x(Eina_Rectangle *thiz) EINA_ARG_NONNULL(1);

/**
 *
 * @brief Gives the rectangle maximum y coordinate.
 *
 * @param thiz The rectangle
 * @return the maximum y coordinate
 *
 * This function calculates the maximum y coordinate of the rectangle by summing
 * the @p height with the current @p y coodinate of the rectangle.
 */
static inline int         eina_rectangle_max_y(Eina_Rectangle *thiz) EINA_ARG_NONNULL(1);

/**
 *
 * @brief Slices a rectangle vertically into two subrectangles starting from left edge
 *
 * @param thiz The rectangle to slice
 * @param slice The sliced part of the rectangle
 * @param remainder The left over part of the original rectangle after slice
 * @param amount The x inner coordinate of the rectangle where to perform the
 * slicing.
 * @return #EINA_TRUE if the cut succeeds, #EINA_FALSE otherwise
 *
 * Use this function if we must cut a rectangle vertically. The @p amount
 * parameter defines the x inner coordinate where to do the cut, starting from
 * the left edge of the rectangle.  If the @p amount value is greater than the
 * rectangle width, there will be not cut possible and #EINA_FALSE will be
 * returned.
 */
static inline Eina_Bool   eina_rectangle_x_cut(Eina_Rectangle *thiz, Eina_Rectangle *slice, Eina_Rectangle *remainder, int amount) EINA_ARG_NONNULL(1);

/**
 *
 * @brief Slices a rectangle horizontally into two subrectangles starting from bottom edge
 *
 * @param thiz The rectangle to slice
 * @param slice The sliced part of the rectangle
 * @param remainder The left over part of the original rectangle after slice
 * @param amount The y inner coordinate of the rectangle where to perform the
 * slicing.
 * @return #EINA_TRUE if the cut succeeds, #EINA_FALSE otherwise
 *
 * Use this function if we must cut a rectangle horizontally. The @p amount
 * parameter defines the y inner coordinate where to do the cut, starting from
 * the bottom edge of the rectangle. If the @p amount value is greater than the
 * rectangle width, there will be not cut possible and #EINA_FALSE will be
 * returned.
 */
static inline Eina_Bool   eina_rectangle_y_cut(Eina_Rectangle *thiz, Eina_Rectangle *slice, Eina_Rectangle *remainder, int amount) EINA_ARG_NONNULL(1);

/**
 *
 * @brief Slices a rectangle vertically starting from right edge
 *
 * @param thiz The rectangle to slice
 * @param slice The sliced part of the rectangle
 * @param remainder The left over part of the original rectangle after slice
 * @param amount The amount to cut off the rectangle starting from the right
 * edge
 * @return #EINA_TRUE if the cut succeeds, #EINA_FALSE otherwise
 *
 * Use this function if we must cut a rectangle vertically. The @p amount
 * parameter defines the inner x coordinate where to do the cut, starting from
 * the right edge of the rectangle.  If the @p amount value is greater than the
 * rectangle width, there will be not cut possible and #EINA_FALSE will be
 * returned.
 */
static inline Eina_Bool   eina_rectangle_width_cut(Eina_Rectangle *thiz, Eina_Rectangle *slice, Eina_Rectangle *remainder, int amount) EINA_ARG_NONNULL(1);

/**
 *
 * @brief Slices a rectangle horizontally starting from top edge
 *
 * @param thiz The rectangle to slice
 * @param slice The sliced part of the rectangle
 * @param remainder The left over part of the original rectangle after slice
 * @param amount The amount to cut off the rectangle starting from the top edge
 * @return #EINA_TRUE if the cut succeeds, #EINA_FALSE otherwise
 *
 * Use this function if we must cut a rectangle horizontally. The @p amount
 * parameter defines the inner y coordinate where to do the cut, starting from
 * the top edge of the rectangle.  If the @p amount value is greater than the
 * rectangle width, there will be not cut possible and #EINA_FALSE will be
 * returned.
 */
static inline Eina_Bool   eina_rectangle_height_cut(Eina_Rectangle *thiz, Eina_Rectangle *slice, Eina_Rectangle *remainder, int amount) EINA_ARG_NONNULL(1);

/**
 * @brief Subtract two rectangles.
 *
 * @param thiz The minuend rectangle
 * @param other The subtrahend rectangle
 *
 * This function subtract two rectangles. The difference is stored on @p out
 * There will be at most four differences, use eina_rectangle_is_valid to
 * confirm the number of differences.
 */
static inline Eina_Bool   eina_rectangle_subtract(Eina_Rectangle *thiz, Eina_Rectangle *other, Eina_Rectangle out[4]) EINA_ARG_NONNULL(1);

/**
 * @brief Add a rectangle in a new pool.
 *
 * @param w The width of the rectangle.
 * @param h The height of the rectangle.
 * @return A newly allocated pool on success, @c NULL otherwise.
 *
 * This function adds the rectangle of size (@p width, @p height) to a
 * new pool. If the pool can not be created, @c NULL is
 * returned. Otherwise the newly allocated pool is returned.
 */
EAPI Eina_Rectangle_Pool *eina_rectangle_pool_new(int w, int h) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Return the pool of the given rectangle.
 *
 * @param rect The rectangle.
 * @return The pool of the given rectangle.
 *
 * This function returns the pool in which @p rect is. If  @p rect is
 * @c NULL, @c NULL is returned.
 */
EAPI Eina_Rectangle_Pool *eina_rectangle_pool_get(Eina_Rectangle *rect) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Return the width and height of the given pool.
 *
 * @param pool The pool.
 * @param w The returned width.
 * @param h The returned height.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * This function returns the width and height of @p pool and store
 * them in respectively @p w and @p h if they are not @c NULL. If
 * @p pool is @c NULL, #EINA_FALSE is returned. Otherwise #EINA_TRUE is
 * returned.
 */
EAPI Eina_Bool            eina_rectangle_pool_geometry_get(Eina_Rectangle_Pool *pool, int *w, int *h) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Get the data from the given pool.
 *
 * @param pool The pool.
 * @return The returned data.
 *
 * This function gets the data from @p pool set by
 * eina_rectangle_pool_data_set(). If @p pool is @c NULL, this
 * function returns @c NULL.
 */
EAPI void                *eina_rectangle_pool_data_get(Eina_Rectangle_Pool *pool) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Set the data to the given pool.
 *
 * @param pool The pool.
 * @param data The data to set.
 *
 * This function sets @p data to @p pool. If @p pool is @c NULL, this
 * function does nothing.
 */
EAPI void                 eina_rectangle_pool_data_set(Eina_Rectangle_Pool *pool, const void *data) EINA_ARG_NONNULL(1);

/**
 * @brief Free the given pool.
 *
 * @param pool The pool to free.
 *
 * This function frees the allocated data of @p pool. If @p pool is
 * @c NULL, this function returned immediately.
 */
EAPI void                 eina_rectangle_pool_free(Eina_Rectangle_Pool *pool) EINA_ARG_NONNULL(1);

/**
 * @brief Return the number of rectangles in the given pool.
 *
 * @param pool The pool.
 * @return The number of rectangles in the pool.
 *
 * This function returns the number of rectangles in @p pool.
 */
EAPI int                  eina_rectangle_pool_count(Eina_Rectangle_Pool *pool) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Request a rectangle of given size in the given pool.
 *
 * @param pool The pool.
 * @param w The width of the rectangle to request.
 * @param h The height of the rectangle to request.
 * @return The requested rectangle on success, @c NULL otherwise.
 *
 * This function retrieve from @p pool the rectangle of width @p w and
 * height @p h. If @p pool is @c NULL, or @p w or @p h are non-positive,
 * the function returns @c NULL. If @p w or @p h are greater than the
 * pool size, the function returns @c NULL. On success, the function
 * returns the rectangle which matches the size (@p w, @p h).
 * Otherwise it returns @c NULL.
 */
EAPI Eina_Rectangle      *eina_rectangle_pool_request(Eina_Rectangle_Pool *pool, int w, int h) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Remove the given rectangle from the pool.
 *
 * @param rect The rectangle to remove from the pool.
 *
 * This function removes @p rect from the pool. If @p rect is
 * @c NULL, the function returns immediately. Otherwise it removes @p
 * rect from the pool.
 */
EAPI void                 eina_rectangle_pool_release(Eina_Rectangle *rect) EINA_ARG_NONNULL(1);

/**
 * @def EINA_RECTANGLE_SET
 * @brief Macro to set the values of a #Eina_Rectangle.
 *
 * @param Rectangle The rectangle to set the values.
 * @param X The X coordinate of the top left corner of the rectangle.
 * @param Y The Y coordinate of the top left corner of the rectangle.
 * @param W The width of the rectangle.
 * @param H The height of the rectangle.
 *
 * This macro set the values of @p Rectangle. (@p X, @p Y) is the
 * coordinates of the top left corner of @p Rectangle, @p W is its
 * width and @p H is its height.
 */
#define EINA_RECTANGLE_SET(Rectangle, X, Y, W, H) \
    {						  \
      (Rectangle)->x = X;			  \
      (Rectangle)->y = Y;			  \
      (Rectangle)->w = W;			  \
      (Rectangle)->h = H;			  \
    }


/**
 * @brief Create a new rectangle.
 *
 * @param x The X coordinate of the top left corner of the rectangle.
 * @param y The Y coordinate of the top left corner of the rectangle.
 * @param w The width of the rectangle.
 * @param h The height of the rectangle.
 * @return The new rectangle on success, @ NULL otherwise.
 *
 * This function creates a rectangle which top left corner has the
 * coordinates (@p x, @p y), with height @p w and height @p h and adds
 * it to the rectangles pool. No check is done on @p w and @p h. This
 * function returns a new rectangle on success, @c NULL otherwhise.
 */
EAPI Eina_Rectangle *eina_rectangle_new(int x, int y, int w, int h) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Free the given rectangle.
 *
 * @param rect The rectangle to free.
 *
 * This function removes @p rect from the rectangles pool.
 */
EAPI void            eina_rectangle_free(Eina_Rectangle *rect) EINA_ARG_NONNULL(1);

/**
 * @brief Sets the type of given rectangle pool.
 *
 * @param pool The rectangle pool for which type is to be set.
 *
 * This function sets @p type of @p pool.
 * @see Eina_Rectangle_Packing
 * @since 1.11
 */
EAPI void            eina_rectangle_pool_packing_set(Eina_Rectangle_Pool *pool,Eina_Rectangle_Packing type) EINA_ARG_NONNULL(1);

/**
 * @brief calculate where rect2 is outside of rect1
 *
 * @param rect1 the rect to use as anchor
 * @param rect2 the rect to look for outside positions
 *
 * @return A or'ed map of Eina_Rectangle_Outside values
 * @since 1.19
 */
EAPI Eina_Rectangle_Outside eina_rectangle_outside_position(Eina_Rectangle *rect1, Eina_Rectangle *rect2);

#include "eina_inline_rectangle.x"

/**
 * @}
 */

/**
 * @}
 */

#endif /*_EINA_RECTANGLE_H_*/

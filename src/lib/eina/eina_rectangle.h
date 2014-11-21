/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Jorge Luis Zapata Muga
 *
 * This library is a free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
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
 * @defgroup Eina_Rectangle_Group Rectangle
 * @ingroup Eina_Tools_Group
 *
 * @brief This group discusses the functions that provide rectangle management.
 *
 * @{
 */

/**
 * @typedef Eina_Rectangle
 * @brief The structure type for the simple rectangle structure.
 */
typedef struct _Eina_Rectangle
{
   int x; /**< The top-left x co-ordinate of the rectangle */
   int y; /**< The top-left y co-ordinate of the rectangle */
   int w; /**< The width of the rectangle */
   int h; /**< The height of the rectangle */
} Eina_Rectangle;

/**
 * @typedef Eina_Rectangle_Pool
 * @brief The structure type for an opaque rectangle pool.
 */
typedef struct _Eina_Rectangle_Pool Eina_Rectangle_Pool;

/**
 * @typedef Eina_Rectangle_Packing
 * @brief Enumeration of the type for an Eina Pool based on the packing algorithm.
 * @since 1.10
 */
typedef enum {
  Eina_Packing_Descending,            /**< Current */
  Eina_Packing_Ascending,             /**< Sorting in ascending order */
  Eina_Packing_Bottom_Left,           /**< Sorting in bottom left fashion */
  Eina_Packing_Bottom_Left_Skyline,    /**< Bottom left skyline */
  Eina_Packing_Bottom_Left_Skyline_improved   /**< Optimized bottom left skyline */
} Eina_Rectangle_Packing;

/**
 * @brief Check if the given spans intersect.
 *
 * @details This function returns #EINA_TRUE if the  given spans intersect,
 *          #EINA_FALSE otherwise.
 *
 * @since_tizen 2.3
 *
 * @param[in] c1 The column of the first span.
 * @param[in] l1 The length of the first span.
 * @param[in] c2 The column of the second span.
 * @param[in] l2 The length of the second span.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 */
static inline int         eina_spans_intersect(int c1, int l1, int c2, int l2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Check if the given rectangle is empty.
 *
 * @details This function returns #EINA_TRUE if @p r is empty, #EINA_FALSE
 *          otherwise. No check is done on @p r, so it must be a valid
 *          rectangle.
 *
 * @since_tizen 2.3
 *
 * @param[in] r The rectangle to check.
 * @return #EINA_TRUE if the rectangle is empty, #EINA_FALSE otherwise.
 */
static inline Eina_Bool   eina_rectangle_is_empty(const Eina_Rectangle *r) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Set the coordinates and size of the given rectangle.
 *
 * @details This function sets its top-left x coordinate to @p x, its top-left
 *          y coordinate to @p y, its width to @p w and its height to @p h. No
 *          check is done on @p r, so it must be a valid rectangle.
 *
 * @since_tizen 2.3
 *
 * @param[in] r The rectangle.
 * @param[in] x The top-left x coordinate of the rectangle.
 * @param[in] y The top-left y coordinate of the rectangle.
 * @param[in] w The width of the rectangle.
 * @param[in] h The height of the rectangle.
 */
static inline void        eina_rectangle_coords_from(Eina_Rectangle *r, int x, int y, int w, int h) EINA_ARG_NONNULL(1);

/**
 * @brief Check if the given rectangles intersect.
 *
 * @details This function returns #EINA_TRUE if @p r1 and @p r2 intersect, #EINA_FALSE
 *          otherwise. No check is done on @p r1 and @p r2, so they must be valid
 *          rectangles.
 *
 * @since_tizen 2.3
 *
 * @param[in] r1 The first rectangle.
 * @param[in] r2 The second rectangle.
 * @return #EINA_TRUE if the rectangles intersect, #EINA_FALSE otherwise.
 */
static inline Eina_Bool   eina_rectangles_intersect(const Eina_Rectangle *r1, const Eina_Rectangle *r2) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Check if the given x-coordinate is in the rectangle .
 *
 * @details This function returns #EINA_TRUE if @p x is in @p r with respect to
 *          the horizontal direction, #EINA_FALSE otherwise. No check is done
 *          on @p r, so it must be a valid rectangle.
 *
 * @since_tizen 2.3
 *
 * @param[in] r The rectangle.
 * @param[in] x The x coordinate.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 */
static inline Eina_Bool   eina_rectangle_xcoord_inside(const Eina_Rectangle *r, int x) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Check if the given y-coordinate is in the rectangle .
 *
 * @details This function returns #EINA_TRUE if @p y is in @p r with respect to
 *          the vertical direction, #EINA_FALSE otherwise. No check is done
 *          on @p r, so it must be a valid rectangle.
 *
 * @since_tizen 2.3
 *
 * @param[in] r The rectangle.
 * @param[in] y The y coordinate.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 */
static inline Eina_Bool   eina_rectangle_ycoord_inside(const Eina_Rectangle *r, int y) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Check if the given point is in the rectangle .
 *
 * @details This function returns #EINA_TRUE if the point of coordinate (@p x,
 *          @p y) is in @p r, #EINA_FALSE otherwise. No check is done on @p r,
 *          so it must be a valid rectangle.
 *
 * @since_tizen 2.3
 *
 * @param[in] r The rectangle.
 * @param[in] x The x coordinate of the point.
 * @param[in] y The y coordinate of the point.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 */
static inline Eina_Bool   eina_rectangle_coords_inside(const Eina_Rectangle *r, int x, int y) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Get the union of two rectangles.
 *
 * @details This function get the union of the rectangles @p dst and @p src. The
 *          result is stored in @p dst. No check is done on @p dst or @p src,
 *          so they must be valid rectangles.
 *
 * @since_tizen 2.3
 *
 * @param[in] dst The first rectangle.
 * @param[in] src The second rectangle.
 */
static inline void        eina_rectangle_union(Eina_Rectangle *dst, const Eina_Rectangle *src) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Get the intersection of two rectangles.
 *
 * @details This function get the intersection of the rectangles @p dst and
 *          @p src. The result is stored in @p dst. No check is done on @p dst
 *          or @p src, so they must be valid rectangles.
 *
 * @since_tizen 2.3
 *
 * @param[in] dst The first rectangle.
 * @param[in] src The second rectangle.
 * @return #EINA_TRUE if the rectangles intersect, #EINA_FALSE
 * otherwise.
 */
static inline Eina_Bool   eina_rectangle_intersection(Eina_Rectangle *dst, const Eina_Rectangle *src) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Rescale a rectangle inner position.
 *
 * @details This function rescales a rectangle by using @p out and @p in.
 *
 * @since_tizen 2.3
 *
 * @param[in] out The outter rectangle.
 * @param[in] in The inner rectangle.
 * @param[in] res The rectangle to be rescaled
 */
static inline void        eina_rectangle_rescale_in(const Eina_Rectangle *out, const Eina_Rectangle *in, Eina_Rectangle *res) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Rescale a rectangle outter position.
 *
 * @details This function rescales a rectangle by using @p out and @p in.
 *
 * @since_tizen 2.3
 *
 * @param[in] out The outter rectangle.
 * @param[in] in The inner rectangle.
 * @param[in] res The rectangle to be rescaled
 */
static inline void        eina_rectangle_rescale_out(const Eina_Rectangle *out, const Eina_Rectangle *in, Eina_Rectangle *res) EINA_ARG_NONNULL(1, 2, 3);


/**
 * @brief Adds a rectangle in a new pool.
 *
 * @details This function adds the rectangle of size (@a width, @a height) to a
 *          new pool. If the pool cannot be created, @c NULL is
 *          returned. Otherwise the newly allocated pool is returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] w The width of the rectangle
 * @param[in] h The height of the rectangle
 * @return A newly allocated pool on success, otherwise @c NULL
 *
 */
EAPI Eina_Rectangle_Pool *eina_rectangle_pool_new(int w, int h) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Gets the pool of the given rectangle.
 *
 * @details This function returns the pool in which @a rect is present. If @a rect is
 *          @c NULL, @c NULL is returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] rect The rectangle
 * @return The pool of the given rectangle
 *
 */
EAPI Eina_Rectangle_Pool *eina_rectangle_pool_get(Eina_Rectangle *rect) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Gets the width and height of the given pool.
 *
 * @details This function returns the width and height of @a pool and stores
 *          them in @a w and @a h respectively if they are not @c NULL. If
 *          @a pool is @c NULL, @c EINA_FALSE is returned. Otherwise @c EINA_TRUE is
 *          returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] pool The pool
 * @param[out] w The returned width
 * @param[out] h The returned height
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool            eina_rectangle_pool_geometry_get(Eina_Rectangle_Pool *pool, int *w, int *h) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Gets the data from the given pool.
 *
 * @details This function gets the data from @a pool set by
 *          eina_rectangle_pool_data_set(). If @a pool is @c NULL, this
 *          function returns @c NULL.
 *
 * @since_tizen 2.3
 *
 * @param[in] pool The pool
 * @return The returned data
 *
 */
EAPI void                *eina_rectangle_pool_data_get(Eina_Rectangle_Pool *pool) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Sets the data to the given pool.
 *
 * @details This function sets @a data to @a pool. If @a pool is @c NULL, this
 *          function does nothing.
 *
 * @since_tizen 2.3
 *
 * @param[in] pool The pool
 * @param[in] data The data to set
 *
 */
EAPI void                 eina_rectangle_pool_data_set(Eina_Rectangle_Pool *pool, const void *data) EINA_ARG_NONNULL(1);

/**
 * @brief Frees the given pool.
 *
 * @details This function frees the allocated data of @a pool. If @a pool is
 *          @c NULL, this function returns immediately.
 *
 * @since_tizen 2.3
 *
 * @param[in] pool The pool to free
 *
 */
EAPI void                 eina_rectangle_pool_free(Eina_Rectangle_Pool *pool) EINA_ARG_NONNULL(1);

/**
 * @brief Returns the number of rectangles in the given pool.
 *
 * @details This function returns the number of rectangles in @a pool.
 *
 * @since_tizen 2.3
 *
 * @param[in] pool The pool
 * @return The number of rectangles in the pool
 *
 */
EAPI int                  eina_rectangle_pool_count(Eina_Rectangle_Pool *pool) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Requests for a rectangle of the given size in the given pool.
 *
 * @details This function retrieves from @a pool the rectangle of width @a w and
 *          height @a h. If @a pool is @c NULL, or @a w or @a h are non-positive,
 *          the function returns @c NULL. If @a w or @a h are greater than the
 *          pool size, the function returns @c NULL. On success, the function
 *          returns the rectangle that matches the size (@a w, @a h).
 *          Otherwise it returns @c NULL.
 *
 * @since_tizen 2.3
 *
 * @param[in] pool The pool
 * @param[in] w The width of the rectangle to request for
 * @param[in] h The height of the rectangle to request for
 * @return The requested rectangle on success, otherwise @c NULL
 *
 */
EAPI Eina_Rectangle      *eina_rectangle_pool_request(Eina_Rectangle_Pool *pool, int w, int h) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Removes the given rectangle from the pool.
 *
 * @details This function removes @a rect from the pool. If @a rect is
 *          @c NULL, the function returns immediately. Otherwise it removes
 *          @a rect from the pool.
 *
 * @since_tizen 2.3
 *
 * @param[in] rect The rectangle to remove from the pool
 *
 */
EAPI void                 eina_rectangle_pool_release(Eina_Rectangle *rect) EINA_ARG_NONNULL(1);

/**
 * @def EINA_RECTANGLE_SET
 * @brief Provides a macro to set the values of a #Eina_Rectangle.
 *
 * @details This macro sets the values of @a Rectangle. (@a X, @a Y) are the
 *          coordinates of the top left corner of @a Rectangle, @a W is its
 *          width and @a H is its height.
 *
 * @since_tizen 2.3
 *
 * @param Rectangle The rectangle to set the values of
 * @param X The X coordinate of the top left corner of the rectangle
 * @param Y The Y coordinate of the top left corner of the rectangle
 * @param W The width of the rectangle
 * @param H The height of the rectangle
 *
 */
#define EINA_RECTANGLE_SET(Rectangle, X, Y, W, H) \
  (Rectangle)->x = X;                             \
  (Rectangle)->y = Y;                             \
  (Rectangle)->w = W;                             \
  (Rectangle)->h = H;


/**
 * @brief Creates a new rectangle.
 *
 * @details This function creates a rectangle whose top left corner has the
 *          coordinates (@a x, @a y), with width @a w and height @a h and adds
 *          it to the rectangle's pool. No check is done on @a w and @a h. This
 *          function returns a new rectangle on success, otherwise it returns @c NULL.
 *
 * @since_tizen 2.3
 *
 * @param[in] x The X coordinate of the top left corner of the rectangle
 * @param[in] y The Y coordinate of the top left corner of the rectangle
 * @param[in] w The width of the rectangle
 * @param[in] h The height of the rectangle
 * @return The new rectangle on success, otherwise @c NULL
 *
 */
EAPI Eina_Rectangle *eina_rectangle_new(int x, int y, int w, int h) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Frees the given rectangle.
 *
 * @details This function removes @a rect from the rectangles pool.
 *
 * @since_tizen 2.3
 *
 * @param[in] rect The rectangle to free
 *
 */
EAPI void            eina_rectangle_free(Eina_Rectangle *rect) EINA_ARG_NONNULL(1);

/**
 * @brief Sets the type of the given rectangle pool.
 *
 * @details This function sets @a type of @a pool.
 *
 * @since 1.10
 *
 * @since_tizen 2.3
 *
 * @param[in] pool The rectangle pool type to set
 * @param[in] type The packing type to set
 *
 * @see Eina_Rectangle_Packing
 */
EAPI void            eina_rectangle_pool_packing_set(Eina_Rectangle_Pool *pool, Eina_Rectangle_Packing type) EINA_ARG_NONNULL(1);

#include "eina_inline_rectangle.x"

/**
 * @}
 */

#endif /*_EINA_RECTANGLE_H_*/

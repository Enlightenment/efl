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

#ifndef EINA_TILER_H_
#define EINA_TILER_H_

#include "eina_types.h"
#include "eina_iterator.h"
#include "eina_rectangle.h"

/**
 * @defgroup Eina_Tiler_Group Tiler
 * @ingroup Eina_Data_Types_Group
 * @brief Eina_Tiler is a tool to facilitate calculations of areas that are damaged
 *         and thus need to be re-rendered.
 *
 * @remarks This is a very low level tool, in most situations(for example if
 *          you're using evas) you won't need this.
 *
 * @section basic Basic usage
 *
 * The basic usage of Eina_Tiler is to give it  the size of your canvas and a
 * set of rectangular areas that need re-rendering. From that and using
 * heuristics, it tells you an efficient way to re-render in the form of a set
 * of non-overlapping rectangles that cover the whole area that needs re-rendering.
 *
 * The following is a pseudo-code showing some simple uses of Eina_Tiler:
 * @code
 * tiler = eina_tiler_new(MY_CANVAS_WIDTH, MY_CANVAS_HEIGHT);
 * EINA_LIST_FOREACH(list_of_areas_that_need_re_rendering, l, rect)
 *   eina_tiler_add(tiler, rect);
 * itr = eina_tiler_iterator_new(tiler);
 * EINA_ITERATOR_FOREACH(itr, rect)
 *   my_function_that_repaints_areas_of_the_canvas(rect);
 * @endcode
 *
 * @see eina_tiler_new()
 * @see eina_tiler_rect_add()
 * @see eina_tiler_iterator_new()
 *
 * @remarks There is no guarantee that this is the most efficient way to
 *          re-render for any particular case.
 *
 * @section grid_slicer Grid Slicer
 *
 * Grid slicer and Eina_Tiler are usually used together, that is however @b not
 * necessary, they can be used independently. Grid slicer provides an easy API
 * to divide an area into tiles, which is useful in certain applications to divide
 * the area that is rendered into tiles. It's customary to, then create one
 * Eina_Tiler for each tile.
 *
 * The following is a pseudo-code showing a very simplified use of a grid slicer
 * together with Eina_Tiler:
 * @code
 * itr = eina_tile_grid_slicer_iterator_new(0, 0, MY_CANVAS_WIDTH, MY_CANVAS_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
 * EINA_ITERATOR_FOREACH(itr, grid_info)
 *   {
 *      tiler = eina_tiler_new(grid_info->rect.w, grid_info->rect.w);
 *      EINA_LIST_FOREACH(list_of_areas_that_need_re_rendering_in_this_tile, l, rect)
 *        eina_tiler_add(tiler, rect);
 *      itr = eina_tiler_iterator_new(tiler);
 *      EINA_ITERATOR_FOREACH(itr, rect)
 *      my_function_that_repaints_areas_of_the_canvas(rect);
 *   }
 * @endcode
 *
 * @see eina_tiler_new()
 * @see eina_tiler_rect_add()
 * @see eina_tile_grid_slicer_setup()
 * @see eina_tile_grid_slicer_next()
 * @see eina_tile_grid_slicer_iterator_new()
 *
 * @{
 */

/**
 * @typedef Eina_Tiler
 * @brief The structure type for the tiler type.
 */
typedef struct _Eina_Tiler Eina_Tiler;

/**
 * @typedef Eina_Tile_Grid_Info
 * @brief The structure type for the grid type of a tiler.
 */
typedef struct Eina_Tile_Grid_Info Eina_Tile_Grid_Info;

/**
 * @struct Eina_Tile_Grid_Info
 * @brief The structure type for the grid type of a tiler.
 */
struct Eina_Tile_Grid_Info
{
   unsigned long  col; /**< Column of the tile grid */
   unsigned long  row; /**< Row of the tile grid */
   Eina_Rectangle rect; /**< Rectangle of the tile grid, coordinates are
                             relative to the tile */
   Eina_Bool      full; /**< Whether the grid is full or not */
};

typedef struct _Eina_Tile_Grid_Slicer Eina_Tile_Grid_Slicer;

/**
 * @brief Creates a new tiler with @a w width and @a h height.
 *
 * @since_tizen 2.3
 *
 * @param[in] w The width of the tiler
 * @param[in] h The height of the tiler
 * @return The newly created tiler
 *
 * @see eina_tiler_free()
 */
EAPI Eina_Tiler        *eina_tiler_new(int w, int h);

/**
 * @brief Frees a tiler.
 *
 * @details This function frees @a t. It does not free the memory allocated for the
 *          elements of @a t.
 *
 * @since_tizen 2.3
 *
 * @param[in] t The tiler to free
 *
 */
EAPI void               eina_tiler_free(Eina_Tiler *t);
/**
 * @brief Sets the size of the tiles for a tiler.
 *
 * @since_tizen 2.3
 *
 * @remarks @a w and @a h @b must be greater than zero, otherwise the tile size
 *          won't be changed.
 * @remarks The tile size is not used.
 *
 * @param[in] t The tiler whose tile size are set
 * @param[in] w The width of the tiles
 * @param[in] h The height of the tiles
 *
 */
EAPI void               eina_tiler_tile_size_set(Eina_Tiler *t, int w, int h);
/**
 * @brief Adds a rectangle to a tiler.
 *
 * @since_tizen 2.3
 *
 * @param[in] t The tiler in which to add a container
 * @param[in] r The rectangle to be added
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 * @see eina_tiler_rect_del()
 */
EAPI Eina_Bool          eina_tiler_rect_add(Eina_Tiler *t, const Eina_Rectangle *r);
/**
 * @brief Removes a rectangle from a tiler.
 *
 * @since_tizen 2.3
 *
 * @param[in] t The tiler in which to add a container
 * @param[in] r The rectangle to be removed
 *
 * @see eina_tiler_rect_add()
 * @see eina_tiler_clear()
 */
EAPI void               eina_tiler_rect_del(Eina_Tiler *t, const Eina_Rectangle *r);
/**
 * @brief Removes all rectangles from the tiles.
 *
 * @since_tizen 2.3
 *
 * @param[in] t The tiler to clear
 *
 * @see eina_tiler_rect_del()
 */
EAPI void               eina_tiler_clear(Eina_Tiler *t);
/**
 * @brief Creates an iterator to access the tiler's calculated rectangles.
 *
 * @since_tizen 2.3
 *
 * @param[in] t The tiler to iterate over
 * @return An iterator containing Eina_Rectangle
 */
EAPI Eina_Iterator     *eina_tiler_iterator_new(const Eina_Tiler *t);

/**
 * @brief Creates a new Eina_Iterator that iterates over a list of tiles.
 *
 * @since_tizen 2.3
 *
 * @remarks The region defined by @a x, @a y, @a w, and @a h is divided into a grid of
 *          tiles of width @a tile_w and height @a tile_h, the returned iterator then
 *          iterates over every tile in the grid having #Eina_Tile_Grid_Info as its data.
 *
 * @remarks This is a convenience function, iterating over the returned iterator is
 *          equivalent to calling eina_tile_grid_slicer_setup() and calling
 *          eina_tile_grid_slicer_next() until it returns @c EINA_FALSE.
 *
 * @param[in] x The x axis coordinate
 * @param[in] y The y axis coordinate
 * @param[in] w The width
 * @param[in] h The height
 * @param[in] tile_w The tile width
 * @param[in] tile_h The tile height
 * @return  A pointer to the Eina_Iterator \n
 *          @c NULL on failure.
 *
 */
EAPI Eina_Iterator     *eina_tile_grid_slicer_iterator_new(int x, int y, int w, int h, int tile_w, int tile_h);

/**
 * @brief Iterates over the tiles set by eina_tile_grid_slicer_setup().
 *
 * @details This function iterates over each Eina_Tile_Grid_Info *rect of the grid.
 *          eina_tile_grid_slicer_setup() must be called first, and *rect is only valid
 *          if this function returns @c EINA_TRUE. Its content shouldn't be modified.
 *
 * @since_tizen 2.3
 *
 * @remarks Consider using eina_tile_grid_slicer_iterator_new() instead.
 *
 * @param[in] slc A pointer to an Eina_Tile_Grid_Slicer struct
 * @param[out] rect A pointer to a struct Eina_Tile_Grid_Info
 * @return  @c EINA_TRUE if the current rectangle is valid, otherwise @c EINA_FALSE if there
 *          are no more rectangles to iterate over (and thus the current one isn't valid)
 *
 */
static inline Eina_Bool eina_tile_grid_slicer_next(Eina_Tile_Grid_Slicer *slc, const Eina_Tile_Grid_Info **rect);

/**
 * @brief Sets up an Eina_Tile_Grid_Slicer struct.
 *
 * @since_tizen 2.3
 *
 * @remarks The region defined by @a x, @a y, @a w, and @a h is divided into a grid of
 *          tiles of width @a tile_w and height @a tile_h, @a slc can then be used with
 *          eina_tile_grid_slicer_next() to access each tile.
 *
 * @remarks Consider using eina_tile_grid_slicer_iterator_new() instead.
 *
 * @param[in] slc A pointer to an Eina_Tile_Grid_Slicer struct
 * @param[in] x The x axis coordinate
 * @param[in] y The y axis coordinate
 * @param[in] w The width
 * @param[in] h The height
 * @param[in] tile_w The tile width
 * @param[in] tile_h The tile height
 * @return  A pointer to the Eina_Iterator \n
 *          @c NULL on failure.
 *
 */
static inline Eina_Bool eina_tile_grid_slicer_setup(Eina_Tile_Grid_Slicer *slc, int x, int y, int w, int h, int tile_w, int tile_h);

#include "eina_inline_tiler.x"

/**
 * @}
 */

#endif /* EINA_TILER_H_ */

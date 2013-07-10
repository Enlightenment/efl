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

#ifndef EINA_TILER_H_
#define EINA_TILER_H_

#include "eina_types.h"
#include "eina_iterator.h"
#include "eina_rectangle.h"

/**
 * @page eina_tiler_example_01
 * @dontinclude eina_tiler_01.c
 *
 * This is an example that illustrates how Eina_Tiler works for a given set of
 * rectangles. The rectangles must be given in the command line in the form:
 * \<width\>x\<height\>+\<x offset\>+\<y offset\>
 * The example will show two panels, the first(input) will show the given
 * rectangles(in different colors) and in the seconds(output) it will show the
 * rectangles given by the tiler. The rectangles will be added one by one every
 * two seconds. A lot of the example deals with actually painting the rectangles
 * so we'll skip over quite a bit of code, but you can see all of it in @ref
 * eina_tiler_01.c "eina_tiler_01.c".
 *
 * The first thing of note in our example is the creation of the tiler:
 * @skipline eina_tiler_new
 * @note @p maxw and @p maxh are calculated such that the tiler's size will
 * fully encompass all given rectangles.
 *
 * We'll now look at the function that actually adds rectangles to our tiler. It
 * first checks if we added all rectangles already and if so stops right there:
 * @dontinclude eina_tiler_01.c
 * @skip static Eina_Bool
 * @until }
 *
 * Our function then clears all rectangles given to us by tiler from the last
 * execution. It does this because each rectangle we add may change everything
 * about the output of eina_tiler:
 * @until output_rects_reset
 *
 * Next we get another rectangle, print it and show it in the input panel:
 * @until add_input_rect
 *
 * We now come to the tiler stuff, we add our new rectangle to it and get a new
 * iterator for the tiler:
 * @until itr
 *
 * We now iterate over our tiler printing every rect it gives us and sowing it
 * in the output panel:
 * @until }
 *
 * We of course must remember to free our iterator and that's it for this
 * function:
 * @until }
 *
 * You should try many different inputs to see how the tiler works, here are a
 * few suggestions:
 * @li 100x100+0+0 100x100+200+200
 * @li 100x100+0+0 100x100+5+5 100x100+10+10 100x100+15+15 100x100+20+20
 * @li 100x100+0+0 100x100+100+100 100x100+200+0 100x100+0+200 100x100+200+200
 * @li 10x10+0+0 10x10+10+10 10x10+20+0 10x10+0+20 10x10+20+20
 *
 * @example eina_tiler_01.c
 */
/**
 * @addtogroup Eina_Data_Types_Group Data Types
 *
 * @{
 */

/**
 * @defgroup Eina_Tiler_Group Tiler
 *
 * @warning This is a very low level tool, in most situations(for example if
 * you're using evas) you won't need this.
 *
 * @section basic Basic usage
 *
 * Eina_Tiler is a tool to facilitate calculations of which areas are damaged
 * and thus need to be re-rendered. The basic usage of Eina_Tiler is to give it
 * the size of your canvas and a set of rectangular areas that need
 * re-rendering, from that and using heuristics it'll tell you an efficient way
 * to re-render in the form of a set of non-overlapping rectangles that covers
 * the whole area that needs re-rendering.
 *
 * The following is pseudo-code showing some simple use of Eina_Tiler:
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
 * @warning There are no guarantees that this will be the most efficient way to
 * re-render for any particular case.
 *
 * @section grid_slicer Grid Slicer
 *
 * Grid slicer and Eina_Tiler are usually used together, that is however @b not
 * necessary, they can be used independently. Grid slicer provides an easy API
 * to divide an area in tiles which is useful in certain applications to divide
 * the area that will be rendered into tiles. It's customary to, then create one
 * Eina_Tiler for each tile.
 *
 * The following is pseudo-code showing a very simplified use of grid slicer
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
 * Tiler type.
 */
typedef struct _Eina_Tiler Eina_Tiler;

/**
 * @typedef Eina_Tile_Grid_Info
 * Grid type of a tiler.
 */
typedef struct Eina_Tile_Grid_Info Eina_Tile_Grid_Info;

/**
 * @struct Eina_Tile_Grid_Info
 * Grid type of a tiler.
 */
struct Eina_Tile_Grid_Info
{
   unsigned long  col; /**< column of the tile grid */
   unsigned long  row; /**< row of the tile grid*/
   Eina_Rectangle rect; /**< rectangle of the tile grid, coordinates are
                             relative to tile*/
   Eina_Bool      full; /**< whether the grid is full or not */
};

typedef struct _Eina_Tile_Grid_Slicer Eina_Tile_Grid_Slicer;

/**
 * @brief Creates a new tiler with @p w width and @p h height.
 *
 * @param w Width of the tiler
 * @param h Height of the tiler
 * @return The newly created tiler
 *
 * @see eina_tiler_free()
 */
EAPI Eina_Tiler        *eina_tiler_new(int w, int h);
/**
 * @brief Frees a tiler.
 *
 * @param t The tiler to free.
 *
 * This function frees @p t. It does not free the memory allocated for the
 * elements of @p t.
 */
EAPI void               eina_tiler_free(Eina_Tiler *t);
/**
 * @brief Sets the size of tiles for a tiler.
 *
 * @param t The tiler whose tile size will be set.
 * @param w Width of the tiles.
 * @param h Height of the tiles.
 *
 * @warning @p w and @p h @b must be greater than zero, otherwise tile size
 * won't be changed.
 * @warning Tile size is not used!
 */
EAPI void               eina_tiler_tile_size_set(Eina_Tiler *t, int w, int h);

/**
 * @brief Change the size of the area covered by the tiler.
 *
 * @param t The tiler whose area size will be set.
 * @param w Width of the area.
 * @param h Height of the area.
 *
 * @since 1.8
 *
 * @warning Must clear the tiler before changing its size.
 */
EAPI void               eina_tiler_area_size_set(Eina_Tiler *t, int w, int h);

/**
 * @brief Get the size of the area covered by the tiler.
 *
 * @param t The tiler whose area size will be fetched.
 * @param w Width of the area.
 * @param h Height of the area.
 *
 * @since 1.8
 */
EAPI void               eina_tiler_area_size_get(const Eina_Tiler *t, int *w, int *h);

/**
 * @brief Define if we need to follow a strict grid of tile or a loosy one
 *
 * @param t The tiler to apply the strict rules to.
 * @param strict Define if it will be strict or loosy
 *
 * By default it will be loosy.
 *
 * @since 1.8
 */
EAPI void               eina_tiler_strict_set(Eina_Tiler *t, Eina_Bool strict);

/**
 * @brief Tell if a tiler is empty or not
 *
 * @param t The tiler to apply the strict rules to.
 * @return EINA_TRUE when empty, EINA_FALSE when not.
 *
 * @since 1.8
 */
EAPI Eina_Bool          eina_tiler_empty(Eina_Tiler *t);

/**
 * @brief Adds a rectangle to a tiler.
 *
 * @param t The tiler in which to add a container.
 * @param r The rectangle to be added.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @see eina_tiler_rect_del()
 */
EAPI Eina_Bool          eina_tiler_rect_add(Eina_Tiler *t, const Eina_Rectangle *r);
/**
 * @brief Removes a rectangle from a tiler.
 *
 * @param t The tiler in which to add a container.
 * @param r The rectangle to be removed.
 *
 * @see eina_tiler_rect_add()
 * @see eina_tiler_clear()
 */
EAPI void               eina_tiler_rect_del(Eina_Tiler *t, const Eina_Rectangle *r);
/**
 * @brief Removes all rectangles from tiles.
 *
 * @param t The tiler to clear.
 *
 * @see eina_tiler_rect_del()
 */
EAPI void               eina_tiler_clear(Eina_Tiler *t);
/**
 * @brief Create a iterator to access the tilers calculated rectangles.
 *
 * @param t The tiler to iterate over.
 * @return A iterator containing Eina_Rectangle.
 */
EAPI Eina_Iterator     *eina_tiler_iterator_new(const Eina_Tiler *t);

/**
 * @brief Creates a new Eina_Iterator that iterates over a list of tiles.
 *
 * @param   x X axis coordinate.
 * @param   y Y axis coordinate.
 * @param   w width.
 * @param   h height.
 * @param   tile_w tile width.
 * @param   tile_h tile height.
 * @return  A pointer to the Eina_Iterator. @c NULL on failure.
 *
 * The region defined by @a x, @a y, @a w, @a h will be divided in to a grid of
 * tiles of width @a tile_w and height @p tile_h, the returned iterator will
 * iterate over every tile in the grid having as its data a #Eina_Tile_Grid_Info.
 *
 * @note This is a convenience function, iterating over the returned iterator is
 * equivalent to calling eina_tile_grid_slicer_setup() and calling
 * eina_tile_grid_slicer_next() untill it returns #EINA_FALSE.
 */
EAPI Eina_Iterator     *eina_tile_grid_slicer_iterator_new(int x, int y, int w, int h, int tile_w, int tile_h);
/**
 * @brief Iterates over the tiles set by eina_tile_grid_slicer_setup().
 *
 * @param   slc Pointer to an Eina_Tile_Grid_Slicer struct.
 * @param   rect Pointer to a struct Eina_Tile_Grid_Info *.
 * @return  #EINA_TRUE if the current rect is valid. #EINA_FALSE if there
 * is no more rects to iterate over (and thus the current one isn't valid).
 *
 * This functions iterates over each Eina_Tile_Grid_Info *rect of the grid.
 * eina_tile_grid_slicer_setup() must be called first, and *rect is only valid
 * if this function returns #EINA_TRUE. Its content shouldn't be modified.
 *
 * @note Consider using eina_tile_grid_slicer_iterator_new() instead.
 */
static inline Eina_Bool eina_tile_grid_slicer_next(Eina_Tile_Grid_Slicer *slc, const Eina_Tile_Grid_Info **rect);
/**
 * @brief Setup an Eina_Tile_Grid_Slicer struct.
 *
 * @param   slc Pointer to an Eina_Tile_Grid_Slicer struct.
 * @param   x X axis coordinate.
 * @param   y Y axis coordinate.
 * @param   w width.
 * @param   h height.
 * @param   tile_w tile width.
 * @param   tile_h tile height.
 * @return  A pointer to the Eina_Iterator. @c NULL on failure.
 *
 * The region defined by @a x, @a y, @a w, @a h will be divided in to a grid of
 * tiles of width @a tile_w and height @p tile_h, @p slc can then be used with
 * eina_tile_grid_slicer_next() to access each tile.
 *
 * @note Consider using eina_tile_grid_slicer_iterator_new() instead.
 */
static inline Eina_Bool eina_tile_grid_slicer_setup(Eina_Tile_Grid_Slicer *slc, int x, int y, int w, int h, int tile_w, int tile_h);

#include "eina_inline_tiler.x"

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_TILER_H_ */

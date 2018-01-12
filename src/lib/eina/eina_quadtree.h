/* EINA - EFL data type library
 * Copyright (C) 2010 Cedric BAIL
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

#ifndef EINA_QUADTREE_H_
#define EINA_QUADTREE_H_

#include "eina_config.h"

#include "eina_inlist.h"

/**
 * @addtogroup Eina_Data_Types_Group Data Types
 *
 * @{
 */

/**
 * @typedef Eina_QuadTree
 *
 * A quadtree is a data structure where each node contains four child
 * nodes.  It can be used to partition 2D spaces through subdivision
 * into quadrants.
 */
typedef struct _Eina_QuadTree Eina_QuadTree;

/**
 * @typedef Eina_QuadTree_Item
 *
 * A quadtree item is a holder for (void *) data items inside a
 * quadtree, that includes some state tracking for lifecycle management
 * and optimization purposes.
 */
typedef struct _Eina_QuadTree_Item Eina_QuadTree_Item;

/**
 * @typedef Eina_Quad_Direction
 */
typedef enum {
   EINA_QUAD_LEFT,
   EINA_QUAD_RIGHT,
   EINA_QUAD_BOTH
} Eina_Quad_Direction;

/**
 * @typedef Eina_Quad_Callback
 *
 * Signature for a callback routine used to determine the location of an
 * object within a quadtree.  These are used in sorting by determining
 * where in the tree the given data @p object belongs, using @p middle
 * as the division line for the two halves of the space.
 */
typedef Eina_Quad_Direction (*Eina_Quad_Callback)(const void *object, size_t middle);

/**
 * @brief Constructs a quadtree object.
 *
 * @param[in] w The geometric width of the quadtree.
 * @param[in] h The geometric height of the quadtree.
 * @param[in] vertical The callback for vertical direction determination.
 * @param[in] horizontal The callback for horizontal direction determination.
 * @return The newly allocated and initialized quadtree, or @c NULL on error.
 *
 * The vertical and horizontal callbacks are used to assist in
 * determining which quadrant a given data item belongs to.
 */
EAPI Eina_QuadTree *eina_quadtree_new(size_t w, size_t h, Eina_Quad_Callback vertical, Eina_Quad_Callback horizontal);

/**
 * @brief Destructs quadtree and its data.
 *
 * @param[in] q The quadtree to be freed.
 *
 * Frees the memory for the Eina_QuadTree object, and any memory used by
 * its change tracking and garbage collection internals.
 */
EAPI void eina_quadtree_free(Eina_QuadTree *q);

/**
 * @brief Changes the width and height of the quadtree.
 *
 * @param[in,out] q The quadtree to resize.
 * @param[in] w The new geometric width for the quadtree.
 * @param[in] h The new geometric height for the quadtree.
 *
 * Sets the width and height of the quadtree, but the actual update is
 * done lazily.
 */
EAPI void eina_quadtree_resize(Eina_QuadTree *q, size_t w, size_t h);

/**
 * @brief Sets the quadtree's index to 0.
 *
 * @param[in,out] q The quadtree to cycle.
 */
EAPI void eina_quadtree_cycle(Eina_QuadTree *q);

/**
 * @brief Increases the index of the quadtree item by one.
 *
 * @param[in,out] object The quadtree item to increase.
 *
 * If necessary, records that the root is no longer sorted.
 */
EAPI void eina_quadtree_increase(Eina_QuadTree_Item *object);

/**
 * @brief Inserts a data object into the quadtree.
 *
 * @param[in,out] q The quadtree to add @p object to.
 * @param[in] object A data object to store in the quadtree.
 * @return Pointer to the stored quadtree item.
 *
 * Creates an Eina_QuadTree_Item (or recycles one from the quadtree's
 * trash) and stores the data @p object in it, then arranges to lazily
 * insert the item into the quadtree (i.e. insertion is delayed until
 * it needs to be used.)
 */
EAPI Eina_QuadTree_Item *eina_quadtree_add(Eina_QuadTree *q, const void *object);

/**
 * @brief Deletes a given quadtree item from the quadtree.
 *
 * @param[in] object The quadtree item to be deleted.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * Moves the item to the quadtree's internal garbage heap for later
 * reclamation.
 */
EAPI Eina_Bool eina_quadtree_del(Eina_QuadTree_Item *object);

/**
 * @brief Marks an object within the quadtree as needing changed.
 *
 * @param[in,out] object The object that has changed.
 * @return #EINA_TRUE if change successfully noted, or #EINA_FALSE otherwise.
 */
EAPI Eina_Bool eina_quadtree_change(Eina_QuadTree_Item *object);

/**
 * @brief Sets @p object invisible.
 *
 * @param[in,out] object The item within the quadtree to hide.
 * @return #EINA_TRUE if @p object was successfully hidden, or
 *         #EINA_FALSE if it wasn't in the quadtree.
 */
EAPI Eina_Bool eina_quadtree_hide(Eina_QuadTree_Item *object);

/**
 * @brief Sets @p object to visible.
 *
 * @param[in,out] object The item within the quadtree to show.
 * @return #EINA_TRUE if @p object was successfully shown, or
 *         #EINA_FALSE if it wasn't in the quadtree.
 */
EAPI Eina_Bool eina_quadtree_show(Eina_QuadTree_Item *object);

/**
 * @brief Retrieves items in quadtree inside the target geometry.
 *
 * @param[in,out] q The quadtree to recompute.
 * @param[in] x New target X coordinate.
 * @param[in] y New target Y coordinate.
 * @param[in] w New target width.
 * @param[in] h New target height.
 * @return The list of collided items or @c NULL on error.
 *
 * Forces a rebuild and resort of the quadtree if needed due to pending
 * changes, then performs a collision detection to find items whose
 * geometry is contained within or intersects the given target geometry.
 */
EAPI Eina_Inlist *eina_quadtree_collide(Eina_QuadTree *q, int x, int y, int w, int h);

/**
 * @brief Retrieves the quadtree item's data for the given inline list.
 *
 * @param[in] list The inline list item to lookup
 * @return The contained data object in the Eina_QuadTree_Item, or @c
 *         NULL if none could be found.
 */
EAPI void *eina_quadtree_object(Eina_Inlist *list);

/**
 * @}
 */

#endif

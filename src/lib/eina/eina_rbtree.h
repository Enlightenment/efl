/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric Bail
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

#ifndef EINA_RBTREE_H__
#define EINA_RBTREE_H__

#include <stdlib.h>

#include "eina_types.h"
#include "eina_error.h"
#include "eina_iterator.h"

/**
 * @defgroup Eina_Rbtree_Group Red-Black tree
 * @ingroup Eina_Containers_Group
 *
 * @brief This group discusses the functions that provide Red-Black trees management.
 *
 * For a brief description look at http://en.wikipedia.org/wiki/Red-black_tree .
 * This code is largely inspired from a tutorial written by Julienne Walker at :
 * http://eternallyconfuzzled.com/tuts/datastructures/jsw_tut_rbtree.aspx . The
 * main difference is that this set of functions never allocate any data, making
 * them particularly useful for memory management.
 *
 * @{
 */

/**
 * @typedef Eina_Rbtree_Color
 * @brief Enumeration of the node color.
 */
typedef enum {
   EINA_RBTREE_RED,
   EINA_RBTREE_BLACK
} Eina_Rbtree_Color;

/**
 * @typedef Eina_Rbtree_Direction
 * @brief Enumeration of the walk direction.
 */
typedef enum {
   EINA_RBTREE_LEFT = 0,
   EINA_RBTREE_RIGHT = 1
} Eina_Rbtree_Direction;

/**
 * @typedef Eina_Rbtree
 * @brief The structure type for a Red-Black tree node. It should be inlined into the user type.
 */
typedef struct _Eina_Rbtree Eina_Rbtree;
struct _Eina_Rbtree
{
   Eina_Rbtree      *son[2];

   Eina_Rbtree_Color color : 1;
};

/**
 * @def EINA_RBTREE
 * @brief Definition of the recommended way to declare the inlined Eina_Rbtree for your type.
 *
 * @code
 * struct my_type {
 *    EINA_RBTREE;
 *    int my_value;
 *    char *my_name;
 * };
 * @endcode
 *
 * @see EINA_RBTREE_GET()
 */
#define EINA_RBTREE Eina_Rbtree __rbtree

/**
 * @def EINA_RBTREE_GET
 *
 * @since_tizen 2.3
 *
 * @brief Definition to access the inlined node if it is created with #EINA_RBTREE.
 */
#define EINA_RBTREE_GET(Rbtree) (&((Rbtree)->__rbtree))

/**
 * @def EINA_RBTREE_CONTAINER_GET
 *
 * @since_tizen 2.3
 *
 * @brief Definition to find back the container of a red black tree.
 */
#define EINA_RBTREE_CONTAINER_GET(Ptr, Type) ((Type *)((char *)Ptr - offsetof(Type, __rbtree)))

/**
 * @typedef Eina_Rbtree_Cmp_Node_Cb
 * @brief Compares two nodes and decides which direction to navigate.
 */
typedef Eina_Rbtree_Direction (*Eina_Rbtree_Cmp_Node_Cb)(const Eina_Rbtree *left, const Eina_Rbtree *right, void *data);

/**
 * @def EINA_RBTREE_CMP_NODE_CB
 * @brief Definition of the cast using #Eina_Rbtree_Cmp_Node_Cb.
 */
#define EINA_RBTREE_CMP_NODE_CB(Function) ((Eina_Rbtree_Cmp_Node_Cb)Function)

/**
 * @typedef Eina_Rbtree_Cmp_Key_Cb
 * @brief Compares a node with the given key of the specified length.
 */
typedef int (*Eina_Rbtree_Cmp_Key_Cb)(const Eina_Rbtree *node, const void *key, int length, void *data);
/**
 * @def EINA_RBTREE_CMP_KEY_CB
 * @brief Definition of the cast using #Eina_Rbtree_Cmp_Key_Cb.
 */
#define EINA_RBTREE_CMP_KEY_CB(Function) ((Eina_Rbtree_Cmp_Key_Cb)Function)

/**
 * @typedef Eina_Rbtree_Free_Cb
 * @brief Called to free a node.
 */
typedef void (*Eina_Rbtree_Free_Cb)(Eina_Rbtree *node, void *data);
/**
 * @def EINA_RBTREE_FREE_CB
 * @brief Definition of the cast using #Eina_Rbtree_Free_Cb.
 */
#define EINA_RBTREE_FREE_CB(Function) ((Eina_Rbtree_Free_Cb)Function)


/**
 * @brief Inserts a new node inside an existing red black tree.
 *
 * @details This function inserts a new node into a valid red black tree. @c NULL is
 *          an empty valid red black tree. The resulting new tree is a valid red
 *          black tree. This function doesn't allocate any data.
 *
 * @since_tizen 2.3
 *
 * @param[in] root The root of an existing valid red black tree
 * @param[in] node The new node to insert
 * @param[in] cmp The callback that is able to compare two nodes
 * @param[in] data The private data to help the compare function
 * @return The new root of the red black tree
 *
 */
EAPI Eina_Rbtree          *eina_rbtree_inline_insert(Eina_Rbtree *root, Eina_Rbtree *node, Eina_Rbtree_Cmp_Node_Cb cmp, const void *data) EINA_ARG_NONNULL(2, 3) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Removes a node from an existing red black tree.
 *
 * @details This function removes a new node from a valid red black tree that should
 *          contain the node that you are removing. This function returns @c NULL
 *          when the red black tree becomes empty. This function doesn't free any data.
 *
 * @since_tizen 2.3
 *
 * @param[in] root The root of a valid red black tree
 * @param[in] node The node to remove from the tree
 * @param[in] cmp The callback that is able to compare two nodes
 * @param[in] data The private data to help the compare function
 * @return The new root of the red black tree
 *
 */
EAPI Eina_Rbtree          *eina_rbtree_inline_remove(Eina_Rbtree *root, Eina_Rbtree *node, Eina_Rbtree_Cmp_Node_Cb cmp, const void *data) EINA_ARG_NONNULL(2, 3) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Deletes all the nodes from a valid red black tree.
 *
 * @since_tizen 2.3
 *
 * @param[in] root The root of a valid red black tree
 * @param[in] func The callback that frees each node
 * @param[in] data The private data to help the compare function
 *
 */
EAPI void                  eina_rbtree_delete(Eina_Rbtree *root, Eina_Rbtree_Free_Cb func, void *data) EINA_ARG_NONNULL(2);

static inline Eina_Rbtree *eina_rbtree_inline_lookup(const Eina_Rbtree *root, const void *key, int length, Eina_Rbtree_Cmp_Key_Cb cmp, const void *data) EINA_PURE EINA_ARG_NONNULL(2, 4) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Returns a new prefix iterator associated to an rbtree.
 *
 * @details This function returns a newly allocated iterator associated to
 *          @a root. It iterates the tree using prefix walk. If @a root is
 *          @c NULL, this function still returns a valid iterator that always
 *          returns @c false on eina_iterator_next(), thus keeping the API sane.
 *
 * @since_tizen 2.3
 *
 * @remarks If the memory cannot be allocated, @c NULL is returned
 *          and #EINA_ERROR_OUT_OF_MEMORY is set. Otherwise, a valid iterator is
 *          returned.
 *
 * @remarks If the rbtree structure changes then the iterator becomes
 *          invalid. That is, if you add or remove nodes this iterator's
 *          behavior is undefined and your program may crash.
 *
 * @param[in] root The root of the rbtree
 * @return A new iterator
 *
 */
EAPI Eina_Iterator        *eina_rbtree_iterator_prefix(const Eina_Rbtree *root) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Returns a new prefix iterator associated to an rbtree.
 *
 * @details This function returns a newly allocated iterator associated to
 *          @a root. It iterates the tree using infix walk. If @a root is
 *          @c NULL, this function still returns a valid iterator that always
 *          returns @c false on eina_iterator_next(), thus keeping the API sane.
 *
 * @since_tizen 2.3
 *
 * @remarks If the memory cannot be allocated, @c NULL is returned
 *          and #EINA_ERROR_OUT_OF_MEMORY is set. Otherwise, a valid iterator is
 *          returned.
 *
 * @remarks If the rbtree structure changes then the iterator becomes
 *          invalid. That is, if you add or remove nodes this iterator's
 *          behavior is undefined and your program may crash.
 *
 * @param[in] root The root of the rbtree
 * @return A new iterator
 *
 */
EAPI Eina_Iterator        *eina_rbtree_iterator_infix(const Eina_Rbtree *root) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Returns a new prefix iterator associated to an rbtree.
 *
 * @details This function returns a newly allocated iterator associated to
 *          @a root. It iterates the tree using postfix walk. If @a root is
 *          @c NULL, this function still returns a valid iterator that always
 *          returns @c false on eina_iterator_next(), thus keeping the API sane.
 *
 * @since_tizen 2.3
 *
 * @remarks If the memory cannot be allocated, @c NULL is returned
 *          and #EINA_ERROR_OUT_OF_MEMORY is set. Otherwise, a valid iterator is
 *          returned.
 *
 * @remarks If the rbtree structure changes then the iterator becomes
 *          invalid. That is, if you add or remove nodes this iterator's
 *          behavior is undefined and your program may crash.
 *
 * @param[in] root The root of the rbtree
 * @return A new iterator
 *
 */
EAPI Eina_Iterator        *eina_rbtree_iterator_postfix(const Eina_Rbtree *root) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

#include "eina_inline_rbtree.x"

/**
 * @}
 */

#endif

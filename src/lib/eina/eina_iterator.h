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

#ifndef EINA_ITERATOR_H__
#define EINA_ITERATOR_H__

#include "eina_config.h"

#include "eina_types.h"
#include "eina_magic.h"

/**
 * @defgroup Eina_Iterator_Group Iterator Functions
 * @ingroup Eina_Content_Access_Group Content
 *
 * @brief This group of functions manages iterators on containers.
 *
 * These functions allow to access elements of a container in a
 * generic way, without knowing which container is used (a bit like
 * iterators in the C++ STL). Iterators only allow sequential access
 * (that is, from one element to the next one). For random access, see
 * @ref Eina_Accessor_Group.
 *
 * Getting an iterator to access elements of a given container is done through
 * the functions of that particular container. There is no function to create
 * a generic iterator as iterators absolutely depend on the container. This
 * means you won't find a iterator creation function here, those can be found on
 * the documentation of the container type you're using. Though created with
 * container specific functions iterators are always deleted with the same
 * function: eina_iterator_free().
 *
 * To get data and iterate it, use eina_iterator_next(). To call a function on
 * all the elements of a container, use eina_iterator_foreach().
 *
 * @{
 */

/**
 * @typedef Eina_Iterator
 * @brief The structure type containing the abstract type for iterators.
 */
typedef struct _Eina_Iterator Eina_Iterator;

/**
 * @typedef Eina_Iterator_Next_Callback
 * @brief The boolean type for a callback that returns the next element in a container.
 */
typedef Eina_Bool           (*Eina_Iterator_Next_Callback)(Eina_Iterator *it, void **data);

/**
 * @typedef Eina_Iterator_Get_Container_Callback
 * @brief Called to return the container.
 */
typedef void               *(*Eina_Iterator_Get_Container_Callback)(Eina_Iterator *it);

/**
 * @typedef Eina_Iterator_Free_Callback
 * @brief Called to free the container.
 */
typedef void                (*Eina_Iterator_Free_Callback)(Eina_Iterator *it);

/**
 * @typedef Eina_Iterator_Lock_Callback
 * @brief Called to lock the container.
 */
typedef Eina_Bool           (*Eina_Iterator_Lock_Callback)(Eina_Iterator *it);

/**
 * @struct _Eina_Iterator
 * @brief The structure type of an iterator.
 *
 * @internal
 * @remarks When creating an iterator remember to set the type using #EINA_MAGIC_SET.
 * @endinternal
 */
struct _Eina_Iterator
{
#define EINA_ITERATOR_VERSION 1
   int                                  version; /**< Version of the Iterator API */

   Eina_Iterator_Next_Callback          next          EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT; /**< Callback called when the next element is requested */
   Eina_Iterator_Get_Container_Callback get_container EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT; /**< Callback called when the container is requested */
   Eina_Iterator_Free_Callback          free          EINA_ARG_NONNULL(1); /**< Callback called when the container is freed */

   Eina_Iterator_Lock_Callback          lock          EINA_WARN_UNUSED_RESULT; /**< Callback called when the container is locked */
   Eina_Iterator_Lock_Callback          unlock        EINA_WARN_UNUSED_RESULT; /**< Callback called when the container is unlocked */

#define EINA_MAGIC_ITERATOR 0x98761233
   EINA_MAGIC
};

/**
 * @def FUNC_ITERATOR_NEXT(Function)
 * @brief Definition of a helper macro to cast @a Function to a Eina_Iterator_Next_Callback.
 */
#define FUNC_ITERATOR_NEXT(Function)          ((Eina_Iterator_Next_Callback)Function)

/**
 * @def FUNC_ITERATOR_GET_CONTAINER(Function)
 * @brief Definition of a helper macro to cast @a Function to a Eina_Iterator_Get_Container_Callback.
 */
#define FUNC_ITERATOR_GET_CONTAINER(Function) ((Eina_Iterator_Get_Container_Callback)Function)

/**
 * @def FUNC_ITERATOR_FREE(Function)
 * @brief Definition of a helper macro to cast @a Function to a Eina_Iterator_Free_Callback.
 */
#define FUNC_ITERATOR_FREE(Function)          ((Eina_Iterator_Free_Callback)Function)

/**
 * @def FUNC_ITERATOR_LOCK(Function)
 * @brief Definition of a helper macro to cast @a Function to a Eina_Iterator_Lock_Callback.
 */
#define FUNC_ITERATOR_LOCK(Function)          ((Eina_Iterator_Lock_Callback)Function)


/**
 * @brief Frees an iterator.
 *
 * @details This function frees @a iterator if it is not @c NULL.
 *
 * @since_tizen 2.3
 *
 * @param[in] iterator The iterator to free
 *
 */
EAPI void      eina_iterator_free(Eina_Iterator *iterator);


/**
 * @brief Gets the container of an iterator.
 *
 * @details This function returns the container that created @a iterator. If
 *          @a iterator is @c NULL, this function returns @c NULL.
 *
 * @since_tizen 2.3
 *
 * @param[in] iterator The iterator
 * @return The container that created the iterator
 *
 */
EAPI void     *eina_iterator_container_get(Eina_Iterator *iterator) EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * @brief Returns the value of the current element and goes to the next one.
 *
 * @details This function returns the value of the current element pointed by
 *          @a iterator in @a data and then goes to the next element. If @a
 *          iterator is @c NULL or if a problem occurs, @c EINA_FALSE is
 *          returned, otherwise @c EINA_TRUE is returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] iterator The iterator
 * @param[in] data The data of the element
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool eina_iterator_next(Eina_Iterator *iterator,
                                  void         **data) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Iterates over the container and executes a callback on each element.
 *
 * @details This function iterates over the elements pointed by @a iterator,
 *          beginning from the current element. For each element, the callback
 *          @a cb is called with the data @a fdata. If @a iterator is @c NULL,
 *          the function returns immediately. Also, if @a cb returns @c EINA_FALSE,
 *          the iteration stops at that point. If @a cb returns @c EINA_TRUE
 *          the iteration continues.
 *
 * @since_tizen 2.3
 *
 * @param[in] iterator The iterator
 * @param[in] callback The callback called on each iteration
 * @param[in] fdata The data passed to the callback
 *
 */
EAPI void eina_iterator_foreach(Eina_Iterator *iterator,
                                Eina_Each_Cb   callback,
                                const void    *fdata) EINA_ARG_NONNULL(2);


/**
 * @brief Locks the container of the iterator.
 *
 * @since_tizen 2.3
 *
 * @remarks If the container of the @a iterator permits it, it is locked. When a
 *          container is locked by calling eina_iterator_foreach() on it, it returns
 *          immediately. If @a iterator is @c NULL or if a problem occurs, @c EINA_FALSE
 *          is returned, otherwise @c EINA_TRUE is returned. If the container isn't
 *          lockable, it returns @c EINA_TRUE.
 *
 * @remarks None of the existing eina data structures are lockable.
 *
 * @param[in] iterator The iterator
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool eina_iterator_lock(Eina_Iterator *iterator) EINA_ARG_NONNULL(1);

/**
 * @brief Unlocks the container of the iterator.
 *
 * @since_tizen 2.3
 *
 * @remarks If the container of the @a iterator permits it and is previously
 *          locked, it is unlocked. If @a iterator is @c NULL or if a
 *          problem occurs, @c EINA_FALSE is returned, otherwise @c EINA_TRUE
 *          is returned. If the container is not lockable, it
 *          returns @c EINA_TRUE.
 *
 * @remarks None of the existing eina data structures are lockable.
 *
 * @param[in] iterator The iterator
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool eina_iterator_unlock(Eina_Iterator *iterator) EINA_ARG_NONNULL(1);

/**
 * @def EINA_ITERATOR_FOREACH
 * @brief Definition of the macro to iterate over all the elements easily.
 *
 * @since_tizen 2.3
 *
 * @details This macro is a convenient way to use iterators, very similar to
 *          EINA_LIST_FOREACH().
 *
 * @remarks This macro can be used for freeing the data of a list, like in the
 *          following example. It has the same goal as the one documented in
 *          EINA_LIST_FOREACH(), but using iterators:
 *
 * @code
 * Eina_List     *list;
 * Eina_Iterator *itr;
 * char          *data;
 *
 * // list is already filled,
 * // its elements are just duplicated strings
 *
 * itr = eina_list_iterator_new(list);
 * EINA_ITERATOR_FOREACH(itr, data)
 *   free(data);
 * eina_iterator_free(itr);
 * eina_list_free(list);
 * @endcode
 *
 * @remarks This example is not an optimal algorithm to release a list as
 *          it walks through the list twice, but it serves as an example. For an
 *          optimized version use EINA_LIST_FREE().
 *
 * @remarks The order in which the elements are traversed depends on the
 *          underlying container and @b shouldn't be relied upon.
 *
 * @remarks Unless explicitly stated in the function's returning iterators,
 *          do not modify the iterated object while you walk through it. In this
 *          example using lists, do not remove the list nodes or the program might
 *          crash. This is not a limitation of the iterators themselves,
 *          but a limitation in the iterators implementations to keep them as simple
 *          and fast as possible.
 *
 * @param itr The iterator to use
 * @param data A pointer to store the data \n
 *             It must be a pointer to support getting
 *             its address since eina_iterator_next() requires a pointer.
 *
 */
#define EINA_ITERATOR_FOREACH(itr,                                   \
                              data) while (eina_iterator_next((itr), \
                                                              (void **)(void *)&(data)))

/**
 * @}
 */

#endif

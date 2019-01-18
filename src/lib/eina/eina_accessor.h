/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric Bail
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

#ifndef EINA_ACCESSOR_H__
#define EINA_ACCESSOR_H__

#include "eina_config.h"

#include "eina_types.h"
#include "eina_magic.h"

/**
 * @page eina_accessor_example_01_page Eina_Accessor usage
 * @dontinclude eina_accessor_01.c
 *
 * We start by including necessary headers, declaring variables, and
 * initializing eina:
 * @skip #include
 * @until eina_init
 *
 * Next we populate our array and list:
 * @until }
 *
 * Now that we have two containers populated we can actually start the example
 * and create an accessor:
 * @until accessor_new
 *
 * Once we have the accessor we can use it to access certain elements in the
 * container:
 * @until }
 * @note Unlike iterators accessors allow us non-linear access, which allows us
 * to print only the odd elements in the container.
 *
 * As with every other resource we allocate we need to free the accessor(and the
 * array):
 * @until array_free
 *
 * Now we create another accessor, this time for the list:
 * @until accessor_new
 *
 * And now the interesting part, we use the same code we used above, to print
 * parts of the array, to print parts of the list:
 * @until }
 *
 * And to free the list we use a gimmick, instead of freeing @a list, we ask the
 * accessor for its container and we free that:
 * @until list_free
 *
 * Finally we shut eina down and leave:
 * @until }
 *
 * The full source code can be found in the examples folder
 * in the @ref eina_accessor_01_c "eina_accessor_01.c" file.
 */

/**
 * @page eina_accessor_01_c Eina_Accessor usage example
 *
 * @include eina_accessor_01.c
 * @example eina_accessor_01.c
 */

/**
 * @defgroup Eina_Accessor_Group Accessor Functions
 * @ingroup Eina_Content_Access_Group
 *
 * @brief This group discusses the functions to manage an accessor on containers.
 *
 * These functions allow to access elements of a container in a
 * generic way, without knowing which container is used (a bit like
 * iterators in the C++ STL). Accessors allows random access (that is, any
 * element in the container). For sequential access, see
 * @ref Eina_Iterator_Group.
 *
 * Getting an accessor to access elements of a given container is done through
 * the functions of that particular container. There is no function to create
 * a generic accessor as accessors absolutely depend on the container. This
 * means you won't find an accessor creation function here, those can be found on
 * the documentation of the container type you're using. Though created with
 * container specific functions, accessors are always deleted by the same
 * function: eina_accessor_free().
 *
 * To get the data of an element at a given
 * position, use eina_accessor_data_get(). To call a function on
 * chosen elements of a container, use eina_accessor_over().
 *
 * See an example @ref eina_accessor_example_01_page "here".
 *
 * @{
 */

/**
 * @typedef Eina_Accessor
 * @brief Type for a accessor structure.
 */
typedef struct _Eina_Accessor Eina_Accessor;

/**
 * @typedef Eina_Accessor_Get_At_Callback
 * @brief Type for a callback that returns the data of a container as the given index.
 */
typedef Eina_Bool (*Eina_Accessor_Get_At_Callback)(Eina_Accessor *it,
                                                   unsigned int   idx,
                                                   void         **data);

/**
 * @typedef Eina_Accessor_Get_Container_Callback
 * @brief Type for a callback to return the container of the accessor.
 */
typedef void *(*Eina_Accessor_Get_Container_Callback)(Eina_Accessor *it);

/**
 * @typedef Eina_Accessor_Free_Callback
 * @brief Type for a callback to free the accessor.
 */
typedef void (*Eina_Accessor_Free_Callback)(Eina_Accessor *it);

/**
 * @typedef Eina_Accessor_Lock_Callback
 * @brief Type for a callback to lock the accessor.
 */
typedef Eina_Bool (*Eina_Accessor_Lock_Callback)(Eina_Accessor *it);

/**
 * @typedef Eina_Accessor_Clone_Callback
 * @brief Type for a callback to return a clone for the accessor.
 * @since 1.10
 */
typedef Eina_Accessor* (*Eina_Accessor_Clone_Callback)(Eina_Accessor *it);

/**
 * @struct _Eina_Accessor
 * @brief Structure to provide random access to data structures.
 *
 * @note When creating an accessor remember to set the type using #EINA_MAGIC_SET
 */
struct _Eina_Accessor
{
#define EINA_ACCESSOR_VERSION 2
   int                                  version; /**< Version of the Accessor API */

   Eina_Accessor_Get_At_Callback        get_at        EINA_ARG_NONNULL(1, 3) EINA_WARN_UNUSED_RESULT; /**< Callback called when a data element is requested */
   Eina_Accessor_Get_Container_Callback get_container EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT; /**< Callback called when the container is requested */
   Eina_Accessor_Free_Callback          free          EINA_ARG_NONNULL(1); /**< Callback called when the container is freed */

   Eina_Accessor_Lock_Callback          lock          EINA_WARN_UNUSED_RESULT; /**< Callback called when the container is locked */
   Eina_Accessor_Lock_Callback          unlock        EINA_WARN_UNUSED_RESULT; /**< Callback called when the container is unlocked */

#define EINA_MAGIC_ACCESSOR 0x98761232
   EINA_MAGIC

   Eina_Accessor_Clone_Callback         clone         EINA_WARN_UNUSED_RESULT; /**< Callback called when the container is to be cloned. @since 1.10 */
};

/**
 * @def FUNC_ACCESSOR_GET_AT(Function)
 * @brief Definition for helper macro to cast @a Function to a Eina_Accessor_Get_At_Callback.
 */
#define FUNC_ACCESSOR_GET_AT(Function)        ((Eina_Accessor_Get_At_Callback)Function)

/**
 * @def FUNC_ACCESSOR_GET_CONTAINER(Function)
 * @brief Definition for helper macro to cast @a Function to a Eina_Accessor_Get_Container_Callback.
 */
#define FUNC_ACCESSOR_GET_CONTAINER(Function) ((Eina_Accessor_Get_Container_Callback)Function)

/**
 * @def FUNC_ACCESSOR_FREE(Function)
 * @brief Definition for helper macro to cast @a Function to a Eina_Accessor_Free_Callback.
 */
#define FUNC_ACCESSOR_FREE(Function)          ((Eina_Accessor_Free_Callback)Function)

/**
 * @def FUNC_ACCESSOR_LOCK(Function)
 * @brief Definition for helper macro to cast @a Function to a Eina_Iterator_Lock_Callback.
 */
#define FUNC_ACCESSOR_LOCK(Function)          ((Eina_Accessor_Lock_Callback)Function)

/**
 * @def FUNC_ACCESSOR_CLONE(Function)
 * @brief Definition for helper macro to cast @p Function to a Eina_Iterator_Clone_Callback.
 * @since 1.10
 */
#define FUNC_ACCESSOR_CLONE(Function)          ((Eina_Accessor_Clone_Callback)Function)


/**
 * @brief Frees an accessor.
 * @details This function frees @a accessor if it is not @c NULL.
 * @param[in] accessor The accessor to free
 *
 */
EAPI void      eina_accessor_free(Eina_Accessor *accessor);

/**
 * @brief Gets the data of an accessor at the given position.
 * @details This function retrieves the data of the element pointed by
 *          @p accessor at the position @p position, and stores it in
 *          @p data. If @p accessor is @c NULL or if an error occurs, @c EINA_FALSE
 *          is returned, otherwise @c EINA_TRUE is returned.
 * @param[in] accessor The accessor
 * @param[in] position The position of the element
 * @param[in] data The pointer that stores the data to retrieve
 * @return #EINA_TRUE on success, otherwise #EINA_FALSE
 *
 */
EAPI Eina_Bool eina_accessor_data_get(Eina_Accessor *accessor,
                                      unsigned int   position,
                                      void         **data) EINA_ARG_NONNULL(1);

/**
 * @brief Gets the container of an accessor.
 * @details This function returns the container that created @p accessor. If
 *          @p accessor is @c NULL, this function returns @c NULL.
 * @param[in] accessor The accessor
 * @return The container that created the accessor
 *
 */
EAPI void *eina_accessor_container_get(Eina_Accessor *accessor) EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * @brief Iterates over the container and executes a callback on the chosen elements.
 * @details This function iterates over the elements pointed by @p accessor,
 *          starting from the element at position @p start and ending at the
 *          element at position @p end. For each element, the callback
 *          @p cb is called with the data @p fdata. If @p accessor is @c NULL
 *          or if @p start is greater than or equal to @p end, the function returns
 *          immediately.
 *
 * @param[in] accessor The accessor
 * @param[in] cb The callback called on the chosen elements
 * @param[in] start The position of the first element
 * @param[in] end The position of the last element
 * @param[in] fdata The data passed to the callback
 *
 */
EAPI void  eina_accessor_over(Eina_Accessor *accessor,
                              Eina_Each_Cb   cb,
                              unsigned int   start,
                              unsigned int   end,
                              const void    *fdata) EINA_ARG_NONNULL(2);

/**
 * @brief Locks the container of the accessor.
 *
 * @param[in] accessor The accessor
 * @return #EINA_TRUE on success, otherwise #EINA_FALSE
 *
 * @note If the container of the @p accessor permits it, it is locked. When a
 *       container is locked by calling eina_accessor_over() on it, it returns
 *       immediately. If @p accessor is @c NULL or if a problem occurs, #EINA_FALSE
 *       is returned, otherwise #EINA_TRUE is returned. If the container is not
 *       lockable, it returns #EINA_TRUE.
 *
 * @warning None of the existing eina data structures are lockable.
 */
EAPI Eina_Bool eina_accessor_lock(Eina_Accessor *accessor) EINA_ARG_NONNULL(1);

/**
 * @brief Clones the container of the accessor.
 *
 * @param[in] accessor The accessor.
 * @return Another accessor
 * @since 1.10
 */
EAPI Eina_Accessor* eina_accessor_clone(Eina_Accessor *accessor) EINA_ARG_NONNULL(1);

/**
 * @brief Unlocks the container of the accessor.
 *
 * @param[in] accessor The accessor
 * @return #EINA_TRUE on success, otherwise #EINA_FALSE
 *
 * @note If the container of the @p accessor permits it and is previously
 *       locked, it is unlocked. If @p accessor is @c NULL or if a
 *       problem occurs, #EINA_FALSE is returned, otherwise #EINA_TRUE
 *       is returned. If the container is not lockable, it returns #EINA_TRUE.
 *
 * @warning None of the existing eina data structures are lockable.
 */
EAPI Eina_Bool eina_accessor_unlock(Eina_Accessor *accessor) EINA_ARG_NONNULL(1);

/**
 * @def EINA_ACCESSOR_FOREACH
 * @brief Definition for helper macro to iterate over all the elements easily.
 * @details This macro allows a convenient way to loop over all elements in an
 *          accessor, very similar to EINA_LIST_FOREACH().
 *
 * @param[in] accessor The accessor to use
 * @param[out] counter A counter used by eina_accessor_data_get() when
 *             iterating over the container.
 * @param[out] data A pointer to store the data \n
 *             It must be a pointer to support getting
 *             its address since eina_accessor_data_get() requires a pointer.
 *
 * This macro can be used for freeing the data of a list, like in the
 * following example. It has the same goal as the one documented in
 * EINA_LIST_FOREACH(), but using accessors:
 *
 * @code
 * Eina_List     *list;
 * Eina_Accessor *accessor;
 * unsigned int   i;
 * char          *data;
 *
 * // list is already filled,
 * // its elements are just duplicated strings
 *
 * accessor = eina_list_accessor_new(list);
 * EINA_ACCESSOR_FOREACH(accessor, i, data)
 *   free(data);
 * eina_accessor_free(accessor);
 * eina_list_free(list);
 * @endcode
 *
 * @note If the data type provides both iterators and accessors, prefer
 *       to use iterators to iterate over, as they're likely to be more
 *       optimized for such a task.
 *
 * @note This example is not an optimal algorithm to release a list as
 *       it walks through the list twice, but it serves as an example. For an
 *       optimized version use EINA_LIST_FREE().
 *
 * @warning Unless explicitly stated in the function's returning accessors,
 *          do not modify the accessed object while you walk through it. In this
 *          example using lists, do not remove list nodes or the program might
 *          crash. This is not a limitation of the accessors themselves,
 *          but a limitation in the accessors implementations to keep them as simple
 *          and fast as possible.
 */
#define EINA_ACCESSOR_FOREACH(accessor, counter, data)                  \
  for ((counter) = 0;                                                   \
       eina_accessor_data_get((accessor), (counter), (void **)(void *)&(data)); \
       (counter)++)

/**
 * @}
 */

#endif

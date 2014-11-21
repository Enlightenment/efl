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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EINA_ARRAY_H_
#define EINA_ARRAY_H_

#include <stdlib.h>

#include "eina_config.h"

#include "eina_types.h"
#include "eina_error.h"
#include "eina_iterator.h"
#include "eina_accessor.h"
#include "eina_magic.h"

/**
 * @defgroup Eina_Array_Group Array
 * @ingroup Eina_Containers_Group
 *
 * @brief This group discusses the functions to provide array management.
 *
 * The Array data type in Eina is designed to have very fast access to
 * its data (compared to the Eina @ref Eina_List_Group). On the other hand,
 * data can be added or removed only at the end of the array. To insert
 * data at any place, the Eina @ref Eina_List_Group is the correct container
 * to use.
 *
 * To use the array data type, eina_init() must be called before any
 * other array function. When no more eina array functions are used,
 * eina_shutdown() must be called to free all the resources.
 *
 * An array must be created with eina_array_new(). It allocates all
 * the necessary data for an array. When not needed anymore, an array
 * is freed with eina_array_free(). This function does not free any
 * allocated memory used to store the data of each element. For that,
 * just iterate over the array to free them. A convenient way to do
 * that is by using #EINA_ARRAY_ITER_NEXT. An example of code is given
 * in the description of this macro.
 *
 * Functions do not check if the used array is valid or not. It's up to
 * the user to be sure of that. It is designed like that for performance
 * reasons.
 *
 * The usual features of an array are classic ones: to append an
 * element, use eina_array_push() and to remove the last element, use
 * eina_array_pop(). To retrieve the element at a given position, use
 * eina_array_data_get(). The number of elements can be retrieved with
 * eina_array_count().
 *
 * Eina_Array is different from a conventional C array in a number of ways, most
 * importantly they grow and shrink dynamically, this means that if you add an
 * element to a full array it grows and when you remove an element from an
 * array it @b may shrink.
 *
 * When the array needs to grow it allocates memory not just for the element
 * currently being added, since that would mean allocating memory(which is
 * computationally expensive) often, instead it grows to be able to hold @a step
 * more elements. Similarly, if you remove elements in such a way that the
 * array is left holding its capacity, @a step elements shrink.
 *
 * The following image illustrates how an Eina_Array grows:
 *
 * @image html eina_array-growth.png
 * @image latex eina_array-growth.eps "eina array growth" width=\textwidth
 *
 * @remarks Eina_Array only stores pointers, but it can store data of any type in the form
 *          of void pointers.
 *
 * @{
 */

/**
 * @typedef Eina_Array
 * @brief The structure type for a generic vector.
 */
typedef struct _Eina_Array Eina_Array;

/**
 * @typedef Eina_Array_Iterator
 * @brief The structure type for an iterator on arrays, used with #EINA_ARRAY_ITER_NEXT.
 */
typedef void **Eina_Array_Iterator;

/**
 * @struct _Eina_Array
 * @brief The structure type for an array of data.
 */
struct _Eina_Array
{
#define EINA_ARRAY_VERSION 1
   int          version; /**< Should match EINA_ARRAY_VERSION used when compiling your apps, provided for ABI compatibility */

   void       **data; /**< Pointer to a vector of a pointer to payload */
   unsigned int total; /**< Total number of slots in the vector */
   unsigned int count; /**< Number of active slots in the vector */
   unsigned int step; /**< To what extent must we grow the vector when it is full */
   EINA_MAGIC
};


/**
 * @brief Creates a new array.
 *
 * @details This function creates a new array. When adding an element, the array
 *          allocates @a step elements. When that buffer is full, adding
 *          another element increases the buffer by @a step elements again.
 *
 * @since_tizen 2.3
 *
 * @remarks This function returns a valid array on success, or @c NULL if memory
 *          allocation fails. In that case, the error is set
 *          to #EINA_ERROR_OUT_OF_MEMORY.
 *
 * @param[in] step The count of pointers to add when increasing the array size
 * @return @c NULL on failure, otherwise a non @c NULL value
 *
 */
EAPI Eina_Array *eina_array_new(unsigned int step) EINA_WARN_UNUSED_RESULT EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Frees an array.
 *
 * @details This function frees @a array. It calls first eina_array_flush() then
 *          frees the memory of the pointer. It does not free the memory
 *          allocated for the elements of @a array. To free them,
 *          use #EINA_ARRAY_ITER_NEXT. For performance reasons, there is no check
 *          on @a array.
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array to free
 *
 */
EAPI void        eina_array_free(Eina_Array *array) EINA_ARG_NONNULL(1);

/**
 * @brief Sets the step of an array.
 *
 * @details This function sets the step of @a array to @a step. For performance
 *          reasons, there is no check on @a array. If it is @c NULL or
 *          invalid, the program may crash.
 *
 * @since_tizen 2.3
 *
 * @remarks This function can @b only be called on uninitialized arrays.
 *
 * @param[in] array The array
 * @param[in] sizeof_eina_array The value returned by sizeof(Eina_Array)
 * @param[in] step The count of pointers to add when increasing the array size
 *
 */
EAPI void        eina_array_step_set(Eina_Array  *array,
                                     unsigned int sizeof_eina_array,
                                     unsigned int step) EINA_ARG_NONNULL(1);
/**
 * @brief Cleans an array.
 *
 * @details This function sets the count member of @a array to 0. However, it doesn't free
 *          any space. This is particularly useful if you need to empty the array and
 *          add lots of elements quickly. For performance reasons, there is no check on
 *          @a array. If it is @c NULL or invalid, the program may crash.
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array to clean
 *
 */
static inline void eina_array_clean(Eina_Array *array) EINA_ARG_NONNULL(1);

/**
 * @brief Flushes an array.
 *
 * @details This function sets the count and total members of @a array to 0,
 *          frees and sets its data member to @c NULL. For performance reasons,
 *          there is no check on @a array. If it is @c NULL or invalid, the
 *          program may crash.
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array to flush
 *
 */
EAPI void eina_array_flush(Eina_Array *array) EINA_ARG_NONNULL(1);

/**
 * @brief Rebuilds an array by specifying the data to keep.
 *
 * @details This function rebuilds @a array by specifying the elements to keep with the
 *          function @a keep. No empty/invalid fields are left in the array. @a gdata is
 *          an additional data to pass to @a keep. For performance reasons, there is no
 *          check on @a array. If it is @c NULL or invalid, the program may crash.
 *
 * @since_tizen 2.3
 *
 * @remarks If it isn't able to remove items due to an allocation failure, it
 *          returns @c EINA_FALSE and the error is set to #EINA_ERROR_OUT_OF_MEMORY.
 *
 * @param[in] array The array
 * @param[in] keep The functions that selects the data to keep
 * @param[in] gdata The data to pass to the function @a keep
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool eina_array_remove(Eina_Array * array,
                                 Eina_Bool (*keep)(void *data, void *gdata),
                                 void *gdata) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Append a data to an array.
 *
 * @details This function appends @p data to @p array. For performance
 *          reasons, there is no check of @p array. If it is @c NULL or
 *          invalid, the program may crash. If @p data is @c NULL, or if an
 *          allocation is necessary and fails, #EINA_FALSE is returned
 *          Otherwise, #EINA_TRUE is returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array.
 * @param[in] data The data to add.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 */
static inline Eina_Bool eina_array_push(Eina_Array *array,
                                        const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Remove the last data of an array.
 *
 * @details This function removes the last data of @p array, decreases the count
 *          of @p array and returns the data. For performance reasons, there
 *          is no check of @p array. If it is @c NULL or invalid, the program
 *          may crash. If the count member is less or equal than 0, @c NULL is
 *          returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array.
 * @return The retrieved data.
 */
static inline void     *eina_array_pop(Eina_Array *array) EINA_ARG_NONNULL(1);

/**
 * @brief Sets the data at the given position in an array.
 *
 * @details This function sets the data at the position @a idx in @a
 *          array to @a data, this effectively replaces the previously held data, you
 *          must therefore get a pointer to it first if you need to free it. For
 *          performance reasons, there is no check on @a array or @a idx. If it is
 *          @c NULL or invalid, the program may crash.
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array
 * @param[in] idx The position of the data to set
 */
static inline void     *eina_array_data_get(const Eina_Array *array,
                                            unsigned int      idx) EINA_ARG_NONNULL(1);
/**
 * @brief Sets the data at the given position in an array.
 *
 * @details This function sets the data at the position @a idx in @a
 *          array to @a data, this effectively replaces the previously held data, you
 *          must therefore get a pointer to it first if you need to free it. For
 *          performance reasons, there is no check on @a array or @a idx. If it is
 *          @c NULL or invalid, the program may crash.
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array
 * @param[in] idx The position of the data to set
 * @param[in] data The data to set
 *
 */
static inline void      eina_array_data_set(const Eina_Array *array,
                                            unsigned int      idx,
                                            const void       *data) EINA_ARG_NONNULL(1);

/**
 * @brief Return the number of elements in an array.
 *
 * @details This function returns the number of elements in @p array (array->count). For
 *          performance reasons, there is no check of @p array. If it is
 *          @c NULL or invalid, the program may crash.
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array.
 * @return The number of elements.
 *
 * @deprecated use eina_array_count()
 */
static inline unsigned int eina_array_count_get(const Eina_Array *array) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Return the number of elements in an array.
 *
 * @details This function returns the number of elements in @p array (array->count). 
 *          For performance reasons, there is no check of @p array. If it is
 *          @c NULL or invalid, the program may crash.
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array.
 * @return The number of elements.
 */
static inline unsigned int eina_array_count(const Eina_Array *array) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Returns a new iterator associated to an array.
 *
 * @details This function returns a newly allocated iterator associated to
 *          @a array. If @a array is @c NULL or the count member of @a array is
 *          less than or equal to 0, this function returns @c NULL. If the memory cannot
 *          be allocated, @c NULL is returned and #EINA_ERROR_OUT_OF_MEMORY is
 *          set. Otherwise, a valid iterator is returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array
 * @return A new iterator
 *
 */
EAPI Eina_Iterator        *eina_array_iterator_new(const Eina_Array *array) EINA_MALLOC EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Returns a new accessor associated to an array.
 *
 * @details This function returns a newly allocated accessor associated to
 *          @a array. If @a array is @c NULL or the count member of @a array is
 *          less than or equal to @c 0, this function returns @c NULL. If the memory cannot
 *          be allocated, @c NULL is returned and #EINA_ERROR_OUT_OF_MEMORY is
 *          set. Otherwise, a valid accessor is returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array
 * @return A new accessor
 *
 */
EAPI Eina_Accessor        *eina_array_accessor_new(const Eina_Array *array) EINA_MALLOC EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
/**
 * @brief Provides a safe way to iterate over an array.
 *
 * @details This function provides a safe way to iterate over an array. @a cb should
 *          return @c EINA_TRUE as long as you want the function to continue iterating,
 *          by returning @c EINA_FALSE it stops and returns @c EINA_FALSE as a result.
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array to iterate over
 * @param[in] cb The callback to call for each item
 * @param[in] fdata The user data to pass to the callback
 * @return @c EINA_TRUE if it successfully iterates all the items of the array, otherwise @c EINA_FALSE
 *
 */
static inline Eina_Bool    eina_array_foreach(Eina_Array  *array,
                                              Eina_Each_Cb cb,
                                              void        *fdata);
/**
 * @def EINA_ARRAY_ITER_NEXT
 * @brief Definition of the macro to iterate over an array easily.
 *
 * @details This macro allows the iteration over @a array in an easy way. It
 *          iterates from the first element to the last one. @a index is an
 *          integer that increases from 0 to the number of elements. @a item is
 *          the data of each element of @a array, so it is a pointer to a type
 *          chosen by the user. @a iterator is of type #Eina_Array_Iterator.
 *
 * @since_tizen 2.3
 *
 * @remarks This macro can be used for freeing the data of an array, like in
 *          the following example:
 *
 * @code
 * Eina_Array         *array;
 * char               *item;
 * Eina_Array_Iterator iterator;
 * unsigned int        i;
 *
 * // array is already filled,
 * // its elements are just duplicated strings,
 * // EINA_ARRAY_ITER_NEXT will be used to free those strings
 *
 * EINA_ARRAY_ITER_NEXT(array, i, item, iterator)
 *   free(item);
 * @endcode
 *
 * @param array The array to iterate over
 * @param index The integer number that is increased while iterating
 * @param item The data
 * @param iterator The iterator
 *
 */
#define EINA_ARRAY_ITER_NEXT(array, index, item, iterator)                  \
  for (index = 0, iterator = (array)->data;                                 \
       (index < eina_array_count(array)) && ((item = *((iterator)++)));     \
                                                  ++(index))

#include "eina_inline_array.x"

/**
 * @}
 */

#endif

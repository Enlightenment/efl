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
 * @page eina_array_01_example_page Basic array usage
 * @dontinclude eina_array_01.c
 *
 * For this example we add stdlib.h, stdio.h and string.h for some
 * convenience functions. The first thing to do to be able to use an
 * @ref Eina_Array is to include Eina.h:
 * @skip #include
 * @until Eina.h
 *
 * Here we have a callback that prints the element given to it:
 * @until }
 *
 * Now we create our entry point and declare some variables, nothing special:
 * @until unsigned
 *
 * Before we can start using any array function we need to initialize eina:
 * @until eina_init
 *
 * So now to actually create our array. The interesting thing here is the
 * argument given to the eina_array_new() function. This argument sets how fast
 * the array grows.
 * @until array_new
 *
 * If you know before hand how big the array will need to be you should set the
 * step to that. In our case we can set it to the number of strings we have and
 * since we didn't do that in the eina_array_new() we can do it now:
 * @until array_step_set
 *
 * Now let us populate our array with some strings:
 * @until push
 * @note Notice we use strdup, so we will have to free that memory later on.
 *
 * Now lets check the size of the array:
 * @until printf
 *
 * And now we call a function on every member of our array to print it:
 * @until foreach
 *
 * One of the strengths of @ref Eina_Array over @ref Eina_List is that it has
 * very fast random access to elements, so this is very efficient:
 * @until printf
 *
 * And now we free up the memory allocated with the strdup()s:
 * @until free
 *
 * And the array memory itself:
 * @until array_free
 *
 * And finally shutdown eina and exit:
 * @until }
 *
 * The full source code can be found in the examples folder
 * in the @ref eina_array_01_c "eina_array_01.c" file.
 */

/**
 * @page eina_array_01_c Basic array usage example
 *
 * @include eina_array_01.c
 * @example eina_array_01.c
 */

/**
 * @page eina_array_02_example_page Removing array elements
 * @dontinclude eina_array_02.c
 *
 * Just the usual includes:
 * @skip #include
 * @until Eina.h
 *
 * This is the callback we are going to use to decide which strings stay on the
 * array and which will be removed.  We use something simple, but this can be as
 * complex as you like:
 * @until }
 *
 * This is the same code we used before to populate the list with the slight
 * difference of not using strdup:
 * @until array_push
 *
 * So we have added all our elements to the array, but it turns out that is not
 * the elements we wanted, so let's empty the array and add the correct strings:
 * @until array_push
 *
 * It seems we made a little mistake in one of our strings so we need to replace
 * it, here is how:
 * @until data_set
 *
 * Now that there is a populated array we can remove elements from it easily:
 * @until array_remove
 *
 * And check that the elements were actually removed:
 * @until printf
 *
 * Since this time we didn't use strdup we don't need to free each string:
 * @until }
 *
 * The full source code can be found in the examples folder
 * in the @ref eina_array_02_c "eina_array_02.c" file.
 */

/**
 * @page eina_array_02_c Basic array usage example
 *
 * @include eina_array_02.c
 * @example eina_array_02.c
 */

/**
 * @addtogroup Eina_Array_Group Array
 *
 * @brief These functions provide array management.
 *
 * The Array data type in Eina is designed to have very fast access to
 * its data (compared to the Eina @ref Eina_List_Group). On the other
 * hand, data can be added or removed only at the end of the array. To
 * insert data at arbitrary positions, the Eina @ref Eina_List_Group is
 * the correct container to use.
 *
 * To use the array data type, eina_init() must be called before any
 * other array functions. When no more eina array functions are used,
 * eina_shutdown() must be called to free all the resources.
 *
 * An array must be created with eina_array_new(). It allocates all
 * the necessary data for an array. When not needed anymore, an array
 * is freed with eina_array_free(). This frees the memory used by the Eina_Array
 * itself, but does not free any memory used to store the data of each element.
 * To free that memory you must iterate over the array and free each data element
 * individually. A convenient way to do that is by using #EINA_ARRAY_ITER_NEXT.
 * An example of that pattern is given in the description of @ref EINA_ARRAY_ITER_NEXT.
 *
 * @warning Functions do not check if the used array is valid or not. It's up to
 * the user to be sure of that. It is designed like that for performance
 * reasons.
 *
 * The usual features of an array are classic ones: to append an
 * element, use eina_array_push() and to remove the last element, use
 * eina_array_pop(). To retrieve the element at a given position, use
 * eina_array_data_get(). The number of elements can be retrieved with
 * eina_array_count().
 *
 * An Eina_Array differs most notably from a conventional C array in that it can
 * grow and shrink dynamically as elements are added and removed.
 * Since allocating memory is expensive, when the array needs to grow it adds
 * enough memory to hold @p step additional elements, not just the element
 * currently being added. Similarly when elements are removed, it won't deallocate
 * until @p step elements are removed.
 *
 * The following image illustrates how an Eina_Array grows:
 *
 * @image html eina_array-growth.png
 * @image latex eina_array-growth.eps "" width=\textwidth
 *
 * Eina_Array only stores pointers but it can store data of any type in the form
 * of void pointers.
 *
 * See here some examples:
 * @li @ref eina_array_01_example_page
 * @li @ref eina_array_02_example_page
 */

/**
 * @addtogroup Eina_Data_Types_Group Data Types
 *
 * @{
 */

/**
 * @addtogroup Eina_Containers_Group Containers
 *
 * @{
 */

/**
 * @defgroup Eina_Array_Group Array
 *
 * @{
 */

/**
 * @typedef Eina_Array
 * Type for a generic one-dimensional linear data structure.
 */
typedef struct _Eina_Array Eina_Array;

/**
 * @typedef Eina_Array_Iterator
 * Type for an iterator on arrays, used with #EINA_ARRAY_ITER_NEXT.
 */
typedef void **Eina_Array_Iterator;

/**
 * @struct _Eina_Array
 * Type for an array of data.
 */
struct _Eina_Array
{
#define EINA_ARRAY_VERSION 1
   int          version; /**< Should match EINA_ARRAY_VERSION used when compiled your apps, provided for ABI compatibility */

   void       **data;  /**< Pointer to a C array of pointers to payloads */
   unsigned int total; /**< Number of allocated slots in @p data */
   unsigned int count; /**< Number of used slots in @p data that point to valid payloads */
   unsigned int step;  /**< Number of slots to grow or shrink @p data */
   EINA_MAGIC
};


/**
 * @brief Creates a new array.
 *
 * @param[in] step The count of pointers to add when increasing the array size.
 * @return @c NULL on failure, non @c NULL otherwise.
 *
 * This function creates a new array. When adding an element, the array
 * allocates @p step elements. When that buffer is full, adding
 * another element will increase the buffer by @p step elements again.
 *
 * This function return a valid array on success, or @c NULL if memory
 * allocation fails.
 */
EAPI Eina_Array *eina_array_new(unsigned int step) EINA_WARN_UNUSED_RESULT EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Frees an array.
 *
 * @param[in] array The array to free.
 *
 * This function finalizes @p array by flushing (see
 * eina_array_flush()), and then freeing the memory of the pointer. It
 * does not free the memory allocated for the elements of @p array. To
 * free them, walk the array with #EINA_ARRAY_ITER_NEXT.
 */
EAPI void eina_array_free(Eina_Array *array);

/**
 * @brief Sets the step of an array.
 *
 * @param[in,out] array The array.
 * @param[in] sizeof_eina_array Should be the value returned by sizeof(Eina_Array).
 * @param[in] step The count of pointers to add when increasing the array size.
 *
 * This function sets the step of @p array to @p step. For performance
 * reasons, there is no check of @p array. If it is @c NULL or
 * invalid, the program may crash.
 *
 * @warning This function can @b only be called on uninitialized arrays.
 */
EAPI void        eina_array_step_set(Eina_Array  *array,
                                     unsigned int sizeof_eina_array,
                                     unsigned int step) EINA_ARG_NONNULL(1);
/**
 * @brief Clears an array of its elements, without deallocating memory.
 *
 * @param[in,out] array The array to clean.
 *
 * This function sets the @p array's member count to 0 without freeing
 * memory.  This facilitates emptying an array and quickly refilling it
 * with new elements.  For performance reasons, there is no check of @p
 * array. If it is @c NULL or invalid, the program may crash.
 */
static inline void eina_array_clean(Eina_Array *array) EINA_ARG_NONNULL(1);

/**
 * @brief Clears an array's elements and deallocates the memory.
 *
 * @param[in,out] array The array to flush.
 *
 * This function sets the count and total members of @p array to 0, and
 * frees its data member and sets it to NULL. For performance reasons,
 * there is no check of @p array. If it is @c NULL or invalid, the
 * program may crash.
 */
EAPI void eina_array_flush(Eina_Array *array) EINA_ARG_NONNULL(1);

/**
 * @brief Rebuilds an array by specifying the data to keep.
 *
 * @param[in,out] array The array.
 * @param[in] keep The functions which selects the data to keep.
 * @param[in] gdata The data to pass to the function keep.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * This function rebuilds @p array by specifying the elements to keep with the
 * function @p keep. No empty/invalid fields are left in the array. @p gdata is
 * an additional data to pass to @p keep. For performance reasons, there is no
 * check of @p array. If it is @c NULL or invalid, the program may crash.
 *
 * If it wasn't able to remove items due to an allocation failure, it will
 * return #EINA_FALSE.
 */
EAPI Eina_Bool eina_array_remove(Eina_Array * array,
                                 Eina_Bool (*keep)(void *data, void *gdata),
                                 void *gdata) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Appends a data item to an array.
 *
 * @param[in,out] array The array.
 * @param[in] data The data to add.
 * @return #EINA_TRUE on success, #EINA_FALSE if allocation is necessary
 * and fails or if @p data is @c NULL.
 *
 * This function appends @p data to @p array. For performance
 * reasons, there is no check of @p array. If it is @c NULL or
 * invalid, the program may crash.
 */
static inline Eina_Bool eina_array_push(Eina_Array *array,
                                        const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Removes the last data item in an array.
 *
 * @param[in,out] array The array.
 * @return The retrieved data, or @c NULL if there are no remaining items.
 *
 * This function removes the last data item from @p array, decreases the
 * length of @p array and returns the data item. For performance reasons,
 * there is no check of @p array, so if it is @c NULL or invalid, the
 * program may crash.
 */
static inline void     *eina_array_pop(Eina_Array *array) EINA_ARG_NONNULL(1);

/**
 * @brief Returns the data at a given position in an array.
 *
 * @param[in] array The array.
 * @param[in] idx The position of the data to retrieve.
 * @return The retrieved data.
 *
 * This function returns the data at the position @p idx in @p
 * array. For performance reasons, there is no check of @p array or @p
 * idx. If @p array is @c NULL or invalid, or if @p idx is larger than
 * the array's size, the program may crash.
 */
static inline void     *eina_array_data_get(const Eina_Array *array,
                                            unsigned int      idx) EINA_ARG_NONNULL(1);
/**
 * @brief Sets the data at a given position in an array.
 *
 * @param[in] array The array.
 * @param[in] idx The position of the data to set.
 * @param[in] data The data to set.
 *
 * This function sets the data at the position @p idx in @p array to @p
 * data, this effectively replaces the previously held data, you must
 * therefore get a pointer to it first if you need to free it. For
 * performance reasons, there is no check of @p array or @p idx. If @p
 * array is @c NULL or invalid, or if @p idx is larger than the array's
 * size, the program may crash.
*/
static inline void      eina_array_data_set(const Eina_Array *array,
                                            unsigned int      idx,
                                            const void       *data) EINA_ARG_NONNULL(1);

/**
 * @deprecated use eina_array_count()
 * @brief Returns the number of elements in an array.
 *
 * @param[in] array The array.
 * @return The number of elements.
 *
 * This function returns the number of elements in @p array (array->count). For
 * performance reasons, there is no check of @p array, so if it is
 * @c NULL or invalid, the program may crash.
 *
 */
static inline unsigned int eina_array_count_get(const Eina_Array *array) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Returns the number of elements in an array.
 *
 * @param[in] array The array.
 * @return The number of elements.
 *
 * This function returns the number of elements in @p array (array->count). For
 * performance reasons, there is no check of @p array, so if it is
 * @c NULL or invalid, the program may crash.
 */
static inline unsigned int eina_array_count(const Eina_Array *array) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Gets a new iterator associated with an array.
 *
 * @param[in] array The array.
 * @return A new iterator, or @c NULL if @p array is @c NULL or has no
 * items, or if memory could not be allocated.
 *
 * This function allocates a new iterator associated with @p array.
 * Use EINA_ARRAY_ITER_NEXT() to iterate through the array's data items
 * in order of entry.
 *
 * @see Eina_Iterator_Group
 */
EAPI Eina_Iterator        *eina_array_iterator_new(const Eina_Array *array) EINA_MALLOC EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Gets a new accessor associated with an array.
 *
 * @param[in] array The array.
 * @return A new accessor, or @c NULL if @p array is @c NULL or has no
 * items, or if memory could not be allocated.
 *
 * This function returns a newly allocated accessor associated with
 * @p array.  Accessors differ from iterators in that they permit
 * random access.
 *
 * @see Eina_Accessor_Group
 */
EAPI Eina_Accessor        *eina_array_accessor_new(const Eina_Array *array) EINA_MALLOC EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Iterates over an array using a callback function.
 *
 * @param[in] array The array to iterate over.
 * @param[in] cb The callback to invoke for each item.
 * @param[in] fdata The user data to pass to the callback.
 * @return #EINA_TRUE if it successfully iterated all items of the array.
 *
 * This function iterates over an array in order, calling @p cb for each
 * item.  @p cb should return #EINA_TRUE if the loop should continue, or
 * #EINA_FALSE to exit the loop, in which case eina_array_foreach() will
 * return #EINA_FALSE.
 */
static inline Eina_Bool    eina_array_foreach(Eina_Array  *array,
                                              Eina_Each_Cb cb,
                                              void        *fdata);
/**
 * @def EINA_ARRAY_ITER_NEXT
 * @brief Iterates through an array's elements.
 *
 * @param[in] array The array to iterate over.
 * @param[out] index The integer number that is increased while iterating.
 * @param[out] item The data
 * @param[in,out] iterator The #Eina_Array_Iterator.
 *
 * This macro iterates over @p array in order, increasing @p index from
 * the first to last element and setting @p item to each element's data
 * item in turn.
 *
 * This macro can be used for freeing the data of an array, such as
 * the following example:
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
 */
#define EINA_ARRAY_ITER_NEXT(array, index, item, iterator)                  \
  for (index = 0, iterator = (array)->data;                                 \
       (index < eina_array_count(array)) && ((item = *((iterator)++)));     \
                                                  ++(index))

#include "eina_inline_array.x"

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#endif

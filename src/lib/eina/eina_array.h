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
 * Now we create our entry point and declare some variables, nothing especial:
 * @until unsigned
 *
 * Before we can start using any array function we need to initialize eina:
 * @until eina_init
 *
 * So now to actually creating our array. The only interesting thing here is the
 * argument given to the eina_array_new() function, this argument sets how fast
 * the array grows.
 * @until array_new
 *
 * If you know before hand how big the array will need to be you should set the
 * step to that. In our case we can set it to the number of string we have and
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
 * The full source code can be found on the examples folder
 * on the @ref eina_array_01_c "eina_array_01.c" file.
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
 * This the callback we are going to use to decide which strings stay on the
 * array and which will be removed, we use something simple, but this can be as
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
 * The full source code can be found on the examples folder
 * on the @ref eina_array_02_c "eina_array_02.c" file.
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
 * its data (compared to the Eina @ref Eina_List_Group). On the other hand,
 * data can be added or removed only at the end of the array. To insert
 * data at any place, the Eina @ref Eina_List_Group is the correct container
 * to use.
 *
 * To use the array data type, eina_init() must be called before any
 * other array functions. When no more eina array functions are used,
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
 * Eina_Array is different from a conventional C array in a number of ways, most
 * importantly they grow and shrink dynamically, this means that if you add an
 * element to a full array it grows and that when you remove an element from an
 * array it @b may shrink.
 *
 * When the array needs to grow it allocates memory not just for the element
 * currently being added since that would mean allocating memory(which is
 * computationally expensive) often, instead it grows to be able to hold @p step
 * more elements. Similarly if you remove elements in such a way that that the
 * array is left holding its capacity - @p step elements it will shrink.
 *
 * The following image illustrates how an Eina_Array grows:
 *
 * @image html eina_array-growth.png
 * @image latex eina_array-growth.eps width=\textwidth
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
 * Type for a generic vector.
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

   void       **data; /**< Pointer to a vector of pointer to payload */
   unsigned int total; /**< Total number of slots in the vector */
   unsigned int count; /**< Number of active slots in the vector */
   unsigned int step; /**< How much must we grow the vector when it is full */
   EINA_MAGIC
};


/**
 * @brief Create a new array.
 *
 * @param step The count of pointers to add when increasing the array size.
 * @return @c NULL on failure, non @c NULL otherwise.
 *
 * This function creates a new array. When adding an element, the array
 * allocates @p step elements. When that buffer is full, then adding
 * another element will increase the buffer by @p step elements again.
 *
 * This function return a valid array on success, or @c NULL if memory
 * allocation fails.
 */
EAPI Eina_Array *eina_array_new(unsigned int step) EINA_WARN_UNUSED_RESULT EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Free an array.
 *
 * @param array The array to free.
 *
 * This function frees @p array. It calls first eina_array_flush() then
 * free the memory of the pointer. It does not free the memory
 * allocated for the elements of @p array. To free them,
 * use #EINA_ARRAY_ITER_NEXT. For performance reasons, there is no check
 * of @p array.
 */
EAPI void        eina_array_free(Eina_Array *array) EINA_ARG_NONNULL(1);

/**
 * @brief Set the step of an array.
 *
 * @param array The array.
 * @param sizeof_eina_array Should be the value returned by sizeof(Eina_Array).
 * @param step The count of pointers to add when increasing the array size.
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
 * @brief Clean an array.
 *
 * @param array The array to clean.
 *
 * This function sets the count member of @p array to 0, however it doesn't free
 * any space. This is particularly useful if you need to empty the array and
 * add lots of elements quickly. For performance reasons, there is no check of
 * @p array. If it is @c NULL or invalid, the program may crash.
 */
static inline void eina_array_clean(Eina_Array *array) EINA_ARG_NONNULL(1);

/**
 * @brief Flush an array.
 *
 * @param array The array to flush.
 *
 * This function sets the count and total members of @p array to 0,
 * frees and set to NULL its data member. For performance reasons,
 * there is no check of @p array. If it is @c NULL or invalid, the
 * program may crash.
 */
EAPI void eina_array_flush(Eina_Array *array) EINA_ARG_NONNULL(1);

/**
 * @brief Rebuild an array by specifying the data to keep.
 *
 * @param array The array.
 * @param keep The functions which selects the data to keep.
 * @param gdata The data to pass to the function keep.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * This function rebuilds @p array be specifying the elements to keep with the
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
static inline Eina_Bool eina_array_push(Eina_Array *array,
                                        const void *data) EINA_ARG_NONNULL(1, 2);
static inline void     *eina_array_pop(Eina_Array *array) EINA_ARG_NONNULL(1);
static inline void     *eina_array_data_get(const Eina_Array *array,
                                            unsigned int      idx) EINA_ARG_NONNULL(1);
/**
 * @brief Set the data at a given position in an array.
 *
 * @param array The array.
 * @param idx The position of the data to set.
 * @param data The data to set.
 *
 * This function sets the data at the position @p idx in @p
 * array to @p data, this effectively replaces the previously held data, you
 * must therefore get a pointer to it first if you need to free it. For
 * performance reasons, there is no check of @p array or @p idx. If it is @c
 * NULL or invalid, the program may crash.
*/
static inline void      eina_array_data_set(const Eina_Array *array,
                                            unsigned int      idx,
                                            const void       *data) EINA_ARG_NONNULL(1);
static inline unsigned int eina_array_count_get(const Eina_Array *array) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
static inline unsigned int eina_array_count(const Eina_Array *array) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Returned a new iterator associated to an array.
 *
 * @param array The array.
 * @return A new iterator.
 *
 * This function returns a newly allocated iterator associated to
 * @p array. If @p array is @c NULL or the count member of @p array is
 * less or equal than 0, this function returns @c NULL. If the memory can
 * not be allocated, NULL is returned. Otherwise, a valid iterator is returned.
 */
EAPI Eina_Iterator        *eina_array_iterator_new(const Eina_Array *array) EINA_MALLOC EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Returned a new accessor associated to an array.
 *
 * @param array The array.
 * @return A new accessor.
 *
 * This function returns a newly allocated accessor associated to
 * @p array. If @p array is @c NULL or the count member of @p array is
 * less or equal than 0, this function returns @c NULL. If the memory can
 * not be allocated, @c NULL is returned Otherwise, a valid accessor is
 * returned.
 */
EAPI Eina_Accessor        *eina_array_accessor_new(const Eina_Array *array) EINA_MALLOC EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
/**
 * @brief Provide a safe way to iterate over an array
 *
 * @param array The array to iterate over.
 * @param cb The callback to call for each item.
 * @param fdata The user data to pass to the callback.
 * @return #EINA_TRUE if it successfully iterate all items of the array.
 *
 * This function provide a safe way to iterate over an array. @p cb should
 * return #EINA_TRUE as long as you want the function to continue iterating,
 * by returning #EINA_FALSE it will stop and return #EINA_FALSE as a result.
 */
static inline Eina_Bool    eina_array_foreach(Eina_Array  *array,
                                              Eina_Each_Cb cb,
                                              void        *fdata);
/**
 * @def EINA_ARRAY_ITER_NEXT
 * @brief Macro to iterate over an array easily.
 *
 * @param array The array to iterate over.
 * @param index The integer number that is increased while iterating.
 * @param item The data
 * @param iterator The iterator
 *
 * This macro allows the iteration over @p array in an easy way. It
 * iterates from the first element to the last one. @p index is an
 * integer that increases from 0 to the number of elements. @p item is
 * the data of each element of @p array, so it is a pointer to a type
 * chosen by the user. @p iterator is of type #Eina_Array_Iterator.
 *
 * This macro can be used for freeing the data of an array, like in
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

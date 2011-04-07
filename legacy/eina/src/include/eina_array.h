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
 * @page tutorial_array_page Array Tutorial
 *
 * The Array data type is allow the storage of data like a C array.
 * It is designed such that the access to its element is very fast.
 * But the addition or removal can be done only at the end of the
 * array. To add or remove an element at any location, the Eina
 * @ref Eina_List_Group is the correct container is the correct one.
 *
 * @section tutorial_error_basic_usage Basic Usage
 *
 * An array must created with eina_array_new(). That function
 * takes an integer as parameter, which is the count of pointers to
 * add when increasing the array size. Once the array is not used
 * anymore, it must be destroyed with eina_array_free().
 *
 * To append data at the end of the array, the function
 * eina_array_push() must be used. To remove the data at the end of
 * the array, eina_array_pop() must be used. Once the array is filled,
 * one can check its elements by iterating over it. A while loop and
 * eina_array_data_get() can be used, or else one can use the
 * predefined macro EINA_ARRAY_ITER_NEXT(). To free all the elements,
 * a while loop can be used with eina_array_count_get(). Here is an
 * example of use:
 *
 * @code
 * #include <stdlib.h>
 * #include <stdio.h>
 * #include <string.h>
 *
 * #include <eina_array.h>
 *
 * int main(void)
 * {
 *     const char *strings[] = {
 *         "first string",
 *         "second string",
 *         "third string",
 *         "fourth string"
 *     };
 *     Eina_Array         *array;
 *     char               *item;
 *     Eina_Array_Iterator iterator;
 *     unsigned int        i;
 *
 *     if (!eina_init())
 *     {
 *         printf ("Error during the initialization of eina\n");
 *         return EXIT_FAILURE;
 *     }
 *
 *     array = eina_array_new(16);
 *     if (!array)
 *         goto shutdown;
 *
 *     for (i = 0; i < 4; i++)
 *     {
 *         eina_array_push(array, strdup(strings[i]));
 *     }
 *
 *     printf("array count: %d\n", eina_array_count_get(array));
 *     EINA_ARRAY_ITER_NEXT(array, i, item, iterator)
 *     {
 *         printf("item #%d: %s\n", i, item);
 *     }
 *
 *     while (eina_array_count_get(array))
 *     {
 *         void *data;
 *
 *         data = eina_array_pop(array);
 *         free(data);
 *     }
 *
 *     eina_array_free(array);
 *     eina_shutdown();
 *
 *     return EXIT_SUCCESS;
 *
 *   shutdown:
 *     eina_shutdown();
 *
 *     return EXIT_FAILURE;
 * }
 * @endcode
 *
 * To be continued
 */

/**
 * @addtogroup Eina_Array_Group Array
 *
 * @brief These functions provide array management.
 *
 * The Array data type in Eina is designed to have a very fast access to
 * its data (compared to the Eina @ref Eina_List_Group). On the other hand,
 * data can be added or removed only at the end of the array. To insert
 * data at any place, the Eina @ref Eina_List_Group is the correct container
 * to use.
 *
 * To use the array data type, eina_init() must be called before any
 * other array functions. When eina is no more array function is used,
 * eina_shutdown() must be called to free all the resources.
 *
 * An array must be created with eina_array_new(). It allocated all
 * the necessary data for an array. When not needed anymore, an array
 * is freed with eina_array_free(). This function does not free any
 * allocated memory used to store the data of each element. For that,
 * just iterate over the array to free them. A convenient way to do
 * that is by using #EINA_ARRAY_ITER_NEXT. An example of code is given
 * in the description of this macro.
 *
 * @warning All the other functions do not check if the used array is
 * valid or not. It's up to the user to be sure of that. It is
 * designed like that for performance reasons.
 *
 * The usual features of an array are classic ones: to append an
 * element, use eina_array_push() and to remove the last element, use
 * eina_array_pop(). To retrieve the element at a given positin, use
 * eina_array_data_get(). The number of elements can be retrieved with
 * eina_array_count_get().
 *
 * For more information, you can look at the @ref tutorial_array_page.
 *
 * @{
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
 * another element will increase the buffer of @p step elements again.
 *
 * This function return a valid array on success, or @c NULL if memory
 * allocation fails. In that case, the error is set to
 * #EINA_ERROR_OUT_OF_MEMORY.
 */
EAPI Eina_Array *eina_array_new(unsigned int step) EINA_WARN_UNUSED_RESULT EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Free an array.
 *
 * @param array The array to free.
 *
 * This function frees @p array. It calls first eina_array_flush() then
 * free the memory of the pointer. It does not free the memory
 * allocated for the elements of @p array. To free them, use
 * #EINA_ARRAY_ITER_NEXT. For performance reasons, there is no check
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
 * invalid, the program may crash. This function should be called when
 * the array is not initialized.
 */
EAPI void        eina_array_step_set(Eina_Array  *array,
                                     unsigned int sizeof_eina_array,
                                     unsigned int step) EINA_ARG_NONNULL(1);
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
 * @return #EINA_TRUE on success, #EINA_FALSE oterwise.
 *
 * This function rebuilds @p array be specifying the elements to keep
 * with the function @p keep. @p gdata is an additional data to pass
 * to @p keep. For performance reasons, there is no check of @p
 * array. If it is @c NULL or invalid, the program may crash.
 *
 * This function always return a valid array. If it wasn't able to
 * remove items due to an allocation failure, it will return #EINA_FALSE
 * and the error is set to #EINA_ERROR_OUT_OF_MEMORY.
 */
EAPI Eina_Bool eina_array_remove(Eina_Array * array,
                                 Eina_Bool (*keep)(void *data, void *gdata),
                                 void *gdata) EINA_ARG_NONNULL(1, 2);
static inline Eina_Bool eina_array_push(Eina_Array *array,
                                        const void *data) EINA_ARG_NONNULL(1, 2);
static inline void     *eina_array_pop(Eina_Array *array) EINA_ARG_NONNULL(1);
static inline void     *eina_array_data_get(const Eina_Array *array,
                                            unsigned int      idx) EINA_ARG_NONNULL(1);
static inline void      eina_array_data_set(const Eina_Array *array,
                                            unsigned int      idx,
                                            const void       *data) EINA_ARG_NONNULL(1);
static inline unsigned int eina_array_count_get(const Eina_Array *array) EINA_ARG_NONNULL(1);

/**
 * @brief Returned a new iterator associated to an array.
 *
 * @param array The array.
 * @return A new iterator.
 *
 * This function returns a newly allocated iterator associated to
 * @p array. If @p array is @c NULL or the count member of @p array is
 * less or equal than 0, this function returns NULL. If the memory can
 * not be allocated, NULL is returned and #EINA_ERROR_OUT_OF_MEMORY is
 * set. Otherwise, a valid iterator is returned.
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
 * less or equal than 0, this function returns NULL. If the memory can
 * not be allocated, NULL is returned and #EINA_ERROR_OUT_OF_MEMORY is
 * set. Otherwise, a valid accessor is returned.
 */
EAPI Eina_Accessor        *eina_array_accessor_new(const Eina_Array *array) EINA_MALLOC EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
static inline Eina_Bool    eina_array_foreach(Eina_Array  *array,
                                              Eina_Each_Cb cb,
                                              void        *data);
/**
 * @def EINA_ARRAY_ITER_NEXT
 * @brief Macro to iterate over an array easily.
 *
 * @param array The array to iterate over.
 * @param index The integer number that is increased while itareting.
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
       (index < eina_array_count_get(array)) && ((item = *((iterator)++))); \
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


/**
 * @}
 */

#endif

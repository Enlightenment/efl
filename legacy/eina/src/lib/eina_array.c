/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
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


/**
 * @page tutorial_array_page Array Tutorial
 *
 * to be written...
 *
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "eina_error.h"
#include "eina_array.h"
#include "eina_inline_array.x"
#include "eina_private.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

#define EINA_MAGIC_CHECK_ARRAY(d)				\
   do {								\
     if (!EINA_MAGIC_CHECK(d, EINA_MAGIC_ARRAY))		\
       EINA_MAGIC_FAIL(d, EINA_MAGIC_ARRAY);			\
   } while (0);

#define EINA_MAGIC_CHECK_ARRAY_ITERATOR(d)			\
   do {								\
     if (!EINA_MAGIC_CHECK(d, EINA_MAGIC_ARRAY_ITERATOR))	\
       EINA_MAGIC_FAIL(d, EINA_MAGIC_ARRAY_ITERATOR);		\
   } while (0);

#define EINA_MAGIC_CHECK_ARRAY_ACCESSOR(d)			\
   do {								\
     if (!EINA_MAGIC_CHECK(d, EINA_MAGIC_ARRAY_ACCESSOR))	\
       EINA_MAGIC_FAIL(d, EINA_MAGIC_ACCESSOR);			\
   } while (0);


typedef struct _Eina_Iterator_Array Eina_Iterator_Array;
struct _Eina_Iterator_Array
{
   Eina_Iterator iterator;

   EINA_MAGIC;

   const Eina_Array *array;
   unsigned int index;
};

typedef struct _Eina_Accessor_Array Eina_Accessor_Array;
struct _Eina_Accessor_Array
{
   Eina_Accessor accessor;

   EINA_MAGIC;

   const Eina_Array *array;
};

static int _eina_array_init_count = 0;

static Eina_Bool
eina_array_iterator_next(Eina_Iterator_Array *it, void **data)
{
   EINA_MAGIC_CHECK_ARRAY_ITERATOR(it);

   if (!(it->index < eina_array_count_get(it->array)))
     return EINA_FALSE;
   if (data)
     *data = eina_array_data_get(it->array, it->index);
   it->index++;
   return EINA_TRUE;
}

static Eina_Array *
eina_array_iterator_get_container(Eina_Iterator_Array *it)
{
   EINA_MAGIC_CHECK_ARRAY_ITERATOR(it);
   return (Eina_Array *) it->array;
}

static void
eina_array_iterator_free(Eina_Iterator_Array *it)
{
   EINA_MAGIC_CHECK_ARRAY_ITERATOR(it);
   MAGIC_FREE(it);
}

static Eina_Bool
eina_array_accessor_get_at(Eina_Accessor_Array *it, unsigned int index, void **data)
{
   EINA_MAGIC_CHECK_ARRAY_ACCESSOR(it);

   if (!(index < eina_array_count_get(it->array)))
     return EINA_FALSE;
   if (data)
     *data = eina_array_data_get(it->array, index);
   return EINA_TRUE;
}

static Eina_Array *
eina_array_accessor_get_container(Eina_Accessor_Array *it)
{
   EINA_MAGIC_CHECK_ARRAY_ACCESSOR(it);
   return (Eina_Array *) it->array;
}

static void
eina_array_accessor_free(Eina_Accessor_Array *it)
{
   EINA_MAGIC_CHECK_ARRAY_ACCESSOR(it);
   MAGIC_FREE(it);
}

EAPI Eina_Bool
eina_array_grow(Eina_Array *array)
{
   void **tmp;
   unsigned int total;

   EINA_MAGIC_CHECK_ARRAY(array);

   total = array->total + array->step;
   eina_error_set(0);
   tmp = realloc(array->data, sizeof (void*) * total);
   if (UNLIKELY(!tmp)) {
      eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
      return 0;
   }

   array->total = total;
   array->data = tmp;

   return 1;
}

/**
 * @endcond
 */


/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

/**
 * @addtogroup Eina_Array_Group Array
 *
 * @brief These functions provide array management.
 *
 * To use the array data type, eina_array_init() must be called before
 * any other array functions. When no more array function is used,
 * eina_array_shutdown() must be called to free all the resources.
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
 * @brief Initialize the array module.
 *
 * @return 1 or greater on success, 0 on error.
 *
 * This function just sets up the error module or Eina. It is also
 * called by eina_init(). It returns 0 on failure, otherwise it
 * returns the number of times eina_error_init() has already been
 * called.
 */
EAPI int
eina_array_init(void)
{
  if (!_eina_array_init_count)
    {
      eina_error_init();
      eina_magic_string_init();

      eina_magic_string_set(EINA_MAGIC_ITERATOR,
			    "Eina Iterator");
      eina_magic_string_set(EINA_MAGIC_ACCESSOR,
			    "Eina Accessor");
      eina_magic_string_set(EINA_MAGIC_ARRAY,
			    "Eina Array");
      eina_magic_string_set(EINA_MAGIC_ARRAY_ITERATOR,
			    "Eina Array Iterator");
      eina_magic_string_set(EINA_MAGIC_ARRAY_ACCESSOR,
			    "Eina Array Accessor");
    }

  return ++_eina_array_init_count;
}

/**
 * @brief Shut down the array module.
 *
 * @return 0 when the error module is completely shut down, 1 or
 * greater otherwise.
 *
 * This function just shut down the error module set up by
 * eina_array_init(). It is also called by eina_shutdown(). It returns
 * 0 when it is called the same number of times than
 * eina_error_init().
 */
EAPI int
eina_array_shutdown(void)
{
   --_eina_array_init_count;
   if (!_eina_array_init_count)
     {
       eina_magic_string_shutdown();
       eina_error_shutdown();
     }

   return _eina_array_init_count;
}

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
EAPI Eina_Array *
eina_array_new(unsigned int step)
{
   Eina_Array *array;

   eina_error_set(0);
   array = malloc(sizeof (Eina_Array));
   if (!array) {
      eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
      return NULL;
   }

   EINA_MAGIC_SET(array, EINA_MAGIC_ARRAY);

   array->data = NULL;
   array->total = 0;
   array->count = 0;
   array->step = step;

   return array;
}

/**
 * @brief Free an array.
 *
 * @param array The array to free.
 *
 * This function frees @p array. It calls first eina_array_flush() then
 * free the memory of the pointeur. It does not free the memory
 * allocated for the elements of @p array. To free them, use
 * #EINA_ARRAY_ITER_NEXT. For performance reasons, there is no check
 * of @p array.
 */
EAPI void
eina_array_free(Eina_Array *array)
{
   eina_array_flush(array);

   EINA_MAGIC_CHECK_ARRAY(array);
   MAGIC_FREE(array);
}

/**
 * @brief Set the step of an array.
 *
 * @param array The array.
 * @param step The count of pointers to add when increasing the array size.
 *
 * This function sets the step of @p array to @p step. For performance
 * reasons, there is no check of @p array. If it is @c NULL or
 * invalid, the program may crash.
 */
EAPI void
eina_array_step_set(Eina_Array *array, unsigned int step)
{
  array->data = NULL;
  array->total = 0;
  array->count = 0;
  array->step = step;
  EINA_MAGIC_SET(array, EINA_MAGIC_ARRAY);
}

/**
 * @brief Clean an array.
 *
 * @param array The array to clean.
 *
 * This function sets the count member of @p array to 0. For
 * performance reasons, there is no check of @p array. If it is
 * @c NULL or invalid, the program may crash.
 */
EAPI void
eina_array_clean(Eina_Array *array)
{
   EINA_MAGIC_CHECK_ARRAY(array);
   array->count = 0;
}

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
EAPI void
eina_array_flush(Eina_Array *array)
{
   EINA_MAGIC_CHECK_ARRAY(array);
   array->count = 0;
   array->total = 0;

   if (array->data) free(array->data);
   array->data = NULL;
}

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
EAPI Eina_Bool
eina_array_remove(Eina_Array *array, Eina_Bool (*keep)(void *data, void *gdata), void *gdata)
{
   void **tmp;
   void *data;
   unsigned int total = 0;
   unsigned int limit;
   unsigned int i;

   EINA_MAGIC_CHECK_ARRAY(array);
   if (array->total == 0) return EINA_TRUE;

   for (i = 0; i < array->count; ++i)
     {
	data = eina_array_data_get(array, i);

	if (keep(data, gdata) == EINA_FALSE) break;
     }
   limit = i;
   if (i < array->count) ++i;
   for (; i < array->count; ++i)
     {
	data = eina_array_data_get(array, i);

	if (keep(data, gdata) == EINA_TRUE) break;
     }
   /* Special case all objects that need to stay are at the beginning of the array. */
   if (i == array->count)
     {
	array->count = limit;
	if (array->count == 0)
	  {
	     free(array->data);
	     array->total = 0;
	     array->data = NULL;
	  }

	return EINA_TRUE;
     }

   eina_error_set(0);
   tmp = malloc(sizeof (void*) * array->total);
   if (!tmp) {
      eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
      return EINA_FALSE;
   }

   memcpy(tmp, array->data, limit * sizeof(void*));
   total = limit;

   if (i < array->count)
     {
	tmp[total] = data;
	total++;
	++i;
     }

   for (; i < array->count; ++i)
     {
	data = eina_array_data_get(array, i);

	if (keep(data, gdata))
	  {
	     tmp[total] = data;
	     total++;
	  }
     }

   free(array->data);

   /* If we do not keep any object in the array, we should have exited
      earlier in test (i == array->count). */
   assert(total != 0);

   array->data = tmp;
   array->count = total;

   return EINA_TRUE;
}

/**
 * @brief Returned a new iterator asociated to an array.
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
EAPI Eina_Iterator *
eina_array_iterator_new(const Eina_Array *array)
{
   Eina_Iterator_Array *it;

   if (!array) return NULL;
   if (eina_array_count_get(array) <= 0) return NULL;

   eina_error_set(0);
   it = calloc(1, sizeof (Eina_Iterator_Array));
   if (!it) {
      eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
      return NULL;
   }

   EINA_MAGIC_SET(it, EINA_MAGIC_ARRAY_ITERATOR);
   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->array = array;

   it->iterator.next = FUNC_ITERATOR_NEXT(eina_array_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(eina_array_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(eina_array_iterator_free);

   return &it->iterator;
}

/**
 * @brief Returned a new accessor asociated to an array.
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
EAPI Eina_Accessor *
eina_array_accessor_new(const Eina_Array *array)
{
   Eina_Accessor_Array *it;

   if (!array) return NULL;

   eina_error_set(0);
   it = calloc(1, sizeof (Eina_Accessor_Array));
   if (!it) {
      eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
      return NULL;
   }

   EINA_MAGIC_SET(it, EINA_MAGIC_ARRAY_ACCESSOR);
   EINA_MAGIC_SET(&it->accessor, EINA_MAGIC_ACCESSOR);

   it->array = array;

   it->accessor.get_at = FUNC_ACCESSOR_GET_AT(eina_array_accessor_get_at);
   it->accessor.get_container = FUNC_ACCESSOR_GET_CONTAINER(eina_array_accessor_get_container);
   it->accessor.free = FUNC_ACCESSOR_FREE(eina_array_accessor_free);

   return &it->accessor;
}

/**
 * @}
 */


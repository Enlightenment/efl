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


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "eina_config.h"
#include "eina_private.h"
#include "eina_log.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_array.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

static const char EINA_MAGIC_ARRAY_STR[] = "Eina Array";
static const char EINA_MAGIC_ARRAY_ITERATOR_STR[] = "Eina Array Iterator";
static const char EINA_MAGIC_ARRAY_ACCESSOR_STR[] = "Eina Array Accessor";

#define EINA_MAGIC_CHECK_ARRAY(d)                       \
   do {                                                  \
        if (!EINA_MAGIC_CHECK(d, EINA_MAGIC_ARRAY)) {        \
             EINA_MAGIC_FAIL(d, EINA_MAGIC_ARRAY); }            \
     } while (0)

#define EINA_MAGIC_CHECK_ARRAY_ITERATOR(d, ...)                 \
   do {                                                          \
        if (!EINA_MAGIC_CHECK(d, EINA_MAGIC_ARRAY_ITERATOR))       \
          {                                                        \
             EINA_MAGIC_FAIL(d, EINA_MAGIC_ARRAY_ITERATOR);        \
             return __VA_ARGS__;                                   \
          }                                                        \
     } while (0)

#define EINA_MAGIC_CHECK_ARRAY_ACCESSOR(d, ...)                 \
   do {                                                          \
        if (!EINA_MAGIC_CHECK(d, EINA_MAGIC_ARRAY_ACCESSOR))       \
          {                                                        \
             EINA_MAGIC_FAIL(d, EINA_MAGIC_ACCESSOR);              \
             return __VA_ARGS__;                                   \
          }                                                        \
     } while (0)


typedef struct _Eina_Iterator_Array Eina_Iterator_Array;
struct _Eina_Iterator_Array
{
   Eina_Iterator iterator;

   const Eina_Array *array;
   unsigned int index;

   EINA_MAGIC
};

typedef struct _Eina_Accessor_Array Eina_Accessor_Array;
struct _Eina_Accessor_Array
{
   Eina_Accessor accessor;
   const Eina_Array *array;
   EINA_MAGIC
};

static int _eina_array_log_dom = -1;

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_array_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eina_array_log_dom, __VA_ARGS__)

static void        eina_array_iterator_free(Eina_Iterator_Array *it) EINA_ARG_NONNULL(1);
static Eina_Array *eina_array_iterator_get_container(Eina_Iterator_Array *it) EINA_ARG_NONNULL(1);
static Eina_Bool   eina_array_iterator_next(Eina_Iterator_Array *it,
                                            void **data) EINA_ARG_NONNULL(1);

static Eina_Bool   eina_array_accessor_get_at(Eina_Accessor_Array *it,
                                              unsigned int idx,
                                              void **data) EINA_ARG_NONNULL(1);
static Eina_Array *eina_array_accessor_get_container(Eina_Accessor_Array *it) EINA_ARG_NONNULL(1);
static void        eina_array_accessor_free(Eina_Accessor_Array *it) EINA_ARG_NONNULL(1);

static Eina_Bool
eina_array_iterator_next(Eina_Iterator_Array *it, void **data)
{
   EINA_MAGIC_CHECK_ARRAY_ITERATOR(it, EINA_FALSE);

   if (!(it->index < eina_array_count(it->array)))
      return EINA_FALSE;

   if (data)
      *data = eina_array_data_get(it->array, it->index);

   it->index++;
   return EINA_TRUE;
}

static Eina_Array *
eina_array_iterator_get_container(Eina_Iterator_Array *it)
{
   EINA_MAGIC_CHECK_ARRAY_ITERATOR(it, NULL);
   return (Eina_Array *)it->array;
}

static void
eina_array_iterator_free(Eina_Iterator_Array *it)
{
   EINA_MAGIC_CHECK_ARRAY_ITERATOR(it);
   MAGIC_FREE(it);
}

static Eina_Bool
eina_array_accessor_get_at(Eina_Accessor_Array *it,
                           unsigned int idx,
                           void **data)
{
   EINA_MAGIC_CHECK_ARRAY_ACCESSOR(it, EINA_FALSE);

   if (!(idx < eina_array_count(it->array)))
      return EINA_FALSE;

   if (data)
      *data = eina_array_data_get(it->array, idx);

   return EINA_TRUE;
}

static Eina_Array *
eina_array_accessor_get_container(Eina_Accessor_Array *it)
{
   EINA_MAGIC_CHECK_ARRAY_ACCESSOR(it, NULL);
   return (Eina_Array *)it->array;
}

static void
eina_array_accessor_free(Eina_Accessor_Array *it)
{
   EINA_MAGIC_CHECK_ARRAY_ACCESSOR(it);
   MAGIC_FREE(it);
}

static EAPI Eina_Accessor *
eina_array_accessor_clone(const Eina_Array *array)
{
   Eina_Accessor_Array *ac;

   EINA_SAFETY_ON_NULL_RETURN_VAL(array, NULL);
   EINA_MAGIC_CHECK_ARRAY(array);

   ac = calloc(1, sizeof (Eina_Accessor_Array));
   if (!ac) return NULL;

   memcpy(ac, array, sizeof(Eina_Accessor_Array));

   return &ac->accessor;
}

/* used from eina_inline_array.x, thus a needed symbol */
EAPI Eina_Bool
eina_array_grow(Eina_Array *array)
{
   void **tmp;
   unsigned int total;

   EINA_SAFETY_ON_NULL_RETURN_VAL(array, EINA_FALSE);

   EINA_MAGIC_CHECK_ARRAY(array);

   total = array->total + array->step;
   tmp = realloc(array->data, sizeof (void *) * total);
   if (EINA_UNLIKELY(!tmp)) return 0;

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

/**
 * @internal
 * @brief Initialize the array module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function sets up the error and magic modules or Eina. It is
 * called by eina_init().
 *
 * @see eina_init()
 */
Eina_Bool
eina_array_init(void)
{
   _eina_array_log_dom = eina_log_domain_register("eina_array",
                                                  EINA_LOG_COLOR_DEFAULT);
   if (_eina_array_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: eina_array");
        return EINA_FALSE;
     }

#define EMS(n) eina_magic_string_static_set(n, n ## _STR)
   EMS(EINA_MAGIC_ARRAY);
   EMS(EINA_MAGIC_ARRAY_ITERATOR);
   EMS(EINA_MAGIC_ARRAY_ACCESSOR);
#undef EMS
   return EINA_TRUE;
}

/**
 * @internal
 * @brief Shut down the array module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function shuts down the array module set up by
 * eina_array_init(). It is called by eina_shutdown().
 *
 * @see eina_shutdown()
 */
Eina_Bool
eina_array_shutdown(void)
{
   eina_log_domain_unregister(_eina_array_log_dom);
   _eina_array_log_dom = -1;
   return EINA_TRUE;
}

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

EAPI Eina_Array *
eina_array_new(unsigned int step)
{
   Eina_Array *array;

   array = malloc(sizeof (Eina_Array));
   if (!array) return NULL;

   EINA_MAGIC_SET(array, EINA_MAGIC_ARRAY);

   array->version = EINA_ARRAY_VERSION;
   array->data = NULL;
   array->total = 0;
   array->count = 0;
   array->step = step;

   return array;
}

EAPI void
eina_array_free(Eina_Array *array)
{
   eina_array_flush(array);

   EINA_SAFETY_ON_NULL_RETURN(array);
   EINA_MAGIC_CHECK_ARRAY(array);
   MAGIC_FREE(array);
}

EAPI void
eina_array_step_set(Eina_Array *array,
		    unsigned int sizeof_eina_array,
		    unsigned int step)
{
   EINA_SAFETY_ON_NULL_RETURN(array);

   if (sizeof (Eina_Array) != sizeof_eina_array)
     {
	ERR("Unknow Eina_Array size ! Got %i, expected %i !\n",
	    sizeof_eina_array,
	    (int) sizeof (Eina_Array));
	/* Force memory to zero to provide a small layer of security */
	memset(array, 0, sizeof_eina_array);
	return;
     }

   array->version = EINA_ARRAY_VERSION;
   array->data = NULL;
   array->total = 0;
   array->count = 0;
   array->step = step;
   EINA_MAGIC_SET(array, EINA_MAGIC_ARRAY);
}

EAPI void
eina_array_flush(Eina_Array *array)
{
   EINA_SAFETY_ON_NULL_RETURN(array);
   EINA_MAGIC_CHECK_ARRAY(array);

   array->count = 0;
   array->total = 0;

   if (!array->data)
      return;

   free(array->data);
   array->data = NULL;
}

EAPI Eina_Bool
eina_array_remove(Eina_Array *array, Eina_Bool (*keep)(void *data,
                                                       void *gdata),
                  void *gdata)
{
   unsigned int i, j, size, count;
   void *data, **tmp;

   EINA_SAFETY_ON_NULL_RETURN_VAL(array, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(keep,  EINA_FALSE);
   EINA_MAGIC_CHECK_ARRAY(array);

   if (array->total == 0) return EINA_TRUE;
   // 1. walk through all items and shuffle down any items on top of
   // previously removed item slots
   for (count = array->count, tmp = array->data, j = 0, i = 0; i < count; i++)
     {
        data = tmp[i];
        if (keep(data, gdata) == EINA_TRUE)
          {
             // we keep it - store it (again) (and j will be <= i ALWAYS)
             tmp[j] = data;
             j++;
          }
     }
   array->count = j;
   // 2. if we reduced size by more than N (block size) then realloc back down
   if ((array->total - array->count) >= array->step)
     {
        if (array->count == 0)
          {
             free(array->data);
             array->total = 0;
             array->data = NULL;
          }
        else
          {
             // realloc back down - rounding up to the nearest step size
             size = ((array->count / array->step) + (array->count % array->step ? 1 : 0)) * array->step;
             tmp = realloc(array->data, sizeof(void *) * size);
             if (!tmp) return EINA_FALSE;
             array->total = size;
             array->data = tmp;
          }
     }
   return EINA_TRUE;
}

EAPI Eina_Iterator *
eina_array_iterator_new(const Eina_Array *array)
{
   Eina_Iterator_Array *it;

   EINA_SAFETY_ON_NULL_RETURN_VAL(array, NULL);
   EINA_MAGIC_CHECK_ARRAY(array);

   it = calloc(1, sizeof (Eina_Iterator_Array));
   if (!it) return NULL;

   EINA_MAGIC_SET(it,            EINA_MAGIC_ARRAY_ITERATOR);
   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->array = array;

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(eina_array_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(
         eina_array_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(eina_array_iterator_free);

   return &it->iterator;
}

EAPI Eina_Accessor *
eina_array_accessor_new(const Eina_Array *array)
{
   Eina_Accessor_Array *ac;

   EINA_SAFETY_ON_NULL_RETURN_VAL(array, NULL);
   EINA_MAGIC_CHECK_ARRAY(array);

   ac = calloc(1, sizeof (Eina_Accessor_Array));
   if (!ac) return NULL;

   EINA_MAGIC_SET(ac,            EINA_MAGIC_ARRAY_ACCESSOR);
   EINA_MAGIC_SET(&ac->accessor, EINA_MAGIC_ACCESSOR);

   ac->array = array;

   ac->accessor.version = EINA_ACCESSOR_VERSION;
   ac->accessor.get_at = FUNC_ACCESSOR_GET_AT(eina_array_accessor_get_at);
   ac->accessor.get_container = FUNC_ACCESSOR_GET_CONTAINER(
         eina_array_accessor_get_container);
   ac->accessor.free = FUNC_ACCESSOR_FREE(eina_array_accessor_free);
   ac->accessor.clone = FUNC_ACCESSOR_CLONE(eina_array_accessor_clone);

   return &ac->accessor;
}


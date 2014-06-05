/* EINA - EFL data type library
 * Copyright (C) 2012 - ProFUSION embedded systems
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

#include <stdlib.h>
#include <string.h>

#include "eina_config.h"
#include "eina_private.h"
#include "eina_log.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_inarray.h"

/*============================================================================*
*                                  Local                                     *
*============================================================================*/

/**
 * @cond LOCAL
 */

static const char EINA_MAGIC_INARRAY_STR[] = "Eina Inline Array";
static const char EINA_MAGIC_INARRAY_ITERATOR_STR[] = "Eina Inline Array Iterator";
static const char EINA_MAGIC_INARRAY_ACCESSOR_STR[] = "Eina Inline Array Accessor";

typedef struct _Eina_Iterator_Inarray Eina_Iterator_Inarray;
typedef struct _Eina_Accessor_Inarray Eina_Accessor_Inarray;

struct _Eina_Iterator_Inarray
{
   Eina_Iterator iterator;
   const Eina_Inarray *array;
   unsigned int pos;
   EINA_MAGIC
};

struct _Eina_Accessor_Inarray
{
   Eina_Accessor accessor;
   const Eina_Inarray *array;
   EINA_MAGIC
};

static int _eina_inarray_log_dom = -1;

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_inarray_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eina_inarray_log_dom, __VA_ARGS__)

#define EINA_MAGIC_CHECK_INARRAY(d, ...)                \
  do                                                    \
    {                                                   \
       if (!EINA_MAGIC_CHECK(d, EINA_MAGIC_INARRAY))    \
         {                                              \
            EINA_MAGIC_FAIL(d, EINA_MAGIC_INARRAY);     \
            return __VA_ARGS__;                         \
         }                                              \
    }                                                   \
  while(0)

#define EINA_MAGIC_CHECK_INARRAY_ITERATOR(d, ...)               \
  do                                                            \
    {                                                           \
       if (!EINA_MAGIC_CHECK(d, EINA_MAGIC_INARRAY_ITERATOR))   \
         {                                                      \
            EINA_MAGIC_FAIL(d, EINA_MAGIC_INARRAY_ITERATOR);    \
            return __VA_ARGS__;                                 \
         }                                                      \
    }                                                           \
  while(0)

#define EINA_MAGIC_CHECK_INARRAY_ACCESSOR(d, ...)               \
  do                                                            \
    {                                                           \
       if (!EINA_MAGIC_CHECK(d, EINA_MAGIC_INARRAY_ACCESSOR))   \
         {                                                      \
            EINA_MAGIC_FAIL(d, EINA_MAGIC_INARRAY_ACCESSOR);    \
            return __VA_ARGS__;                                 \
         }                                                      \
    }                                                           \
  while(0)

static void
_eina_inarray_setup(Eina_Inarray *array, unsigned int member_size, unsigned int step)
{
   EINA_MAGIC_SET(array, EINA_MAGIC_INARRAY);
   array->version = EINA_ARRAY_VERSION;
   array->member_size = member_size;
   array->len = 0;
   array->max = 0;
   array->step = (step > 0) ? step : 32;
   array->members = NULL;
}

static Eina_Bool
_eina_inarray_resize(Eina_Inarray *array, unsigned int new_size)
{
   unsigned int new_max;
   void *tmp;

   if (new_size < array->max) /* don't change this behaviour as eina_inarray_pop rely on it */
     return EINA_TRUE;

   if (new_size % array->step == 0)
     new_max = new_size;
   else
     new_max = ((new_size / array->step) + 1) * array->step;

   tmp = realloc(array->members, new_max * array->member_size);
   if ((!tmp) && (new_max > 0)) return EINA_FALSE;

   array->members = tmp;
   array->max = new_max;
   return EINA_TRUE;
}

static inline void *
_eina_inarray_get(const Eina_Inarray *array, unsigned int position)
{
   unsigned int offset = position * array->member_size;
   return (unsigned char *)array->members + offset;
}

static int
_eina_inarray_search(const Eina_Inarray *array, const void *data, Eina_Compare_Cb compare)
{
   const unsigned char *start, *found;
   start = array->members;
   found = bsearch(data, start, array->len, array->member_size, compare);
   if (!found)
     return -1;
   return (found - start) / array->member_size;
}

static unsigned int
_eina_inarray_search_sorted_near(const Eina_Inarray *array, const void *data, Eina_Compare_Cb compare, int *cmp)
{
   unsigned int start, last, middle;

   if (array->len == 0)
     {
        *cmp = -1;
        return 0;
     }
   else if (array->len == 1)
     {
        *cmp = compare(data, array->members);
        return 0;
     }

   start = 0;
   last = array->len - 1; /* inclusive */
   do
     {
        void *p;
        middle = start + (last - start) / 2; /* avoid overflow */
        p = _eina_inarray_get(array, middle);
        *cmp = compare(data, p);
        if (*cmp == 0)
          return middle;
        else if (*cmp > 0)
          start = middle + 1;
        else if (middle > 0)
          last = middle - 1;
        else
          break;
     }
   while (start <= last);
   return middle;
}


static Eina_Bool
_eina_inarray_iterator_next(Eina_Iterator_Inarray *it, void **data)
{
   EINA_MAGIC_CHECK_INARRAY_ITERATOR(it, EINA_FALSE);

   if (it->pos >= it->array->len)
     return EINA_FALSE;

   *data = _eina_inarray_get(it->array, it->pos);
   it->pos++;

   return EINA_TRUE;
}

static Eina_Bool
_eina_inarray_iterator_prev(Eina_Iterator_Inarray *it, void **data)
{
   EINA_MAGIC_CHECK_INARRAY_ITERATOR(it, EINA_FALSE);

   if (it->pos == 0)
     return EINA_FALSE;

   it->pos--;
   *data = _eina_inarray_get(it->array, it->pos);

   return EINA_TRUE;
}

static Eina_Inarray *
_eina_inarray_iterator_get_container(Eina_Iterator_Inarray *it)
{
   EINA_MAGIC_CHECK_INARRAY_ITERATOR(it, NULL);
   return (Eina_Inarray *)it->array;
}

static void
_eina_inarray_iterator_free(Eina_Iterator_Inarray *it)
{
   EINA_MAGIC_CHECK_INARRAY_ITERATOR(it);
   MAGIC_FREE(it);
}

static Eina_Bool
_eina_inarray_accessor_get_at(Eina_Accessor_Inarray *it, unsigned int pos, void **data)
{
   EINA_MAGIC_CHECK_INARRAY_ACCESSOR(it, EINA_FALSE);

   if (pos >= it->array->len)
     return EINA_FALSE;

   *data = _eina_inarray_get(it->array, pos);
   return EINA_TRUE;
}

static Eina_Inarray *
_eina_inarray_accessor_get_container(Eina_Accessor_Inarray *it)
{
   EINA_MAGIC_CHECK_INARRAY_ACCESSOR(it, NULL);
   return (Eina_Inarray *)it->array;
}

static void
_eina_inarray_accessor_free(Eina_Accessor_Inarray *it)
{
   EINA_MAGIC_CHECK_INARRAY_ACCESSOR(it);
   MAGIC_FREE(it);
}

/**
 * @endcond
 */


/*============================================================================*
*                                 Global                                     *
*============================================================================*/

/**
 * @internal
 * @brief Initialize the inline array module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function sets up the inline array module of Eina. It is called
 * by eina_init().
 *
 * @see eina_init()
 */
Eina_Bool
eina_inarray_init(void)
{
   _eina_inarray_log_dom = eina_log_domain_register("eina_inarray",
                                                    EINA_LOG_COLOR_DEFAULT);
   if (_eina_inarray_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: eina_inarray");
        return EINA_FALSE;
     }

#define EMS(n) eina_magic_string_static_set(n, n ## _STR)
   EMS(EINA_MAGIC_INARRAY);
   EMS(EINA_MAGIC_INARRAY_ITERATOR);
   EMS(EINA_MAGIC_INARRAY_ACCESSOR);
#undef EMS

   return EINA_TRUE;
}

/**
 * @internal
 * @brief Shut down the inline array module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function shuts down the inline array module set up by
 * eina_inarray_init(). It is called by eina_shutdown().
 *
 * @see eina_shutdown()
 */
Eina_Bool
eina_inarray_shutdown(void)
{
   eina_log_domain_unregister(_eina_inarray_log_dom);
   _eina_inarray_log_dom = -1;
   return EINA_TRUE;
}

/*============================================================================*
*                                   API                                      *
*============================================================================*/
EAPI Eina_Inarray *
eina_inarray_new(unsigned int member_size, unsigned int step)
{
   Eina_Inarray *ret;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(member_size == 0, NULL);

   ret = malloc(sizeof(*ret));
   if (!ret) return NULL;
   _eina_inarray_setup(ret, member_size, step);
   return ret;
}

EAPI void
eina_inarray_free(Eina_Inarray *array)
{
   if (!array)
     return;

   EINA_MAGIC_CHECK_INARRAY(array);
   free(array->members);
   free(array);
}

EAPI void
eina_inarray_step_set(Eina_Inarray *array,
		      unsigned int sizeof_eina_inarray,
		      unsigned int member_size,
		      unsigned int step)
{
   EINA_SAFETY_ON_NULL_RETURN(array);
   EINA_SAFETY_ON_TRUE_RETURN(member_size == 0);

   if (sizeof (Eina_Inarray) != sizeof_eina_inarray)
     {
        ERR("Unknow Eina_Inarray size ! Got %i, expected %i\n",
            sizeof_eina_inarray,
            (int) sizeof (Eina_Inarray));
        /* Force memory to zero to provide a small layer of security */
        memset(array, 0, sizeof_eina_inarray);
        return;
     }

   _eina_inarray_setup(array, member_size, step);
}

EAPI void
eina_inarray_flush(Eina_Inarray *array)
{
   EINA_MAGIC_CHECK_INARRAY(array);
   free(array->members);
   array->len = 0;
   array->max = 0;
   array->members = NULL;
}

EAPI Eina_Bool
eina_inarray_resize(Eina_Inarray *array, unsigned int new_size)
{
   Eina_Bool r;
   EINA_MAGIC_CHECK_INARRAY(array, EINA_FALSE);

   r = _eina_inarray_resize(array, new_size);
   if(!r) return EINA_FALSE;

   array->len = new_size;
   return EINA_TRUE;
}

EAPI int
eina_inarray_push(Eina_Inarray *array, const void *data)
{
   void *p;

   EINA_MAGIC_CHECK_INARRAY(array, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, -1);

   if (!_eina_inarray_resize(array, array->len + 1))
     return -1;

   p = _eina_inarray_get(array, array->len);
   memcpy(p, data, array->member_size);

   array->len++;
   return array->len - 1;
}

EAPI void *
eina_inarray_grow(Eina_Inarray *array, unsigned int size)
{
   void *p;

   EINA_MAGIC_CHECK_INARRAY(array, NULL);
   if (!size) return NULL;

   if (!_eina_inarray_resize(array, array->len + size))
     return NULL;

   p = _eina_inarray_get(array, array->len);
   array->len += size;

   return p;
}

EAPI int
eina_inarray_insert(Eina_Inarray *array, const void *data, Eina_Compare_Cb compare)
{
   const unsigned char *itr, *itr_end;
   unsigned int sz;

   EINA_MAGIC_CHECK_INARRAY(array, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(compare, -1);

   sz = array->member_size;
   itr = array->members;
   itr_end = itr + array->len * sz;
   for (; itr < itr_end; itr += sz)
     {
      unsigned int offset, position;
      int cmp = compare(itr, data);
      if (cmp <= 0)
        continue;

      offset = itr - (unsigned char *)array->members;
      position = offset / sz;
      if (!eina_inarray_insert_at(array, position, data))
        return -1;
      return position;
   }
   return eina_inarray_push(array, data);
}

EAPI int
eina_inarray_insert_sorted(Eina_Inarray *array, const void *data, Eina_Compare_Cb compare)
{
   unsigned int pos;
   int cmp;

   EINA_MAGIC_CHECK_INARRAY(array, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(compare, -1);

   pos = _eina_inarray_search_sorted_near(array, data, compare, &cmp);
   if (cmp > 0)
     pos++;

   if (!eina_inarray_insert_at(array, pos, data))
     return -1;
   return pos;
}

EAPI int
eina_inarray_remove(Eina_Inarray *array, const void *data)
{
   const unsigned char *itr, *itr_end;
   unsigned int position, sz;

   EINA_MAGIC_CHECK_INARRAY(array, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, -1);

   sz = array->member_size;
   if ((data >= array->members) &&
       (data < _eina_inarray_get(array, array->len)))
     {
        unsigned int offset = ((unsigned char *)data -
                               (unsigned char *)array->members);
        position = offset / sz;
        goto found;
     }

   itr = array->members;
   itr_end = itr + array->len * sz;
   for (; itr < itr_end; itr += sz)
     {
        if (memcmp(data, itr, sz) == 0)
          {
             unsigned int offset = itr - (unsigned char *)array->members;
             position = offset / sz;
             goto found;
          }
     }
   return -1;

found:
   if (!eina_inarray_remove_at(array, position))
     return -1;
   return position;
}

EAPI void *
eina_inarray_pop(Eina_Inarray *array)
{
   EINA_MAGIC_CHECK_INARRAY(array, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(array->len == 0, NULL);
   if (!_eina_inarray_resize(array, array->len - 1))
     return NULL;
   array->len--;
   return _eina_inarray_get(array, array->len);
}

EAPI void *
eina_inarray_nth(const Eina_Inarray *array, unsigned int position)
{
   EINA_MAGIC_CHECK_INARRAY(array, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(position >= array->len, NULL);
   return _eina_inarray_get(array, position);
}

EAPI Eina_Bool
eina_inarray_insert_at(Eina_Inarray *array, unsigned int position, const void *data)
{
   unsigned int sz;
   unsigned char *p;

   EINA_MAGIC_CHECK_INARRAY(array, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(position > array->len, EINA_FALSE);

   if (!_eina_inarray_resize(array, array->len + 1))
     return EINA_FALSE;

   p = _eina_inarray_get(array, position);
   sz = array->member_size;
   if (array->len > position)
     memmove(p + sz, p, (array->len - position) * sz);
   memcpy(p, data, sz);

   array->len++;
   return EINA_TRUE;
}

EAPI void *
eina_inarray_alloc_at(Eina_Inarray *array, unsigned int position, unsigned int member_count)
{
   unsigned int sz;
   unsigned char *p;

   EINA_MAGIC_CHECK_INARRAY(array, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(position > array->len, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(member_count == 0, NULL);

   if (!_eina_inarray_resize(array, array->len + member_count))
     return NULL;

   p = _eina_inarray_get(array, position);
   sz = array->member_size;
   if (array->len > position)
     memmove(p + member_count * sz, p, (array->len - position) * sz);

   array->len += member_count;
   return p;
}

EAPI Eina_Bool
eina_inarray_replace_at(Eina_Inarray *array, unsigned int position, const void *data)
{
   unsigned char *p;

   EINA_MAGIC_CHECK_INARRAY(array, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(position >= array->len, EINA_FALSE);

   p = _eina_inarray_get(array, position);
   memcpy(p, data, array->member_size);

   return EINA_TRUE;
}

EAPI Eina_Bool
eina_inarray_remove_at(Eina_Inarray *array, unsigned int position)
{
   EINA_MAGIC_CHECK_INARRAY(array, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(position >= array->len, EINA_FALSE);

   if (position + 1 < array->len)
     {
        unsigned int sz = array->member_size;
        unsigned char *p = _eina_inarray_get(array, position);
        memmove(p, p + sz, (array->len - position - 1) * sz);
     }

   if (!_eina_inarray_resize(array, array->len - 1)) return EINA_FALSE; /* should never fail as we reduce the buffer, but just let make compiler happy */
   array->len--;
   return EINA_TRUE;
}

EAPI void
eina_inarray_reverse(Eina_Inarray *array)
{
   size_t sz;
   unsigned char *fwd, *rev, *fwd_end;
   void *tmp;

   EINA_MAGIC_CHECK_INARRAY(array);

   if (array->len < 2)
     return;

   sz = array->member_size;

   tmp = alloca(sz);
   EINA_SAFETY_ON_NULL_RETURN(tmp);

   fwd = array->members;
   fwd_end = fwd + (array->len / 2) * sz;

   rev = fwd + (array->len - 1) * sz;

   for (; fwd < fwd_end; fwd += sz, rev -= sz)
     {
        memcpy(tmp, fwd, sz);
        memcpy(fwd, rev, sz);
        memcpy(rev, tmp, sz);
     }
}

EAPI void
eina_inarray_sort(Eina_Inarray *array, Eina_Compare_Cb compare)
{
   EINA_MAGIC_CHECK_INARRAY(array);
   EINA_SAFETY_ON_NULL_RETURN(compare);
   qsort(array->members, array->len, array->member_size, compare);
}

EAPI int
eina_inarray_search(const Eina_Inarray *array, const void *data, Eina_Compare_Cb compare)
{
   EINA_MAGIC_CHECK_INARRAY(array, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(compare, -1);
   return _eina_inarray_search(array, data, compare);
}

EAPI int
eina_inarray_search_sorted(const Eina_Inarray *array, const void *data, Eina_Compare_Cb compare)
{
   unsigned int pos;
   int cmp;

   EINA_MAGIC_CHECK_INARRAY(array, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(compare, -1);

   pos = _eina_inarray_search_sorted_near(array, data, compare, &cmp);
   if (cmp == 0)
     return pos;
   return -1;
}

EAPI Eina_Bool
eina_inarray_foreach(const Eina_Inarray *array, Eina_Each_Cb function, const void *user_data)
{
   unsigned char *itr, *itr_end;
   unsigned int sz;
   Eina_Bool ret = EINA_TRUE;

   EINA_MAGIC_CHECK_INARRAY(array, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(function, EINA_FALSE);

   sz = array->member_size;
   itr = array->members;
   itr_end = itr + array->len * sz;
   for (; (itr < itr_end) && (ret); itr += sz)
     ret = function(array, itr, (void *)user_data);
   return ret;
}

EAPI int
eina_inarray_foreach_remove(Eina_Inarray *array, Eina_Each_Cb match, const void *user_data)
{
   unsigned int i = 0, count = 0;

   EINA_MAGIC_CHECK_INARRAY(array, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(match, -1);

   while (i < array->len)
     {
        void *p = _eina_inarray_get(array, i);
        if (match(array, p, (void *)user_data) == EINA_FALSE)
          {
             i++;
             continue;
          }

        eina_inarray_remove_at(array, i);
        count++;
     }

   return count;
}

EAPI unsigned int
eina_inarray_count(const Eina_Inarray *array)
{
   EINA_MAGIC_CHECK_INARRAY(array, 0);
   return array->len;
}

EAPI Eina_Iterator *
eina_inarray_iterator_new(const Eina_Inarray *array)
{
   Eina_Iterator_Inarray *it;

   EINA_MAGIC_CHECK_INARRAY(array, NULL);

   it = calloc(1, sizeof(Eina_Iterator_Inarray));
   if (!it) return NULL;

   EINA_MAGIC_SET(it,            EINA_MAGIC_INARRAY_ITERATOR);
   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->array = array;

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_eina_inarray_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER
     (_eina_inarray_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_eina_inarray_iterator_free);

   return &it->iterator;
}

EAPI Eina_Iterator *
eina_inarray_iterator_reversed_new(const Eina_Inarray *array)
{
   Eina_Iterator_Inarray *it;

   EINA_MAGIC_CHECK_INARRAY(array, NULL);

   it = calloc(1, sizeof(Eina_Iterator_Inarray));
   if (!it) return NULL;

   EINA_MAGIC_SET(it,            EINA_MAGIC_INARRAY_ITERATOR);
   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->array = array;
   it->pos = array->len;

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_eina_inarray_iterator_prev);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER
     (_eina_inarray_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_eina_inarray_iterator_free);

   return &it->iterator;
}

EAPI Eina_Accessor *
eina_inarray_accessor_new(const Eina_Inarray *array)
{
   Eina_Accessor_Inarray *ac;

   EINA_MAGIC_CHECK_INARRAY(array, NULL);

   ac = calloc(1, sizeof(Eina_Accessor_Inarray));
   if (!ac) return NULL;

   EINA_MAGIC_SET(ac,            EINA_MAGIC_INARRAY_ACCESSOR);
   EINA_MAGIC_SET(&ac->accessor, EINA_MAGIC_ACCESSOR);

   ac->array = array;

   ac->accessor.version = EINA_ACCESSOR_VERSION;
   ac->accessor.get_at = FUNC_ACCESSOR_GET_AT(_eina_inarray_accessor_get_at);
   ac->accessor.get_container = FUNC_ACCESSOR_GET_CONTAINER
     (_eina_inarray_accessor_get_container);
   ac->accessor.free = FUNC_ACCESSOR_FREE(_eina_inarray_accessor_free);

   return &ac->accessor;
}

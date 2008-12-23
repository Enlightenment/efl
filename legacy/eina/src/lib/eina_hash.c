/* EINA - EFL data type library
 * Copyright (C) 2002-2008 Carsten Haitzler, Gustavo Sverzut Barbieri,
 *                         Vincent Torri, Jorge Luis Zapata Muga, Cedric Bail
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
#include <stdint.h>

#include "eina_hash.h"
#include "eina_rbtree.h"
#include "eina_error.h"
#include "eina_private.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

#define EINA_MAGIC_CHECK_HASH(d)					\
  do {									\
     if (!EINA_MAGIC_CHECK(d, EINA_MAGIC_HASH))				\
       EINA_MAGIC_FAIL(d, EINA_MAGIC_HASH);				\
  } while(0)

#define EINA_MAGIC_CHECK_HASH_ITERATOR(d)				\
  do {									\
     if (!EINA_MAGIC_CHECK(d, EINA_MAGIC_HASH_ITERATOR))		\
       EINA_MAGIC_FAIL(d, EINA_MAGIC_HASH_ITERATOR);			\
  } while(0)

#define EINA_HASH_BUCKET_SIZE 8
#define EINA_HASH_SMALL_BUCKET_SIZE 5

#define EINA_HASH_RBTREE_MASK 0xFFF

typedef struct _Eina_Hash_Head Eina_Hash_Head;
typedef struct _Eina_Hash_El Eina_Hash_El;
typedef struct _Eina_Hash_Foreach_Data Eina_Hash_Foreach_Data;
typedef struct _Eina_Iterator_Hash Eina_Iterator_Hash;
typedef struct _Eina_Hash_Each Eina_Hash_Each;

struct _Eina_Hash
{
   Eina_Key_Length key_length_cb;
   Eina_Key_Cmp key_cmp_cb;
   Eina_Key_Hash key_hash_cb;
   Eina_Free_Cb data_free_cb;

   Eina_Rbtree **buckets;
   int size;
   int mask;

   int population;

   EINA_MAGIC;
};

struct _Eina_Hash_Head
{
   EINA_RBTREE;
   int hash;

   Eina_Rbtree *head;
};

struct _Eina_Hash_El
{
   EINA_RBTREE;
   Eina_Hash_Tuple tuple;
   Eina_Bool begin : 1;
};

struct _Eina_Hash_Foreach_Data
{
   Eina_Hash_Foreach cb;
   const void *fdata;
};

typedef void *(*Eina_Iterator_Get_Content_Callback)(Eina_Iterator_Hash *it);
#define FUNC_ITERATOR_GET_CONTENT(Function) ((Eina_Iterator_Get_Content_Callback)Function)

struct _Eina_Iterator_Hash
{
   Eina_Iterator iterator;

   Eina_Iterator_Get_Content_Callback get_content;
   const Eina_Hash *hash;

   Eina_Iterator *current;
   Eina_Iterator *list;
   Eina_Hash_Head *eh;
   Eina_Hash_El *el;
   int bucket;

   int index;

   EINA_MAGIC;
};

struct _Eina_Hash_Each
{
   Eina_Hash_Head *eh;
   const Eina_Hash_El *el;
   const void *data;
};

static int _eina_hash_init_count = 0;

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
# define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
# define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                        +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

static inline int
_eina_hash_hash_rbtree_cmp_hash(const Eina_Hash_Head *eh, const int *hash, __UNUSED__ int key_length, __UNUSED__ void *data)
{
   return eh->hash - *hash;
}

static Eina_Rbtree_Direction
_eina_hash_hash_rbtree_cmp_node(const Eina_Hash_Head *left, const Eina_Hash_Head *right, __UNUSED__ void *data)
{
   if (left->hash - right->hash < 0)
     return EINA_RBTREE_LEFT;
   return EINA_RBTREE_RIGHT;
}

static inline int
_eina_hash_key_rbtree_cmp_key_data(const Eina_Hash_El *el, const Eina_Hash_Tuple *tuple, __UNUSED__ unsigned int key_length, Eina_Key_Cmp cmp)
{
   int result;

   result = cmp(el->tuple.key, el->tuple.key_length, tuple->key, tuple->key_length);

   if (result == 0 && tuple->data && tuple->data != el->tuple.data)
     return 1;
   return result;
}

static Eina_Rbtree_Direction
_eina_hash_key_rbtree_cmp_node(const Eina_Hash_El *left, const Eina_Hash_El *right, Eina_Key_Cmp cmp)
{
   int result;

   result = cmp(left->tuple.key, left->tuple.key_length,
		right->tuple.key, right->tuple.key_length);

   if (result < 0)
     return EINA_RBTREE_LEFT;
   return EINA_RBTREE_RIGHT;
}

static inline Eina_Bool
eina_hash_add_alloc_by_hash(Eina_Hash *hash,
			    const void *key, int key_length, int alloc_length,
			    int key_hash,
			    const void *data)
{
   Eina_Hash_El *el = NULL;
   Eina_Hash_Head *eh;
   Eina_Error error = 0;
   int hash_num;

   EINA_MAGIC_CHECK_HASH(hash);
   if ((!hash) || (!key) || (!data)) goto on_error;
   error = EINA_ERROR_OUT_OF_MEMORY;

   /* Apply eina mask to hash. */
   hash_num = key_hash & hash->mask;
   key_hash &= EINA_HASH_RBTREE_MASK;

   if (!hash->buckets)
     {
	hash->buckets = malloc(sizeof (Eina_Rbtree*) * hash->size);
	memset(hash->buckets, 0, sizeof (Eina_Rbtree*) * hash->size);

	eh = NULL;
     }
   else
     {
	/* Look up for head node. */
	eh = (Eina_Hash_Head*) eina_rbtree_inline_lookup(hash->buckets[hash_num],
							 &key_hash, 0,
							 EINA_RBTREE_CMP_KEY_CB(_eina_hash_hash_rbtree_cmp_hash), NULL);
     }

   if (!eh)
     {
	/* If not found allocate it and a element. */
	eh = malloc(sizeof (Eina_Hash_Head) + sizeof (Eina_Hash_El) + alloc_length);
	if (!eh) goto on_error;
	eh->hash = key_hash;
	eh->head = NULL;

	hash->buckets[hash_num] = eina_rbtree_inline_insert(hash->buckets[hash_num], EINA_RBTREE_GET(eh),
							    EINA_RBTREE_CMP_NODE_CB(_eina_hash_hash_rbtree_cmp_node), NULL);

	el = (Eina_Hash_El*) (eh + 1);
	el->begin = EINA_TRUE;
     }

   if (!el)
     {
	/*
	  Alloc every needed things
	  (No more lookup as we expect to support more than one item for one key).
	*/
	el = malloc(sizeof (Eina_Hash_El) + alloc_length);
	if (!el) goto on_error;
	el->begin = EINA_FALSE;
     }

   /* Setup the element */
   el->tuple.key_length = key_length;
   el->tuple.data = (void *) data;
   if (alloc_length > 0)
     {
	el->tuple.key = (char *) (el + 1);
	memcpy((char *) el->tuple.key, key, alloc_length);
     }
   else
     {
	el->tuple.key = key;
     }

   /* add the new element to the hash. */
   eh->head = eina_rbtree_inline_insert(eh->head, EINA_RBTREE_GET(el),
					EINA_RBTREE_CMP_NODE_CB(_eina_hash_key_rbtree_cmp_node), hash->key_cmp_cb);
   hash->population++;
   return EINA_TRUE;

 on_error:
   eina_error_set(error);
   return EINA_FALSE;
}

static Eina_Bool
_eina_hash_rbtree_each(__UNUSED__ const Eina_Rbtree *container, const Eina_Hash_Head *eh, Eina_Hash_Each *data)
{
   Eina_Iterator *it;
   Eina_Hash_El *el;
   Eina_Bool found = EINA_TRUE;

   it = eina_rbtree_iterator_prefix(eh->head);
   while (eina_iterator_next(it, (void**) &el))
     {
	if (el->tuple.data == data->data)
	  {
	     data->el = el;
	     data->eh = (Eina_Hash_Head*) eh;
	     found = EINA_FALSE;
	     break ;
	  }
     }

   eina_iterator_free(it);
   return found;
}

static inline Eina_Hash_El *
_eina_hash_find_by_hash(const Eina_Hash *hash, Eina_Hash_Tuple *tuple, int key_hash, Eina_Hash_Head **eh)
{
   Eina_Hash_El *el;
   int rb_hash = key_hash & EINA_HASH_RBTREE_MASK;

   key_hash &= hash->mask;

   if (!hash->buckets) return NULL;

   *eh = (Eina_Hash_Head*) eina_rbtree_inline_lookup(hash->buckets[key_hash],
						    &rb_hash, 0,
						    EINA_RBTREE_CMP_KEY_CB(_eina_hash_hash_rbtree_cmp_hash), NULL);
   if (!*eh) return NULL;

   el = (Eina_Hash_El*) eina_rbtree_inline_lookup((*eh)->head,
						  tuple, 0,
						  EINA_RBTREE_CMP_KEY_CB(_eina_hash_key_rbtree_cmp_key_data), hash->key_cmp_cb);

   return el;
}

static inline Eina_Hash_El *
_eina_hash_find_by_data(const Eina_Hash *hash, const void *data, int *key_hash, Eina_Hash_Head **eh)
{
   Eina_Hash_Each each;
   Eina_Iterator *it;
   int hash_num;

   if (!hash->buckets) return NULL;

   each.el = NULL;
   each.data = data;

   for (hash_num = 0; hash_num < hash->size; hash_num++)
     {
	if (!hash->buckets[hash_num])
	  continue;
	it = eina_rbtree_iterator_prefix(hash->buckets[hash_num]);
	eina_iterator_foreach(it, EINA_EACH(_eina_hash_rbtree_each), &each);
	eina_iterator_free(it);

	if (each.el)
	  {
	    *key_hash = hash_num;
	    *eh = each.eh;
	    return (Eina_Hash_El*) each.el;
	  }
     }

   return NULL;
}

static void
_eina_hash_el_free(Eina_Hash_El *el, Eina_Hash *hash)
{
   if (hash->data_free_cb)
     hash->data_free_cb(el->tuple.data);
   if (el->begin == EINA_FALSE) free(el);
}

static void
_eina_hash_head_free(Eina_Hash_Head *eh, Eina_Hash *hash)
{
   eina_rbtree_delete(eh->head, EINA_RBTREE_FREE_CB(_eina_hash_el_free), hash);
   free(eh);
}

static Eina_Bool
_eina_hash_del_by_hash_el(Eina_Hash *hash, Eina_Hash_El *el, Eina_Hash_Head *eh, int key_hash)
{
   eh->head = eina_rbtree_inline_remove(eh->head, EINA_RBTREE_GET(el), EINA_RBTREE_CMP_NODE_CB(_eina_hash_key_rbtree_cmp_node), hash->key_cmp_cb);
   _eina_hash_el_free(el, hash);

   if (!eh->head)
     {
	key_hash &= hash->mask;

	hash->buckets[key_hash] = eina_rbtree_inline_remove(hash->buckets[key_hash], EINA_RBTREE_GET(eh), EINA_RBTREE_CMP_NODE_CB(_eina_hash_hash_rbtree_cmp_node), NULL);
	free(eh);
     }

   hash->population--;
   if (hash->population == 0)
     {
	free(hash->buckets);
	hash->buckets = NULL;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_hash_del_by_key_hash(Eina_Hash *hash, const void *key, int key_length, int key_hash, const void *data)
{
   Eina_Hash_El *el;
   Eina_Hash_Head *eh;
   Eina_Hash_Tuple tuple;

   EINA_MAGIC_CHECK_HASH(hash);
   if (!hash) return EINA_FALSE;
   if (!key) return EINA_FALSE;
   if (!hash->buckets) return EINA_FALSE;

   tuple.key = (void *) key;
   tuple.key_length = key_length;
   tuple.data = (void *) data;

   el = _eina_hash_find_by_hash(hash, &tuple, key_hash, &eh);
   if (!el) return EINA_FALSE;
   return _eina_hash_del_by_hash_el(hash, el, eh, key_hash);
}

static Eina_Bool
_eina_hash_del_by_key(Eina_Hash *hash, const void *key, const void *data)
{
   int key_length, key_hash;

   EINA_MAGIC_CHECK_HASH(hash);
   if (!hash) return EINA_FALSE;
   if (!key) return EINA_FALSE;
   if (!hash->buckets) return EINA_FALSE;

   key_length = hash->key_length_cb(key);
   key_hash = hash->key_hash_cb(key, key_length);
   return _eina_hash_del_by_key_hash(hash, key, key_length, key_hash, data);
}

static unsigned int
_eina_string_key_length(const char *key)
{
   if (!key) return 0;
   return strlen(key) + 1;
}

static int
_eina_string_key_cmp(const char *key1, __UNUSED__ int key1_length,
		     const char *key2, __UNUSED__ int key2_length)
{
   return strcmp(key1, key2);
}

static unsigned int
_eina_int32_key_length(__UNUSED__ const uint32_t *key)
{
  return 4;
}

static int
_eina_int32_key_cmp(const uint32_t *key1, __UNUSED__ int key1_length,
		    const uint32_t *key2, __UNUSED__ int key2_length)
{
  if (*key1 > *key2)
    return 1;
  if (*key1 < *key2)
    return -1;
  return 0;
}

static unsigned int
_eina_int64_key_length(__UNUSED__ const uint32_t *key)
{
  return 8;
}

static int
_eina_int64_key_cmp(const uint64_t *key1, __UNUSED__ int key1_length,
		    const uint64_t *key2, __UNUSED__ int key2_length)
{
  if (*key1 > *key2)
    return 1;
  if (*key1 < *key2)
    return -1;
  return 0;
}


static Eina_Bool
_eina_foreach_cb(const Eina_Hash *hash, Eina_Hash_Tuple *data, Eina_Hash_Foreach_Data *fdata)
{
   return fdata->cb((Eina_Hash *) hash, data->key, data->data, (void*) fdata->fdata);
}

static void *
_eina_hash_iterator_data_get_content(Eina_Iterator_Hash *it)
{
   Eina_Hash_El *stuff;

   EINA_MAGIC_CHECK_HASH_ITERATOR(it);

   stuff = it->el;

   if (!stuff) return NULL;
   return stuff->tuple.data;
}

static void *
_eina_hash_iterator_key_get_content(Eina_Iterator_Hash *it)
{
   Eina_Hash_El *stuff;

   EINA_MAGIC_CHECK_HASH_ITERATOR(it);

   stuff = it->el;

   if (!stuff) return NULL;
   return (void *) stuff->tuple.key;
}

static Eina_Hash_Tuple *
_eina_hash_iterator_tuple_get_content(Eina_Iterator_Hash *it)
{
   Eina_Hash_El *stuff;

   EINA_MAGIC_CHECK_HASH_ITERATOR(it);

   stuff = it->el;

   if (!stuff) return NULL;
   return &stuff->tuple;
}

static Eina_Bool
_eina_hash_iterator_next(Eina_Iterator_Hash *it, void **data)
{
   Eina_Bool ok;
   int bucket;

   if (!(it->index < it->hash->population)) return EINA_FALSE;
   if (it->current == NULL)
     {
	ok = EINA_FALSE;
	bucket = 0;
	it->index = -1;
     }
   else
     {
	ok = eina_iterator_next(it->list, (void**) &it->el);
	if (!ok)
	  {
	     eina_iterator_free(it->list);
	     it->list = NULL;

	     ok = eina_iterator_next(it->current, (void**) &it->eh);
	     if (!ok)
	       {
		  eina_iterator_free(it->current);
		  it->current = NULL;
		  it->bucket++;
	       }
	     else
	       {
		  it->list = eina_rbtree_iterator_prefix(it->eh->head);
		  ok = eina_iterator_next(it->list, (void**) &it->el);
	       }
	  }

	bucket = it->bucket;
     }

   if (ok == EINA_FALSE)
     {
	while (bucket < it->hash->size)
	  {
	     if (it->hash->buckets[bucket] != NULL)
	       {
		  it->current = eina_rbtree_iterator_prefix(it->hash->buckets[bucket]);
		  ok = eina_iterator_next(it->current, (void**) &it->eh);
		  if (ok) break ;
		  eina_iterator_free(it->current);
		  it->current = NULL;
	       }
	     ++bucket;
	  }
	if (it->list) eina_iterator_free(it->list);
	it->list = eina_rbtree_iterator_prefix(it->eh->head);
	ok = eina_iterator_next(it->list, (void**) &it->el);
	if (bucket == it->hash->size) ok = EINA_FALSE;
     }

   it->index++;
   it->bucket = bucket;

   if (ok && data)
     *data = it->get_content(it);

   return ok;
}

static void *
_eina_hash_iterator_get_container(Eina_Iterator_Hash *it)
{
   EINA_MAGIC_CHECK_HASH_ITERATOR(it);
   return (void *) it->hash;
}

static void
_eina_hash_iterator_free(Eina_Iterator_Hash *it)
{
   EINA_MAGIC_CHECK_HASH_ITERATOR(it);
   if (it->current) eina_iterator_free(it->current);
   if (it->list) eina_iterator_free(it->list);
   free(it);
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
 * @addtogroup Eina_Hash_Group Hash Table
 *
 * @brief give a small description here : what it is for, what it does
 * , etc...
 *
 * Hash API. Give some hints about the use (functions that must be
 * used like init / shutdown), general use, etc... Give also a link to
 * tutorial below.
 *
 * @section hashtable_algo Algorithm
 *
 * Give here the algorithm used in the implementation
 *
 * @section hashtable_perf Performance
 *
 * Give some hints about performance if it is possible, and an image !
 *
 * @section hashtable_tutorial Tutorial
 *
 * Here is a fantastic tutorial about our hash table
 *
 * @{
 */

/**
 * @brief Initialize the hash table module.
 *
 * @return 1 or greater on success, 0 on error.
 *
 * This function just sets up the error module or Eina. It is also
 * called by eina_init(). It returns 0 on failure, otherwise it
 * returns the number of times eina_error_init() has already been
 * called.
 */
EAPI int
eina_hash_init(void)
{
   if (!_eina_hash_init_count)
     eina_error_init();

   return ++_eina_hash_init_count;
}

/**
 * @brief Shut down the hash table module.
 *
 * @return 0 when the error module is completely shut down, 1 or
 * greater otherwise.
 *
 * This function just shut down the error module set up by
 * eina_hash_init(). It is also called by eina_shutdown(). It returns
 * 0 when it is called the same number of times than
 * eina_error_init().
 */
EAPI int
eina_hash_shutdown(void)
{
   if (_eina_hash_init_count == 1)
     eina_error_shutdown();
   return --_eina_hash_init_count;
}
EAPI Eina_Hash *
eina_hash_new(Eina_Key_Length key_length_cb,
	      Eina_Key_Cmp key_cmp_cb,
	      Eina_Key_Hash key_hash_cb,
	      Eina_Free_Cb data_free_cb,
	      int buckets_power_size)
{
   /* FIXME: Use mempool. */
   Eina_Hash *new;

   eina_error_set(0);
   if (!key_length_cb || !key_cmp_cb) return NULL;
   if (buckets_power_size < 3 || buckets_power_size > 16) return NULL;

   new = malloc(sizeof (Eina_Hash));
   if (!new) goto on_error;

   new->key_length_cb = key_length_cb;
   new->key_cmp_cb = key_cmp_cb;
   new->key_hash_cb = key_hash_cb;
   new->data_free_cb = data_free_cb;
   new->buckets = NULL;
   new->population = 0;

   new->size = 1 << buckets_power_size;
   new->mask = new->size - 1;

   EINA_MAGIC_SET(new, EINA_MAGIC_HASH);

   return new;

 on_error:
   eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
   return NULL;
}

EAPI Eina_Hash *
eina_hash_string_djb2_new(Eina_Free_Cb data_free_cb)
{
   return eina_hash_new(EINA_KEY_LENGTH(_eina_string_key_length),
			EINA_KEY_CMP(_eina_string_key_cmp),
			EINA_KEY_HASH(eina_hash_djb2),
			data_free_cb,
			EINA_HASH_BUCKET_SIZE);
}

EAPI Eina_Hash *
eina_hash_string_superfast_new(Eina_Free_Cb data_free_cb)
{
   return eina_hash_new(EINA_KEY_LENGTH(_eina_string_key_length),
			EINA_KEY_CMP(_eina_string_key_cmp),
			EINA_KEY_HASH(eina_hash_superfast),
			data_free_cb,
			EINA_HASH_BUCKET_SIZE);
}

EAPI Eina_Hash *
eina_hash_string_small_new(Eina_Free_Cb data_free_cb)
{
   return eina_hash_new(EINA_KEY_LENGTH(_eina_string_key_length),
			EINA_KEY_CMP(_eina_string_key_cmp),
			EINA_KEY_HASH(eina_hash_superfast),
			data_free_cb,
			EINA_HASH_SMALL_BUCKET_SIZE);
}

EAPI Eina_Hash *
eina_hash_int32_new(Eina_Free_Cb data_free_cb)
{
   return eina_hash_new(EINA_KEY_LENGTH(_eina_int32_key_length),
			EINA_KEY_CMP(_eina_int32_key_cmp),
			EINA_KEY_HASH(eina_hash_int32),
			data_free_cb,
			EINA_HASH_BUCKET_SIZE);
}

EAPI Eina_Hash *
eina_hash_int64_new(Eina_Free_Cb data_free_cb)
{
   return eina_hash_new(EINA_KEY_LENGTH(_eina_int64_key_length),
			EINA_KEY_CMP(_eina_int64_key_cmp),
			EINA_KEY_HASH(eina_hash_int64),
			data_free_cb,
			EINA_HASH_BUCKET_SIZE);
}

EAPI Eina_Hash *
eina_hash_pointer_new(Eina_Free_Cb data_free_cb)
{
#ifdef __LP64__
   return eina_hash_new(EINA_KEY_LENGTH(_eina_int64_key_length),
			EINA_KEY_CMP(_eina_int64_key_cmp),
			EINA_KEY_HASH(eina_hash_int64),
			data_free_cb,
			EINA_HASH_BUCKET_SIZE);
#else
   return eina_hash_new(EINA_KEY_LENGTH(_eina_int32_key_length),
			EINA_KEY_CMP(_eina_int32_key_cmp),
			EINA_KEY_HASH(eina_hash_int32),
			data_free_cb,
			EINA_HASH_BUCKET_SIZE);
#endif
}

/**
 * Retrieves the number of buckets available in the given hash table.
 * @param hash The given hash table.
 * @return @c 256 if @p hash is not @c NULL.  @c 0 otherwise.
 */
EAPI int
eina_hash_population(const Eina_Hash *hash)
{
   EINA_MAGIC_CHECK_HASH(hash);
   if (!hash) return 0;
   return hash->population;
}

/**
 * Free an entire hash table
 * @param hash The hash table to be freed
 *
 * This function frees up all the memory allocated to storing the specified
 * hash tale pointed to by @p hash. Any entries in the table that the program
 * has no more pointers for elsewhere may now be lost, so this should only be
 * called if the program has lready freed any allocated data in the hash table
 * or has the pointers for data in teh table stored elswehere as well.
 *
 * Example:
 * @code
 * extern Eina_Hash *hash;
 *
 * eina_hash_free(hash);
 * hash = NULL;
 * @endcode
 */
EAPI void
eina_hash_free(Eina_Hash *hash)
{
   int i;

   EINA_MAGIC_CHECK_HASH(hash);
   if (!hash) return;

   if (hash->buckets)
     for (i = 0; i < hash->size; i++)
       eina_rbtree_delete(hash->buckets[i], EINA_RBTREE_FREE_CB(_eina_hash_head_free), hash);
   free(hash);
}

/**
 * Adds an entry to the given hash table.
 *
 * @p key is expected to be a unique string within the hash table.
 * Otherwise, you cannot be sure which inserted data pointer will be
 * accessed with @ref eina_hash_find , and removed with
 * @ref eina_hash_del .
 *
 * @p key_hash is expected to always match @p key. Otherwise, you
 * cannot be sure to find it again with @ref eina_hash_find_by_hash.
 *
 * Key strings are case sensitive.
 *
 * @ref eina_error_get should be used to determine if an
 * allocation error occurred during this function.
 *
 * @param   hash The given hash table.  Can be @c NULL.
 * @param   key  A unique key.  Can be @c NULL.
 * @param   key_length Should be the length of @p key (don't forget to count '\\0' for string).
 * @param   key_hash The hash that will always match key.
 * @param   data Data to associate with the string given by @p key.
 * @return  Will return EINA_FALSE if an error occured, and EINA_TRUE if every
 *          thing goes fine.
 */
EAPI Eina_Bool
eina_hash_add_by_hash(Eina_Hash *hash,
		      const void *key, int key_length, int key_hash,
		      const void *data)
{
   return eina_hash_add_alloc_by_hash(hash, key, key_length, key_length, key_hash, data);
}

/**
 * Adds an entry to the given hash table and does not duplicate the string key.
 *
 * @p key is expected to be a unique string within the hash table.
 * Otherwise, you cannot be sure which inserted data pointer will be
 * accessed with @ref eina_hash_find , and removed with
 * @ref eina_hash_del . This call does not make a copy of the key so it must
 * be a string constant or stored elsewhere (in the object being added) etc.
 *
 * @p key_hash is expected to always match @p key. Otherwise, you
 * cannot be sure to find it again with @ref eina_hash_find_by_hash.
 *
 * Key strings are case sensitive.
 *
 * @ref eina_error_get should be used to determine if an
 * allocation error occurred during this function.
 *
 * @param   hash The given hash table.  Can be @c NULL.
 * @param   key  A unique key.  Can be @c NULL.
 * @param   key_length Should be the length of @p key (don't forget to count '\\0' for string).
 * @param   key_hash The hash that will always match key.
 * @param   data Data to associate with the string given by @p key.
 * @return  Will return EINA_FALSE if an error occured, and EINA_TRUE if every
 *          thing goes fine.
 */
EAPI Eina_Bool
eina_hash_direct_add_by_hash(Eina_Hash *hash,
			     const void *key, int key_length, int key_hash,
			     const void *data)
{
   return eina_hash_add_alloc_by_hash(hash, key, key_length, 0, key_hash, data);
}

/**
 * Adds an entry to the given hash table.
 *
 * @p key is expected to be a unique string within the hash table.
 * Otherwise, you cannot be sure which inserted data pointer will be
 * accessed with @ref eina_hash_find , and removed with
 * @ref eina_hash_del .
 *
 * Key strings are case sensitive.
 *
 * @ref eina_error_get() should be used to determine if an
 * allocation error occurred during this function.
 *
 * @param   hash The given hash table.  Can be @c NULL.
 * @param   key  A unique key.  Can be @c NULL.
 * @param   data Data to associate with the string given by @p key.
 * @return  Will return EINA_FALSE if an error occured, and EINA_TRUE if every
 *          thing goes fine.
 */
EAPI Eina_Bool
eina_hash_add(Eina_Hash *hash, const void *key, const void *data)
{
   unsigned int key_length;
   int key_hash;

   EINA_MAGIC_CHECK_HASH(hash);
   if ((!hash) || (!key) || (!data)) return EINA_FALSE;

   key_length = hash->key_length_cb(key);
   key_hash = hash->key_hash_cb(key, key_length);

   return eina_hash_add_by_hash(hash, key, key_length, key_hash, data);
}

/**
 * Adds an entry to the given hash table and does not duplicate the string key.
 *
 * @p key is expected to be a unique string within the hash table.
 * Otherwise, you cannot be sure which inserted data pointer will be
 * accessed with @ref eina_hash_find , and removed with
 * @ref eina_hash_del . This call does not make a copy of the key so it must
 * be a string constant or stored elsewhere (in the object being added) etc.
 *
 * Key strings are case sensitive.
 *
 * @ref eina_error_get() should be used to determine if an
 * allocation error occurred during this function.
 *
 * @param   hash The given hash table.  Can be @c NULL.
 * @param   key  A unique key.  Can be @c NULL.
 * @param   data Data to associate with the string given by @p key.
 * @return  Will return EINA_FALSE if an error occured, and EINA_TRUE if every
 *          thing goes fine.
 */
EAPI Eina_Bool
eina_hash_direct_add(Eina_Hash *hash, const void *key, const void *data)
{
   int key_length;
   int key_hash;

   EINA_MAGIC_CHECK_HASH(hash);
   if ((!hash) || (!key) || (!data)) return EINA_FALSE;

   key_length = hash->key_length_cb(key);
   key_hash = hash->key_hash_cb(key, key_length);

   return eina_hash_direct_add_by_hash(hash, key, key_length, key_hash, data);
}

/**
 * Removes the entry identified by @p key and @p key_hash from the given
 * hash table.
 *
 * @param   hash The given hash table.
 * @param   key  The key.  Cannot be @c NULL.
 * @param   key_length Should be the length of @p key (don't forget to count '\\0' for string).
 * @param   key_hash The hash that always match the key.
 * @return  Will return EINA_FALSE if an error occured, and EINA_TRUE if every
 *          thing goes fine.
 *
 * @note if you don't have the key_hash, use eina_hash_del_by_key() instead.
 * @note if you don't have the key, use eina_hash_del_by_data() instead.
 */
EAPI Eina_Bool
eina_hash_del_by_key_hash(Eina_Hash *hash, const void *key, int key_length, int key_hash)
{
   return _eina_hash_del_by_key_hash(hash, key, key_length, key_hash, NULL);
}

/**
 * Removes the entry identified by @p key from the given hash table.
 *
 * This version will calculate key length and hash by using functions
 * provided to hash creation function.
 *
 * @param   hash The given hash table.
 * @param   key  The key.  Cannot be @c NULL.
 * @return  Will return EINA_FALSE if an error occured, and EINA_TRUE if every
 *          thing goes fine.
 *
 * @note if you already have the key_hash, use eina_hash_del_by_key_hash() instead.
 * @note if you don't have the key, use eina_hash_del_by_data() instead.
 */
EAPI Eina_Bool
eina_hash_del_by_key(Eina_Hash *hash, const void *key)
{
   return _eina_hash_del_by_key(hash, key, NULL);
}

/**
 * Removes the entry identified by @p data from the given hash table.
 *
 * This version is slow since there is no quick access to nodes based on data.
 *
 * @param   hash The given hash table.
 * @param   data  The data value to search and remove.
 * @return  Will return EINA_FALSE if an error occured, and EINA_TRUE if every
 *          thing goes fine.
 *
 * @note if you already have the key, use eina_hash_del_by_key() or eina_hash_del_by_key_hash() instead.
 */
EAPI Eina_Bool
eina_hash_del_by_data(Eina_Hash *hash, const void *data)
{
   Eina_Hash_El *el;
   Eina_Hash_Head *eh;
   int key_hash;

   EINA_MAGIC_CHECK_HASH(hash);
   if (!hash) return EINA_FALSE;
   if (!data) return EINA_FALSE;

   el = _eina_hash_find_by_data(hash, data, &key_hash, &eh);
   if (!el) return EINA_FALSE;
   if (el->tuple.data != data) return EINA_FALSE;
   return _eina_hash_del_by_hash_el(hash, el, eh, key_hash);
}

/**
 * Removes the entry identified by @p key and @p key_hash or @p data from the given
 * hash table.
 *
 * If @p key is @c NULL, then @p data is used to find a match to
 * remove.
 *
 * @param   hash The given hash table.
 * @param   key  The key.  Can be @c NULL.
 * @param   key_length Should be the length of @p key (don't forget to count '\\0' for string).
 * @param   key_hash The hash that always match the key. Ignored if @p key is @c NULL.
 * @param   data The data pointer to remove if @p key is @c NULL.
 *               Otherwise, not required and can be @c NULL.
 * @return  Will return EINA_FALSE if an error occured, and EINA_TRUE if every
 *          thing goes fine.
 *
 * @note if you know you already have the key, use eina_hash_del_by_key_hash(),
 *       if you know you don't have the key, use eina_hash_del_by_data()
 *       directly.
 */
EAPI Eina_Bool
eina_hash_del_by_hash(Eina_Hash *hash, const void *key, int key_length, int key_hash, const void *data)
{
   if (key) return _eina_hash_del_by_key_hash(hash, key, key_length, key_hash, data);
   else return eina_hash_del_by_data(hash, data);
}

/**
 * Removes the entry identified by @p key or @p data from the given
 * hash table.
 *
 * If @p key is @c NULL, then @p data is used to find a match to
 * remove.
 *
 * @param   hash The given hash table.
 * @param   key  The key.  Can be @c NULL.
 * @param   data The data pointer to remove if @p key is @c NULL.
 *               Otherwise, not required and can be @c NULL.
 * @return  Will return EINA_FALSE if an error occured, and EINA_TRUE if every
 *          thing goes fine.
 *
 * @note if you know you already have the key, use
 *       eina_hash_del_by_key() or eina_hash_del_by_key_hash(). If you
 *       know you don't have the key, use eina_hash_del_by_data()
 *       directly.
 */
EAPI Eina_Bool
eina_hash_del(Eina_Hash *hash, const void *key, const void *data)
{
   if (key) return _eina_hash_del_by_key(hash, key, data);
   else return eina_hash_del_by_data(hash, data);
}

/**
 * Retrieves a specific entry in the given hash table.
 * @param   hash The given hash table.
 * @param   key  The key of the entry to find.
 * @param   key_length Should be the length of @p key (don't forget to count '\\0' for string).
 * @param   key_hash The hash that always match the key. Ignored if @p key is @c NULL.
 * @return  The data pointer for the stored entry, or @c NULL if not
 *          found.
 */
EAPI void *
eina_hash_find_by_hash(const Eina_Hash *hash, const void *key, int key_length, int key_hash)
{
   Eina_Hash_Head *eh;
   Eina_Hash_El *el;
   Eina_Hash_Tuple tuple;

   if ((!hash) || (!key)) return NULL;
   EINA_MAGIC_CHECK_HASH(hash);

   tuple.key = key;
   tuple.key_length = key_length;
   tuple.data = NULL;

   el = _eina_hash_find_by_hash(hash, &tuple, key_hash, &eh);
   if (el) return el->tuple.data;
   return NULL;
}

/**
 * Retrieves a specific entry in the given hash table.
 * @param   hash The given hash table.
 * @param   key  The key of the entry to find.
 * @return  The data pointer for the stored entry, or @c NULL if not
 *          found.
 */
EAPI void *
eina_hash_find(const Eina_Hash *hash, const void *key)
{
   int key_length;
   int hash_num;

   if ((!hash) || (!key)) return NULL;
   EINA_MAGIC_CHECK_HASH(hash);

   key_length = hash->key_length_cb(key);
   hash_num = hash->key_hash_cb(key, key_length);

   return eina_hash_find_by_hash(hash, key, key_length, hash_num);
}

/**
 * Modifies the entry pointer at the specified key and returns the old entry
 * @param   hash The given hash table.
 * @param   key  The key of the entry to modify.
 * @param   key_length Should be the length of @p key (don't forget to count '\\0' for string).
 * @param   key_hash The hash that always match the key. Ignored if @p key is @c NULL.
 * @param   data The data to replace the old entry, if it exists.
 * @return  The data pointer for the old stored entry, or @c NULL if not
 *          found. If an existing entry is not found, nothing is added to the
 *          hash.
 */
EAPI void *
eina_hash_modify_by_hash(Eina_Hash *hash, const void *key, int key_length, int key_hash, const void *data)
{
   Eina_Hash_Head *eh;
   Eina_Hash_El *el;
   void *old_data = NULL;
   Eina_Hash_Tuple tuple;

   EINA_MAGIC_CHECK_HASH(hash);
   if (!hash) return NULL;
   tuple.key = key;
   tuple.key_length = key_length;
   tuple.data = NULL;

   el = _eina_hash_find_by_hash(hash, &tuple, key_hash, &eh);
   if (el)
     {
	old_data = el->tuple.data;
	el->tuple.data = (void *) data;
     }

   return old_data;
}

/**
 * Modifies the entry pointer at the specified key and returns the old entry
 * @param   hash The given hash table.
 * @param   key  The key of the entry to modify.
 * @param   data The data to replace the old entry, if it exists.
 * @return  The data pointer for the old stored entry, or @c NULL if not
 *          found. If an existing entry is not found, nothing is added to the
 *          hash.
 */
EAPI void *
eina_hash_modify(Eina_Hash *hash, const void *key, const void *data)
{
   int key_length;
   int hash_num;

   EINA_MAGIC_CHECK_HASH(hash);
   if (!hash) return NULL;

   key_length = hash->key_length_cb(key);
   hash_num = hash->key_hash_cb(key, key_length);

   return eina_hash_modify_by_hash(hash, key, key_length, hash_num, data);
}

/*============================================================================*
 *                                Iterator                                    *
 *============================================================================*/

/**
 * Call a function on every member stored in the hash table
 * @param hash The hash table whose members will be walked
 * @param func The function to call on each parameter
 * @param fdata The data pointer to pass to the function being called
 *
 * This function goes through every entry in the hash table @p hash and calls
 * the function @p func on each member. The function should NOT modify the
 * hash table contents if it returns 1. IF the hash table contents are
 * modified by this function or the function wishes to stop processing it must
 * return 0, otherwise return 1 to keep processing.
 *
 * Example:
 * @code
 * extern Eina_Hash *hash;
 *
 * Eina_Bool hash_fn(Eina_Hash *hash, const char *key, void *data, void *fdata)
 * {
 *   printf("Func data: %s, Hash entry: %s / %p\n", fdata, key, data);
 *   return 1;
 * }
 *
 * int main(int argc, char **argv)
 * {
 *   char *hash_fn_data;
 *
 *   hash_fn_data = strdup("Hello World");
 *   eina_hash_foreach(hash, hash_fn, hash_fn_data);
 *   free(hash_fn_data);
 * }
 * @endcode
 */
EAPI void
eina_hash_foreach(const Eina_Hash *hash,
		  Eina_Hash_Foreach func,
		  const void *fdata)
{
   Eina_Iterator *it;
   Eina_Hash_Foreach_Data foreach;

   EINA_MAGIC_CHECK_HASH(hash);
   if (!hash) return;

   foreach.cb = func;
   foreach.fdata = fdata;

   it = eina_hash_iterator_tuple_new(hash);
   if (!it) return;

   eina_iterator_foreach(it, EINA_EACH(_eina_foreach_cb), &foreach);
   eina_iterator_free(it);
}

EAPI Eina_Iterator *
eina_hash_iterator_data_new(const Eina_Hash *hash)
{
   Eina_Iterator_Hash *it;

   EINA_MAGIC_CHECK_HASH(hash);
   if (!hash) return NULL;
   if (hash->population <= 0) return NULL;

   it = calloc(1, sizeof (Eina_Iterator_Hash));
   if (!it) return NULL;

   it->hash = hash;
   it->get_content = FUNC_ITERATOR_GET_CONTENT(_eina_hash_iterator_data_get_content);

   it->iterator.next = FUNC_ITERATOR_NEXT(_eina_hash_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_eina_hash_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_eina_hash_iterator_free);

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);
   EINA_MAGIC_SET(it, EINA_MAGIC_HASH_ITERATOR);

   return &it->iterator;
}

EAPI Eina_Iterator *
eina_hash_iterator_key_new(const Eina_Hash *hash)
{
   Eina_Iterator_Hash *it;

   EINA_MAGIC_CHECK_HASH(hash);
   if (!hash) return NULL;
   if (hash->population <= 0) return NULL;

   it = calloc(1, sizeof (Eina_Iterator_Hash));
   if (!it) return NULL;

   it->hash = hash;
   it->get_content = FUNC_ITERATOR_GET_CONTENT(_eina_hash_iterator_key_get_content);

   it->iterator.next = FUNC_ITERATOR_NEXT(_eina_hash_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_eina_hash_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_eina_hash_iterator_free);

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);
   EINA_MAGIC_SET(it, EINA_MAGIC_HASH_ITERATOR);

   return &it->iterator;
}

EAPI Eina_Iterator *
eina_hash_iterator_tuple_new(const Eina_Hash *hash)
{
   Eina_Iterator_Hash *it;

   EINA_MAGIC_CHECK_HASH(hash);
   if (!hash) return NULL;
   if (hash->population <= 0) return NULL;

   it = calloc(1, sizeof (Eina_Iterator_Hash));
   if (!it) return NULL;

   it->hash = hash;
   it->get_content = FUNC_ITERATOR_GET_CONTENT(_eina_hash_iterator_tuple_get_content);

   it->iterator.next = FUNC_ITERATOR_NEXT(_eina_hash_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_eina_hash_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_eina_hash_iterator_free);

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);
   EINA_MAGIC_SET(it, EINA_MAGIC_HASH_ITERATOR);

   return &it->iterator;
}

/* Common hash functions */

/* Paul Hsieh (http://www.azillionmonkeys.com/qed/hash.html)
   used by WebCore (http://webkit.org/blog/8/hashtables-part-2/) */
EAPI int
eina_hash_superfast(const char *key, int len)
{
   int hash = len, tmp;
   int rem;

   rem = len & 3;
   len >>= 2;

   /* Main loop */
   for ( ;len > 0; len--)
     {
	hash += get16bits(key);
	tmp = (get16bits(key + 2) << 11) ^ hash;
	hash = (hash << 16) ^ tmp;
	key += 2 * sizeof (uint16_t);
	hash += hash >> 11;
     }

   /* Handle end cases */
   switch (rem)
     {
      case 3:
	 hash += get16bits(key);
	 hash ^= hash << 16;
	 hash ^= key[sizeof (uint16_t)] << 18;
	 hash += hash >> 11;
	 break;
      case 2:
	 hash += get16bits(key);
	 hash ^= hash << 11;
	 hash += hash >> 17;
	 break;
      case 1:
	 hash += *key;
	 hash ^= hash << 10;
	 hash += hash >> 1;
     }

   /* Force "avalanching" of final 127 bits */
   hash ^= hash << 3;
   hash += hash >> 5;
   hash ^= hash << 4;
   hash += hash >> 17;
   hash ^= hash << 25;
   hash += hash >> 6;

   return hash;
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

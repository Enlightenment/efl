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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif

#ifdef _MSC_VER
# include <Evil.h>
#endif

#include "eina_config.h"
#include "eina_private.h"
#include "eina_rbtree.h"
#include "eina_error.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_hash.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

#define EINA_MAGIC_CHECK_HASH(d)                     \
   do {                                              \
        if (!EINA_MAGIC_CHECK(d, EINA_MAGIC_HASH)) { \
             EINA_MAGIC_FAIL(d, EINA_MAGIC_HASH); }  \
     } while(0)

#define EINA_MAGIC_CHECK_HASH_ITERATOR(d, ...)              \
   do {                                                     \
        if (!EINA_MAGIC_CHECK(d, EINA_MAGIC_HASH_ITERATOR)) \
          {                                                 \
             EINA_MAGIC_FAIL(d, EINA_MAGIC_HASH_ITERATOR);  \
             return __VA_ARGS__;                            \
          }                                                 \
     } while(0)

#define EINA_HASH_BUCKET_SIZE 8
#define EINA_HASH_SMALL_BUCKET_SIZE 5

#define EINA_HASH_RBTREE_MASK 0xFFF

typedef struct _Eina_Hash_Head Eina_Hash_Head;
typedef struct _Eina_Hash_Element Eina_Hash_Element;
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

   EINA_MAGIC
};

struct _Eina_Hash_Head
{
   EINA_RBTREE;
   int hash;

   Eina_Rbtree *head;
};

struct _Eina_Hash_Element
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
#define FUNC_ITERATOR_GET_CONTENT(Function) \
   ((Eina_Iterator_Get_Content_Callback)Function)

struct _Eina_Iterator_Hash
{
   Eina_Iterator iterator;

   Eina_Iterator_Get_Content_Callback get_content;
   const Eina_Hash *hash;

   Eina_Iterator *current;
   Eina_Iterator *list;
   Eina_Hash_Head *hash_head;
   Eina_Hash_Element *hash_element;
   int bucket;

   int index;

   EINA_MAGIC
};

struct _Eina_Hash_Each
{
   Eina_Hash_Head *hash_head;
   const Eina_Hash_Element *hash_element;
   const void *data;
};

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
   || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
# define get16bits(d) (*((const uint16_t *)(d)))
#endif

#if !defined (get16bits)
# define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8) \
                       + (uint32_t)(((const uint8_t *)(d))[0]))
#endif

static inline int
_eina_hash_hash_rbtree_cmp_hash(const Eina_Hash_Head *hash_head,
                                const int *hash,
                                __UNUSED__ int key_length,
                                __UNUSED__ void *data)
{
   return hash_head->hash - *hash;
}

static Eina_Rbtree_Direction
_eina_hash_hash_rbtree_cmp_node(const Eina_Hash_Head *left,
                                const Eina_Hash_Head *right,
                                __UNUSED__ void *data)
{
   if (left->hash - right->hash < 0)
      return EINA_RBTREE_LEFT;

   return EINA_RBTREE_RIGHT;
}

static inline int
_eina_hash_key_rbtree_cmp_key_data(const Eina_Hash_Element *hash_element,
                                   const Eina_Hash_Tuple *tuple,
                                   __UNUSED__ unsigned int key_length,
                                   Eina_Key_Cmp cmp)
{
   int result;

   result = cmp(hash_element->tuple.key,
                hash_element->tuple.key_length,
                tuple->key,
                tuple->key_length);

   if (result == 0 && tuple->data && tuple->data != hash_element->tuple.data)
      return 1;

   return result;
}

static Eina_Rbtree_Direction
_eina_hash_key_rbtree_cmp_node(const Eina_Hash_Element *left,
                               const Eina_Hash_Element *right,
                               Eina_Key_Cmp cmp)
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
   Eina_Hash_Element *new_hash_element = NULL;
   Eina_Hash_Head *hash_head;
   Eina_Error error = 0;
   int hash_num;

   EINA_SAFETY_ON_NULL_RETURN_VAL(hash, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key,  EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, EINA_FALSE);
   EINA_MAGIC_CHECK_HASH(hash);

   error = EINA_ERROR_OUT_OF_MEMORY;

   /* Apply eina mask to hash. */
   hash_num = key_hash & hash->mask;
   key_hash &= EINA_HASH_RBTREE_MASK;

   if (!hash->buckets)
     {
        hash->buckets = calloc(sizeof (Eina_Rbtree *), hash->size);
        if (!hash->buckets) goto on_error;

        hash_head = NULL;
     }
   else
      /* Look up for head node. */
      hash_head = (Eina_Hash_Head *)
         eina_rbtree_inline_lookup(hash->buckets[hash_num],
                                   &key_hash, 0,
                                   EINA_RBTREE_CMP_KEY_CB(
                                      _eina_hash_hash_rbtree_cmp_hash),
                                   NULL);

   if (!hash_head)
     {
        /* If not found allocate it and an element. */
        hash_head = malloc(sizeof(Eina_Hash_Head) + sizeof(Eina_Hash_Element)
                           + alloc_length);
        if (!hash_head)
           goto on_error;

        hash_head->hash = key_hash;
        hash_head->head = NULL;

        hash->buckets[hash_num] =
           eina_rbtree_inline_insert(hash->buckets[hash_num],
                                     EINA_RBTREE_GET(hash_head),
                                     EINA_RBTREE_CMP_NODE_CB(
                                        _eina_hash_hash_rbtree_cmp_node),
                                     NULL);

        new_hash_element = (Eina_Hash_Element *)(hash_head + 1);
        new_hash_element->begin = EINA_TRUE;
     }

   if (!new_hash_element)
     {
        /*
           Alloc a new element
           (No more lookup as we expect to support more than one item for one key).
         */
        new_hash_element = malloc(sizeof (Eina_Hash_Element) + alloc_length);
        if (!new_hash_element)
           goto on_error;

        new_hash_element->begin = EINA_FALSE;
     }

   /* Setup the element */
   new_hash_element->tuple.key_length = key_length;
   new_hash_element->tuple.data = (void *)data;
   if (alloc_length > 0)
     {
        new_hash_element->tuple.key = (char *)(new_hash_element + 1);
        memcpy((char *)new_hash_element->tuple.key, key, alloc_length);
     }
   else
      new_hash_element->tuple.key = key;

   /* add the new element to the hash. */
   hash_head->head = eina_rbtree_inline_insert(hash_head->head,
                                            EINA_RBTREE_GET(new_hash_element),
                                            EINA_RBTREE_CMP_NODE_CB(
                                               _eina_hash_key_rbtree_cmp_node),
                                            (const void *)hash->key_cmp_cb);
   hash->population++;
   return EINA_TRUE;

on_error:
   eina_error_set(error);
   return EINA_FALSE;
}

static Eina_Bool
_eina_hash_rbtree_each(__UNUSED__ const Eina_Rbtree *container,
                       const Eina_Hash_Head *hash_head,
                       Eina_Hash_Each *data)
{
   Eina_Iterator *it;
   Eina_Hash_Element *hash_element;
   Eina_Bool found = EINA_TRUE;

   it = eina_rbtree_iterator_prefix(hash_head->head);
   EINA_ITERATOR_FOREACH(it, hash_element)
   {
      if (hash_element->tuple.data == data->data)
        {
           data->hash_element = hash_element;
           data->hash_head = (Eina_Hash_Head *)hash_head;
           found = EINA_FALSE;
           break;
        }
   }

   eina_iterator_free(it);
   return found;
}

static inline Eina_Hash_Element *
_eina_hash_find_by_hash(const Eina_Hash *hash,
                        Eina_Hash_Tuple *tuple,
                        int key_hash,
                        Eina_Hash_Head **hash_head)
{
   Eina_Hash_Element *hash_element;
   int rb_hash = key_hash & EINA_HASH_RBTREE_MASK;

   key_hash &= hash->mask;

   if (!hash->buckets)
      return NULL;

   *hash_head = (Eina_Hash_Head *)
      eina_rbtree_inline_lookup(hash->buckets[key_hash],
                                &rb_hash, 0,
                                EINA_RBTREE_CMP_KEY_CB(
                                   _eina_hash_hash_rbtree_cmp_hash),
                                NULL);
   if (!*hash_head)
      return NULL;

   hash_element = (Eina_Hash_Element *)
      eina_rbtree_inline_lookup((*hash_head)->head,
                                tuple, 0,
                                   EINA_RBTREE_CMP_KEY_CB(
                                   _eina_hash_key_rbtree_cmp_key_data),
                                (const void *)hash->key_cmp_cb);

   return hash_element;
}

static inline Eina_Hash_Element *
_eina_hash_find_by_data(const Eina_Hash *hash,
                        const void *data,
                        int *key_hash,
                        Eina_Hash_Head **hash_head)
{
   Eina_Hash_Each each;
   Eina_Iterator *it;
   int hash_num;

   if (!hash->buckets)
      return NULL;

   each.hash_element = NULL;
   each.data = data;

   for (hash_num = 0; hash_num < hash->size; hash_num++)
     {
        if (!hash->buckets[hash_num])
           continue;

        it = eina_rbtree_iterator_prefix(hash->buckets[hash_num]);
        eina_iterator_foreach(it, EINA_EACH_CB(_eina_hash_rbtree_each), &each);
        eina_iterator_free(it);

        if (each.hash_element)
          {
             *key_hash = hash_num;
             *hash_head = each.hash_head;
             return (Eina_Hash_Element *)each.hash_element;
          }
     }

   return NULL;
}

static void
_eina_hash_el_free(Eina_Hash_Element *hash_element, Eina_Hash *hash)
{
   if (hash->data_free_cb)
      hash->data_free_cb(hash_element->tuple.data);

   if (hash_element->begin == EINA_FALSE)
      free(hash_element);
}

static void
_eina_hash_head_free(Eina_Hash_Head *hash_head, Eina_Hash *hash)
{
   eina_rbtree_delete(hash_head->head, EINA_RBTREE_FREE_CB(_eina_hash_el_free), hash);
   free(hash_head);
}

static Eina_Bool
_eina_hash_del_by_hash_el(Eina_Hash *hash,
                          Eina_Hash_Element *hash_element,
                          Eina_Hash_Head *hash_head,
                          int key_hash)
{
   hash_head->head = eina_rbtree_inline_remove(hash_head->head, EINA_RBTREE_GET(
                                           hash_element), EINA_RBTREE_CMP_NODE_CB(
                                           _eina_hash_key_rbtree_cmp_node),
                                        (const void *)hash->key_cmp_cb);
   _eina_hash_el_free(hash_element, hash);

   if (!hash_head->head)
     {
        key_hash &= hash->mask;

        hash->buckets[key_hash] =
           eina_rbtree_inline_remove(hash->buckets[key_hash], EINA_RBTREE_GET(
                                        hash_head),
                                     EINA_RBTREE_CMP_NODE_CB(
                                        _eina_hash_hash_rbtree_cmp_node), NULL);
        free(hash_head);
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
_eina_hash_del_by_key_hash(Eina_Hash *hash,
                           const void *key,
                           int key_length,
                           int key_hash,
                           const void *data)
{
   Eina_Hash_Element *hash_element;
   Eina_Hash_Head *hash_head;
   Eina_Hash_Tuple tuple;

   EINA_SAFETY_ON_NULL_RETURN_VAL(hash, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key,  EINA_FALSE);
   EINA_MAGIC_CHECK_HASH(hash);

   if (!hash->buckets)
      return EINA_FALSE;

   tuple.key = (void *)key;
   tuple.key_length = key_length;
   tuple.data = (void *)data;

   hash_element = _eina_hash_find_by_hash(hash, &tuple, key_hash, &hash_head);
   if (!hash_element)
      return EINA_FALSE;

   return _eina_hash_del_by_hash_el(hash, hash_element, hash_head, key_hash);
}

static Eina_Bool
_eina_hash_del_by_key(Eina_Hash *hash, const void *key, const void *data)
{
   int key_length, key_hash;

   EINA_MAGIC_CHECK_HASH(hash);
   if (!hash)
      return EINA_FALSE;

   if (!key)
      return EINA_FALSE;

   if (!hash->buckets)
      return EINA_FALSE;

   key_length = hash->key_length_cb ? hash->key_length_cb(key) : 0;
   key_hash = hash->key_hash_cb(key, key_length);
   return _eina_hash_del_by_key_hash(hash, key, key_length, key_hash, data);
}

static unsigned int
_eina_string_key_length(const char *key)
{
   if (!key)
      return 0;

   return (int)strlen(key) + 1;
}

static int
_eina_string_key_cmp(const char *key1, __UNUSED__ int key1_length,
                     const char *key2, __UNUSED__ int key2_length)
{
   return strcmp(key1, key2);
}

static int
_eina_stringshared_key_cmp(const char *key1, __UNUSED__ int key1_length,
                           const char *key2, __UNUSED__ int key2_length)
{
   return key1 - key2;
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
   return *key1 - *key2;
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
   return *key1 - *key2;
}

static Eina_Bool
_eina_foreach_cb(const Eina_Hash *hash,
                 Eina_Hash_Tuple *data,
                 Eina_Hash_Foreach_Data *fdata)
{
   return fdata->cb((Eina_Hash *)hash,
                    data->key,
                    data->data,
                    (void *)fdata->fdata);
}

static void *
_eina_hash_iterator_data_get_content(Eina_Iterator_Hash *it)
{
   Eina_Hash_Element *stuff;

   EINA_MAGIC_CHECK_HASH_ITERATOR(it, NULL);

   stuff = it->hash_element;

   if (!stuff)
      return NULL;

   return stuff->tuple.data;
}

static void *
_eina_hash_iterator_key_get_content(Eina_Iterator_Hash *it)
{
   Eina_Hash_Element *stuff;

   EINA_MAGIC_CHECK_HASH_ITERATOR(it, NULL);

   stuff = it->hash_element;

   if (!stuff)
      return NULL;

   return (void *)stuff->tuple.key;
}

static Eina_Hash_Tuple *
_eina_hash_iterator_tuple_get_content(Eina_Iterator_Hash *it)
{
   Eina_Hash_Element *stuff;

   EINA_MAGIC_CHECK_HASH_ITERATOR(it, NULL);

   stuff = it->hash_element;

   if (!stuff)
      return NULL;

   return &stuff->tuple;
}

static Eina_Bool
_eina_hash_iterator_next(Eina_Iterator_Hash *it, void **data)
{
   Eina_Bool ok;
   int bucket;

   if (!(it->index < it->hash->population))
      return EINA_FALSE;

   if (!it->current)
     {
        ok = EINA_FALSE;
        bucket = 0;
        it->index = -1;
     }
   else
     {
        ok = eina_iterator_next(it->list, (void **)(void*)&it->hash_element);
        if (!ok)
          {
                  eina_iterator_free(it->list);
             it->list = NULL;

             ok = eina_iterator_next(it->current, (void **)(void*)&it->hash_head);
             if (!ok)
               {
                  eina_iterator_free(it->current);
                  it->current = NULL;
                  it->bucket++;
               }
             else
               {
                  it->list = eina_rbtree_iterator_prefix(it->hash_head->head);
                  ok = eina_iterator_next(it->list, (void **)(void*)&it->hash_element);
               }
          }

        bucket = it->bucket;
     }

   if (ok == EINA_FALSE)
     {
        while (bucket < it->hash->size)
          {
             if (it->hash->buckets[bucket])
               {
                  it->current =
                     eina_rbtree_iterator_prefix(it->hash->buckets[bucket]);
                  ok = eina_iterator_next(it->current, (void **)(void*)&it->hash_head);
                  if (ok)
                     break;

                  eina_iterator_free(it->current);
                  it->current = NULL;
               }

             ++bucket;
          }
        if (it->list)
                  eina_iterator_free(it->list);

        it->list = eina_rbtree_iterator_prefix(it->hash_head->head);
        ok = eina_iterator_next(it->list, (void **)(void*)&it->hash_element);
        if (bucket == it->hash->size)
           ok = EINA_FALSE;
     }

   it->index++;
   it->bucket = bucket;

   if (ok)
      *data = it->get_content(it);

   return ok;
}

static void *
_eina_hash_iterator_get_container(Eina_Iterator_Hash *it)
{
      EINA_MAGIC_CHECK_HASH_ITERATOR(it, NULL);
   return (void *)it->hash;
}

static void
_eina_hash_iterator_free(Eina_Iterator_Hash *it)
{
      EINA_MAGIC_CHECK_HASH_ITERATOR(it);
   if (it->current)
      eina_iterator_free(it->current);

   if (it->list)
      eina_iterator_free(it->list);

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

EAPI void
eina_hash_free_set(Eina_Hash *hash, Eina_Free_Cb data_free_cb)
{
   EINA_MAGIC_CHECK_HASH(hash);
   EINA_SAFETY_ON_NULL_RETURN(hash);

   hash->data_free_cb = data_free_cb;
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
   EINA_SAFETY_ON_NULL_RETURN_VAL(key_cmp_cb,  NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key_hash_cb, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(buckets_power_size <= 2,  NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(buckets_power_size >= 17, NULL);

   new = malloc(sizeof (Eina_Hash));
   if (!new)
      goto on_error;

   EINA_MAGIC_SET(new, EINA_MAGIC_HASH);

   new->key_length_cb = key_length_cb;
   new->key_cmp_cb = key_cmp_cb;
   new->key_hash_cb = key_hash_cb;
   new->data_free_cb = data_free_cb;
   new->buckets = NULL;
   new->population = 0;

   new->size = 1 << buckets_power_size;
   new->mask = new->size - 1;

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

EAPI Eina_Hash *
eina_hash_stringshared_new(Eina_Free_Cb data_free_cb)
{
   return eina_hash_new(NULL,
                        EINA_KEY_CMP(_eina_stringshared_key_cmp),
                        EINA_KEY_HASH(eina_hash_superfast),
                        data_free_cb,
                        EINA_HASH_BUCKET_SIZE);
}

EAPI int
eina_hash_population(const Eina_Hash *hash)
{
   if (!hash)
      return 0;

   EINA_MAGIC_CHECK_HASH(hash);
   return hash->population;
}

EAPI void
eina_hash_free(Eina_Hash *hash)
{
   int i;

   EINA_MAGIC_CHECK_HASH(hash);
   EINA_SAFETY_ON_NULL_RETURN(hash);

   if (hash->buckets)
     {
        for (i = 0; i < hash->size; i++)
           eina_rbtree_delete(hash->buckets[i], EINA_RBTREE_FREE_CB(_eina_hash_head_free), hash);
        free(hash->buckets);
     }
   free(hash);
}

EAPI void
eina_hash_free_buckets(Eina_Hash *hash)
{
   int i;

   EINA_MAGIC_CHECK_HASH(hash);
   EINA_SAFETY_ON_NULL_RETURN(hash);

   if (hash->buckets)
     {
        for (i = 0; i < hash->size; i++)
           eina_rbtree_delete(hash->buckets[i],
                              EINA_RBTREE_FREE_CB(_eina_hash_head_free), hash);
        free(hash->buckets);
        hash->buckets = NULL;
        hash->population = 0;
     }
}

EAPI Eina_Bool
eina_hash_add_by_hash(Eina_Hash  *hash,
                      const void *key,
                      int         key_length,
                      int         key_hash,
                      const void *data)
{
   return eina_hash_add_alloc_by_hash(hash,
                                      key,
                                      key_length,
                                      key_length,
                                      key_hash,
                                      data);
}

EAPI Eina_Bool
eina_hash_direct_add_by_hash(Eina_Hash  *hash,
                             const void *key,
                             int         key_length,
                             int         key_hash,
                             const void *data)
{
   return eina_hash_add_alloc_by_hash(hash, key, key_length, 0, key_hash, data);
}

EAPI Eina_Bool
eina_hash_add(Eina_Hash *hash, const void *key, const void *data)
{
   unsigned int key_length;
   int key_hash;

   EINA_MAGIC_CHECK_HASH(hash);
   EINA_SAFETY_ON_NULL_RETURN_VAL(hash,              EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(hash->key_hash_cb, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key,               EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(data,              EINA_FALSE);

   key_length = hash->key_length_cb ? hash->key_length_cb(key) : 0;
   key_hash = hash->key_hash_cb(key, key_length);

   return eina_hash_add_alloc_by_hash(hash, key, key_length, key_length, key_hash, data);
}

EAPI Eina_Bool
eina_hash_direct_add(Eina_Hash *hash, const void *key, const void *data)
{
   int key_length;
   int key_hash;

   EINA_SAFETY_ON_NULL_RETURN_VAL(hash,              EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(hash->key_hash_cb, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key,               EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(data,              EINA_FALSE);
   EINA_MAGIC_CHECK_HASH(hash);

   key_length = hash->key_length_cb ? hash->key_length_cb(key) : 0;
   key_hash = hash->key_hash_cb(key, key_length);

   return eina_hash_add_alloc_by_hash(hash, key, key_length, 0, key_hash, data);
}

EAPI Eina_Bool
eina_hash_del_by_key_hash(Eina_Hash *hash,
                          const void *key,
                          int key_length,
                          int key_hash)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(hash, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key,  EINA_FALSE);

   return _eina_hash_del_by_key_hash(hash, key, key_length, key_hash, NULL);
}

EAPI Eina_Bool
eina_hash_del_by_key(Eina_Hash *hash, const void *key)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(hash, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key,  EINA_FALSE);

   return _eina_hash_del_by_key(hash, key, NULL);
}

EAPI Eina_Bool
eina_hash_del_by_data(Eina_Hash *hash, const void *data)
{
   Eina_Hash_Element *hash_element;
   Eina_Hash_Head *hash_head;
   int key_hash;

   EINA_SAFETY_ON_NULL_RETURN_VAL(hash, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, EINA_FALSE);
   EINA_MAGIC_CHECK_HASH(hash);

   hash_element = _eina_hash_find_by_data(hash, data, &key_hash, &hash_head);
   if (!hash_element)
      goto error;

   if (hash_element->tuple.data != data)
      goto error;

   return _eina_hash_del_by_hash_el(hash, hash_element, hash_head, key_hash);

error:
   return EINA_FALSE;
}

EAPI Eina_Bool
eina_hash_del_by_hash(Eina_Hash *hash,
                      const void *key,
                      int key_length,
                      int key_hash,
                      const void *data)
{
   Eina_Bool ret;

   EINA_SAFETY_ON_NULL_RETURN_VAL(hash, EINA_FALSE);
   EINA_MAGIC_CHECK_HASH(hash);

   if (key)
      ret = _eina_hash_del_by_key_hash(hash, key, key_length, key_hash, data);
   else
      ret = eina_hash_del_by_data(hash, data);

   return ret;
}

EAPI Eina_Bool
eina_hash_del(Eina_Hash *hash, const void *key, const void *data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(hash, EINA_FALSE);
   EINA_MAGIC_CHECK_HASH(hash);

   if (!key)
     return eina_hash_del_by_data(hash, data);

   return _eina_hash_del_by_key(hash, key, data);
}

EAPI void *
eina_hash_find_by_hash(const Eina_Hash *hash,
                       const void *key,
                       int key_length,
                       int key_hash)
{
   Eina_Hash_Head *hash_head;
   Eina_Hash_Element *hash_element;
   Eina_Hash_Tuple tuple;

   if (!hash)
      return NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(key, NULL);
   EINA_MAGIC_CHECK_HASH(hash);

   tuple.key = key;
   tuple.key_length = key_length;
   tuple.data = NULL;

   hash_element = _eina_hash_find_by_hash(hash, &tuple, key_hash, &hash_head);
   if (hash_element)
      return hash_element->tuple.data;

   return NULL;
}

EAPI void *
eina_hash_find(const Eina_Hash *hash, const void *key)
{
   int key_length;
   int hash_num;

   if (!hash)
      return NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(hash->key_hash_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key,               NULL);
   EINA_MAGIC_CHECK_HASH(hash);

   key_length = hash->key_length_cb ? hash->key_length_cb(key) : 0;
   hash_num = hash->key_hash_cb(key, key_length);

   return eina_hash_find_by_hash(hash, key, key_length, hash_num);
}

EAPI void *
eina_hash_modify_by_hash(Eina_Hash *hash,
                         const void *key,
                         int key_length,
                         int key_hash,
                         const void *data)
{
   Eina_Hash_Head *hash_head;
   Eina_Hash_Element *hash_element;
   void *old_data = NULL;
   Eina_Hash_Tuple tuple;

   EINA_SAFETY_ON_NULL_RETURN_VAL(hash, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key,  NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
   EINA_MAGIC_CHECK_HASH(hash);

   tuple.key = key;
   tuple.key_length = key_length;
   tuple.data = NULL;

   hash_element = _eina_hash_find_by_hash(hash, &tuple, key_hash, &hash_head);
   if (hash_element)
     {
        old_data = hash_element->tuple.data;
        hash_element->tuple.data = (void *)data;
     }

   return old_data;
}

EAPI void *
eina_hash_set(Eina_Hash *hash, const void *key, const void *data)
{
   Eina_Hash_Tuple tuple;
   Eina_Hash_Head *hash_head;
   Eina_Hash_Element *hash_element;
   int key_length;
   int key_hash;

   EINA_SAFETY_ON_NULL_RETURN_VAL(hash,              NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(hash->key_hash_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key,               NULL);
   EINA_MAGIC_CHECK_HASH(hash);

   key_length = hash->key_length_cb ? hash->key_length_cb(key) : 0;
   key_hash = hash->key_hash_cb(key, key_length);

   tuple.key = key;
   tuple.key_length = key_length;
   tuple.data = NULL;

   hash_element = _eina_hash_find_by_hash(hash, &tuple, key_hash, &hash_head);
   if (hash_element)
     {
        void *old_data = NULL;

        old_data = hash_element->tuple.data;

	if (data)
	  {
	    hash_element->tuple.data = (void *)data;
	  }
	else
	  {
	    _eina_hash_del_by_hash_el(hash, hash_element, hash_head, key_hash);
	  }

        return old_data;
     }

   if (!data) return NULL;

   eina_hash_add_alloc_by_hash(hash,
                               key,
                               key_length,
                               key_length,
                               key_hash,
                               data);
   return NULL;
}
EAPI void *
eina_hash_modify(Eina_Hash *hash, const void *key, const void *data)
{
   int key_length;
   int hash_num;

   EINA_SAFETY_ON_NULL_RETURN_VAL(hash,              NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(hash->key_hash_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key,               NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(data,              NULL);
   EINA_MAGIC_CHECK_HASH(hash);

   key_length = hash->key_length_cb ? hash->key_length_cb(key) : 0;
   hash_num = hash->key_hash_cb(key, key_length);

   return eina_hash_modify_by_hash(hash, key, key_length, hash_num, data);
}

EAPI Eina_Bool
eina_hash_move(Eina_Hash *hash, const void *old_key, const void *new_key)
{
   Eina_Free_Cb hash_free_cb;
   const void *data;
   Eina_Bool result = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN_VAL(hash,              EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(hash->key_hash_cb, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(old_key,           EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(new_key,           EINA_FALSE);
   EINA_MAGIC_CHECK_HASH(hash);

   data = eina_hash_find(hash, old_key);
   if (!data) goto error;

   hash_free_cb = hash->data_free_cb;
   hash->data_free_cb = NULL;

   eina_hash_del(hash, old_key, data);
   result = eina_hash_add(hash, new_key, data);

   hash->data_free_cb = hash_free_cb;

error:
   return result;
}

/*============================================================================*
*                                Iterator                                    *
*============================================================================*/

EAPI void
eina_hash_foreach(const Eina_Hash *hash,
                  Eina_Hash_Foreach func,
                  const void *fdata)
{
   Eina_Iterator *it;
   Eina_Hash_Foreach_Data foreach;

   EINA_MAGIC_CHECK_HASH(hash);
   EINA_SAFETY_ON_NULL_RETURN(hash);
   EINA_SAFETY_ON_NULL_RETURN(func);

   foreach.cb = func;
   foreach.fdata = fdata;

   it = eina_hash_iterator_tuple_new(hash);
   if (!it)
      return;
   eina_iterator_foreach(it, EINA_EACH_CB(_eina_foreach_cb), &foreach);

   eina_iterator_free(it);
}

EAPI Eina_Iterator *
eina_hash_iterator_data_new(const Eina_Hash *hash)
{
   Eina_Iterator_Hash *it;

   EINA_SAFETY_ON_NULL_RETURN_VAL(hash, NULL);
   EINA_MAGIC_CHECK_HASH(hash);

   eina_error_set(0);
   it = calloc(1, sizeof (Eina_Iterator_Hash));
   if (!it)
     {
        eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
        return NULL;
     }

   it->hash = hash;
   it->get_content = FUNC_ITERATOR_GET_CONTENT(_eina_hash_iterator_data_get_content);

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_eina_hash_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(
         _eina_hash_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_eina_hash_iterator_free);

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);
   EINA_MAGIC_SET(it,            EINA_MAGIC_HASH_ITERATOR);

   return &it->iterator;
}

EAPI Eina_Iterator *
eina_hash_iterator_key_new(const Eina_Hash *hash)
{
   Eina_Iterator_Hash *it;

   EINA_SAFETY_ON_NULL_RETURN_VAL(hash, NULL);
   EINA_MAGIC_CHECK_HASH(hash);

   eina_error_set(0);
   it = calloc(1, sizeof (Eina_Iterator_Hash));
   if (!it)
     {
        eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
        return NULL;
     }

   it->hash = hash;
   it->get_content = FUNC_ITERATOR_GET_CONTENT(
         _eina_hash_iterator_key_get_content);

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_eina_hash_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(
         _eina_hash_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_eina_hash_iterator_free);

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);
   EINA_MAGIC_SET(it,            EINA_MAGIC_HASH_ITERATOR);

   return &it->iterator;
}

EAPI Eina_Iterator *
eina_hash_iterator_tuple_new(const Eina_Hash *hash)
{
   Eina_Iterator_Hash *it;

   EINA_SAFETY_ON_NULL_RETURN_VAL(hash, NULL);
   EINA_MAGIC_CHECK_HASH(hash);

   eina_error_set(0);
   it = calloc(1, sizeof (Eina_Iterator_Hash));
   if (!it)
     {
        eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
        return NULL;
     }

   it->hash = hash;
   it->get_content = FUNC_ITERATOR_GET_CONTENT(
         _eina_hash_iterator_tuple_get_content);

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_eina_hash_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(
         _eina_hash_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_eina_hash_iterator_free);

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);
   EINA_MAGIC_SET(it,            EINA_MAGIC_HASH_ITERATOR);

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
   for (; len > 0; len--)
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

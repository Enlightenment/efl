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

#include <stdint.h>

#include "eina_hash.h"
#include "eina_inlist.h"
#include "eina_error.h"
#include "eina_private.h"

/*============================================================================*
 *                                  Local                                     * 
 *============================================================================*/
typedef struct _Eina_Hash_El Eina_Hash_El;
struct _Eina_Hash
{
   Eina_Key_Length key_length_cb;
   Eina_Key_Cmp key_cmp_cb;
   Eina_Key_Hash key_hash_cb;

   Eina_Inlist *buckets[256];
   int population;
};

struct _Eina_Hash_El
{
   Eina_Inlist _list_data;

   void *data;
   const void *key;
   unsigned int length;
};

static int _eina_hash_init_count = 0;

static inline Eina_Hash_El *
_eina_hash_find_by_hash(const Eina_Hash *hash, const char *key, int key_length, int key_hash)
{
   Eina_Hash_El *el;

   key_hash &= 0xFF;

   EINA_INLIST_ITER_NEXT(hash->buckets[key_hash], el)
     if (!hash->key_cmp_cb(el->key, el->length, key, key_length)) return el;

   return NULL;
}

static inline Eina_Hash_El *
_eina_hash_find_by_data(const Eina_Hash *hash, const void *data, int *key_hash)
{
   Eina_Hash_El *el;
   int hash_num;

   /* FIXME: Use an iterator for this stuff */
   for (hash_num = 0; hash_num < 256; hash_num++)
     EINA_INLIST_ITER_NEXT(hash->buckets[hash_num], el)
       if (el->data == data)
	 {
	    *key_hash = hash_num;
	    return el;
	 }

   return NULL;
}

static inline void
_eina_hash_reorder(Eina_Hash *hash, Eina_Hash_El *el, int key_hash)
{
   key_hash &= 0xFF;

   if ((void*) el != (void*) hash->buckets[key_hash])
     {
	hash->buckets[key_hash] = eina_inlist_remove(hash->buckets[key_hash], el);
	hash->buckets[key_hash] = eina_inlist_prepend(hash->buckets[key_hash], el);
     }
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

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/
/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
/**
 * Initialize the eina hash internal structure.
 * @return  Zero on failure, non-zero on successful initialization.
 */
EAPI int
eina_hash_init(void)
{
   _eina_hash_init_count++;

   if (_eina_hash_init_count == 1)
     eina_error_init();

   return _eina_hash_init_count;
}

/**
 * Shutdown the eina hash internal structures
 */
EAPI int
eina_hash_shutdown(void)
{
   _eina_hash_init_count--;

   if (_eina_hash_init_count == 0) eina_error_shutdown();

   return _eina_hash_init_count;
}

EAPI Eina_Hash *
eina_hash_new(Eina_Key_Length key_length_cb,
	      Eina_Key_Cmp key_cmp_cb,
	      Eina_Key_Hash key_hash_cb)
{
   /* FIXME: Use mempool. */
   Eina_Hash *new;

   eina_error_set(0);
   if (!key_length_cb || !key_cmp_cb) return NULL;

   new = calloc(1, sizeof (Eina_Hash));
   if (!new) goto on_error;

   new->key_length_cb = key_length_cb;
   new->key_cmp_cb = key_cmp_cb;
   new->key_hash_cb = key_hash_cb;

   return new;

 on_error:
   eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
   return NULL;
}

EAPI Eina_Hash *
eina_hash_string_djb2_new(void)
{
   return eina_hash_new(EINA_KEY_LENGTH(_eina_string_key_length),
			EINA_KEY_CMP(_eina_string_key_cmp),
			EINA_KEY_HASH(eina_hash_djb2));
}

EAPI Eina_Hash *
eina_hash_string_superfast_new(void)
{
   return eina_hash_new(EINA_KEY_LENGTH(_eina_string_key_length),
			EINA_KEY_CMP(_eina_string_key_cmp),
			EINA_KEY_HASH(eina_hash_superfast));
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
 * @ingroup Eina_Hash_Data
 */
EAPI Eina_Bool
eina_hash_add_by_hash(Eina_Hash *hash,
		      const void *key, int key_length, int key_hash,
		      const void *data)
{
   Eina_Hash_El *el;

   eina_error_set(0);
   if ((!hash) || (!key) || (!data)) return EINA_FALSE;

   /* Alloc every needed thing.*/
   el = malloc(sizeof (Eina_Hash_El) + key_length);
   if (!el) goto on_error;

   /* Setup the element */
   el->length = key_length;
   el->data = (void *) data;
   el->key = (char *) (el + 1);
   memcpy((char *) el->key, key, key_length);

   /* eina hash have 256 buckets. */
   key_hash &= 0xFF;

   /* add the new element to the hash. */
   hash->buckets[key_hash] = eina_inlist_prepend(hash->buckets[key_hash], el);
   hash->population++;
   return EINA_TRUE;

 on_error:
   eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
   return EINA_FALSE;
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
 * @ingroup Eina_Hash_Data
 */
EAPI Eina_Bool
eina_hash_direct_add_by_hash(Eina_Hash *hash,
			     const void *key, int key_length, int key_hash,
			     const void *data)
{
   Eina_Hash_El *el;

   eina_error_set(0);
   if ((!hash) || (!key) || (!data)) return EINA_FALSE;

   /* Alloc every needed thing.*/
   el = malloc(sizeof (Eina_Hash_El));
   if (!el) goto on_error;

   /* Setup the element */
   el->length = key_length;
   el->data = (void *) data;
   el->key = key;

   /* eina hash have 256 buckets. */
   key_hash &= 0xFF;

   /* add the new element to the hash. */
   hash->buckets[key_hash] = eina_inlist_prepend(hash->buckets[key_hash], el);
   hash->population++;
   return EINA_TRUE;

 on_error:
   eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
   return EINA_FALSE;
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
 * @ingroup Eina_Hash_Data
 */
EAPI Eina_Bool
eina_hash_add(Eina_Hash *hash, const void *key, const void *data)
{
   unsigned int key_length;
   int key_hash;

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
 * @ingroup Eina_Hash_Data
 */
EAPI Eina_Bool
eina_hash_direct_add(Eina_Hash *hash, const void *key, const void *data)
{
   int key_length;
   int key_hash;

   if ((!hash) || (!key) || (!data)) return EINA_FALSE;

   key_length = hash->key_length_cb(key);
   key_hash = hash->key_hash_cb(key, key_length);

   return eina_hash_direct_add_by_hash(hash, key, key_length, key_hash, data);
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
 * @ingroup Eina_Hash_Data
 */
EAPI Eina_Bool
eina_hash_del_by_hash(Eina_Hash *hash, const void *key, int key_length, int key_hash, const void *data)
{
   Eina_Hash_El *el = NULL;

   if (!hash) return EINA_FALSE;
   if (!key) el = _eina_hash_find_by_data(hash, data, &key_hash);
   else el = _eina_hash_find_by_hash(hash, key, key_length, key_hash);

   if (!el) return EINA_FALSE;

   key_hash &= 0xFF;

   hash->buckets[key_hash] = eina_inlist_remove(hash->buckets[key_hash], el);
   free(el);
   hash->population--;

   return EINA_TRUE;
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
 * @ingroup Eina_Hash_Data
 */
EAPI Eina_Bool
eina_hash_del(Eina_Hash *hash, const void *key, const void *data)
{
   int key_length = 0;
   int hash_num;

   if (!hash) return EINA_FALSE;
   if (key)
     {
	key_length = hash->key_length_cb(key);
	hash_num = hash->key_hash_cb(key, key_length);
     }

   return eina_hash_del_by_hash(hash, key, key_length, hash_num, data);
}

/**
 * Retrieves a specific entry in the given hash table.
 * @param   hash The given hash table.
 * @param   key  The key of the entry to find.
 * @param   key_length Should be the length of @p key (don't forget to count '\\0' for string).
 * @param   key_hash The hash that always match the key. Ignored if @p key is @c NULL.
 * @return  The data pointer for the stored entry, or @c NULL if not
 *          found.
 * @ingroup Eina_Hash_Data
 */
EAPI void *
eina_hash_find_by_hash(const Eina_Hash *hash, const void *key, int key_length, int key_hash)
{
   Eina_Hash_El *el;

   if ((!hash) || (!key)) return NULL;

   el = _eina_hash_find_by_hash(hash, key, key_length, key_hash);
   if (el)
     {
	_eina_hash_reorder((Eina_Hash *) hash, el, key_hash);
	return el->data;
     }
   return NULL;
}

/**
 * Retrieves a specific entry in the given hash table.
 * @param   hash The given hash table.
 * @param   key  The key of the entry to find.
 * @return  The data pointer for the stored entry, or @c NULL if not
 *          found.
 * @ingroup Eina_Hash_Data
 */
EAPI void *
eina_hash_find(const Eina_Hash *hash, const void *key)
{
   int key_length;
   int hash_num;

   if ((!hash) || (!key)) return NULL;

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
 * @ingroup Eina_Hash_Data
 */
EAPI void *
eina_hash_modify_by_hash(Eina_Hash *hash, const void *key, int key_length, int key_hash, const void *data)
{
   Eina_Hash_El *el;
   void *old_data = NULL;

   if (!hash) return NULL;

   el = _eina_hash_find_by_hash(hash, key, key_length, key_hash);
   if (el)
     {
	_eina_hash_reorder((Eina_Hash *) hash, el, key_hash);
	old_data = el->data;
	el->data = (void *) data;
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
 * @ingroup Eina_Hash_Data
 */
EAPI void *
eina_hash_modify(Eina_Hash *hash, const void *key, const void *data)
{
   int key_length;
   int hash_num;

   if (!hash) return NULL;

   key_length = hash->key_length_cb(key);
   hash_num = hash->key_hash_cb(key, key_length);

   return eina_hash_modify_by_hash(hash, key, key_length, hash_num, data);
}

/**
 * @defgroup Eina_Hash_General_Group Hash General Functions
 *
 * Miscellaneous functions that operate on hash objects.
 */

/**
 * Retrieves the number of buckets available in the given hash table.
 * @param hash The given hash table.
 * @return @c 256 if @p hash is not @c NULL.  @c 0 otherwise.
 * @ingroup Eina_Hash_General_Group
 */
EAPI int
eina_hash_population(const Eina_Hash *hash)
{
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
 * @ingroup Eina_Hash_General_Group
 */
EAPI void
eina_hash_free(Eina_Hash *hash)
{
   int i;

   if (!hash) return;

   /* FIXME: Should have used an iterator. */
   for (i = 0; i < 256; i++)
     {
	while (hash->buckets[i])
	  {
	     Eina_Hash_El *el;

	     el = (Eina_Hash_El *)hash->buckets[i];
	     hash->buckets[i] = eina_inlist_remove(hash->buckets[i], el);
	     free(el);
	  }
     }
   free(hash);
}

/* FIXME: Create a generic foreach function in the iterator implementation. */
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
 * @ingroup Eina_Hash_General_Group
 */
EAPI void eina_hash_foreach(
		const Eina_Hash *hash,
		Eina_Foreach func,
		const void *fdata)
{
   int i;

   if (!hash) return;
   for (i = 0; i < 256; i++)
     {
	Eina_Inlist *l, *next_l;

	for (l = hash->buckets[i]; l;)
	  {
	     Eina_Hash_El *el;

	     next_l = l->next;
	     el = (Eina_Hash_El *)l;
	     if (!func(hash, el->key, el->data, (void *)fdata))
	       return;
	     l = next_l;
	  }
     }
}

/* Common hash functions */

/* Paul Hsieh (http://www.azillionmonkeys.com/qed/hash.html)
   used by WebCore (http://webkit.org/blog/8/hashtables-part-2/) */
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

int
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


#include "Eina.h"
#include "eina_private.h"
/*============================================================================*
 *                                  Local                                     * 
 *============================================================================*/
typedef struct _Eina_Hash_El Eina_Hash_El;

struct _Eina_Hash
{
   int population;
   Eina_Inlist *buckets[256];
};

struct _Eina_Hash_El
{
   Eina_Inlist _list_data;
   const char *key;
   void *data;
};

static inline int _eina_hash_gen(const char *key);

static int _eina_hash_alloc_error = 0;

static inline int _eina_hash_gen(const char *key)
{
	unsigned int hash_num = 5381;
	const unsigned char *ptr;

	if (!key)
		return 0;
	for (ptr = (unsigned char *)key; *ptr; ptr++)
		hash_num = (hash_num * 33) ^ *ptr;

	hash_num &= 0xff;
	return (int)hash_num;
}
/*============================================================================*
 *                                 Global                                     * 
 *============================================================================*/
/*============================================================================*
 *                                   API                                      * 
 *============================================================================*/
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
 * @ref eina_hash_alloc_error should be used to determine if an
 * allocation error occurred during this function.
 *
 * @param   hash The given hash table.  Can be @c NULL, in which case a
 *               new hash table is allocated and returned.
 * @param   key  A unique string.  Can be @c NULL.
 * @param   data Data to associate with the string given by @p key.
 * @return  Either the given hash table, or if the given value for @p
 *          hash is @c NULL, then a new one.  @c NULL will be returned
 *          if memory could not be allocated for a new table.
 */
EAPI Eina_Hash *
eina_hash_add(Eina_Hash *hash, const char *key, const void *data)
{
	int hash_num;
	Eina_Hash_El *el;

	if ((!key) || (!data)) return hash;
	_eina_hash_alloc_error = 0;
	if (!hash)
	{
		hash = calloc(1, sizeof(struct _Eina_Hash));
		if (!hash)
		{
			_eina_hash_alloc_error = 1;
			return NULL;
		}
	}
	if (!(el = malloc(sizeof(struct _Eina_Hash_El) + strlen(key) + 1)))
	{
		if (hash->population <= 0)
		{
			free(hash);
			hash = NULL;
		}
		_eina_hash_alloc_error = 1;
		return hash;
	};
	el->key = ((char *)el) + sizeof(struct _Eina_Hash_El);
	strcpy((char *) el->key, key);
	el->data = (void *)data;
	hash_num = _eina_hash_gen(key);
	hash->buckets[hash_num] = eina_inlist_prepend(hash->buckets[hash_num], el);
	if (eina_list_alloc_error())
	{
		_eina_hash_alloc_error = 1;
		free(el);
		return hash;
	}
	hash->population++;
	return hash;
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
 * @ref eina_hash_alloc_error should be used to determine if an
 * allocation error occurred during this function.
 *
 * @param   hash The given hash table.  Can be @c NULL, in which case a
 *               new hash table is allocated and returned.
 * @param   key  A unique string.  Can be @c NULL.
 * @param   data Data to associate with the string given by @p key.
 * @return  Either the given hash table, or if the given value for @p
 *          hash is @c NULL, then a new one.  @c NULL will be returned
 *          if memory could not be allocated for a new table.
 * @ingroup Eina_Hash_Data
 */
EAPI Eina_Hash *
eina_hash_direct_add(Eina_Hash *hash, const char *key, const void *data)
{
	int hash_num;
	Eina_Hash_El *el;

	if ((!key) || (!data)) return hash;
	_eina_hash_alloc_error = 0;
	if (!hash)
	{
		hash = calloc(1, sizeof(struct _Eina_Hash));
		if (!hash)
		{
			_eina_hash_alloc_error = 1;
			return NULL;
		}
	}
	if (!(el = malloc(sizeof(struct _Eina_Hash_El))))
	{
		if (hash->population <= 0)
		{
			free(hash);
			hash = NULL;
		}
		_eina_hash_alloc_error = 1;
		return hash;
	};
	el->key = key;
	el->data = (void *)data;
	hash_num = _eina_hash_gen(key);
	hash->buckets[hash_num] = eina_inlist_prepend(hash->buckets[hash_num], el);
	if (eina_list_alloc_error())
	{
		_eina_hash_alloc_error = 1;
		free(el);
		return hash;
	}
	hash->population++;
	return hash;
}

/**
 * Removes the entry identified by @p key or @p data from the given
 * hash table.
 *
 * If @p key is @c NULL, then @p data is used to find a match to
 * remove.
 *
 * @param   hash The given hash table.
 * @param   key  The key string.  Can be @c NULL.
 * @param   data The data pointer to remove if @p key is @c NULL.
 *               Otherwise, not required and can be @c NULL.
 * @return  The modified hash table.  If there are no entries left, the
 *          hash table will be freed and @c NULL will be returned.
 * @ingroup Eina_Hash_Data
 */
EAPI Eina_Hash *
eina_hash_del(Eina_Hash *hash, const char *key, const void *data)
{
	int hash_num;
	Eina_Hash_El *el;
	Eina_Inlist *l;

	if (!hash) return NULL;
	if (!key)
	{
		int hash_num;

		for (hash_num = 0; hash_num < 256; hash_num++)
		{
			for (l = hash->buckets[hash_num]; l; l = l->next)
			{
				el = (Eina_Hash_El *)l;
				if (el->data == data)
				{
					hash->buckets[hash_num] = eina_inlist_remove(hash->buckets[hash_num], el);
					free(el);
					hash->population--;
					if (hash->population <= 0)
					{
						free(hash);
						hash = NULL;
					}
					return hash;
				}
			}
		}
	}
	else
	{
		hash_num = _eina_hash_gen(key);
		for (l = hash->buckets[hash_num]; l; l = l->next)
		{
			el = (Eina_Hash_El *)l;
			if (!strcmp(el->key, key))
			{
				hash->buckets[hash_num] = eina_inlist_remove(hash->buckets[hash_num], el);
				free(el);
				hash->population--;
				if (hash->population <= 0)
				{
					free(hash);
					hash = NULL;
				}
				return hash;
			}
		}
	}
	return hash;
}

/**
 * Retrieves a specific entry in the given hash table.
 * @param   hash The given hash table.
 * @param   key  The key string of the entry to find.
 * @return  The data pointer for the stored entry, or @c NULL if not
 *          found.
 * @ingroup Eina_Hash_Data
 */
EAPI void * eina_hash_find(const Eina_Hash *hash, const char *key)
{
	int hash_num;
	Eina_Hash_El *el;
	Eina_Inlist *l;

	_eina_hash_alloc_error = 0;
	if ((!hash) || (!key))
		return NULL;
	hash_num = _eina_hash_gen(key);
	for (l = hash->buckets[hash_num]; l; l = l->next) {
		el = (Eina_Hash_El *)l;
		if (!strcmp(el->key, key)) {
			if (l != hash->buckets[hash_num]) {
				Eina_Inlist *bucket;

				bucket = hash->buckets[hash_num];
				bucket = eina_inlist_remove(bucket, el);
				bucket = eina_inlist_prepend(bucket, el);
				((Eina_Hash *)hash)->buckets[hash_num]
						= bucket;
			}
			return el->data;
		}
	}
	return NULL;
}

/**
 * Modifies the entry pointer at the specified key and returns the old entry
 * @param   hash The given hash table.
 * @param   key  The key string of the entry to modify.
 * @param   data The data to replace the old entry, if it exists.
 * @return  The data pointer for the old stored entry, or @c NULL if not
 *          found. If an existing entry is not found, nothing is added to the
 *          hash.
 * @ingroup Eina_Hash_Data
 */
EAPI void * eina_hash_modify(Eina_Hash *hash, const char *key, const void *data)
{
	int hash_num;
	Eina_Hash_El *el;
	Eina_Inlist *l;

	_eina_hash_alloc_error = 0;
	if (!hash)
		return NULL;
	hash_num = _eina_hash_gen(key);
	for (l = hash->buckets[hash_num]; l; l = l->next) {
		el = (Eina_Hash_El *)l;
		if ((key) && (!strcmp(el->key, key))) {
			void *old_data;

			if (l != hash->buckets[hash_num]) {
				hash->buckets[hash_num]
						= eina_inlist_remove(
								hash->buckets[hash_num],
								el);
				hash->buckets[hash_num]
						= eina_inlist_prepend(
								hash->buckets[hash_num],
								el);
			}
			old_data = el->data;
			el->data = (void *) data;
			return old_data;
		}
	}
	return NULL;
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
EAPI int eina_hash_size(const Eina_Hash *hash)
{
	if (!hash)
		return 0;
	return 256;
}

/**
 * @todo Complete polishing documentation for eina_hash.c. The
 * functions' docs may be grouped, but they need some simplification.
 */

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
EAPI void eina_hash_free(Eina_Hash *hash)
{
	int i, size;

	if (!hash)
		return;
	size = eina_hash_size(hash);
	for (i = 0; i < size; i++) {
		while (hash->buckets[i]) {
			Eina_Hash_El *el;

			el = (Eina_Hash_El *)hash->buckets[i];
			hash->buckets[i] = eina_inlist_remove(
					hash->buckets[i], el);
			free(el);
		}
	}
	free(hash);
}

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
		Eina_Bool (*func) (const Eina_Hash *hash, const char *key, void *data, void *fdata),
		const void *fdata)
{
	int i, size;

	if (!hash)
		return;
	size = eina_hash_size(hash);
	for (i = 0; i < size; i++) {
		Eina_Inlist *l, *next_l;

		for (l = hash->buckets[i]; l;) {
			Eina_Hash_El *el;

			next_l = l->next;
			el = (Eina_Hash_El *)l;
			if (!func(hash, el->key, el->data, (void *)fdata))
				return;
			l = next_l;
		}
	}
}

/**
 * Return memory allocation failure flag after an function requiring allocation
 * @return The state of the allocation flag
 *
 * This function returns the state of the memory allocation flag. This flag is
 * set if memory allocations fail during eina_hash_add() calls. If they do, 1
 * will be returned, otherwise 0 will be returned. The flag will remain in its
 * current state until the next call that requires allocation is called, and
 * is then reset.
 *
 * Example:
 * @code
 * Eina_Hash *hash = NULL;
 * extern void *my_data;
 *
 * hash = eina_hash_add(hash, "My Data", my_data);
 * if (eina_hash_alloc_error())
 *   {
 *     fprintf(stderr, "ERROR: Memory is low. Hash allocation failed.\n");
 *     exit(-1);
 *   }
 * if (eina_hash_find(hash, "My Data") == my_data)
 *   {
 *     printf("My Data inserted and successfully found.\n");
 *   }
 * @endcode
 * @ingroup Eina_Hash_General_Group
 */
EAPI int eina_hash_alloc_error(void)
{
	return _eina_hash_alloc_error;
}

/* Common hash functions */

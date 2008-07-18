/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "Evas_Data.h"

typedef struct _Evas_Hash_El Evas_Hash_El;

struct _Evas_Hash_El
{
   Evas_Object_List  _list_data;
   const char       *key;
   void             *data;
};

static inline int _evas_hash_gen(const char *key);

static int _evas_hash_alloc_error = 0;

static inline int
_evas_hash_gen(const char *key)
{
   unsigned int hash_num = 5381;
   const unsigned char *ptr;
   
   if (!key) return 0;
   for (ptr = (unsigned char *)key; *ptr; ptr++)
     hash_num = (hash_num * 33) ^ *ptr;
   
   hash_num &= 0xff;
   return (int)hash_num;
}

/**
 * @defgroup Evas_Hash_Data Hash Data Functions
 *
 * Functions that add, access or remove data from hashes.
 *
 * The following example shows how to add and then access data in a
 * hash table:
 * @code
 * Evas_Hash *hash = NULL;
 * extern void *my_data;
 *
 * hash = evas_hash_add(hash, "My Data", my_data);
 * if (evas_hash_alloc_error())
 *   {
 *     fprintf(stderr, "ERROR: Memory is low. Hash allocation failed.\n");
 *     exit(-1);
 *   }
 * if (evas_hash_find(hash, "My Data") == my_data)
 *   {
 *     printf("My Data inserted and successfully found.\n");
 *   }
 * @endcode
 *
 * What follows is another example, showing how the @ref evas_hash_del
 * function is used:
 * @code
 * extern Evas_Hash *hash;
 * extern void *data;
 *
 * printf("Insert some data...\n");
 * hash = evas_hash_add(hash, "My Data", my_data);
 * printf("Removing by key...\n");
 * hash = evas_hash_del(hash, "My Data", NULL);
 * printf("Insert some more data as a NULL key...\n");
 * hash = evas_hash_add(hash, NULL, my_data);
 * printf("Removing by data as a NULL key...\n");
 * hash = evas_hash_del(hash, NULL, my_data);
 * @endcode
 */

/**
 * Adds an entry to the given hash table.
 *
 * @p key is expected to be a unique string within the hash table.
 * Otherwise, you cannot be sure which inserted data pointer will be
 * accessed with @ref evas_hash_find , and removed with
 * @ref evas_hash_del .
 *
 * Key strings are case sensitive.
 *
 * @ref evas_hash_alloc_error should be used to determine if an
 * allocation error occurred during this function.
 *
 * @param   hash The given hash table.  Can be @c NULL, in which case a
 *               new hash table is allocated and returned.
 * @param   key  A unique string.  Can be @c NULL.
 * @param   data Data to associate with the string given by @p key.
 * @return  Either the given hash table, or if the given value for @p
 *          hash is @c NULL, then a new one.  @c NULL will be returned
 *          if memory could not be allocated for a new table.
 * @ingroup Evas_Hash_Data
 */
EAPI Evas_Hash *
evas_hash_add(Evas_Hash *hash, const char *key, const void *data)
{
   int hash_num;
   Evas_Hash_El *el;

   if ((!key) || (!data)) return hash;
   _evas_hash_alloc_error = 0;
   if (!hash)
     {
	hash = calloc(1, sizeof(struct _Evas_Hash));
	if (!hash)
	  {
	     _evas_hash_alloc_error = 1;
	     return NULL;
	  }
     }
   if (!(el = malloc(sizeof(struct _Evas_Hash_El) + strlen(key) + 1)))
     {
        if (hash->population <= 0)
	  {
	     free(hash);
	     hash = NULL;
	  }
	_evas_hash_alloc_error = 1;
	return hash;
     };
   el->key = ((char *)el) + sizeof(struct _Evas_Hash_El);
   strcpy((char *) el->key, key);
   el->data = (void *)data;
   hash_num = _evas_hash_gen(key);
   hash->buckets[hash_num] = evas_object_list_prepend(hash->buckets[hash_num], el);
   hash->population++;
   return hash;
}

/**
 * Adds an entry to the given hash table and does not duplicate the string key.
 *
 * @p key is expected to be a unique string within the hash table.
 * Otherwise, you cannot be sure which inserted data pointer will be
 * accessed with @ref evas_hash_find , and removed with
 * @ref evas_hash_del . This call does not make a copy of the key so it must
 * be a string constant or stored elsewhere (in the object being added) etc.
 *
 * Key strings are case sensitive.
 *
 * @ref evas_hash_alloc_error should be used to determine if an
 * allocation error occurred during this function.
 *
 * @param   hash The given hash table.  Can be @c NULL, in which case a
 *               new hash table is allocated and returned.
 * @param   key  A unique string.  Can be @c NULL.
 * @param   data Data to associate with the string given by @p key.
 * @return  Either the given hash table, or if the given value for @p
 *          hash is @c NULL, then a new one.  @c NULL will be returned
 *          if memory could not be allocated for a new table.
 * @ingroup Evas_Hash_Data
 */
EAPI Evas_Hash *
evas_hash_direct_add(Evas_Hash *hash, const char *key, const void *data)
{
   int hash_num;
   Evas_Hash_El *el;

   if ((!key) || (!data)) return hash;
   _evas_hash_alloc_error = 0;
   if (!hash)
     {
	hash = calloc(1, sizeof(struct _Evas_Hash));
	if (!hash)
	  {
	     _evas_hash_alloc_error = 1;
	     return NULL;
	  }
     }
   if (!(el = malloc(sizeof(struct _Evas_Hash_El))))
     {
        if (hash->population <= 0)
	  {
	     free(hash);
	     hash = NULL;
	  }
	_evas_hash_alloc_error = 1;
	return hash;
     };
   el->key = key;
   el->data = (void *)data;
   hash_num = _evas_hash_gen(key);
   hash->buckets[hash_num] = evas_object_list_prepend(hash->buckets[hash_num], el);
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
 * @ingroup Evas_Hash_Data
 */
EAPI Evas_Hash *
evas_hash_del(Evas_Hash *hash, const char *key, const void *data)
{
   int hash_num;
   Evas_Hash_El *el;
   Evas_Object_List *l;

   if (!hash) return NULL;
   if (!key)
     {
	int hash_num;
	
	for (hash_num = 0; hash_num < 256; hash_num++)
	  {
	     for (l = hash->buckets[hash_num]; l; l = l->next)
	       {
		  el = (Evas_Hash_El *)l;
		  if (el->data == data)
		    {
		       hash->buckets[hash_num] = evas_object_list_remove(hash->buckets[hash_num], el);
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
	hash_num = _evas_hash_gen(key);
	for (l = hash->buckets[hash_num]; l; l = l->next)
	  {
	     el = (Evas_Hash_El *)l;
	     if (!strcmp(el->key, key))
	       {
		  hash->buckets[hash_num] = evas_object_list_remove(hash->buckets[hash_num], el);
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
 * @ingroup Evas_Hash_Data
 */
EAPI void *
evas_hash_find(const Evas_Hash *hash, const char *key)
{
   int hash_num;
   Evas_Hash_El *el;
   Evas_Object_List *l;

   _evas_hash_alloc_error = 0;
   if ((!hash) || (!key)) return NULL;
   hash_num = _evas_hash_gen(key);
   for (l = hash->buckets[hash_num]; l; l = l->next)
     {
	el = (Evas_Hash_El *)l;
	if (!strcmp(el->key, key))
	  {
	     if (l != hash->buckets[hash_num])
	       {
		  Evas_Object_List *bucket;

		  bucket = hash->buckets[hash_num];
		  bucket = evas_object_list_remove(bucket, el);
		  bucket = evas_object_list_prepend(bucket, el);
		  ((Evas_Hash *)hash)->buckets[hash_num] = bucket;
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
 * @ingroup Evas_Hash_Data
 */
EAPI void *
evas_hash_modify(Evas_Hash *hash, const char *key, const void *data)
{
   int hash_num;
   Evas_Hash_El *el;
   Evas_Object_List *l;

   _evas_hash_alloc_error = 0;
   if (!hash) return NULL;
   hash_num = _evas_hash_gen(key);
   for (l = hash->buckets[hash_num]; l; l = l->next)
     {
	el = (Evas_Hash_El *)l;
	if ((key) && (!strcmp(el->key, key)))
	  {
	     void *old_data;
	     
	     if (l != hash->buckets[hash_num])
	       {
		  hash->buckets[hash_num] = evas_object_list_remove(hash->buckets[hash_num], el);
		  hash->buckets[hash_num] = evas_object_list_prepend(hash->buckets[hash_num], el);
	       }
	     old_data = el->data;
	     el->data = (void *) data;
	     return old_data;
	  }
     }
   return NULL;
}

/**
 * @defgroup Evas_Hash_General_Group Hash General Functions
 *
 * Miscellaneous functions that operate on hash objects.
 */

/**
 * Retrieves the number of buckets available in the given hash table.
 * @param hash The given hash table.
 * @return @c 256 if @p hash is not @c NULL.  @c 0 otherwise.
 * @ingroup Evas_Hash_General_Group
 */
EAPI int
evas_hash_size(const Evas_Hash *hash)
{
   if (!hash) return 0;
   return 256;
}

/**
 * @todo Complete polishing documentation for evas_hash.c. The
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
 * extern Evas_Hash *hash;
 *
 * evas_hash_free(hash);
 * hash = NULL;
 * @endcode
 * @ingroup Evas_Hash_General_Group
 */
EAPI void
evas_hash_free(Evas_Hash *hash)
{
   int i, size;

   if (!hash) return;
   size = evas_hash_size(hash);
   for (i = 0; i < size; i++)
     {
	while (hash->buckets[i])
	  {
	     Evas_Hash_El *el;

	     el = (Evas_Hash_El *)hash->buckets[i];
	     hash->buckets[i] = evas_object_list_remove(hash->buckets[i], el);
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
 * extern Evas_Hash *hash;
 *
 * Evas_Bool hash_fn(Evas_Hash *hash, const char *key, void *data, void *fdata)
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
 *   evas_hash_foreach(hash, hash_fn, hash_fn_data);
 *   free(hash_fn_data);
 * }
 * @endcode
 * @ingroup Evas_Hash_General_Group
 */
EAPI void
evas_hash_foreach(const Evas_Hash *hash, Evas_Bool (*func) (const Evas_Hash *hash, const char *key, void *data, void *fdata), const void *fdata)
{
   int i, size;

   if (!hash) return;
   size = evas_hash_size(hash);
   for (i = 0; i < size; i++)
     {
	Evas_Object_List *l, *next_l;

	for (l = hash->buckets[i]; l;)
	  {
	     Evas_Hash_El *el;

	     next_l = l->next;
	     el = (Evas_Hash_El *)l;
	     if (!func(hash, el->key, el->data, (void *)fdata)) return;
	     l = next_l;
	  }
     }
}

/**
 * Return memory allocation failure flag after an function requiring allocation
 * @return The state of the allocation flag
 *
 * This function returns the state of the memory allocation flag. This flag is
 * set if memory allocations fail during evas_hash_add() calls. If they do, 1
 * will be returned, otherwise 0 will be returned. The flag will remain in its
 * current state until the next call that requires allocation is called, and
 * is then reset.
 *
 * Example:
 * @code
 * Evas_Hash *hash = NULL;
 * extern void *my_data;
 *
 * hash = evas_hash_add(hash, "My Data", my_data);
 * if (evas_hash_alloc_error())
 *   {
 *     fprintf(stderr, "ERROR: Memory is low. Hash allocation failed.\n");
 *     exit(-1);
 *   }
 * if (evas_hash_find(hash, "My Data") == my_data)
 *   {
 *     printf("My Data inserted and successfully found.\n");
 *   }
 * @endcode
 * @ingroup Evas_Hash_General_Group
 */
EAPI int
evas_hash_alloc_error(void)
{
   return _evas_hash_alloc_error;
}

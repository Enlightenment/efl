#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

static int evas_hash_gen(const char *key);

static int _evas_hash_alloc_error = 0;

static int
evas_hash_gen(const char *key)
{
   unsigned int hash_num = 0;
   const unsigned char *ptr;
   
   if (!key) return 0;
   
   for (ptr = (unsigned char *)key; *ptr; ptr++) hash_num ^= (int)(*ptr);
   
   hash_num &= 0xff;
   return (int)hash_num;
}

/**
 * Add an entry to the hash table.
 * @param hash The hash table handle to hold the entry
 * @param key The key string for the entry
 * @param data The pointer to the data to be stored
 * @return The modified hash table handle
 * 
 * This function takes the data pointed to by @p data and inserts it into the
 * hash table @p hash, under the string key @p key. It will return a modified
 * hash table handle when done. An empty hash table starts as a NULL pointer.
 * 
 * It is expected that the @p key will be a unique string within the hash table
 * otherwise it is not deterministic as to which inserted data pointer will be
 * returned when evas_hash_find() is called, or which one wil be removed on
 * evas_hash_del() being called. A @p key of NULL is allowed, but will mean
 * unique lookups are impossible (unless the entry is the only one with a NULL
 * key in the hash table). Key strings are casesensitive.
 * 
 * If a memory allocation failure occurs, the old hash table pointer will be
 * returned, with the hash table contents unchanged, and the programmer should
 * use evas_hash_alloc_error() to determine if an allocation error occured and
 * recover from this situation.
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
 */
Evas_Hash *
evas_hash_add(Evas_Hash *hash, const char *key, void *data)
{
   int hash_num;
   Evas_Hash_El *el;

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
   if (key) 
     {
        el->key = strdup(key);
	if (!el->key)
	  {
	     free(el);
	     _evas_hash_alloc_error = 1;
	     return hash;
	  }
        hash_num = evas_hash_gen(key);
     }
   else 
     {
        el->key = NULL;
	hash_num = 0;
     }
   el->data = data;
   hash->buckets[hash_num] = evas_object_list_prepend(hash->buckets[hash_num], el);
   if (evas_list_alloc_error())
     {
	_evas_hash_alloc_error = 1;
	if (el->key) free(el->key);
	free(el);
	return hash;
     }
   hash->population++;
   return hash;
}

/**
 * Remove an entry from the hash table
 * @param hash The hash table handle to remove the entry from
 * @param key The key string for the entry
 * @param data The pointer to the data to be removed
 * @return The modified hash table handle
 * 
 * This function removed an entry from the hash table pointed to by @p hash.
 * The key is identified by its string @p key. If the @p key is NULL, then the
 * data pointer @p data must be provided for a match to be possible so the
 * entry can be removed. As long as the @p key is not NULL, the @p data pointer
 * is not required and can be NULL. Remember than key strings are case
 * sensitive.
 * 
 * This function returns the modified hash table after removal.
 * 
 * Example:
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
Evas_Hash *
evas_hash_del(Evas_Hash *hash, const char *key, void *data)
{
   int hash_num;
   Evas_Hash_El *el;
   Evas_Object_List *l;
   
   if (!hash) return NULL;
   hash_num = evas_hash_gen(key);
   for (l = hash->buckets[hash_num]; l; l = l->next)
     {
	el = (Evas_Hash_El *)l;
	if ((((el->key) && (key) && (!strcmp(el->key, key))) ||
	     ((!el->key) && (!key))) && (el->data == data))
	  {
	     hash->buckets[hash_num] = evas_object_list_remove(hash->buckets[hash_num], el);
	     if (el->key) free(el->key);
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
   return hash;
}

/**
 * Find a specified entry in a hash table
 * @param hash The hash table handle to find the entry in
 * @param key The key string for the entry
 * @return The data pointer for the stored entry, or NULL if not found.
 * 
 * This function finds an entry by its @p key, which is case sensitive. If the
 * entry in the hash table @p hash is found, the data pointer that was stored
 * under than entry is returned, otherwise NULL is returned.
 * 
 * Example:
 * @code
 * extern Evas_Hash *hash;
 * extern void *data;
 * 
 * hash = evas_hash_add(hash, "My Data", my_data);
 * if (evas_hash_find(hash, "My Data") == my_data)
 *   {
 *     printf("Found stored entry!\n");
 *   }
 * @endcode
 */
void *
evas_hash_find(Evas_Hash *hash, const char *key)
{
   int hash_num;
   Evas_Hash_El *el;
   Evas_Object_List *l;

   _evas_hash_alloc_error = 0;
   if (!hash) return NULL;
   hash_num = evas_hash_gen(key);
   for (l = hash->buckets[hash_num]; l; l = l->next)
     {
	el = (Evas_Hash_El *)l;
	if (((el->key) && (key) && (!strcmp(el->key, key))) ||
	    ((!el->key) && (!key)))
	  {
	     if (l != hash->buckets[hash_num])
	       {
		  /* FIXME: move to front of list without alloc */
		  hash->buckets[hash_num] = evas_object_list_remove(hash->buckets[hash_num], el);
		  hash->buckets[hash_num] = evas_object_list_prepend(hash->buckets[hash_num], el);
		  if (evas_list_alloc_error())
		    {
		       _evas_hash_alloc_error = 1;
		       return el->data;
		    }
	       }
	     return el->data;
	  }
     }
   return NULL;
}

/**
 * Return the number of buckets in the hash table
 * @param hash The hash table to return the bucket count of
 * @return The number of buckets in the hash table
 * 
 * This function returns the number of buckes in he hash table @p hash.
 * 
 * Example:
 * @code
 * extern Evas_Hash *hash;
 * 
 * printf("Hash bucket count: %i\n", evas_hash_size(hash));
 * @endcode
 */
int
evas_hash_size(Evas_Hash *hash)
{
   if (!hash) return 0;
   return 256;
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
 * extern Evas_Hash *hash;
 * 
 * evas_hash_free(hash);
 * hash = NULL;
 * @endcode
 */
void
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
	     if (el->key) free(el->key);
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
 * hash table contents if it reeturns 1. IF the hash table contents are
 * modified by this function or the function wishes to stop processing it must
 * return 0, otherwise return 1 to keep processing.
 * 
 * Example:
 * @code
 * extern Evas_Hash *hash;
 * 
 * int hash_fn(Evas_Hash *hash, const char *key, void *data, void *fdata)
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
 */
void
evas_hash_foreach(Evas_Hash *hash, int (*func) (Evas_Hash *hash, const char *key, void *data, void *fdata), void *fdata)
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
	     if (!func(hash, el->key, el->data, fdata)) return;
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
 */
int
evas_hash_alloc_error(void)
{
   return _evas_hash_alloc_error;
}

#include "ecore_private.h"
#include "Ecore.h"
#include "Ecore_Data.h"

#define PRIME_TABLE_MAX 21
#define PRIME_MIN 17
#define PRIME_MAX 16777213

#define ECORE_HASH_CHAIN_MAX 3

#define ECORE_COMPUTE_HASH(hash, key) hash->hash_func(key) % \
					ecore_prime_table[hash->size];

#define ECORE_HASH_INCREASE(hash) ((hash && ecore_prime_table[hash->size] < PRIME_MAX) ? \
		(hash->nodes / ecore_prime_table[hash->size]) > \
		ECORE_HASH_CHAIN_MAX : FALSE)
#define ECORE_HASH_REDUCE(hash) ((hash && ecore_prime_table[hash->size] > PRIME_MIN) ? \
		(double)hash->nodes / (double)ecore_prime_table[hash->size-1] \
		< ((double)ECORE_HASH_CHAIN_MAX * 0.375) : FALSE)

/* Private hash manipulation functions */
static int _ecore_hash_node_add(Ecore_Hash *hash, Ecore_Hash_Node *node);
static Ecore_Hash_Node * _ecore_hash_node_get(Ecore_Hash *hash, const void *key);
static int _ecore_hash_increase(Ecore_Hash *hash);
static int _ecore_hash_decrease(Ecore_Hash *hash);
inline int _ecore_hash_rehash(Ecore_Hash *hash, Ecore_Hash_Node **old_table, int old_size);
static int _ecore_hash_bucket_destroy(Ecore_Hash_Node *list, Ecore_Free_Cb keyd,
				      Ecore_Free_Cb valued);
inline Ecore_Hash_Node * _ecore_hash_bucket_get(Ecore_Hash *hash,
						Ecore_Hash_Node *bucket, const void *key);

static Ecore_Hash_Node *_ecore_hash_node_new(void *key, void *value);
static int _ecore_hash_node_init(Ecore_Hash_Node *node, void *key, void *value);
static int _ecore_hash_node_destroy(Ecore_Hash_Node *node, Ecore_Free_Cb keyd,
				    Ecore_Free_Cb valued);

/**
 * @defgroup Ecore_Data_Hash_ADT_Creation_Group Hash Creation Functions
 *
 * Functions that create hash tables.
 */

/**
 * Creates and initializes a new hash
 * @param hash_func The function for determining hash position.
 * @param compare   The function for comparing node keys.
 * @return @c NULL on error, a new hash on success.
 * @ingroup Ecore_Data_Hash_ADT_Creation_Group
 */
EAPI Ecore_Hash *
ecore_hash_new(Ecore_Hash_Cb hash_func, Ecore_Compare_Cb compare)
{
   Ecore_Hash *new_hash = (Ecore_Hash *)malloc(sizeof(Ecore_Hash));
   if (!new_hash)
     return NULL;

   if (!ecore_hash_init(new_hash, hash_func, compare))
     {
	FREE(new_hash);
	return NULL;
     }

   return new_hash;
}

/**
 * Initializes the given hash.
 * @param   hash       The given hash.
 * @param   hash_func  The function used for hashing node keys.
 * @param   compare    The function used for comparing node keys.
 * @return  @c TRUE on success, @c FALSE on an error.
 * @ingroup Ecore_Data_Hash_ADT_Creation_Group
 */
EAPI int
ecore_hash_init(Ecore_Hash *hash, Ecore_Hash_Cb hash_func, Ecore_Compare_Cb compare)
{
   CHECK_PARAM_POINTER_RETURN("hash", hash, FALSE);

   memset(hash, 0, sizeof(Ecore_Hash));

   hash->hash_func = hash_func;
   hash->compare = compare;

   hash->buckets = (Ecore_Hash_Node **)calloc(ecore_prime_table[0],
					      sizeof(Ecore_Hash_Node *));

   return TRUE;
}

/**
 * @defgroup Ecore_Data_Hash_ADT_Destruction_Group Hash Destruction Functions
 *
 * Functions that destroy hash tables and their contents.
 */

/**
 * Sets the function to destroy the keys of the given hash.
 * @param   hash     The given hash.
 * @param   function The function used to free the node keys. NULL is a
 *          valid value and means that no function will be called.
 * @return  @c TRUE on success, @c FALSE on error.
 * @ingroup Ecore_Data_Hash_ADT_Destruction_Group
 */
EAPI int
ecore_hash_free_key_cb_set(Ecore_Hash *hash, Ecore_Free_Cb function)
{
   CHECK_PARAM_POINTER_RETURN("hash", hash, FALSE);

   hash->free_key = function;

   return TRUE;
}

/**
 * Sets the function to destroy the values in the given hash.
 * @param   hash     The given hash.
 * @param   function The function that will free the node values. NULL is a
 *          valid value and means that no function will be called.
 * @return  @c TRUE on success, @c FALSE on error
 * @ingroup Ecore_Data_Hash_ADT_Destruction_Group
 */
EAPI int
ecore_hash_free_value_cb_set(Ecore_Hash *hash, Ecore_Free_Cb function)
{
   CHECK_PARAM_POINTER_RETURN("hash", hash, FALSE);

   hash->free_value = function;

   return TRUE;
}

/**
 * @defgroup Ecore_Data_Hash_ADT_Data_Group Hash Data Functions
 *
 * Functions that set, access and delete values from the hash tables.
 */

/**
 * Sets a key-value pair in the given hash table.
 * @param   hash    The given hash table.
 * @param   key     The key.
 * @param   value   The value.
 * @return  @c TRUE if successful, @c FALSE if not.
 * @ingroup Ecore_Data_Hash_ADT_Data_Group
 */
EAPI int
ecore_hash_set(Ecore_Hash *hash, void *key, void *value)
{
   int ret = FALSE;
   Ecore_Hash_Node *node;

   CHECK_PARAM_POINTER_RETURN("hash", hash, FALSE);

   node = _ecore_hash_node_get(hash, key);
   if (node)
     {
	if (hash->free_key) hash->free_key(key);
	if (node->value && hash->free_value) hash->free_value(node->value);
	node->value = value;
	ret = TRUE;
     }
   else
     {
	node = _ecore_hash_node_new(key, value);
	if (node)
	  ret = _ecore_hash_node_add(hash, node);
     }

   return ret;
}

/**
 * Sets all key-value pairs from set in the given hash table.
 * @param   hash    The given hash table.
 * @param   set     The hash table to import.
 * @return  @c TRUE if successful, @c FALSE if not.
 * @ingroup Ecore_Data_Hash_ADT_Data_Group
 */
EAPI int
ecore_hash_hash_set(Ecore_Hash *hash, Ecore_Hash *set)
{
   unsigned int i;
   Ecore_Hash_Node *node, *old;

   CHECK_PARAM_POINTER_RETURN("hash", hash, FALSE);
   CHECK_PARAM_POINTER_RETURN("set", set, FALSE);

   for (i = 0; i < ecore_prime_table[set->size]; i++)
     {
	/* Hash into a new list to avoid loops of rehashing the same nodes */
	while ((old = set->buckets[i]))
	  {
	     set->buckets[i] = old->next;
	     old->next = NULL;
	     node = _ecore_hash_node_get(hash, old->key);
	     if (node)
	       {
		  /* This key already exists. Delete the old and add the new
		   * value */
		  if (hash->free_key) hash->free_key(node->key);
		  if (hash->free_value) hash->free_key(node->value);
		  node->key = old->key;
		  node->value = old->value;
		  free(old);
	       }
	     else
	       _ecore_hash_node_add(hash, old);
	  }
     }
   FREE(set->buckets);
   ecore_hash_init(set, set->hash_func, set->compare);
   return TRUE;
}

/**
 * Frees the hash table and the data contained inside it.
 * @param   hash The hash table to destroy.
 * @return  @c TRUE on success, @c FALSE on error.
 * @ingroup Ecore_Data_Hash_ADT_Destruction_Group
 */
EAPI void 
ecore_hash_destroy(Ecore_Hash *hash)
{
   unsigned int i = 0;

   CHECK_PARAM_POINTER("hash", hash);

   if (hash->buckets)
     {
	while (i < ecore_prime_table[hash->size])
	  {
	     if (hash->buckets[i])
	       {
		  Ecore_Hash_Node *bucket;

				/*
				 * Remove the bucket list to avoid possible recursion
				 * on the free callbacks.
				 */
		  bucket = hash->buckets[i];
		  hash->buckets[i] = NULL;
		  _ecore_hash_bucket_destroy(bucket,
					     hash->free_key,
					     hash->free_value);
	       }
	     i++;
	  }

	FREE(hash->buckets);
     }
   FREE(hash);

   return;
}

/**
 * @defgroup Ecore_Data_Hash_ADT_Traverse_Group Hash Traverse Functions
 *
 * Functions that iterate through hash tables.
 */

/**
 * Counts the number of nodes in a hash table.
 * @param   hash The hash table to count current nodes.
 * @return  The number of nodes in the hash.
 * @ingroup Ecore_Data_Hash_ADT_Destruction_Group
 */
EAPI int
ecore_hash_count(Ecore_Hash *hash)
{
   CHECK_PARAM_POINTER_RETURN("hash", hash, 0);

   return hash->nodes;
}

/**
 * Runs the @p for_each_func function on each entry in the given hash.
 * @param   hash          The given hash.
 * @param   for_each_func The function that each entry is passed to.
 * @param		user_data			a pointer passed to calls of for_each_func
 * @return  TRUE on success, FALSE otherwise.
 * @ingroup Ecore_Data_Hash_ADT_Traverse_Group
 */
EAPI int 
ecore_hash_for_each_node(Ecore_Hash *hash, Ecore_For_Each for_each_func, void *user_data)
{
   unsigned int i = 0;

   CHECK_PARAM_POINTER_RETURN("hash", hash, FALSE);
   CHECK_PARAM_POINTER_RETURN("for_each_func", for_each_func, FALSE);

   while (i < ecore_prime_table[hash->size])
     {
	if (hash->buckets[i])
	  {
	     Ecore_Hash_Node *node;

	     for (node = hash->buckets[i]; node; node = node->next)
	       {
		  for_each_func(node, user_data);
	       }
	  }
	i++;
     }

   return TRUE;
}

/**
 * Retrieves an ecore_list of all keys in the given hash.
 * @param   hash          The given hash.
 * @return  new ecore_list on success, NULL otherwise
 * @ingroup Ecore_Data_Hash_ADT_Traverse_Group
 */
EAPI Ecore_List *
ecore_hash_keys(Ecore_Hash *hash)
{
   unsigned int i = 0;
   Ecore_List *keys;

   CHECK_PARAM_POINTER_RETURN("hash", hash, NULL);

   keys = ecore_list_new();
   while (i < ecore_prime_table[hash->size])
     {
	if (hash->buckets[i])
	  {
	     Ecore_Hash_Node *node;

	     for (node = hash->buckets[i]; node; node = node->next)
	       {
		  ecore_list_append(keys, node->key);
	       }
	  }
	i++;
     }
   ecore_list_first_goto(keys);

   return keys;
}

/**
 * Prints the distribution of the given hash table for graphing.
 * @param hash The given hash table.
 */
EAPI void
ecore_hash_dump_graph(Ecore_Hash *hash)
{
   unsigned int i;

   for (i = 0; i < ecore_prime_table[hash->size]; i++)
     if (hash->buckets[i])
       {
	  int n = 0;
	  Ecore_Hash_Node *node;
	  for (node = hash->buckets[i]; node; node = node->next)
	    n++;
	  printf("%d\t%u\n", i, n);
       }
   else
     printf("%d\t0\n", i);
}

/**
 * Prints the distribution of the given hash table for graphing.
 * @param hash The given hash table.
 */
EAPI void
ecore_hash_dump_stats(Ecore_Hash *hash)
{
   unsigned int i;
   double variance, sum_n_2 = 0, sum_n = 0;

   for (i = 0; i < ecore_prime_table[hash->size]; i++)
     {
	if (hash->buckets[i])
	  {
	     int n = 0;
	     Ecore_Hash_Node *node;
	     for (node = hash->buckets[i]; node; node = node->next)
	       n++;
	     sum_n_2 += ((double)n * (double)n);
	     sum_n += (double)n;
	  }
     }
   variance = (sum_n_2 - ((sum_n * sum_n) / (double)i)) / (double)i;
   printf("Average length: %f\n\tvariance^2: %f\n", (sum_n / (double)i),
		   variance);
}

static int
_ecore_hash_bucket_destroy(Ecore_Hash_Node *list, Ecore_Free_Cb keyd, Ecore_Free_Cb valued)
{
   Ecore_Hash_Node *node;

   CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

   for (node = list; node; node = list)
     {
	list = list->next;
	_ecore_hash_node_destroy(node, keyd, valued);
     }

   return TRUE;
}

/*
 * @brief Add the node to the hash table
 * @param hash: the hash table to add the key
 * @param node: the node to add to the hash table
 * @return Returns FALSE on error, TRUE on success
 */
static int
_ecore_hash_node_add(Ecore_Hash *hash, Ecore_Hash_Node *node)
{
   unsigned int hash_val;

   CHECK_PARAM_POINTER_RETURN("hash", hash, FALSE);
   CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

   /* Check to see if the hash needs to be resized */
   if (ECORE_HASH_INCREASE(hash))
     _ecore_hash_increase(hash);

   /* Compute the position in the table */
   if (!hash->hash_func)
     hash_val = (unsigned int)node->key % ecore_prime_table[hash->size];
   else
     hash_val = ECORE_COMPUTE_HASH(hash, node->key);

   /* Prepend the node to the list at the index position */
   node->next = hash->buckets[hash_val];
   hash->buckets[hash_val] = node;
   hash->nodes++;

   return TRUE;
}

/**
 * Retrieves the value associated with the given key from the given hash
 * table.
 * @param   hash The given hash table.
 * @param   key  The key to search for.
 * @return  The value corresponding to key on success, @c NULL otherwise.
 * @ingroup Ecore_Data_Hash_ADT_Data_Group
 */
EAPI void *
ecore_hash_get(Ecore_Hash *hash, const void *key)
{
   void *data;
   Ecore_Hash_Node *node;

   CHECK_PARAM_POINTER_RETURN("hash", hash, NULL);

   node = _ecore_hash_node_get(hash, key);
   if (!node)
     return NULL;

   data = node->value;

   return data;
}

/**
 * Removes the value associated with the given key in the given hash
 * table.
 * @param   hash The given hash table.
 * @param   key  The key to search for.
 * @return  The value corresponding to the key on success.  @c NULL is
 *          returned if there is an error.
 * @ingroup Ecore_Data_Hash_ADT_Data_Group
 */
EAPI void *
ecore_hash_remove(Ecore_Hash *hash, const void *key)
{
   Ecore_Hash_Node *node = NULL;
   Ecore_Hash_Node *list;
   unsigned int hash_val;
   void *ret = NULL;

   CHECK_PARAM_POINTER_RETURN("hash", hash, NULL);

   /* Compute the position in the table */
   if (!hash->hash_func)
     hash_val = (unsigned int )key % ecore_prime_table[hash->size];
   else
     hash_val = ECORE_COMPUTE_HASH(hash, key);

   /*
    * If their is a list that could possibly hold the key/value pair
    * traverse it and remove the hash node.
    */
   if (hash->buckets[hash_val])
     {
	list = hash->buckets[hash_val];

	/*
	 * Traverse the list to find the specified key
	 */
	node = list;
	if (hash->compare)
	  {
	     while ((node) && (hash->compare(node->key, key) != 0))
	       {
		  list = node;
		  node = node->next;
	       }
	  }
	else
	  {
	     while ((node) && (node->key != key))
	       {
		  list = node;
		  node = node->next;
	       }
	  }

	/*
	 * Remove the node with the matching key and free it's memory
	 */
	if (node)
	  {
	     if (list == node)
	       hash->buckets[hash_val] = node->next;
	     else
	       list->next = node->next;
	     ret = node->value;
	     node->value = NULL;
	     _ecore_hash_node_destroy(node, hash->free_key, NULL);
	     hash->nodes--;
	  }
     }

   if (ECORE_HASH_REDUCE(hash))
     _ecore_hash_decrease(hash);

   return ret;
}

/**
 * Retrieves the first value that matches
 * table.
 * @param   hash The given hash table.
 * @param   key  The key to search for.
 * @return  The value corresponding to key on success, @c NULL otherwise.
 * @ingroup Ecore_Data_Hash_ADT_Data_Group
 */
EAPI void *
ecore_hash_find(Ecore_Hash *hash, Ecore_Compare_Cb compare, const void *value)
{
   unsigned int i = 0;

   CHECK_PARAM_POINTER_RETURN("hash", hash, NULL);
   CHECK_PARAM_POINTER_RETURN("compare", compare, NULL);
   CHECK_PARAM_POINTER_RETURN("value", value, NULL);

   while (i < ecore_prime_table[hash->size])
     {
	if (hash->buckets[i])
	  {
	     Ecore_Hash_Node *node;

	     for (node = hash->buckets[i]; node; node = node->next)
	       {
		  if (!compare(node->value, value)) return node->value;
	       }
	  }
	i++;
     }

   return NULL;
}

/*
 * @brief Retrieve the node associated with key
 * @param hash: the hash table to search for the key
 * @param key: the key to search for in the hash table
 * @return Returns NULL on error, node corresponding to key on success
 */
static Ecore_Hash_Node *
_ecore_hash_node_get(Ecore_Hash *hash, const void *key)
{
   unsigned int hash_val;
   Ecore_Hash_Node *node = NULL;

   CHECK_PARAM_POINTER_RETURN("hash", hash, NULL);

   if (!hash->buckets)
     {
	return NULL;
     }

   /* Compute the position in the table */
   if (!hash->hash_func)
     hash_val = (unsigned int )key % ecore_prime_table[hash->size];
   else
     hash_val = ECORE_COMPUTE_HASH(hash, key);

   /* Grab the bucket at the specified position */
   if (hash->buckets[hash_val])
     {
	node = _ecore_hash_bucket_get(hash, hash->buckets[hash_val], key);
	/*
	 * Move matched node to the front of the list as it's likely
	 * to be searched for again soon.
	 */
	if (node && node != hash->buckets[hash_val])
	  {
	     node->next = hash->buckets[hash_val];
	     hash->buckets[hash_val] = node;
	  }
     }

   return node;
}

/*
 * @brief Search the hash bucket for a specified key
 * @param hash: the hash table to retrieve the comparison function
 * @param bucket: the list to search for the key
 * @param key: the key to search for in the list
 * @return Returns NULL on error or not found, the found node on success
 */
inline Ecore_Hash_Node *
_ecore_hash_bucket_get(Ecore_Hash *hash, Ecore_Hash_Node *bucket, const void *key)
{
   Ecore_Hash_Node *prev = NULL;
   Ecore_Hash_Node *node = NULL;

   /*
    * Traverse the list to find the desired node, if the node is in the
    * list, then return the node.
    */
   if (hash->compare)
     {
	for (node = bucket; node; node = node->next)
	  {
	     if (hash->compare(node->key, key) == 0)
	       break;
	     prev = node;
	  }
     }
   else
     {
	for (node = bucket; node; node = node->next)
	  {
	     if (node->key == key)
	       break;
	     prev = node;
	  }
     }

   /*
    * Remove node from the list to replace it at the beginning.
    */
   if (node && prev)
     {
	prev->next = node->next;
	node->next = NULL;
     }

   return node;
}

/*
 * @brief Increase the size of the hash table by approx.  2 * current size
 * @param hash: the hash table to increase the size of
 * @return Returns TRUE on success, FALSE on error
 */
static int
_ecore_hash_increase(Ecore_Hash *hash)
{
   void *old;

   CHECK_PARAM_POINTER_RETURN("hash", hash, FALSE);

   /* Max size reached so return FALSE */
   if ((ecore_prime_table[hash->size] == PRIME_MAX) || (hash->size == PRIME_TABLE_MAX))
     return FALSE;

   /*
    * Increase the size of the hash and save a pointer to the old data
    */
   hash->size++;
   old = hash->buckets;

   /*
    * Allocate a new bucket area, of the new larger size
    */
   hash->buckets = calloc(ecore_prime_table[hash->size], sizeof(Ecore_Hash_Node *));

   /*
    * Make sure the allocation succeeded, if not replace the old data and
    * return a failure.
    */
   if (!hash->buckets)
     {
	hash->buckets = old;
	hash->size--;
	return FALSE;
     }
   hash->nodes = 0;

   /*
    * Now move all of the old data into the new bucket area
    */
   if (_ecore_hash_rehash(hash, old, hash->size - 1))
     {
	FREE(old);
	return TRUE;
     }

   /*
    * Free the old buckets regardless of success.
    */
   FREE(old);

   return FALSE;
}

/*
 * @brief Decrease the size of the hash table by < 1/2 * current size
 * @param hash: the hash table to decrease the size of
 * @return Returns TRUE on success, FALSE on error
 */
static int
_ecore_hash_decrease(Ecore_Hash *hash)
{
   Ecore_Hash_Node **old;

   CHECK_PARAM_POINTER_RETURN("hash", hash, FALSE);

   if (ecore_prime_table[hash->size] == PRIME_MIN)
     return FALSE;

   /*
    * Decrease the hash size and store a pointer to the old data
    */
   hash->size--;
   old = hash->buckets;

   /*
    * Allocate a new area to store the data
    */
   hash->buckets = (Ecore_Hash_Node **)calloc(ecore_prime_table[hash->size],
					      sizeof(Ecore_Hash_Node *));

   /*
    * Make sure allocation succeeded otherwise rreturn to the previous
    * state
    */
   if (!hash->buckets)
     {
	hash->buckets = old;
	hash->size++;
	return FALSE;
     }

   hash->nodes = 0;

   if (_ecore_hash_rehash(hash, old, hash->size + 1))
     {
	FREE(old);
	return TRUE;
     }

   return FALSE;
}

/*
 * @brief Rehash the nodes of a table into the hash table
 * @param hash: the hash to place the nodes of the table
 * @param table: the table to remove the nodes from and place in hash
 * @return Returns TRUE on success, FALSE on error
 */
inline int
_ecore_hash_rehash(Ecore_Hash *hash, Ecore_Hash_Node **old_table, int old_size)
{
   unsigned int i;
   Ecore_Hash_Node *old;

   CHECK_PARAM_POINTER_RETURN("hash", hash, FALSE);
   CHECK_PARAM_POINTER_RETURN("old_table", old_table, FALSE);

   for (i = 0; i < ecore_prime_table[old_size]; i++)
     {
	/* Hash into a new list to avoid loops of rehashing the same nodes */
	while ((old = old_table[i]))
	  {
	     old_table[i] = old->next;
	     old->next = NULL;
	     _ecore_hash_node_add(hash, old);
	  }
     }

   return TRUE;
}

/*
 * @brief Create a new hash node for key and value storage
 * @param key: the key for this node
 * @param value: the value that the key references
 * @return Returns NULL on error, a new hash node on success
 */
static Ecore_Hash_Node *
_ecore_hash_node_new(void *key, void *value)
{
   Ecore_Hash_Node *node;

   node = (Ecore_Hash_Node *)malloc(sizeof(Ecore_Hash_Node));
   if (!node)
     return NULL;

   if (!_ecore_hash_node_init(node, key, value))
     {
	FREE(node);
	return NULL;
     }

   return node;
}

/*
 * @brief Initialize a hash node to some sane default values
 * @param node: the node to set the values
 * @param key: the key to reference this node
 * @param value: the value that key refers to
 * @return Returns TRUE on success, FALSE on error
 */
static int
_ecore_hash_node_init(Ecore_Hash_Node *node, void *key, void *value)
{
   CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

   node->key = key;
   node->value = value;

   return TRUE;
}

/*
 * @brief Destroy a node and call the specified callbacks to free data
 * @param node: the node to be destroyed
 * @param keyd: the function to free the key
 * @param valued: the function  to free the value
 * @return Returns TRUE on success, FALSE on error
 */
static int
_ecore_hash_node_destroy(Ecore_Hash_Node *node, Ecore_Free_Cb keyd, Ecore_Free_Cb valued)
{
   CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

   if (keyd)
     keyd(node->key);

   if (valued)
     valued(node->value);

   FREE(node);

   return TRUE;
}

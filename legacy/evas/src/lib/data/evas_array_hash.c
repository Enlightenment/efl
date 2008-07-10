#include "evas_common.h"

#define EAH_BUCKETS 256
#define EAH_HASH(key) \
	( key % EAH_BUCKETS )

typedef struct _Evas_Array_Hash_El    Evas_Array_Hash_El;

struct _Evas_Array_Hash
{
   Evas_Array_Hash_El	*buckets[EAH_BUCKETS];
};

struct _Evas_Array_Hash_El
{
   int		  data_max;
   int		  data_count;
   int		  *data;
};

/*
 These functions provide an interface for a simple hash. The hash
 is and array of int array pointers. Right now that hash size is 256.
 The hash size is static. The key and data are ints.

 Keys must be added in ascending order because the search function
 assumes that the hash buckets are sorted.
 */
EAPI Evas_Array_Hash	*
evas_array_hash_new(void)
{
   Evas_Array_Hash *hash;

   hash = calloc(1, sizeof(Evas_Array_Hash));
   return hash;
}

EAPI void
evas_array_hash_free(Evas_Array_Hash *hash)
{
   int i;

   for (i = 0; i < EAH_BUCKETS; i++)
     {
	if (hash->buckets[i])
	  {
	     free(hash->buckets[i]->data);
	     free(hash->buckets[i]);
	  }
     }

   free(hash);
}

EAPI void
evas_array_hash_add(Evas_Array_Hash *hash, int key, int data)
{
   int hash_val;
   Evas_Array_Hash_El *el;

   hash_val = EAH_HASH(key);
   el = hash->buckets[hash_val];
   if (!el)
     {
	el = malloc(sizeof(Evas_Array_Hash_El));
	el->data_max = 4;
	el->data = malloc(sizeof(int) * el->data_max);
	el->data_count = 0;
	hash->buckets[hash_val] = el;
     }
   else if (el->data_count == el->data_max)
     {
	el->data_max *= 2;
	el->data = realloc(el->data, sizeof(int) * el->data_max);
     }

   el->data[el->data_count++] = key;
   el->data[el->data_count++] = data;
}

EAPI int
evas_array_hash_search(Evas_Array_Hash *hash, int key)
{
   int hash_val;
   Evas_Array_Hash_El *el;
   int low, high, i, val;

   hash_val = EAH_HASH(key);

   el = hash->buckets[hash_val];
   if (!el)
     return 0;

   /* Binary Search the bucket for key */
   low = 0;
   high = ( el->data_count / 2 ) - 1;

   while ( high >= low )
     {
	i = (high + low) / 2;

	val = el->data[i << 1];

	if (val == key)
	  return el->data[(i << 1) + 1];
	else if (val > key)
	  high = i - 1;
	else
	  low = i + 1;
     }
   return 0;
}

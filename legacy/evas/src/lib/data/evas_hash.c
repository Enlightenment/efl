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

/* FIXME: document this file next */
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

int
evas_hash_size(Evas_Hash *hash)
{
   if (!hash) return 0;
   return 256;
}

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

int
evas_hash_alloc_error(void)
{
   return _evas_hash_alloc_error;
}

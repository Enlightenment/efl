#include <string.h>
#include <stdio.h>

#include "eet_suite.h"

static char*
_eet_str_direct_alloc(const char *str)
{
   return (char*) str;
}

static void
_eet_str_direct_free(const char *str)
{
   /* FIXME: Use attribute unused */
   (void) str;
}

/* Internal list stuff. */
struct _Eet_List
{
   Eet_List *next;
   const void *data;
};
Eet_List*
eet_list_prepend(Eet_List *list, const void *data)
{
   Eet_List *new;

   new = malloc(sizeof (Eet_List));
   if (!new) return list;

   new->next = list;
   new->data = data;

   return new;
}
Eet_List*
eet_list_next(Eet_List *list)
{
   if (!list) return NULL;

   return list->next;
}
void*
eet_list_data(Eet_List *list)
{
   if (!list) return NULL;

   return (void*) list->data;
}
void
eet_list_free(Eet_List *list)
{
   while (list)
     {
	Eet_List *current = list;

	list = list->next;
	free(current);
     }
}

/* Internal hash stuff */
struct _Eet_Hash
{
   Eet_List *bucket[256];
};
typedef struct _Eet_Hash_Item Eet_Hash_Item;
struct _Eet_Hash_Item
{
   const void *data;
   char *key;
};
static inline int
_eet_hash_gen(const char *key)
{
   unsigned int hash_num = 5381;
   const unsigned char *ptr;

   if (!key) return 0;
   for (ptr = (unsigned char *)key; *ptr; ptr++)
     hash_num = (hash_num * 33) ^ *ptr;

   hash_num &= 0xff;
   return (int)hash_num;
}
void
eet_hash_foreach(const Eet_Hash *hash, int (*func) (const Eet_Hash *hash, const char *key, void *data, void *fdata), const void *fdata)
{
   int i;

   if (!hash) return ;

   for (i = 0; i < 256; ++i)
     {
	Eet_List *over;

	for (over = hash->bucket[i]; over; over = eet_list_next(over))
	  {
	     Eet_Hash_Item *item = eet_list_data(over);

	     if (!func(hash, item->key, (void*) item->data, (void*) fdata)) return ;
	  }
     }
}
Eet_Hash*
eet_hash_add(Eet_Hash *hash, const char *key, const void *data)
{
   Eet_Hash_Item *item;

   if (!hash) hash = calloc(1, sizeof (Eet_Hash));
   if (!hash) return NULL;

   item = malloc(sizeof (Eet_Hash_Item) + strlen(key) + 1);
   if (!item) return hash;

   item->data = data;
   item->key = (char*)(item + 1);
   strcpy(item->key, key);

   hash->bucket[_eet_hash_gen(key)] = eet_list_prepend(hash->bucket[_eet_hash_gen(key)], item);

   return hash;
}
void
eet_hash_free(Eet_Hash *hash)
{
   int i;

   if (!hash) return ;

   for (i = 0; i < 256; ++i)
     {
	Eet_List *over;

	for (over = hash->bucket[i]; over; over = eet_list_next(over))
	  free(eet_list_data(over));
	eet_list_free(hash->bucket[i]);
     }

   free(hash);
}

void
eet_test_setup_eddc(Eet_Data_Descriptor_Class *eddc)
{
   eddc->version = EET_DATA_DESCRIPTOR_CLASS_VERSION;
   eddc->func.mem_alloc = NULL;
   eddc->func.mem_free = NULL;
   eddc->func.str_alloc = NULL;
   eddc->func.str_free = NULL;
   eddc->func.list_next = (void*) eet_list_next;
   eddc->func.list_append = (void*) eet_list_prepend;
   eddc->func.list_data = (void*) eet_list_data;
   eddc->func.list_free = (void*) eet_list_free;
   eddc->func.hash_foreach = (void*) eet_hash_foreach;
   eddc->func.hash_add = (void*) eet_hash_add;
   eddc->func.hash_free = (void*) eet_hash_free;
   eddc->func.str_direct_alloc = (void*) _eet_str_direct_alloc;
   eddc->func.str_direct_free = (void*) _eet_str_direct_free;
}


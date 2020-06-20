#include "evas_common_private.h"

EAPI Generic_Cache*
generic_cache_new(void *user_data, Generic_Cache_Free func)
{
   Generic_Cache *cache;
   cache = calloc(1, sizeof(Generic_Cache));
   cache->hash = eina_hash_int32_new(NULL);
   cache->user_data = user_data;
   cache->free_func = func;
   return cache;
}

EAPI void
generic_cache_destroy(Generic_Cache *cache)
{
   Generic_Cache_Entry *entry;
   if (cache)
     {
        EINA_LIST_FREE(cache->lru_list, entry)
          {
             free(entry);
          }

        eina_hash_free(cache->hash);
        free(cache);
     }
}

EAPI void
generic_cache_dump(Generic_Cache *cache)
{
   Generic_Cache_Entry *entry;
   if (cache)
     {
        eina_hash_free_buckets(cache->hash);
        EINA_LIST_FREE(cache->lru_list, entry)
          {
             cache->free_func(cache->user_data, entry->data);
             free(entry);
          }
     }
}

EAPI void
generic_cache_data_set(Generic_Cache *cache, void *key, void *surface)
{
   Generic_Cache_Entry *entry = NULL;
   int count;

   entry = calloc(1, sizeof(Generic_Cache_Entry));
   entry->key = key;
   entry->data = surface;
   entry->ref = 1;
   eina_hash_add(cache->hash, &key, entry);
   cache->lru_list = eina_list_prepend(cache->lru_list, entry);
   count = eina_list_count(cache->lru_list);
   if (count > 50)
   {
      entry = eina_list_data_get(eina_list_last(cache->lru_list));
      // if its still being ref.
      if (entry->ref > 1) return;
      eina_hash_del(cache->hash, &entry->key, entry);
      cache->lru_list = eina_list_remove_list(cache->lru_list, eina_list_last(cache->lru_list));
      cache->free_func(cache->user_data, entry->data);
      free(entry);
   }
}

EAPI void *
generic_cache_data_get(Generic_Cache *cache, void *key)
{
   Generic_Cache_Entry *entry = NULL, *lru_data;
   Eina_List *l;

   entry =  eina_hash_find(cache->hash, &key);
   if (entry)
     {
        // update the ref
        entry->ref += 1;
        // promote in lru
        EINA_LIST_FOREACH(cache->lru_list, l, lru_data)
          {
            if (lru_data == entry)
              {
                 cache->lru_list = eina_list_promote_list(cache->lru_list, l);
                 break;
              }
          }
        return entry->data;
     }
   return NULL;
}

EAPI void
generic_cache_data_drop(Generic_Cache *cache, void *key)
{
   Generic_Cache_Entry *entry = NULL;

   entry =  eina_hash_find(cache->hash, &key);
   if (entry)
     {
        entry->ref -= 1;
        // if its still being ref.
        if (entry->ref) return;
        eina_hash_del(cache->hash, &entry->key, entry);
        // find and remove from lru list
        cache->lru_list = eina_list_remove(cache->lru_list, entry);
        cache->free_func(cache->user_data, entry->data);
        free(entry);
     }
}


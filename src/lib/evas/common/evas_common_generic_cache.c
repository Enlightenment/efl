#include "evas_common_private.h"

// Default memory budget for a size-aware generic cache.  Override with
// EVAS_SURFACE_CACHE_SIZE, in kilobytes.
#define GENERIC_CACHE_DEFAULT_BUDGET (8 * 1024 * 1024)

// Hard ceiling on entries, so a cache of tiny surfaces cannot grow the LRU
// list without bound.  Only reached when the byte budget has not been.
#define GENERIC_CACHE_MAX_ENTRIES 4096

static size_t
_generic_cache_budget(void)
{
   static size_t v = 0;
   const char *e;
   long kb;

   if (v) return v;

   e = getenv("EVAS_SURFACE_CACHE_SIZE");
   kb = e ? atol(e) : 0;
   v = (kb > 0) ? ((size_t)kb * 1024) : GENERIC_CACHE_DEFAULT_BUDGET;
   return v;
}

// Drop entries from the least-recently-used end until the cache is back
// inside its budget.  Entries still handed out (ref > 1) are skipped rather
// than aborting the sweep - stopping at the first one would let a single
// long-lived surface pin the cache above its budget forever.
static void
_generic_cache_trim(Generic_Cache *cache)
{
   Eina_List *l, *prev;
   int count = (int)eina_list_count(cache->lru_list);

   if (!cache->size_func && (count <= 50)) return;

   for (l = eina_list_last(cache->lru_list); l; l = prev)
     {
        Generic_Cache_Entry *entry = eina_list_data_get(l);

        if (cache->size_func)
          {
             if ((cache->bytes <= cache->budget) &&
                 count <= GENERIC_CACHE_MAX_ENTRIES) break;
          }
        else if (count <= 50) break;

        // Never evict the entry just inserted.  It is at the head, and when
        // the budget is smaller than a single surface the head is also the
        // tail - trimming it would free the very data the caller stored a
        // moment ago and is about to draw with.  A budget is advisory when
        // one item does not fit; a use-after-free is not.

        // Never evict the entry just inserted.  It is at the head, and when
        // the budget is smaller than a single surface the head is also the
        // tail - trimming it would free the very data the caller stored a
        // moment ago and is about to draw with.  A budget is advisory when
        // one item does not fit; a use-after-free is not.
        if (l == cache->lru_list) break;

        prev = eina_list_prev(l);
        if (!entry || (entry->ref > 1)) continue;

        eina_hash_del(cache->hash, &entry->key, entry);
        cache->lru_list = eina_list_remove_list(cache->lru_list, l);
        if (cache->bytes >= entry->size) cache->bytes -= entry->size;
        else cache->bytes = 0;
        count--;
        cache->free_func(cache->user_data, entry->data);
        free(entry);
     }
}

EVAS_API Generic_Cache*
generic_cache_new(void *user_data, Generic_Cache_Free func)
{
   Generic_Cache *cache;
   cache = calloc(1, sizeof(Generic_Cache));
   cache->hash = eina_hash_int32_new(NULL);
   cache->user_data = user_data;
   cache->free_func = func;
   cache->budget = _generic_cache_budget();
   return cache;
}

EVAS_API void
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

EVAS_API void
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
        cache->bytes = 0;
     }
}

EVAS_API void
generic_cache_size_func_set(Generic_Cache *cache, Generic_Cache_Size func)
{
   if (cache) cache->size_func = func;
}

EVAS_API void
generic_cache_data_set(Generic_Cache *cache, void *key, void *surface)
{
   Generic_Cache_Entry *entry = NULL;

   entry = calloc(1, sizeof(Generic_Cache_Entry));
   if (!entry) return;
   entry->key = key;
   entry->data = surface;
   entry->ref = 1;
   if (cache->size_func) entry->size = cache->size_func(cache->user_data, surface);
   eina_hash_add(cache->hash, &key, entry);
   cache->lru_list = eina_list_prepend(cache->lru_list, entry);
   entry->node = cache->lru_list;
   cache->bytes += entry->size;

   _generic_cache_trim(cache);
}

EVAS_API void *
generic_cache_data_get(Generic_Cache *cache, void *key)
{
   Generic_Cache_Entry *entry = NULL;

   entry =  eina_hash_find(cache->hash, &key);
   if (entry)
     {
        // update the ref
        entry->ref += 1;
        // promote in lru - the entry knows its own node, so this does not
        // walk the list.  It used to, on every lookup.
        if (entry->node)
          {
             cache->lru_list = eina_list_promote_list(cache->lru_list, entry->node);
             entry->node = cache->lru_list;
          }
        return entry->data;
     }
   return NULL;
}

EVAS_API void
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
        if (entry->node)
          cache->lru_list = eina_list_remove_list(cache->lru_list, entry->node);
        else
          cache->lru_list = eina_list_remove(cache->lru_list, entry);
        if (cache->bytes >= entry->size) cache->bytes -= entry->size;
        else cache->bytes = 0;
        cache->free_func(cache->user_data, entry->data);
        free(entry);
     }
}


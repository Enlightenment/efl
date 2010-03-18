#include "evas_common.h"
#include "evas_private.h"
#include "evas_cs.h"

/**
 * Retrieves if the system wants to share bitmaps using the server.
 * @return @c EINA_TRUE if wants, @c EINA_FALSE otherwise.
 * @ingroup Evas_Cserve
 */
EAPI Eina_Bool
evas_cserve_want_get(void)
{
#ifdef EVAS_CSERVE
   return evas_cserve_use_get();
#endif
   return 0;
}

/**
 * Retrieves if the system is connected to the server used to shae bitmaps.
 * @return @c EINA_TRUE if connected, @c EINA_FALSE otherwise.
 * @ingroup Evas_Cserve
 */
EAPI Eina_Bool
evas_cserve_connected_get(void)
{
#ifdef EVAS_CSERVE
   return evas_cserve_have_get();
#endif
   return 0;
}

/**
 * Retrieves if the system wants to share bitmaps using the server.
 * @param stats pointer to structure to fill with statistics about
 *        cache server.
 * @return @c EINA_TRUE if @p stats were filled with data,
 *         @c EINA_FALSE otherwise and @p stats is untouched.
 * @ingroup Evas_Cserve
 */
EAPI Eina_Bool
evas_cserve_stats_get(Evas_Cserve_Stats *stats)
{
#ifdef EVAS_CSERVE
   Op_Getstats_Reply st;

   if (!evas_cserve_raw_stats_get(&st)) return 0;
   if (!stats) return 1;
   stats->saved_memory = st.saved_memory;
   stats->wasted_memory = st.wasted_memory;
   stats->saved_memory_peak = st.saved_memory_peak;
   stats->wasted_memory_peak = st.wasted_memory_peak;
   stats->saved_time_image_header_load = st.saved_time_image_header_load;
   stats->saved_time_image_data_load = st.saved_time_image_data_load;
   // may expand this in future
   return 1;
#else
   return 0;
#endif
}

EAPI Eina_Bool
evas_cserve_image_cache_contents_get(Evas_Cserve_Image_Cache *cache)
{
#ifdef EVAS_CSERVE
   Op_Getinfo_Reply *info;
   unsigned char *p;
   int i, j;

   if (!(info = evas_cserve_raw_info_get())) return 0;
   if (!cache)
     {
        free(info);
        return 1;
     }
   cache->active.mem_total = info->active.mem_total;
   cache->active.count = info->active.count;
   cache->cached.mem_total = info->cached.mem_total;
   cache->cached.count = info->cached.count;
   cache->images = NULL;
   j = info->active.count + info->cached.count;
   p = (unsigned char *)info;
   p += sizeof(Op_Getinfo_Reply);
   for (i = 0; i < j; i++)
     {
        Evas_Cserve_Image *im;
        Op_Getinfo_Item it;
        char *file, *key;

        memcpy(&it, p, sizeof(Op_Getinfo_Item));
        file = (char*) (p + sizeof(Op_Getinfo_Item));
        key = file + strlen(file) + 1;
        im = calloc(1, sizeof(Evas_Cserve_Image));
        if (!im) continue;
        if (file[0] != 0)
          {
             file = (char *)eina_stringshare_add(file);
             if (!file)
               {
                  free(im);
                  continue;
               }
          }
        else
          file = NULL;
        if (key[0] != 0)
          {
             key = (char *)eina_stringshare_add(key);
             if (!key)
               {
                  if (file) eina_stringshare_del(file);
                  free(im);
                  continue;
               }
          }
        else key = NULL;
        cache->images = eina_list_append(cache->images, im);
        im->file = file;
        im->key = key;
        im->w = it.w;
        im->h = it.h;
        im->cached_time = it.cached_time;
        im->file_mod_time = it.file_mod_time;
        im->file_checked_time = it.file_checked_time;
        im->refcount = it.refcount;
        im->data_refcount = it.data_refcount;
        im->memory_footprint = it.memory_footprint;
        im->head_load_time = it.head_load_time;
        im->data_load_time = it.data_load_time;
        im->active = it.active;
        im->alpha = it.alpha;
        im->data_loaded = it.data_loaded;
        im->dead = it.dead;
        im->useless = it.useless;
     }
   free(info);
   return 1;
#else
   return 0;
#endif
}

EAPI void
evas_cserve_image_cache_contents_clean(Evas_Cserve_Image_Cache *cache)
{
#ifdef EVAS_CSERVE
   Evas_Cserve_Image *im;

   EINA_LIST_FREE(cache->images, im)
     {
        if (im->file) eina_stringshare_del(im->file);
        if (im->key) eina_stringshare_del(im->key);
        free(im);
     }
#endif
}

/**
 * Retrieves the current configuration of the server.
 * @param config where to store current server configuration.
 * @return @c EINA_TRUE if @p config were filled with data,
 *         @c EINA_FALSE otherwise and @p config is untouched.
 * @ingroup Evas_Cserve
 */
EAPI Eina_Bool
evas_cserve_config_get(Evas_Cserve_Config *config)
{
#ifdef EVAS_CSERVE
   Op_Getconfig_Reply conf;

   if (!evas_cserve_raw_config_get(&conf)) return 0;
   if (!config) return 1;
   config->cache_max_usage = conf.cache_max_usage;
   config->cache_item_timeout = conf.cache_item_timeout;
   config->cache_item_timeout_check = conf.cache_item_timeout_check;
   return 1;
#else
   return 0;
#endif
}

/**
 * Changes the configuration of the server.
 * @param config where to store current server configuration.
 * @return @c EINA_TRUE if @p config were successfully applied,
 *         @c EINA_FALSE otherwise.
 * @ingroup Evas_Cserve
 */
EAPI Eina_Bool
evas_cserve_config_set(const Evas_Cserve_Config *config)
{
#ifdef EVAS_CSERVE
   Op_Setconfig conf;

   if (!config) return 1;
   conf.cache_max_usage = config->cache_max_usage;
   conf.cache_item_timeout = config->cache_item_timeout;
   conf.cache_item_timeout_check = config->cache_item_timeout_check;
   return evas_cserve_raw_config_set(&conf);
#else
   return 0;
#endif
}

/**
 * Force system to disconnect from cache server.
 * @ingroup Evas_Cserve
 */
EAPI void
evas_cserve_disconnect(void)
{
#ifdef EVAS_CSERVE
   evas_cserve_discon();
#endif
}

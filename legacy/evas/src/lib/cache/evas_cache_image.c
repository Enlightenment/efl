#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "evas_common.h"
#include "evas_private.h"

//#define CACHEDUMP 1

#ifdef EVAS_CSERVE
// FIXME: cache server and threaded preload clash badly atm - disable
//#undef BUILD_ASYNC_PRELOAD
#endif

#ifdef BUILD_ASYNC_PRELOAD
typedef struct _Evas_Cache_Preload Evas_Cache_Preload;

struct _Evas_Cache_Preload
{
   EINA_INLIST;
   Image_Entry *ie;
};

static LK(engine_lock);
static LK(wakeup);
static int _evas_cache_mutex_init = 0;

static Eina_Condition cond_wakeup;

static void _evas_cache_image_entry_preload_remove(Image_Entry *ie, const void *target);
#endif

#define FREESTRC(Var)             \
   if (Var)                       \
   {                              \
      eina_stringshare_del(Var);  \
      Var = NULL;                 \
   }

static void _evas_cache_image_dirty_add(Image_Entry *im);
static void _evas_cache_image_dirty_del(Image_Entry *im);
static void _evas_cache_image_activ_add(Image_Entry *im);
static void _evas_cache_image_activ_del(Image_Entry *im);
static void _evas_cache_image_lru_add(Image_Entry *im);
static void _evas_cache_image_lru_del(Image_Entry *im);
static void _evas_cache_image_lru_nodata_add(Image_Entry *im);
static void _evas_cache_image_lru_nodata_del(Image_Entry *im);

static void
_evas_cache_image_dirty_add(Image_Entry *im)
{
   if (im->flags.dirty) return;
   _evas_cache_image_activ_del(im);
   _evas_cache_image_lru_del(im);
   _evas_cache_image_lru_nodata_del(im);
   im->flags.dirty = 1;
   im->flags.cached = 1;
#ifdef EVAS_FRAME_QUEUING
   LKL(im->cache->lock);
#endif
   im->cache->dirty = eina_inlist_prepend(im->cache->dirty, EINA_INLIST_GET(im));
#ifdef EVAS_FRAME_QUEUING
   LKU(im->cache->lock);
#endif
   if (im->cache_key)
     {
        eina_stringshare_del(im->cache_key);
        im->cache_key = NULL;
     }
}

static void
_evas_cache_image_dirty_del(Image_Entry *im)
{
   if (!im->flags.dirty) return;
   im->flags.dirty = 0;
   im->flags.cached = 0;
#ifdef EVAS_FRAME_QUEUING
   LKL(im->cache->lock);
#endif
   im->cache->dirty = eina_inlist_remove(im->cache->dirty, EINA_INLIST_GET(im));
#ifdef EVAS_FRAME_QUEUING
   LKU(im->cache->lock);
#endif
}

static void
_evas_cache_image_activ_add(Image_Entry *im)
{
   if (im->flags.activ) return;
   _evas_cache_image_dirty_del(im);
   _evas_cache_image_lru_del(im);
   _evas_cache_image_lru_nodata_del(im);
   if (!im->cache_key) return;
   im->flags.activ = 1;
   im->flags.cached = 1;
#ifdef EVAS_FRAME_QUEUING
   LKL(im->cache->lock);
#endif
   eina_hash_direct_add(im->cache->activ, im->cache_key, im);
#ifdef EVAS_FRAME_QUEUING
   LKU(im->cache->lock);
#endif
}

static void
_evas_cache_image_activ_del(Image_Entry *im)
{
   if (!im->flags.activ) return;
   if (!im->cache_key) return;
   im->flags.activ = 0;
   im->flags.cached = 0;
#ifdef EVAS_FRAME_QUEUING
   LKL(im->cache->lock);
#endif
   eina_hash_del(im->cache->activ, im->cache_key, im);
#ifdef EVAS_FRAME_QUEUING
   LKU(im->cache->lock);
#endif
}

static void
_evas_cache_image_lru_add(Image_Entry *im)
{
   if (im->flags.lru) return;
   _evas_cache_image_dirty_del(im);
   _evas_cache_image_activ_del(im);
   _evas_cache_image_lru_nodata_del(im); 
   if (!im->cache_key) return;
   im->flags.lru = 1;
   im->flags.cached = 1;
#ifdef EVAS_FRAME_QUEUING
   LKL(im->cache->lock);
#endif
   eina_hash_direct_add(im->cache->inactiv, im->cache_key, im);
   im->cache->lru = eina_inlist_prepend(im->cache->lru, EINA_INLIST_GET(im));
   im->cache->usage += im->cache->func.mem_size_get(im);
#ifdef EVAS_FRAME_QUEUING
   LKU(im->cache->lock);
#endif
}

static void
_evas_cache_image_lru_del(Image_Entry *im)
{
   if (!im->flags.lru) return;
   if (!im->cache_key) return;
   im->flags.lru = 0;
   im->flags.cached = 0;
#ifdef EVAS_FRAME_QUEUING
   LKL(im->cache->lock);
#endif
   eina_hash_del(im->cache->inactiv, im->cache_key, im);
   im->cache->lru = eina_inlist_remove(im->cache->lru, EINA_INLIST_GET(im));
   im->cache->usage -= im->cache->func.mem_size_get(im);
#ifdef EVAS_FRAME_QUEUING
   LKU(im->cache->lock);
#endif
}

static void
_evas_cache_image_lru_nodata_add(Image_Entry *im)
{
   if (im->flags.lru_nodata) return;
   _evas_cache_image_dirty_del(im);
   _evas_cache_image_activ_del(im);
   _evas_cache_image_lru_del(im);
   im->flags.lru = 1;
   im->flags.cached = 1;
#ifdef EVAS_FRAME_QUEUING
   LKL(im->cache->lock);
#endif
   im->cache->lru_nodata = eina_inlist_prepend(im->cache->lru_nodata, EINA_INLIST_GET(im));
#ifdef EVAS_FRAME_QUEUING
   LKU(im->cache->lock);
#endif
}

static void
_evas_cache_image_lru_nodata_del(Image_Entry *im)
{
   if (!im->flags.lru_nodata) return;
   im->flags.lru = 0;
   im->flags.cached = 0;
#ifdef EVAS_FRAME_QUEUING
   LKL(im->cache->lock);
#endif
   im->cache->lru_nodata = eina_inlist_remove(im->cache->lru_nodata, EINA_INLIST_GET(im));
#ifdef EVAS_FRAME_QUEUING
   LKU(im->cache->lock);
#endif
}

static void
_evas_cache_image_entry_delete(Evas_Cache_Image *cache, Image_Entry *ie)
{
   if (!ie) return;
   if (cache->func.debug) cache->func.debug("deleting", ie);
#ifdef BUILD_ASYNC_PRELOAD
   if (ie->flags.delete_me == 1) return;
   if (ie->preload)
     {
	ie->flags.delete_me = 1;
	_evas_cache_image_entry_preload_remove(ie, NULL);
	return;
     }
#endif

   _evas_cache_image_dirty_del(ie);
   _evas_cache_image_activ_del(ie);
   _evas_cache_image_lru_del(ie);
   _evas_cache_image_lru_nodata_del(ie);

   cache->func.destructor(ie);
   FREESTRC(ie->cache_key);
   FREESTRC(ie->file);
   FREESTRC(ie->key);
   ie->cache = NULL;
   cache->func.surface_delete(ie);

#ifdef BUILD_ASYNC_PRELOAD
   LKD(ie->lock);
   LKD(ie->lock_cancel);
#endif
#ifdef EVAS_FRAME_QUEUING
   LKD(ie->lock_references);
#endif
   cache->func.dealloc(ie);
}

static Eina_Bool
_timestamp_compare(Image_Timestamp *tstamp, struct stat *st)
{
   if (tstamp->mtime != st->st_mtime) return EINA_FALSE;
   if (tstamp->size != st->st_size) return EINA_FALSE;
   if (tstamp->ino != st->st_ino) return EINA_FALSE;
#ifdef _STAT_VER_LINUX
#if (defined __USE_MISC && defined st_mtime)
   if (tstamp->mtime_nsec != (unsigned long int)st->st_mtim.tv_nsec)
      return EINA_FALSE;
#else
   if (tstamp->mtime_nsec != (unsigned long int)st->st_mtimensec)
      return EINA_FALSE;
#endif
#endif
   return EINA_TRUE;
}

static void
_timestamp_build(Image_Timestamp *tstamp, struct stat *st)
{
   tstamp->mtime = st->st_mtime;
   tstamp->size = st->st_size;
   tstamp->ino = st->st_ino;
#ifdef _STAT_VER_LINUX
#if (defined __USE_MISC && defined st_mtime)
   tstamp->mtime_nsec = (unsigned long int)st->st_mtim.tv_nsec;
#else
   tstamp->mtime_nsec = (unsigned long int)st->st_mtimensec;
#endif
#endif
}

static Image_Entry *
_evas_cache_image_entry_new(Evas_Cache_Image *cache,
                            const char *hkey,
                            Image_Timestamp *tstamp,
                            const char *file,
                            const char *key,
                            RGBA_Image_Loadopts *lo,
                            int *error)
{
   Image_Entry  *ie;

   ie = cache->func.alloc();
   if (!ie)
     {
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	return NULL;
     }
   ie->cache = cache;
   if (hkey) ie->cache_key = eina_stringshare_add(hkey);
   ie->flags.need_data = 1;
   ie->space = EVAS_COLORSPACE_ARGB8888;
   ie->w = -1;
   ie->h = -1;
   ie->scale = 1;
   if (file) ie->file = eina_stringshare_add(file);
   if (key) ie->key = eina_stringshare_add(key);
   if (tstamp) ie->tstamp = *tstamp;
   else memset(&ie->tstamp, 0, sizeof(Image_Timestamp));

#ifdef EVAS_FRAME_QUEUING
   LKI(ie->lock_references);
#endif
#ifdef BUILD_ASYNC_PRELOAD
   LKI(ie->lock);
   LKI(ie->lock_cancel); 
#endif
   
   if (lo) ie->load_opts = *lo;
   if (ie->file)
     {
        *error = cache->func.constructor(ie);
        if (*error != EVAS_LOAD_ERROR_NONE)
          {
             _evas_cache_image_entry_delete(cache, ie);
             return NULL;
          }
     }
   if (cache->func.debug) cache->func.debug("build", ie);
   if (ie->cache_key) _evas_cache_image_activ_add(ie);
   else _evas_cache_image_dirty_add(ie);
   return ie;
}

static void
_evas_cache_image_entry_surface_alloc__locked(Evas_Cache_Image *cache,
					      Image_Entry *ie,
					      unsigned int wmin,
                                              unsigned int hmin)
{
   if ((ie->allocated.w == wmin) && (ie->allocated.h == hmin)) return;
   if (cache->func.surface_alloc(ie, wmin, hmin))
     {
        wmin = 0;
        hmin = 0;
     }
   ie->w = wmin;
   ie->h = hmin;
   ie->allocated.w = wmin;
   ie->allocated.h = hmin;
}

static void
_evas_cache_image_entry_surface_alloc(Evas_Cache_Image *cache,
                                      Image_Entry *ie, int w, int h)
{
   int wmin = w > 0 ? w : 1;
   int hmin = h > 0 ? h : 1;
#ifdef BUILD_ASYNC_PRELOAD
   LKL(engine_lock);
#endif
   _evas_cache_image_entry_surface_alloc__locked(cache, ie, wmin, hmin);
#ifdef BUILD_ASYNC_PRELOAD
   LKU(engine_lock);
#endif
}

#ifdef BUILD_ASYNC_PRELOAD
static void
_evas_cache_image_async_heavy(void *data)
{
   Evas_Cache_Image *cache;
   Image_Entry *current;
   int error;
   int pchannel;

   current = data;

   LKL(current->lock);
   pchannel = current->channel;
   current->channel++;
   cache = current->cache;

   if ((!current->flags.loaded) && 
       ((Evas_Image_Load_Func*) current->info.module)->threadable)
     {
	error = cache->func.load(current);
	if (cache->func.debug) cache->func.debug("load", current);
        current->load_error = error;
	if (error != EVAS_LOAD_ERROR_NONE)
	  {
	     current->flags.loaded = 0;
	     _evas_cache_image_entry_surface_alloc(cache, current,
						   current->w, current->h);
	  }
	else
          {
             current->flags.loaded = 1;
          }
     }
   current->channel = pchannel;
   // check the unload cancel flag
   LKL(current->lock_cancel);
   if (current->unload_cancel)
     {
        current->unload_cancel = EINA_FALSE;
        cache->func.surface_delete(current);
        current->flags.loaded = 0;
        current->flags.preload_done = 0;
     }
   LKU(current->lock_cancel);
   LKU(current->lock);
}

static void
_evas_cache_image_async_end(void *data)
{
   Image_Entry *ie = (Image_Entry *)data;
   Evas_Cache_Target *tmp;

   ie->cache->preload = eina_list_remove(ie->cache->preload, ie);
   ie->cache->pending = eina_list_remove(ie->cache->pending, ie);
   ie->preload = NULL;
   ie->flags.preload_done = ie->flags.loaded;
   while ((tmp = ie->targets))
     {
	evas_object_inform_call_image_preloaded((Evas_Object*) tmp->target);
	ie->targets = (Evas_Cache_Target *)
           eina_inlist_remove(EINA_INLIST_GET(ie->targets), 
                              EINA_INLIST_GET(ie->targets));
	free(tmp);
     }
}

static void
_evas_cache_image_async_cancel(void *data)
{
   Evas_Cache_Image *cache = NULL;
   Image_Entry *ie = (Image_Entry *)data;
   
   ie->preload = NULL;
   ie->cache->pending = eina_list_remove(ie->cache->pending, ie);
   if ((ie->flags.delete_me) || (ie->flags.dirty))
     {
	ie->flags.delete_me = 0;
	_evas_cache_image_entry_delete(ie->cache, ie);
	return;
     }
   if (ie->flags.loaded) _evas_cache_image_async_end(ie);
#ifdef EVAS_FRAME_QUEUING
   LKL(ie->lock_references);
#endif
   if (ie->references == 0)
     {
        _evas_cache_image_lru_add(ie);
        cache = ie->cache;
     }
#ifdef EVAS_FRAME_QUEUING
   LKU(ie->lock_references);
#endif
   if (cache) evas_cache_image_flush(cache);
}

// note - preload_add assumes a target is ONLY added ONCE to the image
// entry. make sure you only add once, or remove first, then add
static int
_evas_cache_image_entry_preload_add(Image_Entry *ie, const void *target)
{
   Evas_Cache_Target *tg;

   if (ie->flags.preload_done) return 0;

   tg = malloc(sizeof (Evas_Cache_Target));
   if (!tg) return 0;

   tg->target = target;
   ie->targets = (Evas_Cache_Target *)
      eina_inlist_append(EINA_INLIST_GET(ie->targets), EINA_INLIST_GET(tg));
   if (!ie->preload)
     {
        ie->cache->preload = eina_list_append(ie->cache->preload, ie);
        ie->flags.pending = 0;
        ie->preload = evas_preload_thread_run(_evas_cache_image_async_heavy,
                                              _evas_cache_image_async_end,
                                              _evas_cache_image_async_cancel,
                                              ie);
     }
   return 1;
}

static void
_evas_cache_image_entry_preload_remove(Image_Entry *ie, const void *target)
{
   if (target)
     {
	Evas_Cache_Target *tg;

	EINA_INLIST_FOREACH(ie->targets, tg)
	  {
	     if (tg->target == target)
	       {
 		  // FIXME: No callback when we cancel only for one target ?
		  ie->targets = (Evas_Cache_Target *)
                     eina_inlist_remove(EINA_INLIST_GET(ie->targets),
                                        EINA_INLIST_GET(tg));
		  free(tg);
		  break;
	       }
	  }
     }
   else
     {
	Evas_Cache_Target *tg;

	while (ie->targets)
	  {
	     tg = ie->targets;
	     ie->targets = (Evas_Cache_Target *)
                eina_inlist_remove(EINA_INLIST_GET(ie->targets),
                                   EINA_INLIST_GET(tg));
	     free(tg);
	  }
     }

   if ((!ie->targets) && (ie->preload) && (!ie->flags.pending))
     {
	ie->cache->preload = eina_list_remove(ie->cache->preload, ie);
	ie->cache->pending = eina_list_append(ie->cache->pending, ie);
	ie->flags.pending = 1;
	evas_preload_thread_cancel(ie->preload);
     }
}
#endif

EAPI int
evas_cache_image_usage_get(Evas_Cache_Image *cache)
{
   return cache->usage;
}

EAPI int
evas_cache_image_get(Evas_Cache_Image *cache)
{
   return cache->limit;
}

EAPI void
evas_cache_image_set(Evas_Cache_Image *cache, unsigned int limit)
{
#ifdef EVAS_FRAME_QUEUING
   LKL(cache->lock);
#endif
   if (cache->limit == limit)
     {
#ifdef EVAS_FRAME_QUEUING
        LKU(cache->lock);
#endif
        return;
     }
   cache->limit = limit;
#ifdef EVAS_FRAME_QUEUING
   LKU(cache->lock);
#endif
   evas_cache_image_flush(cache);
}

EAPI Evas_Cache_Image *
evas_cache_image_init(const Evas_Cache_Image_Func *cb)
{
   Evas_Cache_Image *cache;

#ifdef BUILD_ASYNC_PRELOAD
   if (_evas_cache_mutex_init++ == 0)
     {
        LKI(engine_lock);
        LKI(wakeup);
	eina_condition_new(&cond_wakeup, &wakeup);
     }
#endif

   cache = calloc(1, sizeof(Evas_Cache_Image));
   if (!cache) return NULL;
   cache->func = *cb;
   cache->inactiv = eina_hash_string_superfast_new(NULL);
   cache->activ = eina_hash_string_superfast_new(NULL);
   cache->references = 1;
#ifdef EVAS_FRAME_QUEUING
   LKI(cache->lock);
#endif
   return cache;
}

static Eina_Bool
_evas_cache_image_free_cb(__UNUSED__ const Eina_Hash *hash, __UNUSED__ const void *key, void *data, void *fdata)
{
   Eina_List **delete_list = fdata;
   *delete_list = eina_list_prepend(*delete_list, data);
   return EINA_TRUE;
}

EAPI void
evas_cache_image_shutdown(Evas_Cache_Image *cache)
{
   Eina_List *delete_list;
   Image_Entry  *im;

#ifdef EVAS_FRAME_QUEUING
   LKL(cache->lock);
#endif
   cache->references--;
   if (cache->references != 0)
     {
#ifdef EVAS_FRAME_QUEUING
        LKU(cache->lock);
#endif
        return;
     }
#ifdef EVAS_FRAME_QUEUING
   /* Release and destroy lock early ! */
   LKU(cache->lock);
   LKD(cache->lock);
#endif

#ifdef BUILD_ASYNC_PRELOAD
   EINA_LIST_FREE(cache->preload, im)
     {
	/* By doing that we are protecting us from destroying image when the cache is no longer available. */
	im->flags.delete_me = 1;
	_evas_cache_image_entry_preload_remove(im, NULL);
     }
   evas_async_events_process();
#endif
   while (cache->lru)
     {
        im = (Image_Entry *)cache->lru;
        _evas_cache_image_entry_delete(cache, im);
     }
   while (cache->lru_nodata)
     {
        im = (Image_Entry *)cache->lru_nodata;
        _evas_cache_image_entry_delete(cache, im);
     }
   /* This is mad, I am about to destroy image still alive, but we need to prevent leak. */
   while (cache->dirty)
     {
        im = (Image_Entry *)cache->dirty;
        _evas_cache_image_entry_delete(cache, im);
     }
   delete_list = NULL;
   eina_hash_foreach(cache->activ, _evas_cache_image_free_cb, &delete_list);
   while (delete_list)
     {
	_evas_cache_image_entry_delete(cache, eina_list_data_get(delete_list));
	delete_list = eina_list_remove_list(delete_list, delete_list);
     }

#ifdef BUILD_ASYNC_PRELOAD
   /* Now wait for all pending image to die */
   while (cache->pending)
     {
	evas_async_events_process();
	LKL(wakeup);
        // the lazy bum who did eain threads and converted this code
        // didn't bother to worry about Eina_Lock being a different type
        // to a pthread mutex.
	if (cache->pending) eina_condition_wait(&cond_wakeup);
	LKU(wakeup);
     }
#endif
   eina_hash_free(cache->activ);
   eina_hash_free(cache->inactiv);
   free(cache);

#ifdef BUILD_ASYNC_PRELOAD
   if (--_evas_cache_mutex_init == 0)
     {
	eina_condition_free(&cond_wakeup);
        LKD(engine_lock);
        LKD(wakeup);
     }
#endif
}

EAPI Image_Entry *
evas_cache_image_request(Evas_Cache_Image *cache, const char *file, 
                         const char *key, RGBA_Image_Loadopts *lo, int *error)
{
   const char           *ckey = "(null)";
   char                 *hkey;
   Image_Entry          *im;
   Evas_Image_Load_Opts  prevent = { 0, 0.0, 0, 0, 0, { 0, 0, 0, 0 }, EINA_FALSE };
   size_t                size;
   int                   stat_done = 0, stat_failed = 0;
   size_t                file_length;
   size_t                key_length;
   struct stat           st;
   Image_Timestamp       tstamp;

   if ((!file) || ((!file) && (!key)))
     {
	*error = EVAS_LOAD_ERROR_GENERIC;
	return NULL;
     }

   /* generate hkey from file+key+load opts */
   file_length = strlen(file);
   key_length = key ? strlen(key) : 6;
   size = file_length + key_length + 132;
   hkey = alloca(sizeof (char) * size);
   memcpy(hkey, file, file_length);
   size = file_length;
   memcpy(hkey + size, "//://", 5);
   size += 5;
   if (key) ckey = key;
   memcpy(hkey + size, ckey, key_length);
   size += key_length;
   if ((!lo) ||
       (lo &&
        (lo->scale_down_by == 0) &&
        (lo->dpi == 0.0) &&
        ((lo->w == 0) || (lo->h == 0)) &&
        ((lo->region.w == 0) || (lo->region.h == 0)) &&
	(lo->orientation == 0)
        ))
     {
        lo = &prevent;
     }
   else
     {
	memcpy(hkey + size, "//@/", 4);
	size += 4;
	size += eina_convert_xtoa(lo->scale_down_by, hkey + size);
	hkey[size] = '/';
	size += 1;
	size += eina_convert_dtoa(lo->dpi, hkey + size);
	hkey[size] = '/';
	size += 1;
	size += eina_convert_xtoa(lo->w, hkey + size);
	hkey[size] = 'x';
	size += 1;
	size += eina_convert_xtoa(lo->h, hkey + size);
	hkey[size] = '/';
	size += 1;
	size += eina_convert_xtoa(lo->region.x, hkey + size);
	hkey[size] = '+';
	size += 1;
	size += eina_convert_xtoa(lo->region.y, hkey + size);
	hkey[size] = '.';
	size += 1;
	size += eina_convert_xtoa(lo->region.w, hkey + size);
	hkey[size] = 'x';
	size += 1;
	size += eina_convert_xtoa(lo->region.h, hkey + size);

	if (lo->orientation)
	  {
             hkey[size] = '/';
             size += 1;
             hkey[size] = 'o';
             size += 1;
	  }
     }
   hkey[size] = '\0';

   /* find image by key in active hash */
#ifdef EVAS_FRAME_QUEUING
   LKL(cache->lock);
#endif
   im = eina_hash_find(cache->activ, hkey);
#ifdef EVAS_FRAME_QUEUING
   LKU(cache->lock);
#endif
   if (im)
     {
        int ok = 1;

        stat_done = 1;
        if (stat(file, &st) < 0)
           {
              stat_failed = 1;
              ok = 0;
           }
        else if (!_timestamp_compare(&(im->tstamp), &st)) ok = 0;
        if (ok) goto on_ok;
        /* image we found doesn't match what's on disk (stat info wise)
         * so dirty the active cache entry so we never find it again. this
         * also implicitly guarantees that we only have 1 active copy
         * of an image at a given key. we wither find it and keep re-reffing
         * it or we dirty it and get it out */
        _evas_cache_image_dirty_add(im);
        im = NULL;
     }

   /* find image by key in inactive/lru hash */
#ifdef EVAS_FRAME_QUEUING
   LKL(cache->lock);
#endif
   im = eina_hash_find(cache->inactiv, hkey);
#ifdef EVAS_FRAME_QUEUING
   LKU(cache->lock);
#endif
   if (im)
     {
        int ok = 1;

        if (!stat_done)
          {
             stat_done = 1;
             if (stat(file, &st) < 0)
               {
                  stat_failed = 1;
                  ok = 0;
               }
             else if (!_timestamp_compare(&(im->tstamp), &st)) ok = 0;
          }
        else if (!_timestamp_compare(&(im->tstamp), &st)) ok = 0;

        if (ok)
          {
             /* remove from lru and make it active again */
             _evas_cache_image_lru_del(im);
             _evas_cache_image_activ_add(im);
             goto on_ok;
          }
        /* as avtive cache find - if we match in lru and its invalid, dirty */
        _evas_cache_image_dirty_add(im);
        im = NULL;
     }
   if (stat_failed) goto on_stat_error;
   
   if (!stat_done)
     {
        if (stat(file, &st) < 0) goto on_stat_error;
     }
   _timestamp_build(&tstamp, &st);
   im = _evas_cache_image_entry_new(cache, hkey, &tstamp, file, key, 
                                    lo, error);
   if (!im) goto on_stat_error;
   if (cache->func.debug) cache->func.debug("request", im);

 on_ok:
   *error = EVAS_LOAD_ERROR_NONE;
#ifdef EVAS_FRAME_QUEUING
   LKL(im->lock_references);
#endif
   im->references++;
#ifdef EVAS_FRAME_QUEUING
   LKU(im->lock_references);
#endif
   return im;

 on_stat_error:
#ifndef _WIN32
   if ((errno == ENOENT) || (errno == ENOTDIR) ||
       (errno == ENAMETOOLONG) || (errno == ELOOP))
#else
   if (errno == ENOENT)
#endif
     *error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
#ifndef _WIN32
   else if ((errno == ENOMEM) || (errno == EOVERFLOW))
#else
   else if (errno == ENOMEM)
#endif
     *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
   else if (errno == EACCES)
     *error = EVAS_LOAD_ERROR_PERMISSION_DENIED;
   else
     *error = EVAS_LOAD_ERROR_GENERIC;

   if (im) _evas_cache_image_entry_delete(cache, im);
   return NULL;
}

EAPI void
evas_cache_image_drop(Image_Entry *im)
{
   Evas_Cache_Image *cache;
   int references;

#ifdef EVAS_FRAME_QUEUING
   LKL(im->lock_references);
#endif
   im->references--;
   if (im->references < 0) im->references = 0;
   references = im->references;
#ifdef EVAS_FRAME_QUEUING
   LKU(im->lock_references);
#endif
   
   cache = im->cache;

   if (references == 0)
     {
#ifdef EVAS_FRAME_QUEUING
        LKL(im->ref_fq_add);
        LKL(im->ref_fq_del);
        if (im->ref_fq[0] != im->ref_fq[1])
          {
             LKU(im->ref_fq_add);
             LKU(im->ref_fq_del);
             return;
          }
	LKU(im->ref_fq_add);
	LKU(im->ref_fq_del);
#endif

#ifdef BUILD_ASYNC_PRELOAD
	if (im->preload)
	  {
	     _evas_cache_image_entry_preload_remove(im, NULL);
	     return;
	  }
#endif

	if (im->flags.dirty)
	  {
	     _evas_cache_image_entry_delete(cache, im);
	     return;
	  }
        _evas_cache_image_lru_add(im);
	if (cache) evas_cache_image_flush(cache);
     }
}

EAPI void
evas_cache_image_data_not_needed(Image_Entry *im)
{
   Evas_Cache_Image *cache;
   int references;

   /* FIXME: no one uses this api... well evas_cache_engine_parent_not_needed()
    * does, but nothing uses that! */
   cache = im->cache;
#ifdef EVAS_FRAME_QUEUING
   LKL(im->lock_references);
#endif
   references = im->references;
#ifdef EVAS_FRAME_QUEUING
   LKU(im->lock_references);
#endif
   if (references > 1) return;
   if ((im->flags.dirty) || (!im->flags.need_data)) return;
   _evas_cache_image_lru_nodata_add(im);
}

EAPI Image_Entry *
evas_cache_image_dirty(Image_Entry *im, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
   Image_Entry *im_dirty = im;
   Evas_Cache_Image *cache;
   int references;

   cache = im->cache;
   if (!(im->flags.dirty))
     {
#ifdef EVAS_FRAME_QUEUING
        LKL(im->lock_references);
#endif
        references = im->references;
#ifdef EVAS_FRAME_QUEUING
        LKU(im->lock_references);
#endif
#ifndef EVAS_CSERVE
        // if ref 1 also copy if using shared cache as its read-only
        if (references == 1) im_dirty = im;
        else
#endif
          {
             int error;
             
             im_dirty = 
                evas_cache_image_copied_data(cache, im->w, im->h, 
                                             evas_cache_image_pixels(im), 
                                             im->flags.alpha, im->space);
             if (!im_dirty) goto on_error;
             if (cache->func.debug) cache->func.debug("dirty-src", im);
             error = cache->func.dirty(im_dirty, im);
             if (cache->func.debug) cache->func.debug("dirty-out", im_dirty);
#ifdef EVAS_FRAME_QUEUING
             LKL(im_dirty->lock_references);
#endif
             im_dirty->references = 1;
#ifdef EVAS_FRAME_QUEUING
             LKU(im_dirty->lock_references);
#endif
             evas_cache_image_drop(im);
          }
        _evas_cache_image_dirty_add(im_dirty);
     }
   
   if (cache->func.debug) cache->func.debug("dirty-region", im_dirty);
   if (cache->func.dirty_region)
      cache->func.dirty_region(im_dirty, x, y, w, h);
   return im_dirty;

on_error:
   if (im_dirty) _evas_cache_image_entry_delete(cache, im_dirty);
   evas_cache_image_drop(im);
   return NULL;
}

EAPI Image_Entry *
evas_cache_image_alone(Image_Entry *im)
{
   Evas_Cache_Image *cache;
   Image_Entry *im_dirty = im;
   int references;

   cache = im->cache;
#ifdef EVAS_FRAME_QUEUING
   LKL(im->lock_references);
#endif
   references = im->references;
#ifdef EVAS_FRAME_QUEUING
   LKU(im->lock_references);
#endif

   if (references <= 1)
     {
        if (!im->flags.dirty) _evas_cache_image_dirty_add(im);
     }
   else
     {
        int error;

        im_dirty = evas_cache_image_copied_data(cache, im->w, im->h, 
                                                evas_cache_image_pixels(im), 
                                                im->flags.alpha, 
                                                im->space);
        if (!im_dirty) goto on_error;
        if (cache->func.debug) cache->func.debug("dirty-src", im);
        error = cache->func.dirty(im_dirty, im);
        if (cache->func.debug) cache->func.debug("dirty-out", im_dirty);
#ifdef EVAS_FRAME_QUEUING
        LKL(im_dirty->lock_references);
#endif
        im_dirty->references = 1;
#ifdef EVAS_FRAME_QUEUING
        LKU(im_dirty->lock_references);
#endif
        evas_cache_image_drop(im);
     }
   return im_dirty;
   
on_error:
   if (im_dirty) _evas_cache_image_entry_delete(cache, im_dirty);
   evas_cache_image_drop(im);
   return NULL;
}

EAPI Image_Entry *
evas_cache_image_copied_data(Evas_Cache_Image *cache, 
                             unsigned int w, unsigned int h, 
                             DATA32 *image_data, int alpha, int cspace)
{
   Image_Entry *im;
   
   if ((cspace == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (cspace == EVAS_COLORSPACE_YCBCR422P709_PL) ||
       (cspace == EVAS_COLORSPACE_YCBCR422601_PL))
      w &= ~0x1;
   
   im = _evas_cache_image_entry_new(cache, NULL, NULL, NULL, NULL, NULL, NULL);
   if (!im) return NULL;
   im->space = cspace;
   im->flags.alpha = alpha;
   _evas_cache_image_entry_surface_alloc(cache, im, w, h);
   if (cache->func.copied_data(im, w, h, image_data, alpha, cspace) != 0)
     {
        _evas_cache_image_entry_delete(cache, im);
        return NULL;
     }
#ifdef EVAS_FRAME_QUEUING
   LKL(im->lock_references);
#endif
   im->references = 1;
#ifdef EVAS_FRAME_QUEUING
   LKU(im->lock_references);
#endif
   if (cache->func.debug) cache->func.debug("copied-data", im);
   return im;
}

EAPI Image_Entry *
evas_cache_image_data(Evas_Cache_Image *cache, unsigned int w, unsigned int h, DATA32 *image_data, int alpha, int cspace)
{
   Image_Entry *im;

   if ((cspace == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (cspace == EVAS_COLORSPACE_YCBCR422P709_PL) ||
       (cspace == EVAS_COLORSPACE_YCBCR422601_PL))
      w &= ~0x1;
   
   im = _evas_cache_image_entry_new(cache, NULL, NULL, NULL, NULL, NULL, NULL);
   if (!im) return NULL;
   im->w = w;
   im->h = h;
   im->flags.alpha = alpha;
   if (cache->func.data(im, w, h, image_data, alpha, cspace) != 0)
     {
        _evas_cache_image_entry_delete(cache, im);
        return NULL;
     }
#ifdef EVAS_FRAME_QUEUING
   LKL(im->lock_references);
#endif
   im->references = 1;
#ifdef EVAS_FRAME_QUEUING
   LKU(im->lock_references);
#endif
   if (cache->func.debug) cache->func.debug("data", im);
   return im;
}

EAPI void
evas_cache_image_surface_alloc(Image_Entry *im, unsigned int w, unsigned int h)
{
   Evas_Cache_Image *cache = im->cache;
   
   if ((im->space == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (im->space == EVAS_COLORSPACE_YCBCR422P709_PL) ||
       (im->space == EVAS_COLORSPACE_YCBCR422601_PL))
     w &= ~0x1;

   _evas_cache_image_entry_surface_alloc(cache, im, w, h);
   if (cache->func.debug) cache->func.debug("surface-alloc", im);
}

EAPI Image_Entry *
evas_cache_image_size_set(Image_Entry *im, unsigned int w, unsigned int h)
{
   Evas_Cache_Image *cache;
   Image_Entry *im2 = NULL;
   int error;

   if ((im->space == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (im->space == EVAS_COLORSPACE_YCBCR422P709_PL) ||
       (im->space == EVAS_COLORSPACE_YCBCR422601_PL))
     w &= ~0x1;
   if ((im->w == w) && (im->h == h)) return im;

   cache = im->cache;
   im2 = _evas_cache_image_entry_new(cache, NULL, NULL, NULL, NULL, NULL, &error);
   if (!im2) goto on_error;

   im2->flags.alpha = im->flags.alpha;
   im2->space = im->space;
   im2->load_opts = im->load_opts;
   _evas_cache_image_entry_surface_alloc(cache, im2, w, h);
   error = cache->func.size_set(im2, im, w, h);
   if (error != 0) goto on_error;
#ifdef EVAS_FRAME_QUEUING
   LKL(im2->lock_references);
#endif
   im2->references = 1;
#ifdef EVAS_FRAME_QUEUING
   LKU(im2->lock_references);
#endif
   evas_cache_image_drop(im);
   if (cache->func.debug) cache->func.debug("size_set", im2);
   return im2;

 on_error:
   if (im2) _evas_cache_image_entry_delete(cache, im2);
   evas_cache_image_drop(im);
   return NULL;
}

EAPI int
evas_cache_image_load_data(Image_Entry *im)
{
#ifdef BUILD_ASYNC_PRELOAD
   Eina_Bool preload = EINA_FALSE;
#endif
   int error = EVAS_LOAD_ERROR_NONE;

   if ((im->flags.loaded) && (!im->flags.animated)) return error;
#ifdef BUILD_ASYNC_PRELOAD
   if (im->preload)
     {
	preload = EINA_TRUE;
	if (!im->flags.pending)
	  {
	     im->cache->preload = eina_list_remove(im->cache->preload, im);
	     im->cache->pending = eina_list_append(im->cache->pending, im);
	     im->flags.pending = 1;
	     evas_preload_thread_cancel(im->preload);
	  }
	evas_async_events_process();
	LKL(wakeup);
	while (im->preload)
	  {
             eina_condition_wait(&cond_wakeup);
	     LKU(wakeup);
	     evas_async_events_process();
	     LKL(wakeup);
	  }
	LKU(wakeup);
     }
   
   if ((im->flags.loaded) && (!im->flags.animated)) return error;
   LKL(im->lock);
#endif
   im->flags.in_progress = EINA_TRUE;
   error = im->cache->func.load(im);
   im->flags.in_progress = EINA_FALSE;
#ifdef BUILD_ASYNC_PRELOAD
   LKU(im->lock);
#endif
   im->flags.loaded = 1;
   if (im->cache->func.debug) im->cache->func.debug("load", im);
   if (error != EVAS_LOAD_ERROR_NONE)
     {
        _evas_cache_image_entry_surface_alloc(im->cache, im, im->w, im->h);
        im->flags.loaded = 0;
     }
#ifdef BUILD_ASYNC_PRELOAD
   if (preload) _evas_cache_image_async_end(im);
#endif
   return error;
}

EAPI void
evas_cache_image_unload_data(Image_Entry *im)
{
   if (im->flags.in_progress) return;
   evas_cache_image_preload_cancel(im, NULL);
#ifdef BUILD_ASYNC_PRELOAD
   LKL(im->lock_cancel);
   if (LKT(im->lock) == EINA_FALSE) /* can't get image lock - busy async load */
     {
        im->unload_cancel = EINA_TRUE;
        LKU(im->lock_cancel);
        return;
     }
   LKU(im->lock_cancel);
#endif
   if ((!im->flags.loaded) || (!im->file) || (!im->info.module) || 
       (im->flags.dirty))
     {
#ifdef BUILD_ASYNC_PRELOAD
        LKU(im->lock);
#endif
	return;
     }
   im->cache->func.destructor(im);
#ifdef BUILD_ASYNC_PRELOAD
   LKU(im->lock);
#endif
   //FIXME: imagedataunload - inform owners
}

static Eina_Bool
_evas_cache_image_unload_cb(__UNUSED__ const Eina_Hash *hash, __UNUSED__ const void *key, void *data, __UNUSED__ void *fdata)
{
   evas_cache_image_unload_data(data);
   return EINA_TRUE;
}

EAPI void
evas_cache_image_unload_all(Evas_Cache_Image *cache)
{
   Image_Entry *im;

   EINA_INLIST_FOREACH(cache->lru, im) evas_cache_image_unload_data(im);
   EINA_INLIST_FOREACH(cache->lru_nodata, im) evas_cache_image_unload_data(im);
   eina_hash_foreach(cache->activ, _evas_cache_image_unload_cb, NULL);
   eina_hash_foreach(cache->inactiv, _evas_cache_image_unload_cb, NULL);
}

EAPI Eina_Bool
evas_cache_image_is_loaded(Image_Entry *im)
{
  if (im->flags.loaded) return EINA_TRUE;
  return EINA_FALSE;
}

EAPI void
evas_cache_image_preload_data(Image_Entry *im, const void *target)
{
#ifdef BUILD_ASYNC_PRELOAD
   RGBA_Image *img = (RGBA_Image *)im;
   
   if ((im->flags.loaded) && (img->image.data))
     {
	evas_object_inform_call_image_preloaded((Evas_Object *)target);
	return;
     }
   im->flags.loaded = 0;
   if (!_evas_cache_image_entry_preload_add(im, target))
      evas_object_inform_call_image_preloaded((Evas_Object *)target);
#else
   evas_cache_image_load_data(im);
   evas_object_inform_call_image_preloaded((Evas_Object *)target);
#endif
}

EAPI void
evas_cache_image_preload_cancel(Image_Entry *im, const void *target)
{
#ifdef BUILD_ASYNC_PRELOAD
   if (!target) return;
   _evas_cache_image_entry_preload_remove(im, target);
#else
   (void)im;
#endif
}

#ifdef CACHEDUMP
static int total = 0;

static void
_dump_img(Image_Entry *im, const char *type)
{
   total += im->cache->func.mem_size_get(im);
   printf("%s: %4i: %4ib, %4ix%4i alloc[%4ix%4i] [%s] [%s]\n",
          type,
          im->references,
          im->cache->func.mem_size_get(im),
          im->w, im->h, im->allocated.w, im->allocated.h,
          im->file, im->key);
}

static Eina_Bool
_dump_cache_active(__UNUSED__ const Eina_Hash *hash, __UNUSED__ const void *key, void *data, void *fdata __UNUSED__)
{
   Image_Entry *im = data;
   _dump_img(im, "ACTIVE");
   return EINA_TRUE;
}

static void
_dump_cache(Evas_Cache_Image *cache)
{
   Image_Entry *im;
   
   printf("--CACHE DUMP----------------------------------------------------\n");
   printf("cache: %ikb / %ikb\n",
          cache->usage / 1024,
          cache->limit / 1024);
   printf("................................................................\n");
   total = 0;
   EINA_INLIST_FOREACH(cache->lru_nodata, im)
      _dump_img(im, "NODATA");
   EINA_INLIST_FOREACH(cache->lru, im)
      _dump_img(im, "DATA  ");
   printf("tot: %i\n"
          "usg: %i\n",
          total,
          cache->usage);
   eina_hash_foreach(cache->activ, _dump_cache_active, NULL);
}
#endif

EAPI int
evas_cache_image_flush(Evas_Cache_Image *cache)
{
#ifdef CACHEDUMP
  _dump_cache(cache);
#endif  
   if (cache->limit == (unsigned int)-1) return -1;

   while ((cache->lru) && (cache->limit < (unsigned int)cache->usage))
     {
        Image_Entry *im;
        
        im = (Image_Entry *)cache->lru->last;
        _evas_cache_image_entry_delete(cache, im);
     }
   
   while ((cache->lru_nodata) && (cache->limit < (unsigned int)cache->usage))
     {
        Image_Entry *im;
        
        im = (Image_Entry *) cache->lru_nodata->last;
        _evas_cache_image_lru_nodata_del(im);
        cache->func.surface_delete(im);
        im->flags.loaded = 0;
     }

   return cache->usage;
}

EAPI Image_Entry *
evas_cache_image_empty(Evas_Cache_Image *cache)
{
   Image_Entry *im;

   im = _evas_cache_image_entry_new(cache, NULL, NULL, NULL, NULL, NULL, NULL);
   if (!im) return NULL;
#ifdef EVAS_FRAME_QUEUING
   LKL(im->lock_references);
#endif
   im->references = 1;
#ifdef EVAS_FRAME_QUEUING
   LKU(im->lock_references);
#endif
   return im;
}

EAPI void
evas_cache_image_colorspace(Image_Entry *im, int cspace)
{
   if (im->space == cspace) return;
   im->space = cspace;
   im->cache->func.color_space(im, cspace);
}

EAPI void *
evas_cache_private_from_image_entry_get(Image_Entry *im)
{
   return (void *)im->cache->data;
}

EAPI void *
evas_cache_private_get(Evas_Cache_Image *cache)
{
   return cache->data;
}

EAPI void
evas_cache_private_set(Evas_Cache_Image *cache, const void *data)
{
   cache->data = (void *)data;
}

EAPI DATA32 *
evas_cache_image_pixels(Image_Entry *im)
{
   return im->cache->func.surface_pixels(im);
}

EAPI void
evas_cache_image_wakeup(void)
{
#ifdef BUILD_ASYNC_PRELOAD
   if (_evas_cache_mutex_init > 0)
     eina_condition_broadcast(&cond_wakeup);
#endif
}

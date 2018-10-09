#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef _WIN32
# include <Evil.h>
#endif

#include "evas_common_private.h"
#include "evas_private.h"

//#define CACHEDUMP 1

typedef struct _Evas_Cache_Preload Evas_Cache_Preload;

struct _Evas_Cache_Preload
{
   EINA_INLIST;
   Image_Entry *ie;
};

static SLK(engine_lock);
static int _evas_cache_mutex_init = 0;

static const Image_Entry_Task dummy_task = { NULL, NULL, NULL };

static void _evas_cache_image_entry_preload_remove(Image_Entry *ie, const Eo *target);

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
   if (!im->cache) return;
   _evas_cache_image_activ_del(im);
   _evas_cache_image_lru_del(im);
   _evas_cache_image_lru_nodata_del(im);
   im->flags.dirty = 1;
   im->flags.cached = 1;
   im->cache->dirty = eina_inlist_prepend(im->cache->dirty, EINA_INLIST_GET(im));
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
   if (!im->cache) return;
   im->flags.dirty = 0;
   im->flags.cached = 0;
   im->cache->dirty = eina_inlist_remove(im->cache->dirty, EINA_INLIST_GET(im));  
}

static void
_evas_cache_image_activ_add(Image_Entry *im)
{
   if (im->flags.activ) return;
   if (!im->cache) return;
   _evas_cache_image_dirty_del(im);
   _evas_cache_image_lru_del(im);
   _evas_cache_image_lru_nodata_del(im);
   if (!im->cache_key) return;
   im->flags.activ = 1;
   im->flags.cached = 1;
   if (im->flags.given_mmap)
     eina_hash_direct_add(im->cache->mmap_activ, im->cache_key, im);
   else
     eina_hash_direct_add(im->cache->activ, im->cache_key, im);
}

static void
_evas_cache_image_activ_del(Image_Entry *im)
{
   if (!im->flags.activ) return;
   if (!im->cache_key) return;
   if (!im->cache) return;
   im->flags.activ = 0;
   im->flags.cached = 0;
   if (im->flags.given_mmap)
     eina_hash_del(im->cache->mmap_activ, im->cache_key, im);
   else
     eina_hash_del(im->cache->activ, im->cache_key, im);
}

static void
_evas_cache_image_lru_add(Image_Entry *im)
{
   if (im->flags.lru) return;
   if (!im->cache) return;
   _evas_cache_image_dirty_del(im);
   _evas_cache_image_activ_del(im);
   _evas_cache_image_lru_nodata_del(im);
   if (!im->cache_key) return;
   im->flags.lru = 1;
   im->flags.cached = 1;
   if (im->flags.given_mmap)
     eina_hash_direct_add(im->cache->mmap_inactiv, im->cache_key, im);
   else
     eina_hash_direct_add(im->cache->inactiv, im->cache_key, im);
   im->cache->lru = eina_inlist_prepend(im->cache->lru, EINA_INLIST_GET(im));
   im->cache->usage += im->cache->func.mem_size_get(im);
}

static void
_evas_cache_image_lru_del(Image_Entry *im)
{
   if (!im->flags.lru) return;
   if (!im->cache_key) return;
   if (!im->cache) return;
   im->flags.lru = 0;
   im->flags.cached = 0;
   if (im->flags.given_mmap)
     eina_hash_del(im->cache->mmap_inactiv, im->cache_key, im);
   else
     eina_hash_del(im->cache->inactiv, im->cache_key, im);
   im->cache->lru = eina_inlist_remove(im->cache->lru, EINA_INLIST_GET(im));
   im->cache->usage -= im->cache->func.mem_size_get(im);
}

static void
_evas_cache_image_lru_nodata_add(Image_Entry *im)
{
   if (im->flags.lru_nodata) return;
   if (!im->cache) return;
   _evas_cache_image_dirty_del(im);
   _evas_cache_image_activ_del(im);
   _evas_cache_image_lru_del(im);
   im->flags.lru = 1;
   im->flags.cached = 1;
   im->cache->lru_nodata = eina_inlist_prepend(im->cache->lru_nodata, EINA_INLIST_GET(im));
}

static void
_evas_cache_image_lru_nodata_del(Image_Entry *im)
{
   if (!im->flags.lru_nodata) return;
   if (!im->cache) return;
   im->flags.lru = 0;
   im->flags.cached = 0;
   im->cache->lru_nodata = eina_inlist_remove(im->cache->lru_nodata, EINA_INLIST_GET(im));
}

static void
_evas_cache_image_entry_delete(Evas_Cache_Image *cache, Image_Entry *ie)
{
   Image_Entry_Task *task;

   if (!ie) return;
   if (!ie->cache) return;
////   SLKL(ie->lock);
////   SLKU(ie->lock);
   if ((cache) && (cache->func.debug)) cache->func.debug("deleting", ie);
   if (ie->flags.delete_me == 1) return;
   if (ie->preload)
     {
        ie->flags.delete_me = 1;
        _evas_cache_image_entry_preload_remove(ie, NULL);
        return;
     }

   EINA_LIST_FREE(ie->tasks, task)
     if (task != &dummy_task) free(task);

   _evas_cache_image_dirty_del(ie);
   _evas_cache_image_activ_del(ie);
   _evas_cache_image_lru_del(ie);
   _evas_cache_image_lru_nodata_del(ie);

   cache->func.destructor(ie);
   FREESTRC(ie->cache_key);
   FREESTRC(ie->file);
   FREESTRC(ie->key);
   if (ie->f && ie->flags.given_mmap) eina_file_close(ie->f);
   ie->cache = NULL;
   cache->func.surface_delete(ie);

   SLKD(ie->lock);
   SLKD(ie->lock_cancel);
   SLKD(ie->lock_task);
   cache->func.dealloc(ie);
}

#if 0
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
#endif

static Image_Entry *
_evas_cache_image_entry_new(Evas_Cache_Image *cache,
                            const char *hkey,
                            Image_Timestamp *tstamp,
                            Eina_File *f,
                            const char *file,
                            const char *key,
                            Evas_Image_Load_Opts *lo,
                            int *error)
{
   Image_Entry  *ie;

   if (!cache)
     {
        *error = EVAS_LOAD_ERROR_GENERIC;
        return NULL;
     }

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
   ie->f = eina_file_dup(f);
   ie->loader_data = NULL;
   if (ie->f) ie->flags.given_mmap = EINA_TRUE;
   if (file) ie->file = eina_stringshare_add(file);
   if (key) ie->key = eina_stringshare_add(key);
   if (tstamp) ie->tstamp = *tstamp;
   else memset(&ie->tstamp, 0, sizeof(Image_Timestamp));

   SLKI(ie->lock);
   SLKI(ie->lock_cancel);
   SLKI(ie->lock_task);

   if (lo)
     {
        ie->load_opts = *lo;
     }
   if (ie->file || ie->f)
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
   if ((cache->func.surface_alloc(ie, wmin, hmin)) || (ie->load_failed))
     {
        wmin = 0;
        hmin = 0;
     }
   ie->w = wmin;
   ie->h = hmin;
}

static void
_evas_cache_image_entry_surface_alloc(Evas_Cache_Image *cache,
                                      Image_Entry *ie, int w, int h)
{
   int wmin = w > 0 ? w : 1;
   int hmin = h > 0 ? h : 1;
   SLKL(engine_lock);
   _evas_cache_image_entry_surface_alloc__locked(cache, ie, wmin, hmin);
   SLKU(engine_lock);
}

static Eina_Bool
evas_cache_image_cancelled(void *data)
{
   Image_Entry *current = data;
   Eina_Bool ret;

   evas_cache_image_ref(current);
   ret = evas_preload_thread_cancelled_is(current->preload);
   evas_cache_image_drop(current);
   return ret;
}

static void
_evas_cache_image_async_heavy(void *data)
{
   Evas_Cache_Image *cache;
   Image_Entry *current;
   Image_Entry_Task *task;
   int error;
   int pchannel;

   eina_thread_name_set(eina_thread_self(), "Evas-preload");

   current = data;

   if (!current->cache) return;
   SLKL(current->lock);
   pchannel = current->channel;
   current->channel++;
   cache = current->cache;

   if ((!current->flags.loaded) &&
       (current->info.loader) &&
       (current->info.loader->threadable))
     {
        evas_module_task_register(evas_cache_image_cancelled, current);
        error = cache->func.load(current);
        evas_module_task_unregister();

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

             SLKL(current->lock_task);
             EINA_LIST_FREE(current->tasks, task)
               {
                  if (task != &dummy_task)
                    {
                       task->cb((void *) task->engine_data, current, (void *) task->custom_data);
                       free(task);
                    }
               }
             SLKU(current->lock_task);
          }
     }
   current->channel = pchannel;
   // check the unload cancel flag
   SLKL(current->lock_cancel);
   if (current->flags.unload_cancel)
     {
        current->flags.unload_cancel = EINA_FALSE;
        cache->func.surface_delete(current);
        current->flags.loaded = 0;
        current->flags.preload_done = 0;
     }
   SLKU(current->lock_cancel);
   SLKU(current->lock);
}

static void
_evas_cache_image_preloaded_notify(Image_Entry *ie)
{
   Evas_Cache_Target *tmp;

   while ((tmp = ie->targets))
     {
        ie->targets = (Evas_Cache_Target *)
          eina_inlist_remove(EINA_INLIST_GET(ie->targets),
                             EINA_INLIST_GET(ie->targets));
        if (tmp->simple_cb)
          {
             if (!tmp->delete_me)
               {
                  tmp->simple_cb(tmp->simple_data);
               }
          }
        else
          {
             evas_object_inform_call_image_preloaded((Evas_Object*) tmp->target);
          }
        free(tmp);
     }
}

static void
_evas_cache_image_async_end(void *data)
{
   Image_Entry *ie = (Image_Entry *)data;
   Image_Entry_Task *task;

   if (!ie->cache) return;
   evas_cache_image_ref(ie);
   ie->cache->preload = eina_list_remove(ie->cache->preload, ie);
   ie->cache->pending = eina_list_remove(ie->cache->pending, ie);
   ie->preload = NULL;
   ie->flags.preload_done = ie->flags.loaded;
   ie->flags.updated_data = 1;

   ie->flags.preload_pending = 0;

   _evas_cache_image_preloaded_notify(ie);

   EINA_LIST_FREE(ie->tasks, task)
     if (task != &dummy_task) free(task);
   evas_cache_image_drop(ie);
}

static void
_evas_cache_image_async_cancel(void *data)
{
   Evas_Cache_Image *cache = NULL;
   Image_Entry *ie = (Image_Entry *)data;

   if (!ie->cache) return;
   evas_cache_image_ref(ie);
   ie->preload = NULL;
   ie->cache->pending = eina_list_remove(ie->cache->pending, ie);

   ie->flags.preload_pending = 0;

   if ((ie->flags.delete_me) || (ie->flags.dirty))
     {
        SLKL(engine_lock);
        ie->flags.delete_me = 0;
        SLKU(engine_lock);
        evas_cache_image_drop(ie);
        return;
     }
   SLKL(ie->lock_task);
   if (ie->targets)
     {
        ie->cache->preload = eina_list_append(ie->cache->preload, ie);
        ie->flags.pending = 0;
        ie->flags.preload_pending = 1;
        ie->preload = evas_preload_thread_run(_evas_cache_image_async_heavy,
                                              _evas_cache_image_async_end,
                                              _evas_cache_image_async_cancel,
                                              ie);
     }
   SLKU(ie->lock_task);
   if (ie->references == 0)
     {
        SLKL(engine_lock);
        _evas_cache_image_lru_add(ie);
        SLKU(engine_lock);
        cache = ie->cache;
     }
   if (ie->flags.loaded) _evas_cache_image_async_end(ie);
   evas_cache_image_drop(ie);
   if (cache) evas_cache_image_flush(cache);
}

// note - preload_add assumes a target is ONLY added ONCE to the image
// entry. make sure you only add once, or remove first, then add
static int
_evas_cache_image_entry_preload_add(Image_Entry *ie, const Eo *target,
                                    Evas_Engine_Thread_Task_Cb func, const void *engine_data, const void *custom_data)
{
   Evas_Cache_Target *tg;
   Image_Entry_Task *task;

   if (!ie->cache) return 0;
   evas_cache_image_ref(ie);
   if (ie->flags.preload_done)
     {
        evas_cache_image_drop(ie);
        return 0;
     }

   tg = calloc(1, sizeof(Evas_Cache_Target));
   if (!tg) return 0;
   tg->target = target;

   if (func == NULL && engine_data == NULL && custom_data == NULL)
     {
        task = (Image_Entry_Task*) &dummy_task;
     }
   else
     {
        task = malloc(sizeof (Image_Entry_Task));
        if (!task)
          {
             free(tg);
             return 0;
          }
        task->cb = func;
        task->engine_data = engine_data;
        task->custom_data = custom_data;
     }

   ie->targets = (Evas_Cache_Target *)
      eina_inlist_append(EINA_INLIST_GET(ie->targets), EINA_INLIST_GET(tg));
   SLKL(ie->lock_task);
   ie->tasks = eina_list_append(ie->tasks, task);
   SLKU(ie->lock_task);

   if (!ie->preload)
     {
        ie->cache->preload = eina_list_append(ie->cache->preload, ie);
        ie->flags.pending = 0;
        ie->flags.preload_pending = 1;
        ie->preload = evas_preload_thread_run(_evas_cache_image_async_heavy,
                                              _evas_cache_image_async_end,
                                              _evas_cache_image_async_cancel,
                                              ie);
     }
   evas_cache_image_drop(ie);
   return 1;
}

static void
_evas_cache_image_entry_preload_remove(Image_Entry *ie, const Eo *target)
{
   Evas_Cache_Target *tg;
   Eina_List *l;
   Image_Entry_Task *task;

   if (!ie->cache) return;
//   evas_cache_image_ref(ie);
   if (target)
     {
        SLKL(ie->lock_task);
        l = ie->tasks;
        EINA_INLIST_FOREACH(ie->targets, tg)
          {
             if (tg->target == target)
               {
                  // FIXME: No callback when we cancel only for one target ?
                  ie->targets = (Evas_Cache_Target *)
                     eina_inlist_remove(EINA_INLIST_GET(ie->targets),
                                        EINA_INLIST_GET(tg));

                  task = eina_list_data_get(l);
                  ie->tasks = eina_list_remove_list(ie->tasks, l);
                  if (task != &dummy_task) free(task);
                  free(tg);
                  break;
               }

             l = eina_list_next(l);
          }
        SLKU(ie->lock_task);
     }
   else
     {
        while (ie->targets)
          {
             tg = ie->targets;
             ie->targets = (Evas_Cache_Target *)
                eina_inlist_remove(EINA_INLIST_GET(ie->targets),
                                   EINA_INLIST_GET(tg));
             free(tg);
          }

        SLKL(ie->lock_task);
        EINA_LIST_FREE(ie->tasks, task)
          if (task != &dummy_task) free(task);
        SLKU(ie->lock_task);
     }

   if ((!ie->targets) && (ie->preload) && (!ie->flags.pending))
     {
        ie->cache->preload = eina_list_remove(ie->cache->preload, ie);
        ie->cache->pending = eina_list_append(ie->cache->pending, ie);
        ie->flags.pending = 1;
        evas_preload_thread_cancel(ie->preload);
     }
//   evas_cache_image_drop(ie);
}

EAPI int
evas_cache_image_usage_get(Evas_Cache_Image *cache)
{
   if (!cache) return 0;
   return cache->usage;
}

EAPI int
evas_cache_image_get(Evas_Cache_Image *cache)
{
   if (!cache) return 0;
   return cache->limit;
}

EAPI void
evas_cache_image_set(Evas_Cache_Image *cache, unsigned int limit)
{
   if (!cache) return;
   if (cache->limit == limit)
     {
        return;
     }
   cache->limit = limit;
   evas_cache_image_flush(cache);
}

EAPI Evas_Cache_Image *
evas_cache_image_init(const Evas_Cache_Image_Func *cb)
{
   Evas_Cache_Image *cache;

   if (_evas_cache_mutex_init++ == 0)
     {
        SLKI(engine_lock);
     }

   cache = calloc(1, sizeof(Evas_Cache_Image));
   if (!cache) return NULL;
   cache->func = *cb;
   cache->inactiv = eina_hash_string_superfast_new(NULL);
   cache->activ = eina_hash_string_superfast_new(NULL);
   cache->mmap_activ = eina_hash_string_superfast_new(NULL);
   cache->mmap_inactiv = eina_hash_string_superfast_new(NULL);
   cache->references = 1;
   return cache;
}

static Eina_Bool
_evas_cache_image_free_cb(EINA_UNUSED const Eina_Hash *hash, EINA_UNUSED const void *key, void *data, void *fdata)
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

   cache->references--;
   if (cache->references != 0)
     {
        return;
     }

   EINA_LIST_FREE(cache->preload, im)
     {
        /* By doing that we are protecting us from destroying image when the cache is no longer available. */
        im->flags.delete_me = 1;
        _evas_cache_image_entry_preload_remove(im, NULL);
     }
   evas_async_events_process();

   SLKL(engine_lock);
   EINA_INLIST_FREE(cache->lru, im)
     _evas_cache_image_entry_delete(cache, im);
   EINA_INLIST_FREE(cache->lru_nodata, im)
     _evas_cache_image_entry_delete(cache, im);

   /* This is mad, I am about to destroy image still alive, but we need to prevent leak. */
   while (cache->dirty)
     {
        im = (Image_Entry *)cache->dirty;
        _evas_cache_image_entry_delete(cache, im);
     }
   delete_list = NULL;

   eina_hash_foreach(cache->activ, _evas_cache_image_free_cb, &delete_list);
   eina_hash_foreach(cache->mmap_activ, _evas_cache_image_free_cb, &delete_list);
   while (delete_list)
     {
        _evas_cache_image_entry_delete(cache, eina_list_data_get(delete_list));
        delete_list = eina_list_remove_list(delete_list, delete_list);
     }
   SLKU(engine_lock);

   /* Now wait for all pending image to die */
   while (cache->pending)
     {
        im = eina_list_data_get(cache->pending);
        evas_preload_thread_cancel(im->preload);

        evas_async_events_process();
        if (!evas_preload_pthread_wait(im->preload, 1.0))
          {
             // We have waited long enough without reaction from that said
             // thread, remove it from pending list and silently continue
             // in the hope of an ok shutdown (but something is wrong).
             cache->pending = eina_list_remove_list(cache->pending, cache->pending);
             ERR("Could not stop decoding '%s' during shutdown.\n", im->file);
          }
     }

   eina_hash_free(cache->activ);
   eina_hash_free(cache->inactiv);
   eina_hash_free(cache->mmap_activ);
   eina_hash_free(cache->mmap_inactiv);
   free(cache);

   if (--_evas_cache_mutex_init == 0)
     {
        SLKD(engine_lock);
     }
}

static const Evas_Image_Load_Opts prevent = {
   {
        { 0, 0, 0, 0 },
        {
           0, 0, 0, 0,
           0, 0,
           0,
           0
        },
      0.0,
      0, 0,
      0,
      0,

      EINA_FALSE
   },
   EINA_FALSE
};

static size_t
_evas_cache_image_loadopts_append(char *hkey, Evas_Image_Load_Opts **plo)
{
   Evas_Image_Load_Opts *lo = *plo;
   size_t offset = 0;

   if ((!lo) ||
       (lo &&
           (lo->emile.scale_down_by == 0) &&
           (EINA_DBL_EQ(lo->emile.dpi, 0.0)) &&
           ((lo->emile.w == 0) || (lo->emile.h == 0)) &&
           ((lo->emile.region.w == 0) || (lo->emile.region.h == 0)) &&
           (lo->emile.orientation == 0)
       ))
     {
        *plo = (Evas_Image_Load_Opts*) &prevent;
     }
   else
     {
        memcpy(hkey, "//@/", 4);
        offset += 4;
        offset += eina_convert_xtoa(lo->emile.scale_down_by, hkey + offset);
        hkey[offset] = '/';
        offset += 1;
        offset += eina_convert_dtoa(lo->emile.dpi, hkey + offset);
        hkey[offset] = '/';
        offset += 1;
        offset += eina_convert_xtoa(lo->emile.w, hkey + offset);
        hkey[offset] = 'x';
        offset += 1;
        offset += eina_convert_xtoa(lo->emile.h, hkey + offset);
        hkey[offset] = '/';
        offset += 1;
        offset += eina_convert_xtoa(lo->emile.region.x, hkey + offset);
        hkey[offset] = '+';
        offset += 1;
        offset += eina_convert_xtoa(lo->emile.region.y, hkey + offset);
        hkey[offset] = '.';
        offset += 1;
        offset += eina_convert_xtoa(lo->emile.region.w, hkey + offset);
        hkey[offset] = 'x';
        offset += 1;
        offset += eina_convert_xtoa(lo->emile.region.h, hkey + offset);

        if (lo->emile.orientation)
          {
             hkey[offset] = '/';
             offset += 1;
             hkey[offset] = 'o';
             offset += 1;
          }
     }
   hkey[offset] = '\0';

   return offset;
}

EAPI Image_Entry *
evas_cache_image_mmap_request(Evas_Cache_Image *cache,
                              Eina_File *f, const char *key,
                              Evas_Image_Load_Opts *lo, int *error)
{
   const char  *hexcode = "0123456789abcdef";
   const char  *ckey = "(null)";
   char        *hkey;
   char        *pf;
   Image_Entry *im;
   size_t       size;
   size_t       file_length;
   size_t       key_length;
   unsigned int i;

   // FIXME: In the long term we should certainly merge both mmap and filename path
   //  by just using the mmap path. But for the time being, let's just have two path
   //  as it is unlikely to really have an impact on real world application
   if (!f)
     {
        *error = EVAS_LOAD_ERROR_GENERIC;
        return NULL;
     }

   /* generate hkey from file+key+load opts */
   file_length = sizeof (Eina_File*) * 2;
   key_length = key ? strlen(key) : 6;
   size = file_length + key_length + 132;
   hkey = alloca(sizeof (char) * size);
   pf = (char*) &f;
   for (size = 0, i = 0; i < sizeof (Eina_File*); i++)
     {
        hkey[size++] = hexcode[(pf[i] & 0xF0) >> 4];
        hkey[size++] = hexcode[(pf[i] & 0x0F)];
     }
   memcpy(hkey + size, "//://", 5);
   size += 5;
   if (key) ckey = key;
   memcpy(hkey + size, ckey, key_length);
   size += key_length;
   size += _evas_cache_image_loadopts_append(hkey + size, &lo);


   /* find image by key in active mmap hash */
   SLKL(engine_lock);
   im = eina_hash_find(cache->mmap_activ, hkey);
   if ((im) && (!im->load_failed)) goto on_ok;
   else if ((im) && (im->load_failed))
     {
        _evas_cache_image_dirty_add(im);
        im = NULL;
     }

   /* find image by key in inactive/lru hash */
   im = eina_hash_find(cache->mmap_inactiv, hkey);
   if ((im) && (!im->load_failed))
     {
        _evas_cache_image_lru_del(im);
        _evas_cache_image_activ_add(im);
        goto on_ok;
     }
   else if ((im) && (im->load_failed))
     {
        /* as active cache find - if we match in lru and its invalid, dirty */
        _evas_cache_image_dirty_add(im);
        /* this image never used, so it have to be deleted */
        _evas_cache_image_entry_delete(cache, im);
        im = NULL;
     }

   im = _evas_cache_image_entry_new(cache, hkey, NULL, f, NULL, key, lo, error);
   if (!im)
     {
        SLKU(engine_lock);
        return NULL;
     }

 on_ok:
   *error = EVAS_LOAD_ERROR_NONE;
////   SLKL(im->lock);
   im->references++;
////   SLKU(im->lock);
   SLKU(engine_lock);
   return im;
}

EAPI void
evas_cache_image_ref(Image_Entry *im)
{
   SLKL(engine_lock);
////   SLKL(im->lock);
   im->references++;
////   SLKU(im->lock);
   SLKU(engine_lock);
}

EAPI void
evas_cache_image_drop(Image_Entry *im)
{
   Evas_Cache_Image *cache;
   int references;

   if (!im->cache) return;
   SLKL(engine_lock);
////   SLKL(im->lock);
   im->references--;
   if (im->references < 0) im->references = 0;
   references = im->references;
////   SLKU(im->lock);
   SLKU(engine_lock);

   cache = im->cache;

   if (references == 0)
     {
        if (im->preload)
          {
             _evas_cache_image_entry_preload_remove(im, NULL);
             return;
          }
        if ((im->flags.dirty) || (im->load_failed))
          {
             SLKL(engine_lock);
             _evas_cache_image_entry_delete(cache, im);
             SLKU(engine_lock);
             return;
          }
        if (cache)
          {
             SLKL(engine_lock);
             _evas_cache_image_lru_add(im);
             SLKU(engine_lock);
             evas_cache_image_flush(cache);
          }
     }
}

EAPI void
evas_cache_image_data_not_needed(Image_Entry *im)
{
   int references;

   references = im->references;
   if (references > 1) return;
   if ((im->flags.dirty) || (!im->flags.need_data)) return;
   SLKL(engine_lock);
   _evas_cache_image_lru_nodata_add(im);
   SLKU(engine_lock);
}

EAPI Image_Entry *
evas_cache_image_dirty(Image_Entry *im, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
   Image_Entry *im_dirty = im;
   Evas_Cache_Image *cache;

   if (!im->cache) return NULL;
   cache = im->cache;
   if (!(im->flags.dirty))
     {
        if (im->references == 1) im_dirty = im;
        else
          {
             im_dirty =
                evas_cache_image_copied_data(cache, im->w, im->h,
                                             evas_cache_image_pixels(im),
                                             im->flags.alpha, im->space);
             if (!im_dirty) goto on_error;
             if (cache->func.debug) cache->func.debug("dirty-src", im);
             cache->func.dirty(im_dirty, im);
             if (cache->func.debug) cache->func.debug("dirty-out", im_dirty);
             im_dirty->references = 1;
             evas_cache_image_drop(im);
          }
        SLKL(engine_lock);
        _evas_cache_image_dirty_add(im_dirty);
        SLKU(engine_lock);
     }

   if (cache->func.debug) cache->func.debug("dirty-region", im_dirty);
   if (cache->func.dirty_region)
     cache->func.dirty_region(im_dirty, x, y, w, h);
   return im_dirty;

on_error:
   evas_cache_image_drop(im);
   return NULL;
}

EAPI Image_Entry *
evas_cache_image_alone(Image_Entry *im)
{
   Evas_Cache_Image *cache;
   Image_Entry *im_dirty = im;
   int references;

   if (!im->cache) return NULL;
   cache = im->cache;
   references = im->references;

   if (references <= 1)
     {
        SLKL(engine_lock);
        if (!im->flags.dirty) _evas_cache_image_dirty_add(im);
        SLKU(engine_lock);
     }
   else
     {
        im_dirty = evas_cache_image_copied_data(cache, im->w, im->h, 
                                                evas_cache_image_pixels(im), 
                                                im->flags.alpha, 
                                                im->space);
        if (!im_dirty) goto on_error;
        if (cache->func.debug) cache->func.debug("dirty-src", im);
        cache->func.dirty(im_dirty, im);
        if (cache->func.debug) cache->func.debug("dirty-out", im_dirty);
        im_dirty->references = 1;
        evas_cache_image_drop(im);
     }
   return im_dirty;

on_error:
   evas_cache_image_drop(im);
   return NULL;
}

EAPI Image_Entry *
evas_cache_image_copied_data(Evas_Cache_Image *cache, 
                             unsigned int w, unsigned int h, 
                             DATA32 *image_data, int alpha,
                             Evas_Colorspace cspace)
{
   int err;
   Image_Entry *im;

   if (!cache) return NULL;
   if ((cspace == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (cspace == EVAS_COLORSPACE_YCBCR422P709_PL) ||
       (cspace == EVAS_COLORSPACE_YCBCR422601_PL))
     w &= ~0x1;

   SLKL(engine_lock);
   im = _evas_cache_image_entry_new(cache, NULL, NULL, NULL, NULL, NULL, NULL, &err);
   SLKU(engine_lock);
   if (!im) return NULL;
   im->space = cspace;
   im->flags.alpha = alpha;
   _evas_cache_image_entry_surface_alloc(cache, im, w, h);
   if (cache->func.copied_data(im, w, h, image_data, alpha, cspace) != 0)
     {
        SLKL(engine_lock);
        _evas_cache_image_entry_delete(cache, im);
        SLKU(engine_lock);
        return NULL;
     }
   im->references = 1;
   im->flags.loaded = EINA_TRUE;
   if (cache->func.debug) cache->func.debug("copied-data", im);
   return im;
}

EAPI Image_Entry *
evas_cache_image_data(Evas_Cache_Image *cache, unsigned int w, unsigned int h,
                      DATA32 *image_data, int alpha, Evas_Colorspace cspace)
{
   int err;
   Image_Entry *im;

   if (!cache) return NULL;
   if ((cspace == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (cspace == EVAS_COLORSPACE_YCBCR422P709_PL) ||
       (cspace == EVAS_COLORSPACE_YCBCR422601_PL))
     w &= ~0x1;

   SLKL(engine_lock);
   im = _evas_cache_image_entry_new(cache, NULL, NULL, NULL, NULL, NULL, NULL, &err);
   SLKU(engine_lock);
   if (!im) return NULL;
   im->w = w;
   im->h = h;
   im->flags.alpha = alpha;
   if (cache->func.data(im, w, h, image_data, alpha, cspace) != 0)
     {
        SLKL(engine_lock);
        _evas_cache_image_entry_delete(cache, im);
        SLKU(engine_lock);
        return NULL;
     }
   im->references = 1;
   im->flags.loaded = EINA_TRUE;
   if (cache->func.debug) cache->func.debug("data", im);
   return im;
}

EAPI void
evas_cache_image_surface_alloc(Image_Entry *im, unsigned int w, unsigned int h)
{
   Evas_Cache_Image *cache = im->cache;

   if (!im->cache) return;
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

   if (!im->cache) return im;
   evas_cache_image_ref(im);
   if ((im->space == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (im->space == EVAS_COLORSPACE_YCBCR422P709_PL) ||
       (im->space == EVAS_COLORSPACE_YCBCR422601_PL))
     w &= ~0x1;
   if ((im->w == w) && (im->h == h))
     {
        evas_cache_image_drop(im);
        return im;
     }

   cache = im->cache;
   SLKL(engine_lock);
   im2 = _evas_cache_image_entry_new(cache, NULL, NULL, NULL, NULL, NULL, NULL, &error);
   SLKU(engine_lock);
   if (!im2) goto on_error;

   im2->flags.alpha = im->flags.alpha;
   im2->space = im->space;
   im2->load_opts = im->load_opts;
   _evas_cache_image_entry_surface_alloc(cache, im2, w, h);
   error = cache->func.size_set(im2, im, w, h);
   if (error != 0) goto on_error;
   im2->references = 1;
   im2->flags.loaded = EINA_TRUE;
   evas_cache_image_drop(im);
   if (cache->func.debug) cache->func.debug("size_set", im2);
   evas_cache_image_drop(im);
   return im2;

on_error:
   SLKL(engine_lock);
   if (im2) _evas_cache_image_entry_delete(cache, im2);
   SLKU(engine_lock);
   evas_cache_image_drop(im);
   evas_cache_image_drop(im);
   return NULL;
}

EAPI int
evas_cache_image_load_data(Image_Entry *im)
{
   Eina_Bool preload = EINA_FALSE;
   int error = EVAS_LOAD_ERROR_NONE;

   if (!im->cache) return error;
   evas_cache_image_ref(im);
   if ((im->flags.loaded) && (!im->animated.animated))
     {
        evas_cache_image_drop(im);
        return error;
     }
   evas_common_rgba_pending_unloads_remove(im);
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
        evas_preload_pthread_wait(im->preload, 0.01);

        while (im->flags.preload_pending)
          {
             evas_async_events_process();
             evas_preload_pthread_wait(im->preload, 0.1);
          }
     }

   if ((im->flags.loaded) && (!im->animated.animated))
     {
        evas_cache_image_drop(im);
        return error;
     }

   SLKL(im->lock);
   im->flags.in_progress = EINA_TRUE;
   error = im->cache->func.load(im);
   im->flags.in_progress = EINA_FALSE;
   SLKU(im->lock);

   im->flags.loaded = 1;
   if (im->cache->func.debug) im->cache->func.debug("load", im);
   if (error != EVAS_LOAD_ERROR_NONE)
     {
        _evas_cache_image_entry_surface_alloc(im->cache, im, im->w, im->h);
        im->flags.loaded = 0;
     }
   if (preload) _evas_cache_image_async_end(im);
   evas_cache_image_drop(im);
   return error;
}

EAPI void
evas_cache_image_unload_data(Image_Entry *im)
{
   if (!im->cache) return;
   evas_cache_image_ref(im);
   if (im->flags.in_progress)
     {
        evas_cache_image_drop(im);
        return;
     }
   evas_cache_image_preload_cancel(im, NULL);

   if (SLKT(im->lock) == EINA_FALSE) /* can't get image lock - busy async load */
     {
        SLKL(im->lock_cancel);
        im->flags.unload_cancel = EINA_TRUE;
        SLKU(im->lock_cancel);
        evas_cache_image_drop(im);
        return;
     }

   SLKL(im->lock_cancel);
   if ((!im->flags.loaded) || (!im->file && !im->f) || (!im->info.module) || 
       (im->flags.dirty))
     {
        SLKU(im->lock_cancel);
        SLKU(im->lock);
        evas_cache_image_drop(im);
        return;
     }
   SLKU(im->lock_cancel);
   im->cache->func.destructor(im);
   SLKU(im->lock);
   evas_cache_image_drop(im);
   //FIXME: imagedataunload - inform owners
}

static Eina_Bool
_evas_cache_image_unload_cb(EINA_UNUSED const Eina_Hash *hash, EINA_UNUSED const void *key, void *data, EINA_UNUSED void *fdata)
{
   evas_cache_image_unload_data(data);
   return EINA_TRUE;
}

EAPI void
evas_cache_image_unload_all(Evas_Cache_Image *cache)
{
   Image_Entry *im;

   if (!cache) return;
// _evas_cache_image_unload_cb -> evas_cache_image_unload_data -> evas_cache_image_ref
//  deadlock
//////   SLKL(engine_lock);
   EINA_INLIST_FOREACH(cache->lru, im) evas_cache_image_unload_data(im);
   EINA_INLIST_FOREACH(cache->lru_nodata, im) evas_cache_image_unload_data(im);
   eina_hash_foreach(cache->activ, _evas_cache_image_unload_cb, NULL);
   eina_hash_foreach(cache->inactiv, _evas_cache_image_unload_cb, NULL);
//////   SLKU(engine_lock);
}

static int async_frozen = 0;

EAPI int
evas_cache_async_frozen_get(void)
{
   return async_frozen;
}

EAPI void
evas_cache_async_freeze(void)
{
   async_frozen++;
}

EAPI void
evas_cache_async_thaw(void)
{
   async_frozen--;
}

EAPI Eina_Bool
evas_cache_image_is_loaded(Image_Entry *im)
{
   if (im->flags.loaded) return EINA_TRUE;
   return EINA_FALSE;
}

EAPI void
evas_cache_image_preload_data(Image_Entry *im, const Eo *target,
                              Evas_Engine_Thread_Task_Cb func, const void *engine_data, const void *custom_data)
{
   RGBA_Image *img = (RGBA_Image *)im;

   if (!im->cache) return;
   evas_cache_image_ref(im);
   if (((int)im->w > 0) && ((int)im->h > 0) &&
       (((im->flags.loaded) && (img->image.data)) ||
        (im->flags.textured && !im->flags.updated_data)))
     {
        _evas_cache_image_preloaded_notify(im);
        evas_object_inform_call_image_preloaded((Evas_Object*)target);
        evas_cache_image_drop(im);
        return;
     }
   im->flags.loaded = 0;
   if (!_evas_cache_image_entry_preload_add(im, target, func, engine_data, custom_data))
     evas_object_inform_call_image_preloaded((Evas_Object*) target);
   evas_cache_image_drop(im);
}

EAPI void
evas_cache_image_preload_cancel(Image_Entry *im, const Eo *target)
{
   if (!target) return;
   evas_cache_image_ref(im);
   _evas_cache_image_entry_preload_remove(im, target);
   evas_cache_image_drop(im);
}

#ifdef CACHEDUMP
static int total = 0;

static void
_dump_img(Image_Entry *im, const char *type)
{
   if (!im->cache) return;
   total += im->cache->func.mem_size_get(im);
   printf("%s: %4i: %4ib, %4ix%4i alloc[%4ix%4i] [%s] [%s]\n",
          type,
          im->references,
          im->cache->func.mem_size_get(im),
          im->w, im->h, im->allocated.w, im->allocated.h,
          im->f ? eina_file_filename_get(im->f) : im->file, im->key);
}

static Eina_Bool
_dump_cache_active(EINA_UNUSED const Eina_Hash *hash, EINA_UNUSED const void *key, void *data, void *fdata EINA_UNUSED)
{
   Image_Entry *im = data;
   _dump_img(im, "ACTIVE");
   return EINA_TRUE;
}

static void
_dump_cache(Evas_Cache_Image *cache)
{
   Image_Entry *im;

   if (!cache) return;
   printf("--CACHE DUMP----------------------------------------------------\n");
   printf("cache: %ikb / %ikb\n",
          cache->usage / 1024,
          cache->limit / 1024);
   printf("................................................................\n");
   total = 0;
   SLKL(engine_lock);
   EINA_INLIST_FOREACH(cache->lru_nodata, im)
      _dump_img(im, "NODATA");
   EINA_INLIST_FOREACH(cache->lru, im)
      _dump_img(im, "DATA  ");
   printf("tot: %i\n"
          "usg: %i\n",
          total,
          cache->usage);
   eina_hash_foreach(cache->activ, _dump_cache_active, NULL);
   SLKU(engine_lock);
}
#endif

EAPI int
evas_cache_image_flush(Evas_Cache_Image *cache)
{
   if (!cache) return 0;
#ifdef CACHEDUMP
   _dump_cache(cache);
#endif  
   if (cache->limit == (unsigned int)-1) return -1;

   SLKL(engine_lock);
   while ((cache->lru) && (cache->limit < (unsigned int)cache->usage))
     {
        Image_Entry *im;

        im = (Image_Entry *)cache->lru->last;
        if (!im) im = (Image_Entry *)cache->lru;
        _evas_cache_image_entry_delete(cache, im);
     }

   while ((cache->lru_nodata) && (cache->limit < (unsigned int)cache->usage))
     {
        Image_Entry *im;

        im = (Image_Entry *)cache->lru_nodata->last;
        if (!im) im = (Image_Entry *)cache->lru_nodata;
        _evas_cache_image_lru_nodata_del(im);
        cache->func.surface_delete(im);
        im->flags.loaded = 0;
     }
   SLKU(engine_lock);

   return cache->usage;
}

EAPI Image_Entry *
evas_cache_image_empty(Evas_Cache_Image *cache)
{
   int err;
   Image_Entry *im;

   if (!cache) return NULL;
   SLKL(engine_lock);
   im = _evas_cache_image_entry_new(cache, NULL, NULL, NULL, NULL, NULL, NULL, &err);
   SLKU(engine_lock);
   if (!im) return NULL;
   im->references = 1;
   return im;
}

EAPI void
evas_cache_image_colorspace(Image_Entry *im, Evas_Colorspace cspace)
{
   if (!im->cache) return;
   evas_cache_image_ref(im);
   if (im->space == cspace) goto done;
   im->space = cspace;
   if (!im->cache) goto done;
   im->cache->func.color_space(im, cspace);
done:
   evas_cache_image_drop(im);
}

EAPI void *
evas_cache_private_from_image_entry_get(Image_Entry *im)
{
   void *data;
   if (!im->cache) return NULL;
   evas_cache_image_ref(im);
   data = (void *)im->cache->data;
   evas_cache_image_drop(im);
   return data;
}

EAPI void *
evas_cache_private_get(Evas_Cache_Image *cache)
{
   if (!cache) return NULL;
   return cache->data;
}

EAPI void
evas_cache_private_set(Evas_Cache_Image *cache, const void *data)
{
   if (!cache) return;
   cache->data = (void *)data;
}

EAPI DATA32 *
evas_cache_image_pixels(Image_Entry *im)
{
   if (!im->cache) return NULL;
   return im->cache->func.surface_pixels(im);
}

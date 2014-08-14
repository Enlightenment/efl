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
static LK(wakeup);
static int _evas_cache_mutex_init = 0;

static Eina_Condition cond_wakeup;

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
   im->flags.dirty = 0;
   im->flags.cached = 0;
   im->cache->dirty = eina_inlist_remove(im->cache->dirty, EINA_INLIST_GET(im));  
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
   _evas_cache_image_dirty_del(im);
   _evas_cache_image_activ_del(im);
   _evas_cache_image_lru_nodata_del(im); 
   if (!im->cache_key) return;
   im->flags.lru = 1;
   im->flags.cached = 1;
   if (im->flags.given_mmap)
     {
        eina_hash_direct_add(im->cache->mmap_inactiv, im->cache_key, im);
     }
   else
     {
        eina_hash_direct_add(im->cache->inactiv, im->cache_key, im);
     }
   im->cache->lru = eina_inlist_prepend(im->cache->lru, EINA_INLIST_GET(im));
   im->cache->usage += im->cache->func.mem_size_get(im);
}

static void
_evas_cache_image_lru_del(Image_Entry *im)
{
   if (!im->flags.lru) return;
   if (!im->cache_key) return;
   im->flags.lru = 0;
   im->flags.cached = 0;
   if (im->flags.given_mmap)
     {
        eina_hash_del(im->cache->mmap_inactiv, im->cache_key, im);
     }
   else
     {
        eina_hash_del(im->cache->inactiv, im->cache_key, im);
     }
   im->cache->lru = eina_inlist_remove(im->cache->lru, EINA_INLIST_GET(im));
   im->cache->usage -= im->cache->func.mem_size_get(im);
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
   im->cache->lru_nodata = eina_inlist_prepend(im->cache->lru_nodata, EINA_INLIST_GET(im));
}

static void
_evas_cache_image_lru_nodata_del(Image_Entry *im)
{
   if (!im->flags.lru_nodata) return;
   im->flags.lru = 0;
   im->flags.cached = 0;
   im->cache->lru_nodata = eina_inlist_remove(im->cache->lru_nodata, EINA_INLIST_GET(im));
}

static void
_evas_cache_image_entry_delete(Evas_Cache_Image *cache, Image_Entry *ie)
{
   Image_Entry_Task *task;

   if (!ie) return;
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
                            Eina_File *f,
                            const char *file,
                            const char *key,
                            Evas_Image_Load_Opts *lo,
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

   if (lo) ie->load_opts = *lo;
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
   if (cache->func.surface_alloc(ie, wmin, hmin))
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

static void
_evas_cache_image_async_heavy(void *data)
{
   Evas_Cache_Image *cache;
   Image_Entry *current;
   Image_Entry_Task *task;
   int error;
   int pchannel;

   current = data;

   SLKL(current->lock);
   pchannel = current->channel;
   current->channel++;
   cache = current->cache;

   if ((!current->flags.loaded) &&
       current->info.loader->threadable)
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
_evas_cache_image_async_end(void *data)
{
   Image_Entry *ie = (Image_Entry *)data;
   Image_Entry_Task *task;
   Evas_Cache_Target *tmp;

   ie->cache->preload = eina_list_remove(ie->cache->preload, ie);
   ie->cache->pending = eina_list_remove(ie->cache->pending, ie);
   ie->preload = NULL;
   ie->flags.preload_done = ie->flags.loaded;
   ie->flags.updated_data = 1;
   while ((tmp = ie->targets))
     {
        evas_object_inform_call_image_preloaded((Evas_Object*) tmp->target);
        ie->targets = (Evas_Cache_Target *)
           eina_inlist_remove(EINA_INLIST_GET(ie->targets), 
                              EINA_INLIST_GET(ie->targets));
        free(tmp);
     }

   EINA_LIST_FREE(ie->tasks, task)
     if (task != &dummy_task) free(task);
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
   if (ie->references == 0)
     {
        _evas_cache_image_lru_add(ie);
        cache = ie->cache;
     }
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

   if (ie->flags.preload_done) return 0;

   tg = malloc(sizeof (Evas_Cache_Target));
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
        ie->preload = evas_preload_thread_run(_evas_cache_image_async_heavy,
                                              _evas_cache_image_async_end,
                                              _evas_cache_image_async_cancel,
                                              ie);
     }
   return 1;
}

static void
_evas_cache_image_entry_preload_remove(Image_Entry *ie, const Eo *target)
{
   Evas_Cache_Target *tg;
   Eina_List *l;
   Image_Entry_Task *task;

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
                  SLKU(ie->lock_task);

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
}

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
        LKI(wakeup);
        eina_condition_new(&cond_wakeup, &wakeup);
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
   eina_hash_free(cache->activ);
   eina_hash_free(cache->inactiv);
   eina_hash_free(cache->mmap_activ);
   eina_hash_free(cache->mmap_inactiv);
   free(cache);

   if (--_evas_cache_mutex_init == 0)
     {
        eina_condition_free(&cond_wakeup);
        SLKD(engine_lock);
        LKD(wakeup);
     }
}

static const Evas_Image_Load_Opts prevent = {
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
};

static size_t
_evas_cache_image_loadopts_append(char *hkey, Evas_Image_Load_Opts **plo)
{
   Evas_Image_Load_Opts *lo = *plo;
   size_t offset = 0;

   if ((!lo) ||
       (lo &&
        (lo->scale_down_by == 0) &&
        (lo->dpi == 0.0) &&
        ((lo->w == 0) || (lo->h == 0)) &&
        ((lo->region.w == 0) || (lo->region.h == 0)) &&
        (lo->orientation == 0)
       ))
     {
        *plo = (Evas_Image_Load_Opts*) &prevent;
     }
   else
     {
        memcpy(hkey, "//@/", 4);
        offset += 4;
        offset += eina_convert_xtoa(lo->scale_down_by, hkey + offset);
        hkey[offset] = '/';
        offset += 1;
        offset += eina_convert_dtoa(lo->dpi, hkey + offset);
        hkey[offset] = '/';
        offset += 1;
        offset += eina_convert_xtoa(lo->w, hkey + offset);
        hkey[offset] = 'x';
        offset += 1;
        offset += eina_convert_xtoa(lo->h, hkey + offset);
        hkey[offset] = '/';
        offset += 1;
        offset += eina_convert_xtoa(lo->region.x, hkey + offset);
        hkey[offset] = '+';
        offset += 1;
        offset += eina_convert_xtoa(lo->region.y, hkey + offset);
        hkey[offset] = '.';
        offset += 1;
        offset += eina_convert_xtoa(lo->region.w, hkey + offset);
        hkey[offset] = 'x';
        offset += 1;
        offset += eina_convert_xtoa(lo->region.h, hkey + offset);

        if (lo->orientation)
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
   if ((!f) || (!f && !key))
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
   im = eina_hash_find(cache->mmap_activ, hkey);
   if (im) goto on_ok;

   /* find image by key in inactive/lru hash */
   im = eina_hash_find(cache->mmap_inactiv, hkey);
   if (im)
     {
        _evas_cache_image_lru_del(im);
        _evas_cache_image_activ_add(im);
        goto on_ok;
     }

   im = _evas_cache_image_entry_new(cache, hkey, NULL, f, NULL, key, lo, error);
   if (!im) return NULL;

 on_ok:
   *error = EVAS_LOAD_ERROR_NONE;
   im->references++;
   return im;
}


EAPI Image_Entry *
evas_cache_image_request(Evas_Cache_Image *cache, const char *file, 
                         const char *key, Evas_Image_Load_Opts *lo, int *error)
{
   const char           *ckey = "(null)";
   char                 *hkey;
   Image_Entry          *im;
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
   size += _evas_cache_image_loadopts_append(hkey + size, &lo);

   /* find image by key in active hash */
   im = eina_hash_find(cache->activ, hkey);
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
   im = eina_hash_find(cache->inactiv, hkey);
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
        /* this image never used, so it have to be deleted */
        _evas_cache_image_entry_delete(cache, im);
        im = NULL;
     }
   if (stat_failed) goto on_stat_error;

   if (!stat_done)
     {
        if (stat(file, &st) < 0) goto on_stat_error;
     }
   _timestamp_build(&tstamp, &st);
   im = _evas_cache_image_entry_new(cache, hkey, &tstamp, NULL, file, key, 
                                    lo, error);
   if (!im) goto on_stat_error;
   if (cache->func.debug) cache->func.debug("request", im);

on_ok:
   *error = EVAS_LOAD_ERROR_NONE;
   im->references++;
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
evas_cache_image_ref(Image_Entry *im)
{
   im->references++;
}

EAPI void
evas_cache_image_drop(Image_Entry *im)
{
   Evas_Cache_Image *cache;
   int references;

   im->references--;
   if (im->references < 0) im->references = 0;
   references = im->references;

   cache = im->cache;

   if (references == 0)
     {
        if (im->preload)
          {
             _evas_cache_image_entry_preload_remove(im, NULL);
             return;
          }
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
   int references;

   references = im->references;
   if (references > 1) return;
   if ((im->flags.dirty) || (!im->flags.need_data)) return;
   _evas_cache_image_lru_nodata_add(im);
}

EAPI Image_Entry *
evas_cache_image_dirty(Image_Entry *im, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
   Image_Entry *im_dirty = im;
   Evas_Cache_Image *cache;

   cache = im->cache;
   if (!(im->flags.dirty))
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
   references = im->references;

   if (references <= 1)
     {
        if (!im->flags.dirty) _evas_cache_image_dirty_add(im);
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
   Image_Entry *im;

   if ((cspace == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (cspace == EVAS_COLORSPACE_YCBCR422P709_PL) ||
       (cspace == EVAS_COLORSPACE_YCBCR422601_PL))
     w &= ~0x1;

   im = _evas_cache_image_entry_new(cache, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
   if (!im) return NULL;
   im->space = cspace;
   im->flags.alpha = alpha;
   _evas_cache_image_entry_surface_alloc(cache, im, w, h);
   if (cache->func.copied_data(im, w, h, image_data, alpha, cspace) != 0)
     {
        _evas_cache_image_entry_delete(cache, im);
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
   Image_Entry *im;

   if ((cspace == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (cspace == EVAS_COLORSPACE_YCBCR422P709_PL) ||
       (cspace == EVAS_COLORSPACE_YCBCR422601_PL))
     w &= ~0x1;

   im = _evas_cache_image_entry_new(cache, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
   if (!im) return NULL;
   im->w = w;
   im->h = h;
   im->flags.alpha = alpha;
   if (cache->func.data(im, w, h, image_data, alpha, cspace) != 0)
     {
        _evas_cache_image_entry_delete(cache, im);
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
   im2 = _evas_cache_image_entry_new(cache, NULL, NULL, NULL, NULL, NULL, NULL, &error);
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
   return im2;

on_error:
   if (im2) _evas_cache_image_entry_delete(cache, im2);
   evas_cache_image_drop(im);
   return NULL;
}

EAPI int
evas_cache_image_load_data(Image_Entry *im)
{
   Eina_Bool preload = EINA_FALSE;
   int error = EVAS_LOAD_ERROR_NONE;

   if ((im->flags.loaded) && (!im->animated.animated)) return error;
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

   if ((im->flags.loaded) && (!im->animated.animated)) return error;
   
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
   return error;
}

EAPI void
evas_cache_image_unload_data(Image_Entry *im)
{
   if (im->flags.in_progress) return;
   evas_cache_image_preload_cancel(im, NULL);
   
   if (SLKT(im->lock) == EINA_FALSE) /* can't get image lock - busy async load */
     {
        SLKL(im->lock_cancel);
        im->flags.unload_cancel = EINA_TRUE;
        SLKU(im->lock_cancel);
        return;
     }

   SLKL(im->lock_cancel);
   if ((!im->flags.loaded) || (!im->file && !im->f) || (!im->info.module) || 
       (im->flags.dirty))
     {
        SLKU(im->lock_cancel);
        SLKU(im->lock);
        return;
     }
   SLKU(im->lock_cancel);
   im->cache->func.destructor(im);
   SLKU(im->lock);
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

   EINA_INLIST_FOREACH(cache->lru, im) evas_cache_image_unload_data(im);
   EINA_INLIST_FOREACH(cache->lru_nodata, im) evas_cache_image_unload_data(im);
   eina_hash_foreach(cache->activ, _evas_cache_image_unload_cb, NULL);
   eina_hash_foreach(cache->inactiv, _evas_cache_image_unload_cb, NULL);
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

   if ((im->flags.loaded) && (img->image.data))
     {
        evas_object_inform_call_image_preloaded((Evas_Object*)target);
        return;
     }
   im->flags.loaded = 0;
   if (!_evas_cache_image_entry_preload_add(im, target, func, engine_data, custom_data))
     evas_object_inform_call_image_preloaded((Evas_Object*) target);
}

EAPI void
evas_cache_image_preload_cancel(Image_Entry *im, const Eo *target)
{
   if (!target) return;
   _evas_cache_image_entry_preload_remove(im, target);
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

   im = _evas_cache_image_entry_new(cache, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
   if (!im) return NULL;
   im->references = 1;
   return im;
}

EAPI void
evas_cache_image_colorspace(Image_Entry *im, Evas_Colorspace cspace)
{
   if (im->space == cspace) return;
   im->space = cspace;
   if (!im->cache) return;
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
   if (!im->cache) return NULL;
   return im->cache->func.surface_pixels(im);
}

EAPI void
evas_cache_image_wakeup(void)
{
   if (_evas_cache_mutex_init > 0)
     {
        LKL(wakeup);
        eina_condition_broadcast(&cond_wakeup);
        LKU(wakeup);
     }
}

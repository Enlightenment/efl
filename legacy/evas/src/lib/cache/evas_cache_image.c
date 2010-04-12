/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

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

static LK(engine_lock) = PTHREAD_MUTEX_INITIALIZER;
static LK(wakeup) = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t cond_wakeup = PTHREAD_COND_INITIALIZER;

static void _evas_cache_image_entry_preload_remove(Image_Entry *ie, const void *target);
#endif

#define FREESTRC(Var)              \
  if (Var)                         \
    {                              \
       eina_stringshare_del(Var);  \
       Var = NULL;                 \
    }

static void _evas_cache_image_entry_delete(Evas_Cache_Image *cache, Image_Entry *ie);

static void
_evas_cache_image_make_dirty(Evas_Cache_Image *cache,
                             Image_Entry *im)
{
   im->flags.cached = 1;
   im->flags.dirty = 1;
   im->flags.activ = 0;
   im->flags.lru_nodata = 0;
   cache->dirty = eina_inlist_prepend(cache->dirty, EINA_INLIST_GET(im));

   if (im->cache_key)
     {
        eina_stringshare_del(im->cache_key);
        im->cache_key = NULL;
     }
}

static void
_evas_cache_image_make_activ(Evas_Cache_Image *cache,
                             Image_Entry *im,
                             const char *key)
{
   /* FIXME: Handle case when image is being processed anyway and don't do a double decode. */
   im->cache_key = key;
   if (key != NULL)
     {
        im->flags.cached = 1;
        im->flags.activ = 1;
        im->flags.lru_nodata = 0;
        im->flags.dirty = 0;
	eina_hash_direct_add(cache->activ, key, im);
     }
   else
     {
        _evas_cache_image_make_dirty(cache, im);
     }
}

static void
_evas_cache_image_make_inactiv(Evas_Cache_Image *cache,
                               Image_Entry *im,
                               const char *key)
{
   if (im->cache_key)
     {
	im->flags.activ = 0;
	im->flags.dirty = 0;
	im->flags.cached = 1;
	eina_hash_direct_add(cache->inactiv, key, im);
	cache->lru = eina_inlist_prepend(cache->lru, EINA_INLIST_GET(im));
	cache->usage += cache->func.mem_size_get(im);
     }
   else
     {
	_evas_cache_image_entry_delete(cache, im);
     }
 }

static void
_evas_cache_image_remove_lru_nodata(Evas_Cache_Image *cache,
                                    Image_Entry *im)
{
   if (im->flags.lru_nodata)
     {
        im->flags.lru_nodata = 0;
        cache->lru_nodata = eina_inlist_remove(cache->lru_nodata, EINA_INLIST_GET(im));
        cache->usage -= cache->func.mem_size_get(im);
     }
}

static void
_evas_cache_image_activ_lru_nodata(Evas_Cache_Image *cache,
                                   Image_Entry *im)
{
   im->flags.need_data = 0;
   im->flags.lru_nodata = 1;
   cache->lru_nodata = eina_inlist_prepend(cache->lru_nodata, EINA_INLIST_GET(im));
   cache->usage += cache->func.mem_size_get(im);
}

static void
_evas_cache_image_remove_activ(Evas_Cache_Image *cache,
                               Image_Entry *ie)
{
   if (ie->flags.cached)
     {
        if (ie->flags.activ)
          {
	     eina_hash_del(cache->activ, ie->cache_key, ie);
             _evas_cache_image_remove_lru_nodata(cache, ie);
          }
        else
          {
             if (ie->flags.dirty)
               {
 		  cache->dirty = eina_inlist_remove(cache->dirty, EINA_INLIST_GET(ie));
               }
             else
               {
		  eina_hash_del(cache->inactiv, ie->cache_key, ie);
                  cache->lru = eina_inlist_remove(cache->lru, EINA_INLIST_GET(ie));
                  cache->usage -= cache->func.mem_size_get(ie);
               }
          }
        ie->flags.cached = 0;
        ie->flags.dirty = 0;
        ie->flags.activ = 0;
     }
}

static void
_evas_cache_image_entry_delete(Evas_Cache_Image *cache, Image_Entry *ie)
{
   if (!ie) return ;

   if (cache->func.debug)
     cache->func.debug("deleting", ie);

#ifdef BUILD_ASYNC_PRELOAD
   if (ie->flags.delete_me == 1)
     return ;

   if (ie->preload)
     {
	ie->flags.delete_me = 1;

	_evas_cache_image_entry_preload_remove(ie, NULL);
	return ;
     }
#endif

   cache->func.destructor(ie);

   _evas_cache_image_remove_activ(cache, ie);

   if (ie->cache_key)
     {
        eina_stringshare_del(ie->cache_key);
        ie->cache_key = NULL;
     }

   FREESTRC(ie->file);
   FREESTRC(ie->key);

   ie->cache = NULL;

   cache->func.surface_delete(ie);

#ifdef BUILD_ASYNC_PRELOAD
   LKD(ie->lock);
#endif

   cache->func.dealloc(ie);
}

static Image_Entry *
_evas_cache_image_entry_new(Evas_Cache_Image *cache,
                            const char *hkey,
                            time_t timestamp,
                            const char *file,
                            const char *key,
                            RGBA_Image_Loadopts *lo,
                            int *error)
{
   Image_Entry  *ie;
   const char   *cache_key;

   ie = cache->func.alloc();
   if (!ie)
     {
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	return NULL;
     }

   cache_key = hkey ? eina_stringshare_add(hkey) : NULL;

   ie->flags.loaded = 0;
   ie->flags.need_data = 1;

   _evas_cache_image_make_activ(cache, ie, cache_key);

   ie->space = EVAS_COLORSPACE_ARGB8888;
   ie->w = -1;
   ie->h = -1;
   ie->allocated.w = 0;
   ie->allocated.h = 0;

   ie->references = 0;
   ie->cache = cache;

   ie->file = file ? eina_stringshare_add(file) : NULL;
   ie->key = key ? eina_stringshare_add(key) : NULL;

   ie->timestamp = timestamp;
   ie->laststat = time(NULL);

   ie->load_opts.scale_down_by = 0;
   ie->load_opts.dpi = 0;
   ie->load_opts.w = 0;
   ie->load_opts.h = 0;
   ie->load_opts.region.x = 0;
   ie->load_opts.region.y = 0;
   ie->load_opts.region.w = 0;
   ie->load_opts.region.h = 0;
   ie->scale = 1;

#ifdef BUILD_ASYNC_PRELOAD
   LKI(ie->lock);
   ie->targets = NULL;
   ie->preload = NULL;
   ie->flags.delete_me = 0;
#endif

   if (lo)
     ie->load_opts = *lo;

   if (file)
     {
        *error = cache->func.constructor(ie);
        if (*error != EVAS_LOAD_ERROR_NONE)
          {
             _evas_cache_image_entry_delete(cache, ie);
             return NULL;
          }
     }
   if (cache->func.debug)
     cache->func.debug("build", ie);

   return ie;
}

static void
_evas_cache_image_entry_surface_alloc__locked(Evas_Cache_Image *cache,
					      Image_Entry *ie,
					      int wmin,
					      int hmin)
{
   if (ie->allocated.w == wmin && ie->allocated.h == hmin)
     return ;

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
                                      Image_Entry *ie,
                                      int w,
                                      int h)
{
   int  wmin;
   int  hmin;

   wmin = w > 0 ? w : 1;
   hmin = h > 0 ? h : 1;

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

   if (!current->flags.loaded && ((Evas_Image_Load_Func*) current->info.module)->threadable)
     {
	error = cache->func.load(current);
	if (cache->func.debug)
	  cache->func.debug("load", current);
	if (error != EVAS_LOAD_ERROR_NONE)
	  {
	     current->flags.loaded = 0;
	     _evas_cache_image_entry_surface_alloc(cache, current,
						   current->w, current->h);
	  }
	else
	  current->flags.loaded = 1;
     }

   current->channel = pchannel;
   LKU(current->lock);
}

static void
_evas_cache_image_async_end(void *data)
{
   Image_Entry *ie = (Image_Entry *) data;
   Evas_Cache_Target *tmp;

   ie->cache->preload = eina_list_remove(ie->cache->preload, ie);
   ie->cache->pending = eina_list_remove(ie->cache->pending, ie);

   ie->preload = NULL;
   ie->flags.preload_done = ie->flags.loaded;
   while (ie->targets)
     {
	tmp = ie->targets;

	evas_object_inform_call_image_preloaded((Evas_Object*) tmp->target);
	ie->targets = (Evas_Cache_Target*) eina_inlist_remove(EINA_INLIST_GET(ie->targets), EINA_INLIST_GET(ie->targets));
	free(tmp);
     }
}

static void
_evas_cache_image_async_cancel(void *data)
{
   Image_Entry *ie = (Image_Entry *) data;

   ie->preload = NULL;
   ie->cache->pending = eina_list_remove(ie->cache->pending, ie);

   if (ie->flags.delete_me || ie->flags.dirty)
     {
	ie->flags.delete_me = 0;
	_evas_cache_image_entry_delete(ie->cache, ie);

	return ;
     }

   if (ie->flags.loaded)
     {
	_evas_cache_image_async_end(ie);
     }

   if (ie->references == 0)
     {
	_evas_cache_image_remove_activ(ie->cache, ie);
	_evas_cache_image_make_inactiv(ie->cache, ie, ie->cache_key);
	evas_cache_image_flush(ie->cache);
     }
}

static int
_evas_cache_image_entry_preload_add(Image_Entry *ie,
				    const void *target)
{
   Evas_Cache_Target *tg;

   if (ie->flags.preload_done) return 0;

   tg = malloc(sizeof (Evas_Cache_Target));
   if (!tg) return 0;

   tg->target = target;

   ie->targets = (Evas_Cache_Target*) eina_inlist_append(EINA_INLIST_GET(ie->targets), EINA_INLIST_GET(tg));

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
		  ie->targets = (Evas_Cache_Target*) eina_inlist_remove(EINA_INLIST_GET(ie->targets), EINA_INLIST_GET(tg));
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

	     ie->targets = (Evas_Cache_Target*) eina_inlist_remove(EINA_INLIST_GET(ie->targets), EINA_INLIST_GET(tg));
	     free(tg);
	  }
     }

   if (ie->targets == NULL && ie->preload && !ie->flags.pending)
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
   assert(cache != NULL);

   return cache->usage;
}

EAPI int
evas_cache_image_get(Evas_Cache_Image *cache)
{
   assert(cache != NULL);

   return cache->limit;
}

EAPI void
evas_cache_image_set(Evas_Cache_Image *cache, int limit)
{
   assert(cache != NULL);
   if (cache->limit == limit) return;
   cache->limit = limit;
   evas_cache_image_flush(cache);
}

EAPI Evas_Cache_Image *
evas_cache_image_init(const Evas_Cache_Image_Func *cb)
{
   Evas_Cache_Image    *new;

   new = malloc(sizeof (Evas_Cache_Image));
   if (!new)
     return NULL;

   new->func = *cb;

   new->limit = 0;
   new->usage = 0;

   new->dirty = NULL;
   new->lru = NULL;
   new->lru_nodata = NULL;
   new->inactiv = eina_hash_string_superfast_new(NULL);
   new->activ = eina_hash_string_superfast_new(NULL);

   new->references = 1;

   new->preload = NULL;
   new->pending = NULL;

   return new;
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

   assert(cache != NULL);
   cache->references--;

   if (cache->references > 0)
     return ;

#ifdef BUILD_ASYNC_PRELOAD
   EINA_LIST_FREE(cache->preload, im)
     {
	/* By doing that we are protecting us from destroying image when the cache is no longuer available. */
	im->flags.delete_me = 1;
	_evas_cache_image_entry_preload_remove(im, NULL);
     }

   evas_async_events_process();
#endif

   while (cache->lru)
     {
        im = (Image_Entry *) cache->lru;
        _evas_cache_image_entry_delete(cache, im);
     }

   while (cache->lru_nodata)
     {
        im = (Image_Entry *) cache->lru_nodata;
        _evas_cache_image_entry_delete(cache, im);
     }

   /* This is mad, I am about to destroy image still alive, but we need to prevent leak. */
   while (cache->dirty)
     {
        im = (Image_Entry *) cache->dirty;
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
	if (cache->pending)
	  pthread_cond_wait(&cond_wakeup, &wakeup);
	LKU(wakeup);
     }
#endif

   eina_hash_free(cache->activ);
   eina_hash_free(cache->inactiv);

   free(cache);
}

#define STAT_GAP 2

EAPI Image_Entry *
evas_cache_image_request(Evas_Cache_Image *cache, const char *file, const char *key, RGBA_Image_Loadopts *lo, int *error)
{
   const char           *ckey = "(null)";
   char                 *hkey;
   Image_Entry          *im;
   Evas_Image_Load_Opts  prevent = { 0, 0, 0, 0, { 0, 0, 0, 0 } };
   size_t                size;
   int                   stat_done = 0;
   size_t                file_length;
   size_t                key_length;
   struct stat           st;

   assert(cache != NULL);

   if ((!file) || ((!file) && (!key)))
     {
	*error = EVAS_LOAD_ERROR_GENERIC;
	return NULL;
     }

   file_length = strlen(file);
   key_length = key ? strlen(key) : 6;

   size = file_length + key_length + 128;
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
        ((lo->region.w == 0) || (lo->region.w == 0))
        ))
     {
        lo = &prevent;
//        if (key)
//          format = "%s//://%s";
//        else
//          format = "%s//://%p";
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
     }

   hkey[size] = '\0';

   im = eina_hash_find(cache->activ, hkey);
   if (im)
     {
        time_t  t;
        int     ok;

        ok = 1;
        t = time(NULL);
        if ((t - im->laststat) > STAT_GAP)
          {
             stat_done = 1;
             if (stat(file, &st) < 0) goto on_stat_error;

             im->laststat = t;
             if (st.st_mtime != im->timestamp) ok = 0;
          }
        if (ok) goto on_ok;

        _evas_cache_image_remove_activ(cache, im);
	_evas_cache_image_make_dirty(cache, im);
     }

   im = eina_hash_find(cache->inactiv, hkey);
   if (im)
     {
        int     ok;

        ok = 1;
        if (!stat_done)
          {
             time_t  t;

             t = time(NULL);
             if ((t - im->laststat) > STAT_GAP)
               {
                  stat_done = 1;
                  if (stat(file, &st) < 0) goto on_stat_error;

                  im->laststat = t;
                  if (st.st_mtime != im->timestamp) ok = 0;
               }
          }
        else
          if (st.st_mtime != im->timestamp) ok = 0;

        if (ok)
          {
             _evas_cache_image_remove_activ(cache, im);
             _evas_cache_image_make_activ(cache, im, im->cache_key);
             goto on_ok;
          }

        _evas_cache_image_entry_delete(cache, im);
     }

   if (!stat_done)
     {
        if (stat(file, &st) < 0) goto on_stat_error;
     }

   im = _evas_cache_image_entry_new(cache, hkey, st.st_mtime, file, key, lo, error);
   if (!im) return NULL;

   if (cache->func.debug)
     cache->func.debug("request", im);

 on_ok:
   *error = EVAS_LOAD_ERROR_NONE;
   im->references++;
   if (im->references > 1 && im->flags.lru_nodata)
     _evas_cache_image_remove_lru_nodata(cache, im);

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
   Evas_Cache_Image    *cache;

   assert(im);
   assert(im->cache);

   im->references--;
   cache = im->cache;

   if (im->references == 0)
     {
#ifdef BUILD_ASYNC_PRELOAD
	if (im->preload)
	  {
	     _evas_cache_image_entry_preload_remove(im, NULL);
	     return ;
	  }
#endif

	if (im->flags.dirty)
	  {
	     _evas_cache_image_entry_delete(cache, im);
	     return;
	  }

        _evas_cache_image_remove_activ(cache, im);
	_evas_cache_image_make_inactiv(cache, im, im->cache_key);
	evas_cache_image_flush(cache);
     }
}

EAPI void
evas_cache_image_data_not_needed(Image_Entry *im)
{
   Evas_Cache_Image    *cache;

   assert(im);
   assert(im->cache);

   cache = im->cache;

   if (im->references > 1) return ;
   if (im->flags.dirty || !im->flags.need_data) return ;

   _evas_cache_image_activ_lru_nodata(cache, im);
}

EAPI Image_Entry *
evas_cache_image_dirty(Image_Entry *im, int x, int y, int w, int h)
{
   Image_Entry          *im_dirty = im;
   Evas_Cache_Image     *cache;

   assert(im);
   assert(im->cache);

   cache = im->cache;
   if (!(im->flags.dirty))
     {
#ifndef EVAS_CSERVE
// if ref 1 also copy if using shared cache as its read-only
        if (im->references == 1) im_dirty = im;
        else
#endif
          {
             int        error;

             im_dirty = evas_cache_image_copied_data
               (cache, im->w, im->h, 
                evas_cache_image_pixels(im), 
                im->flags.alpha, 
                im->space);
             if (!im_dirty) goto on_error;
             if (cache->func.debug)
               cache->func.debug("dirty-src", im);
             error = cache->func.dirty(im_dirty, im);
             if (cache->func.debug)
               cache->func.debug("dirty-out", im_dirty);
/*             
             im_dirty = _evas_cache_image_entry_new(cache, NULL, im->timestamp, im->file, im->key, &im->load_opts, &error);
             if (!im_dirty) goto on_error;

             if (cache->func.debug)
               cache->func.debug("dirty-src", im);
             error = cache->func.dirty(im_dirty, im);
             if (cache->func.debug)
               cache->func.debug("dirty-out", im_dirty);

             if (error != 0) goto on_error;
 */
             im_dirty->references = 1;

             evas_cache_image_drop(im);
          }

	_evas_cache_image_remove_activ(cache, im_dirty);
        _evas_cache_image_make_dirty(cache, im_dirty);
     }

   if (cache->func.debug)
     cache->func.debug("dirty-region", im_dirty);
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
   Evas_Cache_Image     *cache;
   Image_Entry          *im_dirty = im;

   assert(im);
   assert(im->cache);

   cache = im->cache;
   if (im->references == 1)
     {
        if (!(im->flags.dirty))
          {
             _evas_cache_image_remove_activ(cache, im);
             _evas_cache_image_make_dirty(cache, im);
          }
     }
   else
     {
        int     error;

        im_dirty = evas_cache_image_copied_data
          (cache, im->w, im->h, 
           evas_cache_image_pixels(im), 
           im->flags.alpha, 
           im->space);
        if (!im_dirty) goto on_error;
        if (cache->func.debug)
          cache->func.debug("dirty-src", im);
        error = cache->func.dirty(im_dirty, im);
        if (cache->func.debug)
          cache->func.debug("dirty-out", im_dirty);
/*        
        im_dirty = _evas_cache_image_entry_new(cache, NULL, im->timestamp, im->file, im->key, &im->load_opts, &error);
        if (!im_dirty) goto on_error;

        if (cache->func.debug)
          cache->func.debug("dirty-src", im);
        error = cache->func.dirty(im_dirty, im);
        if (cache->func.debug)
          cache->func.debug("dirty-out", im_dirty);

        if (error != 0) goto on_error;
 */
        im_dirty->references = 1;

        evas_cache_image_drop(im);
     }

   return im_dirty;
   
   on_error:
   if (im_dirty) _evas_cache_image_entry_delete(cache, im_dirty);
   evas_cache_image_drop(im);
   return NULL;
}

EAPI Image_Entry *
evas_cache_image_copied_data(Evas_Cache_Image *cache, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Image_Entry  *im;

   assert(cache);

   if ((cspace == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (cspace == EVAS_COLORSPACE_YCBCR422P709_PL))
     w &= ~0x1;

   im = _evas_cache_image_entry_new(cache, NULL, 0, NULL, NULL, NULL, NULL);
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

   if (cache->func.debug)
     cache->func.debug("copied-data", im);
   return im;
}

EAPI Image_Entry *
evas_cache_image_data(Evas_Cache_Image *cache, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Image_Entry  *im;

   assert(cache);

   if ((cspace == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (cspace == EVAS_COLORSPACE_YCBCR422P709_PL))
     w &= ~0x1;

   im = _evas_cache_image_entry_new(cache, NULL, 0, NULL, NULL, NULL, NULL);
   im->w = w;
   im->h = h;
   im->flags.alpha = alpha;

   if (cache->func.data(im, w, h, image_data, alpha, cspace) != 0)
     {
        _evas_cache_image_entry_delete(cache, im);
        return NULL;
     }
   im->references = 1;

   if (cache->func.debug)
     cache->func.debug("data", im);
   return im;
}

EAPI void
evas_cache_image_surface_alloc(Image_Entry *im, int w, int h)
{
   Evas_Cache_Image     *cache;

   assert(im);
   assert(im->cache);

   cache = im->cache;

   if ((im->space == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (im->space == EVAS_COLORSPACE_YCBCR422P709_PL))
     w &= ~0x1;

   _evas_cache_image_entry_surface_alloc(cache, im, w, h);

   if (cache->func.debug)
     cache->func.debug("surface-alloc", im);
}

EAPI Image_Entry *
evas_cache_image_size_set(Image_Entry *im, int w, int h)
{
   Evas_Cache_Image    *cache;
   Image_Entry         *new;
   int                  error;

   assert(im);
   assert(im->cache);
   assert(im->references > 0);

   if ((im->space == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (im->space == EVAS_COLORSPACE_YCBCR422P709_PL))
     w &= ~0x1;

   if ((im->w == w) && (im->h == h))
     return im;

   cache = im->cache;

   new = _evas_cache_image_entry_new(cache, NULL, 0, NULL, NULL, NULL, &error);
   if (!new) goto on_error;

   new->flags.alpha = im->flags.alpha;
   new->space = im->space;
   new->load_opts = im->load_opts;

   _evas_cache_image_entry_surface_alloc(cache, new, w, h);

   error = cache->func.size_set(new, im, w, h);
   if (error != 0) goto on_error;

   new->references = 1;

   evas_cache_image_drop(im);

   if (cache->func.debug)
     cache->func.debug("size_set", new);

   return new;

 on_error:
   if (new) _evas_cache_image_entry_delete(cache, new);
   evas_cache_image_drop(im);
   return NULL;
}

EAPI void
evas_cache_image_load_data(Image_Entry *im)
{
#ifdef BUILD_ASYNC_PRELOAD
   Eina_Bool preload = EINA_FALSE;
#endif
   Evas_Cache_Image *cache;
   int error;

   assert(im);
   assert(im->cache);
   cache = im->cache;

   if (im->flags.loaded)
     {
	return;
     }

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
	     pthread_cond_wait(&cond_wakeup, &wakeup);
	     LKU(wakeup);

	     evas_async_events_process();

	     LKL(wakeup);
	  }
	LKU(wakeup);
     }

   if (im->flags.loaded) return ;
   LKL(im->lock);
#endif

   error = cache->func.load(im);
#ifdef BUILD_ASYNC_PRELOAD
   LKU(im->lock);
#endif

   im->flags.loaded = 1;

   if (cache->func.debug)
     cache->func.debug("load", im);

   if (error != EVAS_LOAD_ERROR_NONE)
     {
        _evas_cache_image_entry_surface_alloc(cache, im, im->w, im->h);
        im->flags.loaded = 0;
     }

#ifdef BUILD_ASYNC_PRELOAD
   if (preload)
     _evas_cache_image_async_end(im);
#endif
}

EAPI void
evas_cache_image_unload_data(Image_Entry *im)
{
   Evas_Cache_Image *cache;

   assert(im);
   assert(im->cache);
   cache = im->cache;

#ifdef BUILD_ASYNC_PRELOAD
   LKL(im->lock);
#endif
   if ((!im->flags.loaded) || (!im->file) || 
       (!im->info.module) || (im->flags.dirty))
     {
#ifdef BUILD_ASYNC_PRELOAD
        LKU(im->lock);
#endif
	return;
     }
   cache->func.destructor(im);
   
#ifdef BUILD_ASYNC_PRELOAD
   LKU(im->lock);
#endif
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
  assert(im); 
  if (im->flags.loaded) return EINA_TRUE;
  return EINA_FALSE;
}

EAPI void
evas_cache_image_preload_data(Image_Entry *im, const void *target)
{
#ifdef BUILD_ASYNC_PRELOAD
   Evas_Cache_Image    *cache;

   assert(im);
   assert(im->cache);

   if (im->flags.loaded)
     {
	evas_object_inform_call_image_preloaded((Evas_Object *)target);
	return;
     }

   cache = im->cache;

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
   Evas_Cache_Image *cache;

   assert(im);
   assert(im->cache);
   cache = im->cache;

   if (target == NULL) return ;

   _evas_cache_image_entry_preload_remove(im, target);
#else
   (void)im;
#endif
}

EAPI int
evas_cache_image_flush(Evas_Cache_Image *cache)
{
   assert(cache);

   if (cache->limit == -1) return -1;

   while ((cache->lru) && (cache->limit < cache->usage))
     {
        Image_Entry *im;

        im = (Image_Entry *)cache->lru->last;
        _evas_cache_image_entry_delete(cache, im);
     }

   while ((cache->lru_nodata) && (cache->limit < cache->usage))
     {
        Image_Entry *im;

        im = (Image_Entry *) cache->lru_nodata->last;
        _evas_cache_image_remove_lru_nodata(cache, im);

        cache->func.surface_delete(im);

        im->flags.loaded = 0;
     }

   return cache->usage;
}

EAPI Image_Entry *
evas_cache_image_empty(Evas_Cache_Image *cache)
{
   Image_Entry *im;

   im = _evas_cache_image_entry_new(cache, NULL, 0, NULL, NULL, NULL, NULL);
   if (!im) return NULL;

   im->references = 1;
   return im;
}

EAPI void
evas_cache_image_colorspace(Image_Entry *im, int cspace)
{
   Evas_Cache_Image    *cache;

   assert(im);
   assert(im->cache);

   cache = im->cache;

   if (im->space == cspace) return;

   im->space = cspace;
   cache->func.color_space(im, cspace);
}

EAPI void *
evas_cache_private_from_image_entry_get(Image_Entry *im)
{
   Evas_Cache_Image	*cache;

   assert(im);
   assert(im->cache);

   cache = im->cache;

   return (void*) cache->data;
}

EAPI void *
evas_cache_private_get(Evas_Cache_Image *cache)
{
   assert(cache);

   return cache->data;
}

EAPI void
evas_cache_private_set(Evas_Cache_Image *cache, const void *data)
{
   assert(cache);

   cache->data = (void *)data;
}

EAPI DATA32 *
evas_cache_image_pixels(Image_Entry *im)
{
   Evas_Cache_Image    *cache;

   assert(im);
   assert(im->cache);

   cache = im->cache;

   return cache->func.surface_pixels(im);
}

EAPI void
evas_cache_image_wakeup(void)
{
#ifdef BUILD_ASYNC_PRELOAD
   pthread_cond_broadcast(&cond_wakeup);
#endif
}

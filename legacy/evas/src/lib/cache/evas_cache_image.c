/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <stdlib.h>
#include <assert.h>

#include "evas_common.h"
#include "evas_private.h"

#ifdef BUILD_ASYNC_PRELOAD
#include <pthread.h>

static Eina_List *preload = NULL;
static Image_Entry *current = NULL;

static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_surface_alloc = PTHREAD_MUTEX_INITIALIZER;
static pthread_t tid;

static Evas_Bool running = 0;

static void* _evas_cache_background_load(void *);
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
   im->cache_key = key;
   if (key != NULL)
     {
        im->flags.cached = 1;
        im->flags.activ = 1;
        im->flags.lru_nodata = 0;
        im->flags.dirty = 0;
        cache->activ = evas_hash_direct_add(cache->activ, key, im);
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
	cache->inactiv = evas_hash_direct_add(cache->inactiv, key, im);
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
             cache->activ = evas_hash_del(cache->activ, ie->cache_key, ie);
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
                  cache->inactiv = evas_hash_del(cache->inactiv, ie->cache_key, ie);
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

   cache->func.destructor(ie);

   _evas_cache_image_remove_activ(cache, ie);

   if (ie->cache_key)
     {
        eina_stringshare_del(ie->cache_key);
        ie->cache_key = NULL;
     }

   FREESTRC(ie->file);
   FREESTRC(ie->key);

   cache->func.surface_delete(ie);
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
     return NULL;

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
   ie->scale = 1;

   if (lo)
     ie->load_opts = *lo;

   if (file)
     {
        *error = cache->func.constructor(ie);
        if (*error != 0)
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
_evas_cache_image_entry_surface_alloc(Evas_Cache_Image *cache,
                                      Image_Entry *ie,
                                      int w,
                                      int h)
{
   int  wmin;
   int  hmin;

   wmin = w > 0 ? w : 1;
   hmin = h > 0 ? h : 1;
   if (ie->allocated.w == wmin && ie->allocated.h == hmin)
     return ;

#ifdef BUILD_ASYNC_PRELOAD
   pthread_mutex_lock(&mutex_surface_alloc);
#endif

   if (cache->func.surface_alloc(ie, wmin, hmin))
     {
        wmin = 0;
        hmin = 0;
     }

#ifdef BUILD_ASYNC_PRELOAD
   pthread_mutex_unlock(&mutex_surface_alloc);
#endif

   ie->w = wmin;
   ie->h = hmin;
   ie->allocated.w = wmin;
   ie->allocated.h = hmin;
}

#ifdef BUILD_ASYNC_PRELOAD
static int
_evas_cache_image_entry_preload_add(Evas_Cache_Image *cache,
				    Image_Entry *ie,
				    const void *target)
{
   int ret = 0;

   pthread_mutex_lock(&mutex);

   if (!ie->flags.preload)
     {
	preload = eina_list_append(preload, ie);
	ie->flags.preload = 1;
	ie->target = target;

	if (!running)
	  {
	     if (pthread_create(&tid, NULL, _evas_cache_background_load, NULL) == 0)
	       running = 1;
	  }

	ret = 1;
     }

   pthread_mutex_unlock(&mutex);

   return ret;
}

static int
_evas_cache_image_entry_preload_remove(Evas_Cache_Image *cache,
				       Image_Entry *ie)
{
   int ret = 0;

   if (running)
     {
	pthread_mutex_lock(&mutex);

	if (ie->flags.preload)
	  {
	     if (current == ie)
	       {
		  /* Wait until ie is processed. */
		  pthread_cond_wait(&cond, &mutex);
	       }
	     else
	       {
		  preload = eina_list_remove(preload, ie);
		  ie->flags.preload = 0;
		  ret = 1;
	       }
	  }

	pthread_mutex_unlock(&mutex);
     }

   return ret;
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
   new->inactiv = NULL;
   new->activ = NULL;

   new->references = 1;

   return new;
}

static Evas_Bool
_evas_cache_image_free_cb(__UNUSED__ const Evas_Hash *hash, __UNUSED__ const void *key, void *data, void *fdata)
{
   Eina_List **delete_list = fdata;

   *delete_list = eina_list_prepend(*delete_list, data);

   return 1;
}

EAPI void
evas_cache_image_shutdown(Evas_Cache_Image *cache)
{
   Image_Entry  *im;
   Eina_List *delete_list;

   assert(cache != NULL);
   cache->references--;

   if (cache->references > 0)
     return ;

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

   evas_hash_foreach(cache->activ, _evas_cache_image_free_cb, &delete_list);

   while (delete_list)
     {
	_evas_cache_image_entry_delete(cache, eina_list_data_get(delete_list));
	delete_list = eina_list_remove_list(delete_list, delete_list);
     }

   evas_hash_free(cache->activ);
   evas_hash_free(cache->inactiv);

   free(cache);
}

#define STAT_GAP 2

EAPI Image_Entry *
evas_cache_image_request(Evas_Cache_Image *cache, const char *file, const char *key, RGBA_Image_Loadopts *lo, int *error)
{
   const char           *ckey = "(null)";
   const char           *format;
   char                 *hkey;
   Image_Entry          *im;
   Evas_Image_Load_Opts  prevent = { 0, 0, 0, 0 };
   int                   size;
   int                   stat_done = 0;
   int                   file_length;
   int                   key_length;
   struct stat           st;

   assert(cache != NULL);

   if (!file && !key) return NULL;
   if (!file) return NULL;

   file_length = strlen(file);
   key_length = key ? strlen(key) : 6;

   size = file_length + key_length + 64;
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
        ((lo->w == 0) || (lo->h == 0))))
     {
        lo = &prevent;
        if (key)
          format = "%s//://%s";
        else
          format = "%s//://%p";
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
     }

   hkey[size] = '\0';

   im = evas_hash_find(cache->activ, hkey);
   if (im)
     {
        time_t  t;
        int     ok;

        ok = 1;
        t = time(NULL);

        if ((t - im->laststat) > STAT_GAP)
          {
             stat_done = 1;
             if (stat(file, &st) < 0) goto on_error;

             im->laststat = t;
             if (st.st_mtime != im->timestamp) ok = 0;
          }
        if (ok) goto on_ok;

        _evas_cache_image_remove_activ(cache, im);
	_evas_cache_image_make_dirty(cache, im);
     }

   im = evas_hash_find(cache->inactiv, hkey);
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
                  if (stat(file, &st) < 0) goto on_error;

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
        if (stat(file, &st) < 0) return NULL;
     }

   im = _evas_cache_image_entry_new(cache, hkey, st.st_mtime, file, key, lo, error);
   if (!im)
     return NULL;

   if (cache->func.debug)
     cache->func.debug("request", im);

 on_ok:
   *error = 0;
   im->references++;
   if (im->references > 1 && im->flags.lru_nodata)
     _evas_cache_image_remove_lru_nodata(cache, im);

   return im;

 on_error:
   _evas_cache_image_entry_delete(cache, im);
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
	_evas_cache_image_entry_preload_remove(cache, im);
#endif

	if (im->flags.dirty)
	  {
	     _evas_cache_image_entry_delete(cache, im);
	     return ;
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
        if (im->references == 1) im_dirty = im;
        else
          {
             int        error;

             im_dirty = _evas_cache_image_entry_new(cache, NULL, im->timestamp, im->file, im->key, &im->load_opts, &error);
             if (!im_dirty) goto on_error;

             if (cache->func.debug)
               cache->func.debug("dirty-src", im);
             error = cache->func.dirty(im_dirty, im);
             if (cache->func.debug)
               cache->func.debug("dirty-out", im_dirty);

             if (error != 0) goto on_error;

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

        im_dirty = _evas_cache_image_entry_new(cache, NULL, im->timestamp, im->file, im->key, &im->load_opts, &error);
        if (!im_dirty) goto on_error;

        if (cache->func.debug)
          cache->func.debug("dirty-src", im);
        error = cache->func.dirty(im_dirty, im);
        if (cache->func.debug)
          cache->func.debug("dirty-out", im_dirty);

        if (error != 0) goto on_error;

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
   Evas_Cache_Image    *cache;
   int                  error;

   assert(im);
   assert(im->cache);

   if (im->flags.loaded) return ;

#ifdef BUILD_ASYNC_PRELOAD
   /* We check a first time, to prevent useless lock. */
   _evas_cache_image_entry_preload_remove(cache, im);
   if (im->flags.loaded) return ;
#endif

   cache = im->cache;

   error = cache->func.load(im);

   if (cache->func.debug)
     cache->func.debug("load", im);

   if (error)
     {
        _evas_cache_image_entry_surface_alloc(cache, im, im->w, im->h);
        im->flags.loaded = 0;

        return ;
     }

   im->flags.loaded = 1;
}

EAPI void
evas_cache_image_preload_data(Image_Entry *im, const void *target)
{
#ifdef BUILD_ASYNC_PRELOAD
   Evas_Cache_Image    *cache;

   assert(im);
   assert(im->cache);

   if (im->flags.loaded) return ;

   cache = im->cache;

   _evas_cache_image_entry_preload_add(cache, im, target);
#else
   evas_cache_image_load_data(im);

   evas_async_events_put(target, EVAS_CALLBACK_IMAGE_PRELOADED, NULL, evas_object_event_callback_call);
#endif
}

EAPI void
evas_cache_image_preload_cancel(Image_Entry *im)
{
#ifdef BUILD_ASYNC_PRELOAD
   Evas_Cache_Image    *cache;

   assert(im);
   assert(im->cache);

   _evas_cache_image_entry_preload_remove(cache, im);
#else
   (void) im;
#endif
}

EAPI int
evas_cache_image_flush(Evas_Cache_Image *cache)
{
   assert(cache);

   if (cache->limit == -1)
     return -1;

   while ((cache->lru) && (cache->limit < cache->usage))
     {
        Image_Entry     *im;

        im = (Image_Entry *) cache->lru->last;
        _evas_cache_image_entry_delete(cache, im);
     }

   while ((cache->lru_nodata) && (cache->limit < cache->usage))
     {
        Image_Entry     *im;

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
   Image_Entry  *new;

   new = _evas_cache_image_entry_new(cache, NULL, 0, NULL, NULL, NULL, NULL);
   if (!new) return NULL;

   new->references = 1;

   return new;
}

EAPI void
evas_cache_image_colorspace(Image_Entry *im, int cspace)
{
   Evas_Cache_Image    *cache;

   assert(im);
   assert(im->cache);

   cache = im->cache;

   if (im->space == cspace) return ;

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

   cache->data = data;
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

#ifdef BUILD_ASYNC_PRELOAD
static void*
_evas_cache_background_load(void *data)
{
   (void) data;

 restart:
   while (preload)
     {
	pthread_mutex_lock(&mutex);

	current = eina_list_data_get(preload);
	preload = eina_list_remove(preload, current);

	pthread_mutex_unlock(&mutex);

	if (current)
	  {
	     Evas_Cache_Image *cache;
	     int error;

	     cache = current->cache;

	     error = cache->func.load(current);
	     if (cache->func.debug)
	       cache->func.debug("load", current);

	     if (error)
	       {
		  _evas_cache_image_entry_surface_alloc(cache, current,
							current->w, current->h);
		  current->flags.loaded = 0;
	       }
	     else
	       {
		  current->flags.loaded = 1;
	       }

	     current->flags.preload = 0;
	  }

	evas_async_events_put(current->target, EVAS_CALLBACK_IMAGE_PRELOADED, NULL, evas_object_event_callback_call);

	current = NULL;
	pthread_cond_signal(&cond);
     }

   pthread_mutex_lock(&mutex);
   if (preload)
     {
	pthread_mutex_unlock(&mutex);
	goto restart;
     }

   running = 0;
   pthread_mutex_unlock(&mutex);

   return NULL;
}
#endif

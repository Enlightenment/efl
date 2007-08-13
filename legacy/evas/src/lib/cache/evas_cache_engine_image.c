#include <assert.h>
#include <Evas.h>

#include "evas_common.h"
#include "evas_private.h"

EAPI int
evas_cache_engine_image_usage_get(Evas_Cache_Engine_Image *cache)
{
   assert(cache != NULL);

   return cache->usage;
}

EAPI int
evas_cache_engine_image_get(Evas_Cache_Engine_Image *cache)
{
   assert(cache != NULL);

   return cache->limit;
}

EAPI void
evas_cache_engine_image_set(Evas_Cache_Engine_Image *cache, int limit)
{
   assert(cache != NULL);

   cache->limit = limit;
}

EAPI Evas_Cache_Engine_Image *
evas_cache_engine_image_init(const Evas_Cache_Engine_Image_Func *cb, Evas_Cache_Image *parent)
{
   Evas_Cache_Engine_Image     *new;

   new = malloc(sizeof (Evas_Cache_Engine_Image));
   if (!new)
     return NULL;

   new->func = *cb;

   new->limit = -1;
   new->usage = 0;

   new->dirty = NULL;
   new->activ = NULL;

   new->parent = parent;
   parent->references++;

   return new;
}

static Evas_Bool
_evas_cache_engine_image_free_cb(Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   Evas_Cache_Engine_Image     *cache = fdata;
   RGBA_Engine_Image           *eim = data;
   RGBA_Image                  *im;

   if (cache->func.debug)
     cache->func.debug("shutdown-engine-activ", eim);

   evas_stringshare_del(eim->cache_key);
   eim->cache_key = NULL;

   im = eim->src;
   cache->func.destructor(eim);
   if (im) evas_cache_image_drop(im);
   free(eim);

   return 1;
}

EAPI void
evas_cache_engine_image_shutdown(Evas_Cache_Engine_Image *cache)
{
   RGBA_Engine_Image   *eim;
   RGBA_Image          *im;

   assert(cache != NULL);

   /* This is mad, I am about to destroy image still alive, but we need to prevent leak. */
   while (cache->dirty)
     {
        eim = (RGBA_Engine_Image *) cache->dirty;
        im = eim->src;

        cache->dirty = evas_object_list_remove(cache->dirty, eim);

        if (cache->func.debug)
          cache->func.debug("shutdown-engine-dirty", eim);
        cache->func.destructor(eim);
        if (im) evas_cache_image_drop(im);
        free(eim);
     }

   evas_hash_foreach(cache->activ, _evas_cache_engine_image_free_cb, cache);
   evas_hash_free(cache->activ);
}

EAPI RGBA_Engine_Image *
evas_cache_engine_image_request(Evas_Cache_Engine_Image *cache, const char *file, const char *key,
                                RGBA_Image_Loadopts *lo, void *data, int *error)
{
   RGBA_Engine_Image   *eim;
   RGBA_Image          *im;
   const char          *ekey;

   assert(cache != NULL);

   im = evas_cache_image_request(cache->parent, file, key, lo, error);
   if (!im)
     {
        *error = -1;
        return NULL;
     }

   if (cache->func.key)
     ekey = cache->func.key(im, file, key, lo, data);
   else
     ekey = evas_stringshare_add(im->cache_key);
   if (!ekey)
     {
        *error = -1;
        evas_cache_image_drop(im);
        return NULL;
     }

   eim = evas_hash_find(cache->activ, ekey);
   if (eim) goto on_ok;

   eim = malloc(sizeof(RGBA_Engine_Image));
   if (!eim) goto on_error;

   eim->src = im;
   eim->engine_data = NULL;
   eim->flags.dirty = 0;
   eim->flags.loaded = 0;
   eim->cache = cache;
   eim->cache_key = ekey;
   eim->references = 0;

   *error = cache->func.constructor(eim, data);
   if (cache->func.debug)
     cache->func.debug("constructor-engine", eim);

   if (*error != 0) goto on_error;

   cache->activ = evas_hash_add(cache->activ, ekey, eim);
   cache->usage += strlen(eim->cache_key) + 1 + cache->func.mem_size_get(eim);

  on_ok:
   eim->references++;
   return eim;

  on_error:
   evas_cache_image_drop(im);
   evas_stringshare_del(ekey);
   if (eim) free(eim);
   return NULL;
}

static void
_evas_cache_engine_image_free(Evas_Cache_Engine_Image *cache, RGBA_Engine_Image *eim)
{
   int                     size;

   size = cache->func.mem_size_get(eim);
   cache->usage -= size;

   if (cache->func.debug)
     cache->func.debug("drop-engine", eim);
   cache->func.destructor(eim);
   if (eim->src) evas_cache_image_drop(eim->src);
   if (eim->cache_key) evas_stringshare_del(eim->cache_key);
   free(eim);
}

EAPI void
evas_cache_engine_image_drop(RGBA_Engine_Image *eim)
{
   Evas_Cache_Engine_Image     *cache;

   assert(eim);
   assert(eim->cache);

   eim->references--;
   cache = eim->cache;

   if (eim->flags.dirty)
     {
        cache->dirty = evas_object_list_remove(cache->dirty, eim);

        _evas_cache_engine_image_free(cache, eim);
        return ;
     }

   if (eim->references == 0)
     {
        cache->activ = evas_hash_del(cache->activ, eim->cache_key, eim);

        _evas_cache_engine_image_free(cache, eim);
        return ;
     }
}

EAPI RGBA_Engine_Image *
evas_cache_engine_image_dirty(RGBA_Engine_Image *eim, int x, int y, int w, int h)
{
   RGBA_Engine_Image           *eim_dirty = eim;
   RGBA_Image                  *im;
   RGBA_Image                  *im_dirty;
   Evas_Cache_Engine_Image     *cache;

   assert(eim);
   assert(eim->cache);

   cache = eim->cache;
   if (!(eim->flags.dirty))
     {
        im = eim->src;
        im_dirty = evas_cache_image_dirty(im, x, y, w, h);

        /* If im == im_dirty, this meens that we have only one reference to the eim. */
        if (im != im_dirty)
          {
             if (eim->references == 1)
               {
                  const char   *hkey;

                  hkey = eim->cache_key;
                  cache->activ = evas_hash_del(cache->activ, hkey, eim);

                  cache->usage -= strlen(hkey) + 1;

                  evas_stringshare_del(hkey);

                  eim_dirty = eim;
                  eim_dirty->src = im_dirty;
               }
             else
               {
                  int           error;

                  eim_dirty = malloc(sizeof(RGBA_Engine_Image));
                  if (!eim_dirty) goto on_error;

                  eim_dirty->src = im_dirty;
                  eim_dirty->engine_data = NULL;
                  eim_dirty->flags.dirty = 1;
                  eim_dirty->flags.loaded = 1;
                  eim_dirty->cache = cache;
                  eim_dirty->cache_key = NULL;
                  eim_dirty->references = 1;

                  error = cache->func.dirty(eim_dirty, eim);
                  if (cache->func.debug)
                    cache->func.debug("dirty-engine", eim_dirty);

                  if (error != 0) goto on_error;

                  cache->usage += cache->func.mem_size_get(eim_dirty);

                  evas_cache_engine_image_drop(eim);
               }

             eim_dirty->cache_key = NULL;
             eim_dirty->flags.dirty = 1;

             cache->dirty = evas_object_list_prepend(cache->dirty, eim_dirty);
          }
     }

   if (cache->func.dirty_region)
     cache->func.dirty_region(eim_dirty, x, y, w, h);
   if (cache->func.debug)
     cache->func.debug("dirty-region-engine", eim_dirty);

   return eim_dirty;

  on_error:
   if (eim) evas_cache_engine_image_drop(eim);
   if (eim_dirty)
     evas_cache_engine_image_drop(eim_dirty);
   else
     if (im_dirty) evas_cache_image_drop(im_dirty);

   return NULL;
}

static RGBA_Engine_Image *
_evas_cache_engine_image_push_dirty(Evas_Cache_Engine_Image *cache, RGBA_Image *im, void *engine_data)
{
   RGBA_Engine_Image    *eim;
   int                  error;

   eim = malloc(sizeof(RGBA_Engine_Image));
   if (!eim) goto on_error;

   eim->src = im;
   eim->engine_data = NULL;
   eim->flags.dirty = 1;
   eim->flags.loaded = 1;
   eim->cache = cache;
   eim->cache_key = NULL;
   eim->references = 1;

   error = cache->func.update_data(eim, engine_data);
   if (cache->func.debug)
     cache->func.debug("dirty-update_data-engine", eim);
   if (error != 0) goto on_error;

   cache->dirty = evas_object_list_prepend(cache->dirty, eim);

   return eim;

  on_error:
   if (eim)
     evas_cache_engine_image_drop(eim);
   else
     evas_cache_image_drop(im);
   return NULL;
}

EAPI RGBA_Engine_Image *
evas_cache_engine_image_copied_data(Evas_Cache_Engine_Image *cache, int w, int h, DATA32 *image_data, int alpha, int cspace, void *engine_data)
{
   RGBA_Image           *im;

   assert(cache);

   im = evas_cache_image_copied_data(cache->parent, w, h, image_data, alpha, cspace);

   return _evas_cache_engine_image_push_dirty(cache, im, engine_data);
}

EAPI RGBA_Engine_Image *
evas_cache_engine_image_data(Evas_Cache_Engine_Image *cache, int w, int h, DATA32 *image_data, int alpha, int cspace, void *engine_data)
{
   RGBA_Image           *im;

   assert(cache);

   im = evas_cache_image_data(cache->parent, w, h, image_data, alpha, cspace);

   return _evas_cache_engine_image_push_dirty(cache, im, engine_data);
}

EAPI RGBA_Engine_Image *
evas_cache_engine_image_size_set(RGBA_Engine_Image *eim, int w, int h)
{
   Evas_Cache_Engine_Image     *cache;
   RGBA_Engine_Image           *new;
   RGBA_Image                  *im;
   int                          error;

   assert(eim);
   assert(eim->src);
   assert(eim->cache);
   assert(eim->references > 0);

   if (eim->src->image->w == w
       && eim->src->image->h == h)
     return eim;

   cache = eim->cache;

   im = evas_cache_image_size_set(eim->src, w, h);
   /* Good idea to call update_data ? */
   if (im == eim->src) return eim;
   eim->src = NULL;

   new = malloc(sizeof(RGBA_Engine_Image));
   if (!new) goto on_error;

   new->src = im;
   new->engine_data = NULL;
   new->flags = eim->flags;
   new->flags.loaded = 1;
   new->cache = cache;
   new->cache_key = NULL;
   new->references = 1;

   error = cache->func.size_set(new, eim);
   if (error) goto on_error;

   assert(new->engine_data != eim->engine_data);

   cache->usage += cache->func.mem_size_get(new);

   if (new->flags.dirty || eim->references > 1)
     {
        new->flags.dirty = 1;
        cache->dirty = evas_object_list_prepend(cache->dirty, new);
     }
   else
     {
        const char *cache_key = NULL;

        cache_key = eim->cache_key ? evas_stringshare_add(eim->cache_key) : NULL;
        new->cache_key = cache_key;

        cache->activ = evas_hash_add(cache->activ, cache_key, new);
        cache->usage += strlen(new->cache_key) + 1;
     }

   evas_cache_engine_image_drop(eim);
   return new;

  on_error:
   if (new)
     evas_cache_engine_image_drop(new);
   else
     if (im)
       evas_cache_image_drop(im);
   evas_cache_engine_image_drop(eim);

   return NULL;
}

EAPI void
evas_cache_engine_image_load_data(RGBA_Engine_Image *eim)
{
   Evas_Cache_Engine_Image     *cache;
   int                          size;

   assert(eim);
   assert(eim->src);
   assert(eim->cache);

   if (eim->flags.loaded) return ;

   evas_cache_image_load_data(eim->src);

   cache = eim->cache;
   if (cache->func.debug)
     cache->func.debug("load-engine", eim);

   size = cache->func.mem_size_get(eim);
   cache = eim->cache;
   cache->func.load(eim, eim->src);
   cache->usage += cache->func.mem_size_get(eim) - size;

   eim->flags.loaded = 1;
}

EAPI RGBA_Engine_Image *
evas_cache_engine_image_engine(Evas_Cache_Engine_Image *cache, void *engine_data)
{
   RGBA_Engine_Image   *eim;
   int                  error;

   eim = malloc(sizeof(RGBA_Engine_Image));
   if (!eim) goto on_error;

   eim->src = evas_cache_image_empty(cache->parent);
   if (!eim->src) goto on_error;

   eim->engine_data = NULL;
   eim->flags.dirty = 1;
   eim->flags.loaded = 1;
   eim->cache = cache;
   eim->cache_key = NULL;
   eim->references = 1;

   error = cache->func.update_data(eim, engine_data);
   if (cache->func.debug)
     cache->func.debug("update_data-engine", eim);

   if (error != 0) goto on_error;

   cache->dirty = evas_object_list_prepend(cache->dirty, eim);

   return eim;

  on_error:
   if (eim)
     evas_cache_engine_image_drop(eim);
   return NULL;
}

EAPI void
evas_cache_engine_image_colorspace(RGBA_Engine_Image *eim, int cspace, void *engine_data)
{
   Evas_Cache_Engine_Image     *cache = eim->cache;

   assert(cache);

   cache->func.destructor(eim);
   evas_cache_image_colorspace(eim->src, cspace);
   cache->func.constructor(eim, engine_data);
   if (cache->func.debug)
     cache->func.debug("cosntructor-colorspace-engine", eim);
}


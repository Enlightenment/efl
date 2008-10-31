/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <assert.h>

#include "evas_common.h"
#include "evas_private.h"

static void
_evas_cache_engine_image_make_dirty(Evas_Cache_Engine_Image *cache,
                                    Engine_Image_Entry *eim)
{
   eim->flags.cached = 1;
   eim->flags.dirty = 1;
   eim->flags.loaded = 1;
   eim->flags.activ = 0;
   cache->dirty = eina_inlist_prepend(cache->dirty, EINA_INLIST_GET(eim));
}

static void
_evas_cache_engine_image_make_active(Evas_Cache_Engine_Image *cache,
                                     Engine_Image_Entry *eim,
                                     const char *key)
{
   eim->flags.cached = 1;
   eim->flags.activ = 1;
   eim->flags.dirty = 0;
   cache->activ = evas_hash_add(cache->activ, key, eim);
}

static void
_evas_cache_engine_image_make_inactive(Evas_Cache_Engine_Image *cache,
                                       Engine_Image_Entry *eim,
                                       const char *key)
{
   eim->flags.cached = 1;
   eim->flags.dirty = 0;
   eim->flags.activ = 0;
   cache->inactiv = evas_hash_add(cache->inactiv, key, eim);
   cache->lru = eina_inlist_prepend(cache->lru, EINA_INLIST_GET(eim));
   cache->usage += cache->func.mem_size_get(eim);
}

static void
_evas_cache_engine_image_remove_activ(Evas_Cache_Engine_Image *cache,
                                      Engine_Image_Entry *eim)
{
   if (eim->flags.cached)
     {
        if (eim->flags.dirty)
          {
	     cache->dirty = eina_inlist_remove(cache->dirty, EINA_INLIST_GET(eim));
          }
        else
          if (eim->flags.activ)
            {
               cache->activ = evas_hash_del(cache->activ, eim->cache_key, eim);
            }
          else
            {
               cache->usage -= cache->func.mem_size_get(eim);
               cache->inactiv = evas_hash_del(cache->inactiv, eim->cache_key, eim);
               cache->lru = eina_inlist_remove(cache->lru, EINA_INLIST_GET(eim));
            }
        eim->flags.cached = 0;
        eim->flags.dirty = 0;
        eim->flags.activ = 0;
     }
}

static Engine_Image_Entry *
_evas_cache_engine_image_alloc(Evas_Cache_Engine_Image *cache,
                               Image_Entry *ie,
                               const char *hkey)
{
   Engine_Image_Entry   *eim;

   assert(cache);

   if (cache->func.alloc)
     eim = cache->func.alloc();
   else
     eim = malloc(sizeof (Engine_Image_Entry));

   if (!eim) goto on_error;
   memset(eim, 0, sizeof (Engine_Image_Entry));

   eim->cache = cache;
   if (ie)
     {
        eim->w = ie->w;
        eim->h = ie->h;
        eim->src = ie;
        eim->flags.need_parent = 1;
     }
   else
     {
        eim->w = -1;
        eim->h = -1;
        eim->flags.need_parent = 0;
        eim->src = NULL;
     }

   eim->flags.cached = 0;
   eim->references = 0;
   eim->cache_key = hkey;

   if (hkey)
     _evas_cache_engine_image_make_active(cache, eim, hkey);
   else
     _evas_cache_engine_image_make_dirty(cache, eim);

   return eim;

 on_error:
   if (eim)
     evas_cache_engine_image_drop(eim);
   eina_stringshare_del(hkey);
   evas_cache_image_drop(ie);
   return NULL;
}

static void
_evas_cache_engine_image_dealloc(Evas_Cache_Engine_Image *cache, Engine_Image_Entry *eim)
{
   Image_Entry   *im;

   if (cache->func.debug) cache->func.debug("delete", eim);

   _evas_cache_engine_image_remove_activ(cache, eim);

   im = eim->src;
   cache->func.destructor(eim);
   if (im) evas_cache_image_drop(im);

   if (cache->func.dealloc)
     {
        cache->func.dealloc(eim);
     }
   else
     {
        memset(eim, 0, sizeof (Engine_Image_Entry));
        free(eim);
     }
}

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

   new->limit = 0;
   new->usage = 0;

   new->dirty = NULL;
   new->lru = NULL;
   new->activ = NULL;
   new->inactiv = NULL;

   new->parent = parent;
   parent->references++;

   new->brother = NULL;

   return new;
}

EAPI Evas_Cache_Engine_Image *
evas_cache_engine_image_dup(const Evas_Cache_Engine_Image_Func *cb, Evas_Cache_Engine_Image *brother)
{
   Evas_Cache_Engine_Image     *new;

   new = malloc(sizeof (Evas_Cache_Engine_Image));
   if (!new)
     return NULL;

   new->func = brother->func;

#define ORD(Func) if (cb->Func) new->func.Func = cb->Func;

   ORD(key);
   ORD(constructor);
   ORD(destructor);
   ORD(dirty_region);
   ORD(dirty);
   ORD(size_set);
   ORD(update_data);
   ORD(load);
   ORD(mem_size_get);
   ORD(debug);

#undef ORD

   new->limit = -1;
   new->usage = 0;
   new->references = 1;

   new->dirty = NULL;
   new->activ = NULL;

   new->parent = brother->parent;
   new->parent->references++;

   new->brother = brother;
   brother->references++;

   return new;
}

static Evas_Bool
_evas_cache_engine_image_free_cb(__UNUSED__ const Evas_Hash *hash, __UNUSED__ const void *key, void *data, void *fdata)
{
   Eina_List **delete_list = fdata;

   *delete_list = eina_list_prepend(*delete_list, data);

   return 1;
}

EAPI void
evas_cache_engine_image_flush(Evas_Cache_Engine_Image *cache)
{
   assert(cache != NULL);

   while ((cache->lru) && (cache->limit < cache->usage))
     {
        Engine_Image_Entry      *eim;

        eim = (Engine_Image_Entry *) cache->lru->last;
        _evas_cache_engine_image_dealloc(cache, eim);
     }
}

EAPI void
evas_cache_engine_image_shutdown(Evas_Cache_Engine_Image *cache)
{
   Engine_Image_Entry   *eim;
   Eina_List *delete_list = NULL;

   assert(cache != NULL);

   if (cache->func.debug) cache->func.debug("shutdown-engine", NULL);

   evas_hash_foreach(cache->inactiv, _evas_cache_engine_image_free_cb, &delete_list);
   evas_hash_foreach(cache->activ, _evas_cache_engine_image_free_cb, &delete_list);

   while (delete_list)
     {
	_evas_cache_engine_image_dealloc(cache, eina_list_data_get(delete_list));
	delete_list = eina_list_remove_list(delete_list, delete_list);
     }

   evas_hash_free(cache->inactiv);
   evas_hash_free(cache->activ);

   /* This is mad, I am about to destroy image still alive, but we need to prevent leak. */
   while (cache->dirty)
     {
        eim = (Engine_Image_Entry *) cache->dirty;
        _evas_cache_engine_image_dealloc(cache, eim);
     }


   evas_cache_image_shutdown(cache->parent);
   if (cache->brother)
     evas_cache_engine_image_shutdown(cache->brother);
   free(cache);
}

EAPI Engine_Image_Entry *
evas_cache_engine_image_request(Evas_Cache_Engine_Image *cache,
                                const char *file, const char *key,
                                RGBA_Image_Loadopts *lo, void *data, int *error)
{
   Engine_Image_Entry   *eim;
   Image_Entry          *im;
   const char          *ekey;

   assert(cache != NULL);

   *error = -1;

   ekey = NULL;
   eim = NULL;

   im = evas_cache_image_request(cache->parent, file, key, lo, error);
   if (!im)
     goto on_error;

   if (cache->func.key)
     ekey = cache->func.key(im, file, key, lo, data);
   else
     ekey = eina_stringshare_add(im->cache_key);
   if (!ekey)
     goto on_error;

   eim = evas_hash_find(cache->activ, ekey);
   if (eim)
     {
        evas_cache_image_drop(im);
        goto on_ok;
     }

   eim = evas_hash_find(cache->inactiv, ekey);
   if (eim)
     {
        _evas_cache_engine_image_remove_activ(cache, eim);
        _evas_cache_engine_image_make_active(cache, eim, ekey);
        evas_cache_image_drop(im);
        goto on_ok;
     }

   eim = _evas_cache_engine_image_alloc(cache, im, ekey);
   if (!eim) return NULL;

   *error = cache->func.constructor(eim, data);
   if (*error != 0) goto on_error;
   if (cache->func.debug)
     cache->func.debug("constructor-engine", eim);

  on_ok:
   eim->references++;
   return eim;

  on_error:
   if (!eim)
     {
        if (im) evas_cache_image_drop(im);
        if (ekey) eina_stringshare_del(ekey);
     }
   else
     {
        _evas_cache_engine_image_dealloc(cache, eim);
     }

   return NULL;
}

EAPI void
evas_cache_engine_image_drop(Engine_Image_Entry *eim)
{
   Evas_Cache_Engine_Image     *cache;

   assert(eim);
   assert(eim->cache);

   eim->references--;
   cache = eim->cache;

   if (eim->flags.dirty)
     {
        _evas_cache_engine_image_dealloc(cache, eim);
        return ;
     }

   if (eim->references == 0)
     {
        _evas_cache_engine_image_remove_activ(cache, eim);
        _evas_cache_engine_image_make_inactive(cache, eim, eim->cache_key);
        evas_cache_engine_image_flush(cache);
        return ;
     }
}

EAPI Engine_Image_Entry *
evas_cache_engine_image_dirty(Engine_Image_Entry *eim, int x, int y, int w, int h)
{
   Engine_Image_Entry           *eim_dirty = eim;
   Image_Entry                  *im_dirty = NULL;
   Image_Entry                  *im;
   Evas_Cache_Engine_Image      *cache;
   unsigned char                 alloc_eim;

   assert(eim);
   assert(eim->cache);

   cache = eim->cache;
   if (!(eim->flags.dirty))
     {
        alloc_eim = 0;

        if (eim->flags.need_parent == 1)
          {
             im = eim->src;
             im_dirty = evas_cache_image_dirty(im, x, y, w, h);

             /* If im == im_dirty, this meens that we have only one reference to the eim. */
             if (im != im_dirty)
               {
                  if (eim->references == 1)
                    {
                       _evas_cache_engine_image_remove_activ(cache, eim);
                       _evas_cache_engine_image_make_dirty(cache, eim);

                       eim->src = im_dirty;
                    }
                  else
                    alloc_eim = 1;
               }
          }
        else
          if (eim->references > 1)
            {
               alloc_eim = 1;
            }
          else
            {
               _evas_cache_engine_image_remove_activ(cache, eim_dirty);
               _evas_cache_engine_image_make_dirty(cache, eim_dirty);
            }

        if (alloc_eim == 1)
          {
             int           error;

             eim_dirty = _evas_cache_engine_image_alloc(cache, im_dirty, NULL);
             if (!eim_dirty) goto on_error;

             eim_dirty->w = eim->w;
             eim_dirty->h = eim->h;
             eim_dirty->references = 1;

             error = cache->func.dirty(eim_dirty, eim);
             if (cache->func.debug)
               cache->func.debug("dirty-engine", eim_dirty);

             if (error != 0) goto on_error;

             evas_cache_engine_image_drop(eim);
          }
     }

   if (cache->func.dirty_region)
     cache->func.dirty_region(eim_dirty, x, y, w, h);
   if (cache->func.debug)
     cache->func.debug("dirty-region-engine", eim_dirty);

   return eim_dirty;

  on_error:
   if (eim) evas_cache_engine_image_drop(eim);
   if (eim_dirty && eim_dirty != eim)
     evas_cache_engine_image_drop(eim_dirty);
   else
     if (im_dirty) evas_cache_image_drop(im_dirty);

   return NULL;
}

EAPI Engine_Image_Entry *
evas_cache_engine_image_alone(Engine_Image_Entry *eim, void *data)
{
   Evas_Cache_Engine_Image      *cache;
   Image_Entry                  *im;


   assert(eim);
   assert(eim->cache);

   cache = eim->cache;
   im = evas_cache_image_alone(eim->src);
   if (im != eim->src)
     {
        eim = _evas_cache_engine_image_alloc(cache, im, NULL);
        if (!eim) goto on_error;

        eim->references = 1;

        if (cache->func.constructor(eim, data)) goto on_error;
     }
   /* FIXME */
   return eim;

 on_error:
   evas_cache_image_drop(im);
   return NULL;
}

static Engine_Image_Entry *
_evas_cache_engine_image_push_dirty(Evas_Cache_Engine_Image *cache, Image_Entry *im, void *engine_data)
{
   Engine_Image_Entry    *eim;
   int                  error;

   eim = _evas_cache_engine_image_alloc(cache, im, NULL);
   if (!eim) goto on_error;
   eim->references = 1;

   error = cache->func.update_data(eim, engine_data);
   if (cache->func.debug)
     cache->func.debug("dirty-update_data-engine", eim);
   if (error != 0) goto on_error;

   return eim;

  on_error:
   if (eim)
     evas_cache_engine_image_drop(eim);
   return NULL;
}

EAPI Engine_Image_Entry *
evas_cache_engine_image_copied_data(Evas_Cache_Engine_Image *cache, int w, int h, DATA32 *image_data, int alpha, int cspace, void *engine_data)
{
   Image_Entry           *im;

   assert(cache);

   im = evas_cache_image_copied_data(cache->parent, w, h, image_data, alpha, cspace);

   return _evas_cache_engine_image_push_dirty(cache, im, engine_data);
}

EAPI Engine_Image_Entry *
evas_cache_engine_image_data(Evas_Cache_Engine_Image *cache, int w, int h, DATA32 *image_data, int alpha, int cspace, void *engine_data)
{
   Image_Entry           *im;

   assert(cache);

   im = evas_cache_image_data(cache->parent, w, h, image_data, alpha, cspace);

   return _evas_cache_engine_image_push_dirty(cache, im, engine_data);
}

EAPI Engine_Image_Entry *
evas_cache_engine_image_size_set(Engine_Image_Entry *eim, int w, int h)
{
   Evas_Cache_Engine_Image      *cache;
   Engine_Image_Entry           *new;
   Image_Entry                  *im;
   const char                   *hkey;
   int                           error;

   assert(eim);
   assert(eim->cache);
   assert(eim->references > 0);

   im = NULL;
   cache = eim->cache;

   if (eim->flags.need_parent == 1)
     {
        assert(eim->src);

        if (eim->src->w == w
            && eim->src->h == h)
          return eim;

        im = evas_cache_image_size_set(eim->src, w, h);
        /* FIXME: Good idea to call update_data ? */
        if (im == eim->src) return eim;
        eim->src = NULL;
     }

   hkey = (eim->references > 1 ) ? eina_stringshare_add(eim->cache_key) : NULL;

   new = _evas_cache_engine_image_alloc(cache, im, hkey);
   if (!new) goto on_error;

   new->w = w;
   new->h = h;
   new->references = 1;

   error = cache->func.size_set(new, eim);
   if (error) goto on_error;

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
evas_cache_engine_image_load_data(Engine_Image_Entry *eim)
{
   Evas_Cache_Engine_Image     *cache;
   int                          size = 0;

   assert(eim);
   assert(eim->src);
   assert(eim->cache);

   if (eim->flags.loaded) return ;

   if (eim->src)
     evas_cache_image_load_data(eim->src);

   cache = eim->cache;
   if (cache->func.debug)
     cache->func.debug("load-engine", eim);

   if (eim->flags.dirty)
     size = cache->func.mem_size_get(eim);
   cache = eim->cache;
   cache->func.load(eim, eim->src);
   if (eim->flags.dirty)
     cache->usage += cache->func.mem_size_get(eim) - size;

   eim->flags.loaded = 1;
}

EAPI Engine_Image_Entry *
evas_cache_engine_image_engine(Evas_Cache_Engine_Image *cache, void *engine_data)
{
   Engine_Image_Entry   *eim;
   Image_Entry          *ie;
   int                   error;

   ie = evas_cache_image_empty(cache->parent);
   if (!ie) return NULL;

   eim = _evas_cache_engine_image_alloc(cache, ie, NULL);
   if (!eim) goto on_error;
   eim->references = 1;

   error = cache->func.update_data(eim, engine_data);
   if (cache->func.debug)
     cache->func.debug("update_data-engine", eim);

   if (error != 0) goto on_error;

   return eim;

  on_error:
   if (!eim)
     {
        if (ie)
          evas_cache_image_drop(ie);
     }
   else
     {
        evas_cache_engine_image_drop(eim);
     }
   return NULL;
}

EAPI void
evas_cache_engine_image_colorspace(Engine_Image_Entry *eim, int cspace, void *engine_data)
{
   Evas_Cache_Engine_Image     *cache = eim->cache;

   assert(cache);

   cache->func.destructor(eim);
   evas_cache_image_colorspace(eim->src, cspace);
   cache->func.constructor(eim, engine_data);
   if (cache->func.debug)
     cache->func.debug("cosntructor-colorspace-engine", eim);
}

EAPI void
evas_cache_engine_parent_not_needed(Engine_Image_Entry *eim)
{
   assert(eim);
   assert(eim->cache);

   eim->flags.need_parent = 0;
   evas_cache_image_data_not_needed(eim->src);
}

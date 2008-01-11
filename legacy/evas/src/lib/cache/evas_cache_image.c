#include <stdlib.h>
#include <assert.h>
#include <Evas.h>

#include "evas_common.h"
#include "evas_private.h"

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
   new->inactiv = NULL;
   new->activ = NULL;

   new->references = 1;

   return new;
}

static Evas_Bool
_evas_cache_image_free_cb(Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   Evas_Cache_Image    *cache = fdata;
   RGBA_Image          *im = data;

   if (cache->func.debug)
     cache->func.debug("shutdown-activ", im);

   if (im->cache_key)
     {
	evas_stringshare_del(im->cache_key);
	im->cache_key = NULL;
     }

   cache->func.destructor(im);
   evas_common_image_delete(im);
   return 1;
}

EAPI void
evas_cache_image_shutdown(Evas_Cache_Image *cache)
{
   RGBA_Image  *im;

   assert(cache != NULL);
   cache->references--;

   if (cache->references > 0)
     return ;

   while (cache->lru)
     {
        im = (RGBA_Image *) cache->lru;
        cache->lru = evas_object_list_remove(cache->lru, im);

	if (im->cache_key)
	  {
	     evas_stringshare_del(im->cache_key);
	     im->cache_key = NULL;
	  }

        if (cache->func.debug)
          cache->func.debug("shutdown-lru", im);
        cache->func.destructor(im);
        evas_common_image_delete(im);
     }

   /* This is mad, I am about to destroy image still alive, but we need to prevent leak. */
   while (cache->dirty)
     {
        im = (RGBA_Image *) cache->dirty;
        cache->dirty = evas_object_list_remove(cache->dirty, im);

        if (cache->func.debug)
          cache->func.debug("shutdown-dirty", im);
        cache->func.destructor(im);
        evas_common_image_delete(im);
     }

   evas_hash_foreach(cache->activ, _evas_cache_image_free_cb, cache);
   evas_hash_free(cache->activ);
   evas_hash_free(cache->inactiv);

   free(cache);
}

EAPI RGBA_Image *
evas_cache_image_request(Evas_Cache_Image *cache, const char *file, const char *key, RGBA_Image_Loadopts *lo, int *error)
{
   const char           *format;
   char                 *hkey;
   RGBA_Image           *im;
   Evas_Image_Load_Opts  prevent;
   int                   size;
   struct stat           st;

   assert(cache != NULL);

   if (!file && !key) return NULL;
   if (!file) return NULL;
   if ((!lo) ||
       (lo &&
        (lo->scale_down_by == 0) &&
        (lo->dpi = 0.0) &&
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
        if (key)
          format = "%s//://%s//@/%i/%1.5f/%ix%i";
        else
          format = "%s//://%p//@/%i/%1.5f/%ix%i";
     }
   size = strlen(file) + (key ? strlen(key) : 6) + 64;
   hkey = alloca(sizeof (char) * size);
   snprintf(hkey, size, format, file, key, lo->scale_down_by, lo->dpi, lo->w, lo->h);

   if (stat(file, &st) < 0)
     {
	im = evas_hash_find(cache->inactiv, hkey);
	if (im)
	  {
	     cache->lru = evas_object_list_remove(cache->lru, im);
	     cache->inactiv = evas_hash_del(cache->inactiv, im->cache_key, im);
//	     printf("IMG %p %ix%i %s SUB %i\n",
//		    im, im->image->w, im->image->h, im->cache_key,
//		    cache->func.mem_size_get(im));
	     cache->usage -= cache->func.mem_size_get(im);
	     if (im->cache_key)
	       {
		  evas_stringshare_del(im->cache_key);
		  im->cache_key = NULL;
	       }
	     cache->func.destructor(im);
	     evas_common_image_delete(im);
	  }
	return NULL;
     }

   im = evas_hash_find(cache->activ, hkey);
   if (im)
     {
	if (st.st_mtime == im->timestamp)
	  goto on_ok;
     }

   im = evas_hash_find(cache->inactiv, hkey);
   if (im)
     {
	if (st.st_mtime == im->timestamp)
	  {
	     cache->lru = evas_object_list_remove(cache->lru, im);
	     cache->inactiv = evas_hash_del(cache->inactiv, im->cache_key, im);
	     cache->activ = evas_hash_direct_add(cache->activ, im->cache_key, im);
//	     printf("IMG %p %ix%i %s SUB %i\n",
//		    im, im->image->w, im->image->h, im->cache_key,
//		    cache->func.mem_size_get(im));
	     cache->usage -= cache->func.mem_size_get(im);
	     goto on_ok;
	  }
	else
	  {
	     cache->lru = evas_object_list_remove(cache->lru, im);
	     cache->inactiv = evas_hash_del(cache->inactiv, im->cache_key, im);
//	     printf("IMG %p %ix%i %s SUB %i\n",
//		    im, im->image->w, im->image->h, im->cache_key,
//		    cache->func.mem_size_get(im));
	     cache->usage -= cache->func.mem_size_get(im);
	     if (im->cache_key)
	       {
		  evas_stringshare_del(im->cache_key);
		  im->cache_key = NULL;
	       }
	     cache->func.destructor(im);
	     evas_common_image_delete(im);
	  }
     }

   im = evas_common_image_new();
   if (!im)
     {
        *error = -1;
        return NULL;
     }

   im->timestamp = st.st_mtime;
   im->laststat = time(NULL);

   if (lo) im->load_opts = *lo;

   im->info.file = (char *) evas_stringshare_add(file);
   if (key) im->info.key = (char *) evas_stringshare_add(key);

   *error = cache->func.constructor(im);
   if (*error != 0)
     {
        evas_common_image_delete(im);
        return NULL;
     }

   if (cache->func.debug)
     cache->func.debug("request", im);

   im->references = 0;
   im->cache_key = evas_stringshare_add(hkey);
   im->cache = cache;
   
   cache->activ = evas_hash_direct_add(cache->activ, im->cache_key, im);
   
   on_ok:
   *error = 0;
   im->references++;
   return im;
}

EAPI void
evas_cache_image_drop(RGBA_Image *im)
{
   Evas_Cache_Image    *cache;

   assert(im);
   assert(im->cache);

   im->references--;
   cache = im->cache;

//   if (im->cache_key) printf("DROP %s -> ref = %i\n", im->cache_key, im->references);
   if ((im->flags & RGBA_IMAGE_IS_DIRTY) == RGBA_IMAGE_IS_DIRTY)
     {
//	printf("IMG %p %ix%i %s SUB %i\n",
//	       im, im->image->w, im->image->h, im->cache_key,
//	       cache->func.mem_size_get(im));
//// don't decrement cache usage - unless we remove from the lru	
//        cache->usage -= cache->func.mem_size_get(im);
//	if (im->cache_key) printf("IM-- %s, cache = %i\n", im->cache_key, cache->usage);
        cache->dirty = evas_object_list_remove(cache->dirty, im);
        if (cache->func.debug)
          cache->func.debug("drop", im);

        cache->func.destructor(im);
        evas_common_image_delete(im);

        return ;
     }

   if (im->references == 0)
     {
        cache->activ = evas_hash_del(cache->activ, im->cache_key, im);
        cache->inactiv = evas_hash_direct_add(cache->inactiv, im->cache_key, im);
        cache->lru = evas_object_list_prepend(cache->lru, im);

//	printf("IMG %p %ix%i %s ADD %i\n",
//	       im, im->image->w, im->image->h, im->cache_key,
//	       cache->func.mem_size_get(im));
	cache->usage += cache->func.mem_size_get(im);
//	printf("FLUSH!\n");
	evas_cache_image_flush(cache);
     }
}

EAPI RGBA_Image *
evas_cache_image_dirty(RGBA_Image *im, int x, int y, int w, int h)
{
   RGBA_Image          *im_dirty = im;
   Evas_Cache_Image    *cache;

   assert(im);
   assert(im->cache);

   cache = im->cache;
   if (!(im->flags & RGBA_IMAGE_IS_DIRTY))
     {
        if (im->references == 1)
          {
	     if (im->cache_key)
	       {
		  cache->activ = evas_hash_del(cache->activ, im->cache_key, im);
		  evas_stringshare_del(im->cache_key);
		  im->cache_key = NULL;
	       }
             im_dirty = im;
          }
        else
          {
             int        error;

             im_dirty = evas_common_image_new();
             if (!im_dirty) goto on_error;
             im_dirty->image = evas_common_image_surface_new(im);
             if (!im_dirty->image) goto on_error;
             im_dirty->image->w = w;
             im_dirty->image->h = h;

             if (cache->func.debug)
               cache->func.debug("dirty-src", im);
             error = cache->func.dirty(im_dirty, im);
             if (cache->func.debug)
               cache->func.debug("dirty-out", im_dirty);

             if (error != 0) goto on_error;

             im_dirty->cache = cache;
             im_dirty->references = 1;

             evas_cache_image_drop(im);
          }

        im_dirty->flags |= RGBA_IMAGE_IS_DIRTY;
        cache->dirty = evas_object_list_prepend(cache->dirty, im_dirty);
     }
   
   if (cache->func.debug)
     cache->func.debug("dirty-region", im_dirty);
   if (cache->func.dirty_region)
     cache->func.dirty_region(im_dirty, x, y, w, h);
   
   return im_dirty;
   
   on_error:
   if (im_dirty) evas_common_image_delete(im_dirty);
   evas_cache_image_drop(im);
   return NULL;
}

EAPI RGBA_Image *
evas_cache_image_alone(RGBA_Image *im)
{
   RGBA_Image          *im_dirty = im;
   Evas_Cache_Image    *cache;

   assert(im);
   assert(im->cache);

   cache = im->cache;
   if (im->references == 1)
     {
        if (!(im->flags & RGBA_IMAGE_IS_DIRTY))
          {
             if (im->cache_key)
	       {
		  cache->activ = evas_hash_del(cache->activ, im->cache_key, im);
		  evas_stringshare_del(im->cache_key);
		  im->cache_key = NULL;
	       }
             im->flags |= RGBA_IMAGE_IS_DIRTY;
             cache->dirty = evas_object_list_prepend(cache->dirty, im);
          }
     }
   else
     {
        int     error;

        im_dirty = evas_common_image_new();
        if (!im_dirty) goto on_error;
        im_dirty->image = evas_common_image_surface_new(im);
        if (!im_dirty->image) goto on_error;
        im_dirty->image->w = im->image->w;
        im_dirty->image->h = im->image->h;

        if (cache->func.debug)
          cache->func.debug("dirty-src", im);
        error = cache->func.dirty(im_dirty, im);
        if (cache->func.debug)
          cache->func.debug("dirty-out", im_dirty);

        if (error != 0) goto on_error;

	if (im_dirty->cache_key)
	  {
	     evas_stringshare_del(im_dirty->cache_key);
	     im_dirty->cache_key = NULL;
	  }
        im_dirty->flags |= RGBA_IMAGE_IS_DIRTY;
        im_dirty->references = 1;

        cache->dirty = evas_object_list_prepend(cache->dirty, im_dirty);

        evas_cache_image_drop(im);
     }

   return im_dirty;
   
   on_error:
   if (im_dirty) evas_common_image_delete(im_dirty);
   evas_cache_image_drop(im);
   return NULL;
}

static RGBA_Image *
_evas_cache_image_push_dirty(Evas_Cache_Image *cache, RGBA_Image *im)
{
   cache->dirty = evas_object_list_prepend(cache->dirty, im);

   im->flags |= RGBA_IMAGE_IS_DIRTY;
   if (im->cache_key)
     {
	evas_stringshare_del(im->cache_key);
	im->cache_key = NULL;
     }
   im->cache = cache;
   return im;
}

EAPI RGBA_Image *
evas_cache_image_copied_data(Evas_Cache_Image *cache, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   RGBA_Image  *im;

   assert(cache);

   if ((cspace == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (cspace == EVAS_COLORSPACE_YCBCR422P709_PL))
     w &= ~0x1;

   im = evas_common_image_create(w, h);
   if (!im) return NULL;

   if (cache->func.copied_data(im, w, h, image_data, alpha, cspace) != 0)
     {
        evas_common_image_delete(im);
        return NULL;
     }

   return _evas_cache_image_push_dirty(cache, im);
}

EAPI RGBA_Image *
evas_cache_image_data(Evas_Cache_Image *cache, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   RGBA_Image  *im;

   assert(cache);

   im = evas_common_image_new();
   if (!im) return NULL;
   im->image = evas_common_image_surface_new(im);
   if (!im->image)
     {
        evas_common_image_delete(im);
        return NULL;
     }

   if (cache->func.data(im, w, h, image_data, alpha, cspace) != 0)
     {
        evas_common_image_delete(im);
        return NULL;
     }

   return _evas_cache_image_push_dirty(cache, im);
}

EAPI RGBA_Image *
evas_cache_image_size_set(RGBA_Image *im, int w, int h)
{
   Evas_Cache_Image    *cache;
   RGBA_Image          *new;
   int                  error;

   assert(im);
   assert(im->image);
   assert(im->cache);
   assert(im->references > 0);

   if ((im->image->w == w) && (im->image->h == h))
     return im;

   cache = im->cache;

   new = evas_common_image_new();
   if (!new) goto on_error;
   new->image = evas_common_image_surface_new(im);
   if (!new->image) goto on_error;
   new->image->w = w;
   new->image->h = h;

   if (cache->func.debug)
     cache->func.debug("size_set-in", im);
   error = cache->func.size_set(new, im, w, h);
   if (cache->func.debug)
     cache->func.debug("size_set-out", new);

   if (error != 0) goto on_error;

   new->cache = cache;
   new->cache_key = NULL;

   new->references = 1;
//   cache->usage += cache->func.mem_size_get(new);

   if (((im->flags & RGBA_IMAGE_IS_DIRTY) == RGBA_IMAGE_IS_DIRTY)
       || (im->references > 1))
     {
        new->flags |= RGBA_IMAGE_IS_DIRTY;
        cache->dirty = evas_object_list_prepend(cache->dirty, new);
     }
   else
     {
        new->cache_key = im->cache_key ? evas_stringshare_add(im->cache_key) : NULL;
        cache->activ = evas_hash_direct_add(cache->activ, new->cache_key, new);
     }

   evas_cache_image_drop(im);
   
   return new;
   
   on_error:
   if (new) evas_common_image_delete(new);
   evas_cache_image_drop(im);
   return NULL;
}

EAPI void
evas_cache_image_load_data(RGBA_Image *im)
{
   Evas_Cache_Image    *cache;

   assert(im);
   assert(im->image);
   assert(im->cache);

   if ((im->flags & RGBA_IMAGE_LOADED) == RGBA_IMAGE_LOADED) return ;
   
   cache = im->cache;
   if (cache->func.debug)
     cache->func.debug("load", im);

   cache->func.load(im);
   im->flags |= RGBA_IMAGE_LOADED;

   assert(im->image->data);
}

EAPI int
evas_cache_image_flush(Evas_Cache_Image *cache)
{
   assert(cache);

//   printf("cache->limit = %i (used = %i)\n", cache->limit, cache->usage);
   if (cache->limit == -1)
     return -1;

   while ((cache->lru) && (cache->limit < cache->usage))
     {
        RGBA_Image     *im;

        im = (RGBA_Image *) cache->lru->last;
        cache->lru = evas_object_list_remove(cache->lru, im);
        cache->inactiv = evas_hash_del(cache->inactiv, im->cache_key, im);
//	printf("IMG %p %ix%i %s SUB %i\n",
//	       im, im->image->w, im->image->h, im->cache_key,
//	       cache->func.mem_size_get(im));
        cache->usage -= cache->func.mem_size_get(im);

	if (im->cache_key)
	  {
	     evas_stringshare_del(im->cache_key);
	     im->cache_key = NULL;
	  }
//	printf("DEL IMG FROM CACHE\n");
        cache->func.destructor(im);
        evas_common_image_delete(im);
     }
   return cache->usage;
}

EAPI RGBA_Image *
evas_cache_image_empty(Evas_Cache_Image *cache)
{
   RGBA_Image  *new;

   new = evas_common_image_new();
   if (!new) goto on_error;
   new->image = evas_common_image_surface_new(new);
   if (!new->image) goto on_error;

   new->cache = cache;
   new->references = 1;

   new->cache_key = NULL;
   new->flags |= RGBA_IMAGE_IS_DIRTY;

   cache->dirty = evas_object_list_prepend(cache->dirty, new);

   return new;

  on_error:
   if (new) evas_common_image_delete(new);
   return NULL;
}

EAPI void
evas_cache_image_colorspace(RGBA_Image *im, int cspace)
{
   if (!im) return ;
   if (im->cs.space == cspace) return ;

   evas_common_image_colorspace_set(im, cspace);
}

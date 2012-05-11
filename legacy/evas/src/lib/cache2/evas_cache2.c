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
#include "evas_cache2.h"
#include "evas_cs2.h"
#include "evas_cs2_private.h"

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
static void _evas_cache2_image_entry_preload_remove(Image_Entry *ie, const void *target);
// static void _evas_cache_image_lru_nodata_add(Image_Entry *im);
// static void _evas_cache_image_lru_nodata_del(Image_Entry *im);

static void
_evas_cache_image_dirty_add(Image_Entry *im)
{
   if (im->flags.dirty) return;
   _evas_cache_image_activ_del(im);
   _evas_cache_image_lru_del(im);
   // _evas_cache_image_lru_nodata_del(im);
   im->flags.dirty = 1;
   im->flags.cached = 1;
   im->cache2->dirty = eina_inlist_prepend(im->cache2->dirty, EINA_INLIST_GET(im));
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
   if (!im->cache2) return;
   im->flags.dirty = 0;
   im->flags.cached = 0;
   im->cache2->dirty = eina_inlist_remove(im->cache2->dirty, EINA_INLIST_GET(im));
}

static void
_evas_cache_image_activ_add(Image_Entry *im)
{
   if (im->flags.activ) return;
   _evas_cache_image_dirty_del(im);
   _evas_cache_image_lru_del(im);
   // _evas_cache_image_lru_nodata_del(im);
   if (!im->cache_key) return;
   im->flags.activ = 1;
   im->flags.cached = 1;
   eina_hash_direct_add(im->cache2->activ, im->cache_key, im);
}

static void
_evas_cache_image_activ_del(Image_Entry *im)
{
   if (!im->flags.activ) return;
   if (!im->cache_key) return;
   im->flags.activ = 0;
   im->flags.cached = 0;
   eina_hash_del(im->cache2->activ, im->cache_key, im);
}

static void
_evas_cache_image_lru_add(Image_Entry *im)
{
   if (im->flags.lru) return;
   _evas_cache_image_dirty_del(im);
   _evas_cache_image_activ_del(im);
   // _evas_cache_image_lru_nodata_del(im); 
   if (!im->cache_key) return;
   im->flags.lru = 1;
   im->flags.cached = 1;
   eina_hash_direct_add(im->cache2->inactiv, im->cache_key, im);
   im->cache2->lru = eina_inlist_prepend(im->cache2->lru, EINA_INLIST_GET(im));
   im->cache2->usage += im->cache2->func.mem_size_get(im);
}

static void
_evas_cache_image_lru_del(Image_Entry *im)
{
   if (!im->flags.lru) return;
   if (!im->cache_key) return;
   im->flags.lru = 0;
   im->flags.cached = 0;
   eina_hash_del(im->cache2->inactiv, im->cache_key, im);
   im->cache2->lru = eina_inlist_remove(im->cache2->lru, EINA_INLIST_GET(im));
   im->cache2->usage -= im->cache2->func.mem_size_get(im);
}

/*
static void
_evas_cache_image_lru_nodata_add(Image_Entry *im)
{
   if (im->flags.lru_nodata) return;
   _evas_cache_image_dirty_del(im);
   _evas_cache_image_activ_del(im);
   _evas_cache_image_lru_del(im);
   im->flags.lru = 1;
   im->flags.cached = 1;
   im->cache2->lru_nodata = eina_inlist_prepend(im->cache2->lru_nodata, EINA_INLIST_GET(im));
}

static void
_evas_cache_image_lru_nodata_del(Image_Entry *im)
{
   if (!im->flags.lru_nodata) return;
   im->flags.lru = 0;
   im->flags.cached = 0;
   im->cache2->lru_nodata = eina_inlist_remove(im->cache2->lru_nodata, EINA_INLIST_GET(im));
}
*/

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

static void
_evas_cache_image_entry_delete(Evas_Cache2 *cache, Image_Entry *ie)
{
   if (!ie) return;

   if (ie->flags.delete_me == 1)
     return;

   if (ie->preload_rid)
     {
        ie->flags.delete_me = 1;
        _evas_cache2_image_entry_preload_remove(ie, NULL);
        return;
     }

   _evas_cache_image_dirty_del(ie);
   _evas_cache_image_activ_del(ie);
   _evas_cache_image_lru_del(ie);
   // _evas_cache_image_lru_nodata_del(ie);


   if (ie->data1)
     {
        evas_cserve2_image_unload(ie);
        evas_cache2_image_unload_data(ie);
        evas_cserve2_image_free(ie);
     }
   else
     {
        if (cache)
          cache->func.surface_delete(ie);
     }

   FREESTRC(ie->cache_key);
   FREESTRC(ie->file);
   FREESTRC(ie->key);
   ie->cache2 = NULL;

   evas_common_rgba_image_scalecache_shutdown(ie);

   free(ie);
}

static Image_Entry *
_evas_cache_image_entry_new(Evas_Cache2 *cache,
                            const char *hkey,
                            Image_Timestamp *tstamp,
                            const char *file,
                            const char *key,
                            RGBA_Image_Loadopts *lo,
                            int *error)
{
   Image_Entry  *ie;
   RGBA_Image *im;

   // ie = cache->func.alloc();
   im = calloc(1, sizeof(RGBA_Image));
   if (!im)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return NULL;
     }

   im->flags = RGBA_IMAGE_NOTHING;
   im->ref = 1;
   evas_common_rgba_image_scalecache_init(&im->cache_entry);

   ie = &im->cache_entry;

   ie->cache2 = cache;
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

   if (lo) ie->load_opts = *lo;
   if (ie->file)
     {
        if (!evas_cserve2_image_load(ie, ie->file, ie->key, &(ie->load_opts)))
          {
             ERR("couldn't load '%s' '%s' with cserve2!",
                 ie->file, ie->key ? ie->key : "");
             _evas_cache_image_entry_delete(cache, ie);
             return NULL;
          }
     }

   if (ie->cache_key) _evas_cache_image_activ_add(ie);
   else _evas_cache_image_dirty_add(ie);
   return ie;
}

EAPI void
evas_cache2_image_surface_alloc(Image_Entry *ie, int w, int h)
{
   Evas_Cache2 *cache = ie->cache2;
   int wmin = w > 0 ? w : 1;
   int hmin = h > 0 ? h : 1;

   if (cache->func.surface_alloc(ie, wmin, hmin))
     {
        wmin = 0;
        hmin = 0;
     }

   ie->w = wmin;
   ie->h = hmin;
   ie->allocated.w = wmin;
   ie->allocated.h = hmin;
   ie->flags.loaded = EINA_TRUE;
}

static void
_evas_cache2_image_preloaded_cb(void *data, Eina_Bool success)
{
   Image_Entry *ie = data;
   Evas_Cache_Target *tmp;

   ie->cache2->preload = eina_list_remove(ie->cache2->preload, ie);
   ie->flags.preload_done = success;

   while ((tmp = ie->targets))
     {
        ie->targets = (Evas_Cache_Target *)
           eina_inlist_remove(EINA_INLIST_GET(ie->targets),
                            EINA_INLIST_GET(ie->targets));
        if (!ie->flags.delete_me)
            evas_object_inform_call_image_preloaded((Evas_Object *) tmp->target);
        free(tmp);
     }

   if (ie->flags.delete_me)
     _evas_cache_image_entry_delete(ie->cache2, ie);
}

static Eina_Bool
_evas_cache2_image_entry_preload_add(Image_Entry *ie, const void *target)
{
   Evas_Cache_Target *tg;

   if (ie->flags.preload_done)
     return EINA_FALSE;

   tg = malloc(sizeof(Evas_Cache_Target));
   if (!tg)
     return EINA_TRUE;

   tg->target = target;
   ie->targets = (Evas_Cache_Target *)
      eina_inlist_append(EINA_INLIST_GET(ie->targets), EINA_INLIST_GET(tg));

   if (!ie->preload_rid)
     {
        ie->cache2->preload = eina_list_append(ie->cache2->preload, ie);
        evas_cserve2_image_preload(ie, _evas_cache2_image_preloaded_cb);
     }

   return EINA_TRUE;
}

static void
_evas_cache2_image_entry_preload_remove(Image_Entry *ie, const void *target)
{
   if (target)
     {
        Evas_Cache_Target *tg;

        EINA_INLIST_FOREACH(ie->targets, tg)
          {
             if (tg->target == target)
               {
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

   // FIXME: Should also send message to the server to cancel the request.
}

EAPI Image_Entry *
evas_cache2_image_copied_data(Evas_Cache2 *cache, unsigned int w, unsigned int h, DATA32 *image_data, int alpha, int cspace)
{
   Image_Entry *im;

   if ((cspace == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (cspace == EVAS_COLORSPACE_YCBCR422P709_PL) ||
       (cspace == EVAS_COLORSPACE_YCBCR422601_PL))
     w &= ~0x1;

   im = _evas_cache_image_entry_new(cache, NULL, NULL, NULL, NULL, NULL, NULL);
   if (!im)
     return NULL;

   im->space = cspace;
   im->flags.alpha = alpha;
   evas_cache2_image_surface_alloc(im, w, h);
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
evas_cache2_image_data(Evas_Cache2 *cache, unsigned int w, unsigned int h, DATA32 *image_data, int alpha, int cspace)
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
   im->flags.loaded = 1;
   if (cache->func.data(im, w, h, image_data, alpha, cspace) != 0)
     {
        _evas_cache_image_entry_delete(cache, im);
        return NULL;
     }
   im->references = 1;
   if (cache->func.debug) cache->func.debug("data", im);
   return im;
}

EAPI Image_Entry *
evas_cache2_image_empty(Evas_Cache2 *cache)
{
   Image_Entry *im;

   im = _evas_cache_image_entry_new(cache, NULL, NULL, NULL, NULL, NULL, NULL);
   if (!im)
     return NULL;

   im->references = 1;
   return im;
}

EAPI Image_Entry *
evas_cache2_image_size_set(Image_Entry *im, unsigned int w, unsigned h)
{
   Evas_Cache2 *cache;
   Image_Entry *im2 = NULL;
   int error;

   if ((im->space == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (im->space == EVAS_COLORSPACE_YCBCR422P709_PL) ||
       (im->space == EVAS_COLORSPACE_YCBCR422601_PL))
     w &= ~0x1;

   if ((im->w == w) && (im->h == h)) return im;

   cache = im->cache2;
   im2 = _evas_cache_image_entry_new(cache, NULL, NULL, NULL, NULL, NULL,
                                     NULL);
   if (!im2) goto on_error;

   im2->flags.alpha = im->flags.alpha;
   im2->space = im->space;
   im2->load_opts = im->load_opts;
   evas_cache2_image_surface_alloc(im2, w, h);
   error = cache->func.size_set(im2, im, w, h);
   if (error != 0) goto on_error;

   im2->references = 1;

   evas_cache2_image_close(im);
   return im2;

on_error:
   if (im2)
     _evas_cache_image_entry_delete(cache, im2);
   return NULL;
}

EAPI Evas_Cache2 *
evas_cache2_init(const Evas_Cache2_Image_Func *cb)
{
   Evas_Cache2 *cache = calloc(1, sizeof(Evas_Cache2));

   cache->func = *cb;
   cache->activ = eina_hash_string_superfast_new(NULL);
   cache->inactiv = eina_hash_string_superfast_new(NULL);

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
evas_cache2_shutdown(Evas_Cache2 *cache)
{
   Eina_List *delete_list;
   Image_Entry *im;

   while (cache->lru)
     {
        im = (Image_Entry *)cache->lru;
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

   eina_hash_free(cache->activ);
   eina_hash_free(cache->inactiv);

   free(cache);
}

static void
_create_hash_key(char *hkey, const char *path, size_t pathlen, const char *key, size_t keylen, RGBA_Image_Loadopts *lo)
{
   const char *ckey = "(null)";
   size_t size;

   /* generate hkey from file+key+load opts */
   memcpy(hkey, path, pathlen);
   size = pathlen;
   memcpy(hkey + size, "//://", 5);
   size += 5;
   if (key) ckey = key;
   memcpy(hkey + size, ckey, keylen);
   size += keylen;
   if (lo)
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
}

EAPI Image_Entry *
evas_cache2_image_open(Evas_Cache2 *cache, const char *path, const char *key, RGBA_Image_Loadopts *lo, int *error)
{
   size_t                size;
   size_t                pathlen;
   size_t                keylen;
   char                 *hkey;
   Image_Entry          *im;
   int                   stat_done = 0, stat_failed = 0;
   struct stat           st;
   Image_Timestamp       tstamp;
   Evas_Image_Load_Opts  prevent = { 0, 0.0, 0, 0, 0, { 0, 0, 0, 0 }, EINA_FALSE };

   if ((!path) || ((!path) && (!key)))
     {
        *error = EVAS_LOAD_ERROR_GENERIC;
        return NULL;
     }

   pathlen = strlen(path);
   keylen = key ? strlen(key) : 6;
   size = pathlen + keylen + 132;
   hkey = alloca(sizeof(char) * size);

   _create_hash_key(hkey, path, pathlen, key, keylen, lo);
   DBG("Looking at the hash for key '%s'", hkey);

   /* use local var to copy default load options to the image entry */
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

   im = eina_hash_find(cache->activ, hkey);

   if (im)
     {
        int ok = 1;
        DBG("Found entry on active hash for key: '%s'", hkey);

        stat_done = 1;
        if (stat(path, &st) < 0)
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
        DBG("Entry on inactive hash was invalid (file changed or deleted).");
        _evas_cache_image_dirty_add(im);
        im = NULL;
     }

   im = eina_hash_find(cache->inactiv, hkey);

   if (im)
     {
        int ok = 1;
        DBG("Found entry on inactive hash for key: '%s'", hkey);

        if (!stat_done)
          {
             stat_done = 1;
             if (stat(path, &st) < 0)
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
        DBG("Entry on inactive hash was invalid (file changed or deleted).");
        /* as avtive cache find - if we match in lru and its invalid, dirty */
        _evas_cache_image_dirty_add(im);
        /* this image never used, so it have to be deleted */
        _evas_cache_image_entry_delete(cache, im);
        im = NULL;
     }
   if (stat_failed) goto on_stat_error;

   if (!stat_done)
     {
        if (stat(path, &st) < 0) goto on_stat_error;
     }
   _timestamp_build(&tstamp, &st);
   DBG("Creating a new entry for key '%s'.", hkey);
   im = _evas_cache_image_entry_new(cache, hkey, &tstamp, path, key,
                                    lo, error);
   if (!im) goto on_stat_error;

on_ok:
   *error = EVAS_LOAD_ERROR_NONE;
   DBG("Using entry on hash for key '%s'", hkey);

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

EAPI int
evas_cache2_image_open_wait(Image_Entry *im)
{
   DBG("Wait for open image '%s' '%s'", im->file, im->key);
   if (evas_cserve2_image_load_wait(im) != CSERVE2_NONE)
     return EVAS_LOAD_ERROR_GENERIC;

   return EVAS_LOAD_ERROR_NONE;
}

EAPI void
evas_cache2_image_close(Image_Entry *im)
{
   Evas_Cache2 *cache;
   int references;

   im->references--;
   if (im->references < 0)
     {
        ERR("image with negative references: %d", im->references);
        im->references = 0;
     }

   references = im->references;
   cache = im->cache2;

   if (references > 0)
     return;

   if (im->flags.dirty)
     {
        _evas_cache_image_entry_delete(cache, im);
        return;
     }

   _evas_cache_image_lru_add(im);
   if (cache)
     evas_cache2_flush(cache);
}

EAPI int
evas_cache2_image_load_data(Image_Entry *ie)
{
   int error = EVAS_LOAD_ERROR_NONE;

   if ((ie->flags.loaded) && (!ie->flags.animated))
     return error;

   ie->flags.in_progress = EINA_TRUE;

   DBG("try cserve2 image data '%s' '%s'",
       ie->file, ie->key ? ie->key : "");
   if (evas_cserve2_image_data_load(ie))
     {
        evas_cserve2_image_load_data_wait(ie);
        RGBA_Image *im = (RGBA_Image *)ie;
        im->image.data = evas_cserve2_image_data_get(ie);
        DBG("try cserve2 image data '%s' '%s' loaded!",
            ie->file, ie->key ? ie->key : "");
        if (im->image.data)
          {
             im->image.no_free = 1;
             error = EVAS_LOAD_ERROR_NONE;
          }
        else
          {
             ERR("Failed to load data for image '%s' '%s'.",
                 ie->file, ie->key ? ie->key : "");
             error = EVAS_LOAD_ERROR_GENERIC;
          }
     }
   else
     {
        ERR("Couldn't send LOAD message to cserve2.");
        error = EVAS_LOAD_ERROR_GENERIC;
     }

   ie->flags.in_progress = EINA_FALSE;
   ie->flags.loaded = 1;

   if (error != EVAS_LOAD_ERROR_NONE)
     ie->flags.loaded = 0;

   return error;
}

EAPI void
evas_cache2_image_unload_data(Image_Entry *im)
{
   // FIXME: This function seems pretty useless, since we always have
   // to send an UNLOAD message to the server when closing an image,
   // even if we didn't send a LOAD message before, because the SETOPTS
   // message increases the image refcount.
   if (im->flags.in_progress)
     return;

   if ((!im->file))
     return;

   if (!im->flags.loaded)
     return;

//    if (im->data2)
   im->flags.loaded = 0;
}

EAPI void
evas_cache2_image_preload_data(Image_Entry *im, const void *target)
{
   RGBA_Image *img = (RGBA_Image *)im;

   if ((im->flags.loaded) && (img->image.data))
     {
        evas_object_inform_call_image_preloaded((Evas_Object *)target);
        return;
     }

   if (!_evas_cache2_image_entry_preload_add(im, target))
     evas_object_inform_call_image_preloaded((Evas_Object *)target);
}

EAPI void
evas_cache2_image_preload_cancel(Image_Entry *im, const void *target)
{
   if (!target)
     return;

   _evas_cache2_image_entry_preload_remove(im, target);
}

EAPI DATA32 *
evas_cache2_image_pixels(Image_Entry *im)
{
   return im->cache2->func.surface_pixels(im);
}

EAPI Image_Entry *
evas_cache2_image_writable(Image_Entry *im)
{
   Evas_Cache2 *cache = im->cache2;
   Image_Entry *im2 = NULL;

   if (!im->cache_key)
     {
        if (!im->flags.dirty)
          _evas_cache_image_dirty_add(im);
        return im;
     }

   im2 = evas_cache2_image_copied_data(cache, im->w, im->h,
                                        evas_cache2_image_pixels(im),
                                        im->flags.alpha, im->space);
   if (!im2)
     goto on_error;

   evas_cache2_image_close(im);
   return im2;

on_error:
   if (im2)
     _evas_cache_image_entry_delete(cache, im2);
   return NULL;
}

EAPI Image_Entry *
evas_cache2_image_dirty(Image_Entry *im, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
   Evas_Cache2 *cache = im->cache2;
   Image_Entry *im2 = NULL;

   if (!im->cache_key)
     {
        if (!im->flags.dirty)
          _evas_cache_image_dirty_add(im);
        im2 = im;
     }
   else
     {
        im2 = evas_cache2_image_copied_data(cache, im->w, im->h,
                                            evas_cache2_image_pixels(im),
                                            im->flags.alpha, im->space);
        if (!im2)
          goto on_error;

        evas_cache2_image_close(im);
     }

   if (cache->func.dirty_region)
     cache->func.dirty_region(im2, x, y, w, h);

   return im2;

on_error:
   if (im2)
     _evas_cache_image_entry_delete(cache, im2);
   evas_cache2_image_close(im);
   return NULL;
}

EAPI int
evas_cache2_flush(Evas_Cache2 *cache)
{
   if (cache->limit == -1) return -1;

   while ((cache->lru) && (cache->limit < cache->usage))
     {
        Image_Entry *im;

        im = (Image_Entry *)cache->lru->last;
        DBG("Remove unused entry from cache.");
        _evas_cache_image_entry_delete(cache, im);
     }

   return cache->usage;
}

EAPI void
evas_cache2_limit_set(Evas_Cache2 *cache, int limit)
{
   if (cache->limit == limit)
     return;

   DBG("Cache2 limit set to %d", limit);

   cache->limit = limit;

   evas_cache2_flush(cache);
}

EAPI int
evas_cache2_limit_get(Evas_Cache2 *cache)
{
   return cache->limit;
}

EAPI int
evas_cache2_usage_get(Evas_Cache2 *cache)
{
   return cache->usage;
}

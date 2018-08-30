#ifndef _EVAS_CACHE_H
#define _EVAS_CACHE_H

typedef struct _Evas_Cache_Image                Evas_Cache_Image;
typedef struct _Evas_Cache_Image_Func           Evas_Cache_Image_Func;
typedef struct _Evas_Cache_Engine_Image         Evas_Cache_Engine_Image;
typedef struct _Evas_Cache_Engine_Image_Func    Evas_Cache_Engine_Image_Func;

struct _Evas_Cache_Image_Func
{
   Image_Entry *(*alloc)(void);
   void         (*dealloc)(Image_Entry *im);

   /* The cache provide some helpers for surface manipulation. */
   int          (*surface_alloc)(Image_Entry *im, unsigned int w, unsigned int h);
   void         (*surface_delete)(Image_Entry *im);
   DATA32      *(*surface_pixels)(Image_Entry *im);

   /* The cache is doing the allocation and deallocation, you must just do the rest. */
   int          (*constructor)(Image_Entry *im); /**< return is EVAS_LOAD_ERROR_* or EVAS_LOAD_ERROR_NONE! */
   void         (*destructor)(Image_Entry *im);

   void         (*dirty_region)(Image_Entry *im, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
   /* Only called when references > 0. Need to provide a fresh copie of im. */
   /* The destination surface does have a surface, but no allocated pixel data. */
   int          (*dirty)(Image_Entry *dst, const Image_Entry *src);
   /* Only called when references == 1. We will call drop on `im'. */
   /* The destination surface does not have any surface. */
   int          (*size_set)(Image_Entry *dst, const Image_Entry *src, unsigned int w, unsigned int h);

   /* The destination surface does not have any surface. */
   int          (*copied_data)(Image_Entry *dst, unsigned int w, unsigned int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace);
   /* The destination surface does not have any surface. */
   int          (*data)(Image_Entry *dst, unsigned int w, unsigned int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace);
   int          (*color_space)(Image_Entry *dst, Evas_Colorspace cspace);

   /* This function need to update im->w and im->h. */
   int          (*load)(Image_Entry *im); /**< return is EVAS_LOAD_ERROR_* or EVAS_LOAD_ERROR_NONE! */
   int          (*mem_size_get)(Image_Entry *im);
   void         (*debug)(const char *context, Image_Entry *im);
};

struct _Evas_Cache_Image
{
   Evas_Cache_Image_Func         func;

   Eina_List			*preload;
   Eina_List			*pending;

   Eina_Inlist                  *dirty;

   Eina_Inlist                  *lru;
   Eina_Inlist                  *lru_nodata;
   Eina_Hash                    *inactiv;
   Eina_Hash                    *activ;

   Eina_Hash                    *mmap_activ;
   Eina_Hash                    *mmap_inactiv;

   void                         *data;

   int                           usage;
   unsigned int                  limit;
   int                           references;
};

struct _Evas_Cache_Engine_Image_Func
{
   /* Must return a char* allocated with eina_stringshare_add. */
   char*                (*key)(Image_Entry *im, const char *file, const char *key, Evas_Image_Load_Opts *lo, int *error);

   Engine_Image_Entry*  (*alloc)(void);
   void                 (*dealloc)(Engine_Image_Entry *eim);

   int                  (*constructor)(Engine_Image_Entry *eim, void* data);
   void                 (*destructor)(Engine_Image_Entry *eim);

   void                 (*dirty_region)(Engine_Image_Entry *eim, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
   /* Only called when references > 0. Need to provide a fresh copie of im. */
   int                  (*dirty)(Engine_Image_Entry *dst, const Engine_Image_Entry *src);
   /* Only called when references == 1. We will call drop on `im'. */
   int                  (*size_set)(Engine_Image_Entry *dst, const Engine_Image_Entry *src);

   int                  (*update_data)(Engine_Image_Entry* dst, void* data);

   void                 (*load)(Engine_Image_Entry *eim, const Image_Entry* im);
   int                  (*mem_size_get)(Engine_Image_Entry *eim);
   void                 (*debug)(const char* context, Engine_Image_Entry *eim);
};

struct _Evas_Cache_Engine_Image
{
   Evas_Cache_Engine_Image_Func func;

   Eina_Inlist                 *dirty;

   Eina_Hash                   *activ;
   Eina_Hash                   *inactiv;
   Eina_Inlist                 *lru;

   Evas_Cache_Image            *parent;
   Evas_Cache_Engine_Image     *brother;

   int                          usage;
   int                          limit;

   int                          references;
};


#ifdef __cplusplus
extern "C" {
#endif


EAPI Evas_Cache_Image*        evas_cache_image_init(const Evas_Cache_Image_Func *cb);
EAPI void                     evas_cache_image_shutdown(Evas_Cache_Image *cache);
EAPI Image_Entry*             evas_cache_image_mmap_request(Evas_Cache_Image *cache, Eina_File *f, const char *key, Evas_Image_Load_Opts *lo, int *error);
EAPI void                     evas_cache_image_ref(Image_Entry *im);
EAPI void                     evas_cache_image_drop(Image_Entry *im);
EAPI void                     evas_cache_image_data_not_needed(Image_Entry *im);
EAPI int                      evas_cache_image_flush(Evas_Cache_Image *cache);
EAPI void                     evas_cache_private_set(Evas_Cache_Image *cache, const void *data);
EAPI void*                    evas_cache_private_get(Evas_Cache_Image *cache);
EAPI void*                    evas_cache_private_from_image_entry_get(Image_Entry *im);

EAPI int                      evas_cache_image_usage_get(Evas_Cache_Image *cache);
EAPI int                      evas_cache_image_get(Evas_Cache_Image *cache);
EAPI void                     evas_cache_image_set(Evas_Cache_Image *cache, unsigned int size);

EAPI Image_Entry*             evas_cache_image_alone(Image_Entry *im);
EAPI Image_Entry*             evas_cache_image_dirty(Image_Entry *im, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
EAPI int                      evas_cache_image_load_data(Image_Entry *im);
EAPI void                     evas_cache_image_unload_data(Image_Entry *im);
EAPI Eina_Bool                evas_cache_image_is_loaded(Image_Entry *im);
EAPI void                     evas_cache_image_unload_all(Evas_Cache_Image *cache);
EAPI void                     evas_cache_image_surface_alloc(Image_Entry *im, unsigned int w, unsigned int h);
EAPI DATA32*                  evas_cache_image_pixels(Image_Entry *im);
EAPI Image_Entry*             evas_cache_image_copied_data(Evas_Cache_Image *cache, unsigned int w, unsigned int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace);
EAPI Image_Entry*             evas_cache_image_data(Evas_Cache_Image *cache, unsigned int w, unsigned int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace);
EAPI void                     evas_cache_image_colorspace(Image_Entry *im, Evas_Colorspace cspace);
EAPI Image_Entry*             evas_cache_image_empty(Evas_Cache_Image *cache);
EAPI Image_Entry*             evas_cache_image_size_set(Image_Entry *im, unsigned int w, unsigned int h);

EAPI Evas_Cache_Engine_Image* evas_cache_engine_image_init(const Evas_Cache_Engine_Image_Func *cb, Evas_Cache_Image *parent);
EAPI void                     evas_cache_engine_image_shutdown(Evas_Cache_Engine_Image *cache);

EAPI int                      evas_cache_engine_image_usage_get(Evas_Cache_Engine_Image *cache);
EAPI int                      evas_cache_engine_image_get(Evas_Cache_Engine_Image *cache);
EAPI void                     evas_cache_engine_image_set(Evas_Cache_Engine_Image *cache, int limit);

EAPI void                     evas_cache_engine_parent_not_needed(Engine_Image_Entry *eim);
EAPI Engine_Image_Entry*      evas_cache_engine_image_engine(Evas_Cache_Engine_Image *cache, void *engine_data);
EAPI void                     evas_cache_engine_image_drop(Engine_Image_Entry *eim);
EAPI Engine_Image_Entry*      evas_cache_engine_image_alone(Engine_Image_Entry *eim, void *data);
EAPI Engine_Image_Entry*      evas_cache_engine_image_dirty(Engine_Image_Entry *eim, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
EAPI Engine_Image_Entry*      evas_cache_engine_image_copied_data(Evas_Cache_Engine_Image *cache, unsigned int w, unsigned int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace, void *engine_data);
EAPI Engine_Image_Entry*      evas_cache_engine_image_data(Evas_Cache_Engine_Image *cache, unsigned int w, unsigned int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace, void *engine_data);
EAPI void                     evas_cache_engine_image_colorspace(Engine_Image_Entry *eim, Evas_Colorspace cspace, void *engine_data);
EAPI Engine_Image_Entry*      evas_cache_engine_image_size_set(Engine_Image_Entry *eim, unsigned int w, unsigned int h);

EAPI void                     evas_cache_engine_image_load_data(Engine_Image_Entry *eim);

EAPI void                     evas_cache_image_preload_data(Image_Entry *im, const Eo *target, void (*preloaded_cb)(void *data), void *preloaded_data);
EAPI void                     evas_cache_image_preload_cancel(Image_Entry *im, const Eo *target);

EAPI int                      evas_cache_async_frozen_get(void);
EAPI void                     evas_cache_async_freeze(void);
EAPI void                     evas_cache_async_thaw(void);

#ifdef __cplusplus
}
#endif


#endif /* _EVAS_CACHE_H */

#ifndef _EVAS_CACHE2_H
#define _EVAS_CACHE2_H

typedef struct _Evas_Cache2_Image_Func          Evas_Cache2_Image_Func;
typedef struct _Evas_Cache2                     Evas_Cache2;

struct _Evas_Cache2_Image_Func
{
   // Image_Entry *(*alloc)(void);
   // void         (*dealloc)(Image_Entry *im);

   /* The cache provide some helpers for surface manipulation. */
   int          (*surface_alloc)(Image_Entry *im, unsigned int w, unsigned int h);
   void         (*surface_delete)(Image_Entry *im);
   DATA32      *(*surface_pixels)(Image_Entry *im);

   /* The cache is doing the allocation and deallocation, you must just do the rest. */
   // int          (*constructor)(Image_Entry *im); /**< return is EVAS_LOAD_ERROR_* or EVAS_LOAD_ERROR_NONE! */
   // void         (*destructor)(Image_Entry *im);

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
   // int          (*load)(Image_Entry *im); /**< return is EVAS_LOAD_ERROR_* or EVAS_LOAD_ERROR_NONE! */
   int          (*mem_size_get)(Image_Entry *im);
   void         (*debug)(const char *context, Image_Entry *im);
};

struct _Evas_Cache2
{
   Evas_Cache2_Image_Func func;

   Eina_List *preload;

   Eina_Hash *inactiv;
   Eina_Hash *activ;
   Eina_Inlist *dirty;
   Eina_Inlist *lru;
   int usage;
   int limit;
};

#ifdef __cplusplus
extern "C" {
#endif

EAPI Evas_Cache2*        evas_cache2_init(const Evas_Cache2_Image_Func *cb);
EAPI void                evas_cache2_shutdown(Evas_Cache2 *cache);
EAPI Image_Entry * evas_cache2_image_open(Evas_Cache2 *cache, const char *path, const char *key, Evas_Image_Load_Opts *lo, int *error);
EAPI Image_Entry *evas_cache2_image_scale_load(Image_Entry *im, int src_x, int src_y, int src_w, int src_h, int dst_w, int dst_h, int smooth);
EAPI int evas_cache2_image_open_wait(Image_Entry *im);
EAPI void evas_cache2_image_ref(Image_Entry *im);
EAPI void evas_cache2_image_close(Image_Entry *im);
EAPI int evas_cache2_image_load_data(Image_Entry *ie);
EAPI void evas_cache2_image_unload_data(Image_Entry *im);
EAPI void evas_cache2_image_preload_data(Image_Entry *im, const void *target);
EAPI void evas_cache2_image_preload_cancel(Image_Entry *im, const void *target);
EAPI void evas_cache2_image_cache_key_create(char *hkey, const char *path, size_t pathlen, const char *key, size_t keylen, const Evas_Image_Load_Opts *lo);

EAPI DATA32 * evas_cache2_image_pixels(Image_Entry *im);
EAPI Image_Entry * evas_cache2_image_writable(Image_Entry *im);
EAPI Image_Entry * evas_cache2_image_data(Evas_Cache2 *cache, unsigned int w, unsigned int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace);
EAPI Image_Entry * evas_cache2_image_copied_data(Evas_Cache2 *cache, unsigned int w, unsigned int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace);
EAPI Image_Entry * evas_cache2_image_size_set(Image_Entry *im, unsigned int w, unsigned h);
EAPI Image_Entry * evas_cache2_image_dirty(Image_Entry *im, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
EAPI Image_Entry * evas_cache2_image_empty(Evas_Cache2 *cache);
EAPI void evas_cache2_image_surface_alloc(Image_Entry *ie, int w, int h);
EAPI Eina_Bool evas_cache2_image_cached(Image_Entry *ie);

EAPI int evas_cache2_flush(Evas_Cache2 *cache);
EAPI void evas_cache2_limit_set(Evas_Cache2 *cache, int limit);
EAPI int evas_cache2_limit_get(Evas_Cache2 *cache);
EAPI int evas_cache2_usage_get(Evas_Cache2 *cache);

#ifdef __cplusplus
}
#endif


#endif /* _EVAS_CACHE2_H */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libheif/heif.h>

#include "Evas_Loader.h"
#include "evas_common_private.h"

typedef struct _Evas_Loader_Internal Evas_Loader_Internal;
struct _Evas_Loader_Internal
{
   Eina_File *f;
   Evas_Image_Load_Opts *opts;
   struct heif_context* ctx;
   struct heif_image_handle *handle;
};

static int _evas_loader_heif_log_dom = -1;
static Eina_Module *_evas_loader_heif_mod = NULL;

#define LOAD(x)                                             \
  if (!_evas_loader_heif_mod) {                             \
       if ((_evas_loader_heif_mod = eina_module_new(x))) {  \
            if (!eina_module_load(_evas_loader_heif_mod)) { \
                 eina_module_free(_evas_loader_heif_mod);   \
                 _evas_loader_heif_mod = NULL;              \
              }                                             \
         }                                                  \
    }

#define SYM(x)                                                           \
   if (!(x ## _f = eina_module_symbol_get(_evas_loader_heif_mod, #x))) { \
      ERR("Cannot find symbol '%s' in '%s'",                             \
          #x, eina_module_file_get(_evas_loader_heif_mod));              \
      goto error;                                                        \
   }

// heif_check_filetype
typedef enum heif_filetype_result (*heif_check_filetype_t)(const uint8_t* data,
                                                           int len);
static heif_check_filetype_t heif_check_filetype_f = NULL;

// heif_context_alloc
typedef struct heif_context* (*heif_context_alloc_t)(void);
static heif_context_alloc_t heif_context_alloc_f = NULL;

// heif_context_free
typedef void (*heif_context_free_t)(struct heif_context*);
static heif_context_free_t heif_context_free_f = NULL;

// heif_context_get_primary_image_handle
typedef struct heif_error (*heif_context_get_primary_image_handle_t)(struct heif_context* ctx,
                                                                     struct heif_image_handle**);
static heif_context_get_primary_image_handle_t heif_context_get_primary_image_handle_f = NULL;

// heif_context_read_from_memory_without_copy
typedef struct heif_error (*heif_context_read_from_memory_without_copy_t)(struct heif_context*,
                                                                          const void* mem, size_t size,
                                                                          const struct heif_reading_options*);
static heif_context_read_from_memory_without_copy_t heif_context_read_from_memory_without_copy_f = NULL;

// heif_decode_image
typedef struct heif_error (*heif_decode_image_t)(const struct heif_image_handle* in_handle,
                                                 struct heif_image** out_img,
                                                 enum heif_colorspace colorspace,
                                                 enum heif_chroma chroma,
                                                 const struct heif_decoding_options* options);
static heif_decode_image_t heif_decode_image_f = NULL;

// heif_deinit
typedef void (*heif_deinit_t)();
static heif_deinit_t heif_deinit_f = NULL;

// heif_image_get_plane_readonly
typedef const uint8_t* (*heif_image_get_plane_readonly_t)(const struct heif_image*,
                                                          enum heif_channel channel,
                                                          int* out_stride);
static heif_image_get_plane_readonly_t heif_image_get_plane_readonly_f = NULL;

// heif_image_handle_get_height
typedef int (*heif_image_handle_get_height_t)(const struct heif_image_handle* handle);
static heif_image_handle_get_height_t heif_image_handle_get_height_f = NULL;

// heif_image_handle_get_width
typedef int (*heif_image_handle_get_width_t)(const struct heif_image_handle* handle);
static heif_image_handle_get_width_t heif_image_handle_get_width_f = NULL;

// heif_image_handle_has_alpha_channel
typedef int (*heif_image_handle_has_alpha_channel_t)(const struct heif_image_handle*);
static heif_image_handle_has_alpha_channel_t heif_image_handle_has_alpha_channel_f = NULL;

// heif_init
typedef struct heif_error (*heif_init_t)(struct heif_init_params*);
static heif_init_t heif_init_f = NULL;

// heif_image_handle_release
typedef void (*heif_image_handle_release_t)(const struct heif_image_handle*);
static heif_image_handle_release_t heif_image_handle_release_f = NULL;

// heif_image_release
typedef void (*heif_image_release_t)(const struct heif_image*);
static heif_image_release_t heif_image_release_f = NULL;

/* optional: embedded thumbnails, used when a smaller load size is set */

// heif_image_handle_get_number_of_thumbnails
typedef int (*heif_image_handle_get_number_of_thumbnails_t)(const struct heif_image_handle* handle);
static heif_image_handle_get_number_of_thumbnails_t heif_image_handle_get_number_of_thumbnails_f = NULL;

// heif_image_handle_get_list_of_thumbnail_IDs
typedef int (*heif_image_handle_get_list_of_thumbnail_IDs_t)(const struct heif_image_handle* handle,
                                                             heif_item_id* ids, int count);
static heif_image_handle_get_list_of_thumbnail_IDs_t heif_image_handle_get_list_of_thumbnail_IDs_f = NULL;

// heif_image_handle_get_thumbnail
typedef struct heif_error (*heif_image_handle_get_thumbnail_t)(const struct heif_image_handle* main_image_handle,
                                                               heif_item_id thumbnail_id,
                                                               struct heif_image_handle** out_thumbnail_handle);
static heif_image_handle_get_thumbnail_t heif_image_handle_get_thumbnail_f = NULL;

// heif_image_handle_get_item_id
typedef heif_item_id (*heif_image_handle_get_item_id_t)(const struct heif_image_handle* handle);
static heif_image_handle_get_item_id_t heif_image_handle_get_item_id_f = NULL;

/* from heif_properties.h; enums are passed as int so this builds against
 * libheif versions that lack that header */

// heif_item_get_transformation_properties
typedef int (*heif_item_get_transformation_properties_t)(const struct heif_context* context,
                                                         heif_item_id id,
                                                         uint32_t* out_list,
                                                         int count);
static heif_item_get_transformation_properties_t heif_item_get_transformation_properties_f = NULL;

// heif_item_get_property_type
typedef int (*heif_item_get_property_type_t)(const struct heif_context* context,
                                             heif_item_id id,
                                             uint32_t property_id);
static heif_item_get_property_type_t heif_item_get_property_type_f = NULL;

// heif_item_get_property_transform_rotation_ccw
typedef int (*heif_item_get_property_transform_rotation_ccw_t)(const struct heif_context* context,
                                                               heif_item_id itemId,
                                                               uint32_t propertyId);
static heif_item_get_property_transform_rotation_ccw_t heif_item_get_property_transform_rotation_ccw_f = NULL;

// heif_item_get_property_transform_mirror
typedef int (*heif_item_get_property_transform_mirror_t)(const struct heif_context* context,
                                                         heif_item_id itemId,
                                                         uint32_t propertyId);
static heif_item_get_property_transform_mirror_t heif_item_get_property_transform_mirror_f = NULL;

#define EVAS_HEIF_FOURCC(a, b, c, d) \
   ((int)(((unsigned int)(a) << 24) | ((b) << 16) | ((c) << 8) | (d)))
#define EVAS_HEIF_PROP_IROT EVAS_HEIF_FOURCC('i', 'r', 'o', 't')
#define EVAS_HEIF_PROP_IMIR EVAS_HEIF_FOURCC('i', 'm', 'i', 'r')
#define EVAS_HEIF_MAX_TRANSFORMS 8
#define EVAS_HEIF_MAX_THUMBNAILS 16

static Eina_Bool _evas_loader_heif_thumbnails = EINA_FALSE;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_loader_heif_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_loader_heif_log_dom, __VA_ARGS__)

/* EINA_TRUE if both items carry the same irot/imir transforms in the same
 * order (clap only has to be present in both; the aspect check covers it) */
static Eina_Bool
evas_image_load_heif_same_transforms(struct heif_context *ctx,
                                     heif_item_id a, heif_item_id b)
{
   uint32_t pa[EVAS_HEIF_MAX_TRANSFORMS];
   uint32_t pb[EVAS_HEIF_MAX_TRANSFORMS];
   int na, nb, i;

   na = heif_item_get_transformation_properties_f(ctx, a, NULL, 0);
   nb = heif_item_get_transformation_properties_f(ctx, b, NULL, 0);
   if ((na != nb) || (na < 0) || (na > EVAS_HEIF_MAX_TRANSFORMS))
     return EINA_FALSE;
   if (na == 0)
     return EINA_TRUE;
   if ((heif_item_get_transformation_properties_f(ctx, a, pa, na) != na) ||
       (heif_item_get_transformation_properties_f(ctx, b, pb, nb) != nb))
     return EINA_FALSE;

   for (i = 0; i < na; i++)
     {
        int ta, tb;

        ta = heif_item_get_property_type_f(ctx, a, pa[i]);
        tb = heif_item_get_property_type_f(ctx, b, pb[i]);
        if (ta != tb)
          return EINA_FALSE;
        if ((ta == EVAS_HEIF_PROP_IROT) &&
            (heif_item_get_property_transform_rotation_ccw_f(ctx, a, pa[i]) !=
             heif_item_get_property_transform_rotation_ccw_f(ctx, b, pb[i])))
          return EINA_FALSE;
        if ((ta == EVAS_HEIF_PROP_IMIR) &&
            (heif_item_get_property_transform_mirror_f(ctx, a, pa[i]) !=
             heif_item_get_property_transform_mirror_f(ctx, b, pb[i])))
          return EINA_FALSE;
     }

   return EINA_TRUE;
}

/* When a load size or scale_down_by is set and an embedded thumbnail is at
 * least that big (same aspect ratio and orientation), decode the thumbnail
 * instead of the primary image, like the jpeg loader decodes at a reduced
 * DCT scale. Returns the handle to use; the primary is released if it is
 * replaced. */
static struct heif_image_handle *
evas_image_load_heif_thumbnail_pick(Evas_Loader_Internal *loader,
                                    struct heif_context *ctx,
                                    struct heif_image_handle *handle)
{
   Evas_Image_Load_Opts *opts = loader->opts;
   heif_item_id ids[EVAS_HEIF_MAX_THUMBNAILS];
   struct heif_image_handle *best = NULL;
   unsigned long long w, h, tw, th;
   int bw = 0, n, i;

   if ((!_evas_loader_heif_thumbnails) || (!opts))
     return handle;
   if ((opts->emile.region.w > 0) && (opts->emile.region.h > 0))
     return handle;

   w = heif_image_handle_get_width_f(handle);
   h = heif_image_handle_get_height_f(handle);
   if ((w < 1) || (h < 1))
     return handle;

   /* smallest size the caller asked for, aspect ratio kept */
   if (opts->emile.scale_down_by > 1)
     {
        tw = w / opts->emile.scale_down_by;
        th = h / opts->emile.scale_down_by;
     }
   else if ((opts->emile.w > 0) && (opts->emile.h > 0))
     {
        if ((w * opts->emile.h) > (h * opts->emile.w))
          {
             tw = opts->emile.w;
             th = (h * opts->emile.w) / w;
          }
        else
          {
             th = opts->emile.h;
             tw = (w * opts->emile.h) / h;
          }
     }
   else
     return handle;
   if ((tw >= w) || (th >= h))
     return handle;

   if (heif_image_handle_get_number_of_thumbnails_f(handle) < 1)
     return handle;
   n = heif_image_handle_get_list_of_thumbnail_IDs_f(handle, ids,
                                                     EVAS_HEIF_MAX_THUMBNAILS);
   for (i = 0; i < n; i++)
     {
        struct heif_image_handle *thumb;
        struct heif_error err;
        long long d;
        int thw, thh;

        err = heif_image_handle_get_thumbnail_f(handle, ids[i], &thumb);
        if (err.code != heif_error_Ok)
          continue;
        thw = heif_image_handle_get_width_f(thumb);
        thh = heif_image_handle_get_height_f(thumb);
        /* same aspect ratio to within a pixel in either direction */
        d = (long long)thw * h - (long long)thh * w;
        if (d < 0) d = -d;
        if ((thw > 0) && (thh > 0) &&
            ((unsigned long long)thw >= tw) &&
            ((unsigned long long)thh >= th) &&
            ((unsigned long long)d <= w + h) &&
            ((!best) || (thw < bw)) &&
            evas_image_load_heif_same_transforms
              (ctx, heif_image_handle_get_item_id_f(handle),
               heif_image_handle_get_item_id_f(thumb)))
          {
             if (best)
               heif_image_handle_release_f(best);
             best = thumb;
             bw = thw;
          }
        else
          heif_image_handle_release_f(thumb);
     }

   if (!best)
     return handle;
   INF("using %dx%d thumbnail for %llux%llu load size",
       heif_image_handle_get_width_f(best),
       heif_image_handle_get_height_f(best), tw, th);
   heif_image_handle_release_f(handle);
   return best;
}

static Eina_Bool
evas_image_load_file_head_heif_init(Evas_Loader_Internal *loader,
                                    Emile_Image_Property *prop,
                                    void *map, size_t length,
                                    int *error)
{
   struct heif_context *ctx;
   struct heif_image_handle *handle;
   struct heif_error err;
   Eina_Bool ret;

   ret = EINA_FALSE;
   prop->w = 0;
   prop->h = 0;
   prop->alpha = EINA_FALSE;

   /* heif file must have a 12 bytes long header */
   if ((length < 12) ||
       (heif_check_filetype_f(map, length) != heif_filetype_yes_supported))
     {
        INF("HEIF header invalid");
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        return ret;
     }

   ctx = heif_context_alloc_f();
   if (!ctx)
     {
        INF("cannot allocate heif_context");
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        return ret;
     }

   err = heif_context_read_from_memory_without_copy_f(ctx, map, length, NULL);
   if (err.code != heif_error_Ok)
     {
        INF("%s", err.message);
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        heif_context_free_f(ctx);
        return ret;
   }

   err = heif_context_get_primary_image_handle_f(ctx, &handle);
   if (err.code != heif_error_Ok)
     {
        INF("%s", err.message);
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        heif_context_free_f(ctx);
        return ret;
     }

   handle = evas_image_load_heif_thumbnail_pick(loader, ctx, handle);

   prop->w = heif_image_handle_get_width_f(handle);
   prop->h = heif_image_handle_get_height_f(handle);

   /* if size is invalid, we exit */
   if ((prop->w < 1) || (prop->h < 1) ||
       (prop->w > IMG_MAX_SIZE) || (prop->h > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(prop->w, prop->h))
     {
        if (IMG_TOO_BIG(prop->w, prop->h))
          *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        else
          *error= EVAS_LOAD_ERROR_GENERIC;
        heif_image_handle_release_f(handle);
        heif_context_free_f(ctx);
        return ret;
     }

   prop->alpha = !!heif_image_handle_has_alpha_channel_f(handle);
   loader->ctx = ctx;
   loader->handle = handle;

   *error = EVAS_LOAD_ERROR_NONE;
   ret = EINA_TRUE;

   return ret;
}

static Eina_Bool
evas_image_load_file_head_heif_internal(Evas_Loader_Internal *loader,
                                        Emile_Image_Property *prop,
                                        void *map, size_t length,
                                        int *error)
{

   if (!evas_image_load_file_head_heif_init(loader, prop, map, length, error))
     return EINA_FALSE;

   heif_image_handle_release_f(loader->handle);
   heif_context_free_f(loader->ctx);
   loader->handle = NULL;
   loader->ctx = NULL;

   return EINA_TRUE;
}

static Eina_Bool
evas_image_load_file_data_heif_internal(Evas_Loader_Internal *loader,
                                        Emile_Image_Property *prop,
                                        void *pixels,
                                        void *map, size_t length,
                                        int *error)
{
   struct heif_image *img = NULL;
   struct heif_error err;
   const unsigned char *data;
   unsigned char *dd;
   unsigned char *plane;
   int stride;
   unsigned int x;
   unsigned int y;
   Eina_Bool ret;

   ret = EINA_FALSE;

   if (!evas_image_load_file_head_heif_init(loader, prop, map, length, error))
     return ret;

   err = heif_decode_image_f(loader->handle, &img, heif_colorspace_RGB,
                             prop->alpha ? heif_chroma_interleaved_RGBA
                                         : heif_chroma_interleaved_RGB,
                             NULL);

   if (err.code != heif_error_Ok)
     {
        INF("%s", err.message);
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        goto on_error;
     }

   data = heif_image_get_plane_readonly_f(img, heif_channel_interleaved, &stride);
   if (!data)
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        goto on_error;
     }

   dd = (unsigned char *)pixels;
   plane = (unsigned char *)data;
   if (!prop->alpha)
     {
       for (y = 0; y < prop->h; y++, plane += stride)
          {
             int from = 0;
             for (x = 0; x < prop->w; x++, from += 3)
               {
                  dd[0] = *(plane + from + 2);
                  dd[1] = *(plane + from + 1);
                  dd[2] = *(plane + from + 0);
                  dd[3] = 0xff;
                  dd += 4;
               }
          }
     }
   else
     {
        for (y = 0; y < prop->h; y++, plane += stride)
          {
             int from = 0;
             for (x = 0; x < prop->w; x++, from += 4)
               {
                  dd[0] = *(plane + from + 2);
                  dd[1] = *(plane + from + 1);
                  dd[2] = *(plane + from + 0);
                  dd[3] = *(plane + from + 3);
                  dd += 4;
               }
          }
     }

   ret = EINA_TRUE;

  *error = EVAS_LOAD_ERROR_NONE;
  prop->premul = EINA_TRUE;

 on_error:
   /* the context reads from the file map, which the caller unmaps
    * right after we return, so drop everything now */
   if (img)
     heif_image_release_f(img);
   heif_image_handle_release_f(loader->handle);
   heif_context_free_f(loader->ctx);
   loader->handle = NULL;
   loader->ctx = NULL;
   return ret;
}


static void *
evas_image_load_file_open_heif(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
			       Evas_Image_Load_Opts *opts,
			       Evas_Image_Animated *animated EINA_UNUSED,
			       int *error)
{
   Evas_Loader_Internal *loader;

   loader = calloc(1, sizeof (Evas_Loader_Internal));
   if (!loader)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return NULL;
     }

   loader->f = f;
   loader->opts = opts;

   return loader;
}

static void
evas_image_load_file_close_heif(void *loader_data)
{
   Evas_Loader_Internal *loader;

   loader = loader_data;
   if (loader->handle)
     heif_image_handle_release_f(loader->handle);
   if (loader->ctx)
     heif_context_free_f(loader->ctx);
   free(loader_data);
}

static Eina_Bool
evas_image_load_file_head_heif(void *loader_data,
                               Evas_Image_Property *prop,
                               int *error)
{
   Evas_Loader_Internal *loader = loader_data;
   Eina_File *f;
   void *map;
   Eina_Bool val;

   f = loader->f;

   map = eina_file_map_all(f, EINA_FILE_RANDOM);
   if (!map)
     {
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
        return EINA_FALSE;
     }

   val = evas_image_load_file_head_heif_internal(loader,
                                                 (Emile_Image_Property *)prop,
                                                 map, eina_file_size_get(f),
                                                 error);

   eina_file_map_free(f, map);

   return val;
}

static Eina_Bool
evas_image_load_file_data_heif(void *loader_data,
                               Evas_Image_Property *prop,
			       void *pixels,
			       int *error)
{
   Evas_Loader_Internal *loader;
   Eina_File *f;
   void *map;
   Eina_Bool val = EINA_FALSE;

   loader = (Evas_Loader_Internal *)loader_data;
   f = loader->f;

   map = eina_file_map_all(f, EINA_FILE_WILLNEED);
   if (!map)
     {
        *error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
        goto on_error;
     }

   val = evas_image_load_file_data_heif_internal(loader,
                                                 (Emile_Image_Property *)prop,
                                                 pixels,
                                                 map, eina_file_size_get(f),
                                                 error);

   eina_file_map_free(f, map);

 on_error:
   return val;
}

static const Evas_Image_Load_Func evas_image_load_heif_func = {
   EVAS_IMAGE_LOAD_VERSION,
   evas_image_load_file_open_heif,
   evas_image_load_file_close_heif,
   evas_image_load_file_head_heif,
   NULL,
   evas_image_load_file_data_heif,
   NULL,
   EINA_TRUE,
   EINA_FALSE
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;

   _evas_loader_heif_log_dom = eina_log_domain_register("evas-heif", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_loader_heif_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }

   em->functions = (void *)(&evas_image_load_heif_func);

#if defined (_WIN32)
   LOAD("libheif-1.dll");
   LOAD("libheif.dll");
#elif defined (_CYGWIN__)
   LOAD("cygheif-1.dll");
#elif defined(__APPLE__) && defined(__MACH__)
   LOAD("libheif.1.dylib");
#else
   LOAD("libheif.so.1");
#endif

   if (!_evas_loader_heif_mod)
     {
        EINA_LOG_ERR("Can not open libheif shared library.");
        goto error;
     }

   SYM(heif_check_filetype);
   SYM(heif_context_alloc);
   SYM(heif_context_free);
   SYM(heif_context_get_primary_image_handle);
   SYM(heif_context_read_from_memory_without_copy);
   SYM(heif_decode_image);
   SYM(heif_image_get_plane_readonly);
   SYM(heif_image_handle_get_height);
   SYM(heif_image_handle_get_width);
   SYM(heif_image_handle_has_alpha_channel);
   SYM(heif_image_handle_release);
   SYM(heif_image_release);

#define OPT_SYM(x) \
   x ## _f = eina_module_symbol_get(_evas_loader_heif_mod, #x)
   OPT_SYM(heif_image_handle_get_number_of_thumbnails);
   OPT_SYM(heif_image_handle_get_list_of_thumbnail_IDs);
   OPT_SYM(heif_image_handle_get_thumbnail);
   OPT_SYM(heif_image_handle_get_item_id);
   OPT_SYM(heif_item_get_transformation_properties);
   OPT_SYM(heif_item_get_property_type);
   OPT_SYM(heif_item_get_property_transform_rotation_ccw);
   OPT_SYM(heif_item_get_property_transform_mirror);
#undef OPT_SYM
   _evas_loader_heif_thumbnails =
     heif_image_handle_get_number_of_thumbnails_f &&
     heif_image_handle_get_list_of_thumbnail_IDs_f &&
     heif_image_handle_get_thumbnail_f &&
     heif_image_handle_get_item_id_f &&
     heif_item_get_transformation_properties_f &&
     heif_item_get_property_type_f &&
     heif_item_get_property_transform_rotation_ccw_f &&
     heif_item_get_property_transform_mirror_f;

   heif_init_f = eina_module_symbol_get(_evas_loader_heif_mod, "heif_init");
   heif_deinit_f = eina_module_symbol_get(_evas_loader_heif_mod, "heif_deinit");

   if (heif_init_f)
     {
        heif_init_f(NULL);
     }

   return 1;

 error:
   eina_log_domain_unregister(_evas_loader_heif_log_dom);
   _evas_loader_heif_log_dom = -1;
   return 0;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   if (heif_deinit_f)
     heif_deinit_f();

   if (_evas_loader_heif_mod)
     eina_module_free(_evas_loader_heif_mod);

   if (_evas_loader_heif_log_dom >= 0)
     {
        eina_log_domain_unregister(_evas_loader_heif_log_dom);
        _evas_loader_heif_log_dom = -1;
     }
}

static Evas_Module_Api evas_modapi =
  {
    EVAS_MODULE_API_VERSION,
    "heif",
    "none",
    {
      module_open,
      module_close
    }
  };

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, heif);


#ifndef EVAS_STATIC_BUILD_HEIF
EVAS_EINA_MODULE_DEFINE(image_loader, heif);
#endif

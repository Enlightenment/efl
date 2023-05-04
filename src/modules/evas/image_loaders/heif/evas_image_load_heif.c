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

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_loader_heif_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_loader_heif_log_dom, __VA_ARGS__)

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

   return EINA_TRUE;
}

static Eina_Bool
evas_image_load_file_data_heif_internal(Evas_Loader_Internal *loader,
                                        Emile_Image_Property *prop,
                                        void *pixels,
                                        void *map, size_t length,
                                        int *error)
{
   struct heif_image *img;
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

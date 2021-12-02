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
};

static int _evas_loader_heif_log_dom = -1;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_loader_heif_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_loader_heif_log_dom, __VA_ARGS__)

static Eina_Bool
evas_image_load_file_head_heif_internal(Evas_Loader_Internal *loader EINA_UNUSED,
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
       (heif_check_filetype(map, length) == heif_filetype_no))
     {
        INF("HEIF header invalid");
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        return ret;
     }

   ctx = heif_context_alloc();
   if (!ctx)
     {
        INF("cannot allocate heif_context");
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        return ret;
     }

   err = heif_context_read_from_memory_without_copy(ctx, map, length, NULL);
   if (err.code != heif_error_Ok)
     {
        INF("%s", err.message);
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        goto free_ctx;
   }

   err = heif_context_get_primary_image_handle(ctx, &handle);
   if (err.code != heif_error_Ok)
     {
        INF("%s", err.message);
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        goto free_ctx;
     }

   prop->w = heif_image_handle_get_width(handle);
   prop->h = heif_image_handle_get_height(handle);

   /* if size is invalid, we exit */
   if ((prop->w < 1) || (prop->h < 1) ||
       (prop->w > IMG_MAX_SIZE) || (prop->h > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(prop->w, prop->h))
     {
        if (IMG_TOO_BIG(prop->w, prop->h))
          *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        else
          *error= EVAS_LOAD_ERROR_GENERIC;
        goto release_handle;
     }

   prop->alpha = !!heif_image_handle_has_alpha_channel(handle);

   *error = EVAS_LOAD_ERROR_NONE;
   ret = EINA_TRUE;

 release_handle:
   heif_image_handle_release(handle);
 free_ctx:
   heif_context_free(ctx);

   return ret;
}

static Eina_Bool
evas_image_load_file_data_heif_internal(Evas_Loader_Internal *loader,
                                        Emile_Image_Property *prop,
                                        void *pixels,
                                        void *map, size_t length,
                                        int *error)
{
   struct heif_context *ctx;
   struct heif_image_handle *handle;
   struct heif_image *img;
   struct heif_error err;
   const unsigned char *data;
   unsigned char *dd;
   unsigned char *ds;
   int stride;
   unsigned int x;
   unsigned int y;
   Eina_Bool ret;

   ret = EINA_FALSE;

   ctx = loader->ctx;
   if (!ctx)
     {
        ctx = heif_context_alloc();
        if (!ctx)
          {
            INF("cannot allocate heif_context");
            *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
            return ret;
          }

        err = heif_context_read_from_memory_without_copy(ctx,
                                                         map, length, NULL);
        if (err.code != heif_error_Ok)
          {
             INF("%s", err.message);
             *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
             heif_context_free(ctx);
             return ret;
          }

        err = heif_context_get_primary_image_handle(ctx, &handle);
        if (err.code != heif_error_Ok)
          {
             INF("%s", err.message);
             *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
             heif_image_handle_release(handle);
             heif_context_free(ctx);
             return ret;
          }

        loader->ctx = ctx;
     }

   err = heif_decode_image(handle, &img, heif_colorspace_RGB,
                           prop->alpha ? heif_chroma_interleaved_RGBA
                                       : heif_chroma_interleaved_RGB,
                           NULL);

   if (err.code != heif_error_Ok)
     {
        INF("%s", err.message);
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        goto on_error;
     }

   data = heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride);

   dd  = (unsigned char *)pixels;
   ds = (unsigned char *)data;
   if (!prop->alpha)
     {
       for (y = 0; y < prop->h; y++)
         {
           for (x = 0; x < prop->w; x++)
             {
               dd[3] = 0xff;
               dd[0] = ds[2];
               dd[1] = ds[1];
               dd[2] = ds[0];
               ds+=3;
               dd+=4;
             }
         }
     }
   else
     {
       for (y = 0; y < prop->h; y++)
         {
           for (x = 0; x < prop->w; x++)
             {
               dd[0] = ds[2];
               dd[1] = ds[1];
               dd[2] = ds[0];
               dd[3] = ds[3];
               ds+=4;
               dd+=4;
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
   /*
    * in case _head() fails (because the file is not an heif one),
    * loader is not filled and loader->ctx is NULL
    */
   if (loader->ctx)
     heif_context_free(loader->ctx);
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

   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
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

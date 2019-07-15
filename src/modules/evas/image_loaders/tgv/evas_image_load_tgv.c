#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Emile.h>

#include "Evas_Loader.h"

typedef struct _Evas_Loader_Internal Evas_Loader_Internal;
struct _Evas_Loader_Internal
{
   Emile_Image *image;

   Eina_Rectangle region;
};

static void *
evas_image_load_file_open_tgv(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
                              Evas_Image_Load_Opts *opts,
                              Evas_Image_Animated *animated EINA_UNUSED,
                              int *error)
{
   Evas_Loader_Internal *loader;
   Emile_Image *image;
   Emile_Image_Load_Error image_error;

   image = emile_image_tgv_file_open(f, opts ? &(opts->emile) : NULL,
                                     NULL, &image_error);
   if (!image)
     {
        *error = image_error;
        return NULL;
     }

   loader = calloc(1, sizeof (Evas_Loader_Internal));
   if (!loader)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return NULL;
     }

   loader->image = image;
   if (opts && (opts->emile.region.w > 0) && (opts->emile.region.h > 0))
     {
        EINA_RECTANGLE_SET(&loader->region,
                           opts->emile.region.x,
                           opts->emile.region.y,
                           opts->emile.region.w,
                           opts->emile.region.h);
     }
   else
     {
        EINA_RECTANGLE_SET(&loader->region,
                           0, 0,
                           -1, -1);
     }

   return loader;
}


static void
evas_image_load_file_close_tgv(void *loader_data)
{
   Evas_Loader_Internal *loader = loader_data;

   emile_image_close(loader->image);
   free(loader);
}

static Eina_Bool
evas_image_load_file_head_tgv(void *loader_data,
                              Emile_Image_Property *prop,
                              int *error)
{
   Evas_Loader_Internal *loader = loader_data;
   Emile_Image_Load_Error image_error;
   Eina_Bool ret;

   ret = emile_image_head(loader->image,
                          prop, sizeof (*prop),
                          &image_error);
   *error = image_error;

   return ret;
}

Eina_Bool
evas_image_load_file_data_tgv(void *loader_data,
                              Emile_Image_Property *prop,
                              void *pixels,
                              int *error)
{
   Evas_Loader_Internal *loader = loader_data;
   Emile_Image_Load_Error image_error;
   Eina_Bool ret;

   ret = emile_image_data(loader->image,
                          prop, sizeof (*prop),
                          pixels,
                          &image_error);
   *error = image_error;
   return ret;
}

Evas_Image_Load_Func evas_image_load_tgv_func =
{
  EVAS_IMAGE_LOAD_VERSION,
  evas_image_load_file_open_tgv,
  evas_image_load_file_close_tgv,
  (void*) evas_image_load_file_head_tgv,
  NULL,
  (void*) evas_image_load_file_data_tgv,
  NULL,
  EINA_TRUE,
  EINA_FALSE
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_tgv_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "tgv",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, tgv);

#ifndef EVAS_STATIC_BUILD_TGV
EVAS_EINA_MODULE_DEFINE(image_loader, tgv);
#endif

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include <avif/avif.h>

#include "Evas_Loader.h"

typedef struct _Evas_Loader_Internal Evas_Loader_Internal;
struct _Evas_Loader_Internal
{
   Eina_File *f;
   Evas_Image_Load_Opts *opts;
};

static int _evas_loader_avif_log_dom = -1;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_loader_avif_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_loader_avif_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_loader_avif_log_dom, __VA_ARGS__)

static Eina_Bool
evas_image_load_file_head_avif_internal(unsigned int *w, unsigned int *h,
					unsigned char *alpha,
                                        void *map, size_t length,
                                        int *error)
{
   avifROData raw;
   avifDecoder *decoder;
   avifImage *image;
   avifResult res;

   raw.size = length;
   raw.data = (const uint8_t *)map;

   decoder = avifDecoderCreate();
   if (!decoder)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return EINA_FALSE;
     }

   image = avifImageCreateEmpty();
   if (!image)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto destroy_decoder;
     }

   res = avifDecoderRead(decoder, image, &raw);
   if (res != AVIF_RESULT_OK)
     {
        ERR("avif file format invalid");
        *error = EVAS_LOAD_ERROR_GENERIC;
        goto destroy_image;
     }

   *w = image->width;
   *h = image->height;
   *alpha = !!image->alphaPlane;
   *error = EVAS_LOAD_ERROR_NONE;

   avifImageDestroy(image);
   avifDecoderDestroy(decoder);

   return EINA_TRUE;

 destroy_image:
   avifImageDestroy(image);
 destroy_decoder:
   avifDecoderDestroy(decoder);

   return EINA_FALSE;
}

static Eina_Bool
evas_image_load_file_data_avif_internal(void *pixels,
                                        void *map, size_t length,
                                        int *error)
{
   avifRGBImage rgb;
   avifROData raw;
   avifDecoder *decoder;
   avifImage *image;
   avifResult res;

   raw.size = length;
   raw.data = (const uint8_t *)map;

   decoder = avifDecoderCreate();
   if (!decoder)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return EINA_FALSE;
     }

   image = avifImageCreateEmpty();
   if (!image)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto destroy_decoder;
     }

   res = avifDecoderRead(decoder, image, &raw);
   if (res != AVIF_RESULT_OK)
     {
        ERR("avif file format invalid");
        *error = EVAS_LOAD_ERROR_GENERIC;
        goto destroy_image;
     }

   avifRGBImageSetDefaults(&rgb, image);
   rgb.format = AVIF_RGB_FORMAT_BGRA;
   rgb.depth = 8;
   rgb.pixels = pixels;
   rgb.rowBytes = 4 * image->width;

   avifImageYUVToRGB(image, &rgb);

   *error = EVAS_LOAD_ERROR_NONE;

   avifImageDestroy(image);
   avifDecoderDestroy(decoder);

   return EINA_TRUE;

 destroy_image:
   avifImageDestroy(image);
 destroy_decoder:
   avifDecoderDestroy(decoder);

   return EINA_FALSE;
}

static void *
evas_image_load_file_open_avif(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
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
evas_image_load_file_close_avif(void *loader_data)
{
   free(loader_data);
}

static Eina_Bool
evas_image_load_file_head_avif(void *loader_data,
                               Emile_Image_Property *prop,
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

   val = evas_image_load_file_head_avif_internal(&prop->w, &prop->h,
						 &prop->alpha,
                                                 map, eina_file_size_get(f),
                                                 error);

   eina_file_map_free(f, map);

   return val;
}

static Eina_Bool
evas_image_load_file_data_avif(void *loader_data,
                               Emile_Image_Property *prop EINA_UNUSED,
			       void *pixels,
			       int *error)
{
   Evas_Loader_Internal *loader = loader_data;
   Eina_File *f;
   void *map;
   Eina_Bool val = EINA_FALSE;

   f = loader->f;

   map = eina_file_map_all(f, EINA_FILE_WILLNEED);
   if (!map)
     {
        *error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
        goto on_error;
     }

   val = evas_image_load_file_data_avif_internal(pixels,
                                                 map, eina_file_size_get(f),
                                                 error);

   eina_file_map_free(f, map);

 on_error:
   return val;
}

static Evas_Image_Load_Func evas_image_load_avif_func =
{
  EVAS_IMAGE_LOAD_VERSION,
  evas_image_load_file_open_avif,
  evas_image_load_file_close_avif,
  (void*) evas_image_load_file_head_avif,
  NULL,
  (void*) evas_image_load_file_data_avif,
  NULL,
  EINA_TRUE,
  EINA_TRUE
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;

   _evas_loader_avif_log_dom = eina_log_domain_register("evas-avif", EINA_COLOR_BLUE);
   if (_evas_loader_avif_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }

   em->functions = (void *)(&evas_image_load_avif_func);

   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   if (_evas_loader_avif_log_dom >= 0)
     {
        eina_log_domain_unregister(_evas_loader_avif_log_dom);
        _evas_loader_avif_log_dom = -1;
     }
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "avif",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, avif);

#ifndef EVAS_STATIC_BUILD_AVIF
EVAS_EINA_MODULE_DEFINE(image_loader, avif);
#endif


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include <avif/avif.h>

#include "Evas_Loader.h"
#include "evas_common_private.h"

typedef struct _Evas_Loader_Internal Evas_Loader_Internal;
struct _Evas_Loader_Internal
{
   Eina_File *f;
   Evas_Image_Load_Opts *opts;
   Evas_Image_Animated *animated;
   avifDecoder *decoder;
   double duration;
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
evas_image_load_file_head_avif_internal(Evas_Loader_Internal *loader,
                                        Emile_Image_Property *prop,
                                        void *map, size_t length,
                                        int *error)
{
   Evas_Image_Animated *animated;
   avifROData raw;
   avifDecoder *decoder;
   avifResult res;
   Eina_Bool ret;

   animated = loader->animated;

   ret = EINA_FALSE;
   prop->w = 0;
   prop->h = 0;
   prop->alpha = EINA_FALSE;

   raw.size = length;
   raw.data = (const uint8_t *)map;

   decoder = avifDecoderCreate();
   if (!decoder)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return ret;
     }

   res = avifDecoderParse(decoder, &raw);
   if (res != AVIF_RESULT_OK)
     {
        ERR("avif file format invalid");
        *error = EVAS_LOAD_ERROR_GENERIC;
        goto destroy_decoder;
     }

   if (decoder->imageCount < 1)
     {
        ERR("avif file format invalid");
        *error = EVAS_LOAD_ERROR_GENERIC;
        goto destroy_decoder;
     }

   res = avifDecoderNextImage(decoder);
   if (res != AVIF_RESULT_OK)
     {
        ERR("avif file format invalid");
        *error = EVAS_LOAD_ERROR_GENERIC;
        goto destroy_decoder;
     }

   prop->w = decoder->image->width;
   prop->h = decoder->image->height;

   /* if size is invalid, we exit */
   if ((prop->w < 1) || (prop->h < 1) ||
       (prop->w > IMG_MAX_SIZE) || (prop->h > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(prop->w, prop->h))
     {
        if (IMG_TOO_BIG(prop->w, prop->h))
          *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        else
          *error= EVAS_LOAD_ERROR_GENERIC;
        goto destroy_decoder;
     }

   prop->alpha = !!decoder->image->alphaPlane;

   if (decoder->imageCount > 1)
     {
        animated->loop_hint = EVAS_IMAGE_ANIMATED_HINT_NONE;
        animated->frame_count = decoder->imageCount;
        animated->loop_count = 1;
        animated->animated = EINA_TRUE;
        loader->duration = decoder->duration / decoder->imageCount;
     }

   *error = EVAS_LOAD_ERROR_NONE;
   ret = EINA_TRUE;

 destroy_decoder:
   avifDecoderDestroy(decoder);

   return ret;
}

static Eina_Bool
evas_image_load_file_data_avif_internal(Evas_Loader_Internal *loader,
                                        void *pixels,
                                        void *map, size_t length,
                                        int *error)
{
   avifRGBImage rgb;
   avifDecoder *decoder;
   avifResult res;
   Evas_Image_Animated *animated;
   Eina_Bool ret;

   ret = EINA_FALSE;

   /* FIXME: create decoder in evas_image_load_file_data_avif instead ? */
   decoder = loader->decoder;
   if (!decoder)
     {
        avifROData raw;
        decoder = avifDecoderCreate();
        if (!decoder)
          {
             *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
             return EINA_FALSE;
          }

        raw.size = length;
        raw.data = (const uint8_t *)map;

        res = avifDecoderParse(decoder, &raw);
        if (res != AVIF_RESULT_OK)
          {
             *error = EVAS_LOAD_ERROR_GENERIC;
             goto on_error;
          }

        loader->decoder = decoder;
     }

   animated = loader->animated;
   if (animated->animated)
     {
        /* FIXME: next image instead ? */
        res = avifDecoderNthImage(decoder, animated->cur_frame + 1);
        if (res != AVIF_RESULT_OK)
          {
             *error = EVAS_LOAD_ERROR_GENERIC;
             goto on_error;
          }
     }
   else
     {
        res = avifDecoderNextImage(decoder);
        if (res != AVIF_RESULT_OK)
          {
             *error = EVAS_LOAD_ERROR_GENERIC;
             goto on_error;
          }
     }

   avifRGBImageSetDefaults(&rgb, decoder->image);
#ifdef WORDS_BIGENDIAN
   rgb.format = AVIF_RGB_FORMAT_ARGB;
#else
   rgb.format = AVIF_RGB_FORMAT_BGRA;
#endif
   rgb.depth = 8;
   rgb.pixels = pixels;
   rgb.rowBytes = 4 * decoder->image->width;

   avifImageYUVToRGB(decoder->image, &rgb);

   *error = EVAS_LOAD_ERROR_NONE;

   ret = EINA_TRUE;

 on_error:

   return ret;
}

static void *
evas_image_load_file_open_avif(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
			       Evas_Image_Load_Opts *opts,
			       Evas_Image_Animated *animated,
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
   loader->animated = animated;

   return loader;
}

static void
evas_image_load_file_close_avif(void *loader_data)
{
   Evas_Loader_Internal *loader;

   loader = loader_data;
   avifDecoderDestroy(loader->decoder);
   free(loader_data);
}

static Eina_Bool
evas_image_load_file_head_avif(void *loader_data,
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

   val = evas_image_load_file_head_avif_internal(loader,
                                                 (Emile_Image_Property *)prop,
                                                 map, eina_file_size_get(f),
                                                 error);

   eina_file_map_free(f, map);

   return val;
}

static Eina_Bool
evas_image_load_file_data_avif(void *loader_data,
                               Evas_Image_Property *prop EINA_UNUSED,
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

   val = evas_image_load_file_data_avif_internal(loader,
                                                 pixels,
                                                 map, eina_file_size_get(f),
                                                 error);

   eina_file_map_free(f, map);

 on_error:
   return val;
}

static double
evas_image_load_frame_duration_avif(void *loader_data,
                                    int start_frame,
                                    int frame_num)
{
   Evas_Loader_Internal *loader;
   Evas_Image_Animated *animated;

   loader = (Evas_Loader_Internal *)loader_data;
   animated = loader->animated;

   if (!animated->animated)
     return -1.0;

   if (frame_num < 0)
     return -1.0;

   if ((start_frame + frame_num) > animated->frame_count)
     return -1.0;

   if (frame_num < 1)
     frame_num = 1;

   return loader->duration;
}

static Evas_Image_Load_Func evas_image_load_avif_func =
{
   EVAS_IMAGE_LOAD_VERSION,
   evas_image_load_file_open_avif,
   evas_image_load_file_close_avif,
   evas_image_load_file_head_avif,
   NULL,
   evas_image_load_file_data_avif,
   evas_image_load_frame_duration_avif,
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


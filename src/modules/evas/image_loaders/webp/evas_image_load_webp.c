#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <webp/decode.h>

#include "evas_common_private.h"
#include "evas_private.h"

static Eina_Bool
evas_image_load_file_check(Eina_File *f, void *map,
			   unsigned int *w, unsigned int *h, Eina_Bool *alpha,
			   int *error)
{
   WebPDecoderConfig config;

   if (eina_file_size_get(f) < 30) return EINA_FALSE;

   if (!WebPInitDecoderConfig(&config))
   {
      *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
      return EINA_FALSE;
   }
   if (WebPGetFeatures(map, 30, &config.input) != VP8_STATUS_OK)
   {
      *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
      return EINA_FALSE;
   }

   *w = config.input.width;
   *h = config.input.height;
   *alpha = config.input.has_alpha;

   return EINA_TRUE;
}

static void *
evas_image_load_file_open_webp(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
			       Evas_Image_Load_Opts *opts EINA_UNUSED,
			       Evas_Image_Animated *animated EINA_UNUSED,
			       int *error EINA_UNUSED)
{
   return f;
}

static void
evas_image_load_file_close_webp(void *loader_data EINA_UNUSED)
{
}

static Eina_Bool
evas_image_load_file_head_webp(void *loader_data,
			       Evas_Image_Property *prop,
			       int *error)
{
   Eina_File *f = loader_data;
   Eina_Bool r;
   void *data;

   *error = EVAS_LOAD_ERROR_NONE;

   data = eina_file_map_all(f, EINA_FILE_RANDOM);

   r = evas_image_load_file_check(f, data,
				  &prop->w, &prop->h, &prop->alpha,
				  error);

   if (data) eina_file_map_free(f, data);
   return r;
}

static Eina_Bool
evas_image_load_file_data_webp(void *loader_data,
			       Evas_Image_Property *prop,
			       void *pixels,
			       int *error)
{
   Eina_File *f = loader_data;
   void *data = NULL;
   void *decoded = NULL;
   void *surface = NULL;
   int width, height;

   data = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);

   surface = pixels;

   decoded = WebPDecodeBGRA(data, eina_file_size_get(f), &width, &height);
   if (!decoded)
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        goto free_data;
     }
   *error = EVAS_LOAD_ERROR_NONE;

   if ((int) prop->w != width ||
       (int) prop->h != height)
     goto free_data;

   // XXX: this copy of the surface is inefficient
   memcpy(surface, decoded, width * height * 4);
   prop->premul = EINA_TRUE;

 free_data:
   if (data) eina_file_map_free(f, data);
   free(decoded);

   return EINA_TRUE;
}

static Evas_Image_Load_Func evas_image_load_webp_func =
{
  evas_image_load_file_open_webp,
  evas_image_load_file_close_webp,
  evas_image_load_file_head_webp,
  evas_image_load_file_data_webp,
  NULL,
  EINA_TRUE,
  EINA_FALSE
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_webp_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "webp",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, webp);

#ifndef EVAS_STATIC_BUILD_WEBP
EVAS_EINA_MODULE_DEFINE(image_loader, webp);
#endif

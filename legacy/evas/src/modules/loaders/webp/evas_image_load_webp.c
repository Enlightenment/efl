#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <webp/decode.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "evas_common.h"
#include "evas_private.h"

static Eina_Bool evas_image_load_file_head_webp(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);
static Eina_Bool evas_image_load_file_data_webp(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);

static Evas_Image_Load_Func evas_image_load_webp_func =
{
  EINA_TRUE,
  evas_image_load_file_head_webp,
  evas_image_load_file_data_webp,
  NULL,
  EINA_FALSE
};


static Eina_Bool
evas_image_load_file_head_webp(Image_Entry *ie, const char *file, const char *key __UNUSED__, int *error)
{
   WebPDecoderConfig config;
   FILE *f;
   size_t header_size = 30;
   uint8_t header[30];

   f = fopen(file, "rb");
   if (!f)
   {
      *error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
      return EINA_FALSE;
   }
   if (fread(header, header_size, 1, f) != 1)
   {
      fclose(f);
      *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
      return EINA_FALSE;
   }
   fclose(f);

   if (!WebPInitDecoderConfig(&config))
   {
      *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
      return EINA_FALSE;
   }
   if (WebPGetFeatures(header, header_size, &config.input) != VP8_STATUS_OK)
   {
      *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
      return EINA_FALSE;
   }

   ie->w = config.input.width;
   ie->h = config.input.height;
   ie->flags.alpha = config.input.has_alpha;

   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
}

static Eina_Bool
evas_image_load_file_data_webp(Image_Entry *ie, const char *file, const char *key __UNUSED__, int *error)
{
   FILE *f;
   size_t file_size;
   uint8_t *data, *decoded, *surface;
   int width, height;

   f = fopen(file, "rb");
   if (!f)
   {
      *error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
      return EINA_FALSE;
   }

   if (fseek(f, 0, SEEK_END) != 0)
   {
      *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
      goto close_file;
   }
   file_size = ftell(f);

   if (fseek(f, 0, SEEK_SET) != 0)
   {
      *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
      goto close_file;
   }

   data = malloc(file_size);
   if (!data)
   {
      *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
      goto close_file;
   }
   if (fread(data, file_size, 1, f) != 1)
   {
      *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
      goto free_data;
   }

   evas_cache_image_surface_alloc(ie, ie->w, ie->h);
   surface = evas_cache_image_pixels(ie);
   if (!surface)
   {
      *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
      goto free_data;
   }

   decoded = WebPDecodeBGRA(data, file_size, &width, &height);
   memcpy(surface, decoded, width * height * 4);
   evas_common_image_premul(ie);  

   free(decoded);
   free(data);
   fclose(f);

   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;

free_data:
   free(data);

close_file:
   fclose(f);
   return EINA_FALSE;
}

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_webp_func);
   return 1;
}

static void
module_close(Evas_Module *em __UNUSED__)
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

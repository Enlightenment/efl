#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <webp/decode.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "evas_macros.h"

#include "evas_cserve2.h"
#include "evas_cserve2_slave.h"


static Eina_Bool
evas_image_load_file_head_webp(Evas_Img_Load_Params *ilp, const char *file, const char *key __UNUSED__, int *error)
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
   
   ilp->w = config.input.width;
   ilp->h = config.input.height;
   ilp->alpha = config.input.has_alpha;
   
   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
}

static Eina_Bool
evas_image_load_file_data_webp(Evas_Img_Load_Params *ilp, const char *file, const char *key __UNUSED__, int *error)
{
   FILE *f;
   size_t file_size;
   uint8_t *data, *decoded;
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
      free(data);
      *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
      goto close_file;
   }
   fclose(f);

   decoded = WebPDecodeBGRA(data, file_size, &width, &height);
   memcpy(ilp->buffer, decoded, width * height * 4);
   evas_cserve2_image_premul(ilp);
   
   free(decoded);
   free(data);

   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;

close_file:
   fclose(f);
   return EINA_FALSE;
}

static Evas_Loader_Module_Api modapi =
{
   EVAS_CSERVE2_MODULE_API_VERSION,
   "webp",
   evas_image_load_file_head_webp,
   evas_image_load_file_data_webp
};

static Eina_Bool
module_init(void)
{
   return evas_cserve2_loader_register(&modapi);
}

static void
module_shutdown(void)
{
}

EINA_MODULE_INIT(module_init);
EINA_MODULE_SHUTDOWN(module_shutdown);

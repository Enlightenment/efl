#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include <avif/avif.h>

#include "evas_common_private.h"
#include "evas_private.h"


static int
save_image_avif(RGBA_Image *im, const char *file, int quality)
{
   FILE *f;
   avifRGBImage rgb;
   avifRWData output;
   avifImage *image;
   avifEncoder * encoder;
   avifPixelFormat format;
   avifResult res;
   size_t size;
   int threads_count;
   int quantizer;
   int color;
   int transfer;
   int matrix;
   int ret = 0;

   if (!im || !im->image.data || !file || !*file)
     return 0;

   f = fopen(file, "wb");
   if (!f)
     return ret;

   if (quality < 60)
     {
        format = AVIF_PIXEL_FORMAT_YUV420;
#if (AVIF_VERSION < 704)
        matrix = AVIF_NCLX_MATRIX_COEFFICIENTS_BT601;
        color = AVIF_NCLX_COLOUR_PRIMARIES_BT601;
#else
        matrix = AVIF_MATRIX_COEFFICIENTS_BT601;
        color = AVIF_COLOR_PRIMARIES_BT601;
#endif
     }
   else if (quality >= 90)
     {
        format = AVIF_PIXEL_FORMAT_YUV444;
#if (AVIF_VERSION < 704)
        matrix = AVIF_NCLX_MATRIX_COEFFICIENTS_BT709;
        color = AVIF_NCLX_COLOUR_PRIMARIES_BT709;
#else
        matrix = AVIF_MATRIX_COEFFICIENTS_BT709;
        color = AVIF_COLOR_PRIMARIES_BT709;
#endif
     }
   else
     {
        format = AVIF_PIXEL_FORMAT_YUV422;
#if (AVIF_VERSION < 704)
        matrix = AVIF_NCLX_MATRIX_COEFFICIENTS_BT709;
        color = AVIF_NCLX_COLOUR_PRIMARIES_BT709;
#else
        matrix = AVIF_MATRIX_COEFFICIENTS_BT709;
        color = AVIF_COLOR_PRIMARIES_BT709;
#endif
     }

#if (AVIF_VERSION < 704)
   transfer = AVIF_NCLX_TRANSFER_CHARACTERISTICS_SRGB;
#else
   transfer = AVIF_TRANSFER_CHARACTERISTICS_SRGB;
#endif

   image = avifImageCreate(im->cache_entry.w, im->cache_entry.h, 8, format);
   if (!image)
     goto close_f;

#if (AVIF_VERSION < 704)
   image->nclx.colourPrimaries = color;
   image->nclx.transferCharacteristics = transfer;
   image->nclx.matrixCoefficients = matrix;
#else
   image->colorPrimaries = color;
   image->transferCharacteristics = transfer;
   image->matrixCoefficients = matrix;
#endif
   image->yuvRange = AVIF_RANGE_FULL;

   avifRGBImageSetDefaults(&rgb, image);
#ifdef WORDS_BIGENDIAN
   rgb.format = AVIF_RGB_FORMAT_ARGB;
#else
   rgb.format = AVIF_RGB_FORMAT_BGRA;
#endif
   rgb.depth = 8;
   rgb.pixels = (uint8_t *)im->image.data;
   rgb.rowBytes = 4 * im->cache_entry.w;
   avifImageRGBToYUV(image, &rgb);

   output.data = NULL;
   output.size = 0;
   encoder = avifEncoderCreate();
   if (!encoder)
     goto destroy_image;

   threads_count = 1;
   if (eina_cpu_count() > 2)
     threads_count = eina_cpu_count() - 1;

   quantizer = ((100 - quality) * AVIF_QUANTIZER_WORST_QUALITY) / 100;

   encoder->maxThreads = threads_count;
   encoder->minQuantizer = quantizer;
   encoder->maxQuantizer = quantizer;
   res = avifEncoderWrite(encoder, image, &output);

   if (res != AVIF_RESULT_OK)
     goto destroy_encoder;

   size = fwrite(output.data, output.size, 1, f);
   if (size != 1)
     goto destroy_encoder;

   ret = 1;

 destroy_encoder:
   avifEncoderDestroy(encoder);
   avifRWDataFree(&output);
 destroy_image:
   avifImageDestroy(image);
 close_f:
   fclose(f);

   return ret;
}

static int evas_image_save_file_avif(RGBA_Image *im, const char *file, const char *key EINA_UNUSED,
                                     int quality, int compress EINA_UNUSED, const char *encoding EINA_UNUSED)
{
   return save_image_avif(im, file, quality);
}


static Evas_Image_Save_Func evas_image_save_avif_func =
{
   evas_image_save_file_avif
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_save_avif_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
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

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_SAVER, image_saver, avif);

#ifndef EVAS_STATIC_BUILD_AVIF
EVAS_EINA_MODULE_DEFINE(image_saver, avif);
#endif

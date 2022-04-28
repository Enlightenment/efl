#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include <jxl/encode.h>
#include <jxl/resizable_parallel_runner.h>

#include "evas_common_private.h"
#include "evas_private.h"


static int
save_image_jxl(RGBA_Image *im, const char *file, int quality)
{
   FILE *f;
   JxlParallelRunner *runner;
   JxlEncoder *encoder;
   JxlPixelFormat pixel_format;
   JxlBasicInfo basic_info;
   JxlColorEncoding color_encoding;
   JxlEncoderFrameSettings* frame_settings;
   JxlEncoderStatus st;
   JxlEncoderStatus process_result;
   unsigned char *compressed;
   unsigned char *next;
   void *pixels;
   unsigned long long int *iter_src;
   unsigned long long int *iter_dst;
   size_t size;
   size_t avail;
   size_t sz;
   unsigned int i;
   int ret = 0;

   if (!im || !im->image.data || !file || !*file)
     return ret;

   f = fopen(file, "wb");
   if (!f)
     return ret;

   runner = JxlResizableParallelRunnerCreate(NULL);
   if (!runner)
     goto close_f;

   encoder = JxlEncoderCreate(NULL);
   if (!encoder)
     goto destroy_runner;

   st = JxlEncoderSetParallelRunner(encoder,
                                    JxlResizableParallelRunner,
                                    runner);
   if (st != JXL_ENC_SUCCESS)
     goto destroy_encoder;

   JxlResizableParallelRunnerSetThreads(runner,
                                        JxlResizableParallelRunnerSuggestThreads(im->cache_entry.w, im->cache_entry.h));

   pixel_format.num_channels = 4;
   pixel_format.data_type = JXL_TYPE_UINT8;
#ifdef WORDS_BIGENDIAN
   pixel_format.endianness = JXL_BIG_ENDIAN;
#else
   pixel_format.endianness = JXL_LITTLE_ENDIAN;
#endif
   pixel_format.align = 0;

   JxlEncoderInitBasicInfo(&basic_info);
   basic_info.xsize = im->cache_entry.w;
   basic_info.ysize = im->cache_entry.h;
   basic_info.bits_per_sample = 8;
   basic_info.exponent_bits_per_sample = 0;
   basic_info.uses_original_profile = JXL_FALSE;
   basic_info.num_color_channels = 3;
   basic_info.num_extra_channels =1;
   basic_info.alpha_bits = 8;
   st = JxlEncoderSetBasicInfo(encoder, &basic_info);
   if (st != JXL_ENC_SUCCESS)
     goto destroy_encoder;

   memset(&color_encoding, 0, sizeof(JxlColorEncoding));
   JxlColorEncodingSetToSRGB(&color_encoding,
                             /*is gray ? */
                             pixel_format.num_channels < 3);
   st = JxlEncoderSetColorEncoding(encoder, &color_encoding);
   if (st != JXL_ENC_SUCCESS)
     goto destroy_encoder;

   frame_settings = JxlEncoderFrameSettingsCreate(encoder, NULL);
   if (!frame_settings)
     goto destroy_encoder;

   st = JxlEncoderFrameSettingsSetOption(frame_settings,
                                         JXL_ENC_FRAME_SETTING_EFFORT,
                                         (quality * 7) /100);
   if (st != JXL_ENC_SUCCESS)
     goto destroy_encoder;

   /* conversion RGBA --> BGRA */
   pixels = malloc(4 * im->cache_entry.w * im->cache_entry.h);
   if (!pixels)
     goto destroy_encoder;

   iter_src = (unsigned long long int *)im->image.data;
   iter_dst = (unsigned long long int *)pixels;

   for (i = 0; i < ((im->cache_entry.w * im->cache_entry.h) >> 1); i++, iter_src++, iter_dst++)
     {
        *iter_dst =
          /* we keep A and G */
          (*iter_src & 0xff00ff00ff00ff00) |
          /* we shift R */
          ((*iter_src & 0x000000ff000000ff) << 16) |
          /* we shift B */
          ((*iter_src & 0x00ff000000ff0000) >> 16);
     }

   st = JxlEncoderAddImageFrame(frame_settings, &pixel_format,
                                (void*)pixels,
                                sizeof(int) * im->cache_entry.w * im->cache_entry.h);
   if (st != JXL_ENC_SUCCESS)
     goto free_pixels;

   JxlEncoderCloseInput(encoder);

   size = 64;
   compressed = (unsigned char *)malloc(size);
   if (!compressed)
     goto free_pixels;

   next = compressed;
   avail = size - (next - compressed);
   process_result = JXL_ENC_NEED_MORE_OUTPUT;
   while (process_result == JXL_ENC_NEED_MORE_OUTPUT)
     {
        process_result = JxlEncoderProcessOutput(encoder, &next, &avail);
        if (process_result == JXL_ENC_NEED_MORE_OUTPUT)
          {
             size_t offset = next - compressed;
             size *= 2;
             compressed = realloc(compressed, size);
             next = compressed + offset;
             avail = size - offset;
          }
     }
   size = next - compressed;
   compressed = realloc(compressed, size);
   if (process_result != JXL_ENC_SUCCESS)
     goto free_compressed;

   sz = fwrite(compressed, size, 1, f);
   if (sz != 1)
     goto free_compressed;

   ret = 1;

 free_compressed:
   free(compressed);
 free_pixels:
   free(pixels);
 destroy_encoder:
   JxlEncoderDestroy(encoder);
 destroy_runner:
   JxlResizableParallelRunnerDestroy(runner);
 close_f:
   fclose(f);

   return ret;
}

static int evas_image_save_file_jxl(RGBA_Image *im, const char *file, const char *key EINA_UNUSED,
                                     int quality, int compress EINA_UNUSED, const char *encoding EINA_UNUSED)
{
   return save_image_jxl(im, file, quality);
}


static Evas_Image_Save_Func evas_image_save_jxl_func =
{
   evas_image_save_file_jxl
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_save_jxl_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "jxl",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_SAVER, image_saver, jxl);

#ifndef EVAS_STATIC_BUILD_JXL
EVAS_EINA_MODULE_DEFINE(image_saver, jxl);
#endif

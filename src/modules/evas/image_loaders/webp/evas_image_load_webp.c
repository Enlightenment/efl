#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <webp/decode.h>
#include <webp/demux.h>

#include "evas_common_private.h"
#include "evas_private.h"

typedef struct _Loader_Info
{
   Eina_File *f;
   Evas_Image_Load_Opts *opts;
   Evas_Image_Animated *animated;
   WebPAnimDecoder *dec;
   void *map;
   Eina_Array *frames;
}Loader_Info;

// WebP Frame Information
typedef struct _Image_Frame
{
   int index;
   int timestamp;
   double delay;
   uint8_t *data;
}Image_Frame;

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
			       Evas_Image_Load_Opts *opts,
			       Evas_Image_Animated *animated,
			       int *error)
{
   Loader_Info *loader = calloc(1, sizeof (Loader_Info));
   if (!loader)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return NULL;
     }
   loader->f = eina_file_dup(f);
   loader->opts = opts;
   loader->animated = animated;
   return loader;
}

static void
_free_all_frame(Loader_Info *loader)
{
   Image_Frame *frame;

   if (!loader->frames) return;

   for (unsigned int i = 0; i < eina_array_count(loader->frames); ++i)
     {
        frame = eina_array_data_get(loader->frames, i);
        if (frame->data)
          {
             free(frame->data);
             frame->data = NULL;
          }
        free(frame);
     }
}


static void
evas_image_load_file_close_webp(void *loader_data)
{
   // Free Allocated Data
   Loader_Info *loader = loader_data;
   _free_all_frame(loader);
   eina_array_free(loader->frames);
   if (loader->dec) WebPAnimDecoderDelete(loader->dec);
   if ((loader->map) && (loader->f))
     eina_file_map_free(loader->f, loader->map);
   if (loader->f) eina_file_close(loader->f);
   free(loader);
}


static void
_new_frame(Loader_Info *loader, uint8_t *data, int width, int height, int index,
           int pre_timestamp, int cur_timestamp)
{
   // Allocate Frame Data
   Image_Frame *frame;

   frame = calloc(1, sizeof(Image_Frame));
   if (!frame) return;

   frame->data = calloc(width * height * 4, sizeof(uint8_t));
   if (!frame->data)
     {
        free(frame);
        return;
     }

   frame->index = index;
   frame->timestamp = cur_timestamp;
   frame->delay = ((double)(cur_timestamp - pre_timestamp)/1000.0);
   memcpy(frame->data, data, width * height * 4);

   eina_array_push(loader->frames, frame);
}

static Image_Frame *
_find_frame(Loader_Info *loader, int index)
{
   // Find Frame
   Image_Frame *frame;

   if (!loader->frames) return NULL;

   frame = eina_array_data_get(loader->frames, index - 1);
   if (frame->index == index)
     return frame;

   return NULL;
}

static Eina_Bool
evas_image_load_file_head_webp(void *loader_data,
			       Emile_Image_Property *prop,
			       int *error)
{
   Loader_Info *loader = loader_data;
   Evas_Image_Animated *animated = loader->animated;
   Eina_File *f = loader->f;
   void *data;

   *error = EVAS_LOAD_ERROR_NONE;

   data = eina_file_map_all(f, EINA_FILE_RANDOM);
   loader->map = data;

   if (!evas_image_load_file_check(f, data,
				  &prop->w, &prop->h, &prop->alpha,
				  error))
     {
        ERR("Image File is Invalid");
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        return EINA_FALSE;
     }

   // Init WebP Data
   WebPData webp_data;
   WebPDataInit(&webp_data);

   // Assign Data
   webp_data.bytes = data;
   webp_data.size = eina_file_size_get(f);

   // Set Decode Option
   WebPAnimDecoderOptions dec_options;
   WebPAnimDecoderOptionsInit(&dec_options);
   dec_options.color_mode = MODE_BGRA;

   // Create WebPAnimation Decoder
   WebPAnimDecoder *dec = WebPAnimDecoderNew(&webp_data, &dec_options);
   if (!dec)
     {
        ERR("WebP Decoder Creation is Failed");
        *error = EVAS_LOAD_ERROR_GENERIC;
        return EINA_FALSE;
     }
   loader->dec = dec;

   // Get WebP Animation Info
   WebPAnimInfo anim_info;
   if (!WebPAnimDecoderGetInfo(dec, &anim_info))
     {
        ERR("Getting WebP Information is Failed");
        *error = EVAS_LOAD_ERROR_GENERIC;
        return EINA_FALSE;
     }

   uint8_t* buf;
   int pre_timestamp = 0;
   int cur_timestamp = 0;
   int index = 1;

   // Set Frame Array
   loader->frames = eina_array_new(anim_info.frame_count);
   if (!loader->frames)
     {
        ERR("Frame Array Allocation is Faild");
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return EINA_FALSE;
     }

   // Decode Frames
   while (WebPAnimDecoderHasMoreFrames(dec))
     {
        if (!WebPAnimDecoderGetNext(dec, &buf, &cur_timestamp))
          {
             ERR("WebP Decoded Frame Get is Failed");
             *error = EVAS_LOAD_ERROR_GENERIC;
             return EINA_FALSE;
          }
        _new_frame(loader, buf, anim_info.canvas_width, anim_info.canvas_height, index,
                   pre_timestamp, cur_timestamp);
        pre_timestamp = cur_timestamp;
        index++;
     }

   // Set Animation Info
   if (anim_info.frame_count > 1)
     {
        animated->animated = 1;
        animated->loop_count = anim_info.loop_count;
        animated->loop_hint = EVAS_IMAGE_ANIMATED_HINT_LOOP;
        animated->frame_count = anim_info.frame_count;
     }

   return EINA_TRUE;
}

static Eina_Bool
evas_image_load_file_data_webp(void *loader_data,
			       Emile_Image_Property *prop,
			       void *pixels,
			       int *error)
{
   Loader_Info *loader = loader_data;
   Evas_Image_Animated *animated = loader->animated;

   *error = EVAS_LOAD_ERROR_NONE;

   void *surface = NULL;
   int width, height;
   int index = 0;

   index = animated->cur_frame;

   // Find Cur Frame
   if (index == 0)
     index = 1;
   Image_Frame *frame = _find_frame(loader, index);
   if (frame == NULL) return EINA_FALSE;

   WebPAnimInfo anim_info;
   WebPAnimDecoderGetInfo(loader->dec, &anim_info);
   width = anim_info.canvas_width;
   height = anim_info.canvas_height;

   // Render Frame
   surface = pixels;
   memcpy(surface, frame->data, width * height * 4);
   prop->premul = EINA_TRUE;

   return EINA_TRUE;
}

static double
evas_image_load_frame_duration_webp(void *loader_data,
                                    int start_frame,
                                    int frame_num)
{
   Loader_Info *loader = loader_data;
   Evas_Image_Animated *animated = loader->animated;

   if (!animated->animated) return -1.0;
   if (frame_num < 0) return -1.0;
   if (start_frame < 1) return -1.0;

   // Calculate Duration of Current Frame
   Image_Frame *frame = _find_frame(loader, start_frame);
   if (frame == NULL) return -1.0;

   return frame->delay;
}

static Evas_Image_Load_Func evas_image_load_webp_func =
{
  EVAS_IMAGE_LOAD_VERSION,
  evas_image_load_file_open_webp,
  evas_image_load_file_close_webp,
  (void*) evas_image_load_file_head_webp,
  NULL,
  (void*) evas_image_load_file_data_webp,
  evas_image_load_frame_duration_webp,
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

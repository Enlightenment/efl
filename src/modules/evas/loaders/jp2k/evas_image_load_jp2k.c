#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include <openjpeg.h>

#include "Evas_Loader.h"

static int _evas_loader_jp2k_log_dom = -1;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_loader_jp2k_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_loader_jp2k_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_loader_jp2k_log_dom, __VA_ARGS__)

typedef struct _Evas_Loader_Internal Evas_Loader_Internal;
struct _Evas_Loader_Internal
{
  Eina_File *f;
  Evas_Image_Load_Opts *opts;
};

static void
_jp2k_error_cb(const char *msg EINA_UNUSED, void *data EINA_UNUSED)
{
//   ERR("OpenJPEG internal error: '%s'.", msg);
}

static void
_jp2k_warning_cb(const char *msg EINA_UNUSED, void *data EINA_UNUSED)
{
//   WRN("OpenJPEG internal warning: '%s'.", msg);
}

static void
_jp2k_info_cb(const char *msg EINA_UNUSED, void *data EINA_UNUSED)
{
//   INF("OpenJPEG internal information: '%s'.", msg);
}

static Eina_Bool
evas_image_load_file_head_jp2k_internal(unsigned int *w, unsigned int *h,
					unsigned char *alpha,
					Evas_Image_Load_Opts *opts EINA_UNUSED,
                                        void *map, size_t length,
                                        int *error)
{
   opj_event_mgr_t event_mgr;
   opj_dparameters_t params;
   opj_dinfo_t *info;
   opj_cio_t *cio;
   opj_image_t *image;
   int format;
   int k;

   if (length < 2)
     {
        *error = EVAS_LOAD_ERROR_GENERIC;
        return EINA_FALSE;
     }

   if (((unsigned char *)map)[0] == 0xFF && ((unsigned char *)map)[1] == 0x4F)
     format = CODEC_J2K;
   else
     format = CODEC_JP2;

   memset(&event_mgr, 0, sizeof(event_mgr));
   event_mgr.error_handler = _jp2k_error_cb;
   event_mgr.warning_handler = _jp2k_warning_cb;
   event_mgr.info_handler = _jp2k_info_cb;

   opj_set_default_decoder_parameters(&params);
   info = opj_create_decompress(format);
   if (!info)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return EINA_FALSE;
     }
   opj_set_event_mgr((opj_common_ptr)info, &event_mgr, NULL);
   opj_setup_decoder(info, &params);

   cio = opj_cio_open((opj_common_ptr)info, map, length);
   if (!cio)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return EINA_FALSE;
     }

   image = opj_decode(info, cio);
   if (!image)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return EINA_FALSE;
     }

   for (k = 1; k < image->numcomps; k++)
     {
        if (image->comps[k].w != image->comps[0].w)
          goto free_image;
        if (image->comps[k].h != image->comps[0].h)
          goto free_image;
        if (image->comps[k].prec > 8)
          goto free_image;
    }

   *w = image->comps[0].w;
   *h = image->comps[0].h;
   *alpha = ((image->numcomps == 4) || (image->numcomps == 2)) ? 1 : 0;
   *error = EVAS_LOAD_ERROR_NONE;

   opj_image_destroy(image);
   opj_cio_close(cio);
   opj_destroy_decompress(info);

   return EINA_TRUE;

 free_image:
   *error = EVAS_LOAD_ERROR_GENERIC;
   opj_image_destroy(image);
   opj_cio_close(cio);
   opj_destroy_decompress(info);

   return EINA_FALSE;
}

static Eina_Bool
evas_image_load_file_data_jp2k_internal(Evas_Image_Load_Opts *opts EINA_UNUSED,
					Evas_Image_Property *prop EINA_UNUSED,
					void *pixels,
                                        void *map, size_t length,
                                        int *error)
{
   opj_dparameters_t params;
   opj_dinfo_t *info;
   opj_cio_t *cio;
   opj_image_t *image;
   unsigned int *iter;
   int format;
   int idx;

   if (((unsigned char *)map)[0] == 0xFF && ((unsigned char *)map)[1] == 0x4F)
     format = CODEC_J2K;
   else
     format = CODEC_JP2;

   opj_set_default_decoder_parameters(&params);
   info = opj_create_decompress(format);
   opj_set_event_mgr((opj_common_ptr)info, NULL, NULL);
   opj_setup_decoder(info, &params);
   cio = opj_cio_open((opj_common_ptr)info, map, length);
   image = opj_decode(info, cio);

   iter = pixels;
   idx = 0;

   /*
    * FIXME:
    * image->numcomps == 4, image->color_space == CLRSPC_SYCC : YUV
    */
   /* BGR(A) */
   if ((image->numcomps >= 3) &&
       (image->comps[0].dx == image->comps[1].dx) &&
       (image->comps[1].dx == image->comps[2].dx) &&
       (image->comps[0].dy == image->comps[1].dy) &&
       (image->comps[1].dy == image->comps[2].dy))
     {
        int a;
        int r;
        int g;
        int b;
        int i;
        int j;

        for (j = 0; j < image->comps[0].h; j++)
          {
             for (i = 0; i < image->comps[0].w; i++, idx++, iter++)
               {
                  r = image->comps[0].data[idx];
                  r+= (image->comps[0].sgnd ? 1 << (image->comps[0].prec - 1) : 0);
                  if (r > 255) r = 255;
                  if (r < 0) r = 0;

                  g = image->comps[1].data[idx];
                  g+= (image->comps[1].sgnd ? 1 << (image->comps[1].prec - 1) : 0);
                  if (g > 255) g = 255;
                  if (g < 0) g = 0;

                  b = image->comps[2].data[idx];
                  b+= (image->comps[2].sgnd ? 1 << (image->comps[2].prec - 1) : 0);
                  if (b > 255) b = 255;
                  if (b < 0) b = 0;

                  if (image->numcomps == 4)
                    {
                       a = image->comps[3].data[idx];
                       a+= (image->comps[3].sgnd ? 1 << (image->comps[3].prec - 1) : 0);
                       if (a > 255) a = 255;
                       if (a < 0) a = 0;
                    }
                  else
                    a = 255;

                  *iter = a << 24 | r << 16 | g << 8 | b;
               }
          }
     }
   /* *GRAY(A) */
   else if (((image->numcomps == 1) || (image->numcomps == 2)) &&
            (image->comps[0].dx == image->comps[1].dx) &&
            (image->comps[1].dx == image->comps[2].dx) &&
            (image->comps[0].dy == image->comps[1].dy) &&
            (image->comps[1].dy == image->comps[2].dy))
     {
        int a;
        int g;
        int i;
        int j;

        for (j = 0; j < image->comps[0].h; j++)
          {
             for (i = 0; i < image->comps[0].w; i++, idx++, iter++)
               {
                  g = image->comps[0].data[idx];
                  g+= (image->comps[0].sgnd ? 1 << (image->comps[0].prec - 1) : 0);
                  if (g > 255) g = 255;
                  if (g < 0) g = 0;

                  if (image->numcomps == 2)
                    {
                       a = image->comps[1].data[idx];
                       a+= (image->comps[1].sgnd ? 1 << (image->comps[1].prec - 1) : 0);
                       if (a > 255) a = 255;
                       if (a < 0) a = 0;
                    }
                  else
                    a = 255;

                  *iter = a << 24 | g << 16 | g << 8 | g;
               }
          }
     }

   opj_image_destroy(image);
   opj_cio_close(cio);
   opj_destroy_decompress(info);

   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
}

static void *
evas_image_load_file_open_jp2k(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
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
evas_image_load_file_close_jp2k(void *loader_data)
{
   free(loader_data);
}

static Eina_Bool
evas_image_load_file_head_jp2k(void *loader_data,
                               Evas_Image_Property *prop,
                               int *error)
{
   Evas_Loader_Internal *loader = loader_data;
   Evas_Image_Load_Opts *opts;
   Eina_File *f;
   void *map;
   Eina_Bool val;

   opts = loader->opts;
   f = loader->f;

   map = eina_file_map_all(f, EINA_FILE_RANDOM);
   if (!map)
     {
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
        return EINA_FALSE;
     }

   val = evas_image_load_file_head_jp2k_internal(&prop->w, &prop->h,
						 &prop->alpha,
						 opts,
                                                 map, eina_file_size_get(f),
                                                 error);

   eina_file_map_free(f, map);

   return val;
}

static Eina_Bool
evas_image_load_file_data_jp2k(void *loader_data,
                               Evas_Image_Property *prop,
			       void *pixels,
			       int *error)
{
   Evas_Loader_Internal *loader = loader_data;
   Evas_Image_Load_Opts *opts;
   Eina_File *f;
   void *map;
   Eina_Bool val = EINA_FALSE;

   f = loader->f;
   opts = loader->opts;

   map = eina_file_map_all(f, EINA_FILE_WILLNEED);
   if (!map)
     {
        *error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
        goto on_error;
     }

   val = evas_image_load_file_data_jp2k_internal(opts, prop, pixels,
                                                 map, eina_file_size_get(f),
                                                 error);

   eina_file_map_free(f, map);

 on_error:
   return val;
}

static Evas_Image_Load_Func evas_image_load_jp2k_func =
{
  evas_image_load_file_open_jp2k,
  evas_image_load_file_close_jp2k,
  evas_image_load_file_head_jp2k,
  evas_image_load_file_data_jp2k,
  NULL,
  EINA_TRUE,
  EINA_TRUE
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;

   _evas_loader_jp2k_log_dom = eina_log_domain_register("evas-jp2k", EINA_COLOR_BLUE);
   if (_evas_loader_jp2k_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }

   em->functions = (void *)(&evas_image_load_jp2k_func);

   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   eina_log_domain_unregister(_evas_loader_jp2k_log_dom);
   _evas_loader_jp2k_log_dom = -1;
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "jp2k",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, jp2k);

#ifndef EVAS_STATIC_BUILD_JP2K
EVAS_EINA_MODULE_DEFINE(image_loader, jp2k);
#endif


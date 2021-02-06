#define _XOPEN_SOURCE 600

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#include <libheif/heif.h>

#include "evas_common_private.h"
#include "evas_private.h"

static int _evas_loader_heif_log_dom = -1;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_loader_heif_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_loader_heif_log_dom, __VA_ARGS__)


static void *
evas_image_load_file_open_heif(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
			      Evas_Image_Load_Opts *opts EINA_UNUSED,
			      Evas_Image_Animated *animated EINA_UNUSED,
			      int *error EINA_UNUSED)
{
   return f;
}

static void
evas_image_load_file_close_heif(void *loader_data EINA_UNUSED)
{
}

static Eina_Bool
evas_image_load_file_head_heif(void *loader_data,
			      Emile_Image_Property *prop,
			      int *error)
{
   Eina_File *f = loader_data;
   void *map;
   size_t length;
   struct heif_error err;
   struct heif_context* hc = NULL;
   struct heif_image_handle* hdl = NULL;
   struct heif_image* img = NULL;
   Eina_Bool r = EINA_FALSE;

   *error = EVAS_LOAD_ERROR_NONE;

   map = eina_file_map_all(f, EINA_FILE_RANDOM);
   length = eina_file_size_get(f);

   // init prop struct with some default null values
   prop->w = 0;
   prop->h = 0;

   if (!map || length < 1)
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	goto on_error;
     }

   hc = heif_context_alloc();
   if (!hc) {
     INF("cannot allocate heif_context");
     *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
     goto on_error;
   }

   err = heif_context_read_from_memory_without_copy(hc, map, length, NULL);
   if (err.code != heif_error_Ok) {
     INF("%s", err.message);
     *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
     goto on_error;
   }

   err = heif_context_get_primary_image_handle(hc, &hdl);
   if (err.code != heif_error_Ok) {
     INF("%s", err.message);
     *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
     goto on_error;
   }

   int has_alpha = heif_image_handle_has_alpha_channel(hdl);

   err = heif_decode_image(hdl, &img, heif_colorspace_RGB,
                           has_alpha ? heif_chroma_interleaved_RGBA : heif_chroma_interleaved_RGB,
                           NULL);
   if (err.code != heif_error_Ok) {
     INF("%s", err.message);
     *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
     goto on_error;
   }

   prop->w = heif_image_get_width(img, heif_channel_interleaved);
   prop->h = heif_image_get_height(img, heif_channel_interleaved);
   if (has_alpha != 3)
     prop->alpha = 1;

   r = EINA_TRUE;

 on_error:
   if (img) {
     heif_image_release(img);
   }

   if (hdl) {
     heif_image_handle_release(hdl);
   }

   if (hc) {
     heif_context_free(hc);
   }
   eina_file_map_free(f, map);
   return r;
}

static Eina_Bool
evas_image_load_file_data_heif(void *loader_data,
			      Emile_Image_Property *prop,
			      void *pixels,
			      int *error)
{
   Eina_File *f = loader_data;

   void *map;
   size_t length;
   struct heif_error err;
   struct heif_context* hc = NULL;
   struct heif_image_handle* hdl = NULL;
   struct heif_image* img = NULL;
   unsigned int x, y;
   int stride, bps = 3;
   const uint8_t* data;
   uint8_t* dd = (uint8_t*)pixels, *ds = NULL;
   Eina_Bool result = EINA_FALSE;

   map = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   length = eina_file_size_get(f);
   *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
   if (!map || length < 1)
     goto on_error;

   *error = EVAS_LOAD_ERROR_GENERIC;
   result = EINA_FALSE;

   hc = heif_context_alloc();
   if (!hc) {
     INF("cannot allocate heif_context");
     *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
     goto on_error;
   }

   err = heif_context_read_from_memory_without_copy(hc, map, length, NULL);
   if (err.code != heif_error_Ok) {
     INF("%s", err.message);
     *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
     goto on_error;
   }

   err = heif_context_get_primary_image_handle(hc, &hdl);
   if (err.code != heif_error_Ok) {
     INF("%s", err.message);
     *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
     goto on_error;
   }

   err = heif_decode_image(hdl, &img, heif_colorspace_RGB,
                           prop->alpha ? heif_chroma_interleaved_RGBA : heif_chroma_interleaved_RGB,
                           NULL);
   if (err.code != heif_error_Ok) {
     INF("%s", err.message);
     *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
     goto on_error;
   }
   if (prop->alpha) bps = 4;
   data = heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride);
   ds = (uint8_t*)data;
   for (y = 0; y < prop->h; y++)
      for (x = 0; x < prop->w; x++)
        {
           if (bps == 3)
             {
               dd[3] = 0xff;
               dd[0] = ds[2];
               dd[1] = ds[1];
               dd[2] = ds[0];
               ds+=3;
               dd+=4;
             }
           else
             {
               dd[0] = ds[2];
               dd[1] = ds[1];
               dd[2] = ds[0];
               dd[3] = ds[3];
               ds+=4;
               dd+=4;
             }
        }
  result = EINA_TRUE;

  *error = EVAS_LOAD_ERROR_NONE;
  prop->premul = EINA_TRUE;

on_error:

  if (map) eina_file_map_free(f, map);

  if (img) {
    // Do not free the image here when we pass it to gdk-pixbuf, as its memory will still be used by gdk-pixbuf.
    heif_image_release(img);
  }

  if (hdl) {
    heif_image_handle_release(hdl);
  }

  if (hc) {
    heif_context_free(hc);
  }

  return result;
}

static const Evas_Image_Load_Func evas_image_load_heif_func = {
  EVAS_IMAGE_LOAD_VERSION,
  evas_image_load_file_open_heif,
  evas_image_load_file_close_heif,
  (void*) evas_image_load_file_head_heif,
  NULL,
  (void*) evas_image_load_file_data_heif,
  NULL,
  EINA_TRUE,
  EINA_FALSE
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   _evas_loader_heif_log_dom = eina_log_domain_register
     ("evas-heif", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_loader_heif_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }

   em->functions = (void *)(&evas_image_load_heif_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   if (_evas_loader_heif_log_dom >= 0)
     {
        eina_log_domain_unregister(_evas_loader_heif_log_dom);
        _evas_loader_heif_log_dom = -1;
     }
}

static Evas_Module_Api evas_modapi =
  {
    EVAS_MODULE_API_VERSION,
    "heif",
    "none",
    {
      module_open,
      module_close
    }
  };

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, heif);


#ifndef EVAS_STATIC_BUILD_HEIF
EVAS_EINA_MODULE_DEFINE(image_loader, heif);
#endif

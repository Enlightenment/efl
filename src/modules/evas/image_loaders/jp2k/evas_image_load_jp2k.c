#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

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

#define J2K_CODESTREAM_MAGIC "\xff\x4f\xff\x51"
#define JP2_MAGIC "\x0d\x0a\x87\x0a"
#define JP2_RFC3745_MAGIC "\x00\x00\x00\x0c\x6a\x50\x20\x20\x0d\x0a\x87\x0a"

typedef struct
{
   unsigned char *base;
   size_t length;
   size_t idx;
} Map_St;

typedef struct _Evas_Loader_Internal Evas_Loader_Internal;
struct _Evas_Loader_Internal
{
   Eina_File *f;
   Evas_Image_Load_Opts *opts;
};

static void
_jp2k_quiet_callback(const char *msg, void *client_data)
{
   (void)msg;
   (void)client_data;
}

static OPJ_SIZE_T
_jp2k_read_fn(void *buf, OPJ_SIZE_T size, void *data)
{
   Map_St *map = data;
   OPJ_SIZE_T offset;

   offset = map->length - map->idx;
   if (offset == 0)
     return (OPJ_SIZE_T)-1;
   if (offset > size)
     offset = size;
   memcpy(buf, map->base + map->idx, offset);
   map->idx += offset;

   return offset;
}

static OPJ_OFF_T
_jp2k_seek_cur_fn(OPJ_OFF_T size, void *data)
{
   Map_St *map = data;

   if (size > (OPJ_OFF_T)(map->length - map->idx))
     size = (OPJ_OFF_T)(map->length - map->idx);

   map->idx += size;

   return map->idx;
}

static OPJ_BOOL
_jp2k_seek_set_fn(OPJ_OFF_T size, void *data)
{
   Map_St *map = data;

   if (size > (OPJ_OFF_T)map->length)
     return OPJ_FALSE;

   map->idx = size;

   return OPJ_TRUE;
}

static Eina_Bool
evas_image_load_file_head_jp2k_internal(unsigned int *w, unsigned int *h,
					unsigned char *alpha,
                                        void *map, size_t length,
                                        int *error)
{
   Map_St map_st;
   opj_dparameters_t core;
   opj_codec_t *codec;
   opj_stream_t *st;
   opj_image_t* image;
   OPJ_CODEC_FORMAT cfmt;

   map_st.base = map;
   map_st.length = length;
   map_st.idx = 0;

   /* default parameters */
   memset(&core, 0, sizeof(opj_dparameters_t));
   opj_set_default_decoder_parameters(&core);

   /* magic check */
   cfmt = OPJ_CODEC_UNKNOWN;
   if (map_st.length >= 4)
     {
        if (memcmp(map_st.base, J2K_CODESTREAM_MAGIC, 4) == 0)
          cfmt = OPJ_CODEC_J2K;
        else if ((memcmp(map_st.base, JP2_MAGIC, 4) == 0) ||
                 ((map_st.length >= 12) && (memcmp(map_st.base, JP2_RFC3745_MAGIC, 12) == 0)))
          cfmt = OPJ_CODEC_JP2;
     }

   if (cfmt == OPJ_CODEC_UNKNOWN)
     {
        *error = EVAS_LOAD_ERROR_GENERIC;
        return EINA_FALSE;
     }

   /* codec */
   codec = opj_create_decompress(cfmt);
   if (!codec)
     {
        ERR("can't create codec");
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return EINA_FALSE;
     }
   opj_set_info_handler(codec, _jp2k_quiet_callback, NULL);
   opj_set_warning_handler(codec, _jp2k_quiet_callback, NULL);
   opj_set_error_handler(codec, _jp2k_quiet_callback, NULL);
   if (!opj_setup_decoder(codec, &core))
     {
        ERR("can't setup decoder");
        opj_destroy_codec(codec);
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return EINA_FALSE;
     }
   //opj_codec_set_threads(codec, 0)

   /* stream */
   st = opj_stream_create(OPJ_J2K_STREAM_CHUNK_SIZE, OPJ_TRUE);
   if (!st)
     {
        ERR("can't create stream");
        opj_destroy_codec(codec);
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return EINA_FALSE;
     }

   opj_stream_set_user_data(st, &map_st, NULL);
   opj_stream_set_user_data_length(st, map_st.length);
   opj_stream_set_read_function(st, _jp2k_read_fn);
   opj_stream_set_skip_function(st, _jp2k_seek_cur_fn);
   opj_stream_set_seek_function(st, _jp2k_seek_set_fn);

   opj_read_header(st, codec, &image);
   *w = image->x1 - image->x0;
   *h = image->y1 - image->y0;
   *alpha = ((image->numcomps == 4) || (image->numcomps == 2)) ? 1 : 0;
   *error = EVAS_LOAD_ERROR_NONE;

   opj_image_destroy(image);
   opj_stream_destroy(st);
   opj_destroy_codec(codec);

   return EINA_TRUE;
}

static Eina_Bool
evas_image_load_file_data_jp2k_internal(void *pixels,
                                        void *map, size_t length,
                                        int *error)
{
   Map_St map_st;
   opj_dparameters_t core;
   opj_codec_t *codec;
   opj_stream_t *st;
   opj_image_t* image;
   unsigned int *iter;
   OPJ_CODEC_FORMAT cfmt;
   int idx;

   map_st.base = map;
   map_st.length = length;
   map_st.idx = 0;

   /* default parameters */
   memset(&core, 0, sizeof(opj_dparameters_t));
   opj_set_default_decoder_parameters(&core);
   core.flags |= OPJ_DPARAMETERS_IGNORE_PCLR_CMAP_CDEF_FLAG;

   /* magic check */
   cfmt = OPJ_CODEC_UNKNOWN;
   if (map_st.length >= 4)
     {
        if (memcmp(map_st.base, J2K_CODESTREAM_MAGIC, 4) == 0)
          cfmt = OPJ_CODEC_J2K;
        else if ((memcmp(map_st.base, JP2_MAGIC, 4) == 0) ||
                 ((map_st.length >= 12) && (memcmp(map_st.base, JP2_RFC3745_MAGIC, 12) == 0)))
          cfmt = OPJ_CODEC_JP2;
     }

   if (cfmt == OPJ_CODEC_UNKNOWN)
     {
        ERR("jpeg200 file format invalid\n");
        *error = EVAS_LOAD_ERROR_GENERIC;
        return EINA_FALSE;
     }

   /* codec */
   codec = opj_create_decompress(cfmt);
   if (!codec)
     {
        ERR("can't create codec\n");
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return EINA_FALSE;
     }
   opj_set_info_handler(codec, _jp2k_quiet_callback, NULL);
   opj_set_warning_handler(codec, _jp2k_quiet_callback, NULL);
   opj_set_error_handler(codec, _jp2k_quiet_callback, NULL);
   if (!opj_setup_decoder(codec, &core))
     {
        ERR("can't setup decoder\n");
        opj_destroy_codec(codec);
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return EINA_FALSE;
     }
   //opj_codec_set_threads(codec, 0)

   /* stream */
   st = opj_stream_create(OPJ_J2K_STREAM_CHUNK_SIZE, OPJ_TRUE);
   if (!st)
     {
        ERR("can't create stream\n");
        opj_destroy_codec(codec);
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return EINA_FALSE;
     }

   opj_stream_set_user_data(st, &map_st, NULL);
   opj_stream_set_user_data_length(st, map_st.length);
   opj_stream_set_read_function(st, _jp2k_read_fn);
   opj_stream_set_skip_function(st, _jp2k_seek_cur_fn);
   opj_stream_set_seek_function(st, _jp2k_seek_set_fn);

   if (!opj_read_header(st, codec, &image))
     {
        ERR("can not read image header\n");
        opj_stream_destroy(st);
        opj_destroy_codec(codec);
        *error = EVAS_LOAD_ERROR_GENERIC;
        return EINA_FALSE;
     }

   if (!(opj_decode(codec, st, image) && opj_end_decompress(codec, st)))
     {
        ERR("can not decode image\n");
        opj_image_destroy(image);
        opj_stream_destroy(st);
        opj_destroy_codec(codec);
        *error = EVAS_LOAD_ERROR_GENERIC;
        return EINA_FALSE;
     }

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
        unsigned int i;
        unsigned int j;

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
        unsigned int i;
        unsigned int j;

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
   opj_stream_destroy(st);
   opj_destroy_codec(codec);

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

   val = evas_image_load_file_head_jp2k_internal(&prop->w, &prop->h,
						 &prop->alpha,
                                                 map, eina_file_size_get(f),
                                                 error);

   eina_file_map_free(f, map);

   return val;
}

static Eina_Bool
evas_image_load_file_data_jp2k(void *loader_data,
                               Evas_Image_Property *prop EINA_UNUSED,
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

   val = evas_image_load_file_data_jp2k_internal(pixels,
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
   if (_evas_loader_jp2k_log_dom >= 0)
     {
        eina_log_domain_unregister(_evas_loader_jp2k_log_dom);
        _evas_loader_jp2k_log_dom = -1;
     }
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


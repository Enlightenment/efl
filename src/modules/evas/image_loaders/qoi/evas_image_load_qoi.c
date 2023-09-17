#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Evas_Loader.h"
#include "evas_common_private.h"

/*
 * code based on original qoi.h code (MIT license):
 * https://github.com/phoboslab/qoi/blob/master/qoi.h
 * date: 2023 march the 14th
 */

#define QOI_ZEROARR(a) memset((a),0,sizeof(a))

#define QOI_OP_INDEX  0x00 /* 00xxxxxx */
#define QOI_OP_DIFF   0x40 /* 01xxxxxx */
#define QOI_OP_LUMA   0x80 /* 10xxxxxx */
#define QOI_OP_RUN    0xc0 /* 11xxxxxx */
#define QOI_OP_RGB    0xfe /* 11111110 */
#define QOI_OP_RGBA   0xff /* 11111111 */

#define QOI_MASK_2    0xc0 /* 11000000 */

#define QOI_COLOR_HASH(C) (C.rgba.r*3 + C.rgba.g*5 + C.rgba.b*7 + C.rgba.a*11)

#define QOI_MAGIC \
	(((unsigned int)'q') << 24 | ((unsigned int)'o') << 16 | \
	 ((unsigned int)'i') <<  8 | ((unsigned int)'f'))

#define QOI_HEADER_SIZE 14

#define QOI_PIXELS_MAX ((unsigned int)400000000)

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_loader_qoi_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_loader_qoi_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_loader_qoi_log_dom, __VA_ARGS__)

typedef struct _Evas_Loader_Internal Evas_Loader_Internal;
struct _Evas_Loader_Internal
{
   Eina_File *f;
   Evas_Image_Load_Opts *opts;
   Evas_Image_Animated *animated;
};

typedef union {
	struct { unsigned char r, g, b, a; } rgba;
	unsigned int v;
} qoi_rgba_t;

static int _evas_loader_qoi_log_dom = -1;

static const unsigned char qoi_padding[8] = {0,0,0,0,0,0,0,1};

static unsigned int read_32(const unsigned char *data, int *p)
{
   unsigned int a = data[(*p)++];
   unsigned int b = data[(*p)++];
   unsigned int c = data[(*p)++];
   unsigned int d = data[(*p)++];
   return a << 24 | b << 16 | c << 8 | d;
}

static Eina_Bool
evas_image_load_file_head_qoi_internal(Evas_Loader_Internal *loader EINA_UNUSED,
                                       Emile_Image_Property *prop,
                                       void *map, size_t length,
                                       int *error)
{
   const unsigned char *bytes;
   unsigned int magic;
   unsigned char channels;
   unsigned char colorspace;
   int p = 0;
   Eina_Bool ret;

   ret = EINA_FALSE;
   prop->w = 0;
   prop->h = 0;
   prop->alpha = EINA_FALSE;

   if (length < QOI_HEADER_SIZE + sizeof(qoi_padding))
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        return ret;
     }

   bytes = (const unsigned char *)map;

   magic = read_32(bytes, &p);
   if (magic != QOI_MAGIC)
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        return ret;
     }

   prop->w = read_32(bytes, &p);
   prop->h = read_32(bytes, &p);
   if ((prop->w < 1) ||
       (prop->h < 1) ||
       (prop->h >= QOI_PIXELS_MAX / prop->w) ||
       (prop->w > IMG_MAX_SIZE) ||
       (prop->h > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(prop->w, prop->h))
     {
        *error= EVAS_LOAD_ERROR_GENERIC;
        return ret;
     }

   channels = bytes[p++];
   colorspace = bytes[p++];

   if ((channels < 3) ||
       (channels > 4) ||
       (colorspace > 1))
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        return ret;
     }

   prop->alpha = channels == 4;

   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
}

static Eina_Bool
evas_image_load_file_data_qoi_internal(Evas_Loader_Internal *loader EINA_UNUSED,
                                       Emile_Image_Property *prop,
                                       void *pixels,
                                       void *map, size_t length,
                                       int *error)
{
   qoi_rgba_t index[64];
   const unsigned char *bytes;
   qoi_rgba_t px;
   unsigned int *iter;
   unsigned int magic;
   unsigned char channels;
   unsigned char colorspace;
   int p = 0;
   int run = 0;
   int chunks_len;
   size_t px_len;
   size_t px_pos;
   Eina_Bool ret;

   ret = EINA_FALSE;
   prop->w = 0;
   prop->h = 0;
   prop->alpha = EINA_FALSE;

   if (length < QOI_HEADER_SIZE + sizeof(qoi_padding))
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        return ret;
     }

   bytes = (const unsigned char *)map;

   magic = read_32(bytes, &p);
   if (magic != QOI_MAGIC)
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        return ret;
     }

   prop->w = read_32(bytes, &p);
   prop->h = read_32(bytes, &p);
   if ((prop->w < 1) ||
       (prop->h < 1) ||
       (prop->h >= QOI_PIXELS_MAX / prop->w) ||
       (prop->w > IMG_MAX_SIZE) ||
       (prop->h > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(prop->w, prop->h))
     {
        *error= EVAS_LOAD_ERROR_GENERIC;
        return ret;
     }

   channels = bytes[p++];
   colorspace = bytes[p++];

   if ((channels < 3) ||
       (channels > 4) ||
       (colorspace > 1))
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        return ret;
     }

   prop->alpha = channels == 4;

   px_len = prop->w * prop->h * channels;

   QOI_ZEROARR(index);
   px.rgba.r = 0;
   px.rgba.g = 0;
   px.rgba.b = 0;
   px.rgba.a = 255;

   iter = pixels;
   chunks_len = length - (int)sizeof(qoi_padding);
   for (px_pos = 0; px_pos < px_len; px_pos += channels, iter++)
     {
        if (run > 0)
          {
             run--;
          }
        else if (p < chunks_len)
          {
             int b1 = bytes[p++];

             if (b1 == QOI_OP_RGB)
               {
                  px.rgba.r = bytes[p++];
                  px.rgba.g = bytes[p++];
                  px.rgba.b = bytes[p++];
               }
             else if (b1 == QOI_OP_RGBA)
               {
                  px.rgba.r = bytes[p++];
                  px.rgba.g = bytes[p++];
                  px.rgba.b = bytes[p++];
                  px.rgba.a = bytes[p++];
               }
             else if ((b1 & QOI_MASK_2) == QOI_OP_INDEX)
               {
                  px = index[b1];
               }
             else if ((b1 & QOI_MASK_2) == QOI_OP_DIFF)
               {
                  px.rgba.r += ((b1 >> 4) & 0x03) - 2;
                  px.rgba.g += ((b1 >> 2) & 0x03) - 2;
                  px.rgba.b += ( b1       & 0x03) - 2;
               }
             else if ((b1 & QOI_MASK_2) == QOI_OP_LUMA)
               {
                  int b2 = bytes[p++];
                  int vg = (b1 & 0x3f) - 32;
                  px.rgba.r += vg - 8 + ((b2 >> 4) & 0x0f);
                  px.rgba.g += vg;
                  px.rgba.b += vg - 8 +  (b2       & 0x0f);
               }
             else if ((b1 & QOI_MASK_2) == QOI_OP_RUN)
               {
                  run = (b1 & 0x3f);
               }

             index[QOI_COLOR_HASH(px) % 64] = px;
          }

        if (prop->alpha)
          *iter = (px.rgba.a << 24) |
                  (((px.rgba.r * px.rgba.a) / 255) << 16) |
                  (((px.rgba.g * px.rgba.a) / 255) << 8) |
                  (((px.rgba.b * px.rgba.a) / 255));
        else
          *iter = (255 << 24) |
                  (px.rgba.r << 16) |
                  (px.rgba.g << 8) |
                  (px.rgba.b);
     }

   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
}

static void *
evas_image_load_file_open_qoi(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
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
evas_image_load_file_close_qoi(void *loader_data)
{
   free(loader_data);
}

static Eina_Bool
evas_image_load_file_head_qoi(void *loader_data,
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

   val = evas_image_load_file_head_qoi_internal(loader,
                                                (Emile_Image_Property *)prop,
                                                map, eina_file_size_get(f),
                                                error);

   eina_file_map_free(f, map);

   return val;
}

static Eina_Bool
evas_image_load_file_data_qoi(void *loader_data,
                              Evas_Image_Property *prop,
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

   val = evas_image_load_file_data_qoi_internal(loader,
                                                (Emile_Image_Property *)prop,
                                                pixels,
                                                map, eina_file_size_get(f),
                                                error);

   eina_file_map_free(f, map);

 on_error:
   return val;
}

static Evas_Image_Load_Func evas_image_load_qoi_func =
{
   EVAS_IMAGE_LOAD_VERSION,
   evas_image_load_file_open_qoi,
   evas_image_load_file_close_qoi,
   evas_image_load_file_head_qoi,
   NULL,
   evas_image_load_file_data_qoi,
   NULL,
   EINA_TRUE,
   EINA_FALSE
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;

   _evas_loader_qoi_log_dom = eina_log_domain_register("evas-qoi", EINA_COLOR_BLUE);
   if (_evas_loader_qoi_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }

   em->functions = (void *)(&evas_image_load_qoi_func);

   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   if (_evas_loader_qoi_log_dom >= 0)
     {
        eina_log_domain_unregister(_evas_loader_qoi_log_dom);
        _evas_loader_qoi_log_dom = -1;
     }
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "qoi",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, qoi);

#ifndef EVAS_STATIC_BUILD_QOI
EVAS_EINA_MODULE_DEFINE(image_loader, qoi);
#endif

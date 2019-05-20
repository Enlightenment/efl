#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include "evas_common_private.h"
#include "evas_private.h"

static int
read_mb(unsigned int *data, void *map, size_t length, size_t *position)
{
   int ac = 0, ct;
   unsigned char buf;

   for (ct = 0;;)
     {
        if ((ct++) == 5) return -1;
	if (*position > length) return -1;
	buf = ((unsigned char *) map)[(*position)++];
        ac = (ac << 7) | (buf & 0x7f);
        if ((buf & 0x80) == 0) break;
     }
   *data = ac;
   return 0;
}

static void *
evas_image_load_file_open_wbmp(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
			       Evas_Image_Load_Opts *opts EINA_UNUSED,
			       Evas_Image_Animated *animated EINA_UNUSED,
			       int *error EINA_UNUSED)
{
   return f;
}

static void
evas_image_load_file_close_wbmp(void *loader_data EINA_UNUSED)
{
}

static Eina_Bool
evas_image_load_file_head_wbmp(void *loader_data,
			       Evas_Image_Property *prop,
			       int *error)
{
   Eina_File *f = loader_data;
   void *map = NULL;
   size_t position = 0;
   size_t length;
   unsigned int type, w, h;
   Eina_Bool r = EINA_FALSE;

   *error = EVAS_LOAD_ERROR_GENERIC;
   length = eina_file_size_get(f);
   if (length <= 4) goto bail;

   map = eina_file_map_all(f, EINA_FILE_RANDOM);
   if (!map) goto bail;

   if (read_mb(&type, map, length, &position) < 0) goto bail;

   if (type != 0)
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        goto bail;
     }

   position++; /* skipping one byte */
   if (read_mb(&w, map, length, &position) < 0) goto bail;
   if (read_mb(&h, map, length, &position) < 0) goto bail;

   /* Wbmp header identifier is too weak....
      Here checks size validation whether it's acutal wbmp or not. */
   if ((((w + 7) >> 3) * h) + position != length)
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        goto bail;
     }

   if ((w < 1) || (h < 1) || (w > IMG_MAX_SIZE) || (h > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(w, h))
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto bail;
     }

   prop->w = w;
   prop->h = h;

   *error = EVAS_LOAD_ERROR_NONE;
   r = EINA_TRUE;

 bail:
   if (map) eina_file_map_free(f, map);
   return r;
}

static Eina_Bool
evas_image_load_file_data_wbmp(void *loader_data,
			       Evas_Image_Property *prop,
			       void *pixels,
			       int *error)
{
   Eina_File *f = loader_data;
   void *map = NULL;
   size_t position = 0;
   size_t length;
   unsigned int type, w, h;
   unsigned int line_length;
   unsigned char *line = NULL;
   int cur = 0, x, y;
   DATA32 *dst_data;
   Eina_Bool r = EINA_FALSE;

   *error = EVAS_LOAD_ERROR_GENERIC;
   length = eina_file_size_get(f);
   if (length <= 4) goto bail;

   map = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   if (!map) goto bail;

   if (read_mb(&type, map, length, &position) < 0) goto bail;

   if (type != 0)
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        goto bail;
     }

   position++; /* skipping one byte */
   if (read_mb(&w, map, length, &position) < 0) goto bail;
   if (read_mb(&h, map, length, &position) < 0) goto bail;

   /* Wbmp header identifier is too weak....
      Here checks size validation whether it's acutal wbmp or not. */
   if ((((w + 7) >> 3) * h) + position != length)
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        goto bail;
     }

   if ((w < 1) || (h < 1) || (w > IMG_MAX_SIZE) || (h > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(w, h))
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto bail;
     }

   if (prop->w != w || prop->h != h)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto bail;
     }

   dst_data = pixels;

   line_length = (prop->w + 7) >> 3;

   for (y = 0; y < (int)prop->h; y++)
     {
        line = ((unsigned char*) map) + position;
        position += line_length;
        for (x = 0; x < (int)prop->w; x++)
          {
             int idx = x >> 3;
             int offset = 1 << (0x07 - (x & 0x07));
             if (line[idx] & offset) dst_data[cur] = 0xffffffff;
             else dst_data[cur] = 0xff000000;
             cur++;
          }
     }

   *error = EVAS_LOAD_ERROR_NONE;
   r = EINA_TRUE;

 bail:
   if (map) eina_file_map_free(f, map);
   return r;
}

static Evas_Image_Load_Func evas_image_load_wbmp_func =
{
   evas_image_load_file_open_wbmp,
   evas_image_load_file_close_wbmp,
   evas_image_load_file_head_wbmp,
   evas_image_load_file_data_wbmp,
   NULL,
   EINA_TRUE,
   EINA_FALSE
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_wbmp_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "wbmp",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, wbmp);

#ifndef EVAS_STATIC_BUILD_WBMP
EVAS_EINA_MODULE_DEFINE(image_loader, wbmp);
#endif

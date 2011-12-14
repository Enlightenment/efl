#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "evas_common.h"
#include "evas_private.h"

static Eina_Bool evas_image_load_file_head_wbmp(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);
static Eina_Bool evas_image_load_file_data_wbmp(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);

static Evas_Image_Load_Func evas_image_load_wbmp_func =
{
   EINA_TRUE,
   evas_image_load_file_head_wbmp,
   evas_image_load_file_data_wbmp,
   NULL,
   EINA_FALSE
};


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

static Eina_Bool
evas_image_load_file_head_wbmp(Image_Entry *ie, const char *file, const char *key __UNUSED__, int *error)
{
   Eina_File *f;
   void *map = NULL;
   size_t position = 0;
   size_t length;
   unsigned int type, w, h;

   *error = EVAS_LOAD_ERROR_GENERIC;
   f = eina_file_open(file, 0);
   if (!f)
     {
        *error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
        return EINA_FALSE;
     }

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
   if ((w < 1) || (h < 1) || (w > IMG_MAX_SIZE) || (h > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(w, h))
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto bail;
     }

   eina_file_map_free(f, map);
   eina_file_close(f);
   ie->w = w;
   ie->h = h;

   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
bail:
   if (map) eina_file_map_free(f, map);
   eina_file_close(f);
   return EINA_FALSE;
}

static Eina_Bool
evas_image_load_file_data_wbmp(Image_Entry *ie, const char *file, const char *key __UNUSED__, int *error)
{
   Eina_File *f;
   void *map = NULL;
   size_t position = 0;
   size_t length;
   unsigned int type, w, h;
   unsigned int line_length;
   unsigned char *line = NULL;
   int cur = 0, x, y;
   DATA32 *dst_data;

   *error = EVAS_LOAD_ERROR_GENERIC;
   f = eina_file_open(file, EINA_FALSE);
   if (!f)
     {
        *error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
        return EINA_FALSE;
     }

   length = eina_file_size_get(f);
   if (length <= 4) goto bail;

   map = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   if (!map) goto bail;

   if (read_mb(&type, map, length, &position) < 0) goto bail;
   position++; /* skipping one byte */
   if (read_mb(&w, map, length, &position) < 0) goto bail;
   if (read_mb(&h, map, length, &position) < 0) goto bail;

   if (type != 0)
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

   ie->w = w;
   ie->h = h;

   evas_cache_image_surface_alloc(ie, ie->w, ie->h);
   dst_data = evas_cache_image_pixels(ie);
   if (!dst_data)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto bail;
     }

   line_length = (ie->w + 7) >> 3;

   for (y = 0; y < (int)ie->h; y++)
     {
        if (position + line_length > length) goto bail;
        line = ((unsigned char*) map) + position;
        position += line_length;
        for (x = 0; x < (int)ie->w; x++)
          {
             int idx = x >> 3;
             int offset = 1 << (0x07 - (x & 0x07));
             if (line[idx] & offset) dst_data[cur] = 0xffffffff;
             else dst_data[cur] = 0xff000000;
             cur++;
          }
     }
   eina_file_map_free(f, map);
   eina_file_close(f);
   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
bail:
   if (map) eina_file_map_free(f, map);
   eina_file_close(f);
   return EINA_FALSE;
}

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_wbmp_func);
   return 1;
}

static void
module_close(Evas_Module *em __UNUSED__)
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

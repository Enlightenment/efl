#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "evas_macros.h"

#include "evas_cserve2.h"
#include "evas_cserve2_slave.h"

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
evas_image_load_file_head_wbmp(Evas_Img_Load_Params *ilp, const char *file, const char *key __UNUSED__, int *error)
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
   ilp->w = w;
   ilp->h = h;

   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
bail:
   if (map) eina_file_map_free(f, map);
   eina_file_close(f);
   return EINA_FALSE;
}

static Eina_Bool
evas_image_load_file_data_wbmp(Evas_Img_Load_Params *ilp, const char *file, const char *key __UNUSED__, int *error)
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

   ilp->w = w;
   ilp->h = h;

   dst_data = ilp->buffer;
   if (!dst_data)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto bail;
     }

   line_length = (ilp->w + 7) >> 3;

   for (y = 0; y < (int)ilp->h; y++)
     {
        if (position + line_length > length) goto bail;
        line = ((unsigned char*) map) + position;
        position += line_length;
        for (x = 0; x < (int)ilp->w; x++)
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

static Evas_Loader_Module_Api modapi =
{
   EVAS_CSERVE2_MODULE_API_VERSION,
   "wbmp",
   evas_image_load_file_head_wbmp,
   evas_image_load_file_data_wbmp
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

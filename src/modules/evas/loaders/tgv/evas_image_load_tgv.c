#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef _WIN32
# include <winsock2.h>
#endif /* ifdef _WIN32 */

#include "lz4.h"
#include "rg_etc1.h"
#include "Evas_Loader.h"

/**************************************************************
 * The TGV file format is oriented around compression mecanism
 * that hardware are good at decompressing. We do still provide
 * a fully software implementation in case your hardware doesn't
 * handle it. As OpenGL is pretty bad at handling border of
 * texture, we do duplicate the first pixels of every border.
 *
 * This file format is designed to compress/decompress things
 * in block area. Giving opportunity to store really huge file
 * and only decompress/compress them as we need. Note that region
 * only work with software decompression as we don't have a sane
 * way to duplicate border to avoid artifact when scaling texture.
 *
 * The file format is as follow :
 * - char     magic[4]: "TGV1"
 * - uint8_t  block_size (real block size = (4 << bits[0-3], 4 << bits[4-7])
 * - uint8_t  algorithm (0 -> ETC1)
 * - uint8_t  options[2] (1 -> lz4)
 * - uint32_t width
 * - uint32_t height
 * - blocks[]
 *   - 0 length encoded compress size (if length == 64 * block_size => no compression)
 *   - lzma encoded etc1 block
 **************************************************************/

// FIXME: wondering if we should support mipmap
// FIXME: instead of the complete size, maybe just the usefull left over byte + number of block.

typedef struct _Evas_Loader_Internal Evas_Loader_Internal;
struct _Evas_Loader_Internal
{
   Eina_File *f;

   Eina_Rectangle region;

   struct {
      unsigned int width;
      unsigned int height;
   } block;
   struct {
      unsigned int width;
      unsigned int height;
   } size;

   Eina_Bool compress;
};

static const Evas_Colorspace cspaces[2] = {
  EVAS_COLORSPACE_ETC1,
  EVAS_COLORSPACE_ARGB8888
};

static void *
evas_image_load_file_open_tgv(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
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

   if (eina_file_size_get(f) <= 16)
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        return NULL;
     }

   loader->f = eina_file_dup(f);
   if (!loader->f)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return NULL;
     }

   if (opts && (opts->region.w > 0) && (opts->region.h > 0))
     {
        EINA_RECTANGLE_SET(&loader->region,
                           opts->region.x,
                           opts->region.y,
                           opts->region.w,
                           opts->region.h);
     }
   else
     {
        EINA_RECTANGLE_SET(&loader->region,
                           0, 0,
                           -1, -1);
     }

   return loader;
}


static void
evas_image_load_file_close_tgv(void *loader_data)
{
   Evas_Loader_Internal *loader = loader_data;

   eina_file_close(loader->f);
   free(loader);
}

#define OFFSET_BLOCK_SIZE 4
#define OFFSET_ALGORITHN 5
#define OFFSET_OPTIONS 6
#define OFFSET_WIDTH 8
#define OFFSET_HEIGHT 12
#define OFFSET_BLOCKS 16

static Eina_Bool
evas_image_load_file_head_tgv(void *loader_data,
                              Evas_Image_Property *prop,
                              int *error)
{
   Evas_Loader_Internal *loader = loader_data;
   const char *m;

   m = eina_file_map_all(loader->f, EINA_FILE_SEQUENTIAL);
   if (!m)
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        return EINA_FALSE;
     }

   if (strncmp(m, "TGV1", 4) != 0)
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        return EINA_FALSE;
     }

   loader->block.width = 4 << (m[OFFSET_BLOCK_SIZE] & 0x0f);
   loader->block.height = 4 << ((m[OFFSET_BLOCK_SIZE] & 0xf0) >> 4);

   if (m[OFFSET_ALGORITHN] != 0)
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        return EINA_FALSE;
     }

   loader->compress = m[OFFSET_OPTIONS] & 0x1;

   loader->size.width = ntohl(*((unsigned int*) &(m[OFFSET_WIDTH])));
   loader->size.height = ntohl(*((unsigned int*) &(m[OFFSET_HEIGHT])));

   if (loader->region.w == -1 &&
       loader->region.h == -1)
     {
        loader->region.w = loader->size.width;
        loader->region.h = loader->size.height;
        prop->cspaces = cspaces; // ETC1 colorspace doesn't work with region
     }
   else
     {
        Eina_Rectangle r;

        EINA_RECTANGLE_SET(&r, 0, 0, loader->size.width, loader->size.height);
        if (!eina_rectangle_intersection(&loader->region, &r))
          {
             *error = EVAS_LOAD_ERROR_GENERIC;
             return EINA_FALSE;
          }
     }

   prop->w = loader->size.width;
   prop->h = loader->size.height;

   return EINA_TRUE;
}

static inline unsigned int
_tgv_length_get(const char *m, unsigned int length, unsigned int *offset)
{
   unsigned int r = 0;
   unsigned int shift = 0;

   while (*offset < length && ((*m) & 0x80))
     {
        r = r | (((*m) & 0x7F) << shift);
        shift += 7;
        m++;
        (*offset)++;
     }
   if (*offset < length)
     {
        r = r | (((*m) & 0x7F) << shift);
        (*offset)++;
     }

   return r;
}

Eina_Bool
evas_image_load_file_data_tgv(void *loader_data,
                              Evas_Image_Property *prop,
                              void *pixels,
                              int *error)
{
   Evas_Loader_Internal *loader = loader_data;
   const char *m;
   unsigned int *p = pixels;
   unsigned char *p_etc1 = pixels;
   char *buffer;
   Eina_Rectangle master;
   unsigned int block_length;
   unsigned int length, offset;
   unsigned int x, y;
   unsigned int block_count;
   unsigned int etc1_width = 0;
   Eina_Bool r = EINA_FALSE;

   length = eina_file_size_get(loader->f);
   offset = OFFSET_BLOCKS;

   *error = EVAS_LOAD_ERROR_CORRUPT_FILE;

   m = eina_file_map_all(loader->f, EINA_FILE_WILLNEED);
   if (!m) return EINA_FALSE;

   // By definition, prop{.w, .h} == region{.w, .h}
   EINA_RECTANGLE_SET(&master,
                      loader->region.x, loader->region.y,
                      prop->w, prop->h);

   if (prop->cspace == EVAS_COLORSPACE_ETC1)
     {
        if (master.x % 4 ||
            master.y % 4)
          abort();

        etc1_width = ((prop->w + 2) / 4 + ((prop->w + 2) % 4 ? 1 : 0)) * 8;
     }
   else if (prop->cspace == EVAS_COLORSPACE_ARGB8888)
     {
        // Offset to take duplicated pixels into account
        master.x += 1;
        master.y += 1;
     }

   // Allocate space for each ETC1 block (64bytes per 4 * 4 pixels group)
   block_count = loader->block.width * loader->block.height / (4 * 4);
   if (loader->compress)
     buffer = alloca(8 * block_count);
   else
     buffer = NULL;

   for (y = 0; y < loader->size.height + 2; y += loader->block.height)
     for (x = 0; x < loader->size.width + 2; x += loader->block.width)
       {
          Eina_Rectangle current;
          const char *data_start;
          const char *it;
          unsigned int expand_length;
          unsigned int i, j;

          block_length = _tgv_length_get(m + offset, length, &offset);

          if (block_length == 0) goto on_error;

          data_start = m + offset;
          offset += block_length;

          EINA_RECTANGLE_SET(&current, x, y,
                             loader->block.width, loader->block.height);

          if (!eina_rectangle_intersection(&current, &master))
            continue ;

          if (loader->compress)
            {
               expand_length = LZ4_uncompress(data_start,
                                              buffer, block_count * 8);
               // That's an overhead for now, need to be fixed
               if (expand_length != block_length)
                 goto on_error;
            }
          else
            {
               buffer = (void*) data_start;
               if (block_count * 8 != block_length)
                 goto on_error;
            }
          it = buffer;

          for (i = 0; i < loader->block.height; i += 4)
            for (j = 0; j < loader->block.width; j += 4, it += 8)
              {
                 Eina_Rectangle current_etc;
                 unsigned int temporary[4 * 4] = { 0 };
                 unsigned int offset_x, offset_y;
                 int k;

                 EINA_RECTANGLE_SET(&current_etc, x + j, y + i, 4, 4);

                 if (!eina_rectangle_intersection(&current_etc, &current))
                   continue ;

                 switch (prop->cspace)
                   {
                    case EVAS_COLORSPACE_ARGB8888:
                       if (!rg_etc1_unpack_block(it, temporary, 0))
                         {
                            fprintf(stderr, "HOUSTON WE HAVE A PROBLEM ! Block starting at {%i, %i} is corrupted !\n", x + j, y + i);
                            continue ;
                         }

                       offset_x = current_etc.x - x - j;
                       offset_y = current_etc.y - y - i;
                       for (k = 0; k < current_etc.h; k++)
                         {
                            memcpy(&p[current_etc.x - 1 +
                                      (current_etc.y - 1 + k) * master.w],
                                   &temporary[offset_x + (offset_y + k) * 4],
                                   current_etc.w * sizeof (unsigned int));
                         }
                       break;
                    case EVAS_COLORSPACE_ETC1:
                       memcpy(&p_etc1[(current_etc.x / 4) * 8 +
                                      (current_etc.y / 4) * etc1_width],
                              it, 8);
                       break;
                    default:
                       abort();
                   }
              }
       }

   r = EINA_TRUE;
   *error = EVAS_LOAD_ERROR_NONE;

 on_error:
   eina_file_map_free(loader->f, (void*) m);
   return r;
}

Evas_Image_Load_Func evas_image_load_tgv_func =
{
  evas_image_load_file_open_tgv,
  evas_image_load_file_close_tgv,
  evas_image_load_file_head_tgv,
  evas_image_load_file_data_tgv,
  NULL,
  EINA_TRUE,
  EINA_FALSE
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_tgv_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "tgv",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, tgv);

#ifndef EVAS_STATIC_BUILD_TGV
EVAS_EINA_MODULE_DEFINE(image_loader, tgv);
#endif

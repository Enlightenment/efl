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

#ifdef BUILD_NEON
#include <arm_neon.h>
#endif

#ifndef WORDS_BIGENDIAN
/* x86 */
#define A_VAL(p) (((uint8_t *)(p))[3])
#define R_VAL(p) (((uint8_t *)(p))[2])
#define G_VAL(p) (((uint8_t *)(p))[1])
#define B_VAL(p) (((uint8_t *)(p))[0])
#else
/* ppc */
#define A_VAL(p) (((uint8_t *)(p))[0])
#define R_VAL(p) (((uint8_t *)(p))[1])
#define G_VAL(p) (((uint8_t *)(p))[2])
#define B_VAL(p) (((uint8_t *)(p))[3])
#endif

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
 * - uint8_t  algorithm (0 -> ETC1, 1 -> ETC2 RGB, 2 -> ETC2 RGBA, 3 -> ETC1+Alpha)
 * - uint8_t  options[2] (bitmask: 1 -> lz4, 2 for block-less, 4 -> unpremultiplied)
 * - uint32_t width
 * - uint32_t height
 * - blocks[]
 *   - 0 length encoded compress size (if length == 64 * block_size => no compression)
 *   - lzma encoded etc1 block
 *
 * If the format is ETC1+Alpha (algo = 3), then a second image is encoded
 * in ETC1 right after the first one, and it contains grey-scale alpha
 * values.
 **************************************************************/

// FIXME: wondering if we should support mipmap
// TODO: support ETC1+ETC2 images (RGB only)

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

   Evas_Colorspace cspace;

   Eina_Bool compress : 1;
   Eina_Bool blockless : 1; // Special mode used when copying data directly
   Eina_Bool unpremul : 1;
};

static const Evas_Colorspace cspaces_etc1[2] = {
  EVAS_COLORSPACE_ETC1,
  EVAS_COLORSPACE_ARGB8888
};

static const Evas_Colorspace cspaces_rgb8_etc2[2] = {
  EVAS_COLORSPACE_RGB8_ETC2,
  EVAS_COLORSPACE_ARGB8888
};

static const Evas_Colorspace cspaces_rgba8_etc2_eac[2] = {
  EVAS_COLORSPACE_RGBA8_ETC2_EAC,
  EVAS_COLORSPACE_ARGB8888
};

static const Evas_Colorspace cspaces_etc1_alpha[2] = {
  EVAS_COLORSPACE_ETC1_ALPHA,
  EVAS_COLORSPACE_ARGB8888
};

static void *
evas_image_load_file_open_tgv(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
                              Evas_Image_Load_Opts *opts,
                              Evas_Image_Animated *animated EINA_UNUSED,
                              int *error)
{
   Evas_Loader_Internal *loader;

   if (eina_file_size_get(f) <= 16)
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        return NULL;
     }

   loader = calloc(1, sizeof (Evas_Loader_Internal));
   if (!loader)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return NULL;
     }

   loader->f = eina_file_dup(f);
   if (!loader->f)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        free(loader);
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

static int
roundup(int val, int rup)
{
   if (val >= 0 && rup > 0)
     return (val + rup - 1) - ((val + rup - 1) % rup);
   return 0;
}

#define OFFSET_BLOCK_SIZE 4
#define OFFSET_ALGORITHM 5
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
   Eina_Bool ret = EINA_FALSE;
   char *m;

   m = eina_file_map_all(loader->f, EINA_FILE_SEQUENTIAL);
   if (!m)
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        return EINA_FALSE;
     }

   if (strncmp(m, "TGV1", 4) != 0)
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        goto on_error;
     }

   switch (m[OFFSET_ALGORITHM] & 0xFF)
     {
      case 0:
        prop->cspaces = cspaces_etc1;
        loader->cspace = EVAS_COLORSPACE_ETC1;
        prop->alpha = EINA_FALSE;
        break;
      case 1:
        prop->cspaces = cspaces_rgb8_etc2;
        loader->cspace = EVAS_COLORSPACE_RGB8_ETC2;
        prop->alpha = EINA_FALSE;
        break;
      case 2:
        prop->cspaces = cspaces_rgba8_etc2_eac;
        loader->cspace = EVAS_COLORSPACE_RGBA8_ETC2_EAC;
        prop->alpha = EINA_TRUE;
        break;
      case 3:
        prop->cspaces = cspaces_etc1_alpha;
        loader->cspace = EVAS_COLORSPACE_ETC1_ALPHA;
        loader->unpremul = !!(m[OFFSET_OPTIONS] & 0x4);
        prop->alpha = EINA_TRUE;
        prop->premul = loader->unpremul;
        break;
      default:
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        goto on_error;
     }

   loader->compress = m[OFFSET_OPTIONS] & 0x1;
   loader->blockless = (m[OFFSET_OPTIONS] & 0x2) != 0;

   loader->size.width = ntohl(*((unsigned int*) &(m[OFFSET_WIDTH])));
   loader->size.height = ntohl(*((unsigned int*) &(m[OFFSET_HEIGHT])));

   if (loader->blockless)
     {
        loader->block.width = roundup(loader->size.width + 2, 4);
        loader->block.height = roundup(loader->size.height + 2, 4);
     }
   else
     {
        loader->block.width = 4 << (m[OFFSET_BLOCK_SIZE] & 0x0f);
        loader->block.height = 4 << ((m[OFFSET_BLOCK_SIZE] & 0xf0) >> 4);
     }

   if (loader->region.w == -1 && loader->region.h == -1)
     {
        loader->region.w = loader->size.width;
        loader->region.h = loader->size.height;
     }
   else
     {
        Eina_Rectangle r;

        // ETC1 colorspace doesn't work with region
        prop->cspaces = NULL;

        EINA_RECTANGLE_SET(&r, 0, 0, loader->size.width, loader->size.height);
        if (!eina_rectangle_intersection(&loader->region, &r))
          {
             *error = EVAS_LOAD_ERROR_GENERIC;
             goto on_error;
          }
     }

   prop->w = loader->size.width;
   prop->h = loader->size.height;
   prop->borders.l = 1;
   prop->borders.t = 1;
   prop->borders.r = roundup(prop->w + 2, 4) - prop->w - 1;
   prop->borders.b = roundup(prop->h + 2, 4) - prop->h - 1;

   ret = EINA_TRUE;

on_error:
   eina_file_map_free(loader->f, m);
   return ret;
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
   unsigned char *p_etc = pixels;
   char *buffer = NULL;
   Eina_Rectangle master;
   unsigned int block_length;
   unsigned int length, offset;
   unsigned int x, y;
   unsigned int block_count;
   unsigned int etc_width = 0;
   unsigned int etc_block_size;
   Eina_Bool r = EINA_FALSE;
   int num_planes = 1, plane, alpha_offset = 0;

   length = eina_file_size_get(loader->f);
   offset = OFFSET_BLOCKS;

   *error = EVAS_LOAD_ERROR_CORRUPT_FILE;

   m = eina_file_map_all(loader->f, EINA_FILE_WILLNEED);
   if (!m) return EINA_FALSE;

   // By definition, prop{.w, .h} == region{.w, .h}
   EINA_RECTANGLE_SET(&master,
                      loader->region.x, loader->region.y,
                      prop->w, prop->h);

   switch (loader->cspace)
     {
      case EVAS_COLORSPACE_ETC1:
      case EVAS_COLORSPACE_RGB8_ETC2:
        etc_block_size = 8;
        break;
      case EVAS_COLORSPACE_RGBA8_ETC2_EAC:
        etc_block_size = 16;
        break;
      case EVAS_COLORSPACE_ETC1_ALPHA:
        etc_block_size = 8;
        num_planes = 2;
        alpha_offset = ((prop->w + 2 + 3) / 4) * ((prop->h + 2 + 3) / 4) * 8 / sizeof(*p_etc);
        break;
      default: abort();
     }
   etc_width = ((prop->w + 2 + 3) / 4) * etc_block_size;

   switch (prop->cspace)
     {
      case EVAS_COLORSPACE_ETC1:
      case EVAS_COLORSPACE_RGB8_ETC2:
      case EVAS_COLORSPACE_RGBA8_ETC2_EAC:
      case EVAS_COLORSPACE_ETC1_ALPHA:
        if (master.x % 4 || master.y % 4)
          abort();
        break;
      case EVAS_COLORSPACE_ARGB8888:
        // Offset to take duplicated pixels into account
        master.x += 1;
        master.y += 1;
        break;
      default: abort();
     }

   if (prop->cspace != EVAS_COLORSPACE_ARGB8888 && prop->cspace != loader->cspace)
     {
        if (!((prop->cspace == EVAS_COLORSPACE_RGB8_ETC2) &&
              (loader->cspace == EVAS_COLORSPACE_ETC1)))
          goto on_error;
        // else: ETC2 is compatible with ETC1 and is preferred
     }

   // Allocate space for each ETC block (8 or 16 bytes per 4 * 4 pixels group)
   block_count = loader->block.width * loader->block.height / (4 * 4);
   if (loader->compress)
     buffer = alloca(etc_block_size * block_count);

   for (plane = 0; plane < num_planes; plane++)
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
              continue;

            if (loader->compress)
              {
                 expand_length = LZ4_uncompress(data_start, buffer,
                                                block_count * etc_block_size);
                 // That's an overhead for now, need to be fixed
                 if (expand_length != block_length)
                   goto on_error;
              }
            else
              {
                 buffer = (void*) data_start;
                 if (block_count * etc_block_size != block_length)
                   goto on_error;
              }
            it = buffer;

            for (i = 0; i < loader->block.height; i += 4)
              for (j = 0; j < loader->block.width; j += 4, it += etc_block_size)
                {
                   Eina_Rectangle current_etc;
                   unsigned int temporary[4 * 4];
                   unsigned int offset_x, offset_y;
                   int k, l;

                   EINA_RECTANGLE_SET(&current_etc, x + j, y + i, 4, 4);

                   if (!eina_rectangle_intersection(&current_etc, &current))
                     continue;

                   switch (prop->cspace)
                     {
                      case EVAS_COLORSPACE_ARGB8888:
                        switch (loader->cspace)
                          {
                           case EVAS_COLORSPACE_ETC1:
                           case EVAS_COLORSPACE_ETC1_ALPHA:
                             if (!rg_etc1_unpack_block(it, temporary, 0))
                               {
                                  // TODO: Should we decode as RGB8_ETC2?
                                  fprintf(stderr, "ETC1: Block starting at {%i, %i} is corrupted!\n", x + j, y + i);
                                  continue;
                               }
                             break;
                           case EVAS_COLORSPACE_RGB8_ETC2:
                             rg_etc2_rgb8_decode_block((uint8_t *) it, temporary);
                             break;
                           case EVAS_COLORSPACE_RGBA8_ETC2_EAC:
                             rg_etc2_rgba8_decode_block((uint8_t *) it, temporary);
                             break;
                           default: abort();
                          }

                        offset_x = current_etc.x - x - j;
                        offset_y = current_etc.y - y - i;

                        if (!plane)
                          {
#ifdef BUILD_NEON
                             if (evas_common_cpu_has_feature(CPU_FEATURE_NEON))
                               {
                                  uint32_t *dst = &p[current_etc.x - 1 + (current_etc.y - 1) * master.w];
                                  uint32_t *src = &temporary[offset_x + offset_y * 4];
                                  for (k = 0; k < current_etc.h; k++)
                                    {
                                       if (current_etc.w == 4)
                                         vst1q_u32(dst, vld1q_u32(src));
                                       else if (current_etc.w == 3)
                                         {
                                            vst1_u32(dst, vld1_u32(src));
                                            *(dst + 2) = *(src + 2);
                                         }
                                       else if (current_etc.w == 2)
                                         vst1_u32(dst, vld1_u32(src));
                                       else
                                          *dst = *src;
                                       dst += master.w;
                                       src += 4;
                                    }
                               }
                             else
#endif
                             for (k = 0; k < current_etc.h; k++)
                               {
                                  memcpy(&p[current_etc.x - 1 + (current_etc.y - 1 + k) * master.w],
                                         &temporary[offset_x + (offset_y + k) * 4],
                                         current_etc.w * sizeof (unsigned int));
                               }
                          }
                        else
                          {
                             for (k = 0; k < current_etc.h; k++)
                               for (l = 0; l < current_etc.w; l++)
                                 {
                                    unsigned int *rgbdata =
                                      &p[current_etc.x - 1 + (current_etc.y - 1 + k) * master.w + l];
                                    unsigned int *adata =
                                      &temporary[offset_x + (offset_y + k) * 4 + l];
                                    A_VAL(rgbdata) = G_VAL(adata);
                                 }
                          }
                        break;
                      case EVAS_COLORSPACE_ETC1:
                      case EVAS_COLORSPACE_RGB8_ETC2:
                      case EVAS_COLORSPACE_RGBA8_ETC2_EAC:
                        memcpy(&p_etc[(current_etc.x / 4) * etc_block_size +
                                      (current_etc.y / 4) * etc_width],
                               it, etc_block_size);
                        break;
                      case EVAS_COLORSPACE_ETC1_ALPHA:
                        memcpy(&p_etc[(current_etc.x / 4) * etc_block_size +
                                      (current_etc.y / 4) * etc_width +
                                      plane * alpha_offset],
                               it, etc_block_size);
                        break;
                      default:
                        abort();
                     }
                } // bx,by inside blocks
         } // x,y macroblocks

   // TODO: Add support for more unpremultiplied modes (ETC2)
   if (prop->cspace == EVAS_COLORSPACE_ARGB8888)
     prop->premul = loader->unpremul; // call premul if unpremul data

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

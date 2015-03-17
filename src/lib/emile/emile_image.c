#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef _WIN32
# include <winsock2.h>
#endif /* ifdef _WIN32 */

#ifdef ENABLE_LIBLZ4
# include <lz4.h>
#else
# include "lz4.h"
#endif

#include "rg_etc1.h"
#include "Emile.h"

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

#define OFFSET_BLOCK_SIZE 4
#define OFFSET_ALGORITHM 5
#define OFFSET_OPTIONS 6
#define OFFSET_WIDTH 8
#define OFFSET_HEIGHT 12
#define OFFSET_BLOCKS 16

struct _Emile_Image
{
   Emile_Image_Load_Opts opts;

   struct {
      unsigned int width;
      unsigned int height;
   } size, block;

   Eina_Rectangle region;

   union {
      Eina_Binbuf *bin;
      Eina_File *f;
   } source;

   const unsigned char *source_data;

   Eina_Bool (*bind)(Emile_Image *image,
                     Emile_Image_Load_Opts *opts,
                     Emile_Image_Animated *animated,
                     Emile_Image_Load_Error *error);
   Eina_Bool (*head)(Emile_Image *image,
                     Emile_Image_Property *prop,
                     unsigned int property_size,
                     Emile_Image_Load_Error *error);
   Eina_Bool (*data)(Emile_Image *image,
                     Emile_Image_Property *prop,
                     unsigned int property_size,
                     void *pixels,
                     Emile_Image_Load_Error *error);
   void (*close)(Emile_Image *image);

   Emile_Colorspace cspace;

   Eina_Bool bin_source : 1;

   /* TGV option */
   Eina_Bool unpremul : 1;
   Eina_Bool compress : 1;
   Eina_Bool blockless : 1;
   Eina_Bool load_opts : 1;
};

static const unsigned char *
_emile_image_file_source_map(Emile_Image *image, unsigned int *length)
{
   if (!image) return NULL;

   if (image->bin_source)
     {
        if (length) *length = eina_binbuf_length_get(image->source.bin);
        return eina_binbuf_string_get(image->source.bin);
     }

   if (length) *length = eina_file_size_get(image->source.f);
   if (!image->source_data)
     {
        image->source_data = eina_file_map_all(image->source.f,
                                               EINA_FILE_SEQUENTIAL);
     }
   return image->source_data;
}

static void
_emile_image_file_source_unmap(Emile_Image *image)
{
   if (!(image && image->source_data)) return ;
   eina_file_map_free(image->source.f,
                      (void*) image->source_data);
   image->source_data = NULL;
}

static int
roundup(int val, int rup)
{
   if (val >= 0 && rup > 0)
     return (val + rup - 1) - ((val + rup - 1) % rup);
   return 0;
}

/* TGV Handling */

static const Emile_Colorspace cspaces_etc1[2] = {
  EMILE_COLORSPACE_ETC1,
  EMILE_COLORSPACE_ARGB8888
};

static const Emile_Colorspace cspaces_rgb8_etc2[2] = {
  EMILE_COLORSPACE_RGB8_ETC2,
  EMILE_COLORSPACE_ARGB8888
};

static const Emile_Colorspace cspaces_rgba8_etc2_eac[2] = {
  EMILE_COLORSPACE_RGBA8_ETC2_EAC,
  EMILE_COLORSPACE_ARGB8888
};

static const Emile_Colorspace cspaces_etc1_alpha[2] = {
  EMILE_COLORSPACE_ETC1_ALPHA,
  EMILE_COLORSPACE_ARGB8888
};

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

static Eina_Bool
_emile_tgv_bind(Emile_Image *image,
                Emile_Image_Load_Opts *opts,
                Emile_Image_Animated *animated,
                Emile_Image_Load_Error *error)
{
   const unsigned char *m;
   unsigned int length;

   m = _emile_image_file_source_map(image, &length);
   if (!m) return EINA_FALSE;

   /* Fast check for file characteristic (useful when trying to guess
    a file format without extention). */
   if (length < 16 ||
       strncmp((const char*) m, "TGV1", 4) != 0)
     goto on_error;

   if ((m[OFFSET_ALGORITHM] & 0xFF) > 3)
     goto on_error;

   return EINA_TRUE;

 on_error:
   *error = EMILE_IMAGE_LOAD_ERROR_UNKNOWN_FORMAT;
   _emile_image_file_source_unmap(image);
   return EINA_FALSE;
}

static Eina_Bool
_emile_tgv_head(Emile_Image *image,
                Emile_Image_Property *prop,
                unsigned int property_size,
                Emile_Image_Load_Error *error)
{
   const unsigned char *m;
   unsigned int length;

   m = _emile_image_file_source_map(image, &length);
   if (!m) return EINA_FALSE;

   switch (m[OFFSET_ALGORITHM] & 0xFF)
     {
      case 0:
         prop->cspaces = cspaces_etc1;
         prop->alpha = EINA_FALSE;
         image->cspace = EMILE_COLORSPACE_ETC1;
         break;
      case 1:
         prop->cspaces = cspaces_rgb8_etc2;
         prop->alpha = EINA_FALSE;
         image->cspace = EMILE_COLORSPACE_RGB8_ETC2;
         break;
      case 2:
         prop->cspaces = cspaces_rgba8_etc2_eac;
         prop->alpha = EINA_TRUE;
         image->cspace = EMILE_COLORSPACE_RGBA8_ETC2_EAC;
         break;
      case 3:
         prop->cspaces = cspaces_etc1_alpha;
         prop->alpha = EINA_TRUE;
         prop->premul = !!(m[OFFSET_OPTIONS] & 0x4);
         image->unpremul = prop->premul;
         break;
      default:
         *error = EMILE_IMAGE_LOAD_ERROR_CORRUPT_FILE;
         return EINA_FALSE;
     }

   image->compress = m[OFFSET_OPTIONS] & 0x1;
   image->blockless = (m[OFFSET_OPTIONS] & 0x2) != 0;

   image->size.width = ntohl(*((unsigned int*) &(m[OFFSET_WIDTH])));
   image->size.height = ntohl(*((unsigned int*) &(m[OFFSET_HEIGHT])));

   if (image->blockless)
     {
        image->block.width = roundup(image->size.width + 2, 4);
        image->block.height = roundup(image->size.height + 2, 4);
     }
   else
     {
        image->block.width = 4 << (m[OFFSET_BLOCK_SIZE] & 0x0f);
        image->block.height = 4 << ((m[OFFSET_BLOCK_SIZE] & 0xf0) >> 4);
     }

   EINA_RECTANGLE_SET(&image->region, 0, 0,
                      image->size.width, image->size.height);
   if (image->load_opts &&
       (image->opts.region.w > 0 && image->opts.region.h > 0))
     {
        // ETC colorspace doesn't work with region for now
        prop->cspaces = NULL;

        if (!eina_rectangle_intersection(&image->region,
                                         &image->opts.region))
          {
             *error = EMILE_IMAGE_LOAD_ERROR_GENERIC;
             return EINA_FALSE;
          }
     }

   prop->w = image->size.width;
   prop->h = image->size.height;
   prop->borders.l = 1;
   prop->borders.t = 1;
   prop->borders.r = roundup(prop->w + 2, 4) - prop->w - 1;
   prop->borders.b = roundup(prop->h + 2, 4) - prop->h - 1;

   return EINA_TRUE;
}

static inline unsigned int
_tgv_length_get(const unsigned char *m,
                unsigned int length,
                unsigned int *offset)
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

static Eina_Bool
_emile_tgv_data(Emile_Image *image,
                Emile_Image_Property *prop,
                unsigned int property_size,
                void *pixels,
                Emile_Image_Load_Error *error)
{
   const unsigned char *m;
   unsigned int *p = pixels;
   unsigned char *p_etc = pixels;
   Eina_Binbuf *buffer = NULL;
   Eina_Rectangle master;
   unsigned int block_length;
   unsigned int length, offset;
   unsigned int x, y;
   unsigned int block_count;
   unsigned int etc_width = 0;
   unsigned int etc_block_size;
   int num_planes = 1, plane, alpha_offset = 0;
   Eina_Bool r = EINA_FALSE;

   m = _emile_image_file_source_map(image, &length);
   if (!m)
     {
        *error = EMILE_IMAGE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return EINA_FALSE;
     }

   offset = OFFSET_BLOCKS;

   *error = EMILE_IMAGE_LOAD_ERROR_CORRUPT_FILE;

   // By definition, prop{.w, .h} == region{.w, .h}
   EINA_RECTANGLE_SET(&master,
                      image->region.x, image->region.y,
                      prop->w, prop->h);

   switch (image->cspace)
     {
      case EMILE_COLORSPACE_ETC1:
      case EMILE_COLORSPACE_RGB8_ETC2:
        etc_block_size = 8;
        break;
      case EMILE_COLORSPACE_RGBA8_ETC2_EAC:
        etc_block_size = 16;
        break;
      case EMILE_COLORSPACE_ETC1_ALPHA:
        etc_block_size = 8;
        num_planes = 2;
        alpha_offset = ((prop->w + 2 + 3) / 4) * ((prop->h + 2 + 3) / 4) * 8 / sizeof(*p_etc);
        break;
      default: abort();
     }
   etc_width = ((prop->w + 2 + 3) / 4) * etc_block_size;

   switch (prop->cspace)
     {
      case EMILE_COLORSPACE_ETC1:
      case EMILE_COLORSPACE_RGB8_ETC2:
      case EMILE_COLORSPACE_RGBA8_ETC2_EAC:
      case EMILE_COLORSPACE_ETC1_ALPHA:
        if (master.x % 4 || master.y % 4)
          // FIXME: Should we really abort here ? Seems like a late check for me
          abort();
        break;
      case EMILE_COLORSPACE_ARGB8888:
        // Offset to take duplicated pixels into account
        master.x += 1;
        master.y += 1;
        break;
      default: abort();
     }


   if (prop->cspace != EMILE_COLORSPACE_ARGB8888 && prop->cspace != image->cspace)
     {
        if (!((prop->cspace == EMILE_COLORSPACE_RGB8_ETC2) &&
              (image->cspace == EMILE_COLORSPACE_ETC1)))
          {
             *error = EMILE_IMAGE_LOAD_ERROR_GENERIC;
             return EINA_FALSE;
          }
        // else: ETC2 is compatible with ETC1 and is preferred
     }

   // Allocate space for each ETC block (8 or 16 bytes per 4 * 4 pixels group)
   block_count = image->block.width * image->block.height / (4 * 4);
   if (image->compress)
     buffer = eina_binbuf_manage_read_only_new_length(alloca(etc_block_size * block_count), etc_block_size * block_count);

   for (plane = 0; plane < num_planes; plane++)
     for (y = 0; y < image->size.height + 2; y += image->block.height)
       for (x = 0; x < image->size.width + 2; x += image->block.width)
         {
            Eina_Rectangle current;
            Eina_Binbuf *data_start;
            const unsigned char *it;
            unsigned int expand_length;
            unsigned int i, j;

            block_length = _tgv_length_get(m + offset, length, &offset);

            if (block_length == 0)
              {
                 *error = EMILE_IMAGE_LOAD_ERROR_CORRUPT_FILE;
                 return EINA_FALSE;
              }

            data_start = eina_binbuf_manage_read_only_new_length(m + offset, block_length);
            offset += block_length;

            EINA_RECTANGLE_SET(&current, x, y,
                               image->block.width, image->block.height);

            if (!eina_rectangle_intersection(&current, &master))
              continue;

            if (image->compress)
              {
                 if (!emile_binbuf_expand(data_start, buffer, EMILE_LZ4HC))
                   goto on_error;
              }
            else
              {
                 buffer = data_start;
                 if (block_count * etc_block_size != block_length)
                   goto on_error;
              }
            it = eina_binbuf_string_get(buffer);

            for (i = 0; i < image->block.height; i += 4)
              for (j = 0; j < image->block.width; j += 4, it += etc_block_size)
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
                      case EMILE_COLORSPACE_ARGB8888:
                        switch (image->cspace)
                          {
                           case EMILE_COLORSPACE_ETC1:
                           case EMILE_COLORSPACE_ETC1_ALPHA:
                             if (!rg_etc1_unpack_block(it, temporary, 0))
                               {
                                  // TODO: Should we decode as RGB8_ETC2?
                                  fprintf(stderr, "ETC1: Block starting at {%i, %i} is corrupted!\n", x + j, y + i);
                                  continue;
                               }
                             break;
                           case EMILE_COLORSPACE_RGB8_ETC2:
                             rg_etc2_rgb8_decode_block((uint8_t *) it, temporary);
                             break;
                           case EMILE_COLORSPACE_RGBA8_ETC2_EAC:
                             rg_etc2_rgba8_decode_block((uint8_t *) it, temporary);
                             break;
                           default: abort();
                          }

                        offset_x = current_etc.x - x - j;
                        offset_y = current_etc.y - y - i;

                        if (!plane)
                          {
#ifdef BUILD_NEON
                             if (eina_cpu_features_get() & EINA_CPU_NEON)
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
                      case EMILE_COLORSPACE_ETC1:
                      case EMILE_COLORSPACE_RGB8_ETC2:
                      case EMILE_COLORSPACE_RGBA8_ETC2_EAC:
                        memcpy(&p_etc[(current_etc.x / 4) * etc_block_size +
                                      (current_etc.y / 4) * etc_width],
                               it, etc_block_size);
                        break;
                      case EMILE_COLORSPACE_ETC1_ALPHA:
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
   if (prop->cspace == EMILE_COLORSPACE_ARGB8888)
     prop->premul = image->unpremul; // call premul if unpremul data

   r = EINA_TRUE;

 on_error:
   return r;
}

static void
_emile_tgv_close(Emile_Image *image EINA_UNUSED)
{
   // TGV file loader doesn't keep any data allocated around
}

/* JPEG Handling */
static Eina_Bool
_emile_jpeg_bind(Emile_Image *image,
                 Emile_Image_Load_Opts *opts,
                 Emile_Image_Animated *animated,
                 Emile_Image_Load_Error *error)
{
   return EINA_FALSE;
}

static Eina_Bool
_emile_jpeg_head(Emile_Image *image,
                 Emile_Image_Property *prop,
                 unsigned int property_size,
                 Emile_Image_Load_Error *error)
{
   return EINA_FALSE;
}

static Eina_Bool
_emile_jpeg_data(Emile_Image *image,
                 Emile_Image_Property *prop,
                 unsigned int property_size,
                 void *pixels,
                 Emile_Image_Load_Error *error)
{
   return EINA_FALSE;
}

static void
_emile_jpeg_close(Emile_Image *image)
{
}

/* Generic helper to instantiate a new Emile_Image */

static Emile_Image *
_emile_image_new(Eina_Bool (*bind) (Emile_Image *image,
                                    Emile_Image_Load_Opts *opts,
                                    Emile_Image_Animated *animated,
                                    Emile_Image_Load_Error *error),
                 Eina_Bool (*head)(Emile_Image *image,
                                   Emile_Image_Property *prop,
                                   unsigned int property_size,
                                   Emile_Image_Load_Error *error),
                 Eina_Bool (*data)(Emile_Image *image,
                                   Emile_Image_Property *prop,
                                   unsigned int property_size,
                                   void *pixels,
                                   Emile_Image_Load_Error *error),
                 void (*close)(Emile_Image *image))
{
   Emile_Image *ei;

   ei = calloc(1, sizeof (Emile_Image));
   if (!ei) return NULL;

   ei->bind = bind;
   ei->head = head;
   ei->data = data;
   ei->close = close;

   return ei;
}

static void
_emile_image_binbuf_set(Emile_Image *ei, Eina_Binbuf *source)
{
   ei->source.bin = source;
   ei->bin_source = EINA_TRUE;
}

static void
_emile_image_file_set(Emile_Image *ei, Eina_File *source)
{
   ei->source.f = eina_file_dup(source);
   ei->bin_source = EINA_FALSE;
}

static Emile_Image *
_emile_image_bind(Emile_Image *ei,
                  Emile_Image_Load_Opts *opts,
                  Emile_Image_Animated *animated,
                  Emile_Image_Load_Error *error)
{
   if (opts)
     {
        ei->opts = *opts;
        ei->load_opts = 1;
     }

   *error = EMILE_IMAGE_LOAD_ERROR_NONE;
   if (ei->bind(ei, opts, animated, error))
     return ei;

   /* File is not of that format */
   free(ei);
   return NULL;
}

/* Public API to manipulate Emile_Image */

EAPI Emile_Image *
emile_image_tgv_memory_open(Eina_Binbuf *source,
                            Emile_Image_Load_Opts *opts,
                            Emile_Image_Animated *animated,
                            Emile_Image_Load_Error *error)
{
   Emile_Image *ei;

   ei = _emile_image_new(_emile_tgv_bind,
                         _emile_tgv_head,
                         _emile_tgv_data,
                         _emile_tgv_close);
   if (!ei) return NULL;

   _emile_image_binbuf_set(ei, source);
   return _emile_image_bind(ei, opts, animated, error);
}

EAPI Emile_Image *
emile_image_tgv_file_open(Eina_File *source,
                          Emile_Image_Load_Opts *opts,
                          Emile_Image_Animated *animated,
                          Emile_Image_Load_Error *error)
{
   Emile_Image *ei;

   ei = _emile_image_new(_emile_tgv_bind,
                         _emile_tgv_head,
                         _emile_tgv_data,
                         _emile_tgv_close);
   if (!ei) return NULL;

   _emile_image_file_set(ei, source);
   return _emile_image_bind(ei, opts, animated, error);
}

EAPI Emile_Image *
emile_image_jpeg_memory_open(Eina_Binbuf *source,
                             Emile_Image_Load_Opts *opts,
                             Emile_Image_Animated *animated,
                             Emile_Image_Load_Error *error)
{
   Emile_Image *ei;

   ei = _emile_image_new(_emile_jpeg_bind,
                         _emile_jpeg_head,
                         _emile_jpeg_data,
                         _emile_jpeg_close);
   if (!ei) return NULL;

   _emile_image_binbuf_set(ei, source);
   return _emile_image_bind(ei, opts, animated, error);
}

EAPI Emile_Image *
emile_image_jpeg_file_open(Eina_File *source,
                           Emile_Image_Load_Opts *opts,
                           Emile_Image_Animated *animated,
                           Emile_Image_Load_Error *error)
{
   Emile_Image *ei;

   ei = _emile_image_new(_emile_jpeg_bind,
                         _emile_jpeg_head,
                         _emile_jpeg_data,
                         _emile_jpeg_close);
   if (!ei) return NULL;

   _emile_image_file_set(ei, source);
   return _emile_image_bind(ei, opts, animated, error);
}

EAPI void
emile_image_close(Emile_Image *image)
{
   if (!image) return ;

   _emile_image_file_source_unmap(image);
   if (!image->bin_source)
     eina_file_close(image->source.f);
   image->close(image);
   free(image);
}

EAPI Eina_Bool
emile_image_head(Emile_Image *image,
                 Emile_Image_Property *prop,
                 unsigned int property_size,
                 Emile_Image_Load_Error *error)
{
   if (!image) return EINA_FALSE;

   *error = EMILE_IMAGE_LOAD_ERROR_NONE;
   return image->head(image, prop, property_size, error);
}

EAPI Eina_Bool
emile_image_data(Emile_Image *image,
                 Emile_Image_Property *prop,
                 unsigned int property_size,
                 void *pixels,
                 Emile_Image_Load_Error *error)
{
   if (!image) return EINA_FALSE;

   *error = EMILE_IMAGE_LOAD_ERROR_NONE;
   return image->data(image, prop, property_size, pixels, error);
}

EAPI const char *
emile_load_error_str(Emile_Image *source EINA_UNUSED,
                     Emile_Image_Load_Error error)
{
   switch (error)
     {
      case EMILE_IMAGE_LOAD_ERROR_NONE:
         return "No error";
      case EMILE_IMAGE_LOAD_ERROR_GENERIC:
         return "Generic error encountered while loading image.";
      case EMILE_IMAGE_LOAD_ERROR_DOES_NOT_EXIST:
         return "File does not exist.";
      case EMILE_IMAGE_LOAD_ERROR_PERMISSION_DENIED:
         return "Permission to open file denied.";
      case EMILE_IMAGE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED:
         return "Not enough memory to open file.";
      case EMILE_IMAGE_LOAD_ERROR_CORRUPT_FILE:
         return "File is corrupted.";
      case EMILE_IMAGE_LOAD_ERROR_UNKNOWN_FORMAT:
         return "Unexpected file format.";
     }
   return NULL;
}

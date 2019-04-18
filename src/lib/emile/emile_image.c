#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef ENABLE_LIBLZ4
#include <lz4.h>
#else
#include "lz4.h"
#endif

#include <stdio.h>
#include <setjmp.h>
#include <jpeglib.h>
#include <jerror.h>

#include "rg_etc1.h"
#include "Emile.h"
#include "emile_private.h"

#ifdef BUILD_NEON
#include <arm_neon.h>
#endif

#define IMG_MAX_SIZE 65000

#define IMG_TOO_BIG(w, h)                                 \
  ((((unsigned long long)w) * ((unsigned long long)h)) >= \
   ((1ULL << (29 * (sizeof(void *) / 4))) - 2048))

#define SPANS_COMMON(x1, w1, x2, w2) \
  (!(((int)((x2) + (int)(w2)) <= (int)(x1)) || (int)((x2) >= (int)((x1) + (int)(w1)))))

#define RECTS_INTERSECT(x, y, w, h, xx, yy, ww, hh) \
  ((SPANS_COMMON((x), (w), (xx), (ww))) && (SPANS_COMMON((y), (h), (yy), (hh))))

#define RECTS_CLIP_TO_RECT(_x, _y, _w, _h, _cx, _cy, _cw, _ch)    \
  {                                                               \
     if (RECTS_INTERSECT(_x, _y, _w, _h, _cx, _cy, _cw, _ch))     \
       {                                                          \
          if ((int)_x < (int)(_cx))                               \
            {                                                     \
               if ((int)_w + ((int)_x - (int)(_cx)) < 0) _w = 0;  \
               else _w += ((int)_x - (int)(_cx));                 \
               _x = (_cx);                                        \
            }                                                     \
          if ((int)(_x + _w) > (int)((_cx) + (_cw)))              \
            _w = (_cx) + (_cw) - _x;                              \
          if ((int)_y < (int)(_cy))                               \
            {                                                     \
               if ((int)_h + ((int)_y - (int)(_cy)) < 0) _h = 0;  \
               else _h += ((int)_y - (int)(_cy));                 \
               _y = (_cy);                                        \
            }                                                     \
          if ((int)(_y + _h) > (int)((_cy) + (_ch)))              \
            _h = (_cy) + (_ch) - _y;                              \
       }                                                          \
     else                                                         \
       {                                                          \
          _w = 0; _h = 0;                                         \
       }                                                          \
  }

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

#define ARGB_JOIN(a, r, g, b) \
  (((a) << 24) + ((r) << 16) + ((g) << 8) + (b))

#define OFFSET_BLOCK_SIZE 4
#define OFFSET_ALGORITHM  5
#define OFFSET_OPTIONS    6
#define OFFSET_WIDTH      8
#define OFFSET_HEIGHT     12
#define OFFSET_BLOCKS     16

struct _Emile_Image
{
   Emile_Image_Load_Opts opts;

   struct
   {
      unsigned int width;
      unsigned int height;
   } size, block;

   Eina_Rectangle        region;

   union
   {
      Eina_Binbuf *bin;
      Eina_File   *f;
   } source;

   const unsigned char  *source_data;

   Eina_Bool             (*bind)(Emile_Image *image, Emile_Image_Load_Opts *opts, Emile_Image_Animated *animated, Emile_Image_Load_Error *error);
   Eina_Bool             (*head)(Emile_Image *image, Emile_Image_Property *prop, unsigned int property_size, Emile_Image_Load_Error *error);
   Eina_Bool             (*data)(Emile_Image *image, Emile_Image_Property *prop, unsigned int property_size, void *pixels, Emile_Image_Load_Error *error);
   void                  (*close)(Emile_Image *image);

   Emile_Action_Cb       cancelled;
   const void           *cancelled_data;

   Emile_Colorspace      cspace;

   Eina_Bool             bin_source : 1;

   /* TGV option */
   Eina_Bool             unpremul : 1;
   Eina_Bool             compress : 1;
   Eina_Bool             blockless : 1;
   Eina_Bool             load_opts : 1;
};

static inline Eina_Bool
_emile_image_cancelled_is(Emile_Image *image)
{
   if (!image->cancelled) return EINA_FALSE;
   return image->cancelled((void*) image->cancelled_data, image, EMILE_ACTION_CANCELLED);
}

#define EMILE_IMAGE_TASK_CHECK(Image, Count, Mask, Error, Error_Handler) \
  do {                                                                  \
     Count++;                                                           \
     if ((Count & Mask) == Mask)                                        \
       {                                                                \
          Count = 0;                                                    \
          if (_emile_image_cancelled_is(Image))                         \
            {                                                           \
               *Error = EMILE_IMAGE_LOAD_ERROR_CANCELLED;               \
               goto Error_Handler;                                      \
            }                                                           \
       }                                                                \
  } while (0);

static const unsigned char *
_emile_image_file_source_map(Emile_Image *image, unsigned int *length)
{
   if (!image)
     return NULL;

   if (image->bin_source)
     {
        if (length)
          *length = eina_binbuf_length_get(image->source.bin);
        return eina_binbuf_string_get(image->source.bin);
     }

   if (length)
     *length = eina_file_size_get(image->source.f);
   if (!image->source_data)
     {
        image->source_data = eina_file_map_all(image->source.f, EINA_FILE_SEQUENTIAL);
     }
   return image->source_data;
}

static void
_emile_image_file_source_unmap(Emile_Image *image)
{
   if (!(image && image->source_data))
     return;
   eina_file_map_free(image->source.f, (void *)image->source_data);
   image->source_data = NULL;
}

static int
_roundup(int val, int rup)
{
   if (val >= 0 && rup > 0)
     return (val + rup - 1) - ((val + rup - 1) % rup);
   return 0;
}

/* TGV Handling */

static const Emile_Colorspace cspaces_etc1[] = {
   EMILE_COLORSPACE_ETC1,
   EMILE_COLORSPACE_RGB8_ETC2,
   EMILE_COLORSPACE_ARGB8888
};

static const Emile_Colorspace cspaces_rgb8_etc2[] = {
   EMILE_COLORSPACE_RGB8_ETC2,
   EMILE_COLORSPACE_ARGB8888
};

static const Emile_Colorspace cspaces_rgba8_etc2_eac[] = {
   EMILE_COLORSPACE_RGBA8_ETC2_EAC,
   EMILE_COLORSPACE_ARGB8888
};

static const Emile_Colorspace cspaces_etc1_alpha[] = {
   EMILE_COLORSPACE_ETC1_ALPHA,
   EMILE_COLORSPACE_ARGB8888
};

static const Emile_Colorspace cspaces_gry[] = {
   EMILE_COLORSPACE_GRY8,
   EMILE_COLORSPACE_AGRY88,
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
                Emile_Image_Load_Opts *opts EINA_UNUSED,
                Emile_Image_Animated *animated EINA_UNUSED,
                Emile_Image_Load_Error *error)
{
   const unsigned char *m;
   unsigned int length;

   m = _emile_image_file_source_map(image, &length);
   if (!m)
     return EINA_FALSE;

   /* Fast check for file characteristic (useful when trying to guess
      a file format without extention). */
   if (length < 16 || strncmp((const char *)m, "TGV1", 4) != 0)
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
   if (!m)
     return EINA_FALSE;

   /* This can be used for later ABI change of the structure. */
   if (sizeof(Emile_Image_Property) != property_size)
     return EINA_FALSE;

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
        image->cspace = EMILE_COLORSPACE_ETC1_ALPHA;
        break;

      default:
        *error = EMILE_IMAGE_LOAD_ERROR_CORRUPT_FILE;
        return EINA_FALSE;
     }

   image->compress = m[OFFSET_OPTIONS] & 0x1;
   image->blockless = (m[OFFSET_OPTIONS] & 0x2) != 0;

   image->size.width = eina_ntohl(*((unsigned int *)&(m[OFFSET_WIDTH])));
   image->size.height = eina_ntohl(*((unsigned int *)&(m[OFFSET_HEIGHT])));

   if (image->blockless)
     {
        image->block.width = _roundup(image->size.width + 2, 4);
        image->block.height = _roundup(image->size.height + 2, 4);
     }
   else
     {
        image->block.width = 4 << (m[OFFSET_BLOCK_SIZE] & 0x0f);
        image->block.height = 4 << ((m[OFFSET_BLOCK_SIZE] & 0xf0) >> 4);
     }

   EINA_RECTANGLE_SET(&image->region,
                      0, 0,
                      image->size.width, image->size.height);
   if (image->load_opts &&
       ((image->opts.region.w > 0) && (image->opts.region.h > 0) &&
        (image->opts.region.w != (int) image->size.width) &&
        (image->opts.region.h != (int) image->size.height)))
     {
        /* ETC colorspace doesn't work with region for now */
        prop->cspaces = NULL;

        if (!eina_rectangle_intersection(&image->region, &image->opts.region))
          {
             *error = EMILE_IMAGE_LOAD_ERROR_GENERIC;
             return EINA_FALSE;
          }
     }

   prop->w = image->size.width;
   prop->h = image->size.height;
   prop->borders.l = 1;
   prop->borders.t = 1;
   prop->borders.r = _roundup(prop->w + 2, 4) - prop->w - 1;
   prop->borders.b = _roundup(prop->h + 2, 4) - prop->h - 1;

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

   if (sizeof(Emile_Image_Property) != property_size)
     return EINA_FALSE;

   offset = OFFSET_BLOCKS;

   *error = EMILE_IMAGE_LOAD_ERROR_CORRUPT_FILE;

   /* By definition, prop{.w, .h} == region{.w, .h} */
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

      default:
        abort();
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
        /* Offset to take duplicated pixels into account */
        master.x += 1;
        master.y += 1;
        break;

      default:
        abort();
     }

   if (prop->cspace != EMILE_COLORSPACE_ARGB8888 &&
       prop->cspace != image->cspace)
     {
        if (!((prop->cspace == EMILE_COLORSPACE_RGB8_ETC2) &&
              (image->cspace == EMILE_COLORSPACE_ETC1)))
          {
             *error = EMILE_IMAGE_LOAD_ERROR_GENERIC;
             return EINA_FALSE;
          }
        /* else: ETC2 is compatible with ETC1 and is preferred */
     }

   /* Allocate space for each ETC block (8 or 16 bytes per 4 * 4 pixels group) */
   block_count = image->block.width * image->block.height / (4 * 4);
   if (image->compress)
     buffer = eina_binbuf_manage_new(alloca(etc_block_size * block_count),
                                     etc_block_size * block_count,
                                     EINA_TRUE);

   for (plane = 0; plane < num_planes; plane++)
     for (y = 0; y < image->size.height + 2; y += image->block.height)
       for (x = 0; x < image->size.width + 2; x += image->block.width)
         {
            Eina_Rectangle current;
            Eina_Binbuf *data_start;
            const unsigned char *it;
            unsigned int i, j;

            block_length = _tgv_length_get(m + offset, length, &offset);

            if (block_length == 0)
              {
                 *error = EMILE_IMAGE_LOAD_ERROR_CORRUPT_FILE;
                 goto on_error;
              }

            data_start = eina_binbuf_manage_new(m + offset,
                                                block_length,
                                                EINA_TRUE);
            offset += block_length;

            EINA_RECTANGLE_SET(&current,
                               x, y,
                               image->block.width, image->block.height);

            if (!eina_rectangle_intersection(&current, &master))
              {
                 eina_binbuf_free(data_start);
                 continue;
              }

            if (image->compress)
              {
                 if (!emile_expand(data_start, buffer, EMILE_LZ4HC))
                   {
                      eina_binbuf_free(data_start);
                      goto on_error;
                   }
              }
            else
              {
                 buffer = data_start;
                 if (block_count * etc_block_size != block_length)
                   {
                      eina_binbuf_free(data_start);
                      goto on_error;
                   }
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
                             rg_etc2_rgb8_decode_block((uint8_t *)it, temporary);
                             break;

                           case EMILE_COLORSPACE_RGBA8_ETC2_EAC:
                             rg_etc2_rgba8_decode_block((uint8_t *)it, temporary);
                             break;

                           default:
                             abort();
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
                                         current_etc.w * sizeof(unsigned int));
                               }
                          }
                        else
                          {
                             for (k = 0; k < current_etc.h; k++)
                               for (l = 0; l < current_etc.w; l++)
                                 {
                                    unsigned int *rgbdata = &p[current_etc.x - 1 + (current_etc.y - 1 + k) * master.w + l];
                                    unsigned int *adata = &temporary[offset_x + (offset_y + k) * 4 + l];
                                    A_VAL(rgbdata) = G_VAL(adata);
                                 }
                          }
                        break;

                      case EMILE_COLORSPACE_ETC1:
                      case EMILE_COLORSPACE_RGB8_ETC2:
                      case EMILE_COLORSPACE_RGBA8_ETC2_EAC:
                        memcpy(&p_etc[(current_etc.x / 4) * etc_block_size + (current_etc.y / 4) * etc_width],
                               it,
                               etc_block_size);
                        break;

                      case EMILE_COLORSPACE_ETC1_ALPHA:
                        memcpy(&p_etc[(current_etc.x / 4) * etc_block_size + (current_etc.y / 4) * etc_width + plane * alpha_offset],
                               it,
                               etc_block_size);
                        break;

                      default:
                        abort();
                     }
                } /* bx,by inside blocks */

            eina_binbuf_free(data_start);
         } /* x,y macroblocks */

   // TODO: Add support for more unpremultiplied modes (ETC2)
   if (prop->cspace == EMILE_COLORSPACE_ARGB8888)
     prop->premul = image->unpremul;  /* call premul if unpremul data */

   *error = EMILE_IMAGE_LOAD_ERROR_NONE;
   r = EINA_TRUE;

on_error:
   if (image->compress) eina_binbuf_free(buffer);
   _emile_image_file_source_unmap(image);
   return r;
}

static void
_emile_tgv_close(Emile_Image *image EINA_UNUSED)
{
   /* TGV file loader doesn't keep any data allocated around */
}

/* JPEG Handling */

typedef struct _JPEG_error_mgr *emptr;
struct _JPEG_error_mgr
{
   struct jpeg_error_mgr pub;
   jmp_buf               setjmp_buffer;
};

struct jpeg_membuf_src
{
   struct jpeg_source_mgr  pub;

   const unsigned char    *buf;
   size_t                  len;
   struct jpeg_membuf_src *self;
};

static void
_emile_image_jpeg_error_exit_cb(j_common_ptr cinfo)
{
   char buffer[JMSG_LENGTH_MAX];
   emptr errmgr;

   // Avoid message "Not a JPEG file: starts with 0x%02x 0x%02x"
   if (cinfo->client_data || (cinfo->err->msg_code != JERR_NO_SOI))
     {
        (*cinfo->err->format_message)(cinfo, buffer);
        ERR("%s", buffer);
     }
   errmgr = (emptr)cinfo->err;
   longjmp(errmgr->setjmp_buffer, 1);
}

static void
_emile_image_jpeg_emit_message_cb(j_common_ptr cinfo, int msg_level)
{
   char buffer[JMSG_LENGTH_MAX];
   struct jpeg_error_mgr *err;

   err = cinfo->err;
   if (msg_level < 0)
     {
        if ((err->num_warnings == 0) || (err->trace_level >= 3))
          {
             (*cinfo->err->format_message)(cinfo, buffer);
             WRN("%s", buffer);
          }
        err->num_warnings++;
     }
   else
     {
        if (err->trace_level >= msg_level)
          {
             (*cinfo->err->format_message)(cinfo, buffer);
             INF("%s", buffer);
          }
     }
}

static void
_emile_image_jpeg_output_message_cb(j_common_ptr cinfo)
{
   char buffer[JMSG_LENGTH_MAX];

   (*cinfo->err->format_message)(cinfo, buffer);
   ERR("%s", buffer);
}

static void
_emile_jpeg_membuf_src_init(j_decompress_ptr cinfo EINA_UNUSED)
{
}

static boolean
_emile_jpeg_membuf_src_fill(j_decompress_ptr cinfo)
{
   static const JOCTET jpeg_eoi[2] = { 0xFF, JPEG_EOI };
   struct jpeg_membuf_src *src = (struct jpeg_membuf_src *)cinfo->src;

   src->pub.bytes_in_buffer = sizeof(jpeg_eoi);
   src->pub.next_input_byte = jpeg_eoi;

   return TRUE;
}

static void
_emile_jpeg_membuf_src_skip(j_decompress_ptr cinfo, long num_bytes)
{
   struct jpeg_membuf_src *src = (struct jpeg_membuf_src *)cinfo->src;

   if ((((long)src->pub.bytes_in_buffer - (long)src->len) > num_bytes) ||
       ((long)src->pub.bytes_in_buffer < num_bytes))
     {
        (*(cinfo)->err->error_exit)((j_common_ptr)(cinfo));
        return;
     }
   src->pub.bytes_in_buffer -= num_bytes;
   src->pub.next_input_byte += num_bytes;
}

static void
_emile_jpeg_membuf_src_term(j_decompress_ptr cinfo)
{
   struct jpeg_membuf_src *src = (struct jpeg_membuf_src *)cinfo->src;
   if (!src)
     return;
   free(src);
   cinfo->src = NULL;
}

static int
_emile_jpeg_membuf_src(j_decompress_ptr cinfo, const void *map, size_t length)
{
   struct jpeg_membuf_src *src;

   src = calloc(1, sizeof(*src));
   if (!src)
     return -1;

   src->self = src;

   cinfo->src = &src->pub;
   src->buf = map;
   src->len = length;
   src->pub.init_source = _emile_jpeg_membuf_src_init;
   src->pub.fill_input_buffer = _emile_jpeg_membuf_src_fill;
   src->pub.skip_input_data = _emile_jpeg_membuf_src_skip;
   src->pub.resync_to_restart = jpeg_resync_to_restart;
   src->pub.term_source = _emile_jpeg_membuf_src_term;
   src->pub.bytes_in_buffer = src->len;
   src->pub.next_input_byte = src->buf;

   return 0;
}

/*! Magic number for EXIF header, App0, App1*/
static const unsigned char ExifHeader[] = { 0x45, 0x78, 0x69, 0x66, 0x00, 0x00 };
static const unsigned char JfifHeader[] = { 0x4A, 0x46, 0x49, 0x46, 0x00 };
static const unsigned char JfxxHeader[] = { 0x4A, 0x46, 0x58, 0x58, 0x00 };
static const unsigned char App0[] = { 0xff, 0xe0 };
static const unsigned char App1[] = { 0xff, 0xe1 };
static const unsigned char II[] = { 0x49, 0x49 };
static const unsigned char MM[] = { 0x4d, 0x4d };

typedef enum
{
   EXIF_BYTE_ALIGN_II,
   EXIF_BYTE_ALIGN_MM
} ExifByteAlign;

static Eina_Bool
_get_next_app0(const unsigned char *map, size_t fsize, size_t *position)
{
   unsigned short length = 0;
   unsigned int w = 0, h = 0;
   unsigned int format = 0;
   unsigned int data_size = 0;
   const unsigned char *app0_head, *p;

   /* header_mark:2, length:2, identifier:5 version:2, unit:1, den=4 thum=2 */
   if ((*position + 16) >= fsize)
     return EINA_FALSE;
   app0_head = map + *position;

   /* p is appn's start pointer excluding app0 marker */
   p = app0_head + 2;

   length = ((*p << 8) + *(p + 1));

   /* JFIF segment format */
   if (!memcmp(p + 2, JfifHeader, sizeof(JfifHeader)))
     {
        format = 3;
        w = *(p + 14);
        h = *(p + 15);
     }
   else if (!memcmp(p + 2, JfxxHeader, sizeof(JfxxHeader)))
     {
        if (*(p + 7) == 0x11)
          format = 1;
        else
          format = 3;
        w = *(p + 8);
        h = *(p + 9);
     }

   data_size = format * w * h;

   if ((*position + 2 + length + data_size) > fsize)
     return EINA_FALSE;

   *position = *position + 2 + length + data_size;

   return EINA_TRUE;
}

/* If app1 data is abnormal, returns EINA_FALSE.
   If app1 data is normal, returns EINA_TRUE.
   If app1 data is normal but not orientation data, orientation value is -1.
 */

static Eina_Bool
_get_orientation_app1(const unsigned char *map,
                      size_t fsize,
                      size_t *position,
                      int *orientation_res,
                      Eina_Bool *flipped)
{
   const unsigned char *app1_head, *buf;
   unsigned char orientation[2];  //orientation tag
   ExifByteAlign byte_align;
   unsigned int num_directory = 0;
   unsigned int ifd_offset = 10; //IFD offset start at 10th byte (mark:2 + data_size:2 + exif:6)
   unsigned int i, j;
   int direction;
   unsigned int data_size = 0;

   /* app1 mark:2, data_size:2, exif:6 tiff:8 */
   if ((*position + 18) >= fsize)
     return EINA_FALSE;
   app1_head = map + *position;
   buf = app1_head;

   data_size = ((*(buf + 2) << 8) + *(buf + 3));
   if ((*position + 2 + data_size) > fsize)
     return EINA_FALSE;

   if (memcmp(buf + 4, ExifHeader, sizeof(ExifHeader)))
     {
        *position = *position + 2 + data_size;
        *orientation_res = -1;
        return EINA_TRUE;
     }

   /* 2. get 10&11 byte  get info of "II(0x4949)" or "MM(0x4d4d)" */
   /* 3. get 14th - 17th byte get info for IFD offset */
   /* 4. get directory entry IFD */


   if (!memcmp(buf + 10, MM, sizeof(MM)))
     {
        // get 4byte by little endian
        ifd_offset += (*(buf + 14) << 24) + (*(buf + 15) << 16) + (*(buf + 16) << 8) + (*(buf + 17));

        if (ifd_offset > fsize)
          return EINA_FALSE;

        byte_align = EXIF_BYTE_ALIGN_MM;
        num_directory = ((*(buf + ifd_offset) << 8) + *(buf + ifd_offset + 1));
        orientation[0] = 0x01;
        orientation[1] = 0x12;
     }
   else if (!memcmp(buf + 10, II, sizeof(II)))
     {
        // get 4byte by big endian
        ifd_offset += (*(buf + 14))  + (*(buf + 15) << 8) + (*(buf + 16) << 16) + (*(buf + 17) << 24);

        if (ifd_offset > fsize)
          return EINA_FALSE;

        byte_align = EXIF_BYTE_ALIGN_II;
        num_directory = ((*(buf + ifd_offset + 1) << 8) + *(buf + ifd_offset));
        orientation[0] = 0x12;
        orientation[1] = 0x01;
     }
   else
     return EINA_FALSE;

   /* check num_directory data */
   if ((*position + (12 * num_directory + 20)) > fsize)
     return EINA_FALSE;

   buf = app1_head + ifd_offset + 2;  //next to 0th ifd (1st tag)

   j = 0;

   for (i = 0; i < num_directory; i++)
     {
        if (!memcmp(buf + j, orientation, 2))
          {
             /*get orientation tag */
             if (byte_align == EXIF_BYTE_ALIGN_MM)
               direction = *(buf + j + 9);
             else
               direction = *(buf + j + 8);
             switch (direction)
               {
                case 3:
                  *orientation_res = 180;
                  *flipped = EINA_FALSE;
                  return EINA_TRUE;

                case 4:
                  *orientation_res = 180;
                  *flipped = EINA_TRUE;
                  return EINA_TRUE;

                case 6:
                  *orientation_res = 90;
                  *flipped = EINA_FALSE;
                  return EINA_TRUE;

                case 7:
                  *orientation_res = 90;
                  *flipped = EINA_TRUE;
                  return EINA_TRUE;

                case 5:
                  *orientation_res = 270;
                  *flipped = EINA_TRUE;
                  return EINA_TRUE;

                case 8:
                  *orientation_res = 270;
                  *flipped = EINA_FALSE;
                  return EINA_TRUE;

                case 2:
                  *orientation_res = 0;
                  *flipped = EINA_TRUE;
                  return EINA_TRUE;

                default:
                  *orientation_res = 0;
                  *flipped = EINA_FALSE;
                  return EINA_TRUE;
               }
          }
        else
          j = j + 12;
     }
   return EINA_FALSE;
}

static int
_get_orientation(const void *map, size_t length, Eina_Bool *flipped)
{
   unsigned char *buf;
   size_t position = 0;
   int orientation = -1;
   Eina_Bool res = EINA_FALSE;

   *flipped = EINA_FALSE;

   /* open file and get 22 byte frome file */
   if (!map)
     return 0;
   /* 1. read 22byte */
   if (length < 22)
     return 0;
   buf = (unsigned char *)map;

   position = 2;
   /* 2. check 2,3 bypte with APP0(0xFFE0) or APP1(0xFFE1) */
   while ((length - position) > 0)
     {
        if (!memcmp(buf + position, App0, sizeof(App0)))
          {
             res = _get_next_app0(map, length, &position);
             if (!res)
               break;
          }
        else if (!memcmp(buf + position, App1, sizeof(App1)))
          {
             res = _get_orientation_app1(map,
                                         length,
                                         &position,
                                         &orientation,
                                         flipped);
             if (!res)
               break;
             if (orientation != -1)
               return orientation;
          }
        else
          break;
     }
   return 0;
}

static void
_rotate_region(unsigned int *r_x, unsigned int *r_y,
               unsigned int *r_w, unsigned int *r_h,
               unsigned int x, unsigned int y,
               unsigned int w, unsigned int h,
               unsigned int output_w, unsigned int output_h,
               int degree, Eina_Bool flipped)
{
   switch (degree)
     {
      case 90:
        if (flipped)
          {
             *r_x = output_w - (y + h);
             *r_y = output_h - (x + w);
             *r_w = h;
             *r_h = w;
          }
        else
          {
             *r_x = y;
             *r_y = output_h - (x + w);
             *r_w = h;
             *r_h = w;
          }
        break;

      case 180:
        if (flipped)
          {
             *r_y = output_h - (y + h);
          }
        else
          {
             *r_x = output_w - (x + w);
             *r_y = output_h - (y + h);
          }
        break;

      case 270:
        if (flipped)
          {
             *r_x = y;
             *r_y = x;
             *r_w = h;
             *r_h = w;
          }
        else
          {
             *r_x = output_w - (y + h);
             *r_y = x;
             *r_w = h;
             *r_h = w;
          }
        break;

      default:
        if (flipped)
          *r_x = output_w - (x + w);
        break;
     }
}

static void
_rotate_180(uint32_t *data, int w, int h)
{
   uint32_t *p1, *p2;
   uint32_t pt;
   int x;

   p1 = data;
   p2 = data + (h * w) - 1;
   for (x = (w * h) / 2; --x >= 0; )
     {
        pt = *p1;
        *p1 = *p2;
        *p2 = pt;
        p1++;
        p2--;
     }
}

static void
_flip_horizontal(uint32_t *data, int w, int h)
{
   uint32_t *p1, *p2;
   uint32_t pt;
   int x, y;

   for (y = 0; y < h; y++)
     {
        p1 = data + (y * w);
        p2 = data + ((y + 1) * w) - 1;
        for (x = 0; x < (w >> 1); x++)
          {
             pt = *p1;
             *p1 = *p2;
             *p2 = pt;
             p1++;
             p2--;
          }
     }
}

static void
_flip_vertical(uint32_t *data, int w, int h)
{
   uint32_t *p1, *p2;
   uint32_t pt;
   int x, y;

   for (y = 0; y < (h >> 1); y++)
     {
        p1 = data + (y * w);
        p2 = data + ((h - 1 - y) * w);
        for (x = 0; x < w; x++)
          {
             pt = *p1;
             *p1 = *p2;
             *p2 = pt;
             p1++;
             p2++;
          }
     }
}

static void
_rotate_change_wh(uint32_t *to, uint32_t *from, int w, int h, int dx, int dy)
{
   int x, y;

   for (x = h; --x >= 0; )
     {
        for (y = w; --y >= 0; )
          {
             *to = *from;
             from++;
             to += dy;
          }
        to += dx;
     }
}

static void
_rotate8_180(uint8_t *data, int w, int h)
{
   uint8_t *p1, *p2;
   uint8_t pt;
   int x;

   p1 = data;
   p2 = data + (h * w) - 1;
   for (x = (w * h) / 2; --x >= 0; )
     {
        pt = *p1;
        *p1 = *p2;
        *p2 = pt;
        p1++;
        p2--;
     }
}

static void
_flip_horizontal8(uint8_t *data, int w, int h)
{
   uint8_t *p1, *p2;
   uint8_t pt;
   int x, y;

   for (y = 0; y < h; y++)
     {
        p1 = data + (y * w);
        p2 = data + ((y + 1) * w) - 1;
        for (x = 0; x < (w >> 1); x++)
          {
             pt = *p1;
             *p1 = *p2;
             *p2 = pt;
             p1++;
             p2--;
          }
     }
}

static void
_flip_vertical8(uint8_t *data, int w, int h)
{
   uint8_t *p1, *p2;
   uint8_t pt;
   int x, y;

   for (y = 0; y < (h >> 1); y++)
     {
        p1 = data + (y * w);
        p2 = data + ((h - 1 - y) * w);
        for (x = 0; x < w; x++)
          {
             pt = *p1;
             *p1 = *p2;
             *p2 = pt;
             p1++;
             p2++;
          }
     }
}

static void
_rotate_change_wh8(uint8_t *to, uint8_t *from, int w, int h, int dx, int dy)
{
   int x, y;

   for (x = h; --x >= 0; )
     {
        for (y = w; --y >= 0; )
          {
             *to = *from;
             from++;
             to += dy;
          }
        to += dx;
     }
}

static void
_rotate16_180(uint16_t *data, int w, int h)
{
   uint16_t *p1, *p2;
   uint16_t pt;
   int x;

   p1 = data;
   p2 = data + (h * w) - 1;
   for (x = (w * h) / 2; --x >= 0; )
     {
        pt = *p1;
        *p1 = *p2;
        *p2 = pt;
        p1++;
        p2--;
     }
}

static void
_flip_horizontal16(uint16_t *data, int w, int h)
{
   uint16_t *p1, *p2;
   uint16_t pt;
   int x, y;

   for (y = 0; y < h; y++)
     {
        p1 = data + (y * w);
        p2 = data + ((y + 1) * w) - 1;
        for (x = 0; x < (w >> 1); x++)
          {
             pt = *p1;
             *p1 = *p2;
             *p2 = pt;
             p1++;
             p2--;
          }
     }
}

static void
_flip_vertical16(uint16_t *data, int w, int h)
{
   uint16_t *p1, *p2;
   uint16_t pt;
   int x, y;

   for (y = 0; y < (h >> 1); y++)
     {
        p1 = data + (y * w);
        p2 = data + ((h - 1 - y) * w);
        for (x = 0; x < w; x++)
          {
             pt = *p1;
             *p1 = *p2;
             *p2 = pt;
             p1++;
             p2++;
          }
     }
}

static void
_rotate_change_wh16(uint16_t *to, uint16_t *from, int w, int h, int dx, int dy)
{
   int x, y;

   for (x = h; --x >= 0; )
     {
        for (y = w; --y >= 0; )
          {
             *to = *from;
             from++;
             to += dy;
          }
        to += dx;
     }
}

static Eina_Bool
_emile_jpeg_bind(Emile_Image *image EINA_UNUSED,
                 Emile_Image_Load_Opts *opts EINA_UNUSED,
                 Emile_Image_Animated *animated EINA_UNUSED,
                 Emile_Image_Load_Error *error EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_emile_jpeg_head(Emile_Image *image,
                 Emile_Image_Property *prop,
                 unsigned int property_size,
                 Emile_Image_Load_Error *error)
{
   volatile Emile_Image_Load_Opts *opts = (image->load_opts) ? &image->opts : NULL;
   const unsigned char *m;
   unsigned int scalew, scaleh;
   struct jpeg_decompress_struct cinfo;
   struct _JPEG_error_mgr jerr;
   unsigned int length;

   /* for rotation decoding */
   volatile int degree = 0;
   volatile Eina_Bool change_wh = EINA_FALSE;
   volatile unsigned int load_opts_w = 0, load_opts_h = 0;

   if (sizeof(Emile_Image_Property) != property_size)
     return EINA_FALSE;

   m = _emile_image_file_source_map(image, &length);
   if (!m)
     return EINA_FALSE;

   memset(&cinfo, 0, sizeof(cinfo));
   cinfo.err = jpeg_std_error(&(jerr.pub));
   cinfo.client_data = NULL;
   jerr.pub.error_exit = _emile_image_jpeg_error_exit_cb;
   jerr.pub.emit_message = _emile_image_jpeg_emit_message_cb;
   jerr.pub.output_message = _emile_image_jpeg_output_message_cb;
   if (setjmp(jerr.setjmp_buffer))
     {
        jpeg_destroy_decompress(&cinfo);
        _emile_jpeg_membuf_src_term(&cinfo);
        if (cinfo.saw_JFIF_marker)
          *error = EMILE_IMAGE_LOAD_ERROR_CORRUPT_FILE;
        else
          *error = EMILE_IMAGE_LOAD_ERROR_UNKNOWN_FORMAT;
        return EINA_FALSE;
     }
   jpeg_create_decompress(&cinfo);

   if (_emile_jpeg_membuf_src(&cinfo, m, length))
     {
        jpeg_destroy_decompress(&cinfo);
        _emile_jpeg_membuf_src_term(&cinfo);
        *error = EMILE_IMAGE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return EINA_FALSE;
     }

   jpeg_read_header(&cinfo, TRUE);
   cinfo.do_fancy_upsampling = FALSE;
   cinfo.do_block_smoothing = FALSE;
   cinfo.dct_method = JDCT_ISLOW; /* JDCT_FLOAT JDCT_IFAST(quality loss) */
   cinfo.dither_mode = JDITHER_ORDERED;
   cinfo.buffered_image = TRUE; /* buffered mode in case jpg is progressive */
   jpeg_start_decompress(&cinfo);

   if (cinfo.jpeg_color_space == JCS_GRAYSCALE)
     {
        /* We do handle GRY8 and AGRY88 (with FF for alpha) colorspace as an output for JPEG */
        prop->cspaces = cspaces_gry;
     }

   /* rotation decoding */
   if (opts && opts->orientation)
     {
        degree = _get_orientation(m, length, &prop->flipped);
        if (degree != 0 || prop->flipped)
          {
             opts->degree = degree;
             prop->rotated = EINA_TRUE;

             if (degree == 90 || degree == 270)
               change_wh = EINA_TRUE;
          }
     }

   /* head decoding */
   prop->w = cinfo.output_width;
   prop->h = cinfo.output_height;
   if ((prop->w < 1) ||
       (prop->h < 1) ||
       (prop->w > IMG_MAX_SIZE) ||
       (prop->h > IMG_MAX_SIZE) ||
       (IMG_TOO_BIG(prop->w, prop->h)))
     {
        jpeg_destroy_decompress(&cinfo);
        _emile_jpeg_membuf_src_term(&cinfo);
        if (IMG_TOO_BIG(prop->w, prop->h))
          *error = EMILE_IMAGE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        else
          *error = EMILE_IMAGE_LOAD_ERROR_GENERIC;
        return EINA_FALSE;
     }
   if (opts && opts->scale_down_by > 1)
     {
        prop->w /= opts->scale_down_by;
        prop->h /= opts->scale_down_by;
     }
   else if (opts && opts->dpi > 0.0)
     {
        prop->w = (prop->w * opts->dpi) / 90.0;
        prop->h = (prop->h * opts->dpi) / 90.0;
     }
   else if (opts && ((opts->w > 0) && (opts->h > 0)))
     {
        unsigned int w2 = prop->w, h2 = prop->h;
        /* user set load_opts' w,h on the assumption
           that image already rotated according to it's orientation info */
        if (change_wh)
          {
             load_opts_w = opts->w;
             load_opts_h = opts->h;
             opts->w = load_opts_h;
             opts->h = load_opts_w;
          }

        w2 = opts->w;
        h2 = (opts->w * prop->h) / prop->w;
        if (h2 > opts->h)
          {
             unsigned int w3;
             h2 = opts->h;
             w3 = (opts->h * prop->w) / prop->h;
             if (w3 > w2)
               w2 = w3;
          }
        prop->w = w2;
        prop->h = h2;
        if (change_wh)
          {
             opts->w = load_opts_w;
             opts->h = load_opts_h;
          }
     }
   if (prop->w < 1)
     prop->w = 1;
   if (prop->h < 1)
     prop->h = 1;

   if ((prop->w != cinfo.output_width) || (prop->h != cinfo.output_height))
     {
        scalew = cinfo.output_width / prop->w;
        scaleh = cinfo.output_height / prop->h;

        prop->scale = scalew;
        if (scaleh < scalew)
          prop->scale = scaleh;

        if (prop->scale > 8)
          prop->scale = 8;
        else if (prop->scale < 1)
          prop->scale = 1;

        if (prop->scale == 3)
          prop->scale = 2;
        else if (prop->scale == 5)
          prop->scale = 4;
        else if (prop->scale == 6)
          prop->scale = 4;
        else if (prop->scale == 7)
          prop->scale = 4;
     }

   if (prop->scale > 1)
     {
        jpeg_destroy_decompress(&cinfo);
        _emile_jpeg_membuf_src_term(&cinfo);
        jpeg_create_decompress(&cinfo);

        if (_emile_jpeg_membuf_src(&cinfo, m, length))
          {
             jpeg_destroy_decompress(&cinfo);
             _emile_jpeg_membuf_src_term(&cinfo);
             *error = EMILE_IMAGE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
             return EINA_FALSE;
          }

        jpeg_read_header(&cinfo, TRUE);
        cinfo.do_fancy_upsampling = FALSE;
        cinfo.do_block_smoothing = FALSE;
        cinfo.scale_num = 1;
        cinfo.scale_denom = prop->scale;
        cinfo.buffered_image = TRUE; /* buffered mode in case jpg is progressive */
        jpeg_calc_output_dimensions(&(cinfo));
        jpeg_start_decompress(&cinfo);
     }

   prop->w = cinfo.output_width;
   prop->h = cinfo.output_height;

   /* be nice and clip region to image. if its totally outside, fail load */
   if (opts && ((opts->region.w > 0) && (opts->region.h > 0)))
     {
        unsigned int load_region_x = opts->region.x;
        unsigned int load_region_y = opts->region.y;
        unsigned int load_region_w = opts->region.w;
        unsigned int load_region_h = opts->region.h;
        if (prop->rotated)
          {
             _rotate_region(&load_region_x, &load_region_y,
                            &load_region_w, &load_region_h,
                            opts->region.x, opts->region.y,
                            opts->region.w, opts->region.h,
                            prop->w, prop->h,
                            degree, prop->flipped);
          }
        if (prop->scale > 1)
          {
             load_region_x /= prop->scale;
             load_region_y /= prop->scale;
             load_region_w /= prop->scale;
             load_region_h /= prop->scale;
          }
        RECTS_CLIP_TO_RECT(load_region_x, load_region_y,
                           load_region_w, load_region_h,
                           0, 0, prop->w, prop->h);
        if ((load_region_w == 0) || (load_region_h == 0))
          {
             jpeg_destroy_decompress(&cinfo);
             _emile_jpeg_membuf_src_term(&cinfo);
             *error = EMILE_IMAGE_LOAD_ERROR_GENERIC;
             return EINA_FALSE;
          }
        prop->w = load_region_w;
        prop->h = load_region_h;
     }
   /* end head decoding */

   if (change_wh)
     {
        unsigned int tmp;
        tmp = prop->w;
        prop->w = prop->h;
        prop->h = tmp;
     }
   jpeg_destroy_decompress(&cinfo);
   _emile_jpeg_membuf_src_term(&cinfo);
   *error = EMILE_IMAGE_LOAD_ERROR_NONE;
   return EINA_TRUE;
}

static inline void
_jpeg_convert_copy(volatile uint32_t **dst, uint8_t **src, unsigned int w, Eina_Bool adobe_marker)
{
   uint32_t *ptr2 = (uint32_t*) *dst;
   uint8_t *ptr = *src;
   unsigned int x;

   if (adobe_marker)
     {
        for (x = 0; x < w; x++)
          {
             /* According to libjpeg doc, Photoshop inverse the values of C, M, Y and K, */
             /* that is C is replaces by 255 - C, etc... */
             /* See the comment below for the computation of RGB values from CMYK ones. */
             *ptr2 = (0xff000000) |
               ((ptr[0] * ptr[3] / 255) << 16) |
               ((ptr[1] * ptr[3] / 255) << 8) |
               ((ptr[2] * ptr[3] / 255));
             ptr += 4;
             ptr2++;
          }
     }
   else
     {
        for (x = 0; x < w; x++)
          {
             /* Conversion from CMYK to RGB is done in 2 steps: */
             /* CMYK => CMY => RGB (see http://www.easyrgb.com/index.php?X=MATH) */
             /* after computation, if C, M, Y and K are between 0 and 1, we have: */
             /* R = (1 - C) * (1 - K) * 255 */
             /* G = (1 - M) * (1 - K) * 255 */
             /* B = (1 - Y) * (1 - K) * 255 */
             /* libjpeg stores CMYK values between 0 and 255, */
             /* so we replace C by C * 255 / 255, etc... and we obtain: */
             /* R = (255 - C) * (255 - K) / 255 */
             /* G = (255 - M) * (255 - K) / 255 */
             /* B = (255 - Y) * (255 - K) / 255 */
             /* with C, M, Y and K between 0 and 255. */
             *ptr2 = (0xff000000) |
               (((255 - ptr[0]) * (255 - ptr[3]) / 255) << 16) |
               (((255 - ptr[1]) * (255 - ptr[3]) / 255) << 8) |
               (((255 - ptr[2]) * (255 - ptr[3]) / 255));
             ptr += 4;
             ptr2++;
          }
     }

   *dst = ptr2;
   *src = ptr;
}

static inline void
_jpeg_gry8_convert_copy(uint8_t **dst, uint8_t **src, unsigned int w)
{
   uint8_t *ptrg = (uint8_t*) *dst;
   uint8_t *ptr = *src;
   unsigned int x;

   for (x = 0; x < w; x++)
     {
        *ptrg = ptr[0];
        ptrg++;
        ptr++;
     }

   *dst = ptrg;
   *src = ptr;
}

static inline void
_jpeg_agry88_convert_copy(uint16_t **dst, uint8_t **src, unsigned int w)
{
   uint16_t *ptrag = (uint16_t*) *dst;
   uint8_t *ptr = *src;
   unsigned int x;

   for (x = 0; x < w; x++)
     {
        *ptrag = 0xFF00 | ptr[0];
        ptrag++;
        ptr++;
     }

   *dst = ptrag;
   *src = ptr;
}

static inline void
_jpeg_argb8888_convert_copy(volatile uint32_t **dst, uint8_t **src, unsigned int w)
{
   uint32_t *ptr2 = (uint32_t*) *dst;
   uint8_t *ptr = *src;
   unsigned int x;

   for (x = 0; x < w; x++)
     {
        *ptr2 = ARGB_JOIN(0xff, ptr[0], ptr[0], ptr[0]);
        ptr2++;
        ptr++;
     }

   *dst = ptr2;
   *src = ptr;
}

static inline void
_jpeg_copy(volatile uint32_t **dst, uint8_t **src, unsigned int w)
{
   uint32_t *ptr2 = (uint32_t*) *dst;
   uint8_t *ptr = *src;
   unsigned int x;

   for (x = 0; x < w; x++)
     {
        *ptr2 = ARGB_JOIN(0xff, ptr[0], ptr[1], ptr[2]);
        ptr += 3;
        ptr2++;
     }

   *dst = ptr2;
   *src = ptr;
}

static Eina_Bool
_emile_jpeg_data(Emile_Image *image,
                 Emile_Image_Property *prop,
                 unsigned int property_size,
                 void *pixels,
                 Emile_Image_Load_Error *error)
{
   /* Handle RGB, ARGB, GRY and AGRY */
   volatile Emile_Image_Load_Opts *opts = (image->load_opts) ? &image->opts : NULL;
   unsigned int w, h;
   struct jpeg_decompress_struct cinfo;
   struct _JPEG_error_mgr jerr;
   const unsigned char *m = NULL;
   uint8_t *ptr, *line[16], *data;
   volatile uint32_t *ptr2 = NULL;
   uint32_t *ptr_rotate = NULL;
   uint16_t *ptrag = NULL, *ptrag_rotate = NULL;
   uint8_t *ptrg = NULL, *ptrg_rotate = NULL;
   unsigned int y, l, i, scans;
   volatile int region = 0;
   /* rotation setting */
   unsigned int ie_w = 0, ie_h = 0;
   struct
   {
      unsigned int x, y, w, h;
   } opts_region = {0, 0, 0, 0};
   volatile int degree = 0;
   volatile Eina_Bool change_wh = EINA_FALSE;
   volatile Eina_Bool line_done = EINA_FALSE;
   volatile Eina_Bool ptrg_free = EINA_FALSE;
   volatile Eina_Bool ptrag_free = EINA_FALSE;
   volatile Eina_Bool r = EINA_FALSE;
   unsigned int length;
   volatile unsigned short count = 0;

   if (sizeof(Emile_Image_Property) != property_size)
     return EINA_FALSE;

   m = _emile_image_file_source_map(image, &length);
   if (!m)
     return EINA_FALSE;

   memset(&cinfo, 0, sizeof(cinfo));
   if (prop->rotated)
     {
        degree = opts ? opts->degree : 0;
        if (degree == 90 || degree == 270)
          change_wh = EINA_TRUE;
     }

   cinfo.err = jpeg_std_error(&(jerr.pub));
   cinfo.client_data = (void *)(intptr_t) 0x1;
   jerr.pub.error_exit = _emile_image_jpeg_error_exit_cb;
   jerr.pub.emit_message = _emile_image_jpeg_emit_message_cb;
   jerr.pub.output_message = _emile_image_jpeg_output_message_cb;
   if (setjmp(jerr.setjmp_buffer))
     {
        *error = EMILE_IMAGE_LOAD_ERROR_CORRUPT_FILE;
        goto on_error;
     }
   jpeg_create_decompress(&cinfo);

   if (_emile_jpeg_membuf_src(&cinfo, m, length))
     {
        *error = EMILE_IMAGE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto on_error;
     }

   jpeg_read_header(&cinfo, TRUE);
   cinfo.do_fancy_upsampling = FALSE;
   cinfo.do_block_smoothing = FALSE;
   cinfo.dct_method = JDCT_ISLOW; /* JDCT_FLOAT JDCT_IFAST(quality loss) */
   cinfo.dither_mode = JDITHER_ORDERED;

   if (prop->scale > 1)
     {
        cinfo.scale_num = 1;
        cinfo.scale_denom = prop->scale;
     }

   /* Colorspace conversion options */
   /* libjpeg can do the following conversions: */
   /* GRAYSCLAE => RGB YCbCr => RGB and YCCK => CMYK */
   switch (cinfo.jpeg_color_space)
     {
      case JCS_UNKNOWN:
        break;

      case JCS_GRAYSCALE:
        if (prop->cspace == EMILE_COLORSPACE_GRY8 ||
            prop->cspace == EMILE_COLORSPACE_AGRY88)
          {
             cinfo.out_color_space = JCS_GRAYSCALE;
             break;
          }
        /* The caller doesn't handle GRAYSCALE,
         * fallback to RGB.
         */
        EINA_FALLTHROUGH;

      case JCS_RGB:
      case JCS_YCbCr:
        cinfo.out_color_space = JCS_RGB;
        break;

      case JCS_CMYK:
      case JCS_YCCK:
        cinfo.out_color_space = JCS_CMYK;
        break;

      default:
        cinfo.out_color_space = JCS_RGB;
        break;
     }

/* head decoding */
   jpeg_calc_output_dimensions(&(cinfo));
   jpeg_start_decompress(&cinfo);

   w = cinfo.output_width;
   h = cinfo.output_height;

   if (change_wh)
     {
        ie_w = prop->h;
        ie_h = prop->w;
     }
   else
     {
        ie_w = prop->w;
        ie_h = prop->h;
     }

   if (opts && ((opts->region.w > 0) && (opts->region.h > 0)))
     {
        region = 1;

         /* scale value already applied when decompress.
            When access to decoded image, have to apply scale value to region value */
        if (prop->scale > 1)
          {
             opts_region.x = opts->region.x / prop->scale;
             opts_region.y = opts->region.y / prop->scale;
             opts_region.w = opts->region.w / prop->scale;
             opts_region.h = opts->region.h / prop->scale;

          }
        else
          {
             opts_region.x = opts->region.x;
             opts_region.y = opts->region.y;
             opts_region.w = opts->region.w;
             opts_region.h = opts->region.h;
          }


        if (prop->rotated)
          {
             unsigned int load_region_x = 0, load_region_y = 0;
             unsigned int load_region_w = 0, load_region_h = 0;

             load_region_x = opts_region.x;
             load_region_y = opts_region.y;
             load_region_w = opts_region.w;
             load_region_h = opts_region.h;
             _rotate_region(&opts_region.x, &opts_region.y,
                            &opts_region.w, &opts_region.h,
                            load_region_x, load_region_y,
                            load_region_w, load_region_h,
                            w, h, degree, prop->flipped);
          }

     }
   if ((!region) && ((w != ie_w) || (h != ie_h)))
     {
        *error = EMILE_IMAGE_LOAD_ERROR_GENERIC;
        goto on_error;
     }
   if ((region) && ((ie_w != opts_region.w) || (ie_h != opts_region.h)))
     {
        *error = EMILE_IMAGE_LOAD_ERROR_GENERIC;
        goto on_error;
     }

   switch (prop->cspace)
     {
      case EMILE_COLORSPACE_GRY8:
      case EMILE_COLORSPACE_AGRY88:
        if (!(cinfo.out_color_space == JCS_GRAYSCALE &&
              cinfo.output_components == 1))
          {
             *error = EMILE_IMAGE_LOAD_ERROR_UNKNOWN_FORMAT;
             goto on_error;
          }
        break;

      case EMILE_COLORSPACE_ARGB8888:
        if (!((cinfo.out_color_space == JCS_RGB && cinfo.output_components == 3) ||
              (cinfo.out_color_space == JCS_CMYK && cinfo.output_components == 4)))
          {
             *error = EMILE_IMAGE_LOAD_ERROR_UNKNOWN_FORMAT;
             goto on_error;
          }
        break;

      default:
        *error = EMILE_IMAGE_LOAD_ERROR_GENERIC;
        goto on_error;
     }

   /* end head decoding */
   /* data decoding */
   if (cinfo.rec_outbuf_height > 16)
     {
        *error = EMILE_IMAGE_LOAD_ERROR_UNKNOWN_FORMAT;
        goto on_error;
     }
   data = alloca(w * 16 * cinfo.output_components);
   if ((prop->rotated) && change_wh)
     {
        if (prop->cspace == EMILE_COLORSPACE_GRY8)
          {
             ptrg = malloc(w * h * sizeof(uint8_t));
             ptrg_rotate = ptrg;
             ptrg_free = EINA_TRUE;
          }
        else if (prop->cspace == EMILE_COLORSPACE_AGRY88)
          {
             ptrag = malloc(w * h * sizeof(uint16_t));
             ptrag_rotate = ptrag;
             ptrag_free = EINA_TRUE;
          }
        else
          {
             ptr2 = malloc(w * h * sizeof(uint32_t));
             ptr_rotate = (uint32_t*) ptr2;
          }
     }
   else
     {
        ptr2 = pixels;
        ptrag = pixels;
        ptrg = pixels;
     }

   if (!ptr2 && !ptrag && !ptrg)
     {
        *error = EMILE_IMAGE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto on_error;
     }

   /* We handle first CMYK (4 components) */
   if (cinfo.output_components == 4)
     {
        // FIXME: handle region
        for (i = 0; (int)i < cinfo.rec_outbuf_height; i++)
          line[i] = data + (i * w * 4);
        for (l = 0; l < h; l += cinfo.rec_outbuf_height)
          {
             // Check for continuing every 16 scanlines fetch
             EMILE_IMAGE_TASK_CHECK(image, count, 0xF, error, on_error);

             jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
             scans = cinfo.rec_outbuf_height;
             if ((h - l) < scans)
               scans = h - l;
             ptr = data;
             if (!region)
               {
                  for (y = 0; y < scans; y++)
                    {
                       _jpeg_convert_copy(&ptr2, &ptr, w, cinfo.saw_Adobe_marker);
                    }
               }
             else
               {
                  /* if line # > region last line, break */
                  if (l >= (opts_region.y + opts_region.h))
                    {
                       line_done = EINA_TRUE;
                       /* if rotation flag is set , we have to rotate image */
                       goto done;
                       /*jpeg_destroy_decompress(&cinfo);
                          _emile_jpeg_membuf_src_term(&cinfo);
                        * error = EMILE_IMAGE_LOAD_ERROR_NONE;
                          return EINA_FALSE; */
                    }
                  /* else if scan block intersects region start or later */
                  else if ((l + scans) > (opts_region.y))
                    {
                       for (y = 0; y < scans; y++)
                         {
                            if (((y + l) >= opts_region.y) && ((y + l) < (opts_region.y + opts_region.h)))
                              {
                                 ptr += opts_region.x;
                                 _jpeg_convert_copy(&ptr2, &ptr, opts_region.w, cinfo.saw_Adobe_marker);
                                 ptr += (4 * (w - (opts_region.x + opts_region.w)));
                              }
                            else
                              ptr += (4 * w);
                         }
                    }
               }
          }
     }
   /* We handle then RGB with 3 components */
   else if (cinfo.output_components == 3)
     {
/*
        double t;
        if (region)
          {
             // debug for now
             printf("R| %p %5ix%5i %s: %5i %5i %5ix%5i - ",
                    ie,
                    ie->w, ie->h,
                    ie->file,
                    opts_region.x,
                    opts_region.y,
                    opts_region.w,
                    opts_region.h);
          }
        t = get_time();
 */
        for (i = 0; (int)i < cinfo.rec_outbuf_height; i++)
          line[i] = data + (i * w * 3);
        for (l = 0; l < h; l += cinfo.rec_outbuf_height)
          {
             // Check for continuing every 16 scanlines fetch
             EMILE_IMAGE_TASK_CHECK(image, count, 0xF, error, on_error);

             jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
             scans = cinfo.rec_outbuf_height;
             if ((h - l) < scans)
               scans = h - l;
             ptr = data;
             if (!region)
               {
                  for (y = 0; y < scans; y++)
                    {
                       _jpeg_copy(&ptr2, &ptr, w);
                    }
               }
             else
               {
                  /* if line # > region last line, break
                     but not return immediately for rotation job */
                  if (l >= (opts_region.y + opts_region.h))
                    {
                       line_done = EINA_TRUE;
                       /* if rotation flag is set , we have to rotate image */
                       goto done;
                    }
                  /* else if scan block intersects region start or later */
                  else if ((l + scans) > (opts_region.y))
                    {
                       for (y = 0; y < scans; y++)
                         {
                            if (((y + l) >= opts_region.y) &&
                                ((y + l) < (opts_region.y + opts_region.h)))
                              {
                                 ptr += (3 * opts_region.x);
                                 _jpeg_copy(&ptr2, &ptr, opts_region.w);
                                 ptr += (3 * (w - (opts_region.x + opts_region.w)));
                              }
                            else
                              ptr += (3 * w);
                         }
                    }
               }
          }
/*
        t = get_time() - t;
        printf("%3.3f\n", t);
 */
     }
   /* We finally handle RGB with 1 component */
   else if (cinfo.output_components == 1)
     {
        for (i = 0; (int)i < cinfo.rec_outbuf_height; i++)
          line[i] = data + (i * w);
        for (l = 0; l < h; l += cinfo.rec_outbuf_height)
          {
             // Check for continuing every 16 scanlines fetch
             EMILE_IMAGE_TASK_CHECK(image, count, 0xF, error, on_error);

             jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
             scans = cinfo.rec_outbuf_height;
             if ((h - l) < scans)
               scans = h - l;
             ptr = data;
             if (!region)
               {
                  for (y = 0; y < scans; y++)
                    {
                       switch (prop->cspace)
                         {
                          case EMILE_COLORSPACE_GRY8:
                             _jpeg_gry8_convert_copy(&ptrg, &ptr, w);
                             break;
                          case EMILE_COLORSPACE_AGRY88:
                             _jpeg_agry88_convert_copy(&ptrag, &ptr, w);
                             break;
                          default:
                             _jpeg_argb8888_convert_copy(&ptr2, &ptr, w);
                             break;
                         }
                    }
               }
             else
               {
                  /* if line # > region last line, break */
                  if (l >= (opts_region.y + opts_region.h))
                    {
                       line_done = EINA_TRUE;
                       /* if rotation flag is set , we have to rotate image */
                       goto done;
                       /*jpeg_destroy_decompress(&cinfo);
                          _emile_jpeg_membuf_src_term(&cinfo);
                        * error = EMILE_IMAGE_LOAD_ERROR_NONE;
                          return EINA_TRUE; */
                    }
                  /* else if scan block intersects region start or later */
                  else if ((l + scans) > (opts_region.y))
                    {
                       for (y = 0; y < scans; y++)
                         {
                            if (((y + l) >= opts_region.y) &&
                                ((y + l) < (opts_region.y + opts_region.h)))
                              {
                                 ptr += opts_region.x;
                                 switch (prop->cspace)
                                   {
                                    case EMILE_COLORSPACE_GRY8:
                                       _jpeg_gry8_convert_copy(&ptrg, &ptr, opts_region.w);
                                       break;
                                    case EMILE_COLORSPACE_AGRY88:
                                       _jpeg_agry88_convert_copy(&ptrag, &ptr, opts_region.w);
                                       break;
                                    default:
                                       _jpeg_argb8888_convert_copy(&ptr2, &ptr, opts_region.w);
                                       break;
                                   }
                                 ptr += w - (opts_region.x + opts_region.w);
                              }
                            else
                              ptr += w;
                         }
                    }
               }
          }
     }
   /* if rotation operation need, rotate it */
done:

   if (prop->rotated)
     {
        uint32_t *to;
        uint8_t *to8;
        uint16_t *to16;
        int hw;

        hw = ie_w * ie_h;
        to = pixels;
        to8 = pixels;
        to16 = pixels;

        switch (degree)
          {
           case 90:
             if (prop->cspace == EMILE_COLORSPACE_GRY8)
               {
                  if (prop->flipped)
                    _rotate_change_wh8(to8 + hw - 1, ptrg_rotate, ie_w, ie_h, hw - 1, -ie_h);
                  else
                    _rotate_change_wh8(to8 + ie_h - 1, ptrg_rotate, ie_w, ie_h, -hw - 1, ie_h);
               }
             else if (prop->cspace == EMILE_COLORSPACE_AGRY88)
               {
                  if (prop->flipped)
                    _rotate_change_wh16(to16 + hw - 1, ptrag_rotate, ie_w, ie_h, hw - 1, -ie_h);
                  else
                    _rotate_change_wh16(to16 + ie_h - 1, ptrag_rotate, ie_w, ie_h, -hw - 1, ie_h);
               }
             else
               {
                  if (prop->flipped)
                    _rotate_change_wh(to + hw - 1, ptr_rotate, ie_w, ie_h, hw - 1, -ie_h);
                  else
                    _rotate_change_wh(to + ie_h - 1, ptr_rotate, ie_w, ie_h, -hw - 1, ie_h);
               }
             break;

           case 180:
             if (prop->cspace == EMILE_COLORSPACE_GRY8)
               {
                  if (prop->flipped)
                    _flip_vertical8(to8, ie_w, ie_h);
                  else
                    _rotate8_180(to8, ie_w, ie_h);
               }
             else if (prop->cspace == EMILE_COLORSPACE_AGRY88)
               {
                  if (prop->flipped)
                    _flip_vertical16(to16, ie_w, ie_h);
                  else
                    _rotate16_180(to16, ie_w, ie_h);
               }
             else
               {
                  if (prop->flipped)
                    _flip_vertical(to, ie_w, ie_h);
                  else
                    _rotate_180(to, ie_w, ie_h);
               }
             break;

           case 270:
             if (prop->cspace == EMILE_COLORSPACE_GRY8)
               {
                  if (prop->flipped)
                    _rotate_change_wh8(to8, ptrg_rotate, ie_w, ie_h, -hw + 1, ie_h);
                  else
                    _rotate_change_wh8(to8 + hw - ie_h, ptrg_rotate, ie_w, ie_h, hw + 1, -ie_h);
               }
             else if (prop->cspace == EMILE_COLORSPACE_AGRY88)
               {
                  if (prop->flipped)
                    _rotate_change_wh16(to16, ptrag_rotate, w, h, -hw + 1, h);
                  else
                    _rotate_change_wh16(to16 + hw - ie_h, ptrag_rotate, ie_w, ie_h, hw + 1, -ie_h);
               }
             else
               {
                  if (prop->flipped)
                    _rotate_change_wh(to, ptr_rotate, ie_w, ie_h, -hw + 1, ie_h);
                  else
                    _rotate_change_wh(to + hw - ie_h, ptr_rotate, ie_w, ie_h, hw + 1, -ie_h);
               }
             break;

           default:
             if (prop->flipped)
               {
                  if (prop->cspace == EMILE_COLORSPACE_GRY8)
                    _flip_horizontal8(to8, ie_w, ie_h);
                  else if (prop->cspace == EMILE_COLORSPACE_AGRY88)
                    _flip_horizontal16(to16, ie_w, ie_h);
                  else
                    _flip_horizontal(to, ie_w, ie_h);
               }
             break;
          }
        if (ptr_rotate)
          {
             free(ptr_rotate);
             ptr_rotate = NULL;
          }
     }

   if (line_done)
     {
        *error = EMILE_IMAGE_LOAD_ERROR_NONE;
        goto on_error;
     }
   /* end data decoding */
   jpeg_finish_decompress(&cinfo);
   *error = EMILE_IMAGE_LOAD_ERROR_NONE;
   r = EINA_TRUE;

 on_error:
   if (ptrg_free) free(ptrg);
   if (ptrag_free) free(ptrag);

   jpeg_destroy_decompress(&cinfo);
   _emile_jpeg_membuf_src_term(&cinfo);
   _emile_image_file_source_unmap(image);
   return r;

}

static void
_emile_jpeg_close(Emile_Image *image EINA_UNUSED)
{
   /* JPEG file loader doesn't keep any data allocated around (for now) */
}

/* Generic helper to instantiate a new Emile_Image */

static Emile_Image *
_emile_image_new(Eina_Bool (*bind)(Emile_Image *image, Emile_Image_Load_Opts *opts, Emile_Image_Animated *animated, Emile_Image_Load_Error *error),
                 Eina_Bool (*head)(Emile_Image *image, Emile_Image_Property *prop, unsigned int property_size, Emile_Image_Load_Error *error),
                 Eina_Bool (*data)(Emile_Image *image, Emile_Image_Property *prop, unsigned int property_size, void *pixels, Emile_Image_Load_Error *error),
                 void (*close)(Emile_Image *image))
{
   Emile_Image *ei;

   ei = calloc(1, sizeof(Emile_Image));
   if (!ei)
     return NULL;

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
        ei->opts.orientation = !!ei->opts.orientation;
        ei->load_opts = 1;
     }

   *error = EMILE_IMAGE_LOAD_ERROR_NONE;
   if (ei->bind(ei, opts, animated, error))
     return ei;

   /* File is not of that format */
   if (!ei->bin_source)
     eina_file_close(ei->source.f);
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
   if (!ei)
     return NULL;

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
   if (!ei)
     return NULL;

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
   if (!ei)
     return NULL;

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
   if (!ei)
     return NULL;

   _emile_image_file_set(ei, source);
   return _emile_image_bind(ei, opts, animated, error);
}

EAPI void
emile_image_callback_set(Emile_Image *image, Emile_Action_Cb callback, Emile_Action action, const void *data)
{
   if (!image) return ;
   // We only handle one type of callback for now
   if (action != EMILE_ACTION_CANCELLED) return ;

   image->cancelled_data = data;
   image->cancelled = callback;
}

EAPI void
emile_image_close(Emile_Image *image)
{
   if (!image)
     return;

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
   if (!image)
     return EINA_FALSE;

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
   if (!image)
     return EINA_FALSE;

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

      case EMILE_IMAGE_LOAD_ERROR_CANCELLED:
        return "Loading was stopped by an external request.";
     }
   return NULL;
}

#include "evas_common_private.h"
#include "evas_private.h"

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef _WIN32
# include <winsock2.h>
#endif /* ifdef _WIN32 */

#include "lz4.h"
#include "lz4hc.h"
#include "rg_etc1.h"

// FIXME: Remove DEBUG
#define DEBUG
#if defined(DEBUG) && defined(HAVE_CLOCK_GETTIME) && defined(_POSIX_MONOTONIC_CLOCK)
# include <time.h>
# define DEBUG_STATS
#endif

static int
_block_size_get(int size)
{
   static const int MAX_BLOCK = 6; // 256 pixels

   int k = 0;
   while ((4 << k) < size) k++;
   k = MAX(0, k - 1);
   if ((size * 3 / 2) >= (4 << k)) return MAX(0, MIN(k - 1, MAX_BLOCK));
   return MIN(k, MAX_BLOCK);
}

static inline void
_alpha_to_greyscale_convert(uint32_t *data, int len)
{
   for (int k = 0; k < len; k++)
     {
        int alpha = A_VAL(data);
        *data++ = ARGB_JOIN(alpha, alpha, alpha, alpha);
     }
}

static int
_save_direct_tgv(RGBA_Image *im, const char *file, int compress)
{
   // FIXME: Now we have border information, this comment isn't valid anymore:

   // In case we are directly copying ETC1/2 data, we can't properly
   // duplicate the 1 pixel borders. So we just assume the image contains
   // them already.

   // TODO: Add block by block compression.

   int image_width, image_height, planes = 1;
   uint32_t width, height;
   uint8_t header[8] = "TGV1";
   int etc_block_size, etc_data_size, buffer_size, data_size, remain;
   uint8_t *buffer = NULL;
   uint8_t *data, *ptr;
   FILE *f;

   image_width = im->cache_entry.w;
   image_height = im->cache_entry.h;
   data = im->image.data8;
   width = htonl(image_width);
   height = htonl(image_height);
   compress = !!compress;

   if ((image_width & 0x3) || (image_height & 0x3))
     return 0;

   // header[4]: block size info, unused
   header[4] = 0;

   // header[5]: 0 for ETC1
   switch (im->cache_entry.space)
     {
      case EVAS_COLORSPACE_ETC1:
        etc_block_size = 8;
        header[5] = 0;
        break;
      case EVAS_COLORSPACE_RGB8_ETC2:
        etc_block_size = 8;
        header[5] = 1;
        break;
      case EVAS_COLORSPACE_RGBA8_ETC2_EAC:
        etc_block_size = 16;
        header[5] = 2;
        break;
      case EVAS_COLORSPACE_ETC1_ALPHA:
        // FIXME: Properly handle premul vs. unpremul data
        etc_block_size = 8;
        header[5] = 3;
        planes = 2;
        break;
      default:
        return 0;
     }

   // header[6]: 0 for raw, 1, for LZ4 compressed, 2 for block-less mode
   header[6] = compress | 0x2;

   // header[7]: options (unused)
   header[7] = 0;

   f = fopen(file, "w");
   if (!f) return 0;

   // Write header
   if (fwrite(header, sizeof (uint8_t), 8, f) != 8) goto on_error;
   if (fwrite(&width, sizeof (uint32_t), 1, f) != 1) goto on_error;
   if (fwrite(&height, sizeof (uint32_t), 1, f) != 1) goto on_error;

   etc_data_size = image_width * image_height * etc_block_size * planes / 16;
   if (compress)
     {
        buffer_size = LZ4_compressBound(etc_data_size);
        buffer = malloc(buffer_size);
        if (!buffer) goto on_error;
        data_size = LZ4_compressHC((char *) data, (char *) buffer, etc_data_size);
     }
   else
     {
        data_size = buffer_size = etc_data_size;
        buffer = data;
     }

   // Write block length header -- We keep this even in block-less mode
   if (data_size > 0)
     {
        unsigned int blen = data_size;

        while (blen)
          {
             unsigned char plen;

             plen = blen & 0x7F;
             blen = blen >> 7;

             if (blen) plen = 0x80 | plen;
             if (fwrite(&plen, 1, 1, f) != 1) goto on_error;
          }
     }

   // Write data
   ptr = buffer;
   remain = data_size;
   while (remain > 0)
     {
        int written = fwrite(ptr, 1, remain, f);
        if (written < 0) goto on_error;
        remain -= written;
        ptr += written;
     }

   if (compress) free(buffer);
   fclose(f);
   return 1;

on_error:
   if (compress) free(buffer);
   fclose(f);
   return 0;
}

static int
evas_image_save_file_tgv(RGBA_Image *im,
                         const char *file, const char *key EINA_UNUSED,
                         int quality, int compress, const char *encoding)
{
   rg_etc1_pack_params param;
   FILE *f;
   uint8_t *comp = NULL;
   uint8_t *buffer;
   uint32_t *data = NULL;
   uint32_t nl_width, nl_height;
   uint8_t header[8] = "TGV1";
   int block_width, block_height, macro_block_width, macro_block_height;
   int block_count, image_stride, image_height, etc_block_size;
   Evas_Colorspace cspace;
   Eina_Bool alpha, alpha_texture = EINA_FALSE, unpremul = EINA_FALSE;
   int num_planes = 1;

#ifdef DEBUG_STATS
   struct timespec ts1, ts2;
   long long tsdiff, mse = 0, mse_div = 0, mse_alpha = 0;
   clock_gettime(CLOCK_MONOTONIC, &ts1);
#endif

   if (!im || !im->image.data || !file)
     return 0;

   switch (im->cache_entry.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
        alpha = im->cache_entry.flags.alpha;
        break;
      case EVAS_COLORSPACE_ETC1:
      case EVAS_COLORSPACE_RGB8_ETC2:
      case EVAS_COLORSPACE_RGBA8_ETC2_EAC:
      case EVAS_COLORSPACE_ETC1_ALPHA:
        // Note: This case should probably never happen
        if (encoding)
          WRN("Ignoring 'encoding' argument the data is already ETC1/2");
        return _save_direct_tgv(im, file, compress);
      default:
        return 0;
     }

   image_stride = im->cache_entry.w;
   image_height = im->cache_entry.h;
   nl_width = htonl(image_stride);
   nl_height = htonl(image_height);
   compress = !!compress;

   // Disable dithering, as it will deteriorate the quality of flat surfaces
   param.m_dithering = 0;

   if (quality > 95)
     param.m_quality = rg_etc1_high_quality;
   else if (quality > 30)
     param.m_quality = rg_etc1_medium_quality;
   else
     param.m_quality = rg_etc1_low_quality;

   // header[4]: 4 bit block width, 4 bit block height
   block_width = _block_size_get(image_stride + 2);
   block_height = _block_size_get(image_height + 2);
   header[4] = (block_height << 4) | block_width;

   // header[5]: 0 for ETC1, 1 for RGB8_ETC2, 2 for RGBA8_ETC2_EAC, 3 for ETC1+Alpha
   if (!encoding) encoding = "etc2";
   if (!strcasecmp(encoding, "etc1"))
     {
        if (alpha)
          {
             ERR("ETC1 does not support alpha encoding. Abort. "
                 "Please use 'encoding=etc1+alpha' for ETC1+Alpha encoding.");
             return 0;
          }
        cspace = EVAS_COLORSPACE_ETC1;
        etc_block_size = 8;
        header[5] = 0;
     }
   else if (!strcasecmp(encoding, "etc2"))
     {
        if (!alpha)
          {
             cspace = EVAS_COLORSPACE_RGB8_ETC2;
             etc_block_size = 8;
             header[5] = 1;
          }
        else
          {
             cspace = EVAS_COLORSPACE_RGBA8_ETC2_EAC;
             etc_block_size = 16;
             header[5] = 2;
          }
     }
   else if (!strcasecmp(encoding, "etc1+alpha"))
     {
        if (!alpha)
          {
             INF("Selected etc1+alpha but image has no alpha, reverting to etc1.");
             cspace = EVAS_COLORSPACE_ETC1;
             etc_block_size = 8;
             header[5] = 0;
          }
        else
          {
             // Save as two textures, and unpremul the data
             alpha_texture = EINA_TRUE;
             unpremul = EINA_TRUE;
             num_planes = 2; // RGB and Alpha
             cspace = EVAS_COLORSPACE_ETC1_ALPHA;
             etc_block_size = 8;
             header[5] = 3;
          }
     }
   else
     {
        ERR("Unknown encoding '%.8s' selected. Abort.", encoding);
        return 0;
     }

   // header[6]: 0 for raw, 1, for LZ4 compressed, 4 for unpremultiplied RGBA
   // blockless mode (0x2) is never used here
   header[6] = (compress ? 0x1 : 0x0) | (unpremul ? 0x4 : 0x0);

   // header[7]: unused options
   // Note: consider extending the header instead of filling all the bits here
   header[7] = 0;

   f = fopen(file, "w");
   if (!f) return 0;

   // Write header
   if (fwrite(header, sizeof (uint8_t), 8, f) != 8) goto on_error;
   if (fwrite(&nl_width, sizeof (uint32_t), 1, f) != 1) goto on_error;
   if (fwrite(&nl_height, sizeof (uint32_t), 1, f) != 1) goto on_error;

   // Real block size in pixels, obviously a multiple of 4
   macro_block_width = 4 << block_width;
   macro_block_height = 4 << block_height;

   // Number of ETC1 blocks in a compressed block
   block_count = (macro_block_width * macro_block_height) / (4 * 4);
   buffer = alloca(block_count * etc_block_size);

   if (compress)
     comp = alloca(LZ4_compressBound(block_count * etc_block_size));

   // Write a whole plane (RGB or Alpha)
   for (int plane = 0; plane < num_planes; plane++)
     {
        if (!alpha_texture)
          {
             // Normal mode
             data = im->image.data;
          }
        else if (!plane)
          {
             int len = image_stride * image_height;
             // RGB plane for ETC1+Alpha
             data = malloc(len * 4);
             if (!data) goto on_error;
             memcpy(data, im->image.data, len * 4);
             if (unpremul) evas_data_argb_unpremul(data, len);
          }
        else
          {
             // Alpha plane for ETC1+Alpha
             _alpha_to_greyscale_convert(data, image_stride * image_height);
          }

        // Write macro block
        for (int y = 0; y < image_height + 2; y += macro_block_height)
          {
             uint32_t *input, *last_col, *last_row, *last_pix;
             int real_y;
             int wlen;

             if (y == 0) real_y = 0;
             else if (y < image_height + 1) real_y = y - 1;
             else real_y = image_height - 1;

             for (int x = 0; x < image_stride + 2; x += macro_block_width)
               {
                  uint8_t *offset = buffer;
                  int real_x = x;

                  if (x == 0) real_x = 0;
                  else if (x < image_stride + 1) real_x = x - 1;
                  else real_x = image_stride - 1;

                  input = data + real_y * image_stride + real_x;
                  last_row = data + image_stride * (image_height - 1) + real_x;
                  last_col = data + (real_y + 1) * image_stride - 1;
                  last_pix = data + image_height * image_stride - 1;

                  for (int by = 0; by < macro_block_height; by += 4)
                    {
                       int dup_top = ((y + by) == 0) ? 1 : 0;
                       int max_row = MAX(0, MIN(4, image_height - real_y - by));
                       int oy = (y == 0) ? 1 : 0;

                       for (int bx = 0; bx < macro_block_width; bx += 4)
                         {
                            int dup_left = ((x + bx) == 0) ? 1 : 0;
                            int max_col = MAX(0, MIN(4, image_stride - real_x - bx));
                            uint32_t todo[16] = { 0 };
                            int row, col;
                            int ox = (x == 0) ? 1 : 0;

                            if (dup_left)
                              {
                                 // Duplicate left column
                                 for (row = 0; row < max_row; row++)
                                   todo[row * 4] = input[row * image_stride];
                                 for (row = max_row; row < 4; row++)
                                   todo[row * 4] = last_row[0];
                              }

                            if (dup_top)
                              {
                                 // Duplicate top row
                                 for (col = 0; col < max_col; col++)
                                   todo[col] = input[MAX(col + bx - ox, 0)];
                                 for (col = max_col; col < 4; col++)
                                   todo[col] = last_col[0];
                              }

                            for (row = dup_top; row < 4; row++)
                              {
                                 for (col = dup_left; col < max_col; col++)
                                   {
                                      if (row < max_row)
                                        {
                                           // Normal copy
                                           todo[row * 4 + col] = input[(row + by - oy) * image_stride + bx + col - ox];
                                        }
                                      else
                                        {
                                           // Copy last line
                                           todo[row * 4 + col] = last_row[col + bx - ox];
                                        }
                                   }
                                 for (col = max_col; col < 4; col++)
                                   {
                                      // Right edge
                                      if (row < max_row)
                                        {
                                           // Duplicate last column
                                           todo[row * 4 + col] = last_col[MAX(row + by - oy, 0) * image_stride];
                                        }
                                      else
                                        {
                                           // Duplicate very last pixel again and again
                                           todo[row * 4 + col] = *last_pix;
                                        }
                                   }
                              }

                            switch (cspace)
                              {
                               case EVAS_COLORSPACE_ETC1:
                               case EVAS_COLORSPACE_ETC1_ALPHA:
                                 rg_etc1_pack_block(offset, (uint32_t *) todo, &param);
                                 break;
                               case EVAS_COLORSPACE_RGB8_ETC2:
                                 etc2_rgb8_block_pack(offset, (uint32_t *) todo, &param);
                                 break;
                               case EVAS_COLORSPACE_RGBA8_ETC2_EAC:
                                 etc2_rgba8_block_pack(offset, (uint32_t *) todo, &param);
                                 break;
                               default: return 0;
                              }

#ifdef DEBUG_STATS
                            if (plane == 0)
                              {
                                 // Decode to compute PSNR, this is slow.
                                 uint32_t done[16];

                                 if (alpha)
                                   rg_etc2_rgba8_decode_block(offset, done);
                                 else
                                    rg_etc2_rgb8_decode_block(offset, done);

                                 for (int k = 0; k < 16; k++)
                                   {
                                      const int r = (R_VAL(&(todo[k])) - R_VAL(&(done[k])));
                                      const int g = (G_VAL(&(todo[k])) - G_VAL(&(done[k])));
                                      const int b = (B_VAL(&(todo[k])) - B_VAL(&(done[k])));
                                      const int a = (A_VAL(&(todo[k])) - A_VAL(&(done[k])));
                                      mse += r*r + g*g + b*b;
                                      if (alpha) mse_alpha += a*a;
                                      mse_div++;
                                   }
                              }
#endif

                            offset += etc_block_size;
                         }
                    }

                  if (compress)
                    {
                       wlen = LZ4_compressHC((char *) buffer, (char *) comp,
                                             block_count * etc_block_size);
                    }
                  else
                    {
                       comp = buffer;
                       wlen = block_count * etc_block_size;
                    }

                  if (wlen > 0)
                    {
                       unsigned int blen = wlen;

                       while (blen)
                         {
                            unsigned char plen;

                            plen = blen & 0x7F;
                            blen = blen >> 7;

                            if (blen) plen = 0x80 | plen;
                            if (fwrite(&plen, 1, 1, f) != 1) goto on_error;
                         }
                       if (fwrite(comp, wlen, 1, f) != 1) goto on_error;
                    }
               } // 4 rows
          } // macroblocks
     } // planes
   fclose(f);

#ifdef DEBUG_STATS
   if (mse_div && mse)
     {
        // TODO: Add DSSIM too
        double dmse = (double) mse / (double) (mse_div * 3.0);
        double psnr = 20 * log10(255.0) - 10 * log10(dmse);
        double dmse_alpha = (double) mse_alpha / (double) mse_div;
        double psnr_alpha = (dmse_alpha > 0.0) ? (20 * log10(255.0) - 10 * log10(dmse_alpha)) : 0;
        clock_gettime(CLOCK_MONOTONIC, &ts2);
        tsdiff = ((ts2.tv_sec - ts1.tv_sec) * 1000LL) + ((ts2.tv_nsec - ts1.tv_nsec) / 1000000LL);
        if (!alpha)
          {
             INF("ETC%d encoding stats: %dx%d, Time: %lldms, RGB PSNR: %.02fdB",
                 alpha ? 2 : 1, image_stride, image_height, tsdiff, psnr);
          }
        else
          {
             INF("ETC%d encoding stats: %dx%d, Time: %lldms, RGB PSNR: %.02fdB, Alpha PSNR: %.02fdB",
                 alpha ? 2 : 1, image_stride, image_height, tsdiff, psnr, psnr_alpha);
          }
     }
#endif

   if (alpha_texture) free(data);
   return 1;

on_error:
   if (alpha_texture) free(data);
   fclose(f);
   return 0;
}

static Evas_Image_Save_Func evas_image_save_tgv_func =
{
   evas_image_save_file_tgv
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_save_tgv_func);

   rg_etc1_pack_block_init();
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

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_SAVER, image_saver, tgv);

#ifndef EVAS_STATIC_BUILD_TGV
EVAS_EINA_MODULE_DEFINE(image_saver, tgv);
#endif

#include "evas_common_private.h"
#include "evas_private.h"

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef _WIN32
# include <winsock2.h>
#endif /* ifdef _WIN32 */

#include "lz4.h"
#include "rg_etc1.h"

static int
evas_image_save_file_tgv(RGBA_Image *im,
                         const char *file, const char *key EINA_UNUSED,
                         int quality, int compress)
{
   rg_etc1_pack_params param;
   FILE *f;
   char *comp;
   char *buffer;
   uint32_t *data;
   uint32_t width, height;
   uint8_t header[8] = "TGV1";
   unsigned int block;
   unsigned int x, y;
   unsigned int compress_length;
   unsigned int block_count;

   if (!im || !im->image.data || !file)
     return 0;

   // Surface with alpha are not supported
   if (im->cache_entry.flags.alpha)
     return 0;

   data = im->image.data;
   width = htonl(im->cache_entry.w);
   height = htonl(im->cache_entry.h);

   param.m_dithering = 1;
   if (quality > 70)
     {
        param.m_quality = rg_etc1_high_quality;
        block = 7;
     }
   else if (quality < 30)
     {
        param.m_quality = rg_etc1_medium_quality;
        block = 6;
     }
   else
     {
        param.m_quality = rg_etc1_low_quality;
        block = 5;
     }

   header[4] = (block << 4) | block;
   header[5] = 0;
   header[6] = (!!compress & 0x1);
   header[7] = 0;

   f = fopen(file, "w");
   if (!f) return 0;

   // Write header
   fwrite(header, sizeof (uint8_t), 8, f);
   fwrite(&width, sizeof (uint32_t), 1, f);
   fwrite(&height, sizeof (uint32_t), 1, f);

   block = 4 << block;

   block_count = (block * block) / (4 * 4);
   buffer = alloca(block_count * 8);

   if (compress)
     {
        compress_length = LZ4_compressBound(block_count * 8);
        comp = alloca(compress_length);
     }
   else
     {
        comp = NULL;
     }

   // Write block
   for (y = 0; y < im->cache_entry.h; y += block)
     for (x = 0; x < im->cache_entry.w; x += block)
       {
          unsigned int i, j;
          int wlen;
          char *offset = buffer;

          for (i = 0; i < block; i += 4)
            for (j = 0; j < block; j += 4)
              {
                 unsigned char todo[64] = { 0 };
                 int k, kmax, lmax;

                 kmax = y + i + 4 < im->cache_entry.h ?
                   4 : im->cache_entry.h - y - i - 1;
                 lmax = x + j + 4 < im->cache_entry.w ?
                   4 : im->cache_entry.w - x - j - 1;

                 if (lmax > 0)
                   {
                      for (k = 0; k < kmax; k++)
                        memcpy(&todo[k * 16],
                               &data[(y + i + k) * im->cache_entry.w + x + j],
                               4 * lmax);
                   }

                 rg_etc1_pack_block(offset, (unsigned int*) todo, &param);
                 offset += 8;
              }

          if (compress)
            {
               wlen = LZ4_compress(buffer, comp, block_count * 8);
            }
          else
            {
               comp = buffer;
               wlen = block_count * 8;
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
                    fwrite(&plen, 1, 1, f);
                 }
               fwrite(comp, wlen, 1, f);
            }
       }
   fclose(f);

   return 1;
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

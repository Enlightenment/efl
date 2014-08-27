#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "evas_common_private.h"
#include "evas_private.h"

/* TGA pixel formats */
#define TGA_TYPE_MAPPED      1 // handle
#define TGA_TYPE_COLOR       2
#define TGA_TYPE_GRAY        3
#define TGA_TYPE_MAPPED_RLE  9 // handle
#define TGA_TYPE_COLOR_RLE  10
#define TGA_TYPE_GRAY_RLE   11

/* TGA header flags */
#define TGA_DESC_ABITS      0x0f
#define TGA_DESC_HORIZONTAL 0x10
#define TGA_DESC_VERTICAL   0x20

#define TGA_SIGNATURE "TRUEVISION-XFILE"

typedef struct _tga_header tga_header;
typedef struct _tga_footer tga_footer;

struct _tga_header
{
   unsigned char       idLength;
   unsigned char       colorMapType;
   unsigned char       imageType;
   unsigned char       colorMapIndexLo, colorMapIndexHi;
   unsigned char       colorMapLengthLo, colorMapLengthHi;
   unsigned char       colorMapSize;
   unsigned char       xOriginLo, xOriginHi;
   unsigned char       yOriginLo, yOriginHi;
   unsigned char       widthLo, widthHi;
   unsigned char       heightLo, heightHi;
   unsigned char       bpp;
   unsigned char       descriptor;
} __attribute__((packed));

struct _tga_footer
{
   unsigned int        extensionAreaOffset;
   unsigned int        developerDirectoryOffset;
   char                signature[16];
   char                dot;
   char                null;
} __attribute__((packed));

static void *
evas_image_load_file_open_tga(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
                              Evas_Image_Load_Opts *opts EINA_UNUSED,
                              Evas_Image_Animated *animated EINA_UNUSED,
                              int *error EINA_UNUSED)
{
   return f;
}

static void
evas_image_load_file_close_tga(void *loader_data EINA_UNUSED)
{
}

static Eina_Bool
evas_image_load_file_head_tga(void *loader_data,
                              Evas_Image_Property *prop,
                              int *error)
{
   Eina_File *f = loader_data;
   unsigned char *seg = NULL, *filedata;
   tga_header *header;
   tga_footer *footer, tfooter;
   char hasa = 0;
   int w, h, bpp;
   int x, y;
   Eina_Bool r = EINA_FALSE;

   *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
   if (eina_file_size_get(f) < (off_t)(sizeof(tga_header) + sizeof(tga_footer)))
      goto close_file;
   seg = eina_file_map_all(f, EINA_FILE_RANDOM);
   if (seg == NULL) goto close_file;
   filedata = seg;

   header = (tga_header *)filedata;
   // no unaligned data accessed, so ok
   footer = (tga_footer *)(filedata + (eina_file_size_get(f) - sizeof(tga_footer)));
   memcpy((unsigned char *)(&tfooter),
          (unsigned char *)footer,
          sizeof(tga_footer));
   //printf("0\n");
   if (!memcmp(tfooter.signature, TGA_SIGNATURE, sizeof(tfooter.signature)))
     {
        if ((tfooter.dot == '.') && (tfooter.null == 0))
          {
             // footer is there and matches. this is a tga file - any problems now
             // are a corrupt file
             *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
          }
     }
//   else goto close_file;
   //printf("1\n");

   filedata = (unsigned char *)filedata + sizeof(tga_header);
   switch (header->imageType)
     {
     case TGA_TYPE_COLOR_RLE:
     case TGA_TYPE_GRAY_RLE:
//        rle = 1;
        break;
     case TGA_TYPE_COLOR:
     case TGA_TYPE_GRAY:
//        rle = 0;
        break;
     default:
        goto close_file;
     }
   bpp = header->bpp;
   if (!((bpp == 32) || (bpp == 24) || (bpp == 16) || (bpp == 8)))
     goto close_file;
   if ((bpp == 32) && (header->descriptor & TGA_DESC_ABITS)) hasa = 1;
   if ((bpp == 16) && (header->descriptor & TGA_DESC_ABITS)) hasa = 1;
   // don't handle colormapped images
   if ((header->colorMapType) != 0)
     goto close_file;
   // if colormap size is anything other than legal sizes or 0 - not real tga
   if (!((header->colorMapSize == 0) ||
         (header->colorMapSize == 15) ||
         (header->colorMapSize == 16) ||
         (header->colorMapSize == 24) ||
         (header->colorMapSize == 32)))
     goto close_file;
   x = (header->xOriginHi << 8) | (header->xOriginLo);
   y = (header->yOriginHi << 8) | (header->yOriginLo);
   w = (header->widthHi << 8) | header->widthLo;
   h = (header->heightHi << 8) | header->heightLo;
   // x origin gerater that width, y origin greater than height - wrong file
   if ((x >= w) || (y >= h))
     goto close_file;
   // if descriptor has either of the top 2 bits set... not tga
   if (header->descriptor & 0xc0)
     goto close_file;

   if ((w < 1) || (h < 1) || (w > IMG_MAX_SIZE) || (h > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(w, h))
     goto close_file;

   prop->w = w;
   prop->h = h;
   if (hasa) prop->alpha = 1;

   *error = EVAS_LOAD_ERROR_NONE;
   r = EINA_TRUE;

close_file:
   if (seg != NULL) eina_file_map_free(f, seg);
   return r;
}

static Eina_Bool
evas_image_load_file_data_tga(void *loader_data,
                              Evas_Image_Property *prop,
                              void *pixels,
                              int *error)
{
   Eina_File *f = loader_data;
   unsigned char *seg = NULL, *filedata;
   tga_header *header;
   tga_footer *footer, tfooter;
   char hasa = 0, footer_present = 0, vinverted = 0, rle = 0;
   int w = 0, h = 0, x, y, bpp;
   off_t size;
   unsigned int *surface, *dataptr;
   unsigned int  datasize;
   unsigned char *bufptr, *bufend;
   int abits;
   Eina_Bool res = EINA_FALSE;

   *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
   if (eina_file_size_get(f) < (off_t)(sizeof(tga_header) + sizeof(tga_footer)))
      goto close_file;
   seg = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   if (seg == NULL) goto close_file;
   filedata = seg;
   size = eina_file_size_get(f);

   header = (tga_header *)filedata;
   // no unaligned data accessed, so ok
   footer = (tga_footer *)(filedata + (size - sizeof(tga_footer)));
   memcpy((unsigned char *)&tfooter,
          (unsigned char *)footer,
          sizeof(tga_footer));
   if (!memcmp(tfooter.signature, TGA_SIGNATURE, sizeof(tfooter.signature)))
     {
        if ((tfooter.dot == '.') && (tfooter.null == 0))
          {
             // footer is there and matches. this is a tga file - any problems now
             // are a corrupt file
             *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
             footer_present = 1;
          }
     }

   filedata = (unsigned char *)filedata + sizeof(tga_header);
   vinverted = !(header->descriptor & TGA_DESC_VERTICAL);
   switch (header->imageType)
     {
     case TGA_TYPE_COLOR_RLE:
     case TGA_TYPE_GRAY_RLE:
        rle = 1;
        break;
     case TGA_TYPE_COLOR:
     case TGA_TYPE_GRAY:
        rle = 0;
        break;
     default:
        goto close_file;
     }
   bpp = header->bpp;
   if (!((bpp == 32) || (bpp == 24) || (bpp == 16) || (bpp == 8)))
     goto close_file;
   if ((bpp == 32) && (header->descriptor & TGA_DESC_ABITS)) hasa = 1;
   if ((bpp == 16) && (header->descriptor & TGA_DESC_ABITS)) hasa = 1;
   abits = header->descriptor & TGA_DESC_ABITS;
   // don't handle colormapped images
   if ((header->colorMapType) != 0)
     goto close_file;
   // if colormap size is anything other than legal sizes or 0 - not real tga
   if (!((header->colorMapSize == 0) ||
         (header->colorMapSize == 15) ||
         (header->colorMapSize == 16) ||
         (header->colorMapSize == 24) ||
         (header->colorMapSize == 32)))
     goto close_file;
   x = (header->xOriginHi << 8) | (header->xOriginLo);
   y = (header->yOriginHi << 8) | (header->yOriginLo);
   w = (header->widthHi << 8) | header->widthLo;
   h = (header->heightHi << 8) | header->heightLo;
   // x origin gerater that width, y origin greater than height - wrong file
   if ((x >= w) || (y >= h))
     goto close_file;
   // if descriptor has either of the top 2 bits set... not tga
   if (header->descriptor & 0xc0)
     goto close_file;

   if ((w < 1) || (h < 1) || (w > IMG_MAX_SIZE) || (h > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(w, h))
     goto close_file;

   if ((w != (int)prop->w) || (h != (int)prop->h))
     {
        *error = EVAS_LOAD_ERROR_GENERIC;
        goto close_file;
     }
   surface = pixels;

   datasize = size - sizeof(tga_header) - header->idLength;
   if (footer_present)
     datasize -= sizeof(tga_footer);

   bufptr = filedata + header->idLength;
   bufend = filedata + datasize;

   if (!rle)
     {
        for (y = 0; y < h; y++)
          {
             if (vinverted)
               /* some TGA's are stored upside-down! */
               dataptr = surface + ((h - y - 1) * w);
             else
               dataptr = surface + (y * w);
             switch (bpp)
               {
               case 32:
                  for (x = 0; (x < w) && ((bufptr + 4) <= bufend); x++)
                    {
                       if (hasa)
                         {
                            int a = bufptr[3];

                            switch (abits)
                              {
                               case 1:
                                 a = (a << 7) | (a << 6) | (a << 5) | (a << 4) | (a << 3) | (a << 2) | (a << 1) | (a);
                               case 2:
                                 a = (a << 6) | (a << 4) | (a << 2) | (a);
                               case 3:
                                 a = (a << 5) | (a << 2) | (a >> 1);
                               case 4:
                                 a = (a << 4) | (a);
                               case 5:
                                 a = (a << 3) | (a >> 2);
                               case 6:
                                 a = (a << 2) | (a >> 4);
                               case 7:
                                 a = (a << 1) | (a >> 6);
                               default:
                                 break;
                              }
                            *dataptr = ARGB_JOIN(a, bufptr[2], bufptr[1], bufptr[0]);
                         }
                       else
                         *dataptr = ARGB_JOIN(0xff, bufptr[2], bufptr[1], bufptr[0]);
                       dataptr++;
                       bufptr += 4;
                    }
                  break;
               case 24:
                  for (x = 0; (x < w) && ((bufptr + 3) <= bufend); x++)
                    {
                       *dataptr = ARGB_JOIN(0xff, bufptr[2], bufptr[1], bufptr[0]);
                       dataptr++;
                       bufptr += 3;
                    }
                  break;
               case 16:
                  for (x = 0; (x < w) && ((bufptr + 3) <= bufend); x++)
                    {
                       unsigned char r, g, b, a;
                       unsigned short tmp;

                       tmp =
                         (((unsigned short)bufptr[1]) << 8) |
                         (((unsigned short)bufptr[0]));
                       r = (tmp >> 7) & 0xf8; r |= r >> 5;
                       g = (tmp >> 2) & 0xf8; g |= g >> 5;
                       b = (tmp << 3) & 0xf8; b |= b >> 5;
                       a = 0xff;
                       if ((hasa) && (tmp & 0x8000)) a = 0;
                       *dataptr = ARGB_JOIN(a, r, g, b);
                       dataptr++;
                       bufptr += 2;
                    }
                  break;
               case 8:
                  for (x = 0; (x < w) && ((bufptr + 1) <= bufend); x++)
                    {
                       *dataptr = ARGB_JOIN(0xff, bufptr[0], bufptr[0], bufptr[0]);
                       dataptr++;
                       bufptr += 1;
                    }
                  break;
               default:
                  break;
               }
          }
     }
   else
     {
        int count, i;
        unsigned char val;
        unsigned int *dataend;

        dataptr = surface;
        dataend = dataptr + (w * h);
        while ((bufptr < bufend) && (dataptr < dataend))
          {
             val = *bufptr;
             bufptr++;
             count = (val & 0x7f) + 1;
             if (val & 0x80) // rel packet
               {
                  switch (bpp)
                    {
                    case 32:
                       if (bufptr < (bufend - 4))
                         {
                            unsigned char r, g, b;
                            int a = bufptr[3];

                            switch (abits)
                              {
                               case 1:
                                 a = (a << 7) | (a << 6) | (a << 5) | (a << 4) | (a << 3) | (a << 2) | (a << 1) | (a);
                               case 2:
                                 a = (a << 6) | (a << 4) | (a << 2) | (a);
                               case 3:
                                 a = (a << 5) | (a << 2) | (a >> 1);
                               case 4:
                                 a = (a << 4) | (a);
                               case 5:
                                 a = (a << 3) | (a >> 2);
                               case 6:
                                 a = (a << 2) | (a >> 4);
                               case 7:
                                 a = (a << 1) | (a >> 6);
                               default:
                                 break;
                              }
                            r = bufptr[2];
                            g = bufptr[1];
                            b = bufptr[0];
                            if (!hasa) a = 0xff;
                            bufptr += 4;
                            for (i = 0; (i < count) && (dataptr < dataend); i++)
                              {
                                 *dataptr = ARGB_JOIN(a, r, g, b);
                                 dataptr++;
                              }
                         }
                       break;
                    case 24:
                       if (bufptr < (bufend - 3))
                         {
                            unsigned char r, g, b;

                            r = bufptr[2];
                            g = bufptr[1];
                            b = bufptr[0];
                            bufptr += 3;
                            for (i = 0; (i < count) && (dataptr < dataend); i++)
                              {
                                 *dataptr = ARGB_JOIN(0xff, r, g, b);
                                 dataptr++;
                              }
                         }
                       break;
                    case 16:
                       if (bufptr < (bufend - 2))
                         {
                            unsigned char r, g, b, a;
                            unsigned short tmp;

                            tmp =
                              (((unsigned short)bufptr[1]) << 8) |
                              (((unsigned short)bufptr[0]));
                            r = (tmp >> 7) & 0xf8; r |= r >> 5;
                            g = (tmp >> 2) & 0xf8; g |= g >> 5;
                            b = (tmp << 3) & 0xf8; b |= b >> 5;
                            a = 0xff;
                            if ((hasa) && (tmp & 0x8000)) a = 0;
                            bufptr += 2;
                            for (i = 0; (i < count) && (dataptr < dataend); i++)
                              {
                                 *dataptr = ARGB_JOIN(a, r, g, b);
                                 dataptr++;
                              }
                         }
                       break;
                    case 8:
                       if (bufptr < (bufend - 1))
                         {
                            unsigned char g;

                            g = bufptr[0];
                            bufptr += 1;
                            for (i = 0; (i < count) && (dataptr < dataend); i++)
                              {
                                 *dataptr = ARGB_JOIN(0xff, g, g, g);
                                 dataptr++;
                              }
                         }
                       break;
                    default:
                       break;
                    }
               }
             else // raw
               {
                  switch (bpp)
                    {
                    case 32:
                       for (i = 0; (i < count) && (bufptr < (bufend - 4)) && (dataptr < dataend); i++)
                         {
                            if (hasa)
//                              *dataptr = ARGB_JOIN(255 - bufptr[3], bufptr[2], bufptr[1], bufptr[0]);
                              *dataptr = ARGB_JOIN(bufptr[3], bufptr[2], bufptr[1], bufptr[0]);
                            else
                              *dataptr = ARGB_JOIN(0xff, bufptr[2], bufptr[1], bufptr[0]);
                            dataptr++;
                            bufptr += 4;
                         }
                       break;
                    case 24:
                       for (i = 0; (i < count) && (bufptr < (bufend - 3)) && (dataptr < dataend); i++)
                         {
                            *dataptr = ARGB_JOIN(0xff, bufptr[2], bufptr[1], bufptr[0]);
                            dataptr++;
                            bufptr += 3;
                         }
                       break;
                    case 16:
                       for (i = 0; (i < count) && (bufptr < (bufend - 2)) && (dataptr < dataend); i++)
                         {
                            unsigned char r, g, b, a;
                            unsigned short tmp;

                            tmp =
                              (((unsigned short)bufptr[1]) << 8) |
                              (((unsigned short)bufptr[0]));
                            r = (tmp >> 7) & 0xf8; r |= r >> 5;
                            g = (tmp >> 2) & 0xf8; g |= g >> 5;
                            b = (tmp << 3) & 0xf8; b |= b >> 5;
                            a = 0xff;
                            if ((hasa) && (tmp & 0x8000)) a = 0;
                            *dataptr = ARGB_JOIN(a, r, g, b);
                            dataptr++;
                            bufptr += 2;
                         }
                       break;
                    case 8:
                       for (i = 0; (i < count) && (bufptr < (bufend - 1)) && (dataptr < dataend); i++)
                         {
                            *dataptr = ARGB_JOIN(0xff, bufptr[0], bufptr[0], bufptr[0]);
                            dataptr++;
                            bufptr += 1;
                         }
                       break;
                    default:
                       break;
                    }
               }
          }
        if (vinverted)
          {
             unsigned int *adv, *adv2, tmp;

             adv = surface;
             adv2 = surface + (w * (h - 1));
             for (y = 0; y < (h / 2); y++)
               {
                  for (x = 0; x < w; x++)
                    {
                       tmp = adv[x];
                       adv[x] = adv2[x];
                       adv2[x] = tmp;
                    }
                  adv2 -= w;
                  adv += w;
               }
          }
     }

   prop->premul = EINA_TRUE;

   *error = EVAS_LOAD_ERROR_NONE;
   res = EINA_TRUE;

 close_file:
   if (seg != NULL) eina_file_map_free(f, seg);
   return res;
}

static Evas_Image_Load_Func evas_image_load_tga_func =
{
  evas_image_load_file_open_tga,
  evas_image_load_file_close_tga,
  evas_image_load_file_head_tga,
  evas_image_load_file_data_tga,
  NULL,
  EINA_TRUE,
  EINA_FALSE
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_tga_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "tga",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, tga);

#ifndef EVAS_STATIC_BUILD_TGA
EVAS_EINA_MODULE_DEFINE(image_loader, tga);
#endif


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "evas_common.h"
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

typedef struct
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
} tga_header;
                  
typedef struct
{
   unsigned int        extensionAreaOffset;
   unsigned int        developerDirectoryOffset;
   char                signature[16];
   char                dot;
   char                null;
} tga_footer;


static Eina_Bool evas_image_load_file_head_tga(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);
static Eina_Bool evas_image_load_file_data_tga(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);

static Evas_Image_Load_Func evas_image_load_tga_func =
{
  EINA_TRUE,
  evas_image_load_file_head_tga,
  evas_image_load_file_data_tga
};

static Eina_Bool
evas_image_load_file_head_tga(Image_Entry *ie, const char *file, const char *key __UNUSED__, int *error)
{
   int fd;
   unsigned char *seg = MAP_FAILED, *filedata;
   struct stat ss;
   tga_header *header;
   tga_footer *footer;
   char hasa = 0, footer_present = 0, vinverted = 0, rle = 0;
   int w = 0, h = 0, bpp;

   fd = open(file, O_RDONLY);
   
   *error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
   if (fd < 0) return EINA_FALSE;
   if (fstat(fd, &ss) < 0) goto close_file;
   
   *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
   if (ss.st_size < (sizeof(tga_header) + sizeof(tga_footer))) goto close_file;
   seg = mmap(0, ss.st_size, PROT_READ, MAP_SHARED, fd, 0);
   if (seg == MAP_FAILED) goto close_file;
   filedata = seg;
   
   header = (tga_header *)filedata;
   // no unaligned data accessed, so ok
   footer = (tga_footer *)(filedata + (ss.st_size - sizeof(tga_footer)));
   if (!memcmp(footer->signature, TGA_SIGNATURE, sizeof(footer->signature)))
     {
        // footer is ther and matches. this is a tga file - any problems now
        // are a corrupt file
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        footer_present = 1;
     }
   filedata = (char *)filedata + sizeof(tga_header);
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
   w = (header->widthHi << 8) | header->widthLo;
   h = (header->heightHi << 8) | header->heightLo;
   
   if ((w < 1) || (h < 1) || (w > IMG_MAX_SIZE) || (h > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(w, h))
     goto close_file;
   
   ie->w = w;
   ie->h = h;
   if (hasa) ie->flags.alpha = 1;
   
   if (seg != MAP_FAILED) munmap(seg, ss.st_size);
   close(fd);
   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;

close_file:
   if (seg != MAP_FAILED) munmap(seg, ss.st_size);
   close(fd);
   return EINA_FALSE;
}

static Eina_Bool
evas_image_load_file_data_tga(Image_Entry *ie, const char *file, const char *key __UNUSED__, int *error)
{
   int fd;
   unsigned char *seg = MAP_FAILED, *filedata;
   struct stat ss;
   tga_header *header;
   tga_footer *footer;
   char hasa = 0, footer_present = 0, vinverted = 0, rle = 0;
   int w = 0, h = 0, x, y, bpp;
   unsigned int *surface, *dataptr;
   unsigned int  datasize;
   unsigned char *bufptr, *bufend;
   
   fd = open(file, O_RDONLY);
   
   *error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
   if (fd < 0) return EINA_FALSE;
   if (fstat(fd, &ss) < 0) goto close_file;
   
   *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
   if (ss.st_size < (sizeof(tga_header) + sizeof(tga_footer))) goto close_file;
   seg = mmap(0, ss.st_size, PROT_READ, MAP_SHARED, fd, 0);
   if (seg == MAP_FAILED) goto close_file;
   filedata = seg;
   
   header = (tga_header *)filedata;
   // no unaligned data accessed, so ok
   footer = (tga_footer *)(filedata + (ss.st_size - sizeof(tga_footer)));
   if (!memcmp(footer->signature, TGA_SIGNATURE, sizeof(footer->signature)))
     {
        // footer is ther and matches. this is a tga file - any problems now
        // are a corrupt file
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        footer_present = 1;
     }
   filedata = (char *)filedata + sizeof(tga_header);
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
   w = (header->widthHi << 8) | header->widthLo;
   h = (header->heightHi << 8) | header->heightLo;
   
   if ((w < 1) || (h < 1) || (w > IMG_MAX_SIZE) || (h > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(w, h))
     goto close_file;
   
   if ((w != ie->w) || (h != ie->h))
     {
	*error = EVAS_LOAD_ERROR_GENERIC;
	goto close_file;
     }
   evas_cache_image_surface_alloc(ie, w, h);
   surface = evas_cache_image_pixels(ie);
   if (!surface)
     {
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	goto close_file;
     }

   datasize = ss.st_size - sizeof(tga_header) - header->idLength;
   if (footer_present)
     datasize = ss.st_size - sizeof(tga_header) - header->idLength - 
     sizeof(tga_footer);
   
   bufptr = filedata + header->idLength;
   bufend = filedata + datasize;

   dataptr = surface;

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
                         *dataptr = ARGB_JOIN(255 - bufptr[3], bufptr[2], bufptr[1], bufptr[0]);
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
                            unsigned char r, g, b, a;
                            
                            a = 255 - bufptr[3];
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
                              *dataptr = ARGB_JOIN(255 - bufptr[3], bufptr[2], bufptr[1], bufptr[0]);
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
   
   evas_common_image_premul(ie);
   
   if (seg != MAP_FAILED) munmap(seg, ss.st_size);
   close(fd);
   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;

close_file:
   if (seg != MAP_FAILED) munmap(seg, ss.st_size);
   close(fd);
   return EINA_FALSE;
}

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_tga_func);
   return 1;
}

static void
module_close(Evas_Module *em)
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

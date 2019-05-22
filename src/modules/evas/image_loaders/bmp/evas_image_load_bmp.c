#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include <math.h>

#include "evas_common_private.h"
#include "evas_private.h"

typedef struct _BMP_Header BMP_Header;
struct _BMP_Header
{
   unsigned int bmpsize;
   unsigned short res1;
   unsigned short res2;
   unsigned int offset;
   unsigned int head_size;
   int width;
   int height;
   unsigned short bit_count;
   int comp;
   // hdpi
   // vdpi
   int palette_size;
   // important_colors

   unsigned int rmask;
   unsigned int gmask;
   unsigned int bmask;
   unsigned int amask;

   Eina_Bool hasa;
};

typedef struct _Evas_Loader_Internal Evas_Loader_Internal;
struct _Evas_Loader_Internal
{
   Eina_File *f;
   Evas_Image_Load_Opts *opts;
};

static Eina_Bool
read_short(unsigned char *map, size_t length, size_t *position, short *ret)
{
   unsigned char b[2];

   if (*position + 2 > length) return EINA_FALSE;
   b[0] = map[(*position)++];
   b[1] = map[(*position)++];
   *ret = (b[1] << 8) | b[0];
   return EINA_TRUE;
}

static Eina_Bool
read_ushort(unsigned char *map, size_t length, size_t *position, unsigned short *ret)
{
   unsigned char b[2];

   if (*position + 2 > length) return EINA_FALSE;
   b[0] = map[(*position)++];
   b[1] = map[(*position)++];
   *ret = (b[1] << 8) | b[0];
   return EINA_TRUE;
}

static Eina_Bool
read_int(unsigned char *map, size_t length, size_t *position, int *ret)
{
   unsigned char b[4];
   int i;

   if (*position + 4 > length) return EINA_FALSE;
   for (i = 0; i < 4; i++)
     b[i] = map[(*position)++];
   *ret = ARGB_JOIN(b[3], b[2], b[1], b[0]);
   return EINA_TRUE;
}

static Eina_Bool
read_uint(unsigned char *map, size_t length, size_t *position, unsigned int *ret)
{
   unsigned char b[4];
   int i;

   if (*position + 4 > length) return EINA_FALSE;
   for (i = 0; i < 4; i++)
     b[i] = map[(*position)++];
   *ret = ARGB_JOIN(b[3], b[2], b[1], b[0]);
   return EINA_TRUE;
}

static Eina_Bool
read_uchar(unsigned char *map, size_t length, size_t *position, unsigned char *ret)
{
   if (*position + 1 > length) return EINA_FALSE;
   *ret = map[(*position)++];
   return EINA_TRUE;
}

static Eina_Bool
read_skip(size_t length, size_t *position, int skip)
{
   if (*position + skip > length) return EINA_FALSE;
   *position += skip;
   return EINA_TRUE;
}

static Eina_Bool
read_mem(unsigned char *map, size_t length, size_t *position, void *buffer, int size)
{
   if (*position + size > length) return EINA_FALSE;
   memcpy(buffer, map + *position, size);
   *position += size;
   return EINA_TRUE;
}

static Eina_Bool
_evas_image_load_file_header(void *map, size_t fsize, size_t *position, int *image_size,
                             BMP_Header *header, int *error)
{
   if (strncmp(map, "BM", 2)) return EINA_FALSE; // magic number
   *position += 2;
   *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
   if (!read_uint(map, fsize, position, &header->bmpsize)) return EINA_FALSE;
   if (!read_ushort(map, fsize, position, &header->res1)) return EINA_FALSE;
   if (!read_ushort(map, fsize, position, &header->res2)) return EINA_FALSE;
   if (!read_uint(map, fsize, position, &header->offset)) return EINA_FALSE;
   if (!read_uint(map, fsize, position, &header->head_size)) return EINA_FALSE;
   if (header->offset > fsize) return EINA_FALSE;

   switch (header->head_size)
     {
      case  12: // OS/2 V1 + Windows 3.0
        {
           short tmp;

           if (!read_short(map, fsize, position, &tmp)) return EINA_FALSE;
           header->width = tmp; // width
           if (!read_short(map, fsize, position, &tmp)) return EINA_FALSE;
           header->height = tmp; // height
           if (!read_short(map, fsize, position, &tmp)) return EINA_FALSE;
           //planes = tmp; // must be 1
           if (!read_short(map, fsize, position, &tmp)) return EINA_FALSE;
           header->bit_count = tmp; // bits per pixel: 1, 4, 8 & 24
           break;
        }
      case 64: // OS/2 V2
        {
           short tmp;
           int tmp2;

           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->width = tmp2; // width
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->height = tmp2; // height
           if (!read_short(map, fsize, position, &tmp)) return EINA_FALSE;
           //planes = tmp; // must be 1
           if (!read_short(map, fsize, position, &tmp)) return EINA_FALSE;
           header->bit_count = tmp; // bits per pixel: 1, 4, 8, 16, 24 & 32
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->comp = tmp2; // compression method
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           if (tmp2 <= *image_size) *image_size = tmp2; // bitmap data size, GIMP can handle image size error
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           //hdpi = (tmp2 * 254) / 10000; // horizontal pixels/meter
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           //vdpi = (tmp2 * 254) / 10000; // vertical pixles/meter
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->palette_size = tmp2; // number of palette colors power (2^n - so 0 - 8)
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           //important_colors = tmp2; // number of important colors - 0 if all
           if (!read_skip(fsize, position, 24)) return EINA_FALSE; // skip unused header
           if (*image_size == 0) *image_size = fsize - header->offset;
           break;
        }
      case 40: // Windows 3.0 + (v3)
        {
           short tmp;
           int tmp2;

           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->width = tmp2; // width
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->height = tmp2; // height
           if (!read_short(map, fsize, position, &tmp)) return EINA_FALSE;
           //planes = tmp; // must be 1
           if (!read_short(map, fsize, position, &tmp)) return EINA_FALSE;
           header->bit_count = tmp; // bits per pixel: 1, 4, 8, 16, 24 & 32
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->comp = tmp2; // compression method
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           if (tmp2 <= *image_size) *image_size = tmp2; // bitmap data size, GIMP can handle image size error
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           //hdpi = (tmp2 * 254) / 10000; // horizontal pixels/meter
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           //vdpi = (tmp2 * 254) / 10000; // vertical pixles/meter
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->palette_size = tmp2; // number of palette colors power (2^n - so 0 - 8)
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           //important_colors = tmp2; // number of important colors - 0 if all
           if (*image_size == 0) *image_size = fsize - header->offset;
           if ((header->comp == 6) && (header->bit_count == 32)) header->hasa = 1;
           break;
        }
      case 108: // Windows 95/NT4 + (v4)
        {
           short tmp;
           int tmp2;

           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->width = tmp2; // width
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->height = tmp2; // height
           if (!read_short(map, fsize, position, &tmp)) return EINA_FALSE;
           //planes = tmp; // must be 1
           if (!read_short(map, fsize, position, &tmp)) return EINA_FALSE;
           header->bit_count = tmp; // bits per pixel: 1, 4, 8, 16, 24 & 32
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->comp = tmp2; // compression method
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           if (tmp2 <= *image_size) *image_size = tmp2; // bitmap data size, GIMP can handle image size error
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           //hdpi = (tmp2 * 254) / 10000; // horizontal pixels/meter
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           //vdpi = (tmp2 * 254) / 10000; // vertical pixles/meter
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->palette_size = tmp2; // number of palette colors power (2^n - so 0 - 8)
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           //important_colors = tmp2; // number of important colors - 0 if all
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->rmask = tmp2; // red mask
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->gmask = tmp2; // green mask
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->bmask = tmp2; // blue mask
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->amask = tmp2; // alpha mask
           if (!read_skip(fsize, position, 36)) return EINA_FALSE; // skip unused cie
           if (!read_skip(fsize, position, 12)) return EINA_FALSE; // skip unused gamma
           if (*image_size == 0) *image_size = fsize - header->offset;
           if ((header->amask) && (header->bit_count == 32)) header->hasa = 1;
           break;
        }
      case 124: // Windows 98/2000 + (v5)
        {
           short tmp;
           int tmp2;

           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->width = tmp2; // width
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->height = tmp2; // height
           if (!read_short(map, fsize, position, &tmp)) return EINA_FALSE;
           //planes = tmp; // must be 1
           if (!read_short(map, fsize, position, &tmp)) return EINA_FALSE;
           header->bit_count = tmp; // bits per pixel: 1, 4, 8, 16, 24 & 32
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->comp = tmp2; // compression method
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           if (tmp2 <= *image_size) *image_size = tmp2; // bitmap data size, GIMP can handle image size error
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           //hdpi = (tmp2 * 254) / 10000; // horizontal pixels/meter
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           //vdpi = (tmp2 * 254) / 10000; // vertical pixles/meter
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->palette_size = tmp2; // number of palette colors power (2^n - so 0 - 8)
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           //important_colors = tmp2; // number of important colors - 0 if all
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->rmask = tmp2; // red mask
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->gmask = tmp2; // green mask
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->bmask = tmp2; // blue mask
           if (!read_int(map, fsize, position, &tmp2)) return EINA_FALSE;
           header->amask = tmp2; // alpha mask
           if (!read_skip(fsize, position, 36)) return EINA_FALSE; // skip unused cie
           if (!read_skip(fsize, position, 12)) return EINA_FALSE; // skip unused gamma
           if (!read_skip(fsize, position, 16)) return EINA_FALSE; // skip others
           if (*image_size == 0) *image_size = fsize - header->offset;
           if ((header->amask) && (header->bit_count == 32)) header->hasa = 1;
           break;
        }
      default:
         return EINA_FALSE;
     }

   return EINA_TRUE;
}

static void *
evas_image_load_file_open_bmp(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
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

   loader->f = f;
   loader->opts = opts;

   return loader;
}

static void
evas_image_load_file_close_bmp(void *loader_data)
{
   free(loader_data);
}

static Eina_Bool
evas_image_load_file_head_bmp(void *loader_data,
                              Evas_Image_Property *prop,
                              int *error)
{
   Evas_Loader_Internal *loader;
   Evas_Image_Load_Opts *load_opts;
   Eina_File *f;
   void *map = NULL;
   size_t position = 0;
   BMP_Header header;
   int image_size = 0;
   size_t fsize;
   Eina_Bool r = EINA_FALSE;

   loader = loader_data;
   f = loader->f;
   load_opts = loader->opts;

   *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
   fsize = eina_file_size_get(f);
   if (fsize < 2) goto close_file;

   map = eina_file_map_all(f, EINA_FILE_RANDOM);
   if (!map) goto close_file;

   memset(&header, 0, sizeof (header));

   if (!_evas_image_load_file_header(map, fsize, &position, &image_size, &header, error))
     goto close_file;

   if (header.height < 0)
     {
        header.height = -header.height;
        //right_way_up = 1;
     }

   if ((header.width < 1) || (header.height < 1) ||
       (header.width > IMG_MAX_SIZE) || (header.height > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(header.width, header.height))
     {
        if (IMG_TOO_BIG(header.width, header.height))
          *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        else
          *error = EVAS_LOAD_ERROR_GENERIC;
	goto close_file;
     }

   if (load_opts->emile.region.w > 0 && load_opts->emile.region.h > 0)
     {
        if ((load_opts->emile.region.w + load_opts->emile.region.x > header.width) ||
            (load_opts->emile.region.h + load_opts->emile.region.y > header.height))
          {
             *error = EVAS_LOAD_ERROR_GENERIC;
             goto close_file;
          }
        header.width = load_opts->emile.region.w;
        header.height = load_opts->emile.region.h;
     }

   /* It is not bad idea that bmp loader support scale down decoding 
    * because of memory issue in mobile world.*/
   if (load_opts->emile.scale_down_by > 1)
     {
        header.width /= load_opts->emile.scale_down_by;
        header.height /= load_opts->emile.scale_down_by;
     }

   if (header.bit_count < 16)
     {
        //if ((palette_size < 0) || (palette_size > 256)) pal_num = 256;
        //else pal_num = palette_size;
        if (header.bit_count == 1)
          {
             if (header.comp == 0) // no compression
               {
               }
             else
               goto close_file;
          }
        else if (header.bit_count == 4)
          {
             if (header.comp == 0) // no compression
               {
               }
             else if (header.comp == 2) // rle 4bit/pixel
               {
               }
             else
               goto close_file;
          }
        else if (header.bit_count == 8)
          {
             if (header.comp == 0) // no compression
               {
               }
             else if (header.comp == 1) // rle 8bit/pixel
               {
               }
             else
               goto close_file;
          }
     }
   else if ((header.bit_count == 16) || (header.bit_count == 24) || (header.bit_count == 32))
     {
        if (header.comp == 0) // no compression
          {
             // handled
          }
        else if (header.comp == 3) // bit field
          {
             // handled
          }
        else if (header.comp == 4) // jpeg - only printer drivers
          goto close_file;
        else if (header.comp == 5) // png - only printer drivers
          goto close_file;
        else
          goto close_file;
     }
   else
     goto close_file;

   prop->w = header.width;
   prop->h = header.height;
   if (header.hasa) prop->alpha = 1;

   *error = EVAS_LOAD_ERROR_NONE;
   r = EINA_TRUE;

 close_file:
   if (map) eina_file_map_free(f, map);
   return r;
}

static Eina_Bool
evas_image_load_file_data_bmp(void *loader_data,
                              Evas_Image_Property *prop,
			      void *pixels,
			      int *error)
{
   Evas_Loader_Internal *loader;
   Evas_Image_Load_Opts *opts;
   Eina_File *f;
   BMP_Header header;
   void *map = NULL;
   size_t position = 0;
   unsigned char *buffer = NULL, *buffer_end = NULL, *p;
   int x = 0, y = 0, image_size = 0;
   unsigned int *pal = NULL, pal_num = 0, *pix = NULL, fix, *surface = pixels;
   int right_way_up = 0;
   unsigned char r, g, b, a;
   size_t fsize;

   /* for scale decoding */
   unsigned int *scale_surface = NULL, *scale_pix = NULL;
   int scale_ratio = 1, image_w = 0, image_h = 0;
   int region_set = 0, region_x = 0, region_y = 0, region_w, region_h;
   int row_size = 0; /* Row size is rounded up to a multiple of 4bytes */
   int read_line = 0; /* total read line */

   loader = loader_data;
   f = loader->f;
   opts = loader->opts;

   *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
   fsize = eina_file_size_get(f);
   if (fsize < 2) goto close_file;

   map = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   if (!map) goto close_file;

   memset(&header, 0, sizeof (header));
   header.palette_size = -1;

   if (!_evas_image_load_file_header(map, fsize, &position, &image_size, &header, error))
     goto close_file;

   *error = EVAS_LOAD_ERROR_GENERIC;
   if (header.height < 0)
     {
        header.height = -header.height;
        right_way_up = 1;
     }
   if ((header.width < 1) || (header.height < 1) || (header.width > IMG_MAX_SIZE) || (header.height > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(header.width, header.height))
     {
        if (IMG_TOO_BIG(header.width, header.height))
          *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        else
          *error = EVAS_LOAD_ERROR_GENERIC;
        goto close_file;
     }
   image_w = region_w = header.width;
   image_h = region_h = header.height;

   if (opts->emile.region.w > 0 && opts->emile.region.h > 0)
     {
        if ((opts->emile.region.w + opts->emile.region.x > header.width) ||
            (opts->emile.region.h + opts->emile.region.y > header.height))
          {
             *error = EVAS_LOAD_ERROR_GENERIC;
             goto close_file;
          }
        region_set = 1;
        region_x = opts->emile.region.x;
        region_y = image_h - (opts->emile.region.h + opts->emile.region.y);
        region_w = opts->emile.region.w;
        region_h = opts->emile.region.h;

        header.width = opts->emile.region.w;
        header.height = opts->emile.region.h;
     }
   /* It is not bad idea that bmp loader support scale down decoding 
    * because of memory issue in mobile world. */
   if (opts->emile.scale_down_by > 1)
     scale_ratio = opts->emile.scale_down_by;

   if (scale_ratio > 1)
     {
        header.width /= scale_ratio;
        header.height /= scale_ratio;

        if ((header.width < 1) || (header.height < 1) )
          {
             *error = EVAS_LOAD_ERROR_GENERIC;
             goto close_file;
          }
     }

   if ((header.width != (int)prop->w) || (header.height != (int)prop->h))
     {
	*error = EVAS_LOAD_ERROR_GENERIC;
        goto close_file;
     }

   row_size = ceil((double)(image_w * header.bit_count) / 32) * 4;

   //If the file is not downloaded perfactly, image_size can be smaller (row_size * header.height).
   //in this case, image_size should be filesize-offset size.

   if (image_size > row_size * header.height)
     image_size = row_size * header.height;

   if (region_set)
     read_line = region_y;

   if (header.bit_count < 16)
     {
        unsigned int i;

        if (header.bit_count == 1)
          {
             if ((header.palette_size <= 0) || (header.palette_size > 2)) pal_num = 2;
             else pal_num = header.palette_size;
          }
        else if (header.bit_count == 4)
          {
             if ((header.palette_size <= 0) || (header.palette_size > 16)) pal_num = 16;
             else pal_num = header.palette_size;
          }
        else if (header.bit_count == 8)
          {
             if ((header.palette_size <= 0) || (header.palette_size > 256)) pal_num = 256;
             else pal_num = header.palette_size;
          }
        pal = alloca(256 * 4);
        for (i = 0; i < pal_num; i++)
          {
             if (!read_uchar(map, fsize, &position, &b)) goto close_file;
             if (!read_uchar(map, fsize, &position, &g)) goto close_file;
             if (!read_uchar(map, fsize, &position, &r)) goto close_file;
             if ((header.head_size != 12) /*&& (palette_size != 0)*/)
               { // OS/2 V1 doesn't do the pad byte
                  if (!read_uchar(map, fsize, &position, &a)) goto close_file;
               }
             a = 0xff; // fillin a as solid for paletted images
             pal[i] = ARGB_JOIN(a, r, g, b);
          }
        position = header.offset;

        if ((!region_set && scale_ratio == 1) || (header.comp != 0))
          {
             if (image_size < (int)(fsize - position))
               image_size = fsize - position;
             buffer = malloc(image_size + 8); // add 8 for padding to avoid checks
          }
        else
          {
             scale_surface = malloc(image_w * sizeof(DATA32)); //for one line decoding
             if (!scale_surface)
               {
                  *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                  goto close_file;
               }
             buffer = malloc(row_size); // scale down is usually set because of memory issue, so read line by line
          }

        if (!buffer)
          {
             *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
             goto close_file;
          }
        if ((!region_set && scale_ratio == 1) || (header.comp != 0))
          {
             buffer_end = buffer + image_size;
             if (!read_mem(map, fsize, &position, buffer, image_size)) goto close_file;
          }
        else
          {
             if (region_set)
               position += row_size * region_y;

             if (!read_mem(map, fsize, &position, buffer, row_size)) goto close_file;
             buffer_end = buffer + row_size;
          }
        p = buffer;

        if (header.bit_count == 1)
          {
             if (header.comp == 0) // no compression
               {
                  pix = surface;

                  for (y = 0; y < header.height; y++)
                    {
                       if (!right_way_up) pix = surface + ((header.height - 1 - y) * header.width);
                       if (scale_ratio > 1 || region_set) pix = scale_surface; // one line decoding

                       for (x = 0; x < image_w; x++)
                         {
                            if ((x & 0x7) == 0x0)
                              {
                                 *pix = pal[*p >> 7];
                              }
                            else if ((x & 0x7) == 0x1)
                              {
                                 *pix = pal[(*p >> 6) & 0x1];
                              }
                            else if ((x & 0x7) == 0x2)
                              {
                                 *pix = pal[(*p >> 5) & 0x1];
                              }
                            else if ((x & 0x7) == 0x3)
                              {
                                 *pix = pal[(*p >> 4) & 0x1];
                              }
                            else if ((x & 0x7) == 0x4)
                              {
                                 *pix = pal[(*p >> 3) & 0x1];
                              }
                            else if ((x & 0x7) == 0x5)
                              {
                                 *pix = pal[(*p >> 2) & 0x1];
                              }
                            else if ((x & 0x7) == 0x6)
                              {
                                 *pix = pal[(*p >> 1) & 0x1];
                              }
                            else
                              {
                                 *pix = pal[*p & 0x1];
                                 p++;
                              }
                            if (p >= buffer_end) break;
                            pix++;
                         }

                       if (scale_ratio > 1 || region_set)
                         {
                            if (!right_way_up) scale_pix = surface + ((header.height - 1 - y) * header.width);
                            else scale_pix = surface + (y * header.width);

                            if (region_set)
                              pix = scale_surface + region_x;
                            else
                              pix = scale_surface;

                            for (x = 0; x < header.width; x++)
                              {
                                 *scale_pix = *pix;
                                 scale_pix ++;
                                 pix += scale_ratio;
                              }
                            read_line += scale_ratio;
                            if (read_line >= image_h) break;

                            position += row_size * (scale_ratio - 1);
                            if (!read_mem(map, fsize, &position, buffer, row_size)) goto close_file;
                            p = buffer;
                            buffer_end = buffer + row_size;
                         }
                       else
                         {
                            if ((x & 0x7) != 0) p++;
                            fix = (int)(((uintptr_t)p) & 0x3);
                            if (fix > 0) p += 4 - fix; // align row read
                            if (p >= buffer_end) break;
                         }
                    }
               }
             else
               goto close_file;
          }
        else if (header.bit_count == 4)
          {
             if (header.comp == 0) // no compression
               {
                  pix = surface;
                  for (y = 0; y < header.height; y++)
                    {
                       if (!right_way_up) pix = surface + ((header.height - 1 - y) * header.width);
                       if (scale_ratio > 1 || region_set) pix = scale_surface; // one line decoding
                       for (x = 0; x < image_w; x++)
                         {
                            if ((x & 0x1) == 0x1)
                              {
                                 *pix = pal[*p & 0x0f];
                                 p++;
                              }
                            else
                              {
                                 *pix = pal[*p >> 4];
                              }
                            if (p >= buffer_end) break;
                            pix++;
                         }
                       if (scale_ratio > 1 || region_set)
                         {
                            if (!right_way_up) scale_pix = surface + ((header.height - 1 - y) * header.width);
                            else scale_pix = surface + (y * header.width);

                            if (region_set)
                              pix = scale_surface + region_x;
                            else
                              pix = scale_surface;

                            for (x = 0; x < header.width; x++)
                              {
                                 *scale_pix = *pix;
                                 scale_pix ++;
                                 pix += scale_ratio;
                              }
                            read_line += scale_ratio;
                            if (read_line >= image_h) break;

                            position += row_size * (scale_ratio - 1);
                            if (!read_mem(map, fsize, &position, buffer, row_size)) goto close_file;
                            p = buffer;
                            buffer_end = buffer + row_size;
                         }
                       else
                         {
                            if ((x & 0x1) != 0) p++;
                            fix = (int)(((uintptr_t)p) & 0x3);
                            if (fix > 0) p += 4 - fix; // align row read
                            if (p >= buffer_end) break;
                         }
                    }
               }
             else if (header.comp == 2) // rle 4bit/pixel
               {
                  int count = 0, done = 0, wpad;
                  int scale_x = 0, scale_y = 0;
                  Eina_Bool scale_down_line = EINA_TRUE;

                  pix = surface;
                  if (region_set && region_y > 0) scale_down_line = EINA_FALSE;

                  if (!right_way_up) pix = surface + ((header.height - 1 - y) * header.width);
                  wpad = ((image_w + 1) / 2) * 2;
                  while (p < buffer_end)
                    {
                       if (p[0])
                         {
                            if (scale_down_line)
                              {
                                 if ((x + p[0]) <= wpad)
                                   {
                                      unsigned int col1 = pal[p[1] >> 4];
                                      unsigned int col2 = pal[p[1] & 0xf];

                                      count = p[0] / 2;
                                      while (count > 0)
                                        {
                                           if (x < header.width)
                                             {
                                                if ((x >= region_x) && ((x % scale_ratio) == 0) && (scale_x < header.width))
                                                  {
                                                     *pix = col1;
                                                     pix++;
                                                     scale_x++;
                                                  }
                                                x++;
                                             }
                                           if (x < header.width)
                                             {
                                                if ((x >= region_x) && ((x % scale_ratio) == 0) && (scale_x < header.width))
                                                  {
                                                     *pix = col2;
                                                     pix++;
                                                     scale_x++;
                                                  }
                                                x++;
                                             }
                                           count--;
                                        }
                                      if (p[0] & 0x1)
                                        {
                                           if ((x >= region_x) && ((x % scale_ratio) == 0) && (scale_x < header.width))
                                             {
                                                *pix = col1;
                                                pix++;
                                                scale_x++;
                                             }
                                           x++;
                                        }
                                   }
                              }
                            p += 2;
                         }
                       else
                         {
                            switch (p[1])
                              {
                               case 0: // EOL
                                  x = 0;
                                  scale_x = 0;
                                  y++;
                                  if (y >= region_y && (y % scale_ratio) == 0)
                                    {
                                       scale_y++;
                                       scale_down_line = EINA_TRUE;
                                       if (!right_way_up)
                                         pix = surface + ((header.height - 1 - scale_y) * header.width);
                                       else
                                         pix = surface + (scale_y * header.width);
                                    }
                                  else
                                    scale_down_line = EINA_FALSE;
                                  if (scale_y >= header.height)
                                    {
                                       p = buffer_end;
                                    }
                                  p += 2;
                                  break;
                               case 1: // EOB
                                  p = buffer_end;
                                  break;
                               case 2: // DELTA
                                  x += p[2];
                                  y += p[3];
                                  scale_x = x / scale_ratio;
                                  scale_y = y / scale_ratio;
                                  if ((scale_x >= header.width) || (scale_y >= header.height))
                                    {
                                       p = buffer_end;
                                    }
                                  if (!right_way_up)
                                    pix = surface + scale_x + ((header.height - 1 - scale_y) * header.width);
                                  else
                                    pix = surface + scale_x + (scale_y * header.width);
                                  p += 4;
                                  break;
                               default:
                                  count = p[1];
                                  if (((p + count) > buffer_end) ||
                                      ((x + count) > header.width))
                                    {
                                       p = buffer_end;
                                       break;
                                    }
                                  p += 2;
                                  done = count;
                                  count /= 2;
                                  while (count > 0)
                                    {
                                       if ((y >= region_y) && (x >= region_x) && ((x % scale_ratio) == 0) && (scale_x < header.width))
                                         {
                                            *pix = pal[*p >> 4];
                                            pix++;
                                            scale_x++;
                                         }
                                       x++;
                                       if ((y >= region_y) && (x >= region_x) && ((x % scale_ratio) == 0) && (scale_x < header.width))
                                         {
                                            *pix = pal[*p & 0xf];
                                            pix++;
                                            scale_x++;
                                         }
                                       x++;

                                       p++;
                                       count--;
                                    }

                                  if (done & 0x1)
                                    {
                                       if (((y >= region_y) && (x >= region_x) && (x % scale_ratio) == 0) && (scale_x < header.width))
                                         {
                                            *pix = pal[*p >> 4];
                                            scale_x++;
                                         }
                                       x++;
                                       p++;
                                    }
                                  if ((done & 0x3) == 0x1)
                                    p += 2;
                                  else if ((done & 0x3) == 0x2)
                                    p += 1;
                                  break;
                              }
                         }
                    }
               }
             else
               goto close_file;
          }
        else if (header.bit_count == 8)
          {
             if (header.comp == 0) // no compression
               {
                  pix = surface;
                  for (y = 0; y < header.height; y++)
                    {
                       if (!right_way_up) pix = surface + ((header.height - 1 - y) * header.width);
                       if (region_set) p += region_x;
                       for (x = 0; x < header.width; x++)
                         {
                            *pix = pal[*p];
                            p += scale_ratio;
                            if (p >= buffer_end) break;
                            pix++;
                         }
                       if (scale_ratio > 1 || region_set)
                         {
                            read_line += scale_ratio;
                            if (read_line >= image_h) break;

                            position += row_size * (scale_ratio - 1);
                            if (!read_mem(map, fsize, &position, buffer, row_size)) goto close_file;
                            p = buffer;
                            buffer_end = buffer + row_size;
                         }
                       else
                         {
                            fix = (int)(((uintptr_t)p) & 0x3);
                            if (fix > 0) p += 4 - fix; // align row read
                            if (p >= buffer_end) break;
                         }
                    }
               }
             else if (header.comp == 1) // rle 8bit/pixel
               {
                  int count = 0, done = 0;
                  int scale_x = 0, scale_y = 0;
                  Eina_Bool scale_down_line = EINA_TRUE;
                  pix = surface;
                  if (region_set && 0 < region_y)
                    scale_down_line = EINA_FALSE;

                  if (!right_way_up) pix = surface + ((header.height - 1 - y) * header.width);

                  while (p < buffer_end)
                    {
                       if (p[0])
                         {
                            if (scale_down_line)
                              {
                                 if ((x + p[0]) <= image_w)
                                   {
                                      unsigned int col = pal[p[1]];

                                      count = p[0];
                                      while (count > 0)
                                        {
                                           if ((x >= region_x) && ((x % scale_ratio) == 0) && (scale_x < header.width))
                                             {
                                                *pix = col;
                                                pix++;
                                                scale_x ++;
                                             }
                                           x++;
                                           count--;
                                        }
                                   }
                              }
                            p += 2;
                         }
                       else
                         {
                            switch (p[1])
                              {
                               case 0: // EOL
                                  x = 0;
                                  scale_x = 0;
                                  y++;
                                  if (y >= region_y && (y % scale_ratio) == 0)
                                    {
                                       scale_y++;
                                       scale_down_line = EINA_TRUE;
                                       if (!right_way_up)
                                         pix = surface + ((header.height - 1 - scale_y) * header.width);
                                       else
                                         pix = surface + (scale_y * header.width);
                                    }
                                  else
                                    scale_down_line = EINA_FALSE;

                                  if (scale_y >= header.height)
                                    {
                                       p = buffer_end;
                                    }
                                  p += 2;
                                  break;
                               case 1: // EOB
                                  p = buffer_end;
                                  break;
                               case 2: // DELTA
                                  x += p[2];
                                  y += p[3];
                                  scale_x = x / scale_ratio;
                                  scale_y = y / scale_ratio;
                                  if ((scale_x >= header.width) || (scale_y >= header.height))
                                    {
                                       p = buffer_end;
                                    }
                                  if (!right_way_up)
                                    pix = surface + scale_x + ((header.height - 1 - scale_y) * header.width);
                                  else
                                    pix = surface + scale_x + (scale_y * header.width);
                                  p += 4;
                                  break;
                               default:
                                  count = p[1];
                                  if (((p + count) > buffer_end) ||
                                      ((x + count) > image_w))
                                    {
                                       p = buffer_end;
                                       break;
                                    }
                                  p += 2;
                                  done = count;
                                  while (count > 0)
                                    {
                                       if ((x >= region_x) && ((x % scale_ratio) == 0) && (scale_x < header.width))
                                         {
                                            *pix = pal[*p];
                                            pix++;
                                            scale_x ++;
                                         }
                                       p++;
                                       x++;
                                       count--;
                                    }
                                  if (done & 0x1) p++;
                                  break;
                              }
                         }
                    }
               }
             else
               goto close_file;
          }
     }
   else if ((header.bit_count == 16) || (header.bit_count == 24) || (header.bit_count == 32))
     {
        if (header.comp == 0) // no compression
          {
             position = header.offset;
             if (!region_set && scale_ratio == 1)
               buffer = malloc(image_size + 8); // add 8 for padding to avoid checks
             else
               buffer = malloc(row_size); // scale down is usually set because of memory issue, so read line by line
             if (!buffer)
               {
                  *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                  goto close_file;
               }
             if (!region_set && scale_ratio == 1)
               buffer_end = buffer + image_size;
             else
               buffer_end = buffer + row_size;

             p = buffer;
             if (!region_set && scale_ratio == 1)
               {
                  if (!read_mem(map, fsize, &position, buffer, image_size)) goto close_file;
               }
             else
               {
                  if (region_set) position += row_size * region_y;
                  if (!read_mem(map, fsize, &position, buffer, row_size)) goto close_file;
               }
             if (header.bit_count == 16)
               {
                  unsigned short tmp;

                  pix = surface;
                  for (y = 0; y < header.height; y++)
                    {
                       if (!right_way_up) pix = surface + ((header.height - 1 - y) * header.width);
                       if (region_set) p += 2 * region_x;
                       for (x = 0; x < header.width; x++)
                         {
                            tmp = *((unsigned short *)(p));

                            r = (tmp >> 7) & 0xf8; r |= r >> 5;
                            g = (tmp >> 2) & 0xf8; g |= g >> 5;
                            b = (tmp << 3) & 0xf8; b |= b >> 5;
                            *pix = ARGB_JOIN(0xff, r, g, b);

                            p += 2 * scale_ratio;

                            if (p >= buffer_end) break;
                            pix++;
                         }
                       if (scale_ratio > 1 || region_set)
                         {
                            read_line += scale_ratio;
                            if (read_line >= image_h) break;

                            position += row_size * (scale_ratio - 1);
                            if (!read_mem(map, fsize, &position, buffer, row_size)) goto close_file;
                            p = buffer;
                            buffer_end = buffer + row_size;
                         }
                       else
                         {
                            fix = (int)(((uintptr_t)p) & 0x3);
                            if (fix > 0) p += 4 - fix; // align row read
                            if (p >= buffer_end) break;
                         }
                    }
               }
             else if (header.bit_count == 24)
               {
                  pix = surface;
                  for (y = 0; y < header.height; y++)
                    {
                       if (!right_way_up) pix = surface + ((header.height - 1 - y) * header.width);
                       if (region_set) p += 3 * region_x;
                       for (x = 0; x < header.width; x++)
                         {
                            b = p[0];
                            g = p[1];
                            r = p[2];
                            *pix = ARGB_JOIN(0xff, r, g, b);
                            p += 3 * scale_ratio;
                            if (p >= buffer_end) break;
                            pix++;
                         }
                       if (scale_ratio > 1 || region_set)
                         {
                            read_line += scale_ratio;
                            if (read_line >= image_h) break;

                            position += row_size * (scale_ratio - 1);
                            if (!read_mem(map, fsize, &position, buffer, row_size)) goto close_file;
                            p = buffer;
                            buffer_end = buffer + row_size;
                         }
                       else
                         {
                            fix = (int)(((uintptr_t)p) & 0x3);
                            if (fix > 0) p += 4 - fix; // align row read
                            if (p >= buffer_end) break;
                         }
                    }
               }
             else if (header.bit_count == 32)
               {
                  int none_zero_alpha = 0;
                  pix = surface;
                  for (y = 0; y < header.height; y++)
                    {
                       if (!right_way_up) pix = surface + ((header.height - 1 - y) * header.width);
                       if (region_set) p += 4 * region_x;
                       for (x = 0; x < header.width; x++)
                         {
                            b = p[0];
                            g = p[1];
                            r = p[2];
                            a = p[3];
                            if (a) none_zero_alpha = 1;
                            if (!header.hasa) a = 0xff;
                            *pix = ARGB_JOIN(a, r, g, b);
                            p += 4 * scale_ratio;

                            if (p >= buffer_end) break;
                            pix++;
                         }
                       if (scale_ratio > 1 || region_set)
                         {
                            read_line += scale_ratio;
                            if (read_line >= image_h) break;

                            position += row_size * (scale_ratio - 1);
                            if (!read_mem(map, fsize, &position, buffer, row_size)) goto close_file;
                            p = buffer;
                            buffer_end = buffer + row_size;
                         }
                       else
                         {
                            fix = (int)(((uintptr_t)p) & 0x3);
                            if (fix > 0) p += 4 - fix; // align row read
                            if (p >= buffer_end) break;
                         }
                    }
                  if (!none_zero_alpha)
                    {
                       prop->alpha = 0;
                       if (header.hasa)
                         {
                            unsigned int *pixend = surface + (header.width * header.height);

                            for (pix = surface; pix < pixend; pix++)
                               A_VAL(pix) = 0xff;
                         }
                    }
               }
             else
               goto close_file;
          }
        else if (header.comp == 3) // bit field
          {
             position = header.offset;
             if (!region_set && scale_ratio == 1)
               buffer = malloc(image_size + 8); // add 8 for padding to avoid checks
             else
               buffer = malloc(row_size); // scale down is usually set because of memory issue, so read line by line

             if (!buffer)
               {
                  *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                  goto close_file;
               }
             if (!region_set && scale_ratio == 1)
               buffer_end = buffer + image_size;
             else
               buffer_end = buffer + row_size;

             p = buffer;
             if (!region_set && scale_ratio == 1)
               {
                  if (!read_mem(map, fsize, &position, buffer, image_size)) goto close_file;
               }
             else
               {
                  if (region_set)
                       position += row_size * region_y;
                  if (!read_mem(map, fsize, &position, buffer, row_size)) goto close_file;
               }

             if ((header.bit_count == 16) &&
                 (header.rmask == 0xf800) && (header.gmask == 0x07e0) && (header.bmask == 0x001f)
                 )
               {
                  unsigned short tmp;

                  pix = surface;
                  for (y = 0; y < header.height; y++)
                    {
                       if (!right_way_up) pix = surface + ((header.height - 1 - y) * header.width);
                       if (region_set) p += 2 * region_x;
                       for (x = 0; x < header.width; x++)
                         {
                            tmp = *((unsigned short *)(p));

                            r = (tmp >> 8) & 0xf8; r |= r >> 5;
                            g = (tmp >> 3) & 0xfc; g |= g >> 6;
                            b = (tmp << 3) & 0xf8; b |= b >> 5;
                            *pix = ARGB_JOIN(0xff, r, g, b);

                            p += 2 * scale_ratio;

                            if (p >= buffer_end) break;
                            pix++;
                         }
                       if (scale_ratio > 1 || region_set)
                         {
                            read_line += scale_ratio;
                            if (read_line >= image_h) break;

                            position += row_size * (scale_ratio - 1);
                            if (!read_mem(map, fsize, &position, buffer, row_size)) goto close_file;
                            p = buffer;
                            buffer_end = buffer + row_size;
                         }
                       else
                         {
                            fix = (int)(((uintptr_t)p) & 0x3);
                            if (fix > 0) p += 4 - fix; // align row read
                            if (p >= buffer_end) break;
                         }
                    }
               }
             else if ((header.bit_count == 16) && 
                      (header.rmask == 0x7c00) && (header.gmask == 0x03e0) && (header.bmask == 0x001f)
                     )
               {
                  unsigned short tmp;
                  pix = surface;
                  for (y = 0; y < header.height; y++)
                    {
                       if (!right_way_up) pix = surface + ((header.height - 1 - y) * header.width);
                       if (region_set) p += 2 * region_x;
                       for (x = 0; x < header.width; x++)
                         {
                            tmp = *((unsigned short *)(p));

                            r = (tmp >> 7) & 0xf8; r |= r >> 5;
                            g = (tmp >> 2) & 0xf8; g |= g >> 5;
                            b = (tmp << 3) & 0xf8; b |= b >> 5;
                            *pix = ARGB_JOIN(0xff, r, g, b);
                            p += 2 * scale_ratio;

                            if (p >= buffer_end) break;
                            pix++;
                         }
                       if (scale_ratio > 1 || region_set)
                         {
                            read_line += scale_ratio;
                            if (read_line >= image_h) break;

                            position += row_size * (scale_ratio - 1);
                            if (!read_mem(map, fsize, &position, buffer, row_size)) goto close_file;
                            p = buffer;
                            buffer_end = buffer + row_size;
                         }
                       else
                         {
                            fix = (int)(((uintptr_t)p) & 0x3);
                            if (fix > 0) p += 4 - fix; // align row read
                            if (p >= buffer_end) break;
                         }
                    }
               }
             else if (header.bit_count == 32)
               {
                  pix = surface;
                  for (y = 0; y < header.height; y++)
                    {
                       if (!right_way_up) pix = surface + ((header.height - 1 - y) * header.width);
                       if (region_set) p += 4 * region_x;
                       for (x = 0; x < header.width; x++)
                         {
                            b = p[0];
                            g = p[1];
                            r = p[2];
                            a = p[3];
                            if (!header.hasa) a = 0xff;
                            *pix = ARGB_JOIN(a, r, g, b);

                              p += 4 * scale_ratio;

                            if (p >= buffer_end) break;
                            pix++;
                         }
                       if (scale_ratio > 1 || region_set)
                         {
                            read_line += scale_ratio;
                            if (read_line >= image_h) break;

                            position += row_size * (scale_ratio - 1);
                            if (!read_mem(map, fsize, &position, buffer, row_size)) goto close_file;
                            p = buffer;
                            buffer_end = buffer + row_size;
                         }
                       else
                         {
                            fix = (int)(((uintptr_t)p) & 0x3);
                            if (fix > 0) p += 4 - fix; // align row read
                            if (p >= buffer_end) break;
                         }
                    }
               }
             else
               goto close_file;
          }
        else if (header.comp == 4) // jpeg - only printer drivers
          {
             goto close_file;
          }
        else if (header.comp == 5) // png - only printer drivers
          {
             goto close_file;
          }
        else
          goto close_file;
     }
   else
     goto close_file;

   if (buffer) free(buffer);
   if (scale_surface) free(scale_surface);

   eina_file_map_free(f, map);

   prop->premul = EINA_TRUE;
   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;

 close_file:
   if (buffer) free(buffer);
   if (scale_surface) free(scale_surface);
   if (map) eina_file_map_free(f, map);
   return EINA_FALSE;
}

static Evas_Image_Load_Func evas_image_load_bmp_func =
{
  evas_image_load_file_open_bmp,
  evas_image_load_file_close_bmp,
  evas_image_load_file_head_bmp,
  evas_image_load_file_data_bmp,
  NULL,
  EINA_TRUE,
  EINA_FALSE
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_bmp_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "bmp",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, bmp);

#ifndef EVAS_STATIC_BUILD_BMP
EVAS_EINA_MODULE_DEFINE(image_loader, bmp);
#endif

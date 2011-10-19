#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include <math.h>

#include "evas_common.h"
#include "evas_private.h"

static Eina_Bool evas_image_load_file_head_bmp(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);
static Eina_Bool evas_image_load_file_data_bmp(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);

static Evas_Image_Load_Func evas_image_load_bmp_func =
{
  EINA_TRUE,
  evas_image_load_file_head_bmp,
  evas_image_load_file_data_bmp,
  NULL
};

static int
read_short(FILE *file, short *ret)
{
   unsigned char b[2];
   if (fread(b, sizeof(unsigned char), 2, file) != 2) return 0;
   *ret = (b[1] << 8) | b[0];
   return 1;
}

static int
read_ushort(FILE *file, unsigned short *ret)
{
   unsigned char b[2];
   if (fread(b, sizeof(unsigned char), 2, file) != 2) return 0;
   *ret = (b[1] << 8) | b[0];
   return 1;
}

static int
read_int(FILE *file, int *ret)
{
   unsigned char b[4];
   if (fread(b, sizeof(unsigned char), 4, file) != 4) return 0;
   *ret = ARGB_JOIN(b[3], b[2], b[1], b[0]);
   return 1;
}

static int
read_uint(FILE *file, unsigned int *ret)
{
   unsigned char       b[4];
   if (fread(b, sizeof(unsigned char), 4, file) != 4) return 0;
   *ret = ARGB_JOIN(b[3], b[2], b[1], b[0]);
   return 1;
}

static Eina_Bool
evas_image_load_file_head_bmp(Image_Entry *ie, const char *file, const char *key __UNUSED__, int *error)
{
   FILE *f;
   char buf[4096];
   char hasa = 0;
   int w = 0, h = 0, planes = 0, bit_count = 0,
     image_size = 0, comp = 0, hdpi = 0, vdpi = 0,
     palette_size = -1, important_colors = 0;
   unsigned int offset, head_size, rmask = 0, gmask = 0, bmask = 0, amask = 0;
   unsigned int pal_num = 0;
   int right_way_up = 0;
   int fsize = 0;
   unsigned int bmpsize;
   unsigned short res1, res2;

   f = fopen(file, "rb");
   if (!f)
     {
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
	return EINA_FALSE;
     }

   *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
   fseek(f, 0, SEEK_END);
   fsize = ftell(f);
   fseek(f, 0, SEEK_SET);
   if (fsize < 2) goto close_file;
   
   if (fread(buf, 2, 1, f) != 1) goto close_file;
   if (strncmp(buf, "BM", 2)) goto close_file; // magic number
   *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
   if (!read_uint(f, &bmpsize)) goto close_file;
   if (!read_ushort(f, &res1)) goto close_file;
   if (!read_ushort(f, &res2)) goto close_file;
   if (!read_uint(f, &offset)) goto close_file;
   if (!read_uint(f, &head_size)) goto close_file;
   if (head_size == 12) // OS/2 V1 + Windows 3.0
     {
        short tmp;
        
        if (!read_short(f, &tmp)) goto close_file;
        w = tmp; // width
        if (!read_short(f, &tmp)) goto close_file;
        h = tmp; // height
        if (!read_short(f, &tmp)) goto close_file;
        planes = tmp; // must be 1
        if (!read_short(f, &tmp)) goto close_file;
        bit_count = tmp; // bits per pixel: 1, 4, 8 & 24
     }
   else if (head_size == 64) // OS/2 V2
     {
        short tmp;
        int tmp2;
        
        if (!read_int(f, &tmp2)) goto close_file;
        w = tmp2; // width
        if (!read_int(f, &tmp2)) goto close_file;
        h = tmp2; // height
        if (!read_short(f, &tmp)) goto close_file;
        planes = tmp; // must be 1
        if (!read_short(f, &tmp)) goto close_file;
        bit_count = tmp; // bits per pixel: 1, 4, 8, 16, 24 & 32
        if (!read_int(f, &tmp2)) goto close_file;
        comp = tmp2; // compression method
        if (!read_int(f, &tmp2)) goto close_file;
        image_size = tmp2; // bitmap data size
        if (!read_int(f, &tmp2)) goto close_file;
        hdpi = (tmp2 * 254) / 10000; // horizontal pixels/meter
        if (!read_int(f, &tmp2)) goto close_file;
        vdpi = (tmp2 * 254) / 10000; // vertical pixles/meter
        if (!read_int(f, &tmp2)) goto close_file;
        palette_size = tmp2; // number of palette colors power (2^n - so 0 - 8)
        if (!read_int(f, &tmp2)) goto close_file;
        important_colors = tmp2; // number of important colors - 0 if all
        if (fread(buf, 24, 1, f) != 1) goto close_file; // skip unused header
        if (image_size == 0) image_size = fsize - offset;
     }
   else if (head_size == 40) // Windows 3.0 + (v3)
     {
        short tmp;
        int tmp2;
        
        if (!read_int(f, &tmp2)) goto close_file;
        w = tmp2; // width
        if (!read_int(f, &tmp2)) goto close_file;
        h = tmp2; // height
        if (!read_short(f, &tmp)) goto close_file;
        planes = tmp; // must be 1
        if (!read_short(f, &tmp)) goto close_file;
        bit_count = tmp; // bits per pixel: 1, 4, 8, 16, 24 & 32
        if (!read_int(f, &tmp2)) goto close_file;
        comp = tmp2; // compression method
        if (!read_int(f, &tmp2)) goto close_file;
        image_size = tmp2; // bitmap data size
        if (!read_int(f, &tmp2)) goto close_file;
        hdpi = (tmp2 * 254) / 10000; // horizontal pixels/meter
        if (!read_int(f, &tmp2)) goto close_file;
        vdpi = (tmp2 * 254) / 10000; // vertical pixles/meter
        if (!read_int(f, &tmp2)) goto close_file;
        palette_size = tmp2; // number of palette colors power (2^n - so 0 - 8)
        if (!read_int(f, &tmp2)) goto close_file;
        important_colors = tmp2; // number of important colors - 0 if all
        if (image_size == 0) image_size = fsize - offset;
        if ((comp == 0) && (bit_count == 32)) hasa = 1; // GIMP seems to store it this way
     }
   else if (head_size == 108) // Windows 95/NT4 + (v4)
     {
        short tmp;
        int tmp2;
        
        if (!read_int(f, &tmp2)) goto close_file;
        w = tmp2; // width
        if (!read_int(f, &tmp2)) goto close_file;
        h = tmp2; // height
        if (!read_short(f, &tmp)) goto close_file;
        planes = tmp; // must be 1
        if (!read_short(f, &tmp)) goto close_file;
        bit_count = tmp; // bits per pixel: 1, 4, 8, 16, 24 & 32
        if (!read_int(f, &tmp2)) goto close_file;
        comp = tmp2; // compression method
        if (!read_int(f, &tmp2)) goto close_file;
        image_size = tmp2; // bitmap data size
        if (!read_int(f, &tmp2)) goto close_file;
        hdpi = (tmp2 * 254) / 10000; // horizontal pixels/meter
        if (!read_int(f, &tmp2)) goto close_file;
        vdpi = (tmp2 * 254) / 10000; // vertical pixles/meter
        if (!read_int(f, &tmp2)) goto close_file;
        palette_size = tmp2; // number of palette colors power (2^n - so 0 - 8)
        if (!read_int(f, &tmp2)) goto close_file;
        important_colors = tmp2; // number of important colors - 0 if all
        if (!read_int(f, &tmp2)) goto close_file;
        rmask = tmp2; // red mask
        if (!read_int(f, &tmp2)) goto close_file;
        gmask = tmp2; // green mask
        if (!read_int(f, &tmp2)) goto close_file;
        bmask = tmp2; // blue mask
        if (!read_int(f, &tmp2)) goto close_file;
        amask = tmp2; // alpha mask
        if (fread(buf, 36, 1, f) != 1) goto close_file; // skip unused cie
        if (fread(buf, 12, 1, f) != 1) goto close_file; // skip unused gamma
        if (image_size == 0) image_size = fsize - offset;
        if ((amask) && (bit_count == 32)) hasa = 1;
     }
   else if (head_size == 124) // Windows 98/2000 + (v5)
     {
        short tmp;
        int tmp2;
        
        if (!read_int(f, &tmp2)) goto close_file;
        w = tmp2; // width
        if (!read_int(f, &tmp2)) goto close_file;
        h = tmp2; // height
        if (!read_short(f, &tmp)) goto close_file;
        planes = tmp; // must be 1
        if (!read_short(f, &tmp)) goto close_file;
        bit_count = tmp; // bits per pixel: 1, 4, 8, 16, 24 & 32
        if (!read_int(f, &tmp2)) goto close_file;
        comp = tmp2; // compression method
        if (!read_int(f, &tmp2)) goto close_file;
        image_size = tmp2; // bitmap data size
        if (!read_int(f, &tmp2)) goto close_file;
        hdpi = (tmp2 * 254) / 10000; // horizontal pixels/meter
        if (!read_int(f, &tmp2)) goto close_file;
        vdpi = (tmp2 * 254) / 10000; // vertical pixles/meter
        if (!read_int(f, &tmp2)) goto close_file;
        palette_size = tmp2; // number of palette colors power (2^n - so 0 - 8)
        if (!read_int(f, &tmp2)) goto close_file;
        important_colors = tmp2; // number of important colors - 0 if all
        if (!read_int(f, &tmp2)) goto close_file;
        rmask = tmp2; // red mask
        if (!read_int(f, &tmp2)) goto close_file;
        gmask = tmp2; // green mask
        if (!read_int(f, &tmp2)) goto close_file;
        bmask = tmp2; // blue mask
        if (!read_int(f, &tmp2)) goto close_file;
        amask = tmp2; // alpha mask
        if (fread(buf, 36, 1, f) != 1) goto close_file; // skip unused cie
        if (fread(buf, 12, 1, f) != 1) goto close_file; // skip unused gamma
        if (fread(buf, 16, 1, f) != 1) goto close_file; // skip others
        if (image_size == 0) image_size = fsize - offset;
        if ((amask) && (bit_count == 32)) hasa = 1;
     }
   else
     goto close_file;

   if (h < 0)
     {
        h = -h;
        right_way_up = 1;
     }
   
   if ((w < 1) || (h < 1) || (w > IMG_MAX_SIZE) || (h > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(w, h))
     {
        if (IMG_TOO_BIG(w, h))
          *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        else
          *error = EVAS_LOAD_ERROR_GENERIC;
	goto close_file;
     }
   /* It is not bad idea that bmp loader support scale down decoding 
    * because of memory issue in mobile world.*/
   if (ie->load_opts.scale_down_by > 1)
     {
        w /= ie->load_opts.scale_down_by;
        h /= ie->load_opts.scale_down_by;
     }

   if (bit_count < 16)
     {
        if ((palette_size < 0) || (palette_size > 256)) pal_num = 256;
        else pal_num = palette_size;
        if (bit_count == 1)
          {
             if (comp == 0) // no compression
               {
               }
             else
               goto close_file;
          }
        else if (bit_count == 4)
          {
             if (comp == 0) // no compression
               {
               }
             else if (comp == 2) // rle 4bit/pixel
               {
               }
             else
               goto close_file;
          }
        else if (bit_count == 8)
          {
             if (comp == 0) // no compression
               {
               }
             else if (comp == 1) // rle 8bit/pixel
               {
               }
             else
               goto close_file;
          }
     }
   else if ((bit_count == 16) || (bit_count == 24) || (bit_count == 32))
     {
        if (comp == 0) // no compression
          {
             // handled
          }
        else if (comp == 3) // bit field
          {
             // handled
          }
        else if (comp == 4) // jpeg - only printer drivers
          goto close_file;
        else if (comp == 3) // png - only printer drivers
          goto close_file;
        else
          goto close_file;
     }
   else
     goto close_file;

   ie->w = w;
   ie->h = h;
   if (hasa) ie->flags.alpha = 1;
   
   fclose(f);
   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;

 close_file:
   fclose(f);
   return EINA_FALSE;
}

static Eina_Bool
evas_image_load_file_data_bmp(Image_Entry *ie, const char *file, const char *key __UNUSED__, int *error)
{
   FILE *f;
   char buf[4096];
   unsigned char *buffer = NULL, *buffer_end = NULL, *p;
   char hasa = 0;
   int x = 0, y = 0, w = 0, h = 0, planes = 0, bit_count = 0, image_size = 0,
     comp = 0, hdpi = 0, vdpi = 0, palette_size = -1, important_colors = 0;
   unsigned int offset = 0, head_size = 0;
   unsigned int *pal = NULL, pal_num = 0, *pix = NULL, *surface = NULL, fix,
     rmask = 0, gmask = 0, bmask = 0, amask = 0;
   int right_way_up = 0;
   unsigned char r, g, b, a;
   int fsize = 0;
   unsigned int bmpsize;
   unsigned short res1, res2;

   /* for scale decoding */
   unsigned int *scale_surface = NULL, *scale_pix = NULL;
   int scale_ratio = 1, image_w = 0, image_h = 0;
   int row_size = 0; /* Row size is rounded up to a multiple of 4bytes */
   int read_line = 0; /* total read line */


   f = fopen(file, "rb");
   if (!f)
     {
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
	return EINA_FALSE;
     }
   
   *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
   fseek(f, 0, SEEK_END);
   fsize = ftell(f);
   fseek(f, 0, SEEK_SET);
   if (fsize < 2) goto close_file;
   
   if (fread(buf, 2, 1, f) != 1) goto close_file;
   if (strncmp(buf, "BM", 2)) goto close_file; // magic number
   *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
   if (!read_uint(f, &bmpsize)) goto close_file;
   if (!read_ushort(f, &res1)) goto close_file;
   if (!read_ushort(f, &res2)) goto close_file;
   if (!read_uint(f, &offset)) goto close_file;
   if (!read_uint(f, &head_size)) goto close_file;
   image_size = fsize - offset;
   if (image_size < 1) goto close_file;
   
   if (head_size == 12) // OS/2 V1 + Windows 3.0
     {
        short tmp;
        
        if (!read_short(f, &tmp)) goto close_file;
        w = tmp; // width
        if (!read_short(f, &tmp)) goto close_file;
        h = tmp; // height
        if (!read_short(f, &tmp)) goto close_file;
        planes = tmp; // must be 1
        if (!read_short(f, &tmp)) goto close_file;
        bit_count = tmp; // bits per pixel: 1, 4, 8 & 24
     }
   else if (head_size == 64) // OS/2 V2
     {
        short tmp;
        int tmp2;
        
        if (!read_int(f, &tmp2)) goto close_file;
        w = tmp2; // width
        if (!read_int(f, &tmp2)) goto close_file;
        h = tmp2; // height
        if (!read_short(f, &tmp)) goto close_file;
        planes = tmp; // must be 1
        if (!read_short(f, &tmp)) goto close_file;
        bit_count = tmp; // bits per pixel: 1, 4, 8, 16, 24 & 32
        if (!read_int(f, &tmp2)) goto close_file;
        comp = tmp2; // compression method
        if (!read_int(f, &tmp2)) goto close_file;
        image_size = tmp2; // bitmap data size
        if (!read_int(f, &tmp2)) goto close_file;
        hdpi = (tmp2 * 254) / 10000; // horizontal pixels/meter
        if (!read_int(f, &tmp2)) goto close_file;
        vdpi = (tmp2 * 254) / 10000; // vertical pixles/meter
        if (!read_int(f, &tmp2)) goto close_file;
        palette_size = tmp2; // number of palette colors power (2^n - so 0 - 8)
        if (!read_int(f, &tmp2)) goto close_file;
        important_colors = tmp2; // number of important colors - 0 if all
        if (fread(buf, 24, 1, f) != 1) goto close_file; // skip unused header
        if (image_size == 0) image_size = fsize - offset;
     }
   else if (head_size == 40) // Windows 3.0 + (v3)
     {
        short tmp;
        int tmp2;
        
        if (!read_int(f, &tmp2)) goto close_file;
        w = tmp2; // width
        if (!read_int(f, &tmp2)) goto close_file;
        h = tmp2; // height
        if (!read_short(f, &tmp)) goto close_file;
        planes = tmp; // must be 1
        if (!read_short(f, &tmp)) goto close_file;
        bit_count = tmp; // bits per pixel: 1, 4, 8, 16, 24 & 32
        if (!read_int(f, &tmp2)) goto close_file;
        comp = tmp2; // compression method
        if (!read_int(f, &tmp2)) goto close_file;
        image_size = tmp2; // bitmap data size
        if (!read_int(f, &tmp2)) goto close_file;
        hdpi = (tmp2 * 254) / 10000; // horizontal pixels/meter
        if (!read_int(f, &tmp2)) goto close_file;
        vdpi = (tmp2 * 254) / 10000; // vertical pixles/meter
        if (!read_int(f, &tmp2)) goto close_file;
        palette_size = tmp2; // number of palette colors power (2^n - so 0 - 8)
        if (!read_int(f, &tmp2)) goto close_file;
        important_colors = tmp2; // number of important colors - 0 if all
        if (image_size == 0) image_size = fsize - offset;
        if ((comp == 0) && (bit_count == 32)) hasa = 1; // GIMP seems to store it this way
     }
   else if (head_size == 108) // Windows 95/NT4 + (v4)
     {
        short tmp;
        int tmp2;
        
        if (!read_int(f, &tmp2)) goto close_file;
        w = tmp2; // width
        if (!read_int(f, &tmp2)) goto close_file;
        h = tmp2; // height
        if (!read_short(f, &tmp)) goto close_file;
        planes = tmp; // must be 1
        if (!read_short(f, &tmp)) goto close_file;
        bit_count = tmp; // bits per pixel: 1, 4, 8, 16, 24 & 32
        if (!read_int(f, &tmp2)) goto close_file;
        comp = tmp2; // compression method
        if (!read_int(f, &tmp2)) goto close_file;
        image_size = tmp2; // bitmap data size
        if (!read_int(f, &tmp2)) goto close_file;
        hdpi = (tmp2 * 254) / 10000; // horizontal pixels/meter
        if (!read_int(f, &tmp2)) goto close_file;
        vdpi = (tmp2 * 254) / 10000; // vertical pixles/meter
        if (!read_int(f, &tmp2)) goto close_file;
        palette_size = tmp2; // number of palette colors power (2^n - so 0 - 8)
        if (!read_int(f, &tmp2)) goto close_file;
        important_colors = tmp2; // number of important colors - 0 if all
        if (!read_int(f, &tmp2)) goto close_file;
        rmask = tmp2; // red mask
        if (!read_int(f, &tmp2)) goto close_file;
        gmask = tmp2; // green mask
        if (!read_int(f, &tmp2)) goto close_file;
        bmask = tmp2; // blue mask
        if (!read_int(f, &tmp2)) goto close_file;
        amask = tmp2; // alpha mask
        if (fread(buf, 36, 1, f) != 1) goto close_file; // skip unused cie
        if (fread(buf, 12, 1, f) != 1) goto close_file; // skip unused gamma
        if (image_size == 0) image_size = fsize - offset;
        if ((amask) && (bit_count == 32)) hasa = 1;
     }
   else if (head_size == 124) // Windows 98/2000 + (v5)
     {
        short tmp;
        int tmp2;
        
        if (!read_int(f, &tmp2)) goto close_file;
        w = tmp2; // width
        if (!read_int(f, &tmp2)) goto close_file;
        h = tmp2; // height
        if (!read_short(f, &tmp)) goto close_file;
        planes = tmp; // must be 1
        if (!read_short(f, &tmp)) goto close_file;
        bit_count = tmp; // bits per pixel: 1, 4, 8, 16, 24 & 32
        if (!read_int(f, &tmp2)) goto close_file;
        comp = tmp2; // compression method
        if (!read_int(f, &tmp2)) goto close_file;
        image_size = tmp2; // bitmap data size
        if (!read_int(f, &tmp2)) goto close_file;
        hdpi = (tmp2 * 254) / 10000; // horizontal pixels/meter
        if (!read_int(f, &tmp2)) goto close_file;
        vdpi = (tmp2 * 254) / 10000; // vertical pixles/meter
        if (!read_int(f, &tmp2)) goto close_file;
        palette_size = tmp2; // number of palette colors power (2^n - so 0 - 8)
        if (!read_int(f, &tmp2)) goto close_file;
        important_colors = tmp2; // number of important colors - 0 if all
        if (!read_int(f, &tmp2)) goto close_file;
        rmask = tmp2; // red mask
        if (!read_int(f, &tmp2)) goto close_file;
        gmask = tmp2; // green mask
        if (!read_int(f, &tmp2)) goto close_file;
        bmask = tmp2; // blue mask
        if (!read_int(f, &tmp2)) goto close_file;
        amask = tmp2; // alpha mask
        if (fread(buf, 36, 1, f) != 1) goto close_file; // skip unused cie
        if (fread(buf, 12, 1, f) != 1) goto close_file; // skip unused gamma
        if (fread(buf, 16, 1, f) != 1) goto close_file; // skip others
        if (image_size == 0) image_size = fsize - offset;
        if ((amask) && (bit_count == 32)) hasa = 1;
     }
   else
     goto close_file;

   if (h < 0)
     {
        h = -h;
        right_way_up = 1;
     }
   if ((w < 1) || (h < 1) || (w > IMG_MAX_SIZE) || (h > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(w, h))
     {
        if (IMG_TOO_BIG(w, h))
          *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        else
          *error = EVAS_LOAD_ERROR_GENERIC;
	goto close_file;
     }
   /* It is not bad idea that bmp loader support scale down decoding 
    * because of memory issue in mobile world. */
   if (ie->load_opts.scale_down_by > 1)
     scale_ratio = ie->load_opts.scale_down_by;
   image_w = w;
   image_h = h;

   if (scale_ratio > 1)
     {
        w /= scale_ratio;
        h /= scale_ratio;

        if ((w < 1) || (h < 1) )
          {
             *error = EVAS_LOAD_ERROR_GENERIC;
             goto close_file;
          }
     }

   if ((w != (int)ie->w) || (h != (int)ie->h))
     {
	*error = EVAS_LOAD_ERROR_GENERIC;
	goto close_file;
     }
   evas_cache_image_surface_alloc(ie, ie->w, ie->h);
   surface = evas_cache_image_pixels(ie);
   if (!surface)
     {
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	goto close_file;
     }

   row_size = ceil((double)(image_w * bit_count) / 32) * 4;

   if (bit_count < 16)
     {
        unsigned int i;

        if (bit_count == 1)
          {
             if ((palette_size <= 0) || (palette_size > 2)) pal_num = 2;
             else pal_num = palette_size;
          }
        else if (bit_count == 4)
          {
             if ((palette_size <= 0) || (palette_size > 16)) pal_num = 16;
             else pal_num = palette_size;
          }
        else if (bit_count == 8)
          {
             if ((palette_size <= 0) || (palette_size > 256)) pal_num = 256;
             else pal_num = palette_size;
          }
        pal = alloca(256 * 4);
        for (i = 0; i < pal_num; i++)
          {
             if (fread(&b, 1, 1, f) != 1) goto close_file;
             if (fread(&g, 1, 1, f) != 1) goto close_file;
             if (fread(&r, 1, 1, f) != 1) goto close_file;
             if ((head_size != 12) /*&& (palette_size != 0)*/)
               { // OS/2 V1 doesn't do the pad byte
                  if (fread(&a, 1, 1, f) != 1) goto close_file;
               }
             a = 0xff; // fillin a as solid for paletted images
             pal[i] = ARGB_JOIN(a, r, g, b);
          }
        fseek(f, offset, SEEK_SET);

        if ((scale_ratio == 1) || (comp !=0))
          buffer = malloc(image_size + 8); // add 8 for padding to avoid checks
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
        if ((scale_ratio == 1) || (comp !=0))
          buffer_end = buffer + image_size;
        else
          buffer_end = buffer + row_size;
        p = buffer;

        if ((scale_ratio == 1) || (comp !=0))
          {
             if (fread(buffer, image_size, 1, f) != 1) goto close_file;
          }
        else
          {
             if (fread(buffer, row_size, 1, f) != 1) goto close_file;
          }

        if (bit_count == 1)
          {
             if (comp == 0) // no compression
               {
                  pix = surface;

                  for (y = 0; y < h; y++)
                    {
                       if (!right_way_up) pix = surface + ((h - 1 - y) * w);
                       if (scale_ratio > 1) pix = scale_surface; // one line decoding

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

                       if (scale_ratio > 1)
                         {
                            if (!right_way_up) scale_pix = surface + ((h - 1 - y) * w);
                            else scale_pix = surface + (y * w);

                            pix = scale_surface;
                            for (x = 0; x < w; x++)
                              {
                                 *scale_pix = *pix;
                                 scale_pix ++;
                                 pix += scale_ratio;
                              }
                            read_line += scale_ratio;
                            if (read_line >= image_h) break;

                            fseek(f, row_size * (scale_ratio - 1), SEEK_CUR);
                            if (fread(buffer, row_size, 1, f) != 1) goto close_file;
                            p = buffer;
                            buffer_end = buffer + row_size;
                         }
                       else
                         {
                            if ((x & 0x7) != 0) p++;
                            fix = (int)(((unsigned long)p) & 0x3);
                            if (fix > 0) p += 4 - fix; // align row read
                            if (p >= buffer_end) break;
                         }
                    }
               }
             else
               goto close_file;
          }
        else if (bit_count == 4)
          {
             if (comp == 0) // no compression
               {
                  pix = surface;
                  for (y = 0; y < h; y++)
                    {
                       if (!right_way_up) pix = surface + ((h - 1 - y) * w);
                       if (scale_ratio > 1) pix = scale_surface; // one line decoding
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
                       if (scale_ratio > 1)
                         {
                            if (!right_way_up) scale_pix = surface + ((h - 1 - y) * w);
                            else scale_pix = surface + (y * w);

                            pix = scale_surface;
                            for (x = 0; x < w; x++)
                              {
                                 *scale_pix = *pix;
                                 scale_pix ++;
                                 pix += scale_ratio;
                              }
                            read_line += scale_ratio;
                            if (read_line >= image_h) break;

                            fseek(f, row_size * (scale_ratio - 1), SEEK_CUR);
                            if (fread(buffer, row_size, 1, f) != 1) goto close_file;
                            p = buffer;
                            buffer_end = buffer + row_size;
                         }
                       else
                         {
                            if ((x & 0x1) != 0) p++;
                            fix = (int)(((unsigned long)p) & 0x3);
                            if (fix > 0) p += 4 - fix; // align row read
                            if (p >= buffer_end) break;
                         }
                    }
               }
             else if (comp == 2) // rle 4bit/pixel
               {
                  int count = 0, done = 0, wpad;
                  int scale_x = 0, scale_y = 0;
                  Eina_Bool scale_down_line = EINA_TRUE;

                  pix = surface;
                  if (!right_way_up) pix = surface + ((h - 1 - y) * w);
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
                                           if (x < w)
                                             {
                                                if (((x % scale_ratio) == 0) && (scale_x < w))
                                                  {
                                                     *pix = col1;
                                                     pix++;
                                                     scale_x++;
                                                  }
                                                x++;
                                             }
                                           if (x < w)
                                             {
                                                if (((x % scale_ratio) == 0) && (scale_x < w))
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
                                           if (((x % scale_ratio) == 0) && (scale_x < w))
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
                                  if ((y % scale_ratio) == 0)
                                    {
                                       scale_y++;
                                       scale_down_line = EINA_TRUE;
                                       if (!right_way_up)
                                         pix = surface + ((h - 1 - scale_y) * w);
                                       else
                                         pix = surface + (scale_y * w);
                                    }
                                  else
                                    scale_down_line = EINA_FALSE;
                                  if (scale_y >= h)
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
                                  if ((scale_x >= w) || (scale_y >= h))
                                    {
                                       p = buffer_end;
                                    }
                                  if (!right_way_up)
                                    pix = surface + scale_x + ((h - 1 - scale_y) * w);
                                  else
                                    pix = surface + scale_x + (scale_y * w);
                                  p += 4;
                                  break;
                               default:
                                  count = p[1];
                                  if (((p + count) > buffer_end) ||
                                      ((x + count) > w))
                                    {
                                       p = buffer_end;
                                       break;
                                    }
                                  p += 2;
                                  done = count;
                                  count /= 2;
                                  while (count > 0)
                                    {
                                       if (((x % scale_ratio) == 0) && (scale_x < w))
                                         {
                                            *pix = pal[*p >> 4];
                                            pix++;
                                            scale_x++;
                                         }
                                       x++;
                                       if (((x % scale_ratio) == 0) && (scale_x < w))
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
                                       if (((x % scale_ratio) == 0) && (scale_x < w))
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
        else if (bit_count == 8)
          {
             if (comp == 0) // no compression
               {
                  pix = surface;
                  for (y = 0; y < h; y++)
                    {
                       if (!right_way_up) pix = surface + ((h - 1 - y) * w);
                       for (x = 0; x < w; x++)
                         {
                            *pix = pal[*p];
                            p += scale_ratio;
                            if (p >= buffer_end) break;
                            pix++;
                         }
                       if (scale_ratio > 1)
                         {
                            read_line += scale_ratio;
                            if (read_line >= image_h) break;

                            fseek(f, row_size * (scale_ratio - 1), SEEK_CUR);
                            if (fread(buffer, row_size, 1, f) != 1) goto close_file;
                            p = buffer;
                            buffer_end = buffer + row_size;
                         }
                       else
                         {
                            fix = (int)(((unsigned long)p) & 0x3);
                            if (fix > 0) p += 4 - fix; // align row read
                            if (p >= buffer_end) break;
                         }
                    }
               }
             else if (comp == 1) // rle 8bit/pixel
               {
                  int count = 0, done = 0;
                  int scale_x = 0, scale_y = 0;
                  Eina_Bool scale_down_line = EINA_TRUE;

                  pix = surface;
                  if (!right_way_up) pix = surface + ((h - 1 - y) * w);

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
                                           if (((x % scale_ratio) == 0) && (scale_x < w))
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
                                  if ((y % scale_ratio) == 0)
                                    {
                                       scale_y++;
                                       scale_down_line = EINA_TRUE;
                                       if (!right_way_up)
                                         pix = surface + ((h - 1 - scale_y) * w);
                                       else
                                         pix = surface + (scale_y * w);
                                    }
                                  else
                                    scale_down_line = EINA_FALSE;

                                  if (scale_y >= h)
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
                                  if ((scale_x >= w) || (scale_y >= h))
                                    {
                                       p = buffer_end;
                                    }
                                  if (!right_way_up)
                                    pix = surface + scale_x + ((h - 1 - scale_y) * w);
                                  else
                                    pix = surface + scale_x + (scale_y * w);
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
                                       if (((x % scale_ratio) == 0) && (scale_x < w))
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
   else if ((bit_count == 16) || (bit_count == 24) || (bit_count == 32))
     {
        if (comp == 0) // no compression
          {
             fseek(f, offset, SEEK_SET);
             if (scale_ratio == 1)
               buffer = malloc(image_size + 8); // add 8 for padding to avoid checks
             else
               buffer = malloc(row_size); // scale down is usually set because of memory issue, so read line by line
             if (!buffer)
               {
                  *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                  goto close_file;
               }
             if (scale_ratio == 1)
               buffer_end = buffer + image_size;
             else
               buffer_end = buffer + row_size;

             p = buffer;
             if (scale_ratio == 1)
               {
                  if (fread(buffer, image_size, 1, f) != 1) goto close_file;
               }
             else
               {
                  if (fread(buffer, row_size, 1, f) != 1) goto close_file;
               }
             if (bit_count == 16)
               {
                  unsigned short tmp;

                  pix = surface;
                  for (y = 0; y < h; y++)
                    {
                       if (!right_way_up) pix = surface + ((h - 1 - y) * w);
                       for (x = 0; x < w; x++)
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
                       if (scale_ratio > 1)
                         {
                            read_line += scale_ratio;
                            if (read_line >= image_h) break;

                            fseek(f, row_size * (scale_ratio - 1), SEEK_CUR);
                            if (fread(buffer, row_size, 1, f) != 1) goto close_file;
                            p = buffer;
                            buffer_end = buffer + row_size;
                         }
                       else
                         {
                            fix = (int)(((unsigned long)p) & 0x3);
                            if (fix > 0) p += 4 - fix; // align row read
                            if (p >= buffer_end) break;
                         }
                    }
               }
             else if (bit_count == 24)
               {
                  pix = surface;
                  for (y = 0; y < h; y++)
                    {
                       if (!right_way_up) pix = surface + ((h - 1 - y) * w);
                       for (x = 0; x < w; x++)
                         {
                            b = p[0];
                            g = p[1];
                            r = p[2];
                            *pix = ARGB_JOIN(0xff, r, g, b);
                            p += 3 * scale_ratio;
                            if (p >= buffer_end) break;
                            pix++;
                         }
                       if (scale_ratio > 1)
                         {
                            read_line += scale_ratio;
                            if (read_line >= image_h) break;

                            fseek(f, row_size * (scale_ratio - 1), SEEK_CUR);
                            if (fread(buffer, row_size, 1, f) != 1) goto close_file;
                            p = buffer;
                            buffer_end = buffer + row_size;
                         }
                       else
                         {
                            fix = (int)(((unsigned long)p) & 0x3);
                            if (fix > 0) p += 4 - fix; // align row read
                            if (p >= buffer_end) break;
                         }
                    }
               }
             else if (bit_count == 32)
               {
                  int none_zero_alpha = 0;
                  pix = surface;
                  for (y = 0; y < h; y++)
                    {
                       if (!right_way_up) pix = surface + ((h - 1 - y) * w);
                       for (x = 0; x < w; x++)
                         {
                            b = p[0];
                            g = p[1];
                            r = p[2];
                            a = p[3];
                            if (a) none_zero_alpha = 1;
                            if (!hasa) a = 0xff;
                            *pix = ARGB_JOIN(a, r, g, b);
                            p += 4 * scale_ratio;

                            if (p >= buffer_end) break;
                            pix++;
                         }
                       if (scale_ratio > 1)
                         {
                            read_line += scale_ratio;
                            if (read_line >= image_h) break;

                            fseek(f, row_size * (scale_ratio - 1), SEEK_CUR);
                            if (fread(buffer, row_size, 1, f) != 1) goto close_file;
                            p = buffer;
                            buffer_end = buffer + row_size;
                         }
                       else
                         {
                            fix = (int)(((unsigned long)p) & 0x3);
                            if (fix > 0) p += 4 - fix; // align row read
                            if (p >= buffer_end) break;
                         }
                    }
                  if (!none_zero_alpha)
                    {
                       ie->flags.alpha = 0;
                       if (hasa)
                         {
                            unsigned int *pixend = surface + (w * h);
                            
                            for (pix = surface; pix < pixend; pix++)
                               A_VAL(pix) = 0xff;
                         }
                    }
               }
             else
               goto close_file;
          }
        else if (comp == 3) // bit field
          {
             if (!read_uint(f, &rmask)) goto close_file;
             if (!read_uint(f, &gmask)) goto close_file;
             if (!read_uint(f, &bmask)) goto close_file;

             fseek(f, offset, SEEK_SET);
             if (scale_ratio == 1)
               buffer = malloc(image_size + 8); // add 8 for padding to avoid checks
             else
               buffer = malloc(row_size); // scale down is usually set because of memory issue, so read line by line

             if (!buffer)
               {
                  *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                  goto close_file;
               }
             if (scale_ratio == 1)
               buffer_end = buffer + image_size;
             else
               buffer_end = buffer + row_size;

             p = buffer;
             if (scale_ratio == 1)
               {
                  if (fread(buffer, image_size, 1, f) != 1) goto close_file;
               }
             else
               {
                  if (fread(buffer, row_size, 1, f) != 1) goto close_file;
               }

             if ((bit_count == 16) && 
                 (rmask == 0xf800) && (gmask == 0x07e0) && (bmask == 0x001f)
                 )
               {
                  unsigned short tmp;

                  pix = surface;
                  for (y = 0; y < h; y++)
                    {
                       if (!right_way_up) pix = surface + ((h - 1 - y) * w);
                       for (x = 0; x < w; x++)
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
                       if (scale_ratio > 1)
                         {
                            read_line += scale_ratio;
                            if (read_line >= image_h) break;
                            fseek(f, row_size * (scale_ratio - 1), SEEK_CUR);
                            if (fread(buffer, row_size, 1, f) != 1) goto close_file;
                            p = buffer;
                            buffer_end = buffer + row_size;
                         }
                       else
                         {
                            fix = (int)(((unsigned long)p) & 0x3);
                            if (fix > 0) p += 4 - fix; // align row read
                            if (p >= buffer_end) break;
                         }
                    }
               }
             else if ((bit_count == 16) && 
                      (rmask == 0x7c00) && (gmask == 0x03e0) && (bmask == 0x001f)
                     )
               {
                  unsigned short tmp;
                  pix = surface;
                  for (y = 0; y < h; y++)
                    {
                       if (!right_way_up) pix = surface + ((h - 1 - y) * w);
                       for (x = 0; x < w; x++)
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
                       if (scale_ratio > 1)
                         {
                            read_line += scale_ratio;
                            if (read_line >= image_h) break;
                            fseek(f, row_size * (scale_ratio - 1), SEEK_CUR);
                            if (fread(buffer, row_size, 1, f) != 1) goto close_file;
                            p = buffer;
                            buffer_end = buffer + row_size;
                         }
                       else
                         {
                            fix = (int)(((unsigned long)p) & 0x3);
                            if (fix > 0) p += 4 - fix; // align row read
                            if (p >= buffer_end) break;
                         }
                    }
               }
             else if (bit_count == 32)
               {
                  pix = surface;
                  for (y = 0; y < h; y++)
                    {
                       if (!right_way_up) pix = surface + ((h - 1 - y) * w);
                       for (x = 0; x < w; x++)
                         {
                            b = p[0];
                            g = p[1];
                            r = p[2];
                            a = p[3];
                            if (!hasa) a = 0xff;
                            *pix = ARGB_JOIN(a, r, g, b);

                              p += 4 * scale_ratio;

                            if (p >= buffer_end) break;
                            pix++;
                         }
                       if (scale_ratio > 1)
                         {
                            read_line += scale_ratio;
                            if (read_line >= image_h) break;
                            fseek(f, row_size * (scale_ratio - 1), SEEK_CUR);
                            if (fread(buffer, row_size, 1, f) != 1) goto close_file;
                            p = buffer;
                            buffer_end = buffer + row_size;
                         }
                       else
                         {
                            fix = (int)(((unsigned long)p) & 0x3);
                            if (fix > 0) p += 4 - fix; // align row read
                            if (p >= buffer_end) break;
                         }
                    }
               }
             else
               goto close_file;
          }
        else if (comp == 4) // jpeg - only printer drivers
          {
             goto close_file;
          }
        else if (comp == 3) // png - only printer drivers
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
   fclose(f);

   evas_common_image_premul(ie);
   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;

 close_file:
   if (buffer) free(buffer);
   if (scale_surface) free(scale_surface);
   fclose(f);
   return EINA_FALSE;
}

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_bmp_func);
   return 1;
}

static void
module_close(Evas_Module *em __UNUSED__)
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

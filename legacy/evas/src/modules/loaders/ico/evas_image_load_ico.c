#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "evas_common.h"
#include "evas_private.h"

static Eina_Bool evas_image_load_file_head_ico(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);
static Eina_Bool evas_image_load_file_data_ico(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);

static Evas_Image_Load_Func evas_image_load_ico_func =
{
  EINA_TRUE,
  evas_image_load_file_head_ico,
  evas_image_load_file_data_ico,
  NULL
};

static int
read_ushort(FILE *file, unsigned short *ret)
{
   unsigned char b[2];
   if (fread(b, sizeof(unsigned char), 2, file) != 2) return 0;
   *ret = (b[1] << 8) | b[0];
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

enum
{
   SMALLEST, 
   BIGGEST, 
   SMALLER, 
   BIGGER
};

enum
{
   ICON = 1,
   CURSOR = 2
};

static Eina_Bool
evas_image_load_file_head_ico(Image_Entry *ie, const char *file, const char *key, int *error)
{
   unsigned short word;
   unsigned char byte;
   FILE *f;
   int wanted_w = 0, wanted_h = 0, w, h, cols, i, planes = 0,
      hot_x = 0, hot_y = 0, bpp = 0, pdelta, search = -1, have_choice = 0,
      hasa = 1;
   unsigned int bmoffset, bmsize, fsize;
   unsigned short reserved, type, count;
   struct {
      int pdelta;
      int w, h;
      int cols;
      int bpp, planes;
      int hot_x, hot_y;
      unsigned int bmoffset, bmsize;
   } chosen = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

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
   if (fsize < (6 + 16 + 40)) goto close_file;

   // key:
   //   NULL == highest res
   //   biggest == highest res
   //   smallest == lowest res
   //   
   //   smaller == next size SMALLER than load opts WxH (if possible)
   //   bigger == next size BIGGER than load opts WxH (if possible)
   //   more ?

   search = BIGGEST;
   if ((ie->load_opts.w > 0) && (ie->load_opts.h > 0))
     {
        wanted_w = ie->load_opts.w;
        wanted_h = ie->load_opts.h;
        search = SMALLER;
     }
   
   if (!read_ushort(f, &reserved)) goto close_file;
   if (!read_ushort(f, &type)) goto close_file;
   if (!read_ushort(f, &count)) goto close_file;
   if (!((reserved == 0) && 
         ((type == ICON) || (type == CURSOR)) && (count > 0)))
      goto close_file;
   *error = EVAS_LOAD_ERROR_CORRUPT_FILE;

   if (key)
     {
        if (!strcmp(key, "biggest"))
          {
             wanted_w = 0;
             wanted_h = 0;
             search = BIGGEST;
             chosen.pdelta = 0;
          }
        else if (!strcmp(key, "smallest"))
          {
             wanted_w = 1;
             wanted_h = 1;
             search = SMALLEST;
             chosen.pdelta = 0x7fffffff;
          }
        else if (!strcmp(key, "smaller"))
          {
             chosen.pdelta = 0x7fffffff;
             search = SMALLER;
          }
        else if (!strcmp(key, "bigger"))
          {
             chosen.pdelta = 0x7fffffff;
             search = BIGGER;
          }
     }
   for (i = 0; i < count; i++)
     {
        if (fread(&byte, 1, 1, f) != 1) goto close_file;
        w = byte;
        if (w <= 0) w = 256;
        if (fread(&byte, 1, 1, f) != 1) goto close_file;
        h = byte;
        if (h <= 0) h = 256;
        if (fread(&byte, 1, 1, f) != 1) goto close_file;
        cols = byte;
        if (cols <= 0) cols = 256;
        if (fread(&byte, 1, 1, f) != 1) goto close_file;
        if (!read_ushort(f, &word)) goto close_file;
        if (type == CURSOR) planes = word;
        else hot_x = word;
        if (!read_ushort(f, &word)) goto close_file;
        if (type == CURSOR) bpp = word;
        else hot_y = word;
        if (!read_uint(f, &bmsize)) goto close_file;
        if (!read_uint(f, &bmoffset)) goto close_file;
        if ((bmsize <= 0) || (bmoffset <= 0) || (bmoffset >= fsize)) goto close_file;
        if (search == BIGGEST)
          {
             pdelta = w * h;
             if ((!have_choice) ||
                 ((pdelta >= chosen.pdelta) &&
                     (((bpp >= 3) && (bpp >= chosen.bpp)) ||
                         ((bpp < 3) && (cols >= chosen.cols)))))
               {
                  have_choice = 1;
                  chosen.pdelta = pdelta;
                  chosen.w = w;
                  chosen.h = h;
                  chosen.cols = cols;
                  chosen.bpp = bpp;
                  chosen.planes = planes;
                  chosen.bmsize = bmsize;
                  chosen.bmoffset = bmoffset;
               }
          }
        else
          {
             if (search == SMALLEST)
               {
                  pdelta = w * h;
                  if ((!have_choice) ||
                       ((pdelta <= chosen.pdelta) &&
                           (((bpp >= 3) && (bpp >= chosen.bpp)) ||
                               ((bpp < 3) && (cols >= chosen.cols)))))
                    {
                       have_choice = 1;
                       chosen.pdelta = pdelta;
                       chosen.w = w;
                       chosen.h = h;
                       chosen.cols = cols;
                       chosen.bpp = bpp;
                       chosen.planes = planes;
                       chosen.bmsize = bmsize;
                       chosen.bmoffset = bmoffset;
                    }
               }
             else if (search == SMALLER)
               {
                  pdelta = (wanted_w * wanted_h) - (w * h);
                  if ((!have_choice) ||
                      ((w <= wanted_w) && (h <= wanted_h) &&
                          (pdelta <= chosen.pdelta) &&
                          (((bpp >= 3) && (bpp >= chosen.bpp)) ||
                              ((bpp < 3) && (cols >= chosen.cols)))))
                     {
                        have_choice = 1;
                        if (pdelta < 0) pdelta = 0x7fffffff;
                        chosen.pdelta = pdelta;
                        chosen.w = w;
                        chosen.h = h;
                        chosen.cols = cols;
                        chosen.bpp = bpp;
                        chosen.planes = planes;
                        chosen.bmsize = bmsize;
                        chosen.bmoffset = bmoffset;
                    }
               }
             else if (search == BIGGER)
               {
                  pdelta = (w * h) - (wanted_w * wanted_h);
                  if ((!have_choice) ||
                      ((w >= wanted_w) && (h >= wanted_h) &&
                          (pdelta <= chosen.pdelta) &&
                          (((bpp >= 3) && (bpp >= chosen.bpp)) ||
                              ((bpp < 3) && (cols >= chosen.cols)))))
                     {
                        have_choice = 1;
                        if (pdelta < 0) pdelta = 0x7fffffff;
                        chosen.pdelta = pdelta;
                        chosen.w = w;
                        chosen.h = h;
                        chosen.cols = cols;
                        chosen.bpp = bpp;
                        chosen.planes = planes;
                        chosen.bmsize = bmsize;
                        chosen.bmoffset = bmoffset;
                    }
               }
          }
     }
   if (chosen.bmoffset == 0) goto close_file;
   if (fseek(f, chosen.bmoffset, SEEK_SET) != 0) goto close_file;

   w = chosen.w;
   h = chosen.h;
   if ((w > 256) || (h > 256)) goto close_file;
   if ((w < 1) || (h < 1) || (w > IMG_MAX_SIZE) || (h > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(w, h))
     {
        if (IMG_TOO_BIG(w, h))
           *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        else
           *error = EVAS_LOAD_ERROR_GENERIC;
        goto close_file;
     }

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
evas_image_load_file_data_ico(Image_Entry *ie, const char *file, const char *key, int *error)
{
   unsigned short word;
   unsigned char byte;
   unsigned int dword;
   FILE *f;
   int wanted_w = 0, wanted_h = 0, w, h, cols, i, planes = 0,
      hot_x = 0, hot_y = 0, bpp = 0, pdelta, search = -1, have_choice = 0,
      stride, pstride, j, right_way_up = 0, diff_size = 0, cols2;
   unsigned int bmoffset, bmsize, bitcount, compression, imagesize, fsize, 
      colorsused, colorsimportant, *pal, *surface, *pix, none_zero_alpha = 0;
   unsigned short reserved, type, count, planes2;
   unsigned char *maskbuf, *pixbuf, *p;
   struct {
      int pdelta;
      int w, h;
      int cols;
      int bpp, planes;
      int hot_x, hot_y;
      unsigned int bmoffset, bmsize;
   } chosen = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

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
   if (fsize < (6 + 16 + 40)) goto close_file;

   // key:
   //   NULL == highest res
   //   biggest == highest res
   //   smallest == lowest res
   //   
   //   smaller == next size SMALLER than load opts WxH (if possible)
   //   bigger == next size BIGGER than load opts WxH (if possible)
   //   more ?

   search = BIGGEST;
   if ((ie->load_opts.w > 0) && (ie->load_opts.h > 0))
     {
        wanted_w = ie->load_opts.w;
        wanted_h = ie->load_opts.h;
        search = SMALLER;
     }
   
   if (!read_ushort(f, &reserved)) goto close_file;
   if (!read_ushort(f, &type)) goto close_file;
   if (!read_ushort(f, &count)) goto close_file;
   if (!((reserved == 0) && 
         ((type == ICON) || (type == CURSOR)) && (count > 0)))
      goto close_file;
   *error = EVAS_LOAD_ERROR_CORRUPT_FILE;

   if (key)
     {
        if (!strcmp(key, "biggest"))
          {
             wanted_w = 0;
             wanted_h = 0;
             search = BIGGEST;
             chosen.pdelta = 0;
          }
        else if (!strcmp(key, "smallest"))
          {
             wanted_w = 1;
             wanted_h = 1;
             search = SMALLEST;
             chosen.pdelta = 0x7fffffff;
          }
        else if (!strcmp(key, "smaller"))
          {
             chosen.pdelta = 0x7fffffff;
             search = SMALLER;
          }
        else if (!strcmp(key, "bigger"))
          {
             chosen.pdelta = 0x7fffffff;
             search = BIGGER;
          }
     }
   for (i = 0; i < count; i++)
     {
        if (fread(&byte, 1, 1, f) != 1) goto close_file;
        w = byte;
        if (w <= 0) w = 256;
        if (fread(&byte, 1, 1, f) != 1) goto close_file;
        h = byte;
        if (h <= 0) h = 256;
        if (fread(&byte, 1, 1, f) != 1) goto close_file;
        cols = byte;
        if (cols <= 0) cols = 256;
        if (fread(&byte, 1, 1, f) != 1) goto close_file;
        if (!read_ushort(f, &word)) goto close_file;
        if (type == 1) planes = word;
        else hot_x = word;
        if (!read_ushort(f, &word)) goto close_file;
        if (type == 1) bpp = word;
        else hot_y = word;
        if (!read_uint(f, &bmsize)) goto close_file;
        if (!read_uint(f, &bmoffset)) goto close_file;
        if ((bmsize <= 0) || (bmoffset <= 0) || (bmoffset >= fsize)) goto close_file;
        if (search == BIGGEST)
          {
             pdelta = w * h;
             if ((!have_choice) ||
                 ((pdelta >= chosen.pdelta) &&
                     (((bpp >= 3) && (bpp >= chosen.bpp)) ||
                         ((bpp < 3) && (cols >= chosen.cols)))))
               {
                  have_choice = 1;
                  chosen.pdelta = pdelta;
                  chosen.w = w;
                  chosen.h = h;
                  chosen.cols = cols;
                  chosen.bpp = bpp;
                  chosen.planes = planes;
                  chosen.bmsize = bmsize;
                  chosen.bmoffset = bmoffset;
               }
          }
        else
          {
             if (search == SMALLEST)
               {
                  pdelta = w * h;
                  if ((!have_choice) ||
                       ((pdelta <= chosen.pdelta) &&
                           (((bpp >= 3) && (bpp >= chosen.bpp)) ||
                               ((bpp < 3) && (cols >= chosen.cols)))))
                    {
                       have_choice = 1;
                       chosen.pdelta = pdelta;
                       chosen.w = w;
                       chosen.h = h;
                       chosen.cols = cols;
                       chosen.bpp = bpp;
                       chosen.planes = planes;
                       chosen.bmsize = bmsize;
                       chosen.bmoffset = bmoffset;
                    }
               }
             else if (search == SMALLER)
               {
                  pdelta = (wanted_w * wanted_h) - (w * h);
                  if ((!have_choice) ||
                      ((w <= wanted_w) && (h <= wanted_h) &&
                          (pdelta <= chosen.pdelta) &&
                          (((bpp >= 3) && (bpp >= chosen.bpp)) ||
                              ((bpp < 3) && (cols >= chosen.cols)))))
                     {
                        have_choice = 1;
                        if (pdelta < 0) pdelta = 0x7fffffff;
                        chosen.pdelta = pdelta;
                        chosen.w = w;
                        chosen.h = h;
                        chosen.cols = cols;
                        chosen.bpp = bpp;
                        chosen.planes = planes;
                        chosen.bmsize = bmsize;
                        chosen.bmoffset = bmoffset;
                    }
               }
             else if (search == BIGGER)
               {
                  pdelta = (w * h) - (wanted_w * wanted_h);
                  if ((!have_choice) ||
                      ((w >= wanted_w) && (h >= wanted_h) &&
                          (pdelta <= chosen.pdelta) &&
                          (((bpp >= 3) && (bpp >= chosen.bpp)) ||
                              ((bpp < 3) && (cols >= chosen.cols)))))
                     {
                        have_choice = 1;
                        if (pdelta < 0) pdelta = 0x7fffffff;
                        chosen.pdelta = pdelta;
                        chosen.w = w;
                        chosen.h = h;
                        chosen.cols = cols;
                        chosen.bpp = bpp;
                        chosen.planes = planes;
                        chosen.bmsize = bmsize;
                        chosen.bmoffset = bmoffset;
                    }
               }
          }
     }
   if (chosen.bmoffset == 0) goto close_file;
   if (fseek(f, chosen.bmoffset, SEEK_SET) != 0) goto close_file;

   w = chosen.w;
   h = chosen.h;
   cols = chosen.cols;
   bpp = chosen.bpp;
   // changed since we loaded header?
   if (((int)ie->w != w) || ((int)ie->h != h)) goto close_file;
   
   // read bmp header time... let's do some checking
   if (!read_uint(f, &dword)) goto close_file; // headersize - dont care
   if (!read_uint(f, &dword)) goto close_file; // width
   if (dword > 0)
     {
        if ((int)dword != w)
          {
             w = dword;
             diff_size = 1;
          }
     }
   if (!read_uint(f, &dword)) goto close_file; // height
   if (dword > 0)
     {
        if ((int)dword != (h * 2))
          {
             h = dword / 2;
             diff_size = 1;
          }
     }
   if (diff_size)
     {
        ERR("Broken ICO file: %s - "
            "  Reporting size of %ix%i in index, but bitmap is %ix%i. "
            "  May be expanded or cropped.",
            file, ie->w, ie->h, w, h);
     }
   if (!read_ushort(f, &word)) goto close_file; // planes
   planes2 = word;
   if (!read_ushort(f, &word)) goto close_file; // bitcount
   bitcount = word;
   if (!read_uint(f, &dword)) goto close_file; // compression
   compression = dword;
   if (!read_uint(f, &dword)) goto close_file; // imagesize
   imagesize = dword;
   if (!read_uint(f, &dword)) goto close_file; // z pixels per m
   if (!read_uint(f, &dword)) goto close_file; // y pizels per m
   if (!read_uint(f, &dword)) goto close_file; // colors used
   colorsused = dword;
   if (!read_uint(f, &dword)) goto close_file; // colors important
   colorsimportant = dword;

   evas_cache_image_surface_alloc(ie, ie->w, ie->h);
   surface = evas_cache_image_pixels(ie);
   if (!surface)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto close_file;
     }
   memset(surface, 0, ie->w * ie->h * 4);

   if (!((bitcount == 1) || (bitcount == 4) || (bitcount == 8) ||
         (bitcount == 24) || (bitcount == 32)))
      goto close_file;
   if (bitcount <= 8)
     {
        cols2 = 1 << bitcount;
        if (cols == 0) cols = cols2;
        if (cols > cols2) cols = cols2;
        if (cols > 256) cols = 256;
     }
   else
      cols = 0;
   if (bitcount > 8) cols = 0;

   pal = alloca(256 * 4);
   for (i = 0; i < cols; i++)
     {
        unsigned char a, r, g, b;
        
        if (fread(&b, 1, 1, f) != 1) goto close_file;
        if (fread(&g, 1, 1, f) != 1) goto close_file;
        if (fread(&r, 1, 1, f) != 1) goto close_file;
        if (fread(&a, 1, 1, f) != 1) goto close_file;
        a = 0xff;
        pal[i] = ARGB_JOIN(a, r, g, b);
     }
   stride = ((w + 31) / 32);
   maskbuf = alloca(stride * h);
   pixbuf = alloca(stride * 32 * 4); // more than enough
   if (bitcount == 1)
     {
        pstride = stride * 4;
        for (i = 0; i < h; i++)
          {
             pix = surface + (i * ie->w);
             if (!right_way_up) pix = surface + ((ie->h - 1 - i) * ie->w);
             if (fread(pixbuf, pstride, 1, f) != 1) goto close_file;
             p = pixbuf;
             if (i >= (int)ie->h) continue;
             for (j = 0; j < w; j++)
               {
                  if (j >= (int)ie->w) break;
                  if ((j & 0x7) == 0x0)
                    {
                       *pix = pal[*p >> 7];
                    }
                  else if ((j & 0x7) == 0x1)
                    {
                       *pix = pal[(*p >> 6) & 0x1];
                    }
                  else if ((j & 0x7) == 0x2)
                    {
                       *pix = pal[(*p >> 5) & 0x1];
                    }
                  else if ((j & 0x7) == 0x3)
                    {
                       *pix = pal[(*p >> 4) & 0x1];
                    }
                  else if ((j & 0x7) == 0x4)
                    {
                       *pix = pal[(*p >> 3) & 0x1];
                    }
                  else if ((j & 0x7) == 0x5)
                    {
                       *pix = pal[(*p >> 2) & 0x1];
                    }
                  else if ((j & 0x7) == 0x6)
                    {
                       *pix = pal[(*p >> 1) & 0x1];
                    }
                  else
                    {
                       *pix = pal[*p & 0x1];
                       p++;
                    }
                  pix++;
               }
          }
     }
   else if (bitcount == 4)
     {
        pstride = ((w + 7) / 8) * 4;
        for (i = 0; i < h; i++)
          {
             pix = surface + (i * ie->w);
             if (!right_way_up) pix = surface + ((ie->h - 1 - i) * ie->w);
             if (fread(pixbuf, pstride, 1, f) != 1) goto close_file;
             p = pixbuf;
             if (i >= (int)ie->h) continue;
             for (j = 0; j < w; j++)
               {
                  if (j >= (int)ie->w) break;
                  if ((j & 0x1) == 0x1)
                    {
                       *pix = pal[*p & 0x0f];
                       p++;
                    }
                  else
                    {
                       *pix = pal[*p >> 4];
                    }
                  pix++;
               }
          }
     }
   else if (bitcount == 8)
     {
        pstride = ((w + 3) / 4) * 4;
        for (i = 0; i < h; i++)
          {
             pix = surface + (i * ie->w);
             if (!right_way_up) pix = surface + ((ie->h - 1 - i) * ie->w);
             if (fread(pixbuf, pstride, 1, f) != 1) goto close_file;
             p = pixbuf;
             if (i >= (int)ie->h) continue;
             for (j = 0; j < w; j++)
               {
                  if (j >= (int)ie->w) break;
                  *pix = pal[*p];
                  p++;
                  pix++;
               }
          }
     }
   else if (bitcount == 24)
     {
        pstride = w * 3;
        for (i = 0; i < h; i++)
          {
             pix = surface + (i * ie->w);
             if (!right_way_up) pix = surface + ((ie->h - 1 - i) * ie->w);
             if (fread(pixbuf, pstride, 1, f) != 1) goto close_file;
             p = pixbuf;
             if (i >= (int)ie->h) continue;
             for (j = 0; j < w; j++)
               {
                  unsigned char a, r, g, b;
                  
                  if (j >= (int)ie->w) break;
                  b = p[0];
                  g = p[1];
                  r = p[2];
                  p += 3;
                  a = 0xff;
                  *pix = ARGB_JOIN(a, r, g, b);
                  pix++;
               }
          }
     }
   else if (bitcount == 32)
     {
        pstride = w * 4;
        for (i = 0; i < h; i++)
          {
             pix = surface + (i * ie->w);
             if (!right_way_up) pix = surface + ((ie->h - 1 - i) * ie->w);
             if (fread(pixbuf, pstride, 1, f) != 1) goto close_file;
             p = pixbuf;
             if (i >= (int)ie->h) continue;
             for (j = 0; j < w; j++)
               {
                  unsigned char a, r, g, b;
                  
                  if (j >= (int)ie->w) break;
                  b = p[0];
                  g = p[1];
                  r = p[2];
                  a = p[3];
                  p += 4;
                  if (a) none_zero_alpha = 1;
                  *pix = ARGB_JOIN(a, r, g, b);
                  pix++;
               }
          }
     }
   if (!none_zero_alpha)
     {
        if (fread(maskbuf, stride * 4 * h, 1, f) != 1) goto close_file;
        // apply mask
        pix = surface;
        for (i = 0; i < h; i++)
          {
             unsigned char *m;
             
             pix = surface + (i * ie->w);
             if (!right_way_up) pix = surface + ((ie->h - 1 - i) * ie->w);
             m = maskbuf + (stride * i * 4);
             if (i >= (int)ie->h) continue;
             for (j = 0; j < w; j++)
               {
                  if (j >= (int)ie->w) break;
                  if (*m & (1 << (7 - (j & 0x7))))
                     A_VAL(pix) = 0x00;
                  else
                     A_VAL(pix) = 0xff;
                  if ((j & 0x7) == 0x7) m++;
                  pix++;
               }
          }
     }
   
   fclose(f);
   
   evas_common_image_premul(ie);
   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;

 close_file:
   fclose(f);
   return EINA_FALSE;
}

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_ico_func);
   return 1;
}

static void
module_close(Evas_Module *em __UNUSED__)
{
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "ico",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, ico);

#ifndef EVAS_STATIC_BUILD_ICO
EVAS_EINA_MODULE_DEFINE(image_loader, ico);
#endif

#include "evas_common.h"
#include "evas_private.h"

#include <tiffio.h>

int evas_image_save_file_tiff(RGBA_Image *im, const char *file, const char *key, int quality, int compress);

Evas_Image_Save_Func evas_image_save_tiff_func =
{
   evas_image_save_file_tiff
};

static int
save_image_tiff(RGBA_Image *im, const char *file, int compress __UNUSED__, int interlace __UNUSED__)
{
   TIFF               *tif = NULL;
   uint8              *buf = NULL;
   DATA32              pixel;
   DATA32             *data;
   uint32              x, y;
   uint8               r, g, b, a = 0;
   int                 i = 0;
   int                 has_alpha;

   if (!im || !im->image.data || !file)
      return 0;

   has_alpha = im->cache_entry.flags.alpha;
   data = im->image.data;

   tif = TIFFOpen(file, "w");
   if (!tif)
      return 0;

   /* None of the TIFFSetFields are checked for errors, but since they */
   /* shouldn't fail, this shouldn't be a problem */

   TIFFSetField(tif, TIFFTAG_IMAGELENGTH, im->cache_entry.h);
   TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, im->cache_entry.w);
   TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
   TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
   TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
   TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_NONE);

   /* By default uses patent-free use COMPRESSION_DEFLATE,
    * another lossless compression technique */
   TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
   
   if (has_alpha)
     {
        uint16 extras[] = { EXTRASAMPLE_ASSOCALPHA };
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 4);
        TIFFSetField(tif, TIFFTAG_EXTRASAMPLES, 1, extras);
     }
   else
     {
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
     }

   TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
   TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, 0));

   buf = (uint8 *) _TIFFmalloc(TIFFScanlineSize(tif));
   if (!buf)
     {
        TIFFClose(tif);
        return 0;
     }

   for (y = 0; y < im->cache_entry.h; y++)
     {
        i = 0;
        for (x = 0; x < im->cache_entry.w; x++)
          {
             pixel = data[(y * im->cache_entry.w) + x];

             r = (pixel >> 16) & 0xff;
             g = (pixel >> 8) & 0xff;
             b = pixel & 0xff;
             if (has_alpha)
                a = (pixel >> 24) & 0xff;

             /* This might be endian dependent */
             buf[i++] = r;
             buf[i++] = g;
             buf[i++] = b;
             if (has_alpha)
                buf[i++] = a;
          }

        if (!TIFFWriteScanline(tif, buf, y, 0))
          {
             _TIFFfree(buf);
             TIFFClose(tif);
             return 0;
          }
     }

   _TIFFfree(buf);
   TIFFClose(tif);

   return 1;
}

int evas_image_save_file_tiff(RGBA_Image *im, const char *file, const char *key __UNUSED__, int quality __UNUSED__, int compress)
{
   return save_image_tiff(im, file, compress, 0);
}

EAPI int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_save_tiff_func);
   return 1;
}

EAPI void
module_close(void)
{
   
}

EAPI Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
     EVAS_MODULE_TYPE_IMAGE_SAVER,
     "tiff",
     "none"
};

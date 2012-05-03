#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <tiffio.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "evas_macros.h"

#include "evas_cserve2.h"
#include "evas_cserve2_slave.h"

typedef struct TIFFRGBAImage_Extra TIFFRGBAImage_Extra;

struct TIFFRGBAImage_Extra {
   TIFFRGBAImage       rgba;
   char                pper;
   uint32              num_pixels;
   uint32              py;
};

static Eina_Bool
evas_image_load_file_head_tiff(Evas_Img_Load_Params *ilp, const char *file, const char *key __UNUSED__, int *error)
{
   char                txt[1024];
   TIFFRGBAImage       tiff_image;
   TIFF               *tif = NULL;
   FILE               *ffile;
   int                 fd;
   uint16              magic_number;

   ffile = fopen(file, "rb");
   if (!ffile)
     {
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
	return EINA_FALSE;
     }

   if (fread(&magic_number, sizeof(uint16), 1, ffile) != 1)
     {
        fclose(ffile);
	*error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
	return EINA_FALSE;
     }
   /* Apparently rewind(f) isn't sufficient */
   fseek(ffile, 0, SEEK_SET);

   if ((magic_number != TIFF_BIGENDIAN) /* Checks if actually tiff file */
       && (magic_number != TIFF_LITTLEENDIAN))
     {
        fclose(ffile);
	*error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
	return EINA_FALSE;
     }

   fd = fileno(ffile);
   fd = dup(fd);
   lseek(fd, (long)0, SEEK_SET);
   fclose(ffile);

   tif = TIFFFdOpen(fd, file, "r");
   if (!tif)
     {
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	return EINA_FALSE;
     }

   strcpy(txt, "Evas Tiff loader: cannot be processed by libtiff");
   if (!TIFFRGBAImageOK(tif, txt))
     {
        TIFFClose(tif);
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	return EINA_FALSE;
     }
   strcpy(txt, "Evas Tiff loader: cannot begin reading tiff");
   if (!TIFFRGBAImageBegin(& tiff_image, tif, 1, txt))
     {
        TIFFClose(tif);
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	return EINA_FALSE;
     }

   if (tiff_image.alpha != EXTRASAMPLE_UNSPECIFIED)
     ilp->alpha = 1;
   if ((tiff_image.width < 1) || (tiff_image.height < 1) ||
       (tiff_image.width > IMG_MAX_SIZE) || (tiff_image.height > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(tiff_image.width, tiff_image.height))
     {
	TIFFClose(tif);
	if (IMG_TOO_BIG(tiff_image.width, tiff_image.height))
	  *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	else
	  *error = EVAS_LOAD_ERROR_GENERIC;
	return EINA_FALSE;
     }
   ilp->w = tiff_image.width;
   ilp->h = tiff_image.height;

   TIFFRGBAImageEnd(&tiff_image);
   TIFFClose(tif);
   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
}

static Eina_Bool
evas_image_load_file_data_tiff(Evas_Img_Load_Params *ilp, const char *file, const char *key __UNUSED__, int *error)
{
   char                txt[1024];
   TIFFRGBAImage_Extra rgba_image;
   TIFF               *tif = NULL;
   FILE               *ffile;
   uint32             *rast = NULL;
   uint32              num_pixels;
   int                 fd, x, y;
   uint16              magic_number;
   unsigned int       *surface;

   ffile = fopen(file, "rb");
   if (!ffile)
     {
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
	return EINA_FALSE;
     }

   if (fread(&magic_number, sizeof(uint16), 1, ffile) != 1)
     {
        fclose(ffile);
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	return EINA_FALSE;
     }
   /* Apparently rewind(f) isn't sufficient */
   fseek(ffile, (long)0, SEEK_SET);

   if ((magic_number != TIFF_BIGENDIAN) /* Checks if actually tiff file */
       && (magic_number != TIFF_LITTLEENDIAN))
     {
        fclose(ffile);
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	return EINA_FALSE;
     }

   fd = fileno(ffile);
   fd = dup(fd);
   lseek(fd, (long)0, SEEK_SET);
   fclose(ffile);

   tif = TIFFFdOpen(fd, file, "r");
   if (!tif)
     {
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	return EINA_FALSE;
     }

   strcpy(txt, "Evas Tiff loader: cannot be processed by libtiff");
   if (!TIFFRGBAImageOK(tif, txt))
     {
        TIFFClose(tif);
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	return EINA_FALSE;
     }
   strcpy(txt, "Evas Tiff loader: cannot begin reading tiff");
   if (!TIFFRGBAImageBegin((TIFFRGBAImage *) & rgba_image, tif, 0, txt))
     {
        TIFFClose(tif);
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	return EINA_FALSE;
     }

   if (rgba_image.rgba.alpha != EXTRASAMPLE_UNSPECIFIED)
     ilp->alpha = 1;
   if ((rgba_image.rgba.width != ilp->w) ||
       (rgba_image.rgba.height != ilp->h))
     {
        TIFFClose(tif);
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	return EINA_FALSE;
     }

   surface = ilp->buffer;
   if (!surface)
     {
        TIFFRGBAImageEnd((TIFFRGBAImage *) & rgba_image);
        TIFFClose(tif);
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	return EINA_FALSE;
     }

   rgba_image.num_pixels = num_pixels = ilp->w * ilp->h;

   rgba_image.pper = rgba_image.py = 0;
   rast = (uint32 *) _TIFFmalloc(sizeof(uint32) * num_pixels);

   if (!rast)
     {
       ERR("Evas Tiff loader: out of memory");

       TIFFRGBAImageEnd((TIFFRGBAImage *) & rgba_image);
       TIFFClose(tif);
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	return EINA_FALSE;
     }
   if (rgba_image.rgba.bitspersample == 8)
     {
        if (!TIFFRGBAImageGet((TIFFRGBAImage *) &rgba_image, rast,
                              rgba_image.rgba.width, rgba_image.rgba.height))
          {
             _TIFFfree(rast);
             TIFFRGBAImageEnd((TIFFRGBAImage *) & rgba_image);
             TIFFClose(tif);
	     *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	     return EINA_FALSE;
          }
     }
   /* process rast -> image rgba. really same as prior code anyway just simpler */
   for (y = 0; y < (int)ilp->h; y++)
     {
        DATA32 *pix, *pd;
        uint32 *ps, pixel;
        unsigned int a, r, g, b;

        pix = surface;
        pd = pix + ((ilp->h - y - 1) * ilp->w);
        ps = rast + (y * ilp->w);
        for (x = 0; x < (int)ilp->w; x++)
          {
             pixel = *ps;
             a = TIFFGetA(pixel);
             r = TIFFGetR(pixel);
             g = TIFFGetG(pixel);
             b = TIFFGetB(pixel);
             if (!ilp->alpha) a = 255;
             if ((rgba_image.rgba.alpha == EXTRASAMPLE_UNASSALPHA) &&
                 (a < 255))
               {
                  r = (r * (a + 1)) >> 8;
                  g = (g * (a + 1)) >> 8;
                  b = (b * (a + 1)) >> 8;
               }
             *pd = ARGB_JOIN(a, r, g, b);
             ps++;
             pd++;
          }
     }

   _TIFFfree(rast);

   TIFFRGBAImageEnd((TIFFRGBAImage *) & rgba_image);

   TIFFClose(tif);

   evas_cserve2_image_alpha_sparse_set(ilp);
   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
}

static Evas_Loader_Module_Api modapi =
{
   EVAS_CSERVE2_MODULE_API_VERSION,
   "tiff",
   evas_image_load_file_head_tiff,
   evas_image_load_file_data_tiff
};

static Eina_Bool
module_init(void)
{
   return evas_cserve2_loader_register(&modapi);
}

static void
module_shutdown(void)
{
}

EINA_MODULE_INIT(module_init);
EINA_MODULE_SHUTDOWN(module_shutdown);

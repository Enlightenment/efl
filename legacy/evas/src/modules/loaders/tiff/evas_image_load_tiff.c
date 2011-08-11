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

#include "evas_common.h"
#include "evas_private.h"

static int _evas_loader_tiff_log_dom = -1;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_loader_tiff_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_loader_tiff_log_dom, __VA_ARGS__)

static Eina_Bool evas_image_load_file_head_tiff(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);
static Eina_Bool evas_image_load_file_data_tiff(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);

static Evas_Image_Load_Func evas_image_load_tiff_func =
{
  EINA_TRUE,
  evas_image_load_file_head_tiff,
  evas_image_load_file_data_tiff,
  NULL
};

typedef struct TIFFRGBAImage_Extra TIFFRGBAImage_Extra;

struct TIFFRGBAImage_Extra {
   TIFFRGBAImage       rgba;
   Image_Entry        *image;
   char                pper;
   uint32              num_pixels;
   uint32              py;
};

static Eina_Bool
evas_image_load_file_head_tiff(Image_Entry *ie, const char *file, const char *key __UNUSED__, int *error)
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
     ie->flags.alpha = 1;
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
   ie->w = tiff_image.width;
   ie->h = tiff_image.height;

   TIFFRGBAImageEnd(&tiff_image);
   TIFFClose(tif);
   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
}

static Eina_Bool
evas_image_load_file_data_tiff(Image_Entry *ie, const char *file, const char *key __UNUSED__, int *error)
{
   char                txt[1024];
   TIFFRGBAImage_Extra rgba_image;
   TIFF               *tif = NULL;
   FILE               *ffile;
   uint32             *rast = NULL;
   uint32              num_pixels;
   int                 fd, x, y;
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
   rgba_image.image = ie;

   if (rgba_image.rgba.alpha != EXTRASAMPLE_UNSPECIFIED)
     ie->flags.alpha = 1;
   if ((rgba_image.rgba.width != ie->w) ||
       (rgba_image.rgba.height != ie->h))
     {
        TIFFClose(tif);
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	return EINA_FALSE;
     }

   evas_cache_image_surface_alloc(ie, rgba_image.rgba.width, rgba_image.rgba.height);
   if (!evas_cache_image_pixels(ie))
     {
        TIFFRGBAImageEnd((TIFFRGBAImage *) & rgba_image);
        TIFFClose(tif);
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	return EINA_FALSE;
     }

   rgba_image.num_pixels = num_pixels = ie->w * ie->h;

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
   else
     {
        INF("channel bits == %i", (int)rgba_image.rgba.samplesperpixel);
     }
   /* process rast -> image rgba. really same as prior code anyway just simpler */
   for (y = 0; y < (int)ie->h; y++)
     {
        DATA32 *pix, *pd;
        uint32 *ps, pixel;
        unsigned int a, r, g, b;
        
        pix = evas_cache_image_pixels(ie);
        pd = pix + ((ie->h - y - 1) * ie->w);
        ps = rast + (y * ie->w);
        for (x = 0; x < (int)ie->w; x++)
          {
             pixel = *ps;
             a = TIFFGetA(pixel);
             r = TIFFGetR(pixel);
             g = TIFFGetG(pixel);
             b = TIFFGetB(pixel);
             if (!ie->flags.alpha) a = 255;
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

   evas_common_image_set_alpha_sparse(ie);
   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
}

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   _evas_loader_tiff_log_dom = eina_log_domain_register
     ("evas-tiff", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_loader_tiff_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }
   em->functions = (void *)(&evas_image_load_tiff_func);
   return 1;
}

static void
module_close(Evas_Module *em __UNUSED__)
{
   eina_log_domain_unregister(_evas_loader_tiff_log_dom);
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "tiff",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, tiff);

#ifndef EVAS_STATIC_BUILD_TIFF
EVAS_EINA_MODULE_DEFINE(image_loader, tiff);
#endif

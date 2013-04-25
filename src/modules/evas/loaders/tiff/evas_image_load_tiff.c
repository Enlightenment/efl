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

static Eina_Bool evas_image_load_file_head_tiff(Eina_File *f, const char *key, Evas_Image_Property *prop, Evas_Image_Load_Opts *opts, Evas_Image_Animated *animated, int *error);
static Eina_Bool evas_image_load_file_data_tiff(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);

static Evas_Image_Load_Func evas_image_load_tiff_func =
{
  EINA_TRUE,
  evas_image_load_file_head_tiff,
  evas_image_load_file_data_tiff,
  NULL,
  EINA_FALSE
};

typedef struct TIFFRGBAImage_Extra TIFFRGBAImage_Extra;

struct TIFFRGBAImage_Extra {
   TIFFRGBAImage       rgba;
   Image_Entry        *image;
   char                pper;
   uint32              num_pixels;
   uint32              py;
};

static tsize_t
_evas_tiff_RWProc(thandle_t handle EINA_UNUSED,
                  tdata_t data EINA_UNUSED,
                  tsize_t size EINA_UNUSED)
{
   return 0;
}

static toff_t
_evas_tiff_SeekProc(thandle_t handle EINA_UNUSED,
                    toff_t size EINA_UNUSED,
                    int origin EINA_UNUSED)
{
   return 0;
}

static int
_evas_tiff_CloseProc(thandle_t handle EINA_UNUSED)
{
   return 0;
}

static toff_t
_evas_tiff_SizeProc(thandle_t handle)
{
   Eina_File *f = (Eina_File *) handle;

   return eina_file_size_get(f);
}

static int
_evas_tiff_MapProc(thandle_t handle, tdata_t *mem, toff_t *size)
{
   Eina_File *f = (Eina_File *) handle;

   *mem = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   *size = eina_file_size_get(f);

   return 1;
}

static void
_evas_tiff_UnmapProc(thandle_t handle, tdata_t data, toff_t size EINA_UNUSED)
{
   Eina_File *f = (Eina_File *) handle;

   eina_file_map_free(f, data);
}

static Eina_Bool
evas_image_load_file_head_tiff(Eina_File *f, const char *key EINA_UNUSED,
			       Evas_Image_Property *prop,
			       Evas_Image_Load_Opts *opts EINA_UNUSED,
			       Evas_Image_Animated *animated EINA_UNUSED,
			       int *error)
{
   char           txt[1024];
   TIFFRGBAImage  tiff_image;
   TIFF          *tif = NULL;
   unsigned char *map;
   uint16         magic_number;
   Eina_Bool      r = EINA_FALSE;

   map = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   if (!map || eina_file_size_get(f) < 3)
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        goto on_error;
     }

   magic_number = *((uint16*) map);

   if ((magic_number != TIFF_BIGENDIAN) /* Checks if actually tiff file */
       && (magic_number != TIFF_LITTLEENDIAN))
     {
	*error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        goto on_error;
     }

   tif = TIFFClientOpen("evas", "rM", f,
                        _evas_tiff_RWProc, _evas_tiff_RWProc,
                        _evas_tiff_SeekProc, _evas_tiff_CloseProc,
                        _evas_tiff_SizeProc,
                        _evas_tiff_MapProc, _evas_tiff_UnmapProc);
   if (!tif)
     {
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        goto on_error;
     }

   strcpy(txt, "Evas Tiff loader: cannot be processed by libtiff");
   if (!TIFFRGBAImageOK(tif, txt))
     {
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        goto on_error;
     }
   strcpy(txt, "Evas Tiff loader: cannot begin reading tiff");
   if (!TIFFRGBAImageBegin(& tiff_image, tif, 1, txt))
     {
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        goto on_error;
     }

   if (tiff_image.alpha != EXTRASAMPLE_UNSPECIFIED)
     prop->alpha = 1;
   if ((tiff_image.width < 1) || (tiff_image.height < 1) ||
       (tiff_image.width > IMG_MAX_SIZE) || (tiff_image.height > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(tiff_image.width, tiff_image.height))
     {
	if (IMG_TOO_BIG(tiff_image.width, tiff_image.height))
	  *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	else
	  *error = EVAS_LOAD_ERROR_GENERIC;
        goto on_error_end;
     }
   prop->w = tiff_image.width;
   prop->h = tiff_image.height;

   *error = EVAS_LOAD_ERROR_NONE;
   r = EINA_TRUE;

 on_error_end:
   TIFFRGBAImageEnd(&tiff_image);
 on_error:
   if (tif) TIFFClose(tif);
   if (map) eina_file_map_free(f, map);
   return r;
}

static Eina_Bool
evas_image_load_file_data_tiff(Image_Entry *ie, const char *file, const char *key EINA_UNUSED, int *error)
{
   char                txt[1024];
   TIFFRGBAImage_Extra rgba_image;
   TIFF               *tif = NULL;
   Eina_File          *f;
   unsigned char      *map;
   uint32             *rast = NULL;
   uint32              num_pixels;
   int                 x, y;
   uint16              magic_number;
   Eina_Bool           res = EINA_FALSE;

   f = eina_file_open(file, EINA_FALSE);
   if (!f)
     {
        *error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
	return EINA_FALSE;
     }

   map = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   if (!map || eina_file_size_get(f) < 3)
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        goto on_error;
     }

   magic_number = *((uint16*) map);

   if ((magic_number != TIFF_BIGENDIAN) /* Checks if actually tiff file */
       && (magic_number != TIFF_LITTLEENDIAN))
     {
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        goto on_error;
     }

   tif = TIFFClientOpen("evas", "rM", f,
                        _evas_tiff_RWProc, _evas_tiff_RWProc,
                        _evas_tiff_SeekProc, _evas_tiff_CloseProc,
                        _evas_tiff_SizeProc,
                        _evas_tiff_MapProc, _evas_tiff_UnmapProc);
   if (!tif)
     {
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        goto on_error;
     }

   strcpy(txt, "Evas Tiff loader: cannot be processed by libtiff");
   if (!TIFFRGBAImageOK(tif, txt))
     {
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        goto on_error;
     }
   strcpy(txt, "Evas Tiff loader: cannot begin reading tiff");
   if (!TIFFRGBAImageBegin((TIFFRGBAImage *) & rgba_image, tif, 0, txt))
     {
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        goto on_error;
     }
   rgba_image.image = ie;

   if (rgba_image.rgba.alpha != EXTRASAMPLE_UNSPECIFIED)
     ie->flags.alpha = 1;
   if ((rgba_image.rgba.width != ie->w) ||
       (rgba_image.rgba.height != ie->h))
     {
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto on_error_end;
     }

   evas_cache_image_surface_alloc(ie, rgba_image.rgba.width, rgba_image.rgba.height);
   if (!evas_cache_image_pixels(ie))
     {
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto on_error_end;
     }

   rgba_image.num_pixels = num_pixels = ie->w * ie->h;

   rgba_image.pper = rgba_image.py = 0;
   rast = (uint32 *) _TIFFmalloc(sizeof(uint32) * num_pixels);

   if (!rast)
     {
        ERR("Evas Tiff loader: out of memory");

	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	goto on_error_end;
     }
   if (rgba_image.rgba.bitspersample == 8)
     {
        if (!TIFFRGBAImageGet((TIFFRGBAImage *) &rgba_image, rast,
                              rgba_image.rgba.width, rgba_image.rgba.height))
          {
             _TIFFfree(rast);
	     *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
             goto on_error_end;
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

   evas_common_image_set_alpha_sparse(ie);
   *error = EVAS_LOAD_ERROR_NONE;
   res = EINA_TRUE;

 on_error_end:
   TIFFRGBAImageEnd((TIFFRGBAImage *) & rgba_image);
 on_error:
   if (tif) TIFFClose(tif);
   if (map) eina_file_map_free(f, map);
   eina_file_close(f);
   return res;
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
module_close(Evas_Module *em EINA_UNUSED)
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

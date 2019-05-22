#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <tiffio.h>

#include "evas_common_private.h"
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

typedef struct TIFFRGBAImage_Extra TIFFRGBAImage_Extra;
typedef struct TIFFRGBAMap TIFFRGBAMap;

struct TIFFRGBAImage_Extra {
   TIFFRGBAImage       rgba;
   char                pper;
   uint32              num_pixels;
   uint32              py;
};

struct TIFFRGBAMap {
   tdata_t mem;
   toff_t size;
};

static tsize_t
_evas_tiff_RWProc(thandle_t handle,
                  tdata_t data,
                  tsize_t size)
{
   TIFFRGBAMap *map = (TIFFRGBAMap*) handle;
   if (!data) return 0;
   memcpy(data, map->mem, size);

   return size;
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
   TIFFRGBAMap *map = (TIFFRGBAMap*) handle;

   return map->size;
}

static int
_evas_tiff_MapProc(thandle_t handle, tdata_t *mem, toff_t *size)
{
   TIFFRGBAMap *map = (TIFFRGBAMap*) handle;

   *mem = map->mem;
   *size = map->size;

   return 1;
}

static void
_evas_tiff_UnmapProc(thandle_t handle EINA_UNUSED, tdata_t data EINA_UNUSED, toff_t size EINA_UNUSED)
{
}

static void *
evas_image_load_file_open_tiff(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
			       Evas_Image_Load_Opts *opts EINA_UNUSED,
			       Evas_Image_Animated *animated EINA_UNUSED,
			       int *error EINA_UNUSED)
{
   return f;
}

static void
evas_image_load_file_close_tiff(void *loader_data EINA_UNUSED)
{
}

static Eina_Bool
evas_image_load_file_head_tiff(void *loader_data,
			       Evas_Image_Property *prop,
			       int *error)
{
   Eina_File *f = loader_data;
   char           txt[1024];
   TIFFRGBAImage  tiff_image;
   TIFFRGBAMap    tiff_map;
   TIFF          *tif = NULL;
   unsigned char *map;
   uint16         magic_number;
   Eina_Bool      r = EINA_FALSE;

   map = eina_file_map_all(f, EINA_FILE_RANDOM);
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

   tiff_map.mem = map;
   tiff_map.size = eina_file_size_get(f);

   tif = TIFFClientOpen("evas", "rM", &tiff_map,
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
evas_image_load_file_data_tiff(void *loader_data,
			       Evas_Image_Property *prop,
                               void *pixels,
			       int *error)
{
   Eina_File          *f = loader_data;
   char                txt[1024];
   TIFFRGBAImage_Extra rgba_image;
   TIFFRGBAMap         rgba_map;
   TIFF               *tif = NULL;
   unsigned char      *map;
   uint32             *rast = NULL;
   uint32              num_pixels;
   int                 x, y;
   uint16              magic_number;
   Eina_Bool           res = EINA_FALSE;

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

   rgba_map.mem = map;
   rgba_map.size = eina_file_size_get(f);

   tif = TIFFClientOpen("evas", "rM", &rgba_map,
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

   if (rgba_image.rgba.alpha != EXTRASAMPLE_UNSPECIFIED)
     prop->alpha = 1;
   if ((rgba_image.rgba.width != prop->w) ||
       (rgba_image.rgba.height != prop->h))
     {
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto on_error_end;
     }

   rgba_image.num_pixels = num_pixels = prop->w * prop->h;

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
   for (y = 0; y < (int)prop->h; y++)
     {
        DATA32 *pix, *pd;
        uint32 *ps, pixel;
        unsigned int a, r, g, b;
	unsigned int nas = 0;
        
        pix = pixels;
        pd = pix + ((prop->h - y - 1) * prop->w);
        ps = rast + (y * prop->w);
        for (x = 0; x < (int)prop->w; x++)
          {
             pixel = *ps;
             a = TIFFGetA(pixel);
             r = TIFFGetR(pixel);
             g = TIFFGetG(pixel);
             b = TIFFGetB(pixel);
             if (!prop->alpha) a = 255;
             if ((rgba_image.rgba.alpha == EXTRASAMPLE_UNASSALPHA) &&
                 (a < 255))
               {
                  r = (r * (a + 1)) >> 8;
                  g = (g * (a + 1)) >> 8;
                  b = (b * (a + 1)) >> 8;
               }
             *pd = ARGB_JOIN(a, r, g, b);

	     if (a == 0xff) nas++;
             ps++;
             pd++;
          }

	if ((ALPHA_SPARSE_INV_FRACTION * nas) >= (prop->w * prop->h))
	  prop->alpha_sparse = EINA_TRUE;
     }

   _TIFFfree(rast);

   *error = EVAS_LOAD_ERROR_NONE;
   res = EINA_TRUE;

 on_error_end:
   TIFFRGBAImageEnd((TIFFRGBAImage *) & rgba_image);
 on_error:
   if (tif) TIFFClose(tif);
   if (map) eina_file_map_free(f, map);
   return res;
}

static Evas_Image_Load_Func evas_image_load_tiff_func =
{
  evas_image_load_file_open_tiff,
  evas_image_load_file_close_tiff,
  evas_image_load_file_head_tiff,
  evas_image_load_file_data_tiff,
  NULL,
  EINA_TRUE,
  EINA_FALSE
};

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
   if (_evas_loader_tiff_log_dom >= 0)
     {
        eina_log_domain_unregister(_evas_loader_tiff_log_dom);
        _evas_loader_tiff_log_dom = -1;
     }
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

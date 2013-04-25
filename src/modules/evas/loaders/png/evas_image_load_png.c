#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <png.h>
#include <setjmp.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "evas_common.h"
#include "evas_private.h"

#define PNG_BYTES_TO_CHECK 4

static Eina_Bool evas_image_load_file_head_png(Eina_File *f, const char *key, Evas_Image_Property *prop, Evas_Image_Load_Opts *opts, Evas_Image_Animated *animated, int *error);
static Eina_Bool evas_image_load_file_data_png(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);

static Evas_Image_Load_Func evas_image_load_png_func =
{
  EINA_TRUE,
  evas_image_load_file_head_png,
  evas_image_load_file_data_png,
  NULL,
  EINA_FALSE
};

typedef struct _Evas_PNG_Info Evas_PNG_Info;
struct _Evas_PNG_Info
{
   unsigned char *map;
   size_t length;
   size_t position;
};

static void
_evas_image_png_read(png_structp png_ptr, png_bytep out, png_size_t count)
{
   Evas_PNG_Info *epi = png_get_io_ptr(png_ptr);

   if (!epi) return ;
   if (epi->position == epi->length) return ;

   if (epi->position + count > epi->length) count = epi->length - epi->position;
   memcpy(out, epi->map + epi->position, count);
   epi->position += count;
}

static Eina_Bool
evas_image_load_file_head_png(Eina_File *f, const char *key EINA_UNUSED,
			      Evas_Image_Property *prop,
			      Evas_Image_Load_Opts *opts,
			      Evas_Image_Animated *animated EINA_UNUSED,
			      int *error)
{
   Evas_PNG_Info epi;
   png_structp png_ptr = NULL;
   png_infop info_ptr = NULL;
   png_uint_32 w32, h32;
   int bit_depth, color_type, interlace_type;
   char hasa;
   Eina_Bool r = EINA_FALSE;

   hasa = 0;
   epi.map = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   if (!epi.map)
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        goto close_file;
     }
   epi.length = eina_file_size_get(f);
   epi.position = 0;

   if (epi.length < PNG_BYTES_TO_CHECK)
     {
	*error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
	goto close_file;
     }

   if (png_sig_cmp(epi.map, 0, PNG_BYTES_TO_CHECK))
     {
	*error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
	goto close_file;
     }

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png_ptr)
     {
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	goto close_file;
     }

   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
     {
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	goto close_file;
     }

   png_set_read_fn(png_ptr, &epi, _evas_image_png_read);

   if (setjmp(png_jmpbuf(png_ptr)))
     {
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	goto close_file;
     }

   png_read_info(png_ptr, info_ptr);
   png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *) (&w32),
		(png_uint_32 *) (&h32), &bit_depth, &color_type,
		&interlace_type, NULL, NULL);
   if ((w32 < 1) || (h32 < 1) || (w32 > IMG_MAX_SIZE) || (h32 > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(w32, h32))
     {
	if (IMG_TOO_BIG(w32, h32))
	  *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	else
	  *error = EVAS_LOAD_ERROR_GENERIC;
	goto close_file;
     }
   if (opts->scale_down_by > 1)
     {
        prop->w = (int) w32 / opts->scale_down_by;
        prop->h = (int) h32 / opts->scale_down_by;
        if ((prop->w < 1) || (prop->h < 1))
          {
             *error = EVAS_LOAD_ERROR_GENERIC;
             goto close_file;
          }
     }
   else
     {
        prop->w = (int) w32;
        prop->h = (int) h32;
     }
   if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) hasa = 1;
   if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) hasa = 1;
   if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) hasa = 1;
   if (hasa) prop->alpha = 1;

   *error = EVAS_LOAD_ERROR_NONE;
   r = EINA_TRUE;

 close_file:
   if (png_ptr) png_destroy_read_struct(&png_ptr,
                                        info_ptr ? &info_ptr : NULL,
                                        NULL);
   if (epi.map) eina_file_map_free(f, epi. map);

   return r;
}

static Eina_Bool
evas_image_load_file_data_png(Image_Entry *ie, const char *file, const char *key EINA_UNUSED, int *error)
{
   Eina_File *f;
   unsigned char *surface;
   unsigned char **lines;
   unsigned char *tmp_line;
   DATA32 *src_ptr, *dst_ptr;
   png_structp png_ptr = NULL;
   png_infop info_ptr = NULL;
   Evas_PNG_Info epi;
   png_uint_32 w32, h32;
   int w, h;
   int bit_depth, color_type, interlace_type;
   char hasa;
   int i, j;
   int scale_ratio = 1, image_w = 0;
   Eina_Bool r = EINA_FALSE;

   hasa = 0;
   f = eina_file_open(file, EINA_FALSE);
   if (!f)
     {
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
	return EINA_FALSE;
     }

   epi.map = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   if (!epi.map)
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        goto close_file;
     }
   epi.length = eina_file_size_get(f);
   epi.position = 0;

   if (epi.length < PNG_BYTES_TO_CHECK)
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        goto close_file;
     }

   /* if we havent read the header before, set the header data */
   if (png_sig_cmp(epi.map, 0, PNG_BYTES_TO_CHECK))
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
	goto close_file;
     }
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png_ptr)
     {
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	goto close_file;
     }

   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
     {
	png_destroy_read_struct(&png_ptr, NULL, NULL);
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	goto close_file;
     }

   png_set_read_fn(png_ptr, &epi, _evas_image_png_read);

   if (setjmp(png_jmpbuf(png_ptr)))
     {
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	goto close_file;
     }

   png_read_info(png_ptr, info_ptr);
   png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *) (&w32),
		(png_uint_32 *) (&h32), &bit_depth, &color_type,
		&interlace_type, NULL, NULL);
   image_w = w32;
   if (ie->load_opts.scale_down_by > 1)
     {
        scale_ratio = ie->load_opts.scale_down_by;
        w32 /= scale_ratio;
        h32 /= scale_ratio;
     }
   evas_cache_image_surface_alloc(ie, w32, h32);
   surface = (unsigned char *) evas_cache_image_pixels(ie);
   if (!surface)
     {
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	goto close_file;
     }
   if ((w32 != ie->w) || (h32 != ie->h))
     {
	*error = EVAS_LOAD_ERROR_GENERIC;
	goto close_file;
     }
   if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) hasa = 1;
   if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) hasa = 1;
   if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) hasa = 1;
   if (hasa) ie->flags.alpha = 1;

   /* Prep for transformations...  ultimately we want ARGB */
   /* expand palette -> RGB if necessary */
   if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
   /* expand gray (w/reduced bits) -> 8-bit RGB if necessary */
   if ((color_type == PNG_COLOR_TYPE_GRAY) ||
       (color_type == PNG_COLOR_TYPE_GRAY_ALPHA))
     {
	png_set_gray_to_rgb(png_ptr);
	if (bit_depth < 8) png_set_expand_gray_1_2_4_to_8(png_ptr);
     }
   /* expand transparency entry -> alpha channel if present */
   if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
     png_set_tRNS_to_alpha(png_ptr);
   /* reduce 16bit color -> 8bit color if necessary */
   if (bit_depth > 8) png_set_strip_16(png_ptr);
   /* pack all pixels to byte boundaries */
   png_set_packing(png_ptr);

   w = ie->w;
   h = ie->h;
   /* we want ARGB */
#ifdef WORDS_BIGENDIAN
   png_set_swap_alpha(png_ptr);
   if (!hasa) png_set_filler(png_ptr, 0xff, PNG_FILLER_BEFORE);
#else
   png_set_bgr(png_ptr);
   if (!hasa) png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
#endif

   /* we read image line by line if scale down was set */
   if (scale_ratio == 1)
     {
        lines = (unsigned char **) alloca(h * sizeof(unsigned char *));
        for (i = 0; i < h; i++)
          lines[i] = surface + (i * w * sizeof(DATA32));
        png_read_image(png_ptr, lines);
        png_read_end(png_ptr, info_ptr);
     }
   else
     {
        tmp_line = (unsigned char *) alloca(image_w * sizeof(DATA32));
        dst_ptr = (DATA32 *)surface;
        for (i = 0; i < h; i++)
          {
             png_read_row(png_ptr, tmp_line, NULL);
             src_ptr = (DATA32 *)tmp_line;
             for (j = 0; j < w; j++)
               {
                  *dst_ptr = *src_ptr;
                  dst_ptr++;
                  src_ptr += scale_ratio;
               }
             for (j = 0; j < (scale_ratio - 1); j++)
               {
                  png_read_row(png_ptr, tmp_line, NULL);
               }
          }
     }

   evas_common_image_premul(ie);

   *error = EVAS_LOAD_ERROR_NONE;
   r = EINA_TRUE;

 close_file:
   if (png_ptr) png_destroy_read_struct(&png_ptr,
                                        info_ptr ? &info_ptr : NULL,
                                        NULL);
   if (epi.map) eina_file_map_free(f, epi.map);
   eina_file_close(f);
   return r;
}

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_png_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "png",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, png);

#ifndef EVAS_STATIC_BUILD_PNG
EVAS_EINA_MODULE_DEFINE(image_loader, png);
#endif

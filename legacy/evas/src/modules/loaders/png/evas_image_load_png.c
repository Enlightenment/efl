#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <png.h>
#include <setjmp.h>

#include "evas_common.h"
#include "evas_private.h"


#define PNG_BYTES_TO_CHECK 4


int evas_image_load_file_head_png(Image_Entry *ie, const char *file, const char *key);
int evas_image_load_file_data_png(Image_Entry *ie, const char *file, const char *key);

Evas_Image_Load_Func evas_image_load_png_func =
{
  evas_image_load_file_head_png,
  evas_image_load_file_data_png
};


int
evas_image_load_file_head_png(Image_Entry *ie, const char *file, const char *key)
{
   png_uint_32 w32, h32;
   FILE *f;
   png_structp png_ptr = NULL;
   png_infop info_ptr = NULL;
   int bit_depth, color_type, interlace_type;
   unsigned char buf[PNG_BYTES_TO_CHECK];
   char hasa;

   if ((!file)) return 0;
   hasa = 0;
   f = fopen(file, "rb");
   if (!f) return 0;

   /* if we havent read the header before, set the header data */
   if (fread(buf, PNG_BYTES_TO_CHECK, 1, f) != 1)
     {
	fclose(f);
	return 0;
     }
   if (!png_check_sig(buf, PNG_BYTES_TO_CHECK))
     {
	fclose(f);
	return 0;
     }
   rewind(f);
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png_ptr)
     {
	fclose(f);
	return 0;
     }
   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
     {
	png_destroy_read_struct(&png_ptr, NULL, NULL);
	fclose(f);
	return 0;
     }
   if (setjmp(png_ptr->jmpbuf))
     {
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(f);
	return 0;
     }
   png_init_io(png_ptr, f);
   png_read_info(png_ptr, info_ptr);
   png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *) (&w32),
		(png_uint_32 *) (&h32), &bit_depth, &color_type,
		&interlace_type, NULL, NULL);
   if ((w32 < 1) || (h32 < 1) || (w32 > 8192) || (h32 > 8192))
     {
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
	fclose(f);
	return 0;
     }
   ie->w = (int) w32;
   ie->h = (int) h32;
   if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) hasa = 1;
   if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) hasa = 1;
   if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) hasa = 1;
   if (hasa) ie->flags.alpha = 1;
   png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
   fclose(f);
   return 1;
   key = 0;
}

int
evas_image_load_file_data_png(Image_Entry *ie, const char *file, const char *key)
{
   unsigned char *surface;
   png_uint_32 w32, h32;
   int w, h;
   FILE *f;
   png_structp png_ptr = NULL;
   png_infop info_ptr = NULL;
   int bit_depth, color_type, interlace_type;
   unsigned char buf[PNG_BYTES_TO_CHECK];
   unsigned char **lines;
   char hasa;
   int i;

   if ((!file)) return 0;
   hasa = 0;
   f = fopen(file, "rb");
   if (!f) return 0;

   /* if we havent read the header before, set the header data */
   fread(buf, 1, PNG_BYTES_TO_CHECK, f);
   if (!png_check_sig(buf, PNG_BYTES_TO_CHECK))
     {
	fclose(f);
	return 0;
     }
   rewind(f);
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png_ptr)
     {
	fclose(f);
	return 0;
     }
   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
     {
	png_destroy_read_struct(&png_ptr, NULL, NULL);
	fclose(f);
	return 0;
     }
   if (setjmp(png_ptr->jmpbuf))
     {
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(f);
	return 0;
     }
   png_init_io(png_ptr, f);
   png_read_info(png_ptr, info_ptr);
   png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *) (&w32),
		(png_uint_32 *) (&h32), &bit_depth, &color_type,
		&interlace_type, NULL, NULL);
   evas_cache_image_surface_alloc(ie, w32, h32);
   surface = (unsigned char *) evas_cache_image_pixels(ie);
   if (!surface)
     {
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
	fclose(f);
	return 0;
     }
   if ((w32 != ie->w) || (h32 != ie->h))
     {
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
	fclose(f);
	return 0;
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
   lines = (unsigned char **) alloca(h * sizeof(unsigned char *));

   for (i = 0; i < h; i++)
     lines[i] = surface + (i * w * sizeof(DATA32));
   png_read_image(png_ptr, lines);
   png_read_end(png_ptr, info_ptr);
   png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
   fclose(f);
   evas_common_image_premul(ie);

   return 1;
   key = 0;
}

EAPI int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_png_func);
   return 1;
}

EAPI void
module_close(void)
{
   
}

EAPI Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
     EVAS_MODULE_TYPE_IMAGE_LOADER,
     "png",
     "none"
};

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <png.h>
#include <setjmp.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#ifdef _WIN32_WCE
# define E_FOPEN(file, mode) evil_fopen_native((file), (mode))
# define E_FREAD(buffer, size, count, stream) evil_fread_native(buffer, size, count, stream)
# define E_FCLOSE(stream) evil_fclose_native(stream)
#else
# define E_FOPEN(file, mode) fopen((file), (mode))
# define E_FREAD(buffer, size, count, stream) fread(buffer, size, count, stream)
# define E_FCLOSE(stream) fclose(stream)
#endif

#include "evas_macros.h"

#include "evas_cserve2.h"
#include "evas_cserve2_slave.h"

#define PNG_BYTES_TO_CHECK 4


static Eina_Bool
evas_image_load_file_head_png(Evas_Img_Load_Params *ilp, const char *file, const char *key __UNUSED__, int *error)
{
   png_uint_32 w32, h32;
   FILE *f;
   png_structp png_ptr = NULL;
   png_infop info_ptr = NULL;
   int bit_depth, color_type, interlace_type;
   unsigned char buf[PNG_BYTES_TO_CHECK];
   char hasa;

   hasa = 0;
   f = E_FOPEN(file, "rb");
   if (!f)
     {
	*error = CSERVE2_DOES_NOT_EXIST;
	return EINA_FALSE;
     }

   /* if we havent read the header before, set the header data */
   if (E_FREAD(buf, PNG_BYTES_TO_CHECK, 1, f) != 1)
     {
	*error = CSERVE2_UNKNOWN_FORMAT;
	goto close_file;
     }

   if (png_sig_cmp(buf, 0, PNG_BYTES_TO_CHECK))
     {
	*error = CSERVE2_UNKNOWN_FORMAT;
	goto close_file;
     }

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png_ptr)
     {
	*error = CSERVE2_RESOURCE_ALLOCATION_FAILED;
	goto close_file;
     }

   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
     {
	png_destroy_read_struct(&png_ptr, NULL, NULL);
	*error = CSERVE2_RESOURCE_ALLOCATION_FAILED;
	goto close_file;
     }
   if (setjmp(png_jmpbuf(png_ptr)))
     {
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	*error = CSERVE2_CORRUPT_FILE;
	goto close_file;
     }
   png_init_io(png_ptr, f);
   png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);
   png_read_info(png_ptr, info_ptr);
   png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *) (&w32),
		(png_uint_32 *) (&h32), &bit_depth, &color_type,
		&interlace_type, NULL, NULL);
   if ((w32 < 1) || (h32 < 1) || (w32 > IMG_MAX_SIZE) || (h32 > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(w32, h32))
     {
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	if (IMG_TOO_BIG(w32, h32))
	  *error = CSERVE2_RESOURCE_ALLOCATION_FAILED;
	else
	  *error = CSERVE2_GENERIC;
	goto close_file;
     }
   if (ilp->opts.scale_down_by > 1)
     {
        ilp->w = (int) w32 / ilp->opts.scale_down_by;
        ilp->h = (int) h32 / ilp->opts.scale_down_by;
        if ((ilp->w < 1) || (ilp->h < 1))
          {
             *error = CSERVE2_GENERIC;
             goto close_file;
          }
     }
   else
     {
        ilp->w = (int) w32;
        ilp->h = (int) h32;
     }
   if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) hasa = 1;
   if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) hasa = 1;
   if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) hasa = 1;
   if (hasa) ilp->alpha = 1;
   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
   E_FCLOSE(f);

   *error = CSERVE2_NONE;
   return EINA_TRUE;

 close_file:
   E_FCLOSE(f);
   return EINA_FALSE;
}

static Eina_Bool
evas_image_load_file_data_png(Evas_Img_Load_Params *ilp, const char *file, const char *key __UNUSED__, int *error)
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
   int i, j;
   int scale_ratio = 1, image_w = 0;
   unsigned char *tmp_line;
   DATA32 *src_ptr, *dst_ptr;

   hasa = 0;
   f = E_FOPEN(file, "rb");
   if (!f)
     {
	*error = CSERVE2_DOES_NOT_EXIST;
	return EINA_FALSE;
     }

   /* if we havent read the header before, set the header data */
   if (E_FREAD(buf, PNG_BYTES_TO_CHECK, 1, f) != 1)
     {
	*error = CSERVE2_CORRUPT_FILE;
        goto close_file;
     }
   if (png_sig_cmp(buf, 0, PNG_BYTES_TO_CHECK))
     {
	*error = CSERVE2_CORRUPT_FILE;
	goto close_file;
     }
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png_ptr)
     {
	*error = CSERVE2_RESOURCE_ALLOCATION_FAILED;
	goto close_file;
     }

   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
     {
	png_destroy_read_struct(&png_ptr, NULL, NULL);
	*error = CSERVE2_RESOURCE_ALLOCATION_FAILED;
	goto close_file;
     }
   if (setjmp(png_jmpbuf(png_ptr)))
     {
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	*error = CSERVE2_CORRUPT_FILE;
	goto close_file;
     }
   png_init_io(png_ptr, f);
   png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);
   png_read_info(png_ptr, info_ptr);
   png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *) (&w32),
		(png_uint_32 *) (&h32), &bit_depth, &color_type,
		&interlace_type, NULL, NULL);
   image_w = w32;
   if (ilp->opts.scale_down_by > 1)
     {
        scale_ratio = ilp->opts.scale_down_by;
        w32 /= scale_ratio;
        h32 /= scale_ratio;
     }
   surface = (unsigned char *) ilp->buffer;
   if (!surface)
     {
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	*error = CSERVE2_RESOURCE_ALLOCATION_FAILED;
	goto close_file;
     }

   if ((w32 != ilp->w) || (h32 != ilp->h))
     {
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	*error = CSERVE2_GENERIC;
	goto close_file;
     }

   if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) hasa = 1;
   if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) hasa = 1;
   if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) hasa = 1;
   if (hasa) ilp->alpha = 1;

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

   w = ilp->w;
   h = ilp->h;
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

   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
   E_FCLOSE(f);
   evas_cserve2_image_premul(ilp);

   *error = CSERVE2_NONE;
   return EINA_TRUE;

 close_file:
   E_FCLOSE(f);
   return EINA_FALSE;
}

static Evas_Loader_Module_Api modapi =
{
   EVAS_CSERVE2_MODULE_API_VERSION,
   "png",
   evas_image_load_file_head_png,
   evas_image_load_file_data_png
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

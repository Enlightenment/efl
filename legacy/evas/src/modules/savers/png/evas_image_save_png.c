
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <png.h>
#include <setjmp.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "evas_common.h"
#include "evas_private.h"

int evas_image_save_file_png(RGBA_Image *im, const char *file, const char *key, int quality, int compress);

Evas_Image_Save_Func evas_image_save_png_func =
{
   evas_image_save_file_png
};

static int
save_image_png(RGBA_Image *im, const char *file, int compress, int interlace)
{
   FILE               *f;
   png_structp         png_ptr;
   png_infop           info_ptr;
   DATA32             *ptr, *data = NULL;
   int                 x, y, j;
   png_bytep           row_ptr, png_data = NULL;
   png_color_8         sig_bit;
   int                 num_passes = 1, pass;

   if (!im || !im->image.data || !file)
      return 0;
   
   f = fopen(file, "wb");
   if (!f) return 0;
   
   png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png_ptr)
     {
	fclose(f);
	return 0;
     }
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
     {
	fclose(f);
	png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
	return 0;
     }
   if (setjmp(png_ptr->jmpbuf))
     {
	fclose(f);
	png_destroy_write_struct(&png_ptr, (png_infopp) & info_ptr);
	png_destroy_info_struct(png_ptr, (png_infopp) & info_ptr);
	return 0;
     }
   
   if (interlace)
     {
#ifdef PNG_WRITE_INTERLACING_SUPPORTED
	png_ptr->interlaced = PNG_INTERLACE_ADAM7;
	num_passes = png_set_interlace_handling(png_ptr);
#endif
     }
   
   if (im->cache_entry.flags.alpha)
     {
	data = malloc(im->cache_entry.w * im->cache_entry.h * sizeof(DATA32));
	if (!data)
	  {
	    fclose(f);
	    png_destroy_write_struct(&png_ptr, (png_infopp) & info_ptr);
	    png_destroy_info_struct(png_ptr, (png_infopp) & info_ptr);
	    return 0;
	  }
	memcpy(data, im->image.data, im->cache_entry.w * im->cache_entry.h * sizeof(DATA32));
	evas_common_convert_argb_unpremul(data, im->cache_entry.w * im->cache_entry.h);
	png_init_io(png_ptr, f);
        png_set_IHDR(png_ptr, info_ptr, im->cache_entry.w, im->cache_entry.h, 8,
		     PNG_COLOR_TYPE_RGB_ALPHA, png_ptr->interlaced,
		     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
#ifdef WORDS_BIGENDIAN
	png_set_swap_alpha(png_ptr);
#else
	png_set_bgr(png_ptr);
#endif
     }
   else
     {
	data = im->image.data;
	png_init_io(png_ptr, f);
	png_set_IHDR(png_ptr, info_ptr, im->cache_entry.w, im->cache_entry.h, 8,
		     PNG_COLOR_TYPE_RGB, png_ptr->interlaced,
		     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_data = alloca(im->cache_entry.w * 3 * sizeof(char));
     }
   sig_bit.red = 8;
   sig_bit.green = 8;
   sig_bit.blue = 8;
   sig_bit.alpha = 8;
   png_set_sBIT(png_ptr, info_ptr, &sig_bit);
   
   png_set_compression_level(png_ptr, compress);
   png_write_info(png_ptr, info_ptr);
   png_set_shift(png_ptr, &sig_bit);
   png_set_packing(png_ptr);

   for (pass = 0; pass < num_passes; pass++)
     {
	ptr = data;
	
	for (y = 0; y < im->cache_entry.h; y++)
	  {
	     if (im->cache_entry.flags.alpha)
	       row_ptr = (png_bytep) ptr;
	     else
	       {
		  for (j = 0, x = 0; x < im->cache_entry.w; x++)
		    {
		       png_data[j++] = (ptr[x] >> 16) & 0xff;
		       png_data[j++] = (ptr[x] >> 8) & 0xff;
		       png_data[j++] = (ptr[x]) & 0xff;
		    }
		  row_ptr = (png_bytep) png_data;
	       }
	     png_write_rows(png_ptr, &row_ptr, 1);
	     ptr += im->cache_entry.w;
	  }
     }
   png_write_end(png_ptr, info_ptr);
   png_destroy_write_struct(&png_ptr, (png_infopp) & info_ptr);
   png_destroy_info_struct(png_ptr, (png_infopp) & info_ptr);
   
   if (im->cache_entry.flags.alpha)
     free(data);
   fclose(f);
   return 1;
}

int evas_image_save_file_png(RGBA_Image *im, const char *file, const char *key, int quality, int compress)
{
   return save_image_png(im, file, compress, 0);
}

EAPI int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_save_png_func);
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
     "png",
     "none"
};

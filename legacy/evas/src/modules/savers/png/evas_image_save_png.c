#include <png.h>
#include <setjmp.h>

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
   DATA32             *ptr;
   int                 x, y, j;
   png_bytep           row_ptr, data = NULL;
   png_color_8         sig_bit;
   int                 num_passes = 1, pass;

   if (!im || !im->image || !im->image->data || !file)
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
   
   png_init_io(png_ptr, f);
   
   if (im->flags & RGBA_IMAGE_HAS_ALPHA)
     {
        png_set_IHDR(png_ptr, info_ptr, im->image->w, im->image->h, 8,
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
	png_set_IHDR(png_ptr, info_ptr, im->image->w, im->image->h, 8,
		     PNG_COLOR_TYPE_RGB, png_ptr->interlaced,
		     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	data = alloca(im->image->w * 3 * sizeof(char));
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
	ptr = im->image->data;
	
	for (y = 0; y < im->image->h; y++)
	  {
	     if (im->flags & RGBA_IMAGE_HAS_ALPHA)
	       row_ptr = (png_bytep) ptr;
	     else
	       {
		  for (j = 0, x = 0; x < im->image->w; x++)
		    {
		       data[j++] = (ptr[x] >> 16) & 0xff;
		       data[j++] = (ptr[x] >> 8) & 0xff;
		       data[j++] = (ptr[x]) & 0xff;
		    }
		  row_ptr = (png_bytep) data;
	       }
	     png_write_rows(png_ptr, &row_ptr, 1);
	     ptr += im->image->w;
	  }
     }
   png_write_end(png_ptr, info_ptr);
   png_destroy_write_struct(&png_ptr, (png_infopp) & info_ptr);
   png_destroy_info_struct(png_ptr, (png_infopp) & info_ptr);
   
   fclose(f);
   return 1;
}

int evas_image_save_file_png(RGBA_Image *im, const char *file, const char *key, int quality, int compress)
{
   return save_image_png(im, file, compress, 0);
}

int module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_save_png_func);
   return 1;
}

void module_close(void)
{
   
}

Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
     EVAS_MODULE_TYPE_IMAGE_SAVER,
     "png",
     "none"
};

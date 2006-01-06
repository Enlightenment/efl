#include "config.h"
#include "evas_options.h"

#define SWAP32(x) (x) = ((((x) & 0x000000ff ) << 24) | (((x) & 0x0000ff00 ) << 8) | (((x) & 0x00ff0000 ) >> 8) | (((x) & 0xff000000 ) >> 24))

#ifdef BUILD_LOADER_PNG
#include <png.h>
#include <setjmp.h>
#define PNG_BYTES_TO_CHECK 4
#endif

#ifdef BUILD_LOADER_JPEG
#include <stdio.h>
#include <jpeglib.h>
#include <setjmp.h>
#endif

#ifdef BUILD_LOADER_EET
#include <Eet.h>
#endif

#ifdef BUILD_LOADER_EDB
#include <Edb.h>
#include <zlib.h>
#endif

#ifdef BUILD_LOADER_TEMPLATE
#include <Template.h>
#endif

#include "evas_common.h"
#include "evas_private.h"

#ifdef BUILD_LOADER_PNG
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
   int                 quality = 75, compression = 3, num_passes = 1, pass;
   
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
#endif

#ifdef BUILD_LOADER_JPEG
struct _JPEG_error_mgr
{
   struct     jpeg_error_mgr pub;
   jmp_buf    setjmp_buffer;
};
typedef struct _JPEG_error_mgr *emptr;

static void _JPEGFatalErrorHandler(j_common_ptr cinfo);
static void
_JPEGFatalErrorHandler(j_common_ptr cinfo)
{
   emptr errmgr;
   
   errmgr = (emptr) cinfo->err;
   longjmp(errmgr->setjmp_buffer, 1);
   return;
}

static void _JPEGErrorHandler(j_common_ptr cinfo);
static void
_JPEGErrorHandler(j_common_ptr cinfo)
{
   emptr errmgr;
   
   errmgr = (emptr) cinfo->err;
   return;
}

static void _JPEGErrorHandler2(j_common_ptr cinfo, int msg_level);
static void
_JPEGErrorHandler2(j_common_ptr cinfo, int msg_level)
{
   emptr errmgr;
   
   errmgr = (emptr) cinfo->err;
   return;
   msg_level = 0;
}

static int
save_image_jpeg(RGBA_Image *im, const char *file, int quality)
{
   struct _JPEG_error_mgr jerr;
   struct jpeg_compress_struct cinfo;
   FILE               *f;
   DATA8              *buf;
   DATA32             *ptr;
   JSAMPROW           *jbuf;
   int                 y = 0;
   int                 i, j;
   
   buf = alloca(im->image->w * 3 * sizeof(DATA8));
   f = fopen(file, "wb");
   if (!f)
     {
	return 0;
     }
   jerr.pub.error_exit = _JPEGFatalErrorHandler;
   jerr.pub.emit_message = _JPEGErrorHandler2;
   jerr.pub.output_message = _JPEGErrorHandler;
   cinfo.err = jpeg_std_error(&(jerr.pub));
   if (sigsetjmp(jerr.setjmp_buffer, 1))
     {
	jpeg_destroy_compress(&cinfo);
	fclose(f);
	return 0;
     }
   jpeg_create_compress(&cinfo);
   jpeg_stdio_dest(&cinfo, f);
   cinfo.image_width = im->image->w;
   cinfo.image_height = im->image->h;
   cinfo.input_components = 3;
   cinfo.in_color_space = JCS_RGB;
   jpeg_set_defaults(&cinfo);
   jpeg_set_quality(&cinfo, quality, TRUE);
   jpeg_start_compress(&cinfo, TRUE);
   ptr = im->image->data;
   while (cinfo.next_scanline < cinfo.image_height)
     {
	for (j = 0, i = 0; i < im->image->w; i++)
	  {
	     buf[j++] = ((*ptr) >> 16) & 0xff;
	     buf[j++] = ((*ptr) >> 8) & 0xff;
	     buf[j++] = ((*ptr)) & 0xff;
	     ptr++;
	  }
	jbuf = (JSAMPROW *) (&buf);
	jpeg_write_scanlines(&cinfo, jbuf, 1);
	y++;
     }
   jpeg_finish_compress(&cinfo);
   jpeg_destroy_compress(&cinfo);
   fclose(f);
   return 1;
}
#endif

#ifdef BUILD_LOADER_EET
static int
save_image_eet(RGBA_Image *im, const char *file, const char *key, int quality, int compress)
{
   Eet_File            *ef;
   int alpha = 0, lossy = 0, ok = 0;

   ef = eet_open((char *)file, EET_FILE_MODE_READ_WRITE);
   if (!ef) ef = eet_open((char *)file, EET_FILE_MODE_WRITE);
   if (!ef) return 0;
   if ((quality <= 100) || (compress < 0)) lossy = 1;
   if (im->flags & RGBA_IMAGE_HAS_ALPHA) alpha = 1;
   ok = eet_data_image_write(ef, (char *)key, im->image->data,
			     im->image->w, im->image->h, alpha, compress,
			     quality, lossy);
   eet_close(ef);
   return ok;
}
#endif

#ifdef BUILD_LOADER_EDB
static int
save_image_edb(RGBA_Image *im, const char *file, const char *key, int quality, int compress)
{
   return 0;
}
#endif

#ifdef BUILD_LOADER_TEMPLATE
#endif

int
evas_common_save_image_to_file(RGBA_Image *im, const char *file, const char *key, int quality, int compress)
{
   char *p;
   
   p = strrchr(file, '.');
   if (p)
     {
	p++;
	
#ifdef BUILD_LOADER_PNG
	if (!strcasecmp(p, "png"))
	  return save_image_png(im, file, compress, 0);
#endif
#ifdef BUILD_LOADER_JPEG
	if ((!strcasecmp(p, "jpg")) ||
	    (!strcasecmp(p, "jpeg")) ||
	    (!strcasecmp(p, "jfif")))
	  return save_image_jpeg(im, file, quality);
#endif
#ifdef BUILD_LOADER_EET
	if (!strcasecmp(p, "eet"))
	  return save_image_eet(im, file, key, quality, compress);
#endif
#ifdef BUILD_LOADER_EDB
	if (!strcasecmp(p, "edb"))
	  return save_image_edb(im, file, key, quality, compress);
#endif
     }
   return 0;
}

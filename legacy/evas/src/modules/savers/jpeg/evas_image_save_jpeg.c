#include "evas_common.h"
#include "evas_private.h"

#include <stdio.h>
#include <jpeglib.h>
#include <setjmp.h>

int evas_image_save_file_jpeg(RGBA_Image *im, const char *file, const char *key, int quality, int compress);

Evas_Image_Save_Func evas_image_save_jpeg_func =
{
   evas_image_save_file_jpeg
};

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

   if (!im || !im->image.data || !file)
      return 0;
   
   buf = alloca(im->cache_entry.w * 3 * sizeof(DATA8));
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
   cinfo.image_width = im->cache_entry.w;
   cinfo.image_height = im->cache_entry.h;
   cinfo.input_components = 3;
   cinfo.in_color_space = JCS_RGB;
   jpeg_set_defaults(&cinfo);
   jpeg_set_quality(&cinfo, quality, TRUE);
   jpeg_start_compress(&cinfo, TRUE);
   ptr = im->image.data;
   while (cinfo.next_scanline < cinfo.image_height)
     {
	for (j = 0, i = 0; i < im->cache_entry.w; i++)
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

int evas_image_save_file_jpeg(RGBA_Image *im, const char *file, const char *key, int quality, int compress)
{
   return save_image_jpeg(im, file, quality);
}

EAPI int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_save_jpeg_func);
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
     "jpeg",
     "none"
};

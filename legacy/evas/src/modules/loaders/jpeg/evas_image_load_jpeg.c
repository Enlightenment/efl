#include <stdio.h>
#include <jpeglib.h>
#include <setjmp.h>

#include "evas_common.h"
#include "evas_private.h"


typedef struct _JPEG_error_mgr *emptr;
struct _JPEG_error_mgr
{
   struct     jpeg_error_mgr pub;
   jmp_buf    setjmp_buffer;
};

static void _JPEGFatalErrorHandler(j_common_ptr cinfo);
static void _JPEGErrorHandler(j_common_ptr cinfo);
static void _JPEGErrorHandler2(j_common_ptr cinfo, int msg_level);

static int evas_image_load_file_head_jpeg_internal(RGBA_Image *im, FILE *f);
static int evas_image_load_file_data_jpeg_internal(RGBA_Image *im, FILE *f);
#if 0 /* not used at the moment */
static int evas_image_load_file_data_jpeg_alpha_internal(RGBA_Image *im, FILE *f);
#endif

int evas_image_load_file_head_jpeg(RGBA_Image *im, const char *file, const char *key);
int evas_image_load_file_data_jpeg(RGBA_Image *im, const char *file, const char *key);

Evas_Image_Load_Func evas_image_load_jpeg_func =
{
  evas_image_load_file_head_jpeg,
  evas_image_load_file_data_jpeg
};


static void
_JPEGFatalErrorHandler(j_common_ptr cinfo)
{
   emptr errmgr;

   errmgr = (emptr) cinfo->err;
   /*   cinfo->err->output_message(cinfo);*/
   longjmp(errmgr->setjmp_buffer, 1);
   return;
}

static void
_JPEGErrorHandler(j_common_ptr cinfo)
{
   emptr errmgr;

   errmgr = (emptr) cinfo->err;
   /*   cinfo->err->output_message(cinfo);*/
   /*   longjmp(errmgr->setjmp_buffer, 1);*/
   return;
}

static void
_JPEGErrorHandler2(j_common_ptr cinfo, int msg_level)
{
   emptr errmgr;

   errmgr = (emptr) cinfo->err;
   /*   cinfo->err->output_message(cinfo);*/
   /*   longjmp(errmgr->setjmp_buffer, 1);*/
   return;
   msg_level = 0;
}

static int
evas_image_load_file_head_jpeg_internal(RGBA_Image *im, FILE *f)
{
   int w, h, scalew, scaleh;
   struct jpeg_decompress_struct cinfo;
   struct _JPEG_error_mgr jerr;

   if (!f) return 0;
   cinfo.err = jpeg_std_error(&(jerr.pub));
   jerr.pub.error_exit = _JPEGFatalErrorHandler;
   jerr.pub.emit_message = _JPEGErrorHandler2;
   jerr.pub.output_message = _JPEGErrorHandler;
   if (setjmp(jerr.setjmp_buffer))
     {
	jpeg_destroy_decompress(&cinfo);
	return 0;
     }
   jpeg_create_decompress(&cinfo);
   jpeg_stdio_src(&cinfo, f);
   jpeg_read_header(&cinfo, TRUE);
   cinfo.do_fancy_upsampling = FALSE;
   cinfo.do_block_smoothing = FALSE;
   jpeg_start_decompress(&cinfo);

/* head decoding */
   if (!im->image)
     im->image = evas_common_image_surface_new(im);
   if (!im->image)
     {
	jpeg_destroy_decompress(&cinfo);
	return 0;
     }
   w = cinfo.output_width;
   h = cinfo.output_height;
   if (im->load_opts.scale_down_by > 1)
     {
	w /= im->load_opts.scale_down_by;
	h /= im->load_opts.scale_down_by;
     }
   else if (im->load_opts.dpi > 0.0)
     {
	w = (w * im->load_opts.dpi) / 90.0;
	h = (h * im->load_opts.dpi) / 90.0;
     }
   else if ((im->load_opts.w > 0) &&
	    (im->load_opts.h > 0))
     {
	int w2, h2;
	
	w2 = im->load_opts.w;
	h2 = (im->load_opts.w * h) / w;
	if (h2 > im->load_opts.h)
	  {
	     h2 = im->load_opts.h;
	     w2 = (im->load_opts.h * w) / h;
	  }
	w = w2;
	h = h2;
     }
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   
   if ((w != cinfo.output_width) || (h != cinfo.output_height))
     {
	scalew = cinfo.output_width / w;
	scaleh = cinfo.output_height / h;
	
	im->scale = scalew;
	if (scaleh < scalew) im->scale = scaleh;
	
	if      (im->scale > 8) im->scale = 8;
	else if (im->scale < 1) im->scale = 1;
	
	if      (im->scale == 3) im->scale = 2;
	else if (im->scale == 5) im->scale = 4;
	else if (im->scale == 6) im->scale = 4;
	else if (im->scale == 7) im->scale = 4;
     }

   if (im->scale > 1)
     {
	jpeg_destroy_decompress(&cinfo);
   
	rewind(f);
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, f);
	jpeg_read_header(&cinfo, TRUE);
	cinfo.do_fancy_upsampling = FALSE;
	cinfo.do_block_smoothing = FALSE;
	cinfo.scale_num = 1;
	cinfo.scale_denom = im->scale;
	jpeg_calc_output_dimensions(&(cinfo));
	jpeg_start_decompress(&cinfo);
     }
   
   im->image->w = cinfo.output_width;
   im->image->h = cinfo.output_height;
/* end head decoding */

   jpeg_destroy_decompress(&cinfo);
   return 1;
}

static int
evas_image_load_file_data_jpeg_internal(RGBA_Image *im, FILE *f)
{
   int w, h;
   struct jpeg_decompress_struct cinfo;
   struct _JPEG_error_mgr jerr;
   DATA8 *ptr, *line[16], *data;
   DATA32 *ptr2;
   int x, y, l, i, scans, count, prevy;

   if (!f) return 0;
   cinfo.err = jpeg_std_error(&(jerr.pub));
   jerr.pub.error_exit = _JPEGFatalErrorHandler;
   jerr.pub.emit_message = _JPEGErrorHandler2;
   jerr.pub.output_message = _JPEGErrorHandler;
   if (setjmp(jerr.setjmp_buffer))
     {
	jpeg_destroy_decompress(&cinfo);
	return 0;
     }
   jpeg_create_decompress(&cinfo);
   jpeg_stdio_src(&cinfo, f);
   jpeg_read_header(&cinfo, TRUE);
   cinfo.do_fancy_upsampling = FALSE;
   cinfo.do_block_smoothing = FALSE;
   cinfo.dct_method = JDCT_IFAST;
   
   if (im->scale > 1)
     {
	cinfo.scale_num = 1;
	cinfo.scale_denom = im->scale;
     }
   
/* head decoding */
   jpeg_calc_output_dimensions(&(cinfo));
   jpeg_start_decompress(&cinfo);
   
   w = cinfo.output_width;
   h = cinfo.output_height;
   
   if ((w != im->image->w) || (h != im->image->h))
     {
	jpeg_destroy_decompress(&cinfo);
	return 0;
     }
   
/* end head decoding */
/* data decoding */
   if (cinfo.rec_outbuf_height > 16)
     {
	jpeg_destroy_decompress(&cinfo);
	return 0;
     }
   data = alloca(w * 16 * 3);
   evas_common_image_surface_alloc(im->image);
   if (!im->image->data)
     {
	jpeg_destroy_decompress(&cinfo);
	return 0;
     }
   ptr2 = im->image->data;
   count = 0;
   prevy = 0;
   if (cinfo.output_components == 3)
     {
	for (i = 0; i < cinfo.rec_outbuf_height; i++)
	  line[i] = data + (i * w * 3);
	for (l = 0; l < h; l += cinfo.rec_outbuf_height)
	  {
	     jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
	     scans = cinfo.rec_outbuf_height;
	     if ((h - l) < scans) scans = h - l;
	     ptr = data;
	     for (y = 0; y < scans; y++)
	       {
		  for (x = 0; x < w; x++)
		    {
		       *ptr2 =
			 (0xff000000) | ((ptr[0]) << 16) | ((ptr[1]) << 8) | (ptr[2]);
		       ptr += 3;
		       ptr2++;
		    }
	       }
	  }
     }
   else if (cinfo.output_components == 1)
     {
	for (i = 0; i < cinfo.rec_outbuf_height; i++)
	  line[i] = data + (i * w);
	for (l = 0; l < h; l += cinfo.rec_outbuf_height)
	  {
	     jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
	     scans = cinfo.rec_outbuf_height;
	     if ((h - l) < scans) scans = h - l;
	     ptr = data;
	     for (y = 0; y < scans; y++)
	       {
		  for (x = 0; x < w; x++)
		    {
		       *ptr2 =
			 (0xff000000) | ((ptr[0]) << 16) | ((ptr[0]) << 8) | (ptr[0]);
		       ptr++;
		       ptr2++;
		    }
	       }
	  }
     }
/* end data decoding */
   jpeg_finish_decompress(&cinfo);
   jpeg_destroy_decompress(&cinfo);
   return 1;
}

#if 0 /* not used at the moment */
static int
evas_image_load_file_data_jpeg_alpha_internal(RGBA_Image *im, FILE *f)
{
   int w, h;
   struct jpeg_decompress_struct cinfo;
   struct _JPEG_error_mgr jerr;
   DATA8 *ptr, *line[16], *data;
   DATA32 *ptr2;
   int x, y, l, i, scans, count, prevy;

   if (!f) return 0;
   cinfo.err = jpeg_std_error(&(jerr.pub));
   jerr.pub.error_exit = _JPEGFatalErrorHandler;
   jerr.pub.emit_message = _JPEGErrorHandler2;
   jerr.pub.output_message = _JPEGErrorHandler;
   if (setjmp(jerr.setjmp_buffer))
     {
	jpeg_destroy_decompress(&cinfo);
	return 0;
     }
   jpeg_create_decompress(&cinfo);
   jpeg_stdio_src(&cinfo, f);
   jpeg_read_header(&cinfo, TRUE);
   cinfo.do_fancy_upsampling = FALSE;
   cinfo.do_block_smoothing = FALSE;
   jpeg_start_decompress(&cinfo);

/* head decoding */
   im->image->w = w = cinfo.output_width;
   im->image->h = h = cinfo.output_height;
/* end head decoding */
/* data decoding */
   if (cinfo.rec_outbuf_height > 16)
     {
	jpeg_destroy_decompress(&cinfo);
	return 0;
     }
   data = alloca(w * 16 * 3);
   if (!im->image->data)
     {
	jpeg_destroy_decompress(&cinfo);
	return 0;
     }
   ptr2 = im->image->data;
   count = 0;
   prevy = 0;
   if (cinfo.output_components == 3)
     {
	for (i = 0; i < cinfo.rec_outbuf_height; i++)
	  line[i] = data + (i * w * 3);
	for (l = 0; l < h; l += cinfo.rec_outbuf_height)
	  {
	     jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
	     scans = cinfo.rec_outbuf_height;
	     if ((h - l) < scans) scans = h - l;
	     ptr = data;
	     for (y = 0; y < scans; y++)
	       {
		  for (x = 0; x < w; x++)
		    {
		       *ptr2 =
			 ((*ptr2) & 0x00ffffff) |
			 (((ptr[0] + ptr[1] + ptr[2]) / 3) << 24);
		       ptr += 3;
		       ptr2++;
		    }
	       }
	  }
     }
   else if (cinfo.output_components == 1)
     {
	for (i = 0; i < cinfo.rec_outbuf_height; i++)
	  line[i] = data + (i * w);
	for (l = 0; l < h; l += cinfo.rec_outbuf_height)
	  {
	     jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
	     scans = cinfo.rec_outbuf_height;
	     if ((h - l) < scans) scans = h - l;
	     ptr = data;
	     for (y = 0; y < scans; y++)
	       {
		  for (x = 0; x < w; x++)
		    {
		       *ptr2 =
			 ((*ptr2) & 0x00ffffff) |
			 ((ptr[0]) << 24);
		       ptr++;
		       ptr2++;
		    }
	       }
	  }
     }
/* end data decoding */
   jpeg_finish_decompress(&cinfo);
   jpeg_destroy_decompress(&cinfo);
   return 1;
}
#endif

int
evas_image_load_file_head_jpeg(RGBA_Image *im, const char *file, const char *key)
{
   int val;
   FILE *f;

   if ((!file)) return 0;
   f = fopen(file, "rb");
   if (!f) return 0;
   val = evas_image_load_file_head_jpeg_internal(im, f);
   fclose(f);
   return val;
   key = 0;
}

int
evas_image_load_file_data_jpeg(RGBA_Image *im, const char *file, const char *key)
{
   int val;
   FILE *f;

   if ((!file)) return 0;
   f = fopen(file, "rb");
   if (!f) return 0;
   val = evas_image_load_file_data_jpeg_internal(im, f);
   fclose(f);
   return val;
   key = 0;
}

EAPI int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_jpeg_func);
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
     "jpeg",
     "none"
};

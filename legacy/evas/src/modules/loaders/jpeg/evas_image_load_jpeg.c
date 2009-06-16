#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <jpeglib.h>
#include <setjmp.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

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

static int evas_image_load_file_head_jpeg_internal(Image_Entry *ie, FILE *f);
static int evas_image_load_file_data_jpeg_internal(Image_Entry *ie, FILE *f);
#if 0 /* not used at the moment */
static int evas_image_load_file_data_jpeg_alpha_internal(Image_Entry *ie, FILE *f);
#endif

static int evas_image_load_file_head_jpeg(Image_Entry *ie, const char *file, const char *key);
static int evas_image_load_file_data_jpeg(Image_Entry *ie, const char *file, const char *key);

static Evas_Image_Load_Func evas_image_load_jpeg_func =
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
evas_image_load_file_head_jpeg_internal(Image_Entry *ie, FILE *f)
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
   cinfo.dct_method = JDCT_IFAST;
   cinfo.dither_mode = JDITHER_ORDERED;
   jpeg_start_decompress(&cinfo);

/* head decoding */
   w = cinfo.output_width;
   h = cinfo.output_height;
   if ((w < 1) || (h < 1) || (w > 8192) || (h > 8192))
     {
        jpeg_destroy_decompress(&cinfo);
	return 0;
     }
   if (ie->load_opts.scale_down_by > 1)
     {
	w /= ie->load_opts.scale_down_by;
	h /= ie->load_opts.scale_down_by;
     }
   else if (ie->load_opts.dpi > 0.0)
     {
	w = (w * ie->load_opts.dpi) / 90.0;
	h = (h * ie->load_opts.dpi) / 90.0;
     }
   else if ((ie->load_opts.w) && (ie->load_opts.h))
     {
	int w2, h2;
	if (ie->load_opts.w > 0)
	  {
	     w2 = ie->load_opts.w;
	     h2 = (ie->load_opts.w * h) / w;
	     if ((ie->load_opts.h > 0) && (h2 > ie->load_opts.h))
	       {
		  h2 = ie->load_opts.h;
		  w2 = (ie->load_opts.h * w) / h;
	       }
	  }
	else if (ie->load_opts.h > 0)
	  {
	     h2 = ie->load_opts.h;
	     w2 = (ie->load_opts.h * w) / h;
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

	ie->scale = scalew;
	if (scaleh < scalew) ie->scale = scaleh;

	if      (ie->scale > 8) ie->scale = 8;
	else if (ie->scale < 1) ie->scale = 1;

	if      (ie->scale == 3) ie->scale = 2;
	else if (ie->scale == 5) ie->scale = 4;
	else if (ie->scale == 6) ie->scale = 4;
	else if (ie->scale == 7) ie->scale = 4;
     }

   if (ie->scale > 1)
     {
	jpeg_destroy_decompress(&cinfo);

	rewind(f);
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, f);
	jpeg_read_header(&cinfo, TRUE);
	cinfo.do_fancy_upsampling = FALSE;
	cinfo.do_block_smoothing = FALSE;
	cinfo.scale_num = 1;
	cinfo.scale_denom = ie->scale;
	jpeg_calc_output_dimensions(&(cinfo));
	jpeg_start_decompress(&cinfo);
     }

   ie->w = cinfo.output_width;
   ie->h = cinfo.output_height;
/* end head decoding */

   jpeg_destroy_decompress(&cinfo);
   return 1;
}

static int
evas_image_load_file_data_jpeg_internal(Image_Entry *ie, FILE *f)
{
   int w, h;
   struct jpeg_decompress_struct cinfo;
   struct _JPEG_error_mgr jerr;
   DATA8 *ptr, *line[16], *data;
   DATA32 *ptr2;
   int x, y, l, i, scans, count;

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
   cinfo.dither_mode = JDITHER_ORDERED;

   if (ie->scale > 1)
     {
	cinfo.scale_num = 1;
	cinfo.scale_denom = ie->scale;
     }

   /* Colorspace conversion options */
   /* libjpeg can do the following conversions: */
   /* GRAYSCLAE => RGB YCbCr => RGB and YCCK => CMYK */
   switch (cinfo.jpeg_color_space)
     {
     case JCS_UNKNOWN:
       break;
     case JCS_GRAYSCALE:
     case JCS_RGB:
     case JCS_YCbCr:
       cinfo.out_color_space = JCS_RGB;
       break;
     case JCS_CMYK:
     case JCS_YCCK:
       cinfo.out_color_space = JCS_CMYK;
       break;
     }

/* head decoding */
   jpeg_calc_output_dimensions(&(cinfo));
   jpeg_start_decompress(&cinfo);

   w = cinfo.output_width;
   h = cinfo.output_height;

   if ((w != ie->w) || (h != ie->h))
     {
	jpeg_destroy_decompress(&cinfo);
	return 0;
     }

   if (!(((cinfo.out_color_space == JCS_RGB) &&
          ((cinfo.output_components == 3) || (cinfo.output_components == 1))) ||
         ((cinfo.out_color_space == JCS_CMYK) && (cinfo.output_components == 4))))
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
   data = alloca(w * 16 * cinfo.output_components);
   evas_cache_image_surface_alloc(ie, w, h);
   if (ie->flags.loaded)
     {
	jpeg_destroy_decompress(&cinfo);
	return 0;
     }
   ptr2 = evas_cache_image_pixels(ie);
   count = 0;
   /* We handle first CMYK (4 components) */
   if (cinfo.output_components == 4)
     {
	for (i = 0; i < cinfo.rec_outbuf_height; i++)
	  line[i] = data + (i * w * 4);
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
                       if (cinfo.saw_Adobe_marker)
                         /* According to libjpeg doc, Photoshop inverse the values of C, M, Y and K, */
                         /* that is C is replaces by 255 - C, etc...*/
                         /* See the comment below for the computation of RGB values from CMYK ones. */
                         *ptr2 =
                           (0xff000000) |
                           ((ptr[0] * ptr[3] / 255) << 16) |
                           ((ptr[1] * ptr[3] / 255) << 8) |
                           ((ptr[2] * ptr[3] / 255));
                       else
                         /* Conversion from CMYK to RGB is done in 2 steps: */
                         /* CMYK => CMY => RGB (see http://www.easyrgb.com/index.php?X=MATH) */
                         /* after computation, if C, M, Y and K are between 0 and 1, we have: */
                         /* R = (1 - C) * (1 - K) * 255 */
                         /* G = (1 - M) * (1 - K) * 255 */
                         /* B = (1 - Y) * (1 - K) * 255 */
                         /* libjpeg stores CMYK values between 0 and 255, */
                         /* so we replace C by C * 255 / 255, etc... and we obtain: */
                         /* R = (255 - C) * (255 - K) / 255 */
                         /* G = (255 - M) * (255 - K) / 255 */
                         /* B = (255 - Y) * (255 - K) / 255 */
                         /* with C, M, Y and K between 0 and 255. */
                         *ptr2 =
                           (0xff000000) |
                           (((255 - ptr[0]) * (255 - ptr[3]) / 255) << 16) |
                           (((255 - ptr[1]) * (255 - ptr[3]) / 255) << 8) |
                           (((255 - ptr[2]) * (255 - ptr[3]) / 255));
		       ptr += 4;
		       ptr2++;
		    }
	       }
	  }
     }
   /* We handle then RGB with 3 components */
   else if (cinfo.output_components == 3)
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
   /* We finally handle RGB with 1 component */
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
evas_image_load_file_data_jpeg_alpha_internal(Image_Entry *ie, FILE *f)
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
   ie->w = w = cinfo.output_width;
   ie->h = h = cinfo.output_height;
/* end head decoding */
/* data decoding */
   if (cinfo.rec_outbuf_height > 16)
     {
	jpeg_destroy_decompress(&cinfo);
	return 0;
     }
   data = alloca(w * 16 * 3);
   if (!ie->flags.loaded)
     {
	jpeg_destroy_decompress(&cinfo);
	return 0;
     }
   ptr2 = evas_cache_image_pixels(ie);
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

static int
evas_image_load_file_head_jpeg(Image_Entry *ie, const char *file, const char *key)
{
   int val;
   FILE *f;

   if ((!file)) return 0;
   f = fopen(file, "rb");
   if (!f) return 0;
   val = evas_image_load_file_head_jpeg_internal(ie, f);
   fclose(f);
   return val;
   key = 0;
}

static int
evas_image_load_file_data_jpeg(Image_Entry *ie, const char *file, const char *key)
{
   int val;
   FILE *f;

   if ((!file)) return 0;
   f = fopen(file, "rb");
   if (!f) return 0;
   val = evas_image_load_file_data_jpeg_internal(ie, f);
   fclose(f);
   return val;
   key = 0;
}

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_jpeg_func);
   return 1;
}

static void
module_close(Evas_Module *em)
{
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "jpeg",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, jpeg);

#ifndef EVAS_STATIC_BUILD_JPEG
EVAS_EINA_MODULE_DEFINE(image_loader, jpeg);
#endif


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include <setjmp.h>
#include <jpeglib.h>

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

static Eina_Bool evas_image_load_file_head_jpeg_internal(Image_Entry *ie,
                                                         void *map,
                                                         size_t len,
                                                         int *error) EINA_ARG_NONNULL(1, 2, 3);
static Eina_Bool evas_image_load_file_data_jpeg_internal(Image_Entry *ie,
                                                         void *map,
                                                         size_t len,
                                                         int *error) EINA_ARG_NONNULL(1, 2, 3);
#if 0 /* not used at the moment */
static int evas_image_load_file_data_jpeg_alpha_internal(Image_Entry *ie, FILE *f) EINA_ARG_NONNULL(1, 2);
#endif

static Eina_Bool evas_image_load_file_head_jpeg(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);
static Eina_Bool evas_image_load_file_data_jpeg(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);

static Evas_Image_Load_Func evas_image_load_jpeg_func =
{
  EINA_TRUE,
  evas_image_load_file_head_jpeg,
  evas_image_load_file_data_jpeg,
  NULL
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
_JPEGErrorHandler(j_common_ptr cinfo __UNUSED__)
{
/*    emptr errmgr; */

/*    errmgr = (emptr) cinfo->err; */
   /*   cinfo->err->output_message(cinfo);*/
   /*   longjmp(errmgr->setjmp_buffer, 1);*/
   return;
}

static void
_JPEGErrorHandler2(j_common_ptr cinfo __UNUSED__, int msg_level __UNUSED__)
{
/*    emptr errmgr; */

/*    errmgr = (emptr) cinfo->err; */
   /*   cinfo->err->output_message(cinfo);*/
   /*   longjmp(errmgr->setjmp_buffer, 1);*/
   return;
}

struct jpeg_membuf_src
{
   struct jpeg_source_mgr pub;

   const unsigned char    *buf;
   size_t                  len;
   struct jpeg_membuf_src *self;
};

static void
_evas_jpeg_membuf_src_init(j_decompress_ptr cinfo __UNUSED__)
{
}

static boolean
_evas_jpeg_membuf_src_fill(j_decompress_ptr cinfo)
{
   static const JOCTET jpeg_eoi[2] = { 0xFF, JPEG_EOI };
   struct jpeg_membuf_src *src = (struct jpeg_membuf_src *)cinfo->src;

   src->pub.bytes_in_buffer = sizeof(jpeg_eoi);
   src->pub.next_input_byte = jpeg_eoi;

   return TRUE;
}

static void
_evas_jpeg_membuf_src_skip(j_decompress_ptr cinfo,
                           long              num_bytes)
{
   struct jpeg_membuf_src *src = (struct jpeg_membuf_src *)cinfo->src;

   if ((((long)src->pub.bytes_in_buffer - (long)src->len) > num_bytes) ||
       ((long)src->pub.bytes_in_buffer < num_bytes))
     {
        (*(cinfo)->err->error_exit) ((j_common_ptr) (cinfo));
        return;
     }
   src->pub.bytes_in_buffer -= num_bytes;
   src->pub.next_input_byte += num_bytes;
}

static void
_evas_jpeg_membuf_src_term(j_decompress_ptr cinfo)
{
   struct jpeg_membuf_src *src = (struct jpeg_membuf_src *)cinfo->src;
   if (!src) return;
   free(src);
   cinfo->src = NULL;
}

static int
_evas_jpeg_membuf_src(j_decompress_ptr cinfo,
                      void *map, size_t length)
{
   struct jpeg_membuf_src *src;

   src = calloc(1, sizeof(*src));
   if (!src)
      return -1;

   src->self = src;

   cinfo->src = &src->pub;
   src->buf = map;
   src->len = length;
   src->pub.init_source = _evas_jpeg_membuf_src_init;
   src->pub.fill_input_buffer = _evas_jpeg_membuf_src_fill;
   src->pub.skip_input_data = _evas_jpeg_membuf_src_skip;
   src->pub.resync_to_restart = jpeg_resync_to_restart;
   src->pub.term_source = _evas_jpeg_membuf_src_term;
   src->pub.bytes_in_buffer = src->len;
   src->pub.next_input_byte = src->buf;

   return 0;
}

/*! Magic number for EXIF header & App1*/
static const unsigned char ExifHeader[] = {0x45, 0x78, 0x69, 0x66, 0x00, 0x00};
static const unsigned char App1[] = {0xff, 0xe1};
typedef enum {
     EXIF_BYTE_ALIGN_II,
     EXIF_BYTE_ALIGN_MM
} ExifByteAlign;

static int
_get_orientation(void *map, size_t length)
{
   char *buf;
   char orientation[2];
   ExifByteAlign byte_align;
   unsigned int num_directory = 0;
   unsigned int i, j;
   int direction;

   /* open file and get 22 byte frome file */
   if (!map) return 0;
   /* 1. read 22byte */
   if (length < 22) return 0;
   buf = (char *)map;

   /* 2. check 2,3 bypte with APP1(0xFFE1) */
   if (memcmp(buf + 2, App1, sizeof (App1))) return 0;

   /* 3. check 6~11bype with Exif Header (0x45786966 0000) */
   if (memcmp(buf + 6, ExifHeader, sizeof (ExifHeader))) return 0;

   /* 4. get 12&13 byte  get info of "II(0x4949)" or "MM(0x4d4d)" */
   /* 5. get [20]&[21] get directory entry # */
   if (!strncmp(buf + 12, "MM", 2))
     {
        byte_align = EXIF_BYTE_ALIGN_MM;
        num_directory = ((*(buf + 20) << 8) + *(buf + 21));
        orientation[0] = 0x01;
        orientation[1] = 0x12;
     }
   else if (!strncmp(buf + 12, "II", 2))
     {
        byte_align = EXIF_BYTE_ALIGN_II;
        num_directory = ((*(buf + 21) << 8) + *(buf + 20));
        orientation[0] = 0x12;
        orientation[1] = 0x01;
     }
   else return 0;

   buf = map + 22;

   if (length < (12 * num_directory + 22)) return 0;

   j = 0;

   for (i = 0; i < num_directory; i++ )
     {
        if (!strncmp(buf + j, orientation, 2))
          {
             /*get orientation tag */
             if (byte_align == EXIF_BYTE_ALIGN_MM)
               direction = *(buf+ j + 11);
             else direction = *(buf+ j + 8);
             switch (direction)
               {
                case 3:
                case 4:
                   return 180;
                case 6:
                case 7:
                   return 90;
                case 5:
                case 8:
                   return 270;
                default:
                   return 0;
               }
          }
        else
          j = j + 12;
     }
   return 0;
}

static Eina_Bool
evas_image_load_file_head_jpeg_internal(Image_Entry *ie,
                                        void *map, size_t length,
                                        int *error)
{
   unsigned int w, h, scalew, scaleh;
   struct jpeg_decompress_struct cinfo;
   struct _JPEG_error_mgr jerr;

   /* for rotation decoding */
   int degree = 0;
   Eina_Bool change_wh = EINA_FALSE;
   unsigned int load_opts_w = 0, load_opts_h = 0;

   cinfo.err = jpeg_std_error(&(jerr.pub));
   jerr.pub.error_exit = _JPEGFatalErrorHandler;
   jerr.pub.emit_message = _JPEGErrorHandler2;
   jerr.pub.output_message = _JPEGErrorHandler;
   if (setjmp(jerr.setjmp_buffer))
     {
	jpeg_destroy_decompress(&cinfo);
        _evas_jpeg_membuf_src_term(&cinfo);
	if (cinfo.saw_JFIF_marker)
	  *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	else
	  *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
	return EINA_FALSE;
     }
   jpeg_create_decompress(&cinfo);

   if (_evas_jpeg_membuf_src(&cinfo, map, length))
     {
        jpeg_destroy_decompress(&cinfo);
        _evas_jpeg_membuf_src_term(&cinfo);
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return EINA_FALSE;
     }

   jpeg_read_header(&cinfo, TRUE);
   cinfo.do_fancy_upsampling = FALSE;
   cinfo.do_block_smoothing = FALSE;
   cinfo.dct_method = JDCT_IFAST;
   cinfo.dither_mode = JDITHER_ORDERED;
   jpeg_start_decompress(&cinfo);

   /* rotation decoding */
   if (ie->load_opts.orientation)
     {
        degree = _get_orientation(map, length);
        if (degree != 0)
          {
             ie->load_opts.degree = degree;
             ie->flags.rotated = EINA_TRUE;

             if (degree == 90 || degree == 270)
               change_wh = EINA_TRUE;
          }

     }

   /* head decoding */
   w = cinfo.output_width;
   h = cinfo.output_height;
   if ((w < 1) || (h < 1) || (w > IMG_MAX_SIZE) || (h > IMG_MAX_SIZE) ||
       (IMG_TOO_BIG(w, h)))
     {
        jpeg_destroy_decompress(&cinfo);
        _evas_jpeg_membuf_src_term(&cinfo);
	if (IMG_TOO_BIG(w, h))
	  *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	else
	  *error = EVAS_LOAD_ERROR_GENERIC;
	return EINA_FALSE;
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
   else if ((ie->load_opts.w > 0) && (ie->load_opts.h > 0))
     {
        unsigned int w2 = w, h2 = h;
        /* user set load_opts' w,h on the assumption
           that image already rotated according to it's orientation info */
        if (change_wh)
          {
             load_opts_w = ie->load_opts.w;
             load_opts_h = ie->load_opts.h;
             ie->load_opts.w = load_opts_h;
             ie->load_opts.h = load_opts_w;
          }

	if (ie->load_opts.w > 0)
	  {
	     w2 = ie->load_opts.w;
	     h2 = (ie->load_opts.w * h) / w;
	     if ((ie->load_opts.h > 0) && (h2 > ie->load_opts.h))
	       {
	          unsigned int w3;
		  h2 = ie->load_opts.h;
		  w3 = (ie->load_opts.h * w) / h;
		  if (w3 > w2)
		    w2 = w3;
	       }
	  }
	else if (ie->load_opts.h > 0)
	  {
	     h2 = ie->load_opts.h;
	     w2 = (ie->load_opts.h * w) / h;
	  }
	w = w2;
	h = h2;
        if (change_wh)
          {
             ie->load_opts.w = load_opts_w;
             ie->load_opts.h = load_opts_h;
          }
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
        _evas_jpeg_membuf_src_term(&cinfo);
	jpeg_create_decompress(&cinfo);

        if (_evas_jpeg_membuf_src(&cinfo, map, length))
          {
             jpeg_destroy_decompress(&cinfo);
             _evas_jpeg_membuf_src_term(&cinfo);
             *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
             return EINA_FALSE;
          }

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
   
   // be nice and clip region to image. if its totally outside, fail load
   if ((ie->load_opts.region.w > 0) && (ie->load_opts.region.h > 0))
     {
        unsigned int load_region_x = 0, load_region_y = 0;
        unsigned int load_region_w = 0, load_region_h = 0;
        if (ie->flags.rotated)
          {
             load_region_x = ie->load_opts.region.x;
             load_region_y = ie->load_opts.region.y;
             load_region_w = ie->load_opts.region.w;
             load_region_h = ie->load_opts.region.h;

             switch (degree)
               {
                case 90:
                   ie->load_opts.region.x = load_region_y;
                   ie->load_opts.region.y = h - (load_region_x + load_region_w);
                   ie->load_opts.region.w = load_region_h;
                   ie->load_opts.region.h = load_region_w;
                   break;
                case 180:
                   ie->load_opts.region.x = w - (load_region_x+ load_region_w);
                   ie->load_opts.region.y = h - (load_region_y + load_region_h);

                   break;
                case 270:
                   ie->load_opts.region.x = w - (load_region_y + load_region_h);
                   ie->load_opts.region.y = load_region_x;
                   ie->load_opts.region.w = load_region_h;
                   ie->load_opts.region.h = load_region_w;
                   break;
                default:
                   break;
               }

          }
        RECTS_CLIP_TO_RECT(ie->load_opts.region.x, ie->load_opts.region.y,
                           ie->load_opts.region.w, ie->load_opts.region.h,
                           0, 0, ie->w, ie->h);
        if ((ie->load_opts.region.w <= 0) || (ie->load_opts.region.h <= 0))
          {
             jpeg_destroy_decompress(&cinfo);
             _evas_jpeg_membuf_src_term(&cinfo);
	     *error = EVAS_LOAD_ERROR_GENERIC;
	     return EINA_FALSE;
          }
        ie->w = ie->load_opts.region.w;
        ie->h = ie->load_opts.region.h;
        if (ie->flags.rotated)
          {
             ie->load_opts.region.x = load_region_x;
             ie->load_opts.region.y = load_region_y;
             ie->load_opts.region.w = load_region_w;
             ie->load_opts.region.h = load_region_h;
          }
     }
/* end head decoding */

   if (change_wh)
     {
        unsigned int tmp;
        tmp = ie->w;
        ie->w = ie->h;
        ie->h = tmp;
     }
   jpeg_destroy_decompress(&cinfo);
   _evas_jpeg_membuf_src_term(&cinfo);
   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
}

/*
static double
get_time(void)
{
   struct timeval      timev;
   
   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}
*/

static Eina_Bool
evas_image_load_file_data_jpeg_internal(Image_Entry *ie,
                                        void *map, size_t size,
                                        int *error)
{
   unsigned int w, h;
   struct jpeg_decompress_struct cinfo;
   struct _JPEG_error_mgr jerr;
   DATA8 *ptr, *line[16], *data;
   DATA32 *ptr2, *ptr_rotate;
   unsigned int x, y, l, i, scans;
   int region = 0;

   /* rotation setting */
   unsigned int tmp;
   unsigned int load_region_x = 0, load_region_y = 0;
   unsigned int load_region_w = 0, load_region_h = 0;
   int degree = 0;
   Eina_Bool change_wh = EINA_FALSE;
   Eina_Bool line_done = EINA_FALSE;

   if (ie->flags.rotated)
     {
        degree = ie->load_opts.degree;
        if (degree == 90 || degree == 270)
          change_wh = EINA_TRUE;
     }

   cinfo.err = jpeg_std_error(&(jerr.pub));
   jerr.pub.error_exit = _JPEGFatalErrorHandler;
   jerr.pub.emit_message = _JPEGErrorHandler2;
   jerr.pub.output_message = _JPEGErrorHandler;
   if (setjmp(jerr.setjmp_buffer))
     {
	jpeg_destroy_decompress(&cinfo);
        _evas_jpeg_membuf_src_term(&cinfo);
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	return EINA_FALSE;
     }
   jpeg_create_decompress(&cinfo);

   if (_evas_jpeg_membuf_src(&cinfo, map, size))
     {
        jpeg_destroy_decompress(&cinfo);
        _evas_jpeg_membuf_src_term(&cinfo);
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return 0;
     }

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

   if (change_wh)
     {
        tmp = ie->w;
        ie->w = ie->h;
        ie->h = tmp;
     }

   if ((ie->load_opts.region.w > 0) && (ie->load_opts.region.h > 0))
     {
        region = 1;

        if (ie->flags.rotated)
          {
             load_region_x = ie->load_opts.region.x;
             load_region_y = ie->load_opts.region.y;
             load_region_w = ie->load_opts.region.w;
             load_region_h = ie->load_opts.region.h;

             switch (degree)
               {
                case 90:
                   ie->load_opts.region.x = load_region_y;
                   ie->load_opts.region.y = h - (load_region_x + load_region_w);
                   ie->load_opts.region.w = load_region_h;
                   ie->load_opts.region.h = load_region_w;
                   break;
                case 180:
                   ie->load_opts.region.x = w - (load_region_x+ load_region_w);
                   ie->load_opts.region.y = h - (load_region_y + load_region_h);

                   break;
                case 270:
                   ie->load_opts.region.x = w - (load_region_y + load_region_h);
                   ie->load_opts.region.y = load_region_x;
                   ie->load_opts.region.w = load_region_h;
                   ie->load_opts.region.h = load_region_w;
                   break;
                default:
                   break;
               }

          }
#ifdef BUILD_LOADER_JPEG_REGION
        cinfo.region_x = ie->load_opts.region.x;
        cinfo.region_y = ie->load_opts.region.y;
        cinfo.region_w = ie->load_opts.region.w;
        cinfo.region_h = ie->load_opts.region.h;
#endif
     }
   if ((!region) && ((w != ie->w) || (h != ie->h)))
     {
	// race condition, the file could have change from when we call header
	// this test will not solve the problem with region code.
	jpeg_destroy_decompress(&cinfo);
        _evas_jpeg_membuf_src_term(&cinfo);
	*error = EVAS_LOAD_ERROR_GENERIC;
	return EINA_FALSE;
     }
   if ((region) &&
       ((ie->w != ie->load_opts.region.w) || (ie->h != ie->load_opts.region.h)))
     {
        ie->w = ie->load_opts.region.w;
        ie->h = ie->load_opts.region.h;
     }

   if (!(((cinfo.out_color_space == JCS_RGB) &&
          ((cinfo.output_components == 3) || (cinfo.output_components == 1))) ||
         ((cinfo.out_color_space == JCS_CMYK) && (cinfo.output_components == 4))))
     {
	jpeg_destroy_decompress(&cinfo);
        _evas_jpeg_membuf_src_term(&cinfo);
	*error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
	return EINA_FALSE;
     }

/* end head decoding */
/* data decoding */
   if (cinfo.rec_outbuf_height > 16)
     {
	jpeg_destroy_decompress(&cinfo);
        _evas_jpeg_membuf_src_term(&cinfo);
	*error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
	return EINA_FALSE;
     }
   data = alloca(w * 16 * cinfo.output_components);
   evas_cache_image_surface_alloc(ie, ie->w, ie->h);
   if (ie->flags.loaded)
     {
	jpeg_destroy_decompress(&cinfo);
        _evas_jpeg_membuf_src_term(&cinfo);
	*error = EVAS_LOAD_ERROR_NONE;
        if (region && ie->flags.rotated)
          {
             ie->load_opts.region.x = load_region_x;
             ie->load_opts.region.y = load_region_y;
             ie->load_opts.region.w = load_region_w;
             ie->load_opts.region.h = load_region_h;
          }
	return EINA_TRUE;
     }
   if ((ie->flags.rotated) && change_wh)
     {
        ptr2 = malloc(ie->w * ie->h * sizeof(DATA32));
        ptr_rotate = ptr2;
     }
   else
   ptr2 = evas_cache_image_pixels(ie);

   if (!ptr2)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return EINA_FALSE;
     }

   /* We handle first CMYK (4 components) */
   if (cinfo.output_components == 4)
     {
        // FIXME: handle region
	for (i = 0; (int)i < cinfo.rec_outbuf_height; i++)
	  line[i] = data + (i * w * 4);
	for (l = 0; l < h; l += cinfo.rec_outbuf_height)
	  {
	     jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
	     scans = cinfo.rec_outbuf_height;
	     if ((h - l) < scans) scans = h - l;
	     ptr = data;
             if (!region)
               {
                  for (y = 0; y < scans; y++)
                    {
                       if (cinfo.saw_Adobe_marker)
                         {
                            for (x = 0; x < w; x++)
                              {
                                 /* According to libjpeg doc, Photoshop inverse the values of C, M, Y and K, */
                                 /* that is C is replaces by 255 - C, etc...*/
                                 /* See the comment below for the computation of RGB values from CMYK ones. */
                                 *ptr2 =
                                   (0xff000000) |
                                   ((ptr[0] * ptr[3] / 255) << 16) |
                                   ((ptr[1] * ptr[3] / 255) << 8) |
                                   ((ptr[2] * ptr[3] / 255));
                                 ptr += 4;
                                 ptr2++;
                              }
                         }
                       else
                         {
                            for (x = 0; x < w; x++)
                              {
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
             else
               {
                  // if line # > region last line, break
                  if (l >= (ie->load_opts.region.y + ie->load_opts.region.h))
                    {
                       line_done = EINA_TRUE;
                       /* if rotation flag is set , we have to rotate image */
                       goto done;
                       /*jpeg_destroy_decompress(&cinfo);
                         _evas_jpeg_membuf_src_term(&cinfo);
                        *error = EVAS_LOAD_ERROR_NONE;
                        return EINA_FALSE;*/
                    }
                  // els if scan block intersects region start or later
                  else if ((l + scans) > 
                           (ie->load_opts.region.y))
                    {
                       for (y = 0; y < scans; y++)
                         {
                            if (((y + l) >= ie->load_opts.region.y) &&
                                ((y + l) < (ie->load_opts.region.y + ie->load_opts.region.h)))
                              {
                                 ptr += ie->load_opts.region.x;
                                 if (cinfo.saw_Adobe_marker)
                                   {
                                      for (x = 0; x < ie->load_opts.region.w; x++)
                                        {
                                           /* According to libjpeg doc, Photoshop inverse the values of C, M, Y and K, */
                                           /* that is C is replaces by 255 - C, etc...*/
                                           /* See the comment below for the computation of RGB values from CMYK ones. */
                                           *ptr2 =
                                             (0xff000000) |
                                             ((ptr[0] * ptr[3] / 255) << 16) |
                                             ((ptr[1] * ptr[3] / 255) << 8) |
                                             ((ptr[2] * ptr[3] / 255));
                                           ptr += 4;
                                           ptr2++;
                                        }
                                   }
                                 else
                                   {
                                      for (x = 0; x < ie->load_opts.region.w; x++)
                                        {
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
                                 ptr += (4 * (w - (ie->load_opts.region.x + ie->load_opts.region.w)));
                              }
                            else
                              ptr += (4 * w);
                         }
                    }
               }
	  }
     }
   /* We handle then RGB with 3 components */
   else if (cinfo.output_components == 3)
     {
/*        
        double t;
        if (region)
          {
             // debug for now
             printf("R| %p %5ix%5i %s: %5i %5i %5ix%5i - ",
                    ie,
                    ie->w, ie->h,
                    ie->file,
                    ie->load_opts.region.x,
                    ie->load_opts.region.y,
                    ie->load_opts.region.w,
                    ie->load_opts.region.h);
          }
        t = get_time();
 */
        for (i = 0; (int)i < cinfo.rec_outbuf_height; i++)
	  line[i] = data + (i * w * 3);
	for (l = 0; l < h; l += cinfo.rec_outbuf_height)
	  {
	     jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
	     scans = cinfo.rec_outbuf_height;
	     if ((h - l) < scans) scans = h - l;
	     ptr = data;
             if (!region)
               {
                  for (y = 0; y < scans; y++)
                    {
                       for (x = 0; x < w; x++)
                         {
                            *ptr2 = ARGB_JOIN(0xff, ptr[0], ptr[1], ptr[2]);
                            ptr += 3;
                            ptr2++;
                         }
                    }
	       }
             else
               {
                  // if line # > region last line, break
                  // but not return immediately for rotation job
                  if (l >= (ie->load_opts.region.y + ie->load_opts.region.h))
                    {
                       line_done = EINA_TRUE;
                       /* if rotation flag is set , we have to rotate image */
                       goto done;
                    }
                  // else if scan block intersects region start or later
                  else if ((l + scans) > 
                           (ie->load_opts.region.y))
                    {
                       for (y = 0; y < scans; y++)
                         {
                            if (((y + l) >= ie->load_opts.region.y) &&
                                ((y + l) < (ie->load_opts.region.y + ie->load_opts.region.h)))
                              {
                                 ptr += (3 * ie->load_opts.region.x);
                                 for (x = 0; x < ie->load_opts.region.w; x++)
                                   {
                                      *ptr2 = ARGB_JOIN(0xff, ptr[0], ptr[1], ptr[2]);
                                      ptr += 3;
                                      ptr2++;
                                   }
                                 ptr += (3 * (w - (ie->load_opts.region.x + ie->load_opts.region.w)));
                              }
                            else
                              ptr += (3 * w);
                         }
                    }
               }
	  }
/*        
        t = get_time() - t;
        printf("%3.3f\n", t);
 */
     }
   /* We finally handle RGB with 1 component */
   else if (cinfo.output_components == 1)
     {
	for (i = 0; (int)i < cinfo.rec_outbuf_height; i++)
	  line[i] = data + (i * w);
	for (l = 0; l < h; l += cinfo.rec_outbuf_height)
	  {
	     jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
	     scans = cinfo.rec_outbuf_height;
	     if ((h - l) < scans) scans = h - l;
	     ptr = data;
             if (!region)
               {
                  for (y = 0; y < scans; y++)
                    {
                       for (x = 0; x < w; x++)
                         {
                            *ptr2 = ARGB_JOIN(0xff, ptr[0], ptr[0], ptr[0]);
                            ptr++;
                            ptr2++;
                         }
                    }
	       }
             else
               {
                  // if line # > region last line, break
                  if (l >= (ie->load_opts.region.y + ie->load_opts.region.h))
                    {
                       line_done = EINA_TRUE;
                       /* if rotation flag is set , we have to rotate image */
                       goto done;
                       /*jpeg_destroy_decompress(&cinfo);
                         _evas_jpeg_membuf_src_term(&cinfo);
                        *error = EVAS_LOAD_ERROR_NONE;
                        return EINA_TRUE;*/
                    }
                  // els if scan block intersects region start or later
                  else if ((l + scans) > 
                           (ie->load_opts.region.y))
                    {
                       for (y = 0; y < scans; y++)
                         {
                            if (((y + l) >= ie->load_opts.region.y) &&
                                ((y + l) < (ie->load_opts.region.y + ie->load_opts.region.h)))
                              {
                                 ptr += ie->load_opts.region.x;
                                 for (x = 0; x < ie->load_opts.region.w; x++)
                                   {
                                      *ptr2 = ARGB_JOIN(0xff, ptr[0], ptr[0], ptr[0]);
                                      ptr++;
                                      ptr2++;
                                   }
                                 ptr += w - (ie->load_opts.region.x + ie->load_opts.region.w);
                              }
                            else
                              ptr += w;
                         }
                    }
               }
	  }
     }
   /* if rotation operation need, rotate it */
done:

   if (ie->flags.rotated)
     {
        DATA32             *data1, *data2,  *to, *from;
        int                 x, y, w, h,  hw;

        if (change_wh)
          {
             tmp = ie->w;
             ie->w = ie->h;
             ie->h = tmp;
          }

        w = ie->w;
        h = ie->h;
        hw =w * h;

        data1 = evas_cache_image_pixels(ie);

        if (degree == 180)
          {
             DATA32 tmp;

             data2 = data1 + (h * w) -1;
             for (x = (w * h) / 2; --x >= 0;)
               {
                  tmp = *data1;
                  *data1 = *data2;
                  *data2 = tmp;
                  data1++;
                  data2--;
               }
          }
        else
          {
             data2 = ptr_rotate;

             if (degree == 90)
               {
                  to = data1 + w - 1;
                  hw = -hw - 1;
               }

             else if (degree == 270)
               {
                  to = data1 + hw - w;
                  w = -w;
                  hw = hw + 1;
               }

             from = data2;
             for (x = ie->w; --x >= 0;)
               {
                  for (y =ie->h; --y >= 0;)
                    {
                       *to = *from;
                       from++;
                       to += w;
                    }
                  to += hw;
               }
             if (ptr_rotate)
               {
                  free(ptr_rotate);
                  ptr_rotate = NULL;
               }

          }
        if (region )
          {
             ie->load_opts.region.x = load_region_x;
             ie->load_opts.region.y = load_region_y;
             ie->load_opts.region.w = load_region_w;
             ie->load_opts.region.h = load_region_h;
          }
     }
   if (line_done)
     {
        jpeg_destroy_decompress(&cinfo);
        _evas_jpeg_membuf_src_term(&cinfo);
        *error = EVAS_LOAD_ERROR_NONE;
        return EINA_FALSE;
     }
   /* end data decoding */
   jpeg_finish_decompress(&cinfo);
   jpeg_destroy_decompress(&cinfo);
   _evas_jpeg_membuf_src_term(&cinfo);
   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
}

#if 0 /* not used at the moment */
static Eina_Bool
evas_image_load_file_data_jpeg_alpha_internal(Image_Entry *ie, FILE *f, int *error)
{
   int w, h;
   struct jpeg_decompress_struct cinfo;
   struct _JPEG_error_mgr jerr;
   DATA8 *ptr, *line[16], *data;
   DATA32 *ptr2;
   int x, y, l, i, scans, prevy;

   if (!f)
     {
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
	return EINA_FALSE;
     }
   cinfo.err = jpeg_std_error(&(jerr.pub));
   jerr.pub.error_exit = _JPEGFatalErrorHandler;
   jerr.pub.emit_message = _JPEGErrorHandler2;
   jerr.pub.output_message = _JPEGErrorHandler;
   if (setjmp(jerr.setjmp_buffer))
     {
	jpeg_destroy_decompress(&cinfo);
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	return EINA_FALSE;
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
	*error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
	return EINA_FALSE;;
     }
   data = alloca(w * 16 * 3);
   if (!ie->flags.loaded)
     {
	jpeg_destroy_decompress(&cinfo);
	*error = EVAS_LOAD_ERROR_NONE;
	return EINA_TRUE;
     }
   ptr2 = evas_cache_image_pixels(ie);
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
   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
}
#endif

static Eina_Bool
evas_image_load_file_head_jpeg(Image_Entry *ie,
                               const char *file, const char *key __UNUSED__,
                               int *error)
{
   Eina_File *f;
   void *map;
   Eina_Bool val = EINA_FALSE;

   f = eina_file_open(file, EINA_FALSE);
   if (!f)
     {
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
	return EINA_FALSE;
     }
   map = eina_file_map_all(f, EINA_FILE_WILLNEED);
   if (!map)
     {
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
        goto on_error;
     }

   val = evas_image_load_file_head_jpeg_internal(ie,
                                                 map, eina_file_size_get(f),
                                                 error);

   eina_file_map_free(f, map);

 on_error:
   eina_file_close(f);
   return val;
}

static Eina_Bool
evas_image_load_file_data_jpeg(Image_Entry *ie,
                               const char *file, const char *key __UNUSED__,
                               int *error)
{
   Eina_File *f;
   void *map;
   Eina_Bool val = EINA_FALSE;

   f = eina_file_open(file, EINA_FALSE);
   if (!f)
     {
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
	return EINA_FALSE;
     }
   map = eina_file_map_all(f, EINA_FILE_WILLNEED);
   if (!map)
     {
        *error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
        goto on_error;
     }

   val = evas_image_load_file_data_jpeg_internal(ie,
                                                 map, eina_file_size_get(f),
                                                 error);

   eina_file_map_free(f, map);

 on_error:
   eina_file_close(f);
   return val;
}

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_jpeg_func);
   return 1;
}

static void
module_close(Evas_Module *em __UNUSED__)
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


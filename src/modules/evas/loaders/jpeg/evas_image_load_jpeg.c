#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#ifdef _WIN32
# define XMD_H /* This prevents libjpeg to redefine INT32 */
#endif

#include <setjmp.h>
#include <jpeglib.h>

#include "evas_common_private.h"
#include "evas_private.h"

typedef struct _JPEG_error_mgr *emptr;
struct _JPEG_error_mgr
{
   struct     jpeg_error_mgr pub;
   jmp_buf    setjmp_buffer;
};

typedef struct _Evas_Loader_Internal Evas_Loader_Internal;
struct _Evas_Loader_Internal
{
  Eina_File *f;
  Evas_Image_Load_Opts *opts;
};


static void _JPEGFatalErrorHandler(j_common_ptr cinfo);
static void _JPEGErrorHandler(j_common_ptr cinfo);
static void _JPEGErrorHandler2(j_common_ptr cinfo, int msg_level);
static Eina_Bool _get_next_app0(unsigned char *map, size_t fsize, size_t *position);
static Eina_Bool _get_orientation_app1(unsigned char *map, size_t fsize, size_t *position,
                                       int *orientation, Eina_Bool *flipped);
static int _get_orientation(void *map, size_t length, Eina_Bool *flipped);

#if 0 /* not used at the moment */
static int evas_image_load_file_data_jpeg_alpha_internal(Image_Entry *ie, FILE *f) EINA_ARG_NONNULL(1, 2);
#endif

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
_JPEGErrorHandler(j_common_ptr cinfo EINA_UNUSED)
{
/*    emptr errmgr; */

/*    errmgr = (emptr) cinfo->err; */
   /*   cinfo->err->output_message(cinfo);*/
   /*   longjmp(errmgr->setjmp_buffer, 1);*/
   return;
}

static void
_JPEGErrorHandler2(j_common_ptr cinfo EINA_UNUSED, int msg_level EINA_UNUSED)
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
_evas_jpeg_membuf_src_init(j_decompress_ptr cinfo EINA_UNUSED)
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

/*! Magic number for EXIF header, App0, App1*/
static const unsigned char ExifHeader[] = {0x45, 0x78, 0x69, 0x66, 0x00, 0x00};
static const unsigned char JfifHeader[] = {0x4A, 0x46, 0x49, 0x46, 0x00};
static const unsigned char JfxxHeader[] = {0x4A, 0x46, 0x58, 0x58, 0x00};
static const unsigned char App0[] = {0xff, 0xe0};
static const unsigned char App1[] = {0xff, 0xe1};
static const unsigned char II[] = {0x49, 0x49};
static const unsigned char MM[] = {0x4d, 0x4d};
typedef enum {
     EXIF_BYTE_ALIGN_II,
     EXIF_BYTE_ALIGN_MM
} ExifByteAlign;

static Eina_Bool
_get_next_app0(unsigned char *map, size_t fsize, size_t *position)
{
   unsigned short length = 0;
   unsigned int w = 0, h = 0;
   unsigned int format = 0;
   unsigned int data_size = 0;
   unsigned char *app0_head, *p;

   /* header_mark:2, length:2, identifier:5 version:2, unit:1, den=4 thum=2 */
   if ((*position + 16) >= fsize) return EINA_FALSE;
   app0_head  = map + *position;

   /* p is appn's start pointer excluding app0 marker */
   p = app0_head + 2;

   length = ((*p << 8) + *(p + 1));

   /* JFIF segment format */
   if (!memcmp(p + 2, JfifHeader, sizeof (JfifHeader)))
     {
        format = 3;
        w = *(p + 14);
        h = *(p + 15);
     }
   else if (!memcmp(p + 2, JfxxHeader, sizeof (JfxxHeader)))
     {
        if (*(p + 7) == 0x11)
          format = 1;
        else
          format = 3;
        w = *(p + 8);
        h = *(p + 9);
     }

     data_size = format * w * h;

     if ((*position + 2+ length + data_size) > fsize)
       return EINA_FALSE;

     *position = *position + 2 + length + data_size;

     return EINA_TRUE;
}

/* If app1 data is abnormal, returns EINA_FALSE.
   If app1 data is normal, returns EINA_TRUE.
   If app1 data is normal but not orientation data, orientation value is -1.
 */

static Eina_Bool
_get_orientation_app1(unsigned char *map, size_t fsize, size_t *position,
                      int *orientation_res, Eina_Bool *flipped)
{
   unsigned char *app1_head, *buf;
   unsigned char orientation[2];
   ExifByteAlign byte_align;
   unsigned int num_directory = 0;
   unsigned int i, j;
   int direction;
   unsigned int data_size = 0;

   /* app1 mark:2, data_size:2, exif:6 tiff:8 */
   if ((*position + 18) >= fsize) return EINA_FALSE;
   app1_head  = map + *position;
   buf = app1_head;

   data_size = ((*(buf + 2) << 8) + *(buf + 3));
   if ((*position + 2 + data_size) > fsize) return EINA_FALSE;

   if (memcmp(buf + 4, ExifHeader, sizeof (ExifHeader)))
     {
        *position = *position + 2 + data_size;
        *orientation_res = -1;
        return EINA_TRUE;
     }

   /* 2. get 10&11 byte  get info of "II(0x4949)" or "MM(0x4d4d)" */
   /* 3. get [18]&[19] get directory entry # */
   if (!memcmp(buf + 10, MM, sizeof (MM)))
     {
        byte_align = EXIF_BYTE_ALIGN_MM;
        num_directory = ((*(buf + 18) << 8) + *(buf + 19));
        orientation[0] = 0x01;
        orientation[1] = 0x12;
     }
   else if (!memcmp(buf + 10, II, sizeof (II)))
     {
        byte_align = EXIF_BYTE_ALIGN_II;
        num_directory = ((*(buf + 19) << 8) + *(buf + 18));
        orientation[0] = 0x12;
        orientation[1] = 0x01;
     }
   else return EINA_FALSE;

   /* check num_directory data */
   if ((*position + (12 * num_directory + 20)) > fsize) return EINA_FALSE;

   buf = app1_head + 20;

   j = 0;

   for (i = 0; i < num_directory; i++ )
     {
        if (!memcmp(buf + j, orientation, 2))
          {
             /*get orientation tag */
             if (byte_align == EXIF_BYTE_ALIGN_MM)
               direction = *(buf+ j + 9);
             else direction = *(buf+ j + 8);
             switch (direction)
               {
                case 3:
                  *orientation_res = 180;
                  *flipped = EINA_FALSE;
                  return EINA_TRUE;
                case 4:
                  *orientation_res = 180;
                  *flipped = EINA_TRUE;
                  return EINA_TRUE;
                case 6:
                  *orientation_res = 90;
                  *flipped = EINA_FALSE;
                  return EINA_TRUE;
                case 7:
                  *orientation_res = 90;
                  *flipped = EINA_TRUE;
                  return EINA_TRUE;
                case 5:
                  *orientation_res = 270;
                  *flipped = EINA_TRUE;
                  return EINA_TRUE;
                case 8:
                  *orientation_res = 270;
                  *flipped = EINA_FALSE;
                  return EINA_TRUE;
                case 2:
                  *orientation_res = 0;
                  *flipped = EINA_TRUE;
                  return EINA_TRUE;
                default:
                  *orientation_res = 0;
                  *flipped = EINA_FALSE;
                  return EINA_TRUE;
               }
          }
        else
          j = j + 12;
     }
   return EINA_FALSE;
}

static int
_get_orientation(void *map, size_t length, Eina_Bool *flipped)
{
   unsigned char *buf;
   size_t position = 0;
   int orientation = -1;
   Eina_Bool res = EINA_FALSE;

   *flipped = EINA_FALSE;

   /* open file and get 22 byte frome file */
   if (!map) return 0;
   /* 1. read 22byte */
   if (length < 22) return 0;
   buf = (unsigned char *)map;

   position = 2;
   /* 2. check 2,3 bypte with APP0(0xFFE0) or APP1(0xFFE1) */
   while((length - position) > 0)
     {
        if (!memcmp(buf + position, App0, sizeof (App0)))
          {
             res = _get_next_app0(map, length, &position);
             if (!res) break;
          }
        else if (!memcmp(buf + position, App1, sizeof (App1)))
          {
             res = _get_orientation_app1(map, length, &position, &orientation, flipped);
             if (!res) break;
             if (orientation != -1) return orientation;
          }
        else break;
     }
   return 0;
}

static void
_rotate_region(unsigned int *r_x, unsigned int *r_y, unsigned int *r_w, unsigned int *r_h,
               unsigned int x, unsigned int y, unsigned int w, unsigned int h,
               unsigned int output_w, unsigned int output_h,
               int degree, Eina_Bool flipped)
{
   switch (degree)
     {
      case 90:
        if (flipped)
          {
             *r_x = output_w - (y + h);
             *r_y = output_h - (x + w);
             *r_w = h;
             *r_h = w;
          }
        else
          {
             *r_x = y;
             *r_y = output_h - (x + y);
             *r_w = h;
             *r_h = w;
          }
        break;
      case 180:
        if (flipped)
          {
             *r_y = output_h - (y + h);
          }
        else
          {
             *r_x = output_w - (x + w);
             *r_y = output_h - (y + h);
          }
        break;
      case 270:
        if (flipped)
          {
             *r_x = y;
             *r_y = x;
             *r_w = h;
             *r_h = w;
          }
        else
          {
             *r_x = output_w - (y + h);
             *r_y = x;
             *r_w = h;
             *r_h = w;
          }
        break;
      default:
        if (flipped)
           *r_x = output_w - (x + w);
        break;
     }
}

static Eina_Bool
evas_image_load_file_head_jpeg_internal(unsigned int *w, unsigned int *h,
                                        unsigned char *scale,
                                        unsigned char *rotated,
                                        Eina_Bool *flipped,
                                        Evas_Image_Load_Opts *opts,
                                        void *map, size_t length,
                                        int *error)
{
   unsigned int scalew, scaleh;
   struct jpeg_decompress_struct cinfo;
   struct _JPEG_error_mgr jerr;

   /* for rotation decoding */
   int degree = 0;
   Eina_Bool change_wh = EINA_FALSE;
   unsigned int load_opts_w = 0, load_opts_h = 0;

   memset(&cinfo, 0, sizeof(cinfo));
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
   cinfo.dct_method = JDCT_ISLOW; // JDCT_FLOAT JDCT_IFAST(quality loss)
   cinfo.dither_mode = JDITHER_ORDERED;
   cinfo.buffered_image = TRUE; // buffered mode in case jpg is progressive
   jpeg_start_decompress(&cinfo);

   /* rotation decoding */
   if (opts->orientation)
     {
        degree = _get_orientation(map, length, flipped);
        if (degree != 0 || *flipped)
          {
             opts->degree = degree;
	     *rotated = EINA_TRUE;

             if (degree == 90 || degree == 270)
               change_wh = EINA_TRUE;
          }

     }

   /* head decoding */
   *w = cinfo.output_width;
   *h = cinfo.output_height;
   if ((*w < 1) || (*h < 1) || (*w > IMG_MAX_SIZE) || (*h > IMG_MAX_SIZE) ||
       (IMG_TOO_BIG(*w, *h)))
     {
        jpeg_destroy_decompress(&cinfo);
        _evas_jpeg_membuf_src_term(&cinfo);
	if (IMG_TOO_BIG(*w, *h))
	  *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	else
	  *error = EVAS_LOAD_ERROR_GENERIC;
	return EINA_FALSE;
     }
   if (opts->scale_down_by > 1)
     {
	*w /= opts->scale_down_by;
	*h /= opts->scale_down_by;
     }
   else if (opts->dpi > 0.0)
     {
	*w = (*w * opts->dpi) / 90.0;
	*h = (*h * opts->dpi) / 90.0;
     }
   else if ((opts->w > 0) && (opts->h > 0))
     {
        unsigned int w2 = *w, h2 = *h;
        /* user set load_opts' w,h on the assumption
           that image already rotated according to it's orientation info */
        if (change_wh)
          {
             load_opts_w = opts->w;
             load_opts_h = opts->h;
             opts->w = load_opts_h;
             opts->h = load_opts_w;
          }

	if (opts->w > 0)
	  {
	     w2 = opts->w;
	     h2 = (opts->w * *h) / *w;
	     if ((opts->h > 0) && (h2 > opts->h))
	       {
	          unsigned int w3;
		  h2 = opts->h;
		  w3 = (opts->h * *w) / *h;
		  if (w3 > w2)
		    w2 = w3;
	       }
	  }
	else if (opts->h > 0)
	  {
	     h2 = opts->h;
	     w2 = (opts->h * *w) / *h;
	  }
	*w = w2;
	*h = h2;
        if (change_wh)
          {
             opts->w = load_opts_w;
             opts->h = load_opts_h;
          }
     }
   if (*w < 1) *w = 1;
   if (*h < 1) *h = 1;

   if ((*w != cinfo.output_width) || (*h != cinfo.output_height))
     {
	scalew = cinfo.output_width / *w;
	scaleh = cinfo.output_height / *h;

	*scale = scalew;
	if (scaleh < scalew) *scale = scaleh;

	if      (*scale > 8) *scale = 8;
	else if (*scale < 1) *scale = 1;

	if      (*scale == 3) *scale = 2;
	else if (*scale == 5) *scale = 4;
	else if (*scale == 6) *scale = 4;
	else if (*scale == 7) *scale = 4;
     }

   if (*scale > 1)
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
	cinfo.scale_denom = *scale;
        cinfo.buffered_image = TRUE; // buffered mode in case jpg is progressive
	jpeg_calc_output_dimensions(&(cinfo));
	jpeg_start_decompress(&cinfo);
     }

   *w = cinfo.output_width;
   *h = cinfo.output_height;

   // be nice and clip region to image. if its totally outside, fail load
   if ((opts->region.w > 0) && (opts->region.h > 0))
     {
        unsigned int load_region_x = 0, load_region_y = 0;
        unsigned int load_region_w = 0, load_region_h = 0;
        if (*rotated)
          {
             load_region_x = opts->region.x;
             load_region_y = opts->region.y;
             load_region_w = opts->region.w;
             load_region_h = opts->region.h;

             _rotate_region(&opts->region.x, &opts->region.y, &opts->region.w, &opts->region.h,
                            load_region_x, load_region_y, load_region_w, load_region_h,
                            *w, *h, degree, *flipped);
          }
        RECTS_CLIP_TO_RECT(opts->region.x, opts->region.y,
                           opts->region.w, opts->region.h,
                           0, 0, *w, *h);
        if ((opts->region.w <= 0) || (opts->region.h <= 0))
          {
             jpeg_destroy_decompress(&cinfo);
             _evas_jpeg_membuf_src_term(&cinfo);
	     *error = EVAS_LOAD_ERROR_GENERIC;
	     return EINA_FALSE;
          }
        *w = opts->region.w;
        *h = opts->region.h;
        if (*rotated)
          {
             opts->region.x = load_region_x;
             opts->region.y = load_region_y;
             opts->region.w = load_region_w;
             opts->region.h = load_region_h;
          }
     }
/* end head decoding */

   if (change_wh)
     {
        unsigned int tmp;
        tmp = *w;
        *w = *h;
        *h = tmp;
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

static void
_rotate_180(DATA32 *data, int w, int h)
{
   DATA32 *p1, *p2;
   DATA32 pt;
   int x;

   p1 = data;
   p2 = data + (h * w) - 1;
   for (x = (w * h) / 2; --x >= 0;)
     {
        pt = *p1;
        *p1 = *p2;
        *p2 = pt;
        p1++;
        p2--;
     }
}

static void
_flip_horizontal(DATA32 *data, int w, int h)
{
   DATA32 *p1, *p2;
   DATA32 pt;
   int x, y;

   for (y = 0; y < h; y++)
     {
        p1 = data + (y * w);
        p2 = data + ((y + 1) * w) - 1;
        for (x = 0; x < (w >> 1); x++)
          {
             pt = *p1;
             *p1 = *p2;
             *p2 = pt;
             p1++;
             p2--;
          }
     }
}

static void
_flip_vertical(DATA32 *data, int w, int h)
{
   DATA32 *p1, *p2;
   DATA32 pt;
   int x, y;

   for (y = 0; y < (h >> 1); y++)
     {
        p1 = data + (y * w);
        p2 = data + ((h - 1 - y) * w);
        for (x = 0; x < w; x++)
          {
             pt = *p1;
             *p1 = *p2;
             *p2 = pt;
             p1++;
             p2++;
          }
     }
}

static void
_rotate_change_wh(DATA32 *to, DATA32 *from,
                  int w, int h,
                  int dx, int dy)
{
   int x, y;

   for (x = h; --x >= 0;)
     {
        for (y = w; --y >= 0;)
          {
             *to = *from;
             from++;
             to += dy;
          }
        to += dx;
     }
}

static Eina_Bool
evas_image_load_file_data_jpeg_internal(Evas_Image_Load_Opts *opts,
                                        Evas_Image_Property *prop,
                                        void *pixels,
                                        void *map, size_t size,
                                        int *error)
{
   unsigned int w, h;
   struct jpeg_decompress_struct cinfo;
   struct _JPEG_error_mgr jerr;
   DATA8 *ptr, *line[16], *data;
   DATA32 *ptr2, *ptr_rotate = NULL;
   unsigned int x, y, l, i, scans;
   int region = 0;
   /* rotation setting */
   unsigned int ie_w = 0, ie_h = 0;
   unsigned int load_region_x = 0, load_region_y = 0;
   unsigned int load_region_w = 0, load_region_h = 0;
   volatile int degree = 0;
   volatile Eina_Bool change_wh = EINA_FALSE;
   Eina_Bool line_done = EINA_FALSE;

   memset(&cinfo, 0, sizeof(cinfo));
   if (prop->rotated)
     {
        degree = opts->degree;
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
   cinfo.dct_method = JDCT_ISLOW; // JDCT_FLOAT JDCT_IFAST(quality loss)
   cinfo.dither_mode = JDITHER_ORDERED;

   if (prop->scale > 1)
     {
	cinfo.scale_num = 1;
	cinfo.scale_denom = prop->scale;
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
      default:
        cinfo.out_color_space = JCS_RGB;
        break;
     }

/* head decoding */
   jpeg_calc_output_dimensions(&(cinfo));
   jpeg_start_decompress(&cinfo);

   w = cinfo.output_width;
   h = cinfo.output_height;

   if (change_wh)
     {
        ie_w = prop->h;
        ie_h = prop->w;
     }
   else
     {
        ie_w = prop->w;
        ie_h = prop->h;
     }

   if ((opts->region.w > 0) && (opts->region.h > 0))
     {
        region = 1;

        if (prop->rotated)
          {
             load_region_x = opts->region.x;
             load_region_y = opts->region.y;
             load_region_w = opts->region.w;
             load_region_h = opts->region.h;

             _rotate_region(&opts->region.x, &opts->region.y, &opts->region.w, &opts->region.h,
                            load_region_x, load_region_y, load_region_w, load_region_h,
                            w, h, degree, prop->flipped);
          }
#ifdef BUILD_LOADER_JPEG_REGION
        cinfo.region_x = opts->region.x;
        cinfo.region_y = opts->region.y;
        cinfo.region_w = opts->region.w;
        cinfo.region_h = opts->region.h;
#endif
     }
   if ((!region) && ((w != ie_w) || (h != ie_h)))
     {
        // race condition, the file could have change from when we call header
        // this test will not solve the problem with region code.
        jpeg_destroy_decompress(&cinfo);
        _evas_jpeg_membuf_src_term(&cinfo);
        *error = EVAS_LOAD_ERROR_GENERIC;
        return EINA_FALSE;
     }
   if ((region) &&
       ((ie_w != opts->region.w) || (ie_h != opts->region.h)))
     {
        jpeg_destroy_decompress(&cinfo);
        _evas_jpeg_membuf_src_term(&cinfo);
        *error = EVAS_LOAD_ERROR_GENERIC;
        return EINA_FALSE;
        /* ie_w = opts->region.w; */
        /* ie_h = opts->region.h; */
        /* if (change_wh) */
        /*   { */
        /*      ie->w = ie_h; */
        /*      ie->h = ie_w; */
        /*   } */
        /* else */
        /*   { */
        /*      ie->w = ie_w; */
        /*      ie->h = ie_h; */
        /*   } */
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
   if ((prop->rotated) && change_wh)
     {
        ptr2 = malloc(w * h * sizeof(DATA32));
        ptr_rotate = ptr2;
     }
   else
     ptr2 = pixels;

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
                  if (l >= (opts->region.y + opts->region.h))
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
                           (opts->region.y))
                    {
                       for (y = 0; y < scans; y++)
                         {
                            if (((y + l) >= opts->region.y) &&
                                ((y + l) < (opts->region.y + opts->region.h)))
                              {
                                 ptr += opts->region.x;
                                 if (cinfo.saw_Adobe_marker)
                                   {
                                      for (x = 0; x < opts->region.w; x++)
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
                                      for (x = 0; x < opts->region.w; x++)
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
                                 ptr += (4 * (w - (opts->region.x + opts->region.w)));
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
                    opts->region.x,
                    opts->region.y,
                    opts->region.w,
                    opts->region.h);
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
                  if (l >= (opts->region.y + opts->region.h))
                    {
                       line_done = EINA_TRUE;
                       /* if rotation flag is set , we have to rotate image */
                       goto done;
                    }
                  // else if scan block intersects region start or later
                  else if ((l + scans) >
                           (opts->region.y))
                    {
                       for (y = 0; y < scans; y++)
                         {
                            if (((y + l) >= opts->region.y) &&
                                ((y + l) < (opts->region.y + opts->region.h)))
                              {
                                 ptr += (3 * opts->region.x);
                                 for (x = 0; x < opts->region.w; x++)
                                   {
                                      *ptr2 = ARGB_JOIN(0xff, ptr[0], ptr[1], ptr[2]);
                                      ptr += 3;
                                      ptr2++;
                                   }
                                 ptr += (3 * (w - (opts->region.x + opts->region.w)));
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
                  if (l >= (opts->region.y + opts->region.h))
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
                           (opts->region.y))
                    {
                       for (y = 0; y < scans; y++)
                         {
                            if (((y + l) >= opts->region.y) &&
                                ((y + l) < (opts->region.y + opts->region.h)))
                              {
                                 ptr += opts->region.x;
                                 for (x = 0; x < opts->region.w; x++)
                                   {
                                      *ptr2 = ARGB_JOIN(0xff, ptr[0], ptr[0], ptr[0]);
                                      ptr++;
                                      ptr2++;
                                   }
                                 ptr += w - (opts->region.x + opts->region.w);
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

   if (prop->rotated)
     {
        DATA32 *to;
        int hw;

        hw = w * h;
        to = pixels;

        switch (degree)
          {
           case 90:
             if (prop->flipped)
               _rotate_change_wh(to + hw - 1, ptr_rotate, w, h, hw - 1, -h);
             else
               _rotate_change_wh(to + h - 1, ptr_rotate, w, h, -hw - 1, h);
             break;
           case 180:
             if (prop->flipped)
               _flip_vertical(to, w, h);
             else
               _rotate_180(to, w, h);
             break;
           case 270:
             if (prop->flipped)
               _rotate_change_wh(to, ptr_rotate, w, h, -hw + 1, h);
             else
               _rotate_change_wh(to + hw - h, ptr_rotate, w, h, hw + 1, -h);
             break;
           default:
             if (prop->flipped)
               _flip_horizontal(to, w, h);
             break;
          }
        if (ptr_rotate)
          {
             free(ptr_rotate);
             ptr_rotate = NULL;
          }
        if (region)
          {
             opts->region.x = load_region_x;
             opts->region.y = load_region_y;
             opts->region.w = load_region_w;
             opts->region.h = load_region_h;
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
	return EINA_FALSE;
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

static void *
evas_image_load_file_open_jpeg(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
			       Evas_Image_Load_Opts *opts,
			       Evas_Image_Animated *animated EINA_UNUSED,
			       int *error)
{
   Evas_Loader_Internal *loader;

   loader = calloc(1, sizeof (Evas_Loader_Internal));
   if (!loader)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return NULL;
     }

   loader->f = f;
   loader->opts = opts;

   return loader;
}

static void
evas_image_load_file_close_jpeg(void *loader_data)
{
   free(loader_data);
}

static Eina_Bool
evas_image_load_file_head_jpeg(void *loader_data,
                               Evas_Image_Property *prop,
                               int *error)
{
   Evas_Loader_Internal *loader = loader_data;
   Evas_Image_Load_Opts *opts;
   Eina_File *f;
   void *map;
   Eina_Bool val;

   opts = loader->opts;
   f = loader->f;

   map = eina_file_map_all(f, EINA_FILE_RANDOM);
   if (!map)
     {
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
        return EINA_FALSE;
     }

   val = evas_image_load_file_head_jpeg_internal(&prop->w, &prop->h,
                                                 &prop->scale, &prop->rotated,
                                                 &prop->flipped,
                                                 opts,
                                                 map, eina_file_size_get(f),
                                                 error);

   eina_file_map_free(f, map);

   return val;
}

static Eina_Bool
evas_image_load_file_data_jpeg(void *loader_data,
                               Evas_Image_Property *prop,
                               void *pixels,
                               int *error)
{
   Evas_Loader_Internal *loader = loader_data;
   Evas_Image_Load_Opts *opts;
   Eina_File *f;
   void *map;
   Eina_Bool val = EINA_FALSE;

   f = loader->f;
   opts = loader->opts;

   map = eina_file_map_all(f, EINA_FILE_WILLNEED);
   if (!map)
     {
        *error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
        goto on_error;
     }

   val = evas_image_load_file_data_jpeg_internal(opts, prop, pixels,
                                                 map, eina_file_size_get(f),
                                                 error);

   eina_file_map_free(f, map);

 on_error:
   return val;
}

static Evas_Image_Load_Func evas_image_load_jpeg_func =
{
  evas_image_load_file_open_jpeg,
  evas_image_load_file_close_jpeg,
  evas_image_load_file_head_jpeg,
  evas_image_load_file_data_jpeg,
  NULL,
  EINA_TRUE,
  EINA_TRUE
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_jpeg_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
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


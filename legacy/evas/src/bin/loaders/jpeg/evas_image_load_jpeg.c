#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include <setjmp.h>
#include <jpeglib.h>

#include "evas_macros.h"

#include "evas_cserve2.h"
#include "evas_cserve2_slave.h"

typedef struct _JPEG_error_mgr *emptr;
struct _JPEG_error_mgr
{
   struct     jpeg_error_mgr pub;
   jmp_buf    setjmp_buffer;
};

static void _JPEGFatalErrorHandler(j_common_ptr cinfo);
static void _JPEGErrorHandler(j_common_ptr cinfo);
static void _JPEGErrorHandler2(j_common_ptr cinfo, int msg_level);

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
evas_image_load_file_head_jpeg_internal(Evas_Img_Load_Params *ilp,
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
	  *error = CSERVE2_CORRUPT_FILE;
	else
	  *error = CSERVE2_UNKNOWN_FORMAT;
	return EINA_FALSE;
     }

   degree = 0;
   change_wh = EINA_FALSE;
   jpeg_create_decompress(&cinfo);

   if (_evas_jpeg_membuf_src(&cinfo, map, length))
     {
        jpeg_destroy_decompress(&cinfo);
        _evas_jpeg_membuf_src_term(&cinfo);
        *error = CSERVE2_RESOURCE_ALLOCATION_FAILED;
        return EINA_FALSE;
     }

   jpeg_read_header(&cinfo, TRUE);
   cinfo.do_fancy_upsampling = FALSE;
   cinfo.do_block_smoothing = FALSE;
   cinfo.dct_method = JDCT_ISLOW; // JDCT_FLOAT JDCT_IFAST(quality loss)
   cinfo.dither_mode = JDITHER_ORDERED;
   jpeg_start_decompress(&cinfo);

   /* rotation decoding */
   if (ilp->opts.orientation)
     {
        degree = _get_orientation(map, length);
        if (degree != 0)
          {
             ilp->degree = degree;
             ilp->rotated = EINA_TRUE;

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
	  *error = CSERVE2_RESOURCE_ALLOCATION_FAILED;
	else
	  *error = CSERVE2_GENERIC;
	return EINA_FALSE;
     }
   if (ilp->opts.scale_down_by > 1)
     {
	w /= ilp->opts.scale_down_by;
	h /= ilp->opts.scale_down_by;
     }
   else if (ilp->opts.dpi > 0.0)
     {
	w = (w * ilp->opts.dpi) / 90.0;
	h = (h * ilp->opts.dpi) / 90.0;
     }
   else if ((ilp->opts.w > 0) && (ilp->opts.h > 0))
     {
        unsigned int w2 = w, h2 = h;
        /* user set load_opts' w,h on the assumption
           that image already rotated according to it's orientation info */
        if (change_wh)
          {
             load_opts_w = ilp->opts.w;
             load_opts_h = ilp->opts.h;
             ilp->opts.w = load_opts_h;
             ilp->opts.h = load_opts_w;
          }

	if (ilp->opts.w > 0)
	  {
	     w2 = ilp->opts.w;
	     h2 = (ilp->opts.w * h) / w;
	     if ((ilp->opts.h > 0) && (h2 > ilp->opts.h))
	       {
	          unsigned int w3;
		  h2 = ilp->opts.h;
		  w3 = (ilp->opts.h * w) / h;
		  if (w3 > w2)
		    w2 = w3;
	       }
	  }
	else if (ilp->opts.h > 0)
	  {
	     h2 = ilp->opts.h;
	     w2 = (ilp->opts.h * w) / h;
	  }
	w = w2;
	h = h2;
        if (change_wh)
          {
             ilp->opts.w = load_opts_w;
             ilp->opts.h = load_opts_h;
          }
     }
   if (w < 1) w = 1;
   if (h < 1) h = 1;

   if ((w != cinfo.output_width) || (h != cinfo.output_height))
     {
	scalew = cinfo.output_width / w;
	scaleh = cinfo.output_height / h;

	ilp->scale = scalew;
	if (scaleh < scalew) ilp->scale = scaleh;

	if      (ilp->scale > 8) ilp->scale = 8;
	else if (ilp->scale < 1) ilp->scale = 1;

	if      (ilp->scale == 3) ilp->scale = 2;
	else if (ilp->scale == 5) ilp->scale = 4;
	else if (ilp->scale == 6) ilp->scale = 4;
	else if (ilp->scale == 7) ilp->scale = 4;
     }

   if (ilp->scale > 1)
     {
	jpeg_destroy_decompress(&cinfo);
        _evas_jpeg_membuf_src_term(&cinfo);
	jpeg_create_decompress(&cinfo);

        if (_evas_jpeg_membuf_src(&cinfo, map, length))
          {
             jpeg_destroy_decompress(&cinfo);
             _evas_jpeg_membuf_src_term(&cinfo);
             *error = CSERVE2_RESOURCE_ALLOCATION_FAILED;
             return EINA_FALSE;
          }

	jpeg_read_header(&cinfo, TRUE);
	cinfo.do_fancy_upsampling = FALSE;
	cinfo.do_block_smoothing = FALSE;
	cinfo.scale_num = 1;
	cinfo.scale_denom = ilp->scale;
	jpeg_calc_output_dimensions(&(cinfo));
	jpeg_start_decompress(&cinfo);
     }

   ilp->w = cinfo.output_width;
   ilp->h = cinfo.output_height;
   
   // be nice and clip region to image. if its totally outside, fail load
   if ((ilp->opts.rw > 0) && (ilp->opts.rh > 0))
     {
        unsigned int load_region_x = 0, load_region_y = 0;
        unsigned int load_region_w = 0, load_region_h = 0;
        if (ilp->rotated)
          {
             load_region_x = ilp->opts.rx;
             load_region_y = ilp->opts.ry;
             load_region_w = ilp->opts.rw;
             load_region_h = ilp->opts.rh;

             switch (degree)
               {
                case 90:
                   ilp->opts.rx = load_region_y;
                   ilp->opts.ry = h - (load_region_x + load_region_w);
                   ilp->opts.rw = load_region_h;
                   ilp->opts.rh = load_region_w;
                   break;
                case 180:
                   ilp->opts.rx = w - (load_region_x+ load_region_w);
                   ilp->opts.ry = h - (load_region_y + load_region_h);

                   break;
                case 270:
                   ilp->opts.rx = w - (load_region_y + load_region_h);
                   ilp->opts.ry = load_region_x;
                   ilp->opts.rw = load_region_h;
                   ilp->opts.rh = load_region_w;
                   break;
                default:
                   break;
               }

          }
        RECTS_CLIP_TO_RECT(ilp->opts.rx, ilp->opts.ry,
                           ilp->opts.rw, ilp->opts.rh,
                           0, 0, ilp->w, ilp->h);
        if ((ilp->opts.rw <= 0) || (ilp->opts.rh <= 0))
          {
             jpeg_destroy_decompress(&cinfo);
             _evas_jpeg_membuf_src_term(&cinfo);
	     *error = CSERVE2_GENERIC;
	     return EINA_FALSE;
          }
        ilp->w = ilp->opts.rw;
        ilp->h = ilp->opts.rh;
        if (ilp->rotated)
          {
             ilp->opts.rx = load_region_x;
             ilp->opts.ry = load_region_y;
             ilp->opts.rw = load_region_w;
             ilp->opts.rh = load_region_h;
          }
     }
/* end head decoding */

   if (change_wh)
     {
        unsigned int tmp;
        tmp = ilp->w;
        ilp->w = ilp->h;
        ilp->h = tmp;
     }
   jpeg_destroy_decompress(&cinfo);
   _evas_jpeg_membuf_src_term(&cinfo);
   *error = CSERVE2_NONE;
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
evas_image_load_file_data_jpeg_internal(Evas_Img_Load_Params *ilp,
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
   unsigned int tmp;
   unsigned int load_region_x = 0, load_region_y = 0;
   unsigned int load_region_w = 0, load_region_h = 0;
   volatile int degree = 0;
   volatile Eina_Bool change_wh = EINA_FALSE;
   Eina_Bool line_done = EINA_FALSE;

   if (ilp->rotated)
     {
        degree = ilp->degree;
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
	*error = CSERVE2_CORRUPT_FILE;
	return EINA_FALSE;
     }
   jpeg_create_decompress(&cinfo);

   if (_evas_jpeg_membuf_src(&cinfo, map, size))
     {
        jpeg_destroy_decompress(&cinfo);
        _evas_jpeg_membuf_src_term(&cinfo);
        *error = CSERVE2_RESOURCE_ALLOCATION_FAILED;
        return 0;
     }

   jpeg_read_header(&cinfo, TRUE);
   cinfo.do_fancy_upsampling = FALSE;
   cinfo.do_block_smoothing = FALSE;
   cinfo.dct_method = JDCT_ISLOW; // JDCT_FLOAT JDCT_IFAST(quality loss)
   cinfo.dither_mode = JDITHER_ORDERED;

   if (ilp->scale > 1)
     {
	cinfo.scale_num = 1;
	cinfo.scale_denom = ilp->scale;
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
       /* unhandled format, do something */
       break;
     }

/* head decoding */
   jpeg_calc_output_dimensions(&(cinfo));
   jpeg_start_decompress(&cinfo);

   w = cinfo.output_width;
   h = cinfo.output_height;

   if (change_wh)
     {
        tmp = ilp->w;
        ilp->w = ilp->h;
        ilp->h = tmp;
     }

   if ((ilp->opts.rw > 0) && (ilp->opts.rh > 0))
     {
        region = 1;

        if (ilp->rotated)
          {
             load_region_x = ilp->opts.rx;
             load_region_y = ilp->opts.ry;
             load_region_w = ilp->opts.rw;
             load_region_h = ilp->opts.rh;

             switch (degree)
               {
                case 90:
                   ilp->opts.rx = load_region_y;
                   ilp->opts.ry = h - (load_region_x + load_region_w);
                   ilp->opts.rw = load_region_h;
                   ilp->opts.rh = load_region_w;
                   break;
                case 180:
                   ilp->opts.rx = w - (load_region_x+ load_region_w);
                   ilp->opts.ry = h - (load_region_y + load_region_h);

                   break;
                case 270:
                   ilp->opts.rx = w - (load_region_y + load_region_h);
                   ilp->opts.ry = load_region_x;
                   ilp->opts.rw = load_region_h;
                   ilp->opts.rh = load_region_w;
                   break;
                default:
                   break;
               }

          }
#ifdef BUILD_LOADER_JPEG_REGION
        cinfo.region_x = ilp->opts.rx;
        cinfo.region_y = ilp->opts.ry;
        cinfo.region_w = ilp->opts.rw;
        cinfo.region_h = ilp->opts.rh;
#endif
     }
   /* what to do about this? We'll check for this kind of races on loaders
    * or cache invalidation due to file modification should always be
    * detected by the server? */

   if ((!region) && ((w != ilp->w) || (h != ilp->h)))
     {
	// race condition, the file could have change from when we call header
	// this test will not solve the problem with region code.
	jpeg_destroy_decompress(&cinfo);
        _evas_jpeg_membuf_src_term(&cinfo);
	*error = CSERVE2_GENERIC;
	return EINA_FALSE;
     }

   if ((region) &&
       ((ilp->w != ilp->opts.rw) || (ilp->h != ilp->opts.rh)))
     {
        ilp->w = ilp->opts.rw;
        ilp->h = ilp->opts.rh;
     }

   if (!(((cinfo.out_color_space == JCS_RGB) &&
          ((cinfo.output_components == 3) || (cinfo.output_components == 1))) ||
         ((cinfo.out_color_space == JCS_CMYK) && (cinfo.output_components == 4))))
     {
	jpeg_destroy_decompress(&cinfo);
        _evas_jpeg_membuf_src_term(&cinfo);
	*error = CSERVE2_UNKNOWN_FORMAT;
	return EINA_FALSE;
     }

/* end head decoding */
/* data decoding */
   if (cinfo.rec_outbuf_height > 16)
     {
	jpeg_destroy_decompress(&cinfo);
        _evas_jpeg_membuf_src_term(&cinfo);
	*error = CSERVE2_UNKNOWN_FORMAT;
	return EINA_FALSE;
     }
   data = alloca(w * 16 * cinfo.output_components);
   if ((ilp->rotated) && change_wh)
     {
        ptr2 = malloc(ilp->w * ilp->h * sizeof(DATA32));
        ptr_rotate = ptr2;
     }
   else
     ptr2 = ilp->buffer;;

   if (!ptr2)
     {
        *error = CSERVE2_RESOURCE_ALLOCATION_FAILED;
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
                  if (l >= (ilp->opts.ry + ilp->opts.rh))
                    {
                       line_done = EINA_TRUE;
                       /* if rotation flag is set , we have to rotate image */
                       goto done;
                       /*jpeg_destroy_decompress(&cinfo);
                         _evas_jpeg_membuf_src_term(&cinfo);
                        *error = NONE;
                        return EINA_FALSE;*/
                    }
                  // els if scan block intersects region start or later
                  else if ((l + scans) > 
                           (ilp->opts.ry))
                    {
                       for (y = 0; y < scans; y++)
                         {
                            if (((y + l) >= ilp->opts.ry) &&
                                ((y + l) < (ilp->opts.ry + ilp->opts.rh)))
                              {
                                 ptr += ilp->opts.rx;
                                 if (cinfo.saw_Adobe_marker)
                                   {
                                      for (x = 0; x < ilp->opts.rw; x++)
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
                                      for (x = 0; x < ilp->opts.rw; x++)
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
                                 ptr += (4 * (w - (ilp->opts.rx + ilp->opts.rw)));
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
                    ilp->opts.rx,
                    ilp->opts.ry,
                    ilp->opts.rw,
                    ilp->opts.rh);
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
                  if (l >= (ilp->opts.ry + ilp->opts.rh))
                    {
                       line_done = EINA_TRUE;
                       /* if rotation flag is set , we have to rotate image */
                       goto done;
                    }
                  // else if scan block intersects region start or later
                  else if ((l + scans) > 
                           (ilp->opts.ry))
                    {
                       for (y = 0; y < scans; y++)
                         {
                            if (((y + l) >= ilp->opts.ry) &&
                                ((y + l) < (ilp->opts.ry + ilp->opts.rh)))
                              {
                                 ptr += (3 * ilp->opts.rx);
                                 for (x = 0; x < ilp->opts.rw; x++)
                                   {
                                      *ptr2 = ARGB_JOIN(0xff, ptr[0], ptr[1], ptr[2]);
                                      ptr += 3;
                                      ptr2++;
                                   }
                                 ptr += (3 * (w - (ilp->opts.rx + ilp->opts.rw)));
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
                  if (l >= (ilp->opts.ry + ilp->opts.rh))
                    {
                       line_done = EINA_TRUE;
                       /* if rotation flag is set , we have to rotate image */
                       goto done;
                       /*jpeg_destroy_decompress(&cinfo);
                         _evas_jpeg_membuf_src_term(&cinfo);
                        *error = NONE;
                        return EINA_TRUE;*/
                    }
                  // els if scan block intersects region start or later
                  else if ((l + scans) > 
                           (ilp->opts.ry))
                    {
                       for (y = 0; y < scans; y++)
                         {
                            if (((y + l) >= ilp->opts.ry) &&
                                ((y + l) < (ilp->opts.ry + ilp->opts.rh)))
                              {
                                 ptr += ilp->opts.rx;
                                 for (x = 0; x < ilp->opts.rw; x++)
                                   {
                                      *ptr2 = ARGB_JOIN(0xff, ptr[0], ptr[0], ptr[0]);
                                      ptr++;
                                      ptr2++;
                                   }
                                 ptr += w - (ilp->opts.rx + ilp->opts.rw);
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

   if (ilp->rotated)
     {
        DATA32             *data1, *data2,  *to, *from;
        int                 lx, ly, lw, lh,  hw;

        if (change_wh)
          {
             tmp = ilp->w;
             ilp->w = ilp->h;
             ilp->h = tmp;
          }

        lw = ilp->w;
        lh = ilp->h;
        hw = lw * lh;

        data1 = ilp->buffer;

        if (degree == 180)
          {
             DATA32 tmpd;

             data2 = data1 + (lh * lw) -1;
             for (lx = (lw * lh) / 2; --lx >= 0;)
               {
                  tmpd = *data1;
                  *data1 = *data2;
                  *data2 = tmpd;
                  data1++;
                  data2--;
               }
          }
        else
          {
             data2 = NULL;
             to = NULL;
             if (ptr_rotate) data2 = ptr_rotate;

             if (degree == 90)
               {
                  to = data1 + lw - 1;
                  hw = -hw - 1;
               }
             else if (degree == 270)
               {
                  to = data1 + hw - lw;
                  lw = -lw;
                  hw = hw + 1;
               }

             if (to)
               {
                  from = data2;
                  for (lx = ilp->w; --lx >= 0;)
                    {
                       for (ly = ilp->h; --ly >= 0;)
                         {
                            *to = *from;
                            from++;
                            to += lw;
                         }
                       to += hw;
                    }
               }
             if (ptr_rotate)
               {
                  free(ptr_rotate);
                  ptr_rotate = NULL;
               }
          }
        if (region)
          {
             ilp->opts.rx = load_region_x;
             ilp->opts.ry = load_region_y;
             ilp->opts.rw = load_region_w;
             ilp->opts.rh = load_region_h;
          }
     }
   if (line_done)
     {
        jpeg_destroy_decompress(&cinfo);
        _evas_jpeg_membuf_src_term(&cinfo);
        *error = CSERVE2_NONE;
        return EINA_FALSE;
     }
   /* end data decoding */
   jpeg_finish_decompress(&cinfo);
   jpeg_destroy_decompress(&cinfo);
   _evas_jpeg_membuf_src_term(&cinfo);
   *error = CSERVE2_NONE;
   return EINA_TRUE;
}

Eina_Bool
evas_image_load_file_head_jpeg(Evas_Img_Load_Params *ilp,
                               const char *file, const char *key __UNUSED__,
                               int *error)
{
   Eina_File *f;
   void *map;
   Eina_Bool val = EINA_FALSE;

   f = eina_file_open(file, EINA_FALSE);
   if (!f)
     {
	*error = CSERVE2_DOES_NOT_EXIST;
	return EINA_FALSE;
     }
   map = eina_file_map_all(f, EINA_FILE_WILLNEED);
   if (!map)
     {
	*error = CSERVE2_DOES_NOT_EXIST;
        goto on_error;
     }

   val = evas_image_load_file_head_jpeg_internal(ilp,
                                                 map, eina_file_size_get(f),
                                                 error);

   eina_file_map_free(f, map);

 on_error:
   eina_file_close(f);
   return val;
}

static Eina_Bool
evas_image_load_file_data_jpeg(Evas_Img_Load_Params *ilp,
                               const char *file, const char *key __UNUSED__,
                               int *error)
{
   Eina_File *f;
   void *map;
   Eina_Bool val = EINA_FALSE;

   f = eina_file_open(file, EINA_FALSE);
   if (!f)
     {
	*error = CSERVE2_DOES_NOT_EXIST;
	return EINA_FALSE;
     }
   map = eina_file_map_all(f, EINA_FILE_WILLNEED);
   if (!map)
     {
        *error = CSERVE2_DOES_NOT_EXIST;
        goto on_error;
     }

   val = evas_image_load_file_data_jpeg_internal(ilp,
                                                 map, eina_file_size_get(f),
                                                 error);

   eina_file_map_free(f, map);

 on_error:
   eina_file_close(f);
   return val;
}

static Evas_Loader_Module_Api modapi =
{
   EVAS_CSERVE2_MODULE_API_VERSION,
   "jpeg",
   evas_image_load_file_head_jpeg,
   evas_image_load_file_data_jpeg
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

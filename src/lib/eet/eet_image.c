#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#ifdef __OpenBSD__
# include <sys/types.h>
#endif /* ifdef __OpenBSD__ */

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef _WIN32
# include <winsock2.h>
# define HAVE_BOOLEAN
# define XMD_H /* This prevents libjpeg to redefine INT32 */
#endif /* ifdef _WIN32 */

#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <zlib.h>
#include <jpeglib.h>

#include "Eet.h"
#include "Eet_private.h"

#ifdef ENABLE_LIBLZ4
# include <lz4.h>
# include <lz4hc.h>
#else
# include "lz4.h"
# include "lz4hc.h"
#endif

#include "rg_etc1.h"

#ifdef BUILD_NEON
#include <arm_neon.h>
#endif

#ifndef WORDS_BIGENDIAN
/* x86 */
#define A_VAL(p) (((uint8_t *)(p))[3])
#define R_VAL(p) (((uint8_t *)(p))[2])
#define G_VAL(p) (((uint8_t *)(p))[1])
#define B_VAL(p) (((uint8_t *)(p))[0])
#else
/* ppc */
#define A_VAL(p) (((uint8_t *)(p))[0])
#define R_VAL(p) (((uint8_t *)(p))[1])
#define G_VAL(p) (((uint8_t *)(p))[2])
#define B_VAL(p) (((uint8_t *)(p))[3])
#endif

#define ARGB_JOIN(a,r,g,b) \
        (((a) << 24) + ((r) << 16) + ((g) << 8) + (b))

#define OFFSET_BLOCK_SIZE 4
#define OFFSET_ALGORITHM 5
#define OFFSET_OPTIONS 6
#define OFFSET_WIDTH 8
#define OFFSET_HEIGHT 12
#define OFFSET_BLOCKS 16

#undef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#undef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

/*---*/

typedef struct _JPEG_error_mgr *emptr;

/*---*/

struct _JPEG_error_mgr
{
   struct jpeg_error_mgr pub;
   jmp_buf               setjmp_buffer;
};

struct jpeg_membuf_src
{
   struct jpeg_source_mgr  pub;

   const unsigned char    *buf;
   size_t                  len;
   struct jpeg_membuf_src *self;
};

static void
_eet_jpeg_membuf_src_init(j_decompress_ptr cinfo)
{
   /* FIXME: Use attribute unused */
    (void)cinfo;
}

static boolean
_eet_jpeg_membuf_src_fill(j_decompress_ptr cinfo)
{
   static const JOCTET jpeg_eoi[2] = { 0xFF, JPEG_EOI };
   struct jpeg_membuf_src *src = (struct jpeg_membuf_src *)cinfo->src;

   src->pub.bytes_in_buffer = sizeof(jpeg_eoi);
   src->pub.next_input_byte = jpeg_eoi;

   return TRUE;
}

static void
_eet_jpeg_membuf_src_skip(j_decompress_ptr cinfo,
                          long             num_bytes)
{
   struct jpeg_membuf_src *src = (struct jpeg_membuf_src *)cinfo->src;

   src->pub.bytes_in_buffer -= num_bytes;
   src->pub.next_input_byte += num_bytes;
}

static void
_eet_jpeg_membuf_src_term(j_decompress_ptr cinfo)
{
   struct jpeg_membuf_src *src = ((struct jpeg_membuf_src *)cinfo->src)->self;

   free(src);
   cinfo->src = NULL;
}

static int
eet_jpeg_membuf_src(j_decompress_ptr cinfo,
                    const void      *buf,
                    size_t           len)
{
   struct jpeg_membuf_src *src;

   src = calloc(1, sizeof(*src));
   if (!src)
     return -1;

   src->self = src;

   cinfo->src = &src->pub;
   src->buf = buf;
   src->len = len;
   src->pub.init_source = _eet_jpeg_membuf_src_init;
   src->pub.fill_input_buffer = _eet_jpeg_membuf_src_fill;
   src->pub.skip_input_data = _eet_jpeg_membuf_src_skip;
   src->pub.resync_to_restart = jpeg_resync_to_restart;
   src->pub.term_source = _eet_jpeg_membuf_src_term;
   src->pub.bytes_in_buffer = src->len;
   src->pub.next_input_byte = src->buf;

   return 0;
}

struct jpeg_membuf_dst
{
   struct jpeg_destination_mgr pub;

   void                      **dst_buf;
   size_t                     *dst_len;

   unsigned char              *buf;
   size_t                      len;
   int                         failed;
   struct jpeg_membuf_dst     *self;
};

static void
_eet_jpeg_membuf_dst_init(j_compress_ptr cinfo)
{
   /* FIXME: Use eina attribute */
    (void)cinfo;
}

static boolean
_eet_jpeg_membuf_dst_flush(j_compress_ptr cinfo)
{
   struct jpeg_membuf_dst *dst = (struct jpeg_membuf_dst *)cinfo->dest;
   unsigned char *buf;

   if (dst->len >= 0x40000000 ||
       !(buf = realloc(dst->buf, dst->len * 2)))
     {
        dst->failed = 1;
        dst->pub.next_output_byte = dst->buf;
        dst->pub.free_in_buffer = dst->len;
        return TRUE;
     }

   dst->pub.next_output_byte =
     buf + ((unsigned char *)dst->pub.next_output_byte - dst->buf);
   dst->buf = buf;
   dst->pub.free_in_buffer += dst->len;
   dst->len *= 2;

   return FALSE;
}

static void
_eet_jpeg_membuf_dst_term(j_compress_ptr cinfo)
{
   struct jpeg_membuf_dst *dst = ((struct jpeg_membuf_dst *)cinfo->dest)->self;

   if (dst->failed)
     {
        *dst->dst_buf = NULL;
        *dst->dst_len = 0;
        free(dst->buf);
     }
   else
     {
        *dst->dst_buf = dst->buf;
        *dst->dst_len = (unsigned char *)dst->pub.next_output_byte - dst->buf;
     }

   free(dst);
   cinfo->dest = NULL;
}

static int
eet_jpeg_membuf_dst(j_compress_ptr cinfo,
                    void         **buf,
                    size_t        *len)
{
   struct jpeg_membuf_dst *dst;

   dst = calloc(1, sizeof(*dst));
   if (!dst)
     return -1;

   dst->buf = malloc(32768);
   if (!dst->buf)
     {
        free(dst);
        return -1;
     }

   dst->self = dst;
   dst->len = 32768;

   cinfo->dest = &dst->pub;
   dst->pub.init_destination = _eet_jpeg_membuf_dst_init;
   dst->pub.empty_output_buffer = _eet_jpeg_membuf_dst_flush;
   dst->pub.term_destination = _eet_jpeg_membuf_dst_term;
   dst->pub.free_in_buffer = dst->len;
   dst->pub.next_output_byte = dst->buf;
   dst->dst_buf = buf;
   dst->dst_len = len;
   dst->failed = 0;

   return 0;
}

/*---*/

static void _JPEGFatalErrorHandler(j_common_ptr cinfo);
static void _JPEGErrorHandler(j_common_ptr cinfo);
static void _JPEGErrorHandler2(j_common_ptr cinfo,
                               int          msg_level);

static int
eet_data_image_jpeg_header_decode(const void   *data,
                                  int           size,
                                  unsigned int *w,
                                  unsigned int *h);
static int
eet_data_image_jpeg_rgb_decode(const void   *data,
                               int           size,
                               unsigned int  src_x,
                               unsigned int  src_y,
                               unsigned int *d,
                               unsigned int  w,
                               unsigned int  h,
                               unsigned int  row_stride);
static int
eet_data_image_jpeg_alpha_decode(const void   *data,
                                 int           size,
                                 unsigned int  src_x,
                                 unsigned int  src_y,
                                 unsigned int *d,
                                 unsigned int  w,
                                 unsigned int  h,
                                 unsigned int  row_stride);
static void *
eet_data_image_lossless_convert(int         *size,
                                const void  *data,
                                unsigned int w,
                                unsigned int h,
                                int          alpha);
static void *
eet_data_image_lossless_compressed_convert(int         *size,
                                           const void  *data,
                                           unsigned int w,
                                           unsigned int h,
                                           int          alpha,
                                           int          compression);
static void *
eet_data_image_jpeg_convert(int         *size,
                            const void  *data,
                            unsigned int w,
                            unsigned int h,
                            int          alpha,
                            int          quality);
static void *
eet_data_image_jpeg_alpha_convert(int         *size,
                                  const void  *data,
                                  unsigned int w,
                                  unsigned int h,
                                  int          alpha,
                                  int          quality);

/*---*/

static int _eet_image_words_bigendian = -1;

/*---*/

#define SWAP64(x) (x) =                                        \
  ((((unsigned long long)(x) & 0x00000000000000ffULL) << 56) | \
   (((unsigned long long)(x) & 0x000000000000ff00ULL) << 40) | \
   (((unsigned long long)(x) & 0x0000000000ff0000ULL) << 24) | \
   (((unsigned long long)(x) & 0x00000000ff000000ULL) << 8) |  \
   (((unsigned long long)(x) & 0x000000ff00000000ULL) >> 8) |  \
   (((unsigned long long)(x) & 0x0000ff0000000000ULL) >> 24) | \
   (((unsigned long long)(x) & 0x00ff000000000000ULL) >> 40) | \
   (((unsigned long long)(x) & 0xff00000000000000ULL) >> 56))
#define SWAP32(x) (x) =              \
  ((((int)(x) & 0x000000ff) << 24) | \
   (((int)(x) & 0x0000ff00) << 8) |  \
   (((int)(x) & 0x00ff0000) >> 8) |  \
   (((int)(x) & 0xff000000) >> 24))
#define SWAP16(x) (x) =           \
  ((((short)(x) & 0x00ff) << 8) | \
   (((short)(x) & 0xff00) >> 8))

#ifdef CONV8
# undef CONV8
#endif /* ifdef CONV8 */
#ifdef CONV16
# undef CONV16
#endif /* ifdef CONV16 */
#ifdef CONV32
# undef CONV32
#endif /* ifdef CONV32 */
#ifdef CONV64
# undef CONV64
#endif /* ifdef CONV64 */

#define CONV8(x)
#define CONV16(x) {if (_eet_image_words_bigendian) {SWAP16(x); }}
#define CONV32(x) {if (_eet_image_words_bigendian) {SWAP32(x); }}
#define CONV64(x) {if (_eet_image_words_bigendian) {SWAP64(x); }}

/*---*/

static void
_JPEGFatalErrorHandler(j_common_ptr cinfo)
{
   emptr errmgr;

   errmgr = (emptr)cinfo->err;
   /*   cinfo->err->output_message(cinfo);*/
   longjmp(errmgr->setjmp_buffer, 1);
   return;
}

static void
_JPEGErrorHandler(j_common_ptr cinfo EINA_UNUSED)
{
   /*   emptr errmgr; */

    /*   errmgr = (emptr) cinfo->err; */
    /*   cinfo->err->output_message(cinfo);*/
    /*   longjmp(errmgr->setjmp_buffer, 1);*/
       return;
}

static void
_JPEGErrorHandler2(j_common_ptr cinfo EINA_UNUSED,
                   int          msg_level EINA_UNUSED)
{
   /*   emptr errmgr; */

    /*   errmgr = (emptr) cinfo->err; */
    /*   cinfo->err->output_message(cinfo);*/
    /*   longjmp(errmgr->setjmp_buffer, 1);*/
       return;
}

static int
eet_data_image_jpeg_header_decode(const void   *data,
                                  int           size,
                                  unsigned int *w,
                                  unsigned int *h)
{
   struct jpeg_decompress_struct cinfo;
   struct _JPEG_error_mgr jerr;

   memset(&cinfo, 0, sizeof (struct jpeg_decompress_struct));

   cinfo.err = jpeg_std_error(&(jerr.pub));
   jerr.pub.error_exit = _JPEGFatalErrorHandler;
   jerr.pub.emit_message = _JPEGErrorHandler2;
   jerr.pub.output_message = _JPEGErrorHandler;
   if (setjmp(jerr.setjmp_buffer))
     return 0;

   jpeg_create_decompress(&cinfo);

   if (eet_jpeg_membuf_src(&cinfo, data, (size_t)size))
     {
        jpeg_destroy_decompress(&cinfo);
        return 0;
     }

   jpeg_read_header(&cinfo, TRUE);
   cinfo.do_fancy_upsampling = FALSE;
   cinfo.do_block_smoothing = FALSE;
   jpeg_start_decompress(&cinfo);

   /* head decoding */
   *w = cinfo.output_width;
   *h = cinfo.output_height;

   free(cinfo.src);
   cinfo.src = NULL;

   jpeg_destroy_decompress(&cinfo);

   if ((*w < 1) || (*h < 1) || (*w > 8192) || (*h > 8192))
     return 0;

   return 1;
}

static int
eet_data_image_jpeg_rgb_decode(const void   *data,
                               int           size,
                               unsigned int  src_x,
                               unsigned int  src_y,
                               unsigned int *d,
                               unsigned int  w,
                               unsigned int  h,
                               unsigned int  row_stride)
{
   struct jpeg_decompress_struct cinfo;
   struct _JPEG_error_mgr jerr;
   unsigned char *ptr, *line[16], *tdata = NULL;
   unsigned int *ptr2, *tmp;
   unsigned int iw, ih;
   unsigned int x, y, l, scans;
   unsigned int i;

   /* FIXME: handle src_x, src_y and row_stride correctly */
   if (!d)
     return 0;

   memset(&cinfo, 0, sizeof (struct jpeg_decompress_struct));

   cinfo.err = jpeg_std_error(&(jerr.pub));
   jerr.pub.error_exit = _JPEGFatalErrorHandler;
   jerr.pub.emit_message = _JPEGErrorHandler2;
   jerr.pub.output_message = _JPEGErrorHandler;
   if (setjmp(jerr.setjmp_buffer))
     return 0;

   jpeg_create_decompress(&cinfo);

   if (eet_jpeg_membuf_src(&cinfo, data, (size_t)size))
     {
        jpeg_destroy_decompress(&cinfo);
        return 0;
     }

   jpeg_read_header(&cinfo, TRUE);
   cinfo.dct_method = JDCT_ISLOW; // JDCT_FLOAT JDCT_IFAST(quality loss)
   cinfo.do_fancy_upsampling = FALSE;
   cinfo.do_block_smoothing = FALSE;
   jpeg_start_decompress(&cinfo);

   /* head decoding */
   iw = cinfo.output_width;
   ih = cinfo.output_height;
   if ((iw != w) || (ih != h))
     {
        free(cinfo.src);
        cinfo.src = NULL;

        jpeg_destroy_decompress(&cinfo);
        return 0;
     }

   /* end head decoding */
   /* data decoding */
   if (cinfo.rec_outbuf_height > 16)
     {
        free(cinfo.src);
        cinfo.src = NULL;

        jpeg_destroy_decompress(&cinfo);
        return 0;
     }

   tdata = alloca((iw) * 16 * 3);
   ptr2 = d;

   if (cinfo.output_components == 3)
     {
        for (i = 0; i < (unsigned int)cinfo.rec_outbuf_height; i++)
          line[i] = tdata + (i * (iw) * 3);
        for (l = 0; l < ih; l += cinfo.rec_outbuf_height)
          {
             jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
             scans = cinfo.rec_outbuf_height;
             if ((ih - l) < scans)
               scans = ih - l;

             ptr = tdata;

             if (l + scans >= src_y && l < src_y + h)
               {
                  y = src_y - l;
                  if (src_y < l)
                    y = 0;

                  for (ptr += 3 * iw * y; y < scans && (y + l) < (src_y + h);
                       y++)
                    {
                       tmp = ptr2;
                       ptr += 3 * src_x;
                       for (x = 0; x < w; x++)
                         {
                            *ptr2 =
                              (0xff000000) |
                              ((ptr[0]) << 16) | ((ptr[1]) << 8) | (ptr[2]);
                            ptr += 3;
                            ptr2++;
                         }
                       ptr += 3 * (iw - w);
                       ptr2 = tmp + row_stride / 4;
                    }
               }
          }
     }
   else if (cinfo.output_components == 1)
     {
        for (i = 0; i < (unsigned int)cinfo.rec_outbuf_height; i++)
          line[i] = tdata + (i * (iw));
        for (l = 0; l < (ih); l += cinfo.rec_outbuf_height)
          {
             jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
             scans = cinfo.rec_outbuf_height;
             if (((ih) - l) < scans)
               scans = (ih) - l;

             ptr = tdata;

             if (l >= src_y && l < src_y + h)
               {
                  y = src_y - l;
                  if (src_y < l)
                    y = 0;

                  for (ptr += iw * y; y < scans && (y + l) < (src_y + h); y++)
                    {
                       tmp = ptr2;
                       ptr += src_x;
                       for (x = 0; x < w; x++)
                         {
                            *ptr2 =
                              (0xff000000) |
                              ((ptr[0]) << 16) | ((ptr[0]) << 8) | (ptr[0]);
                            ptr++;
                            ptr2++;
                         }
                       ptr += iw - w;
                       ptr2 = tmp + row_stride / 4;
                    }
               }
          }
     }

   /* end data decoding */
   jpeg_finish_decompress(&cinfo);
   jpeg_destroy_decompress(&cinfo);
   return 1;
}

static int
eet_data_image_jpeg_alpha_decode(const void   *data,
                                 int           size,
                                 unsigned int  src_x,
                                 unsigned int  src_y,
                                 unsigned int *d,
                                 unsigned int  w,
                                 unsigned int  h,
                                 unsigned int  row_stride)
{
   struct jpeg_decompress_struct cinfo;
   struct _JPEG_error_mgr jerr;
   unsigned char *ptr, *line[16], *tdata = NULL;
   unsigned int *ptr2, *tmp;
   unsigned int x, y, l, scans;
   unsigned int i, iw;

   /* FIXME: handle src_x, src_y and row_stride correctly */
   if (!d)
     return 0;

   memset(&cinfo, 0, sizeof (struct jpeg_decompress_struct));

   cinfo.err = jpeg_std_error(&(jerr.pub));
   jerr.pub.error_exit = _JPEGFatalErrorHandler;
   jerr.pub.emit_message = _JPEGErrorHandler2;
   jerr.pub.output_message = _JPEGErrorHandler;
   if (setjmp(jerr.setjmp_buffer))
     return 0;

   jpeg_create_decompress(&cinfo);

   if (eet_jpeg_membuf_src(&cinfo, data, (size_t)size))
     {
        jpeg_destroy_decompress(&cinfo);
        return 0;
     }

   jpeg_read_header(&cinfo, TRUE);
   cinfo.dct_method = JDCT_ISLOW; // JDCT_FLOAT JDCT_IFAST(quality loss)
   cinfo.do_fancy_upsampling = FALSE;
   cinfo.do_block_smoothing = FALSE;
   jpeg_start_decompress(&cinfo);

   /* head decoding */
   iw = cinfo.output_width;
   if (w != cinfo.output_width
       || h != cinfo.output_height)
     {
        free(cinfo.src);
        cinfo.src = NULL;

        jpeg_destroy_decompress(&cinfo);
        return 0;
     }

   /* end head decoding */
   /* data decoding */
   if (cinfo.rec_outbuf_height > 16)
     {
        free(cinfo.src);
        cinfo.src = NULL;

        jpeg_destroy_decompress(&cinfo);
        return 0;
     }

   tdata = alloca(w * 16 * 3);
   ptr2 = d;

   if (cinfo.output_components == 1)
     {
        for (i = 0; i < (unsigned int)cinfo.rec_outbuf_height; i++)
          line[i] = tdata + (i * w);
        for (l = 0; l < h; l += cinfo.rec_outbuf_height)
          {
             jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
             scans = cinfo.rec_outbuf_height;
             if ((h - l) < scans)
               scans = h - l;

             ptr = tdata;

             if (l >= src_y && l < src_y + h)
               {
                  y = src_y - l;
                  if (src_y < l)
                    y = 0;

                  for (ptr += iw * y; y < scans && (y + l) < (src_y + h); y++)
                    {
                       tmp = ptr2;
                       ptr += src_x;
                       for (x = 0; x < w; x++)
                         {
                            *ptr2 =
                              ((*ptr2) & 0x00ffffff) |
                              ((ptr[0]) << 24);
                            ptr++;
                            ptr2++;
                         }
                       ptr += iw - w;
                       ptr2 = tmp + row_stride / 4;
                    }
               }
          }
     }

   /* end data decoding */
   jpeg_finish_decompress(&cinfo);
   jpeg_destroy_decompress(&cinfo);
   return 1;
}

// FIXME: Importing two functions from evas here: premul & unpremul
static void
_eet_argb_premul(unsigned int *data, unsigned int len)
{
   unsigned int *de = data + len;

   while (data < de)
     {
        unsigned int  a = 1 + (*data >> 24);

        *data = (*data & 0xff000000) +
          (((((*data) >> 8) & 0xff) * a) & 0xff00) +
          (((((*data) & 0x00ff00ff) * a) >> 8) & 0x00ff00ff);
        data++;
     }
}

static void
_eet_argb_unpremul(unsigned int *data, unsigned int len)
{
   unsigned int *de = data + len;
   unsigned int p_val = 0x00000000, p_res = 0x00000000;

   while (data < de)
     {
        unsigned int  a = (*data >> 24) + 1;

        if (p_val == *data) *data = p_res;
        else
          {
             p_val = *data;
             if ((a > 1) && (a < 256))
               *data = ARGB_JOIN(a,
                                 (R_VAL(data) * 255) / a,
                                 (G_VAL(data) * 255) / a,
                                 (B_VAL(data) * 255) / a);
             else if (a == 1)
               *data = 0x00000000;
             p_res = *data;
          }
        data++;
     }
}

static inline unsigned int
_tgv_length_get(const char *m, unsigned int length, unsigned int *offset)
{
   unsigned int r = 0;
   unsigned int shift = 0;

   while (*offset < length && ((*m) & 0x80))
     {
        r = r | (((*m) & 0x7F) << shift);
        shift += 7;
        m++;
        (*offset)++;
     }
   if (*offset < length)
     {
        r = r | (((*m) & 0x7F) << shift);
        (*offset)++;
     }

   return r;
}

static int
roundup(int val, int rup)
{
   if (val >= 0 && rup > 0)
     return (val + rup - 1) - ((val + rup - 1) % rup);
   return 0;
}

static int
eet_data_image_etc2_decode(const void *data,
                           unsigned int length,
                           unsigned int *p,
                           unsigned int dst_x,
                           unsigned int dst_y,
                           unsigned int dst_w,
                           unsigned int dst_h,
                           Eina_Bool alpha,
                           Eet_Colorspace cspace,
                           Eet_Image_Encoding lossy)
{
   const char *m = NULL;
   unsigned int bwidth, bheight;
   unsigned char *p_etc;
   char *buffer = NULL;
   Eina_Rectangle master;
   unsigned int block_length;
   unsigned int offset;
   unsigned int x, y, w, h;
   unsigned int block_count;
   unsigned int etc_width = 0;
   unsigned int etc_block_size;
   unsigned int num_planes = 1, plane, alpha_offset = 0;
   Eet_Colorspace file_cspace;
   Eina_Bool compress, blockless, unpremul;

   m = data;

   // Fix for ABI incompatibility between 1.10 and 1.11
   if (cspace == 8) cspace = 9;

   if (strncmp(m, "TGV1", 4) != 0)
     return 0;

   compress = m[OFFSET_OPTIONS] & 0x1;
   blockless = (m[OFFSET_OPTIONS] & 0x2) != 0;
   unpremul = (m[OFFSET_OPTIONS] & 0x4) != 0;
   w = ntohl(*((unsigned int*) &(m[OFFSET_WIDTH])));
   h = ntohl(*((unsigned int*) &(m[OFFSET_HEIGHT])));

   switch (m[OFFSET_ALGORITHM] & 0xFF)
     {
      case 0:
        if (lossy != EET_IMAGE_ETC1) return 0;
        file_cspace = EET_COLORSPACE_ETC1;
        if (alpha != EINA_FALSE) return 0;
        etc_block_size = 8;
        break;
      case 1:
        if (lossy != EET_IMAGE_ETC2_RGB) return 0;
        file_cspace = EET_COLORSPACE_RGB8_ETC2;
        if (alpha != EINA_FALSE) return 0;
        etc_block_size = 8;
        break;
      case 2:
        if (lossy != EET_IMAGE_ETC2_RGBA) return 0;
        file_cspace = EET_COLORSPACE_RGBA8_ETC2_EAC;
        if (alpha != EINA_TRUE) return 0;
        etc_block_size = 16;
        break;
      case 3:
        if (lossy != EET_IMAGE_ETC1_ALPHA) return 0;
        file_cspace = EET_COLORSPACE_ETC1_ALPHA;
        if (alpha != EINA_TRUE) return 0;
        etc_block_size = 8;
        num_planes = 2;
        alpha_offset = ((w + 2 + 3) / 4) * ((h + 2 + 3) / 4) * 8 / sizeof(*p_etc);
        break;
      default:
        return 0;
     }
   etc_width = ((w + 2 + 3) / 4) * etc_block_size;

   if (cspace != EET_COLORSPACE_ARGB8888 && cspace != file_cspace)
     {
        if (!((cspace == EET_COLORSPACE_RGB8_ETC2) && (file_cspace == EET_COLORSPACE_ETC1)))
          return 0;
        // else: ETC2 is compatible with ETC1 and is preferred
     }

   if (blockless)
     {
        bwidth = roundup(w + 2, 4);
        bheight = roundup(h + 2, 4);
     }
   else
     {
        bwidth = 4 << (m[OFFSET_BLOCK_SIZE] & 0x0f);
        bheight = 4 << ((m[OFFSET_BLOCK_SIZE] & 0xf0) >> 4);
     }

   EINA_RECTANGLE_SET(&master, dst_x, dst_y, dst_w, dst_h);

   switch (cspace)
     {
      case EET_COLORSPACE_ETC1:
      case EET_COLORSPACE_RGB8_ETC2:
      case EET_COLORSPACE_RGBA8_ETC2_EAC:
      case EET_COLORSPACE_ETC1_ALPHA:
        if (master.x % 4 || master.y % 4)
          abort();
        break;
      case EET_COLORSPACE_ARGB8888:
        // Offset to take duplicated pixels into account
        master.x += 1;
        master.y += 1;
        break;
      default: abort();
     }

   p_etc = (unsigned char*) p;
   offset = OFFSET_BLOCKS;

   // Allocate space for each ETC block (8 or 16 bytes per 4 * 4 pixels group)
   block_count = bwidth * bheight / (4 * 4);
   if (compress)
     buffer = alloca(etc_block_size * block_count);

   for (plane = 0; plane < num_planes; plane++)
     for (y = 0; y < h + 2; y += bheight)
       for (x = 0; x < w + 2; x += bwidth)
         {
            Eina_Rectangle current;
            const char *data_start;
            const char *it;
            unsigned int expand_length;
            unsigned int i, j;

            block_length = _tgv_length_get(m + offset, length, &offset);

            if (block_length == 0) goto on_error;

            data_start = m + offset;
            offset += block_length;

            EINA_RECTANGLE_SET(&current, x, y,
                               bwidth, bheight);

            if (!eina_rectangle_intersection(&current, &master))
              continue;

            if (compress)
              {
                 expand_length = LZ4_uncompress(data_start, buffer,
                                                block_count * etc_block_size);
                 // That's an overhead for now, need to be fixed
                 if (expand_length != block_length)
                   goto on_error;
              }
            else
              {
                 buffer = (void*) data_start;
                 if (block_count * etc_block_size != block_length)
                   goto on_error;
              }
            it = buffer;

            for (i = 0; i < bheight; i += 4)
              for (j = 0; j < bwidth; j += 4, it += etc_block_size)
                {
                   Eina_Rectangle current_etc;
                   unsigned int temporary[4 * 4];
                   unsigned int offset_x, offset_y;
                   int k, l;

                   EINA_RECTANGLE_SET(&current_etc, x + j, y + i, 4, 4);

                   if (!eina_rectangle_intersection(&current_etc, &current))
                     continue;

                   switch (cspace)
                     {
                      case EET_COLORSPACE_ARGB8888:
                        switch (file_cspace)
                          {
                           case EET_COLORSPACE_ETC1:
                           case EET_COLORSPACE_ETC1_ALPHA:
                             if (!rg_etc1_unpack_block(it, temporary, 0))
                               {
                                  // TODO: Should we decode as RGB8_ETC2?
                                  fprintf(stderr, "ETC1: Block starting at {%i, %i} is corrupted!\n", x + j, y + i);
                                  continue;
                               }
                             break;
                           case EET_COLORSPACE_RGB8_ETC2:
                             rg_etc2_rgb8_decode_block((uint8_t *) it, temporary);
                             break;
                           case EET_COLORSPACE_RGBA8_ETC2_EAC:
                             rg_etc2_rgba8_decode_block((uint8_t *) it, temporary);
                             break;
                           default: abort();
                          }

                        offset_x = current_etc.x - x - j;
                        offset_y = current_etc.y - y - i;

                        if (!plane)
                          {
#ifdef BUILD_NEON
                             if (eina_cpu_features_get() & EINA_CPU_NEON)
                               {
                                  uint32_t *dst = &p[current_etc.x - 1 + (current_etc.y - 1) * master.w];
                                  uint32_t *src = &temporary[offset_x + offset_y * 4];
                                  for (k = 0; k < current_etc.h; k++)
                                    {
                                       if (current_etc.w == 4)
                                         vst1q_u32(dst, vld1q_u32(src));
                                       else if (current_etc.w == 3)
                                         {
                                            vst1_u32(dst, vld1_u32(src));
                                            *(dst + 2) = *(src + 2);
                                         }
                                       else if (current_etc.w == 2)
                                         vst1_u32(dst, vld1_u32(src));
                                       else
                                          *dst = *src;
                                       dst += master.w;
                                       src += 4;
                                    }
                               }
                             else
#endif
                             for (k = 0; k < current_etc.h; k++)
                               {
                                  memcpy(&p[current_etc.x - 1 + (current_etc.y - 1 + k) * master.w],
                                         &temporary[offset_x + (offset_y + k) * 4],
                                         current_etc.w * sizeof (unsigned int));
                               }
                          }
                        else
                          {
                             for (k = 0; k < current_etc.h; k++)
                               for (l = 0; l < current_etc.w; l++)
                                 {
                                    unsigned int *rgbdata =
                                      &p[current_etc.x - 1 + (current_etc.y - 1 + k) * master.w + l];
                                    unsigned int *adata =
                                      &temporary[offset_x + (offset_y + k) * 4 + l];
                                    A_VAL(rgbdata) = G_VAL(adata);
                                 }
                          }
                        break;
                      case EET_COLORSPACE_ETC1:
                      case EET_COLORSPACE_RGB8_ETC2:
                      case EET_COLORSPACE_RGBA8_ETC2_EAC:
                        memcpy(&p_etc[(current_etc.x / 4) * etc_block_size +
                                      (current_etc.y / 4) * etc_width],
                               it, etc_block_size);
                        break;
                      case EET_COLORSPACE_ETC1_ALPHA:
                        memcpy(&p_etc[(current_etc.x / 4) * etc_block_size +
                                      (current_etc.y / 4) * etc_width +
                                      plane * alpha_offset],
                               it, etc_block_size);
                        break;
                      default:
                        abort();
                     }
                } // bx,by inside blocks
         } // x,y macroblocks

   // TODO: Add support for more unpremultiplied modes (ETC2)
   if ((cspace == EET_COLORSPACE_ARGB8888) && unpremul)
     _eet_argb_premul(p, w * h);

   return 1;

on_error:
   ERR("ETC image data is corrupted in this EET file");
   return 0;
}

static void *
eet_data_image_lossless_convert(int         *size,
                                const void  *data,
                                unsigned int w,
                                unsigned int h,
                                int          alpha)
{
   if (_eet_image_words_bigendian == -1)
     {
        unsigned long int v;

        v = htonl(0x12345678);
        if (v == 0x12345678)
          _eet_image_words_bigendian = 1;
        else
          _eet_image_words_bigendian = 0;
     }

   {
      unsigned char *d;
      int *header;

      d = malloc((w * h * 4) + (8 * 4));
      if (!d)
        return NULL;

      header = (int *)d;
      memset(d, 0, 32);

      header[0] = 0xac1dfeed;
      header[1] = w;
      header[2] = h;
      header[3] = alpha;

      memcpy(d + 32, data, w * h * 4);

      if (_eet_image_words_bigendian)
        {
           unsigned int i;

           for (i = 0; i < ((w * h) + 8); i++) SWAP32(header[i]);
        }

      *size = ((w * h * 4) + (8 * 4));
      return d;
   }
}

static void *
eet_data_image_lossless_compressed_convert(int         *size,
                                           const void  *data,
                                           unsigned int w,
                                           unsigned int h,
                                           int          alpha,
                                           int          compression)
{
   if (_eet_image_words_bigendian == -1)
     {
        unsigned long int v;

        v = htonl(0x12345678);
        if (v == 0x12345678)
          _eet_image_words_bigendian = 1;
        else
          _eet_image_words_bigendian = 0;
     }

   {
      unsigned char *d, *comp;
      int *header, *bigend_data = NULL, ret, ok = 1;
      uLongf buflen = 0;

      buflen = (((w * h * 101) / 100) + 3) * 4;
      ret = LZ4_compressBound((w * h * 4));
      if ((ret > 0) && ((uLongf)ret > buflen)) buflen = ret;

      if (_eet_image_words_bigendian)
        {
           unsigned int i;

           bigend_data = (int *) malloc(w * h * 4);
           if (!bigend_data) return NULL;

           memcpy(bigend_data, data, w * h * 4);
           for (i = 0; i < w * h; i++) SWAP32(bigend_data[i]);

           data = (const char *) bigend_data;
        }

      comp = malloc(buflen);
      if (!comp)
        {
         free(bigend_data);
         return NULL;
        }

      switch (compression)
        {
         case EET_COMPRESSION_VERYFAST:
           ret = LZ4_compressHC((const char *)data, (char *)comp,
                                (w * h * 4));
           if (ret <= 0) ok = 0;
           buflen = ret;
           break;
         case EET_COMPRESSION_SUPERFAST:
           ret = LZ4_compress((const char *)data, (char *)comp,
                              (w * h * 4));
           if (ret <= 0) ok = 0;
           buflen = ret;
           break;
         default: /* zlib etc. */
           ret = compress2((Bytef *)comp, &buflen, (Bytef *)(data),
                           (uLong)(w * h * 4), compression);
           if (ret != Z_OK) ok = 0;
           break;
        }
      if ((!ok) || (buflen > (w * h * 4)))
        {
           free(comp);
           free(bigend_data);
           *size = -1;
           return NULL;
        }

      d = malloc((8 * sizeof(int)) + buflen);
      if (!d)
        {
           free(comp);
           free(bigend_data);
           return NULL;
        }

      header = (int *)d;
      memset(d, 0, 8 * sizeof(int));
      header[0] = 0xac1dfeed;
      header[1] = w;
      header[2] = h;
      header[3] = alpha;
      header[4] = compression;

      if (_eet_image_words_bigendian)
        {
           unsigned int i;
           
           for (i = 0; i < 8; i++) SWAP32(header[i]);
           free(bigend_data);
        }

      memcpy(d + (8 * sizeof(int)), comp, buflen);
      *size = (8 * sizeof(int)) + buflen;
      free(comp);
      return d;
   }
}

static int
_block_size_get(int size)
{
   static const int MAX_BLOCK = 6; // 256 pixels

   int k = 0;
   while ((4 << k) < size) k++;
   k = MAX(0, k - 1);
   if ((size * 3 / 2) >= (4 << k)) return MAX(0, MIN(k - 1, MAX_BLOCK));
   return MIN(k, MAX_BLOCK);
}

static inline void
_alpha_to_greyscale_convert(uint32_t *data, int len)
{
   for (int k = 0; k < len; k++)
     {
        int alpha = A_VAL(data);
        *data++ = ARGB_JOIN(alpha, alpha, alpha, alpha);
     }
}

static void *
eet_data_image_etc1_compressed_convert(int         *size,
                                       const unsigned char *data8,
                                       unsigned int w,
                                       unsigned int h,
                                       int          quality,
                                       int          compress,
                                       Eet_Image_Encoding lossy)
{
   rg_etc1_pack_params param;
   uint8_t *comp = NULL;
   uint8_t *buffer;
   uint32_t *data;
   uint32_t nl_width, nl_height;
   uint8_t header[8] = "TGV1";
   int block_width, block_height, macro_block_width, macro_block_height;
   int block_count, image_stride, image_height, etc_block_size;
   int num_planes = 1;
   Eet_Colorspace cspace;
   Eina_Bool unpremul = EINA_FALSE, alpha_texture = EINA_FALSE;
   Eina_Binbuf *r = NULL;
   void *result;
   const char *codec;

   r = eina_binbuf_new();
   if (!r) return NULL;

   image_stride = w;
   image_height = h;
   nl_width = htonl(image_stride);
   nl_height = htonl(image_height);
   compress = !!compress;

   // Disable dithering, as it will deteriorate the quality of flat surfaces
   param.m_dithering = 0;

   if (quality > 95)
     param.m_quality = rg_etc1_high_quality;
   else if (quality > 30)
     param.m_quality = rg_etc1_medium_quality;
   else
     param.m_quality = rg_etc1_low_quality;

   // header[4]: 4 bit block width, 4 bit block height
   block_width = _block_size_get(image_stride + 2);
   block_height = _block_size_get(image_height + 2);
   header[4] = (block_height << 4) | block_width;

   // header[5]: 0 for ETC1, 1 for RGB8_ETC2, 2 for RGBA8_ETC2_EAC, 3 for ETC1_ALPHA
   switch (lossy)
     {
      case EET_IMAGE_ETC1:
        cspace = EET_COLORSPACE_ETC1;
        etc_block_size = 8;
        header[5] = 0;
        codec = "ETC1";
        break;
      case EET_IMAGE_ETC2_RGB:
        cspace = EET_COLORSPACE_RGB8_ETC2;
        etc_block_size = 8;
        header[5] = 1;
        codec = "ETC2 (RGB)";
        break;
      case EET_IMAGE_ETC2_RGBA:
        cspace = EET_COLORSPACE_RGBA8_ETC2_EAC;
        etc_block_size = 16;
        header[5] = 2;
        codec = "ETC2 (RGBA)";
        break;
      case EET_IMAGE_ETC1_ALPHA:
        cspace = EET_COLORSPACE_ETC1_ALPHA;
        etc_block_size = 8;
        num_planes = 2; // RGB and Alpha
        header[5] = 3;
        codec = "ETC1+Alpha";
        break;
      default: abort();
     }

   // header[6]: 0 for raw, 1, for LZ4 compressed, 4 for unpremultiplied RGBA
   // blockless mode (0x2) is never used here
   header[6] = (compress ? 0x1 : 0x0) | (unpremul ? 0x4 : 0x0);

   // header[7]: unused options
   // Note: consider extending the header instead of filling all the bits here
   header[7] = 0;

   // Encoding being super slow, let's inform the user first.
   // FIXME: Ctrl+C must be handled
   INF("Encoding %dx%d image to %s, this may take a while...", w, h, codec);

   // Write header
   eina_binbuf_append_length(r, header, sizeof (header));
   eina_binbuf_append_length(r, (unsigned char*) &nl_width, sizeof (nl_width));
   eina_binbuf_append_length(r, (unsigned char*) &nl_height, sizeof (nl_height));

   // Real block size in pixels, obviously a multiple of 4
   macro_block_width = 4 << block_width;
   macro_block_height = 4 << block_height;

   // Number of ETC1 blocks in a compressed block
   block_count = (macro_block_width * macro_block_height) / (4 * 4);
   buffer = alloca(block_count * etc_block_size);

   if (compress)
     comp = alloca(LZ4_compressBound(block_count * etc_block_size));

   // Write a whole plane (RGB or Alpha)
   for (int plane = 0; plane < num_planes; plane++)
     {
        if (!alpha_texture)
          {
             // Normal mode
             data = (uint32_t *) data8;
          }
        else if (!plane)
          {
             int len = image_stride * image_height;
             // RGB plane for ETC1+Alpha
             data = malloc(len * 4);
             if (!data) goto finish;
             memcpy(data, data8, len * 4);
             if (unpremul) _eet_argb_unpremul(data, len);
          }
        else
          {
             // Alpha plane for ETC1+Alpha
             _alpha_to_greyscale_convert(data, image_stride * image_height);
          }

        // Write macro block
        for (int y = 0; y < image_height + 2; y += macro_block_height)
          {
             uint32_t *input, *last_col, *last_row, *last_pix;
             int real_y;
             int wlen;

             if (y == 0) real_y = 0;
             else if (y < image_height + 1) real_y = y - 1;
             else real_y = image_height - 1;

             for (int x = 0; x < image_stride + 2; x += macro_block_width)
               {
                  uint8_t *offset = buffer;
                  int real_x = x;

                  if (x == 0) real_x = 0;
                  else if (x < image_stride + 1) real_x = x - 1;
                  else real_x = image_stride - 1;

                  input = data + real_y * image_stride + real_x;
                  last_row = data + image_stride * (image_height - 1) + real_x;
                  last_col = data + (real_y + 1) * image_stride - 1;
                  last_pix = data + image_height * image_stride - 1;

                  for (int by = 0; by < macro_block_height; by += 4)
                    {
                       int dup_top = ((y + by) == 0) ? 1 : 0;
                       int max_row = MAX(0, MIN(4, image_height - real_y - by));
                       int oy = (y == 0) ? 1 : 0;

                       for (int bx = 0; bx < macro_block_width; bx += 4)
                         {
                            int dup_left = ((x + bx) == 0) ? 1 : 0;
                            int max_col = MAX(0, MIN(4, image_stride - real_x - bx));
                            uint32_t todo[16] = { 0 };
                            int row, col;
                            int ox = (x == 0) ? 1 : 0;

                            if (dup_left)
                              {
                                 // Duplicate left column
                                 for (row = 0; row < max_row; row++)
                                   todo[row * 4] = input[row * image_stride];
                                 for (row = max_row; row < 4; row++)
                                   todo[row * 4] = last_row[0];
                              }

                            if (dup_top)
                              {
                                 // Duplicate top row
                                 for (col = 0; col < max_col; col++)
                                   todo[col] = input[MAX(col + bx - ox, 0)];
                                 for (col = max_col; col < 4; col++)
                                   todo[col] = last_col[0];
                              }

                            for (row = dup_top; row < 4; row++)
                              {
                                 for (col = dup_left; col < max_col; col++)
                                   {
                                      if (row < max_row)
                                        {
                                           // Normal copy
                                           todo[row * 4 + col] = input[(row + by - oy) * image_stride + bx + col - ox];
                                        }
                                      else
                                        {
                                           // Copy last line
                                           todo[row * 4 + col] = last_row[col + bx - ox];
                                        }
                                   }
                                 for (col = max_col; col < 4; col++)
                                   {
                                      // Right edge
                                      if (row < max_row)
                                        {
                                           // Duplicate last column
                                           todo[row * 4 + col] = last_col[MAX(row + by - oy, 0) * image_stride];
                                        }
                                      else
                                        {
                                           // Duplicate very last pixel again and again
                                           todo[row * 4 + col] = *last_pix;
                                        }
                                   }
                              }

                            switch (cspace)
                              {
                               case EET_COLORSPACE_ETC1:
                               case EET_COLORSPACE_ETC1_ALPHA:
                                 rg_etc1_pack_block(offset, (uint32_t *) todo, &param);
                                 break;
                               case EET_COLORSPACE_RGB8_ETC2:
                                 etc2_rgb8_block_pack(offset, (uint32_t *) todo, &param);
                                 break;
                               case EET_COLORSPACE_RGBA8_ETC2_EAC:
                                 etc2_rgba8_block_pack(offset, (uint32_t *) todo, &param);
                                 break;
                               default: return 0;
                              }

#ifdef DEBUG_STATS
                            if (plane == 0)
                              {
                                 // Decode to compute PSNR, this is slow.
                                 uint32_t done[16];

                                 if (alpha)
                                   rg_etc2_rgba8_decode_block(offset, done);
                                 else
                                    rg_etc2_rgb8_decode_block(offset, done);

                                 for (int k = 0; k < 16; k++)
                                   {
                                      const int r = (R_VAL(&(todo[k])) - R_VAL(&(done[k])));
                                      const int g = (G_VAL(&(todo[k])) - G_VAL(&(done[k])));
                                      const int b = (B_VAL(&(todo[k])) - B_VAL(&(done[k])));
                                      const int a = (A_VAL(&(todo[k])) - A_VAL(&(done[k])));
                                      mse += r*r + g*g + b*b;
                                      if (alpha) mse_alpha += a*a;
                                      mse_div++;
                                   }
                              }
#endif

                            offset += etc_block_size;
                         }
                    }

                  if (compress)
                    {
                       wlen = LZ4_compressHC((char *) buffer, (char *) comp,
                                             block_count * etc_block_size);
                    }
                  else
                    {
                       comp = buffer;
                       wlen = block_count * etc_block_size;
                    }

                  if (wlen > 0)
                    {
                       unsigned int blen = wlen;

                       while (blen)
                         {
                            unsigned char plen;

                            plen = blen & 0x7F;
                            blen = blen >> 7;

                            if (blen) plen = 0x80 | plen;
                            eina_binbuf_append_length(r, &plen, 1);
                         }
                       eina_binbuf_append_length(r, (unsigned char *) comp, wlen);
                    }
               } // 4 rows
          } // macroblocks
     } // planes

finish:
   if (alpha_texture) free(data);
   *size = eina_binbuf_length_get(r);
   result = eina_binbuf_string_steal(r);
   eina_binbuf_free(r);

   return result;
}

static void *
eet_data_image_jpeg_convert(int         *size,
                            const void  *data,
                            unsigned int w,
                            unsigned int h,
                            int          alpha,
                            int          quality)
{
   struct jpeg_compress_struct cinfo;
   struct _JPEG_error_mgr jerr;
   const int *ptr;
   void *d = NULL;
   size_t sz = 0;
   JSAMPROW *jbuf;
   unsigned char *buf;

   (void)alpha; /* unused */

   buf = alloca(3 * w);

   memset(&cinfo, 0, sizeof (struct jpeg_compress_struct));

   cinfo.err = jpeg_std_error(&(jerr.pub));
   jerr.pub.error_exit = _JPEGFatalErrorHandler;
   jerr.pub.emit_message = _JPEGErrorHandler2;
   jerr.pub.output_message = _JPEGErrorHandler;
   if (setjmp(jerr.setjmp_buffer))
     return NULL;

   jpeg_create_compress(&cinfo);

   if (eet_jpeg_membuf_dst(&cinfo, &d, &sz))
     {
        jpeg_destroy_compress(&cinfo);
        return NULL;
     }

   cinfo.image_width = w;
   cinfo.image_height = h;
   cinfo.input_components = 3;
   cinfo.in_color_space = JCS_RGB;
   cinfo.optimize_coding = FALSE;
   cinfo.dct_method = JDCT_ISLOW; // JDCT_FLOAT JDCT_IFAST(quality loss)
   if (quality < 60) cinfo.dct_method = JDCT_IFAST;
   jpeg_set_defaults(&cinfo);
   jpeg_set_quality(&cinfo, quality, TRUE);

   if (quality >= 90)
     {
        cinfo.comp_info[0].h_samp_factor = 1;
        cinfo.comp_info[0].v_samp_factor = 1;
        cinfo.comp_info[1].h_samp_factor = 1;
        cinfo.comp_info[1].v_samp_factor = 1;
        cinfo.comp_info[2].h_samp_factor = 1;
        cinfo.comp_info[2].v_samp_factor = 1;
     }

   jpeg_start_compress(&cinfo, TRUE);

   while (cinfo.next_scanline < cinfo.image_height)
     {
        unsigned int i, j;

        /* convert scaline from ARGB to RGB packed */
        ptr = ((const int *)data) + cinfo.next_scanline * w;
        for (j = 0, i = 0; i < w; i++)
          {
             buf[j++] = ((*ptr) >> 16) & 0xff;
             buf[j++] = ((*ptr) >> 8) & 0xff;
             buf[j++] = ((*ptr)) & 0xff;
             ptr++;
          }
        jbuf = (JSAMPROW *)(&buf);
        jpeg_write_scanlines(&cinfo, jbuf, 1);
     }

   jpeg_finish_compress(&cinfo);
   jpeg_destroy_compress(&cinfo);

   *size = sz;
   return d;
}

static void *
eet_data_image_jpeg_alpha_convert(int         *size,
                                  const void  *data,
                                  unsigned int w,
                                  unsigned int h,
                                  int          alpha,
                                  int          quality)
{
   unsigned char *d1, *d2;
   unsigned char *d;
   int *header;
   int sz1, sz2;

   (void)alpha; /* unused */

   if (_eet_image_words_bigendian == -1)
     {
        unsigned long int v;

        v = htonl(0x12345678);
        if (v == 0x12345678)
          _eet_image_words_bigendian = 1;
        else
          _eet_image_words_bigendian = 0;
     }

   {
      const int *ptr;
      void *dst = NULL;
      size_t sz = 0;
      struct _JPEG_error_mgr jerr;
      JSAMPROW *jbuf;
      struct jpeg_compress_struct cinfo;
      unsigned char *buf;

      buf = alloca(3 * w);

      cinfo.err = jpeg_std_error(&(jerr.pub));
      jerr.pub.error_exit = _JPEGFatalErrorHandler;
      jerr.pub.emit_message = _JPEGErrorHandler2;
      jerr.pub.output_message = _JPEGErrorHandler;
      if (setjmp(jerr.setjmp_buffer))
        return NULL;

      jpeg_create_compress(&cinfo);
      if (eet_jpeg_membuf_dst(&cinfo, &dst, &sz))
        {
           jpeg_destroy_compress(&cinfo);
           return NULL;
        }

      cinfo.image_width = w;
      cinfo.image_height = h;
      cinfo.input_components = 3;
      cinfo.in_color_space = JCS_RGB;
      cinfo.optimize_coding = FALSE;
      cinfo.dct_method = JDCT_ISLOW; // JDCT_FLOAT JDCT_IFAST(quality loss)
      if (quality < 60) cinfo.dct_method = JDCT_IFAST;
      jpeg_set_defaults(&cinfo);
      jpeg_set_quality(&cinfo, quality, TRUE);
      if (quality >= 90)
        {
           cinfo.comp_info[0].h_samp_factor = 1;
           cinfo.comp_info[0].v_samp_factor = 1;
           cinfo.comp_info[1].h_samp_factor = 1;
           cinfo.comp_info[1].v_samp_factor = 1;
           cinfo.comp_info[2].h_samp_factor = 1;
           cinfo.comp_info[2].v_samp_factor = 1;
        }

      jpeg_start_compress(&cinfo, TRUE);

      while (cinfo.next_scanline < cinfo.image_height)
        {
           unsigned int i, j;

           ptr = ((const int *)data) + cinfo.next_scanline * w;
           /* convert scaline from ARGB to RGB packed */
           for (j = 0, i = 0; i < w; i++)
             {
                buf[j++] = ((*ptr) >> 16) & 0xff;
                buf[j++] = ((*ptr) >> 8) & 0xff;
                buf[j++] = ((*ptr)) & 0xff;
                ptr++;
             }
           jbuf = (JSAMPROW *)(&buf);
           jpeg_write_scanlines(&cinfo, jbuf, 1);
        }

      jpeg_finish_compress(&cinfo);
      jpeg_destroy_compress(&cinfo);

      d1 = dst;
      sz1 = sz;
   }
   {
      const int *ptr;
      void *dst = NULL;
      size_t sz = 0;
      struct _JPEG_error_mgr jerr;
      JSAMPROW *jbuf;
      struct jpeg_compress_struct cinfo;
      unsigned char *buf;

      buf = alloca(3 * w);

      cinfo.err = jpeg_std_error(&(jerr.pub));
      jerr.pub.error_exit = _JPEGFatalErrorHandler;
      jerr.pub.emit_message = _JPEGErrorHandler2;
      jerr.pub.output_message = _JPEGErrorHandler;
      if (setjmp(jerr.setjmp_buffer))
        {
           free(d1);
           return NULL;
        }

      jpeg_create_compress(&cinfo);
      if (eet_jpeg_membuf_dst(&cinfo, &dst, &sz))
        {
           jpeg_destroy_compress(&cinfo);
           free(d1);
           return NULL;
        }

      cinfo.image_width = w;
      cinfo.image_height = h;
      cinfo.input_components = 1;
      cinfo.in_color_space = JCS_GRAYSCALE;
      jpeg_set_defaults(&cinfo);
      jpeg_set_quality(&cinfo, quality, TRUE);
      if (quality >= 90)
        {
           cinfo.comp_info[0].h_samp_factor = 1;
           cinfo.comp_info[0].v_samp_factor = 1;
           cinfo.comp_info[1].h_samp_factor = 1;
           cinfo.comp_info[1].v_samp_factor = 1;
           cinfo.comp_info[2].h_samp_factor = 1;
           cinfo.comp_info[2].v_samp_factor = 1;
        }

      jpeg_start_compress(&cinfo, TRUE);

      while (cinfo.next_scanline < cinfo.image_height)
        {
           unsigned int i, j;

           ptr = ((const int *)data) + cinfo.next_scanline * w;
           /* convert scaline from ARGB to RGB packed */
           for (j = 0, i = 0; i < w; i++)
             {
                buf[j++] = ((*ptr) >> 24) & 0xff;
                ptr++;
             }
           jbuf = (JSAMPROW *)(&buf);
           jpeg_write_scanlines(&cinfo, jbuf, 1);
        }

      jpeg_finish_compress(&cinfo);
      jpeg_destroy_compress(&cinfo);

      d2 = dst;
      sz2 = sz;
   }
   d = malloc(12 + sz1 + sz2);
   if (!d)
     {
        free(d1);
        free(d2);
        return NULL;
     }

   header = (int *)d;
   header[0] = 0xbeeff00d;
   header[1] = sz1;
   header[2] = sz2;
   if (_eet_image_words_bigendian)
     {
        int i;

        for (i = 0; i < 3; i++) SWAP32(header[i]);
     }

   memcpy(d + 12, d1, sz1);
   memcpy(d + 12 + sz1, d2, sz2);

   free(d1);
   free(d2);
   *size = 12 + sz1 + sz2;
   return d;
}

EAPI int
eet_data_image_write_cipher(Eet_File    *ef,
                            const char  *name,
                            const char  *cipher_key,
                            const void  *data,
                            unsigned int w,
                            unsigned int h,
                            int          alpha,
                            int          comp,
                            int          quality,
                            Eet_Image_Encoding lossy)
{
   void *d = NULL;
   int size = 0;

   d = eet_data_image_encode(data, &size, w, h, alpha, comp, quality, lossy);
   if (d)
     {
        int v;

        v = eet_write_cipher(ef, name, d, size, 0, cipher_key);
        free(d);
        return v;
     }

   return 0;
}

EAPI int
eet_data_image_write(Eet_File    *ef,
                     const char  *name,
                     const void  *data,
                     unsigned int w,
                     unsigned int h,
                     int          alpha,
                     int          comp,
                     int          quality,
                     Eet_Image_Encoding lossy)
{
   return eet_data_image_write_cipher(ef,
                                      name,
                                      NULL,
                                      data,
                                      w,
                                      h,
                                      alpha,
                                      comp,
                                      quality,
                                      lossy);
}

EAPI void *
eet_data_image_read_cipher(Eet_File     *ef,
                           const char   *name,
                           const char   *cipher_key,
                           unsigned int *w,
                           unsigned int *h,
                           int          *alpha,
                           int          *comp,
                           int          *quality,
                           Eet_Image_Encoding *lossy)
{
   unsigned int *d = NULL;
   void *data = NULL;
   int free_data = 0;
   int size;

   if (!cipher_key)
     data = (void *)eet_read_direct(ef, name, &size);

   if (!data)
     {
        data = eet_read_cipher(ef, name, &size, cipher_key);
        free_data = 1;
        if (!data)
          return NULL;
     }

   d = eet_data_image_decode(data, size, w, h, alpha, comp, quality, lossy);

   if (free_data)
     free(data);

   return d;
}

EAPI void *
eet_data_image_read(Eet_File     *ef,
                    const char   *name,
                    unsigned int *w,
                    unsigned int *h,
                    int          *alpha,
                    int          *comp,
                    int          *quality,
                    Eet_Image_Encoding *lossy)
{
   return eet_data_image_read_cipher(ef, name, NULL, w, h, alpha,
                                     comp, quality, lossy);
}

EAPI int
eet_data_image_read_to_cspace_surface_cipher(Eet_File     *ef,
                                             const char   *name,
                                             const char   *cipher_key,
                                             unsigned int  src_x,
                                             unsigned int  src_y,
                                             unsigned int *d,
                                             unsigned int  w,
                                             unsigned int  h,
                                             unsigned int  row_stride,
                                             Eet_Colorspace cspace,
                                             int          *alpha,
                                             int          *comp,
                                             int          *quality,
                                             Eet_Image_Encoding *lossy)
{
   void *data = NULL;
   int free_data = 0;
   int res = 1;
   int size;

   if (!cipher_key)
     data = (void *)eet_read_direct(ef, name, &size);

   if (!data)
     {
        data = eet_read_cipher(ef, name, &size, cipher_key);
        free_data = 1;
        if (!data)
          return 0;
     }

   res = eet_data_image_decode_to_cspace_surface_cipher(data, NULL, size, src_x, src_y, d,
                                                        w, h, row_stride, cspace, alpha,
                                                        comp, quality, lossy);

   if (free_data)
     free(data);

   return res;
}

EAPI int
eet_data_image_read_to_surface_cipher(Eet_File     *ef,
                                      const char   *name,
                                      const char   *cipher_key,
                                      unsigned int  src_x,
                                      unsigned int  src_y,
                                      unsigned int *d,
                                      unsigned int  w,
                                      unsigned int  h,
                                      unsigned int  row_stride,
                                      int          *alpha,
                                      int          *comp,
                                      int          *quality,
                                      Eet_Image_Encoding *lossy)
{
   return eet_data_image_read_to_cspace_surface_cipher(ef, name, cipher_key,
                                                       src_x, src_y, d, w, h, row_stride,
                                                       EET_COLORSPACE_ARGB8888,
                                                       alpha, comp, quality, lossy);
}

EAPI int
eet_data_image_read_to_surface(Eet_File     *ef,
                               const char   *name,
                               unsigned int  src_x,
                               unsigned int  src_y,
                               unsigned int *d,
                               unsigned int  w,
                               unsigned int  h,
                               unsigned int  row_stride,
                               int          *alpha,
                               int          *comp,
                               int          *quality,
                               Eet_Image_Encoding *lossy)
{
   return eet_data_image_read_to_surface_cipher(ef, name, NULL,
                                                src_x, src_y, d,
                                                w, h, row_stride,
                                                alpha, comp, quality,
                                                lossy);
}

EAPI int
eet_data_image_header_read_cipher(Eet_File     *ef,
                                  const char   *name,
                                  const char   *cipher_key,
                                  unsigned int *w,
                                  unsigned int *h,
                                  int          *alpha,
                                  int          *comp,
                                  int          *quality,
                                  Eet_Image_Encoding *lossy)
{
   void *data = NULL;
   int size = 0;
   int free_data = 0;
   int d;

   if (!cipher_key)
     data = (void *)eet_read_direct(ef, name, &size);

   if (!data)
     {
        data = eet_read_cipher(ef, name, &size, cipher_key);
        free_data = 1;
        if (!data)
          return 0;
     }

   d = eet_data_image_header_decode(data, size, w, h, alpha,
                                    comp, quality, lossy);
   if (free_data)
     free(data);

   return d;
}

EAPI int
eet_data_image_header_read(Eet_File     *ef,
                           const char   *name,
                           unsigned int *w,
                           unsigned int *h,
                           int          *alpha,
                           int          *comp,
                           int          *quality,
                           Eet_Image_Encoding *lossy)
{
   return eet_data_image_header_read_cipher(ef, name, NULL,
                                            w, h, alpha,
                                            comp, quality, lossy);
}

EAPI void *
eet_data_image_encode_cipher(const void  *data,
                             const char  *cipher_key,
                             unsigned int w,
                             unsigned int h,
                             int          alpha,
                             int          comp,
                             int          quality,
                             Eet_Image_Encoding lossy,
                             int         *size_ret)
{
   void *d = NULL;
   void *ciphered_d = NULL;
   unsigned int ciphered_sz = 0;
   int size = 0;

   switch (lossy)
     {
      case EET_IMAGE_LOSSLESS:
         if (comp > 0)
           d = eet_data_image_lossless_compressed_convert(&size, data,
                                                          w, h, alpha, comp);

         /* eet_data_image_lossless_compressed_convert will refuse to compress something
            if the result is bigger than the entry. */
         if (comp <= 0 || !d)
           d = eet_data_image_lossless_convert(&size, data, w, h, alpha);
         break;
      case EET_IMAGE_JPEG:
         if (!alpha)
           d = eet_data_image_jpeg_convert(&size, data, w, h, alpha, quality);
         else
           d = eet_data_image_jpeg_alpha_convert(&size, data,
                                                 w, h, alpha, quality);
         break;
      case EET_IMAGE_ETC1:
      case EET_IMAGE_ETC2_RGB:
        if (alpha) abort();
        // fallthrough
      case EET_IMAGE_ETC2_RGBA:
      case EET_IMAGE_ETC1_ALPHA:
         d = eet_data_image_etc1_compressed_convert(&size, data, w, h,
                                                    quality, comp, lossy);
         break;
      default:
         abort();
     }

   if (cipher_key)
     {
        if(!eet_cipher(d, size, cipher_key, strlen(cipher_key), &ciphered_d,
                       &ciphered_sz))
          {
             if (d)
               free(d);

             d = ciphered_d;
             size = ciphered_sz;
          }
        else
          if (ciphered_d)
            free(ciphered_d);
     }

   if (size_ret)
     *size_ret = size;

   return d;
}

EAPI void *
eet_data_image_encode(const void  *data,
                      int         *size_ret,
                      unsigned int w,
                      unsigned int h,
                      int          alpha,
                      int          comp,
                      int          quality,
                      Eet_Image_Encoding lossy)
{
   return eet_data_image_encode_cipher(data, NULL, w, h, alpha,
                                       comp, quality, lossy, size_ret);
}

EAPI int
eet_data_image_header_decode_cipher(const void   *data,
                                    const char   *cipher_key,
                                    int           size,
                                    unsigned int *w,
                                    unsigned int *h,
                                    int          *alpha,
                                    int          *comp,
                                    int          *quality,
                                    Eet_Image_Encoding *lossy)
{
   int header[8];
   void *deciphered_d = NULL;
   unsigned int deciphered_sz = 0;

   if (cipher_key)
     {
        if (!eet_decipher(data, size, cipher_key, strlen(cipher_key),
                          &deciphered_d, &deciphered_sz))
          {
             data = deciphered_d;
             size = deciphered_sz;
          }
        else
          {
             free(deciphered_d);
             deciphered_d = NULL;
          }
     }

   if (_eet_image_words_bigendian == -1)
     {
        unsigned long int v;

        v = htonl(0x12345678);
        if (v == 0x12345678)
          _eet_image_words_bigendian = 1;
        else
          _eet_image_words_bigendian = 0;
     }

   if (size < 32)
     {
        free(deciphered_d);
        return 0;
     }

   memcpy(header, data, 32);
   if (_eet_image_words_bigendian)
     {
        int i;

        for (i = 0; i < 8; i++) SWAP32(header[i]);
     }

   if ((unsigned)header[0] == 0xac1dfeed)
     {
        int iw, ih, al, cp;

        iw = header[1];
        ih = header[2];
        al = header[3];
        cp = header[4];
        if ((iw < 1) || (ih < 1) || (iw > 8192) || (ih > 8192))
          {
             free(deciphered_d);
             return 0;
          }

        if ((cp == 0) && (size < ((iw * ih * 4) + 32)))
          {
             free(deciphered_d);
             return 0;
          }

        if (w)
          *w = iw;

        if (h)
          *h = ih;

        if (alpha)
          *alpha = al ? 1 : 0;

        if (comp)
          *comp = cp;

        if (lossy)
          *lossy = EET_IMAGE_LOSSLESS;

        if (quality)
          *quality = 100;

        return 1;
     }
   else if ((unsigned)header[0] == 0xbeeff00d)
     {
        unsigned int iw = 0, ih = 0;
        unsigned const char *dt;
        int sz1;
        int ok;

        sz1 = header[1];
/*  sz2 = header[2]; */
        dt = data;
        dt += 12;
        ok = eet_data_image_jpeg_header_decode(dt, sz1, &iw, &ih);
        if (ok)
          {
             if (w)
               *w = iw;

             if (h)
               *h = ih;

             if (alpha)
               *alpha = 1;

             if (comp)
               *comp = 0;

             if (lossy)
               *lossy = EET_IMAGE_JPEG;

             if (quality)
               *quality = 75;

             return 1;
          }
     }
   else if (!strncmp(data, "TGV1", 4))
     {
        const char *m = data;

        if (w) *w = ntohl(*((unsigned int*) &(m[OFFSET_WIDTH])));
        if (h) *h = ntohl(*((unsigned int*) &(m[OFFSET_HEIGHT])));
        if (comp) *comp = m[OFFSET_OPTIONS] & 0x1;
        switch (m[OFFSET_ALGORITHM] & 0xFF)
          {
           case 0:
             if (lossy) *lossy = EET_IMAGE_ETC1;
             if (alpha) *alpha = EINA_FALSE;
             break;
           case 1:
             if (lossy) *lossy = EET_IMAGE_ETC2_RGB;
             if (alpha) *alpha = EINA_FALSE;
             break;
           case 2:
             if (alpha) *alpha = EINA_TRUE;
             if (lossy) *lossy = EET_IMAGE_ETC2_RGBA;
             break;
           case 3:
             if (alpha) *alpha = EINA_TRUE;
             if (lossy) *lossy = EET_IMAGE_ETC1_ALPHA;
             break;
           default:
             return 0;
          }
        if (quality) *quality = 50;

        return 1;
     }
   else
     {
        unsigned int iw = 0, ih = 0;
        int ok;

        ok = eet_data_image_jpeg_header_decode(data, size, &iw, &ih);
        if (ok)
          {
             if (w)
               *w = iw;

             if (h)
               *h = ih;

             if (alpha)
               *alpha = 0;

             if (comp)
               *comp = 0;

             if (lossy)
               *lossy = EET_IMAGE_JPEG;

             if (quality)
               *quality = 75;

             return 1;
          }
     }

   free(deciphered_d);
   return 0;
}

static const Eet_Colorspace _eet_etc1_colorspace[] = {
  EET_COLORSPACE_ETC1,
  EET_COLORSPACE_ARGB8888
};

static const Eet_Colorspace _eet_etc1_alpha_colorspace[] = {
  EET_COLORSPACE_ETC1_ALPHA,
  EET_COLORSPACE_ARGB8888
};

static const Eet_Colorspace _eet_etc2_rgb_colorspace[] = {
  EET_COLORSPACE_RGB8_ETC2,
  EET_COLORSPACE_ARGB8888
};

static const Eet_Colorspace _eet_etc2_rgba_colorspace[] = {
  EET_COLORSPACE_RGBA8_ETC2_EAC,
  EET_COLORSPACE_ARGB8888
};

EAPI int
eet_data_image_colorspace_get(Eet_File *ef,
                              const char *name,
                              const char *cipher_key,
                              const Eet_Colorspace **cspaces)
{
   Eet_Image_Encoding lossy;
   int r;

   r = eet_data_image_header_read_cipher(ef, name, cipher_key, NULL, NULL, NULL, NULL, NULL, &lossy);
   if (!r) return r;

   if (cspaces)
     {
        if (lossy == EET_IMAGE_ETC1)
          *cspaces = _eet_etc1_colorspace;
        else if (lossy == EET_IMAGE_ETC2_RGB)
          *cspaces = _eet_etc2_rgb_colorspace;
        else if (lossy == EET_IMAGE_ETC2_RGBA)
          *cspaces = _eet_etc2_rgba_colorspace;
        else if (lossy == EET_IMAGE_ETC1_ALPHA)
          *cspaces = _eet_etc1_alpha_colorspace;
     }

   return r;
}

EAPI int
eet_data_image_header_decode(const void   *data,
                             int           size,
                             unsigned int *w,
                             unsigned int *h,
                             int          *alpha,
                             int          *comp,
                             int          *quality,
                             Eet_Image_Encoding *lossy)
{
   return eet_data_image_header_decode_cipher(data,
                                              NULL,
                                              size,
                                              w,
                                              h,
                                              alpha,
                                              comp,
                                              quality,
                                              lossy);
}

static void
_eet_data_image_copy_buffer(const unsigned int *src,
                            unsigned int        src_x,
                            unsigned int        src_y,
                            unsigned int        src_w,
                            unsigned int       *dst,
                            unsigned int        w,
                            unsigned int        h,
                            unsigned int        row_stride)
{
   src += src_x + src_y * src_w;

   if (row_stride == src_w * 4 && w == src_w)
     memcpy(dst, src, row_stride * h);
   else
     {
        unsigned int *over = dst;
        unsigned int y;

        for (y = 0; y < h; ++y, src += src_w, over += row_stride)
          memcpy(over, src, w * 4);
     }
}

static int
_eet_data_image_decode_inside(const void   *data,
                              int           size,
                              unsigned int  src_x,
                              unsigned int  src_y,
                              unsigned int  src_w,
                              unsigned int  src_h,
                              unsigned int *d,
                              unsigned int  w,
                              unsigned int  h,
                              unsigned int  row_stride,
                              int           alpha,
                              int           comp,
                              int           quality,
                              Eet_Image_Encoding lossy,
                              Eet_Colorspace cspace)
{
   if (lossy == EET_IMAGE_LOSSLESS && quality == 100)
     {
        unsigned int *body;

        body = ((unsigned int *)data) + 8;
        if (!comp)
          _eet_data_image_copy_buffer(body, src_x, src_y, src_w, d,
                                      w, h, row_stride);
        else
          {
             if ((src_h == h) && (src_w == w) && (row_stride == src_w * 4))
               {
                  switch (comp)
                    {
                     case EET_COMPRESSION_VERYFAST:
                     case EET_COMPRESSION_SUPERFAST:
                       if (LZ4_uncompress((const char *)body,
                                          (char *)d, w * h * 4)
                           != (size - 32)) return 0;
                       break;
                     default:
                         {
                            uLongf dlen = w * h * 4;
                            
                            if (uncompress((Bytef *)d, &dlen, (Bytef *)body,
                                           (uLongf)(size - 32)) != Z_OK)
                              return 0;
                         }
                       break;
                    }
               }
             else
               {
                  switch (comp)
                    {
                     case EET_COMPRESSION_VERYFAST:
                     case EET_COMPRESSION_SUPERFAST:
                         {
                            char *dtmp;
                            
                            dtmp = malloc(src_w * src_h * 4);
                            if (!dtmp) return 0;
                            if (LZ4_uncompress((const char *)body,
                                               dtmp, w * h * 4)
                                != (size - 32))
                              {
                                 free(dtmp);
                                 return 0;
                              }
                            _eet_data_image_copy_buffer((unsigned int *)dtmp,
                                                        src_x, src_y, src_w, d,
                                                        w, h, row_stride);
                            free(dtmp);
                         }
                       break;
                     default:
                         {
                            Bytef *dtmp;
                            uLongf dlen = src_w * src_h * 4;
                  
                            /* FIXME: This could create a huge alloc. So
                             compressed data and tile could not always work.*/
                            dtmp = malloc(dlen);
                            if (!dtmp) return 0;
                  
                            if (uncompress(dtmp, &dlen, (Bytef *)body,
                                           (uLongf)(size - 32)) != Z_OK)
                              {
                                 free(dtmp);
                                 return 0;
                              }
                            _eet_data_image_copy_buffer((unsigned int *)dtmp,
                                                        src_x, src_y, src_w, d,
                                                        w, h, row_stride);
                            free(dtmp);
                         }
                    }
               }
          }
        /* Fix swapiness. */
        if (_eet_image_words_bigendian)
          {
             unsigned int x;

             for (x = 0; x < (w * h); x++) SWAP32(d[x]);
          }
     }
   else if (comp == 0 && lossy == EET_IMAGE_JPEG)
     {
        if (alpha)
          {
             unsigned const char *dt;
             int header[8];
             int sz1, sz2;

             memcpy(header, data, 32);
             if (_eet_image_words_bigendian)
               {
                  int i;

                  for (i = 0; i < 8; i++) SWAP32(header[i]);
               }

             sz1 = header[1];
             sz2 = header[2];
             dt = data;
             dt += 12;

             if (eet_data_image_jpeg_rgb_decode(dt, sz1, src_x, src_y, d, w, h,
                                                row_stride))
               {
                  dt += sz1;
                  if (!eet_data_image_jpeg_alpha_decode(dt, sz2, src_x, src_y,
                                                        d, w, h, row_stride))
                    return 0;
               }
          }
        else if (!eet_data_image_jpeg_rgb_decode(data, size, src_x, src_y, d, w,
                                                 h, row_stride))
          return 0;
     }
   else if ((lossy == EET_IMAGE_ETC1) ||
            (lossy == EET_IMAGE_ETC2_RGB) ||
            (lossy == EET_IMAGE_ETC2_RGBA) ||
            (lossy == EET_IMAGE_ETC1_ALPHA))
     {
        return eet_data_image_etc2_decode(data, size, d,
                                          src_x, src_y, src_w, src_h,
                                          alpha, cspace, lossy);
     }
   else
     abort();

   return 1;
}

EAPI void *
eet_data_image_decode_cipher(const void   *data,
                             const char   *cipher_key,
                             int           size,
                             unsigned int *w,
                             unsigned int *h,
                             int          *alpha,
                             int          *comp,
                             int          *quality,
                             Eet_Image_Encoding *lossy)
{
   unsigned int *d = NULL;
   unsigned int iw, ih;
   int ialpha, icompress, iquality;
   Eet_Image_Encoding ilossy;
   void *deciphered_d = NULL;
   unsigned int deciphered_sz = 0;

   if (cipher_key)
     {
        if (!eet_decipher(data, size, cipher_key, strlen(cipher_key),
                          &deciphered_d, &deciphered_sz))
          {
             data = deciphered_d;
             size = deciphered_sz;
          }
        else
        if (deciphered_d)
          free(deciphered_d);
     }

   /* All check are done during header decode, this simplify the code a lot. */
   if (!eet_data_image_header_decode(data, size, &iw, &ih, &ialpha, &icompress,
                                     &iquality, &ilossy))
     return NULL;

   d = malloc(iw * ih * 4);
   if (!d)
     return NULL;

   if (!_eet_data_image_decode_inside(data, size, 0, 0, iw, ih, d, iw, ih, iw *
                                      4, ialpha, icompress, iquality, ilossy,
                                      EET_COLORSPACE_ARGB8888))
     {
        free(d);
        return NULL;
     }

   if (w)
     *w = iw;

   if (h)
     *h = ih;

   if (alpha)
     *alpha = ialpha;

   if (comp)
     *comp = icompress;

   if (quality)
     *quality = iquality;

   if (lossy)
     *lossy = ilossy;

   return d;
}

EAPI void *
eet_data_image_decode(const void   *data,
                      int           size,
                      unsigned int *w,
                      unsigned int *h,
                      int          *alpha,
                      int          *comp,
                      int          *quality,
                      Eet_Image_Encoding *lossy)
{
   return eet_data_image_decode_cipher(data, NULL, size, w, h,
                                       alpha, comp, quality, lossy);
}

EAPI int
eet_data_image_decode_to_cspace_surface_cipher(const void   *data,
                                               const char   *cipher_key,
                                               int           size,
                                               unsigned int  src_x,
                                               unsigned int  src_y,
                                               unsigned int *d,
                                               unsigned int  w,
                                               unsigned int  h,
                                               unsigned int  row_stride,
                                               Eet_Colorspace cspace,
                                               int          *alpha,
                                               int          *comp,
                                               int          *quality,
                                               Eet_Image_Encoding *lossy)
{
   unsigned int iw, ih;
   int ialpha, icompress, iquality;
   Eet_Image_Encoding ilossy;
   void *deciphered_d = NULL;
   unsigned int deciphered_sz = 0;

   if (cipher_key)
     {
        if (!eet_decipher(data, size, cipher_key, strlen(cipher_key),
                          &deciphered_d, &deciphered_sz))
          {
             data = deciphered_d;
             size = deciphered_sz;
          }
        else
        if (deciphered_d)
          free(deciphered_d);
     }

   /* All check are done during header decode, this simplify the code a lot. */
   if (!eet_data_image_header_decode(data, size, &iw, &ih, &ialpha, &icompress,
                                     &iquality, &ilossy))
     return 0;

   if (!d)
     return 0;

   if (cspace == EET_COLORSPACE_ETC1 &&
       ilossy != EET_IMAGE_ETC1)
     return 0;

   if (cspace == EET_COLORSPACE_RGB8_ETC2 &&
       ilossy != EET_IMAGE_ETC2_RGB)
     return 0;

   if (cspace == EET_COLORSPACE_RGBA8_ETC2_EAC &&
       ilossy != EET_IMAGE_ETC2_RGBA)
     return 0;

   if (cspace == EET_COLORSPACE_ETC1_ALPHA &&
       ilossy != EET_IMAGE_ETC1_ALPHA)
     return 0;

   if (cspace == EET_COLORSPACE_ARGB8888 &&
       w * 4 > row_stride)
     return 0;

   if (w > iw || h > ih)
     return 0;

   if (!_eet_data_image_decode_inside(data, size, src_x, src_y, iw, ih, d, w, h,
                                      row_stride, ialpha, icompress, iquality,
                                      ilossy, cspace))
     return 0;

   if (alpha)
     *alpha = ialpha;

   if (comp)
     *comp = icompress;

   if (quality)
     *quality = iquality;

   if (lossy)
     *lossy = ilossy;

   return 1;
}

EAPI int
eet_data_image_decode_to_surface_cipher(const void   *data,
                                        const char   *cipher_key,
                                        int           size,
                                        unsigned int  src_x,
                                        unsigned int  src_y,
                                        unsigned int *d,
                                        unsigned int  w,
                                        unsigned int  h,
                                        unsigned int  row_stride,
                                        int          *alpha,
                                        int          *comp,
                                        int          *quality,
                                        Eet_Image_Encoding *lossy)
{
   return eet_data_image_decode_to_cspace_surface_cipher(data, cipher_key, size, src_x, src_y, d, w, h, row_stride, EET_COLORSPACE_ARGB8888, alpha, comp, quality, lossy);
}

EAPI int
eet_data_image_decode_to_surface(const void   *data,
                                 int           size,
                                 unsigned int  src_x,
                                 unsigned int  src_y,
                                 unsigned int *d,
                                 unsigned int  w,
                                 unsigned int  h,
                                 unsigned int  row_stride,
                                 int          *alpha,
                                 int          *comp,
                                 int          *quality,
                                 Eet_Image_Encoding *lossy)
{
   return eet_data_image_decode_to_surface_cipher(data, NULL, size,
                                                  src_x, src_y, d,
                                                  w, h, row_stride,
                                                  alpha, comp, quality,
                                                  lossy);
}


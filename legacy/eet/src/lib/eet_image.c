/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
void *alloca (size_t);
#endif

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <jpeglib.h>
#include <setjmp.h>

#include "Eet.h"
#include "Eet_private.h"

/*---*/

typedef struct _JPEG_error_mgr             *emptr;

/*---*/

struct _JPEG_error_mgr
{
   struct     jpeg_error_mgr pub;
   jmp_buf    setjmp_buffer;
};

/*---*/

static void _JPEGFatalErrorHandler(j_common_ptr cinfo);
static void _JPEGErrorHandler(j_common_ptr cinfo);
static void _JPEGErrorHandler2(j_common_ptr cinfo, int msg_level);

static int   eet_data_image_jpeg_header_decode(const void *data, int size, unsigned int *w, unsigned int *h);
static int   eet_data_image_jpeg_rgb_decode(const void *data, int size, unsigned int src_x, unsigned int src_y, unsigned int *d, unsigned int w, unsigned int h, unsigned int row_stride);
static void *eet_data_image_jpeg_alpha_decode(const void *data, int size, unsigned int src_x, unsigned int src_y, unsigned int *d, unsigned int w, unsigned int h, unsigned int row_stride);
static void *eet_data_image_lossless_convert(int *size, const void *data, unsigned int w, unsigned int h, int alpha);
static void *eet_data_image_lossless_compressed_convert(int *size, const void *data, unsigned int w, unsigned int h, int alpha, int compression);
static void *eet_data_image_jpeg_convert(int *size, const void *data, unsigned int w, unsigned int h, int alpha, int quality);
static void *eet_data_image_jpeg_alpha_convert(int *size, const void *data, unsigned int w, unsigned int h, int alpha, int quality);

/*---*/

static int words_bigendian = -1;

/*---*/

#define SWAP64(x) (x) = \
   ((((unsigned long long)(x) & 0x00000000000000ffULL ) << 56) |\
       (((unsigned long long)(x) & 0x000000000000ff00ULL ) << 40) |\
       (((unsigned long long)(x) & 0x0000000000ff0000ULL ) << 24) |\
       (((unsigned long long)(x) & 0x00000000ff000000ULL ) << 8) |\
       (((unsigned long long)(x) & 0x000000ff00000000ULL ) >> 8) |\
       (((unsigned long long)(x) & 0x0000ff0000000000ULL ) >> 24) |\
       (((unsigned long long)(x) & 0x00ff000000000000ULL ) >> 40) |\
       (((unsigned long long)(x) & 0xff00000000000000ULL ) >> 56))
#define SWAP32(x) (x) = \
   ((((int)(x) & 0x000000ff ) << 24) |\
       (((int)(x) & 0x0000ff00 ) << 8) |\
       (((int)(x) & 0x00ff0000 ) >> 8) |\
       (((int)(x) & 0xff000000 ) >> 24))
#define SWAP16(x) (x) = \
   ((((short)(x) & 0x00ff ) << 8) |\
       (((short)(x) & 0xff00 ) >> 8))

#define CONV8(x)
#define CONV16(x) {if (words_bigendian) SWAP16(x);}
#define CONV32(x) {if (words_bigendian) SWAP32(x);}
#define CONV64(x) {if (words_bigendian) SWAP64(x);}

/*---*/

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
eet_data_image_jpeg_header_decode(const void *data, int size, unsigned int *w, unsigned int *h)
{
   struct jpeg_decompress_struct cinfo;
   struct _JPEG_error_mgr jerr;
   FILE *f;

   f = _eet_memfile_read_open(data, (size_t)size);
   if (!f) return 0;
   cinfo.err = jpeg_std_error(&(jerr.pub));
   jerr.pub.error_exit = _JPEGFatalErrorHandler;
   jerr.pub.emit_message = _JPEGErrorHandler2;
   jerr.pub.output_message = _JPEGErrorHandler;
   if (setjmp(jerr.setjmp_buffer))
     {
	jpeg_destroy_decompress(&cinfo);
	_eet_memfile_read_close(f);
	return 0;
     }
   jpeg_create_decompress(&cinfo);
   jpeg_stdio_src(&cinfo, f);
   jpeg_read_header(&cinfo, TRUE);
   cinfo.do_fancy_upsampling = FALSE;
   cinfo.do_block_smoothing = FALSE;
   jpeg_start_decompress(&cinfo);

   /* head decoding */
   *w = cinfo.output_width;
   *h = cinfo.output_height;
   if ((*w < 1) || (*h < 1) || (*w > 8192) || (*h > 8192))
     {
	jpeg_destroy_decompress(&cinfo);
	_eet_memfile_read_close(f);
	return 0;
     }
   /* end head decoding */
   jpeg_destroy_decompress(&cinfo);
   _eet_memfile_read_close(f);
   return 1;
}

static int
eet_data_image_jpeg_rgb_decode(const void *data, int size, unsigned int src_x, unsigned int src_y,
			       unsigned int *d, unsigned int w, unsigned int h, unsigned int row_stride)
{
   struct jpeg_decompress_struct cinfo;
   struct _JPEG_error_mgr jerr;
   unsigned char *ptr, *line[16], *tdata = NULL;
   unsigned int *ptr2, *tmp;
   unsigned int iw, ih;
   int x, y, l, scans;
   int i, count, prevy;
   FILE *f;

   /* FIXME: handle src_x, src_y and row_stride correctly */
   if (!d) return 0;

   f = _eet_memfile_read_open(data, (size_t)size);
   if (!f) return 0;
   cinfo.err = jpeg_std_error(&(jerr.pub));
   jerr.pub.error_exit = _JPEGFatalErrorHandler;
   jerr.pub.emit_message = _JPEGErrorHandler2;
   jerr.pub.output_message = _JPEGErrorHandler;
   if (setjmp(jerr.setjmp_buffer))
     {
	if (tdata) free(tdata);
	jpeg_destroy_decompress(&cinfo);
	_eet_memfile_read_close(f);
	return 0;
     }
   jpeg_create_decompress(&cinfo);
   jpeg_stdio_src(&cinfo, f);
   jpeg_read_header(&cinfo, TRUE);
   cinfo.dct_method = JDCT_FASTEST;
   cinfo.do_fancy_upsampling = FALSE;
   cinfo.do_block_smoothing = FALSE;
   jpeg_start_decompress(&cinfo);

   /* head decoding */
   iw = cinfo.output_width;
   ih = cinfo.output_height;
   if ((iw != w) || (ih != h))
     {
	jpeg_destroy_decompress(&cinfo);
	_eet_memfile_read_close(f);
	return 0;
     }
   /* end head decoding */
   /* data decoding */
   if (cinfo.rec_outbuf_height > 16)
     {
	jpeg_destroy_decompress(&cinfo);
	_eet_memfile_read_close(f);
	return 0;
     }
   tdata = alloca((iw) * 16 * 3);
   ptr2 = d;
   count = 0;
   prevy = 0;

   if (cinfo.output_components == 3)
     {
	for (i = 0; i < cinfo.rec_outbuf_height; i++)
	  line[i] = tdata + (i * (iw) * 3);
	for (l = 0; l < ih; l += cinfo.rec_outbuf_height)
	  {
	     jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
	     scans = cinfo.rec_outbuf_height;
	     if ((ih - l) < scans) scans = ih - l;
	     ptr = tdata;

	     if (l + scans >= src_y && l < src_y + h)
	       {
		  y = src_y - l;
		  if (y < 0) y = 0;
		  for (ptr += 3 * iw * y; y < scans && (y + l) < (src_y + h); y++)
		    {
		       tmp = ptr2;
		       ptr += 3 * src_x;
		       for (x = 0; x < w; x++)
			 {
			    *ptr2 =
			      (0xff000000) | ((ptr[0]) << 16) | ((ptr[1]) << 8) | (ptr[2]);
			    ptr += 3;
			    ptr2++;
			 }
		       ptr += 3 * (iw - w);
		       ptr2 = tmp + row_stride / 4;
		    }
	       }
	     else
	       {
		  ptr += 3 * iw * scans;
	       }
	  }
     }
   else if (cinfo.output_components == 1)
     {
	for (i = 0; i < cinfo.rec_outbuf_height; i++)
	  line[i] = tdata + (i * (iw));
	for (l = 0; l < (ih); l += cinfo.rec_outbuf_height)
	  {
	     jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
	     scans = cinfo.rec_outbuf_height;
	     if (((ih) - l) < scans) scans = (ih) - l;
	     ptr = tdata;

	     if (l >= src_y && l < src_y + h)
	       {
		  y = src_y - l;
		  if (y < 0) y = 0;
		  for (ptr += iw * y; y < scans && (y + l) < (src_y + h); y++)
		    {
		       tmp = ptr2;
		       ptr += src_x;
		       for (x = 0; x < w; x++)
			 {
			    *ptr2 =
			      (0xff000000) | ((ptr[0]) << 16) | ((ptr[0]) << 8) | (ptr[0]);
			    ptr++;
			    ptr2++;
			 }
		       ptr += iw - w;
		       ptr2 = tmp + row_stride / 4;
		    }
	       }
	     else
	       {
		  ptr += iw * scans;
	       }
	  }
     }
   /* end data decoding */
   jpeg_finish_decompress(&cinfo);
   jpeg_destroy_decompress(&cinfo);
   _eet_memfile_read_close(f);
   return 1;
}

static void *
eet_data_image_jpeg_alpha_decode(const void *data, int size, unsigned int src_x, unsigned int src_y,
				 unsigned int *d, unsigned int w, unsigned int h, unsigned int row_stride)
{
   struct jpeg_decompress_struct cinfo;
   struct _JPEG_error_mgr jerr;
   unsigned char *ptr, *line[16], *tdata = NULL;
   unsigned int *ptr2, *tmp;
   int x, y, l, scans;
   int i, count, prevy, iw;
   FILE *f;

   f = _eet_memfile_read_open(data, (size_t)size);
   if (!f) return NULL;

   if (0)
     {
	char buf[1];

	while (fread(buf, 1, 1, f));
	_eet_memfile_read_close(f);
	return d;
     }
   cinfo.err = jpeg_std_error(&(jerr.pub));
   jerr.pub.error_exit = _JPEGFatalErrorHandler;
   jerr.pub.emit_message = _JPEGErrorHandler2;
   jerr.pub.output_message = _JPEGErrorHandler;
   if (setjmp(jerr.setjmp_buffer))
     {
	if (tdata) free(tdata);
	jpeg_destroy_decompress(&cinfo);
	_eet_memfile_read_close(f);
	return NULL;
     }
   jpeg_create_decompress(&cinfo);
   jpeg_stdio_src(&cinfo, f);
   jpeg_read_header(&cinfo, TRUE);
   cinfo.dct_method = JDCT_FASTEST;
   cinfo.do_fancy_upsampling = FALSE;
   cinfo.do_block_smoothing = FALSE;
   jpeg_start_decompress(&cinfo);

   /* head decoding */
   iw = cinfo.output_width;
   if (w != cinfo.output_width
       || h != cinfo.output_height)
     {
	jpeg_destroy_decompress(&cinfo);
	_eet_memfile_read_close(f);
	return NULL;
     }
   /* end head decoding */
   /* data decoding */
   if (cinfo.rec_outbuf_height > 16)
     {
	jpeg_destroy_decompress(&cinfo);
	_eet_memfile_read_close(f);
	return NULL;
     }
   tdata = alloca(w * 16 * 3);
   ptr2 = d;
   count = 0;
   prevy = 0;
   if (cinfo.output_components == 1)
     {
	for (i = 0; i < cinfo.rec_outbuf_height; i++)
	  line[i] = tdata + (i * w);
	for (l = 0; l < h; l += cinfo.rec_outbuf_height)
	  {
	     jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
	     scans = cinfo.rec_outbuf_height;
	     if ((h - l) < scans) scans = h - l;
	     ptr = tdata;

	     if (l >= src_y && l < src_y + h)
	       {
		  y = src_y - l;
		  if (y < 0) y = 0;
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
	     else
	       {
		  ptr += iw * scans;
	       }
	  }
     }
   /* end data decoding */
   jpeg_finish_decompress(&cinfo);
   jpeg_destroy_decompress(&cinfo);
   _eet_memfile_read_close(f);
   return d;
}

static void *
eet_data_image_lossless_convert(int *size, const void *data, unsigned int w, unsigned int h, int alpha)
{
   if (words_bigendian == -1)
     {
	unsigned long int v;

	v = htonl(0x12345678);
	if (v == 0x12345678) words_bigendian = 1;
	else words_bigendian = 0;
     }
     {
	unsigned char *d;
	int           *header;

	d = malloc((w * h * 4) + (8 * 4));
	if (!d) return NULL;

	header = (int *)d;
	memset(d, 0, 32);

	header[0] = 0xac1dfeed;
	header[1] = w;
	header[2] = h;
	header[3] = alpha;

	memcpy(d + 32, data, w * h * 4);

	if (words_bigendian)
	  {
	     unsigned int i;

	     for (i = 0; i < ((w * h) + 8); i++) SWAP32(header[i]);
	  }
	*size = ((w * h * 4) + (8 * 4));
	return d;
     }
}

static void *
eet_data_image_lossless_compressed_convert(int *size, const void *data, unsigned int w, unsigned int h, int alpha, int compression)
{
   if (words_bigendian == -1)
     {
	unsigned long int v;

	v = htonl(0x12345678);
	if (v == 0x12345678) words_bigendian = 1;
	else words_bigendian = 0;
     }

     {
	unsigned char *d;
	unsigned char *comp;
	int           *header;
	int            ret;
	uLongf         buflen;

	d = malloc((w * h * 4) + (8 * 4));
	if (!d) return NULL;
	buflen = (((w * h * 101) / 100) + 3) * 4;
	comp = malloc(buflen);
	if (!comp)
	  {
	     free(d);
	     return NULL;
	  }
	header = (int *)d;
	memset(d, 0, 32);

	header[0] = 0xac1dfeed;
	header[1] = w;
	header[2] = h;
	header[3] = alpha;
	header[4] = compression;
	memcpy(d + 32, data, w * h * 4);

	if (words_bigendian)
	  {
	     unsigned int i;

	     for (i = 0; i < ((w * h) + 8); i++) SWAP32(header[i]);
	  }
	ret = compress2((Bytef *)comp, &buflen,
			(Bytef *)(d + 32),
			(uLong)(w * h * 4),
			compression);
	if (buflen > (w * h * 4))
	  {
	     free(comp);
	     free(d);
	     *size = -1;
	     return NULL;
	  }
	memcpy(d + 32, comp, buflen);
	*size = (8 * 4) + buflen;
	free(comp);
	return d;
     }
}

static void *
eet_data_image_jpeg_convert(int *size, const void *data, unsigned int w, unsigned int h, int alpha, int quality)
{
   const int *ptr;
   void *d = NULL;
   size_t sz = 0;
   struct _JPEG_error_mgr jerr;
   JSAMPROW *jbuf;
   struct jpeg_compress_struct cinfo;
   FILE *f;
   unsigned char *buf;

   (void) alpha; /* unused */

   f =_eet_memfile_write_open(&d, &sz);
   if (!f) return NULL;

   buf = alloca(3 * w);

   cinfo.err = jpeg_std_error(&(jerr.pub));
   jerr.pub.error_exit = _JPEGFatalErrorHandler;
   jerr.pub.emit_message = _JPEGErrorHandler2;
   jerr.pub.output_message = _JPEGErrorHandler;
   if (setjmp(jerr.setjmp_buffer))
     {
	jpeg_destroy_compress(&cinfo);
	_eet_memfile_write_close(f);
	if (d) free(d);
	return NULL;
     }
   jpeg_create_compress(&cinfo);
   jpeg_stdio_dest(&cinfo, f);
   cinfo.image_width = w;
   cinfo.image_height = h;
   cinfo.input_components = 3;
   cinfo.in_color_space = JCS_RGB;
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

   ptr = data;
   while (cinfo.next_scanline < cinfo.image_height)
     {
	unsigned int i, j;

	/* convert scaline from ARGB to RGB packed */
	for (j = 0, i = 0; i < w; i++)
	  {
	     buf[j++] = ((*ptr) >> 16) & 0xff;
	     buf[j++] = ((*ptr) >> 8) & 0xff;
	     buf[j++] = ((*ptr)) & 0xff;
	     ptr++;
	  }
	jbuf = (JSAMPROW *) (&buf);
	jpeg_write_scanlines(&cinfo, jbuf, 1);
     }

   jpeg_finish_compress(&cinfo);
   jpeg_destroy_compress(&cinfo);

   _eet_memfile_write_close(f);
   *size = sz;
   return d;
}

static void *
eet_data_image_jpeg_alpha_convert(int *size, const void *data, unsigned int w, unsigned int h, int alpha, int quality)
{
   unsigned char *d1, *d2;
   unsigned char *d;
   int *header;
   int sz1, sz2;

   (void) alpha; /* unused */

   if (words_bigendian == -1)
     {
	unsigned long int v;

	v = htonl(0x12345678);
	if (v == 0x12345678) words_bigendian = 1;
	else words_bigendian = 0;
     }

     {
	const int *ptr;
	void *d = NULL;
	size_t sz = 0;
	struct _JPEG_error_mgr jerr;
	JSAMPROW *jbuf;
	struct jpeg_compress_struct cinfo;
	FILE *f;
	unsigned char *buf;

	f = _eet_memfile_write_open(&d, &sz);
	if (!f) return NULL;

	buf = alloca(3 * w);

	cinfo.err = jpeg_std_error(&(jerr.pub));
	jerr.pub.error_exit = _JPEGFatalErrorHandler;
	jerr.pub.emit_message = _JPEGErrorHandler2;
	jerr.pub.output_message = _JPEGErrorHandler;
	if (setjmp(jerr.setjmp_buffer))
	  {
	     jpeg_destroy_compress(&cinfo);
	     _eet_memfile_write_close(f);
	     if (d) free(d);
	     return NULL;
	  }
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, f);
	cinfo.image_width = w;
	cinfo.image_height = h;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
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

	ptr = data;
	while (cinfo.next_scanline < cinfo.image_height)
	  {
	     unsigned int i, j;

	     /* convert scaline from ARGB to RGB packed */
	     for (j = 0, i = 0; i < w; i++)
	       {
		  buf[j++] = ((*ptr) >> 16) & 0xff;
		  buf[j++] = ((*ptr) >> 8) & 0xff;
		  buf[j++] = ((*ptr)) & 0xff;
		  ptr++;
	       }
	     jbuf = (JSAMPROW *) (&buf);
	     jpeg_write_scanlines(&cinfo, jbuf, 1);
	  }

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	_eet_memfile_write_close(f);
	d1 = d;
	sz1 = sz;
     }
     {
	const int *ptr;
	void *d = NULL;
	size_t sz = 0;
	struct _JPEG_error_mgr jerr;
	JSAMPROW *jbuf;
	struct jpeg_compress_struct cinfo;
	FILE *f;
	unsigned char *buf;

	f = _eet_memfile_write_open(&d, &sz);
	if (!f)
	  {
	     free(d1);
	     return NULL;
	  }

	buf = alloca(3 * w);

	cinfo.err = jpeg_std_error(&(jerr.pub));
	jerr.pub.error_exit = _JPEGFatalErrorHandler;
	jerr.pub.emit_message = _JPEGErrorHandler2;
	jerr.pub.output_message = _JPEGErrorHandler;
	if (setjmp(jerr.setjmp_buffer))
	  {
	     jpeg_destroy_compress(&cinfo);
	     _eet_memfile_write_close(f);
	     if (d) free(d);
	     free(d1);
	     return NULL;
	  }
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, f);
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

	ptr = data;
	while (cinfo.next_scanline < cinfo.image_height)
	  {
	     unsigned int i, j;

	     /* convert scaline from ARGB to RGB packed */
	     for (j = 0, i = 0; i < w; i++)
	       {
		  buf[j++] = ((*ptr) >> 24) & 0xff;
		  ptr++;
	       }
	     jbuf = (JSAMPROW *) (&buf);
	     jpeg_write_scanlines(&cinfo, jbuf, 1);
	  }

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	_eet_memfile_write_close(f);
	d2 = d;
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
   if (words_bigendian)
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
eet_data_image_write(Eet_File *ef, const char *name,
		     const void *data, unsigned int w, unsigned int h, int alpha,
		     int compress, int quality, int lossy)
{
   void *d = NULL;
   int size = 0;

   d = eet_data_image_encode(data, &size, w, h, alpha, compress, quality, lossy);
   if (d)
     {
	int v;

	v = eet_write(ef, name, d, size, 0);
	free(d);
	return v;
     }
   return 0;
}

EAPI void *
eet_data_image_read(Eet_File *ef, const char *name,
		    unsigned int *w, unsigned int *h, int *alpha,
		    int *compress, int *quality, int *lossy)
{
   unsigned int *d = NULL;
   void		*data;
   int		 size;
   int		 free_data = 0;


   data = (void *)eet_read_direct(ef, name, &size);
   if (!data)
     {
	data = eet_read(ef, name, &size);
	free_data = 1;
     }

   if (!data) return NULL;
   d = eet_data_image_decode(data, size, w, h, alpha, compress, quality, lossy);

   if (free_data)
     free(data);

   return d;
}

EAPI int
eet_data_image_read_to_surface(Eet_File *ef, const char *name, unsigned int src_x, unsigned int src_y,
			       unsigned int *d, unsigned int w, unsigned int h, unsigned int row_stride,
			       int *alpha, int *compress, int *quality, int *lossy)
{
   void		*data;
   int		 size;
   int		 free_data = 0;
   int		 res = 1;


   data = (void *)eet_read_direct(ef, name, &size);
   if (!data)
     {
	data = eet_read(ef, name, &size);
	free_data = 1;
     }

   if (!data) return 0;
   res = eet_data_image_decode_to_surface(data, size, src_x, src_y, d, w, h, row_stride, alpha, compress, quality, lossy);

   if (free_data)
     free(data);

   return res;
}

EAPI int
eet_data_image_header_read(Eet_File *ef, const char *name,
			   unsigned int *w, unsigned int *h, int *alpha,
			   int *compress, int *quality, int *lossy)
{
   void	*data = NULL;
   int	size = 0;
   int	d;
   int	free_data = 0;

   data = (void *)eet_read_direct(ef, name, &size);
   if (!data)
     {
	data = eet_read(ef, name, &size);
	free_data = 1;
     }

   if (!data) return 0;
   d = eet_data_image_header_decode(data, size, w, h, alpha, compress, quality, lossy);
   if (free_data)
     free(data);

   return d;
}

EAPI void *
eet_data_image_encode(const void *data, int *size_ret, unsigned int w, unsigned int h, int alpha, int compress, int quality, int lossy)
{
   void *d = NULL;
   int size = 0;

   if (lossy == 0)
     {
	if (compress > 0)
	  d = eet_data_image_lossless_compressed_convert(&size, data, w, h, alpha, compress);

	/* eet_data_image_lossless_compressed_convert will refuse to compress something
	   if the result is bigger than the entry. */
	if (compress <= 0 || d == NULL)
	  d = eet_data_image_lossless_convert(&size, data, w, h, alpha);
     }
   else
     {
	if (!alpha)
	  d = eet_data_image_jpeg_convert(&size, data, w, h, alpha, quality);
	else
	  d = eet_data_image_jpeg_alpha_convert(&size, data, w, h, alpha, quality);
     }
   if (size_ret) *size_ret = size;
   return d;
}

EAPI int
eet_data_image_header_decode(const void *data, int size, unsigned int *w, unsigned int *h, int *alpha, int *compress, int *quality, int *lossy)
{
   int header[8];

   if (words_bigendian == -1)
     {
	unsigned long int v;

	v = htonl(0x12345678);
	if (v == 0x12345678) words_bigendian = 1;
	else words_bigendian = 0;
     }

   if (size < 32) return 0;

   memcpy(header, data, 32);
   if (words_bigendian)
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
	if ((iw < 1) || (ih < 1) || (iw > 8192) || (ih > 8192)) return 0;
	if ((cp == 0) && (size < ((iw * ih * 4) + 32))) return 0;
	if (w) *w = iw;
	if (h) *h = ih;
	if (alpha) *alpha = al ? 1 : 0;
	if (compress) *compress = cp;
	if (lossy) *lossy = 0;
	if (quality) *quality = 100;
	return 1;
     }
   else if ((unsigned)header[0] == 0xbeeff00d)
     {
	unsigned int iw = 0, ih = 0;
	unsigned const char *dt;
	int sz1, sz2;
	int ok;

	sz1 = header[1];
	sz2 = header[2];
	dt = data;
	dt += 12;
	ok = eet_data_image_jpeg_header_decode(dt, sz1, &iw, &ih);
	if (ok)
	  {
	     if (w) *w = iw;
	     if (h) *h = ih;
	     if (alpha) *alpha = 1;
	     if (compress) *compress = 0;
	     if (lossy) *lossy = 1;
	     if (quality) *quality = 75;
	     return 1;
	  }
     }
   else
     {
	unsigned int iw = 0, ih = 0;
	int ok;

	ok = eet_data_image_jpeg_header_decode(data, size, &iw, &ih);
	if (ok)
	  {
	     if (w) *w = iw;
	     if (h) *h = ih;
	     if (alpha) *alpha = 0;
	     if (compress) *compress = 0;
	     if (lossy) *lossy = 1;
	     if (quality) *quality = 75;
	     return 1;
	  }
     }
   return 0;
}

static void
_eet_data_image_copy_buffer(const unsigned int *src, unsigned int src_x, unsigned int src_y, unsigned int src_w,
			    unsigned int *dst, unsigned int w, unsigned int h, unsigned int row_stride)
{
   src += src_x + src_y * src_w;

   if (row_stride == src_w * 4 && w == src_w)
     {
	memcpy(dst, src, row_stride * h);
     }
   else
     {
	unsigned int *over = dst;
	unsigned int y;

	for (y = 0; y < h; ++y, src += src_w, over += row_stride)
	  memcpy(over, src, w * 4);
     }
}


static int
_eet_data_image_decode_inside(const void *data, int size, unsigned int src_x, unsigned int src_y,
			      unsigned int src_w, unsigned int src_h,
			      unsigned int *d, unsigned int w, unsigned int h, unsigned int row_stride,
			      int alpha, int compress, int quality, int lossy)
{
   if (lossy == 0 && quality == 100)
     {
	unsigned int *body;

	body = ((unsigned int *)data) + 8;
	if (!compress)
	  {
	     _eet_data_image_copy_buffer(body, src_x, src_y, src_w, d, w, h, row_stride);
	  }
	else
	  {
	     if (src_h == h && src_w == w && row_stride == src_w * 4)
	       {
		  uLongf dlen;

		  dlen = w * h * 4;
		  uncompress((Bytef *)d, &dlen, (Bytef *)body,
			     (uLongf)(size - 32));
	       }
	     else
	       {
		  Bytef *dtmp;
		  uLongf dlen = src_w * src_h * 4;

		  /* FIXME: This could create a huge alloc. So compressed data and tile could not always work. */
		  dtmp = malloc(dlen);
		  if (!dtmp) return 0;

		  uncompress(dtmp, &dlen, (Bytef *)body, (uLongf)(size - 32));

		  _eet_data_image_copy_buffer((unsigned int *) dtmp, src_x, src_y, src_w, d, w, h, row_stride);

		  free(dtmp);
	       }
	  }

	/* Fix swapiness. */
	if (words_bigendian)
	  {
	     unsigned int x;

	     for (x = 0; x < (w * h); x++) SWAP32(d[x]);
	  }
     }
   else if (compress == 0 && lossy == 1)
     {
	if (alpha)
	  {
	     unsigned const char *dt;
	     int header[8];
	     int sz1, sz2;

	     memcpy(header, data, 32);
	     if (words_bigendian)
	       {
		  int i;

		  for (i = 0; i < 8; i++) SWAP32(header[i]);
	       }

	     sz1 = header[1];
	     sz2 = header[2];
	     dt = data;
	     dt += 12;

	     if (eet_data_image_jpeg_rgb_decode(dt, sz1, src_x, src_y, d, w, h, row_stride))
	       {
		  dt += sz1;
		  if (!eet_data_image_jpeg_alpha_decode(dt, sz2, src_x, src_y, d, w, h, row_stride))
		    return 0;
	       }
	  }
	else
	  {
	     if (!eet_data_image_jpeg_rgb_decode(data, size, src_x, src_y, d, w, h, row_stride))
	       return 0;
	  }
     }
   else
     {
	abort();
     }

   return 1;
}

EAPI void *
eet_data_image_decode(const void *data, int size, unsigned int *w, unsigned int *h, int *alpha, int *compress, int *quality, int *lossy)
{
   unsigned int *d = NULL;
   unsigned int iw, ih;
   int ialpha, icompress, iquality, ilossy;

   /* All check are done during header decode, this simplify the code a lot. */
   if (!eet_data_image_header_decode(data, size, &iw, &ih, &ialpha, &icompress, &iquality, &ilossy))
     return NULL;

   d = malloc(iw * ih * 4);
   if (!d) return NULL;

   if (!_eet_data_image_decode_inside(data, size, 0, 0, iw, ih, d, iw, ih, iw * 4, ialpha, icompress, iquality, ilossy))
     {
	if (d) free(d);
	return NULL;
     }

   if (w) *w = iw;
   if (h) *h = ih;
   if (alpha) *alpha = ialpha;
   if (compress) *compress = icompress;
   if (quality) *quality = iquality;
   if (lossy) *lossy = ilossy;

   return d;
}

EAPI int
eet_data_image_decode_to_surface(const void *data, int size, unsigned int src_x, unsigned int src_y,
				 unsigned int *d, unsigned int w, unsigned int h, unsigned int row_stride,
				 int *alpha, int *compress, int *quality, int *lossy)
{
   unsigned int iw, ih;
   int ialpha, icompress, iquality, ilossy;

   /* All check are done during header decode, this simplify the code a lot. */
   if (!eet_data_image_header_decode(data, size, &iw, &ih, &ialpha, &icompress, &iquality, &ilossy))
     return 0;

   if (!d) return 0;
   if (w * 4 > row_stride) return 0;
   if (w > iw || h > ih) return 0;

   if (!_eet_data_image_decode_inside(data, size, src_x, src_y, iw, ih, d, w, h, row_stride, ialpha, icompress, iquality, ilossy))
     return 0;

   if (alpha) *alpha = ialpha;
   if (compress) *compress = icompress;
   if (quality) *quality = iquality;
   if (lossy) *lossy = ilossy;

   return 1;
}
